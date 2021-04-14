// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// �鴫�䷢��ģ�����ļ�
// Authors: jdh99 <jdh821@163.com>

#include "dcomblocktx.h"
#include "dcomparam.h"
#include "dcomtx.h"
#include "dcomlog.h"

#include "tzlist.h"
#include "tztime.h"
#include "tzmalloc.h"
#include "crc16.h"
#include "pt.h"

#include <string.h>

// ���м��.��λ:us
#define INTERVAL_RUN 100000

#pragma pack(1)

typedef struct {
    int protocol;
    uint64_t pipe;
    uint64_t dstIA;
    int code;
    int rid;
    int token;

    // ��һ֡��Ҫ�ط�����
    bool isFirstFrame;
    uint64_t firstFrameRetryTime;
    int firstFrameRetryNum;

    uint64_t lastRxAckTime;

    uint16_t crc16;
    int dataLen;
    uint8_t data[];
} tItem;

#pragma pack()

static intptr_t gList = 0;

// checkTimeoutAndRetrySendFirstFrame ��鳬ʱ�ڵ���ط���֡
static void checkTimeoutAndRetrySendFirstFrame(TZListNode* node);

static bool isNodeExist(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token);
static void sendFrame(tItem* item, int offset);
static TZListNode* createNode(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token, uint8_t* data, int dataLen);

// checkNodeAndDealBackFrame ���ڵ��Ƿ��������,��������BACK֡
// ����true��ʾ�ڵ��������
static bool checkNodeAndDealBackFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, TZListNode* node);

// DComBlockTxLoad ģ������
void DComBlockTxLoad(void) {
    gList = TZListCreateList(DComMid);
}

// DComBlockTxRun ģ������
// ����ֵ��PT�к�,���ô���
int DComBlockTxRun(void) {
    static struct pt pt = {0};
    static uint64_t time = 0;

    PT_BEGIN(&pt);

    time = TZTimeGet();
    PT_WAIT_UNTIL(&pt, TZTimeGet() - time > INTERVAL_RUN);

    if (gList == 0) {
        PT_EXIT(&pt);
    }

    TZListNode* node = TZListGetHeader(gList);
    TZListNode* nextNode = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }
        nextNode = node->Next;
        checkTimeoutAndRetrySendFirstFrame(node);
        node = nextNode;
    }

    PT_END(&pt);
}

// checkTimeoutAndRetrySendFirstFrame ��鳬ʱ�ڵ���ط���֡
static void checkTimeoutAndRetrySendFirstFrame(TZListNode* node) {
    tItem* item = (tItem*)node->Data;
    uint64_t now = TZTimeGet();
    if (item->isFirstFrame == false) {
        // ����֡
        if (now - item->lastRxAckTime > (uint64_t)DComParamGetBlockRetryInterval() *
            (uint64_t)DComParamGetBlockRetryMaxNum() * 1000) {
            DComLogWarn("block tx timeout!remove task.token:%d", item->token);
            TZListRemove(gList, node);
        }
        return;
    }

    // ��֡����
    if (now - item->firstFrameRetryTime < (uint64_t)DComParamGetBlockRetryInterval() * 1000) {
        return;
    }

    if (item->firstFrameRetryNum >= DComParamGetBlockRetryMaxNum()) {
        DComLogWarn("block tx timeout!first frame send retry too many.token:%d", item->token);
        TZListRemove(gList, node);
    } else {
        DComLogInfo("block tx send first frame.token:%d retry num:%d", item->token, item->firstFrameRetryNum);
        sendFrame(item, 0);
        item->firstFrameRetryNum++;
        item->firstFrameRetryTime = now;
    }
}

static void sendFrame(tItem* item, int offset) {
    DComLogInfo("block tx send.token:%d offset:%d", item->token, offset);
    int delta = item->dataLen - offset;
    int payloadLen = DCOM_PAYLOAD_SIZE_MAX - sizeof(DComBlockHeader);
    if (payloadLen > delta) {
        payloadLen = delta;
    }

    DComBlockFrame* frame = (DComBlockFrame*)TZMalloc(DComMid, (int)sizeof(DComBlockFrame) + payloadLen);
    if (frame == NULL) {
        return;
    }
    frame->ControlWord.bit.Code = (uint32_t)item->code;
    frame->ControlWord.bit.BlockFlag = 1;
    frame->ControlWord.bit.Rid = (uint32_t)item->rid;
    frame->ControlWord.bit.Token = (uint32_t)item->token;
    frame->ControlWord.bit.PayloadLen = (uint32_t)sizeof(DComBlockHeader) + (uint32_t)payloadLen;
    frame->BlockHeader.Crc16 = item->crc16;
    frame->BlockHeader.Total = (uint16_t)item->dataLen;
    frame->BlockHeader.Offset = (uint16_t)offset;
    memcpy(frame->Payload, item->data + offset, (size_t)payloadLen);
    (void)DComBlockSend(item->protocol, item->pipe, item->dstIA, frame);
    TZFree(frame);
}

// DComBlockTx �鴫�䷢��
void DComBlockTx(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token, uint8_t* data, int dataLen) {
    if (dataLen <= DCOM_PAYLOAD_SIZE_MAX) {
        return;
    }
    if (isNodeExist(protocol, pipe, dstIA, code, rid, token)) {
        return;
    }

    DComLogInfo("block tx new task.token:%d dst ia:0x%x code:%d rid:%d", token, dstIA, code, rid);
    TZListNode* node = createNode(protocol, pipe, dstIA, code, rid, token, data, dataLen);
    if (node == NULL) {
        return;
    }
    tItem* item = (tItem*)node->Data;
    sendFrame(item, 0);
    item->firstFrameRetryNum++;
    item->firstFrameRetryTime = TZTimeGet();
    TZListAppend(gList, node);
}

static bool isNodeExist(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token) {
    TZListNode* node = TZListGetHeader(gList);
    tItem* item = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }
        item = (tItem*)node->Data;

        if (item->protocol == protocol && item->pipe == pipe && item->dstIA == dstIA && item->code == code &&
            item->rid == rid && item->token == token) {
            return true;
        }
        node = node->Next;
    }
    return false;
}

static TZListNode* createNode(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token, uint8_t* data, int dataLen) {
    TZListNode* node = TZListCreateNode(gList);
    if (node == NULL) {
        return NULL;
    }
    node->Data = TZMalloc(DComMid, (int)sizeof(tItem) + dataLen);
    if (node->Data == NULL) {
        TZFree(node);
        return NULL;
    }
    tItem* item = (tItem*)node->Data;
    item->protocol = protocol;
    item->pipe = pipe;
    item->dstIA = dstIA;
    item->code = code;
    item->rid = rid;
    item->token = token;
    item->dataLen = dataLen;
    memcpy(item->data, data, (size_t)dataLen);
    item->crc16 = Checksum(data, (uint16_t)dataLen);

    item->isFirstFrame = true;
    item->firstFrameRetryNum = 0;
    uint64_t now = TZTimeGet();
    item->firstFrameRetryTime = now;
    item->lastRxAckTime = now;
    return node;
}

// DComBlockRxBackFrame ���յ�BACK֡ʱ������
void DComBlockRxBackFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame) {
    if (frame->ControlWord.bit.Code != DCOM_CODE_BACK) {
        return;
    }

    TZListNode* node = TZListGetHeader(gList);
    TZListNode* nextNode = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }
        nextNode = node->Next;
        if (checkNodeAndDealBackFrame(protocol, pipe, srcIA, frame, node)) {
            break;
        }
        node = nextNode;
    }
}

// checkNodeAndDealBackFrame ���ڵ��Ƿ��������,��������BACK֡
// ����true��ʾ�ڵ��������
static bool checkNodeAndDealBackFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, TZListNode* node) {
    tItem* item = (tItem*)node->Data;

    if (item->protocol != protocol || item->pipe != pipe || item->dstIA != srcIA ||
        item->rid != frame->ControlWord.bit.Rid || item->token != frame->ControlWord.bit.Token) {
        return false;
    }
    DComLogInfo("block tx receive back.token:%d", item->token);
    if (frame->ControlWord.bit.PayloadLen != 2) {
        DComLogWarn("block rx receive back deal failed!token:%d payload len is wrong:%d", item->token,
            frame->ControlWord.bit.PayloadLen);
        return false;
    }
    int startOffset = (int)((frame->Payload[0] << 8) + frame->Payload[1]);
    if (startOffset >= item->dataLen) {
        // �������
        DComLogInfo("block tx end.receive back token:%d start offset:%d >= data len:%d", item->token, startOffset,
            item->dataLen);
        TZListRemove(gList, node);
        return true;
    }

    if (item->isFirstFrame) {
        item->isFirstFrame = false;
    }
    item->lastRxAckTime = TZTimeGet();

    sendFrame(item, startOffset);
    return true;
}

// DComBlockTxDealRstFrame �鴫�䷢��ģ�鴦��λ����֡
void DComBlockTxDealRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame) {
    TZListNode* node = TZListGetHeader(gList);
    tItem* item = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }

        item = (tItem*)node->Data;
        if (item->protocol == protocol && item->pipe == pipe && item->dstIA == srcIA &&
            item->rid == frame->ControlWord.bit.Rid && item->token == frame->ControlWord.bit.Token) {
            DComLogWarn("block tx receive rst.token:%d", item->token);
            TZListRemove(gList, node);
            return;
        }

        node = node->Next;
    }
}

// DComBlockRemove �鴫�䷢���Ƴ�����
void DComBlockRemove(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token) {
    TZListNode* node = TZListGetHeader(gList);
    tItem* item = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }

        item = (tItem*)node->Data;
        if (item->protocol == protocol && item->pipe == pipe && item->dstIA == dstIA &&
                item->code == code && item->rid == rid && item->token == token) {
            DComLogWarn("block tx remove task.token:%d", item->token);
            TZListRemove(gList, node);
            return;
        }

        node = node->Next;
    }
}
