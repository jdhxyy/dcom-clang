// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// ����ģ�����ļ�
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

// dealRecv �����������
// ע��:ʹ��payloadLen����ʶ�غɳ��ȶ�����ͨ��frame�е��غɳ����ֶ�
static void dealRecv(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen);

// DComRxLoad ģ������
void DComRxLoad(void) {
    DComBlockRxSetCallback(dealRecv);
}

// dealRecv �����������
// ע��:ʹ��payloadLen����ʶ�غɳ��ȶ�����ͨ��frame�е��غɳ����ֶ�
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

// DComReceive ��������
// Ӧ��ģ����յ����ݺ�����ñ�����
// ����������֡�ĸ�ʽΪDCOMЭ������
void DComReceive(int protocol, uint64_t pipe, uint64_t srcIA, uint8_t* bytes, int size) {
    if (size < (int)sizeof(DComFrame)) {
        DComLogWarn("receive data error:bytes to frame failed,size is short.src ia:0x%x size:%d", srcIA, size);
        return;
    }
    DComFrame* frame = (DComFrame*)bytes;
    frame->ControlWord.value = DComNtohl(frame->ControlWord.value);
    if (frame->ControlWord.bit.PayloadLen + sizeof(DComFrame) != size) {
        DComLogWarn("receive data error:bytes to frame failed", srcIA);
        return;
    }
    if (frame->ControlWord.bit.BlockFlag == 0) {
        dealRecv(protocol, pipe, srcIA, frame, frame->ControlWord.bit.PayloadLen);
    } else {
        DComBlockFrame* blockFrame = (DComBlockFrame*)frame;
        blockFrame->BlockHeader.Crc16 = DComNtohs(blockFrame->BlockHeader.Crc16);
        blockFrame->BlockHeader.Total = DComNtohs(blockFrame->BlockHeader.Total);
        blockFrame->BlockHeader.Offset = DComNtohs(blockFrame->BlockHeader.Offset);
        DComBlockRxReceive(protocol, pipe, srcIA, blockFrame);
    }
}
