// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 块传输接收模块主文件
// Authors: jdh99 <jdh821@163.com>

#include "dcomblockrx.h"
#include "dcomparam.h"
#include "dcomtx.h"
#include "dcomlog.h"

#include "tzlist.h"
#include "tztime.h"
#include "tzmalloc.h"
#include "pt.h"
#include "crc16.h"

#include <string.h>

#pragma pack(1)

typedef struct {
    int protocol;
    uint64_t pipe;
    uint64_t srcIA;
    DComFrame* frame;
    DComBlockHeader blockHeader;
    // 上次发送时间
    uint64_t lastTxTime;
    int retryNums;
} tItem;

#pragma pack()

static intptr_t gList = 0;
static DComBlockRecvFunc gBlockRecv = NULL;

static void sendAllBackFrame(void);
static bool sendBackFrame(tItem* item);
static TZListNode* getNode(int protocol, uint64_t pipe, uint64_t srcIA, DComBlockFrame* frame);
static void createAndAppendNode(int protocol, uint64_t pipe, uint64_t srcIA, DComBlockFrame* frame);

// createNode 创建节点
// total是块接收总字节数
static TZListNode* createNode(int total);

static void editNode(int protocol, uint64_t pipe, TZListNode* node, DComBlockFrame* frame);

// createNode 创建节点
// total是块接收总字节数
static TZListNode* createNode(int total);

// DComBlockRxLoad 模块载入
void DComBlockRxLoad(void) {
    gList = TZListCreateList(DComMid);
}

// DComBlockRxRun 模块运行
// 返回值是PT行号,不用处理
int DComBlockRxRun(void) {
    static struct pt pt = {0};
    static uint64_t time = 0;

    PT_BEGIN(&pt);

    time = TZTimeGet();
    PT_WAIT_UNTIL(&pt, TZTimeGet() - time > INTERVAL);

    sendAllBackFrame();

    PT_END(&pt);
}

static void sendAllBackFrame(void) {
    uint64_t now = TZTimeGet();
    uint64_t interval = (uint64_t)DComParamGetBlockRetryInterval() * 1000;
    int retryMaxNums = DComParamGetBlockRetryMaxNum();

    TZListNode* node = TZListGetHeader(gList);
    TZListNode* nextNode = NULL;
    tItem* item = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }
        nextNode = node->Next;

        do {
            item = (tItem*)node->Data;
            if (now - item->lastTxTime < interval) {
                break;
            }
            if (item->retryNums > retryMaxNums) {
                DComLogWarn("block rx send back retry num too many!token:%d", item->frame->ControlWord.bit.Token);
                TZFree(item->frame);
                TZListRemove(gList, node);
                break;
            }
            // 超时重发
            if (DComIsAllowSend(item->pipe) == false) {
                break;
            }
            DComLogWarn("block rx send back retry num:%d token:%d", item->retryNums, item->frame->ControlWord.bit.Token);
            if (sendBackFrame(item) == false) {
                return;
            }
        } while (0);

        node = nextNode;
    }
}

static bool sendBackFrame(tItem* item) {
    DComLogInfo("block rx send back frame.token:%d offset:%d", item->frame->ControlWord.bit.Token, item->blockHeader.Offset);
    DComFrame* frame = (DComFrame*)TZMalloc(DComMid, sizeof(DComFrame) + 2);
    if (frame == NULL) {
        return false;
    }
    frame->ControlWord.bit.Code = DCOM_CODE_BACK;
    frame->ControlWord.bit.BlockFlag = 0;
    frame->ControlWord.bit.Rid = item->frame->ControlWord.bit.Rid;
    frame->ControlWord.bit.Token = item->frame->ControlWord.bit.Token;
    frame->ControlWord.bit.PayloadLen = 2;
    frame->Payload[0] = item->blockHeader.Offset >> 8;
    frame->Payload[1] = (uint8_t)item->blockHeader.Offset;
    DComSend(item->protocol, item->pipe, item->srcIA, frame);

    TZFree(frame);
    item->retryNums++;
    item->lastTxTime = TZTimeGet();
    return true;
}

// DComBlockRxSetCallback 设置接收回调函数
// 回调函数载荷长度在DComBlockHeader.total字段标识.DComControlWord.PayloadLen字段无效
void DComBlockRxSetCallback(DComBlockRecvFunc recvFunc) {
    gBlockRecv = recvFunc;
}

// DComBlockRxReceive 块传输接收数据
void DComBlockRxReceive(int protocol, uint64_t pipe, uint64_t srcIA, DComBlockFrame* frame) {
    DComLogInfo("block rx receive.token:%d src_ia:0x%x", frame->ControlWord.bit.Token, srcIA);
    TZListNode* node = getNode(protocol, pipe, srcIA, frame);
    if (node == NULL) {
        createAndAppendNode(protocol, pipe, srcIA, frame);
    } else {
        editNode(protocol, pipe, node, frame);
    }
}

static TZListNode* getNode(int protocol, uint64_t pipe, uint64_t srcIA, DComBlockFrame* frame) {
    TZListNode* node = TZListGetHeader(gList);
    tItem* item = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }
        item = (tItem*)node->Data;
        if (item->protocol == protocol && item->pipe == pipe && item->srcIA == srcIA && 
            item->frame->ControlWord.bit.Token == frame->ControlWord.bit.Token && 
            item->frame->ControlWord.bit.Rid == frame->ControlWord.bit.Rid && 
            item->frame->ControlWord.bit.Code == frame->ControlWord.bit.Code) {
            return node;
        }
        node = node->Next;
    }
    return NULL;
}

static void createAndAppendNode(int protocol, uint64_t pipe, uint64_t srcIA, DComBlockFrame* frame) {
    if (frame->BlockHeader.Offset != 0) {
        DComLogWarn("block rx create and append item failed!offset is not 0:%d.token:%d send rst",
			frame->BlockHeader.Offset, frame->ControlWord.bit.Token);
        (void)DComSendRstFrame(protocol, pipe, srcIA, DCM_SYSTEM_ERROR_CODE_WRONG_BLOCK_OFFSET, frame->ControlWord.bit.Rid, 
            frame->ControlWord.bit.Token);
        return;
    }

    TZListNode* node = createNode(frame->BlockHeader.Total);
    if (node == NULL) {
        (void)DComSendRstFrame(protocol, pipe, srcIA, DCOM_SYSTEM_ERROR_NOT_ENOUGH_MEMORY, frame->ControlWord.bit.Rid, 
            frame->ControlWord.bit.Token);
        return;
    }
    tItem* item = (tItem*)node->Data;
    item->pipe = pipe;
    item->srcIA = srcIA;
    item->frame->ControlWord.value = frame->ControlWord.value;
    item->frame->ControlWord.bit.PayloadLen = 0;
    item->blockHeader = frame->BlockHeader;
    memcpy(item->frame->Payload, frame->Payload, frame->ControlWord.bit.PayloadLen - sizeof(DComBlockHeader));
    item->blockHeader.Offset = frame->ControlWord.bit.PayloadLen - sizeof(DComBlockHeader);
    TZListAppend(gList, node);
    (void)sendBackFrame(item);
}

// createNode 创建节点
// total是块接收总字节数
static TZListNode* createNode(int total) {
    TZListNode* node = TZListCreateNode(gList);
    if (node == NULL) {
        return NULL;
    }
    node->Data = TZMalloc(DComMid, sizeof(tItem));
    if (node->Data == NULL) {
        TZFree(node);
        return NULL;
    }
    tItem* item = (tItem*)node->Data;
    item->frame = (DComFrame*)TZMalloc(DComMid, (int)sizeof(DComFrame) + total);
    if (item->frame == NULL) {
        TZFree(node->Data);
        TZFree(node);
        return NULL;
    }
    return node;
}

static void editNode(int protocol, uint64_t pipe, TZListNode* node, DComBlockFrame* frame) {
    tItem* item = (tItem*)node->Data;
    if (item->blockHeader.Offset != frame->BlockHeader.Offset || item->protocol != protocol || item->pipe != pipe) {
        DComLogWarn("block rx edit item failed!token:%d.item<->frame:offset:%d %d,protocol:%d %d,pipe:%d %d",
			frame->ControlWord.bit.Token, item->blockHeader.Offset, frame->BlockHeader.Offset, item->protocol, protocol,
			item->pipe, pipe);
        return;
    }

    memcpy(item->frame->Payload + item->blockHeader.Offset, frame->Payload, 
        frame->ControlWord.bit.PayloadLen - sizeof(DComBlockHeader));
    item->blockHeader.Offset += frame->ControlWord.bit.PayloadLen - sizeof(DComBlockHeader);

    item->retryNums = 0;
    sendBackFrame(item);

    if (item->blockHeader.Offset >= item->blockHeader.Total) {
        DComLogInfo("block rx receive end.token:%d", item->frame->ControlWord.bit.Token);
        uint16_t crcCalc = Crc16Checksum(item->frame->Payload, item->blockHeader.Total);
        if (crcCalc != item->blockHeader.Crc16) {
            DComLogWarn("block rx crc is wrong.token:%d crc calc:0x%x get:0x%x", item->frame->ControlWord.bit.Token, 
                crcCalc, item->blockHeader.Crc16);
            // todo crc错误
            TZFree(item->frame);
            TZListRemove(gList, node);
            return;
        }

        if (gBlockRecv != NULL) {
            gBlockRecv(item->protocol, item->pipe, item->srcIA, item->frame, item->blockHeader.Total);
        }
        TZFree(item->frame);
        TZListRemove(gList, node);
    }
}

// DComBlockRxDealRstFrame 块传输接收模块处理复位连接帧
void DComBlockRxDealRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame) {
    TZListNode* node = TZListGetHeader(gList);
    tItem* item = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }
        item = (tItem*)node->Data;
        if (item->protocol == protocol && item->pipe == pipe && item->srcIA == srcIA && 
            item->frame->ControlWord.bit.Token == frame->ControlWord.bit.Token && 
            item->frame->ControlWord.bit.Rid == frame->ControlWord.bit.Rid) {
            DComLogWarn("block rx rst.token:%d", item->frame->ControlWord.bit.Token);
            TZFree(item->frame);
            TZListRemove(gList, node);
            return;
        }
        node = node->Next;
    }
}
