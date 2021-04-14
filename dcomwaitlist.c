// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// �ȴ��������ļ�
// Authors: jdh99 <jdh821@163.com>

#include "dcom.h"
#include "dcomcommon.h"
#include "dcomparam.h"
#include "dcomtx.h"
#include "dcomblocktx.h"
#include "dcomlog.h"

#include "tzlist.h"
#include "tztime.h"
#include "tzmalloc.h"
#include "pt.h"

#include <string.h>

#pragma pack(1)

typedef struct {
    struct pt pt;
    int protocol;
    uint64_t pipe;
    uint64_t timeoutUs;
    uint8_t* req;
    int reqLen;
    uint8_t** resp;
    int* respLen;
    uint64_t timeStart;
    // �ϴη���ʱ��.�����ش�
    uint64_t lastRetryTimestamp;
    int retryNum;
    int code;
    DComAckCallback ackCallback;

    uint64_t dstIA;
    int rid;
    int token;
    bool isRxAck;
    int* result;
    int ackLen;
    uint8_t* ack;
} tItem;

#pragma pack()

static intptr_t gList = 0;

static void checkWaitItems(void);
static void retrySend(TZListNode* node);

static TZListNode* createNode(void);
static int sendFrame(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token, uint8_t* data, int dataLen);

// checkTimeoutAndRetrySendFirstFrame ���ڵ��Ƿ��������,��������ACK֡
// ����true��ʾ�ڵ��������
static bool checkNodeAndDealAckFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen, TZListNode* node);

// dealRstFrame ����λ����֡
// ����true��ʾ�ڵ��������
static bool dealRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, TZListNode* node);

// DComWaitlistLoad ģ������
void DComWaitlistLoad(void) {
    gList = TZListCreateList(DComMid);
}

// DComWaitlistRun ģ������
int DComWaitlistRun(void) {
    static struct pt pt = {0};
    static uint64_t time = 0;

    PT_BEGIN(&pt);

    time = TZTimeGet();
    PT_WAIT_UNTIL(&pt, TZTimeGet() - time > INTERVAL);

    checkWaitItems();

    PT_END(&pt);
}

static void checkWaitItems(void) {
    TZListNode* node = TZListGetHeader(gList);
    TZListNode* nextNode = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }
        nextNode = node->Next;
        retrySend(node);
        node = nextNode;
    }
}

static void retrySend(TZListNode* node) {
    tItem* item = (tItem*)node->Data;
    uint64_t now = TZTimeGet();
    if (now - item->timeStart > item->timeoutUs) {
        DComLogInfo("wait ack timeout!task failed!token:%d", item->token);
        if (item->reqLen > DCOM_PAYLOAD_SIZE_MAX) {
            DComBlockRemove(item->protocol, item->pipe, item->dstIA, item->code, item->rid, item->token);
        }

        if (item->ackCallback) {
            // �ص���ʽ
            item->ackCallback(NULL, 0, DCOM_SYSTEM_ERROR_CODE_RX_TIMEOUT);
            TZFree(item->ack);
            TZListRemove(gList, node);
        } else {
            // ͬ������.ͬ����ʽ��Э�������ڴ�
            item->isRxAck = true;
            *item->result = DCOM_SYSTEM_ERROR_CODE_RX_TIMEOUT;
        }
        return;
    }

    // �Ѿ����յ�,������ʱ�����ط�
    if (item->isRxAck) {
        return;
    }

    // �鴫�䲻�ô˴��ش�.�鴫��ģ���Լ�����
    if (item->reqLen > DCOM_PAYLOAD_SIZE_MAX) {
        return;
    }

    if (now - item->lastRetryTimestamp < (uint64_t)DComParamGetBlockRetryInterval() * 1000) {
        return;
    }

    // �ش�
    item->retryNum++;
    if (item->retryNum >= DComParamGetBlockRetryMaxNum()) {
        DComLogWarn("retry too many!task failed!token:%d", item->token);

        if (item->ackCallback) {
            // �ص���ʽ
            item->ackCallback(NULL, 0, DCOM_SYSTEM_ERROR_CODE_RX_TIMEOUT);
            TZFree(item->ack);
            TZListRemove(gList, node);
        } else {
            // ͬ������.ͬ����ʽ��Э�������ڴ�
            item->isRxAck = true;
            *item->result = DCOM_SYSTEM_ERROR_CODE_RX_TIMEOUT;
        }
        return;
    }

    item->lastRetryTimestamp = now;
    DComLogWarn("retry send.token:%d retry num:%d", item->token, item->retryNum);
    int ret = sendFrame(item->protocol, item->pipe, item->dstIA, item->code, item->rid, item->token, item->req,
        item->reqLen);
    if (ret != DCOM_OK) {
         DComLogWarn("retry send failed!error:%d", ret);
    }
}

// DComCallCreateHandle ����ͬ�����þ��
// protocol��Э���
// pipe��ͨ�Źܵ�
// timeout�ǳ�ʱʱ��,��λ:ms
// respΪNULL,respLenΪNULL,timeoutΪ0,��һ����������ͱ�ʾ����ҪӦ��
// ���������ú������DComRpcCallCoroutine����RPCͨ��,���ý��result�д洢���Ǵ�����.��DCOM_OK��ʾ����ʧ��
// ���óɹ���,Ӧ�����ݱ�����resp��,ע��Ҫ�ͷ�
// ���ؾ��.��0��ʾ�����ɹ�
intptr_t DComCallCreateHandle(int protocol, uint64_t pipe, uint64_t dstIA, int rid, int timeout, uint8_t* req, int reqLen,
    uint8_t** resp, int* respLen, int* result) {
    if (result == NULL) {
        return 0;
    }

    DComLogInfo("call.protocol:%d pipe:0x%x dst ia:0x%x rid:%d timeout:%d", protocol, pipe, dstIA, rid, timeout);

    TZListNode* node = createNode();
    if (node == NULL) {
        return 0;
    }

    tItem* item = (tItem*)node->Data;

    item->code = DCOM_CODE_CON;
    if (resp == NULL || respLen == NULL || timeout == 0) {
        item->code = DCOM_CODE_NON;
        item->isRxAck = true;
    } else {
        *resp = NULL;
        *respLen = 0;
    }

    item->ackCallback = NULL;
    item->protocol = protocol;
    item->pipe = pipe;
    PT_INIT(&item->pt);
    item->timeoutUs = (uint64_t)timeout * 1000;
    item->req = req;
    item->reqLen = reqLen;
    item->resp = resp;
    item->respLen = respLen;
    item->timeStart = TZTimeGet();

    item->dstIA = dstIA;
    item->rid = rid;
    item->token = DComGetToken();

    item->result = result;

    item->retryNum = 0;
    item->lastRetryTimestamp = TZTimeGet();
    TZListAppend(gList, node);

    (void)sendFrame(protocol, pipe, dstIA, item->code, rid, item->token, req, reqLen);
    return (intptr_t)node;
}

static TZListNode* createNode(void) {
    if (gList == 0) {
        gList = TZListCreateList(DComMid);
        if (gList == 0) {
            return NULL;
        }
    }

    TZListNode* node = TZListCreateNode(gList);
    if (node == NULL) {
        return NULL;
    }
    node->Data = TZMalloc(DComMid, sizeof(tItem));
    if (node->Data == NULL) {
        TZFree(node);
        return NULL;
    }
    return node;
}

// DComCall ͨ��Э�̵ķ�ʽ����DCOM��RPCͬ������
// ע�Ȿ������PTЭ�̷�ʽ���õ�,��Ҫʹ��PT_WAIT_THREAD�ȴ��������ý���
// ����ֵ��PT�к�
int DComCall(intptr_t handle) {
    if (handle == 0) {
        return 0;
    }

    TZListNode* node = (TZListNode*)handle;
    tItem* item = (tItem*)node->Data;

    PT_BEGIN(&item->pt);

    PT_WAIT_UNTIL(&item->pt, item->isRxAck);
    DComLogInfo("call resp.result:0x%x len:%d", *item->result, item->ackLen);
    if (*item->result == DCOM_OK) {
        *item->resp = item->ack;
        *item->respLen = item->ackLen;
    } else {
        TZFree(item->ack);
    }
    TZListRemove(gList, node);

    PT_END(&item->pt);
}

static int sendFrame(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token, uint8_t* data, int dataLen) {
    if (dataLen > DCOM_PAYLOAD_SIZE_MAX) {
        DComBlockTx(protocol, pipe, dstIA, code, rid, token, data, dataLen);
        return DCOM_OK;
    }

    DComFrame* frame = (DComFrame*)TZMalloc(DComMid, (int)sizeof(DComFrame) + dataLen);
    if (frame == NULL) {
        return DCOM_SYSTEM_ERROR_NOT_ENOUGH_MEMORY;
    }

    frame->ControlWord.bit.Code = (uint32_t)code;
    frame->ControlWord.bit.BlockFlag = 0;
    frame->ControlWord.bit.Rid = (uint32_t)rid;
    frame->ControlWord.bit.Token = (uint32_t)token;
    frame->ControlWord.bit.PayloadLen = (uint32_t)dataLen;
    if (dataLen > 0) {
        memcpy(frame->Payload, data, (size_t)dataLen);
    }
    DComLogInfo("send frame.token:%d", token);
    DComSend(protocol, pipe, dstIA, frame);
    TZFree(frame);
    return DCOM_OK;
}

// DComCallAsync RPC�첽����
// protocol��Э���
// pipe��ͨ�Źܵ�
// ackCallback��Ӧ��ص�
// timeoutΪ0,ackCallbackΪNULL,��һ����������ͱ�ʾ����ҪӦ��
// ���ص��ý��.��DCOM_OK��ʾ����ʧ��
int DComCallAsync(int protocol, uint64_t pipe, uint64_t dstIA, int rid, int timeout, uint8_t* req, int reqLen,
    DComAckCallback ackCallback) {
    int code = DCOM_CODE_CON;
    if (timeout == 0 || ackCallback == NULL) {
        code = DCOM_CODE_NON;
    }

    int token = DComGetToken();
    DComLogInfo("call async.token:%d protocol:%d pipe:0x%x dst ia:0x%x rid:%d timeout:%d", token, protocol, pipe, dstIA,
        rid, timeout);
    int ret = sendFrame(protocol, pipe, dstIA, code, rid, token, req, reqLen);
    if (ret != DCOM_OK) {
        return ret;
    }

    if (code == DCOM_CODE_NON) {
        return DCOM_OK;
    }

    TZListNode* node = createNode();
    if (node == NULL) {
        return DCOM_SYSTEM_ERROR_NOT_ENOUGH_MEMORY;
    }

    tItem* item = (tItem*)node->Data;
    item->ackCallback = ackCallback;
    item->protocol = protocol;
    item->pipe = pipe;
    item->timeoutUs = (uint64_t)timeout * 1000;
    item->req = req;
    item->reqLen = reqLen;
    item->timeStart = TZTimeGet();

    item->dstIA = dstIA;
    item->rid = rid;
    item->token = token;
    item->code = code;

    item->retryNum = 0;
    item->lastRetryTimestamp = TZTimeGet();
    TZListAppend(gList, node);

    return DCOM_OK;
}

// DComRxAckFrame ���յ�ACK֡ʱ������
// payloadLen���غɳ���,�˲����ɼ��ݿ鴫��ĳ�֡
void DComRxAckFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen) {
    if (gList == 0 || srcIA == 0 || frame == NULL) {
        return;
    }

    DComLogInfo("rx ack frame.src ia:0x%x", srcIA);

    TZListNode* node = TZListGetHeader(gList);
    TZListNode* nodeNext = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }
        nodeNext = node->Next;
        if (checkNodeAndDealAckFrame(protocol, pipe, srcIA, frame, payloadLen, node)) {
            break;
        }
        node = nodeNext;
    }
}

// checkTimeoutAndRetrySendFirstFrame ���ڵ��Ƿ��������,��������ACK֡
// ����true��ʾ�ڵ��������
static bool checkNodeAndDealAckFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen, TZListNode* node) {
    tItem* item = (tItem*)node->Data;
    if (item->protocol != protocol || item->pipe != pipe || item->dstIA != srcIA ||
            item->rid != frame->ControlWord.bit.Rid || item->token != frame->ControlWord.bit.Token) {
        return false;
    }

    DComLogInfo("deal ack frame.token:%d", item->token);
    if (item->ackCallback) {
        // �ص���ʽ
        if (payloadLen == 0) {
            item->ackCallback(NULL, 0, DCOM_OK);
            TZListRemove(gList, node);
            return true;
        }

        item->ack = TZMalloc(DComMid, payloadLen);
        if (item->ack == NULL) {
            item->ackCallback(NULL, 0, DCOM_SYSTEM_ERROR_NOT_ENOUGH_MEMORY);
            TZListRemove(gList, node);
            return true;
        }
        memcpy(item->ack, frame->Payload, (size_t)payloadLen);
        item->ackLen = payloadLen;

        item->ackCallback(item->ack, item->ackLen, DCOM_OK);
        TZFree(item->ack);
        TZListRemove(gList, node);
    } else {
        // ͬ������.ͬ����ʽ��Э�������ڴ�
        if (item->isRxAck) {
            // �ظ�����,��Ϣ��δ����
            return true;
        }
        item->isRxAck = true;
        *item->result = DCOM_OK;
        if (payloadLen == 0) {
            return true;
        }

        item->ack = TZMalloc(DComMid, payloadLen);
        if (item->ack == NULL) {
            *item->result = DCOM_SYSTEM_ERROR_NOT_ENOUGH_MEMORY;
            return true;
        }
        memcpy(item->ack, frame->Payload, (size_t)payloadLen);
        item->ackLen = payloadLen;
    }

    return true;
}

// DComRxRstFrame ���յ�RST֡ʱ������
void DComRxRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame) {
    if (gList == 0 || srcIA == 0 || frame == NULL) {
        return;
    }

    DComLogWarn("rx rst frame.src ia:0x%x", srcIA);

    TZListNode* node = TZListGetHeader(gList);
    TZListNode* nodeNext = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }
        nodeNext = node->Next;
        if (dealRstFrame(protocol, pipe, srcIA, frame, node)) {
            break;
        }
        node = nodeNext;
    }
}

// dealRstFrame ����λ����֡
// ����true��ʾ�ڵ��������
static bool dealRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, TZListNode* node) {
    tItem* item = (tItem*)node->Data;
    if (item->protocol != protocol || item->pipe != pipe || item->dstIA != srcIA ||
        item->rid != frame->ControlWord.bit.Rid || item->token != frame->ControlWord.bit.Token) {
        return false;
    }
    int result = frame->Payload[0];
    DComLogWarn("deal rst frame.token:%d result:0x%x", item->token, result);

    if (item->ackCallback) {
        // �ص���ʽ
        item->ackCallback(NULL, 0, result);
        TZListRemove(gList, node);
    } else {
        // ͬ������
        item->isRxAck = true;
        *item->result = result;
    }
    return true;
}
