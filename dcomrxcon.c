// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// ���յ����Ӵ������ļ�
// Authors: jdh99 <jdh821@163.com>

#include "dcomrxcon.h"
#include "dcomcallback.h"
#include "dcomtx.h"
#include "dcomblocktx.h"
#include "dcomlog.h"

#include "tzmalloc.h"

#include <string.h>

// DComRxCon ���յ�����֡ʱ������
// payloadLen���غɳ���,���������Ҫ�����鴫�䷢��
void DComRxCon(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen) {
    DComLogInfo("rx con.token:%d", frame->ControlWord.bit.Token);
    uint8_t* resp = NULL;
    int respLen = 0;
    int ret = DComCallback(protocol, pipe, srcIA, frame->ControlWord.bit.Rid, (uint8_t*)frame + sizeof(DComFrame), 
        payloadLen, &resp, &respLen);
    
    // NON����ҪӦ��
    if (frame->ControlWord.bit.Code == DCOM_CODE_NON) {
        TZFree(resp);
        return;
    }

    if (ret != DCOM_OK) {
        DComLogInfo("service send err:0x%x token:%d", ret, frame->ControlWord.bit.Token);
        (void)DComSendRstFrame(protocol, pipe, srcIA, ret, frame->ControlWord.bit.Rid, frame->ControlWord.bit.Token);
        TZFree(resp);
        return;
    }

    if (respLen > DCOM_SINGLE_FRAME_SIZE_MAX) {
        // ���ȹ����������鴫��
        DComLogInfo("service send too long:%d.start block tx.token:%d", respLen, frame->ControlWord.bit.Token);
        DComBlockTx(protocol, pipe, srcIA, DCOM_CODE_ACK, frame->ControlWord.bit.Rid, frame->ControlWord.bit.Token, 
            resp, respLen);
        TZFree(resp);
        return;
    }

    DComFrame* ackFrame = (DComFrame*)TZMalloc(DComMid, (int)sizeof(DComFrame) + respLen);
    if (ackFrame == NULL) {
        return;
    }
    ackFrame->ControlWord.bit.Code = DCOM_CODE_ACK;
    ackFrame->ControlWord.bit.BlockFlag = 0;
    ackFrame->ControlWord.bit.Rid = frame->ControlWord.bit.Rid;
    ackFrame->ControlWord.bit.Token = frame->ControlWord.bit.Token;
    ackFrame->ControlWord.bit.PayloadLen = (uint32_t)respLen;
    memcpy(ackFrame->Payload, resp, (size_t)respLen);
    TZFree(resp);
    (void)DComSend(protocol, pipe, srcIA, ackFrame);
    TZFree(ackFrame);
}
