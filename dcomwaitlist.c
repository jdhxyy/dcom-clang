// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 等待队列主文件
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
    // 上次发送时间.用于重传
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

// checkTimeoutAndRetrySendFirstFrame 检查节点是否符合条件,符合则处理ACK帧
// 返回true表示节点符合条件
static bool checkNodeAndDealAckFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen, TZListNode* node);

// dealRstFrame 处理复位连接帧
// 返回true表示节点符合条件
static bool dealRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, TZListNode* node);

// DComWaitlistLoad 模块载入
void DComWaitlistLoad(void) {
    gList = TZListCreateList(DComMid);
}

// DComWaitlistRun 模块运行
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
            // 回调方式
            item->ackCallback(NULL, 0, DCOM_SYSTEM_ERROR_CODE_RX_TIMEOUT);
            TZFree(item->ack);
            TZListRemove(gList, node);
        } else {
            // 同步调用.同步方式由协程清理内存
            item->isRxAck = true;
            *item->result = DCOM_SYSTEM_ERROR_CODE_RX_TIMEOUT;
        }
        return;
    }

    // 已经接收到,处理不及时则不用重发
    if (item->isRxAck) {
        return;
    }

    // 块传输不用此处重传.块传输模块自己负责
    if (item->reqLen > DCOM_PAYLOAD_SIZE_MAX) {
        return;
    }

    if (now - item->lastRetryTimestamp < (uint64_t)DComParamGetBlockRetryInterval() * 1000) {
        return;
    }

    // 重传
    item->retryNum++;
    if (item->retryNum >= DComParamGetBlockRetryMaxNum()) {
        DComLogWarn("retry too many!task failed!token:%d", item->token);

        if (item->ackCallback) {
            // 回调方式
            item->ackCallback(NULL, 0, DCOM_SYSTEM_ERROR_CODE_RX_TIMEOUT);
            TZFree(item->ack);
            TZListRemove(gList, node);
        } else {
            // 同步调用.同步方式由协程清理内存
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

// DComCallCreateHandle 创建同步调用句柄
// protocol是协议号
// pipe是通信管道
// timeout是超时时间,单位:ms
// resp为NULL,respLen为NULL,timeout为0,有一个条件满足就表示不需要应答
// 本函数调用后需调用DComRpcCallCoroutine进行RPC通信,调用结果result中存储的是错误码.非DCOM_OK表示调用失败
// 调用成功后,应答数据保存在resp中,注意要释放
// 返回句柄.非0表示创建成功
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

// DComCall 通过协程的方式进行DCOM的RPC同步调用
// 注意本函数是PT协程方式调用的,需要使用PT_WAIT_THREAD等待函数调用结束
// 返回值是PT行号
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

// DComCallAsync RPC异步调用
// protocol是协议号
// pipe是通信管道
// ackCallback是应答回调
// timeout为0,ackCallback为NULL,有一个条件满足就表示不需要应答
// 返回调用结果.非DCOM_OK表示调用失败
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

// DComRxAckFrame 接收到ACK帧时处理函数
// payloadLen是载荷长度,此参数可兼容块传输的长帧
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

// checkTimeoutAndRetrySendFirstFrame 检查节点是否符合条件,符合则处理ACK帧
// 返回true表示节点符合条件
static bool checkNodeAndDealAckFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen, TZListNode* node) {
    tItem* item = (tItem*)node->Data;
    if (item->protocol != protocol || item->pipe != pipe || item->dstIA != srcIA ||
            item->rid != frame->ControlWord.bit.Rid || item->token != frame->ControlWord.bit.Token) {
        return false;
    }

    DComLogInfo("deal ack frame.token:%d", item->token);
    if (item->ackCallback) {
        // 回调方式
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
        // 同步调用.同步方式由协程清理内存
        if (item->isRxAck) {
            // 重复接收,信息尚未处理
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

// DComRxRstFrame 接收到RST帧时处理函数
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

// dealRstFrame 处理复位连接帧
// 返回true表示节点符合条件
static bool dealRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, TZListNode* node) {
    tItem* item = (tItem*)node->Data;
    if (item->protocol != protocol || item->pipe != pipe || item->dstIA != srcIA ||
        item->rid != frame->ControlWord.bit.Rid || item->token != frame->ControlWord.bit.Token) {
        return false;
    }
    int result = frame->Payload[0];
    DComLogWarn("deal rst frame.token:%d result:0x%x", item->token, result);

    if (item->ackCallback) {
        // 回调方式
        item->ackCallback(NULL, 0, result);
        TZListRemove(gList, node);
    } else {
        // 同步调用
        item->isRxAck = true;
        *item->result = result;
    }
    return true;
}
