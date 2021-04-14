// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 接收模块主文件
// Authors: jdh99 <jdh821@163.com>

#include "dcomrx.h"
#include "dcomblockrx.h"
#include "dcomrxcon.h"
#include "dcomwaitlist.h"
#include "dcomblocktx.h"
#include "dcomcommon.h"
#include "dcomlog.h"

#include "tzlist.h"
#include "tzmalloc.h"
#include "pt.h"
#include "tztime.h"
#include "tzfifo.h"
#include "dcom.h"

#pragma pack(1)

// 接收结构头部
typedef struct {
    int protocol;
    uint64_t pipe;
    uint64_t srcIA;
    int size;
} tDcomRxHeader;

#pragma pack()

// 接收缓存
static intptr_t gFifo = 0;

// dealRecv 处理接收数据
// 注意:使用payloadLen来标识载荷长度而不是通过frame中的载荷长度字段
static void dealRecv(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen);

// DComRxLoad 模块载入
void DComRxLoad(void) {
    gFifo = TZFifoCreate(DComMid, DCOM_RX_FIFO_LEN, sizeof(tDcomRxHeader) + DCOM_FRAME_SIZE_MAX);
    DComBlockRxSetCallback(dealRecv);
}

// dealRecv 处理接收数据
// 注意:使用payloadLen来标识载荷长度而不是通过frame中的载荷长度字段
static void dealRecv(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen) {
    DComLogInfo("receive data.token:%d code:%d src ia:0x%x", frame->ControlWord.bit.Token, frame->ControlWord.bit.Code, 
        srcIA);
    if (frame->ControlWord.bit.Code == DCOM_CODE_CON || frame->ControlWord.bit.Code == DCOM_CODE_NON) {
        DComRxCon(protocol, pipe, srcIA, frame, payloadLen);
        return;
    }
    if (frame->ControlWord.bit.Code == DCOM_CODE_ACK) {
        DComRxAckFrame(protocol, pipe, srcIA, frame, payloadLen);
        return;
    }
    if (frame->ControlWord.bit.Code == DCOM_CODE_BACK) {
        DComBlockRxBackFrame(protocol, pipe, srcIA, frame);
        return;
    }
    if (frame->ControlWord.bit.Code == DCOM_CODE_RST) {
        if (payloadLen != 1 || frame->ControlWord.bit.PayloadLen != 1) {
            return;
        }
        DComRxRstFrame(protocol, pipe, srcIA, frame);
        DComBlockRxDealRstFrame(protocol, pipe, srcIA, frame);
        DComBlockTxDealRstFrame(protocol, pipe, srcIA, frame);
        return;
    }
}

// DComReceive 接收数据
// 应用模块接收到数据后需调用本函数
// 本函数接收帧的格式为DCOM协议数据
void DComReceive(int protocol, uint64_t pipe, uint64_t srcIA, uint8_t* bytes, int size) {
    static tDcomRxHeader rx;

    if (size < (int)sizeof(DComFrame) || size > DCOM_FRAME_SIZE_MAX) {
        DComLogWarn("receive data error:bytes to frame failed,size is wrong.src ia:0x%x size:%d", srcIA, size);
        return;
    }

    if (TZFifoWriteable(gFifo) == false) {
        DComLogWarn("rx fifo is full!.src ia:0x%x size:%d", srcIA, size);
        return;
    }

    rx.protocol = protocol;
    rx.pipe = pipe;
    rx.srcIA = srcIA;
    rx.size = size;
    TZFifoWriteMix(gFifo, (uint8_t*)&rx, sizeof(tDcomRxHeader), bytes, size);
}

// DComRxLoad 模块运行
int DComRxRun(void) {
    static struct pt pt;
    static tDcomRxHeader rx;
    static uint8_t bytes[DCOM_FRAME_SIZE_MAX] = {0};

    PT_BEGIN(&pt);
    
    PT_WAIT_UNTIL(&pt, TZFifoReadable(gFifo));
    if (TZFifoReadMix(gFifo, (uint8_t*)&rx, sizeof(tDcomRxHeader), bytes, DCOM_FRAME_SIZE_MAX) == false) {
        PT_EXIT(&pt);
    }

    if (rx.size < (int)sizeof(DComFrame)) {
        DComLogError("receive data error!:bytes to frame failed,size is short.src ia:0x%x size:%d", rx.srcIA, rx.size);
        PT_EXIT(&pt);
    }
    DComFrame* frame = (DComFrame*)bytes;
    frame->ControlWord.value = DComNtohl(frame->ControlWord.value);
    if (frame->ControlWord.bit.PayloadLen + sizeof(DComFrame) != (uint32_t)rx.size) {
        DComLogWarn("receive data error:bytes to frame failed", rx.srcIA);
        PT_EXIT(&pt);
    }
    if (frame->ControlWord.bit.BlockFlag == 0) {
        dealRecv(rx.protocol, rx.pipe, rx.srcIA, frame, frame->ControlWord.bit.PayloadLen);
    } else {
        DComBlockFrame* blockFrame = (DComBlockFrame*)frame;
        blockFrame->BlockHeader.Crc16 = DComNtohs(blockFrame->BlockHeader.Crc16);
        blockFrame->BlockHeader.Total = DComNtohs(blockFrame->BlockHeader.Total);
        blockFrame->BlockHeader.Offset = DComNtohs(blockFrame->BlockHeader.Offset);
        DComBlockRxReceive(rx.protocol, rx.pipe, rx.srcIA, blockFrame);
    }

    PT_END(&pt);
}
