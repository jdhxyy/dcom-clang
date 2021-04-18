// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 发送模块主文件
// Authors: jdh99 <jdh821@163.com>

#include "dcomtx.h"
#include "dcomlog.h"
#include "dcom.h"

#include "tzmalloc.h"
#include "tzbox.h"

#include <string.h>

static DComIsAllowSendFunc gIsAllowSend = NULL;
static DComSendFunc gSend = NULL;

// DComSetSendFunction 设置发送函数
void DComSetSendFunction(DComIsAllowSendFunc isAllowSend, DComSendFunc send) {
    gIsAllowSend = isAllowSend;
    gSend = send;
}

// DComSend 发送数据
void DComSend(int protocol, uint64_t pipe, uint64_t dstIA, DComFrame* frame) {
    if (frame == NULL || gSend == NULL || gIsAllowSend == NULL) {
        return;
    }
    if (gIsAllowSend(pipe) == false) {
        DComLogWarn("send failed!pipe:0x%x is not allow send.token:%d", pipe, frame->ControlWord.bit.Token);
        return;
    }

    DComLogInfo("send frame.token:%d protocol:%d pipe:0x%x dst ia:0x%x", frame->ControlWord.bit.Token, protocol, pipe,
        dstIA);

    int payloadLen = frame->ControlWord.bit.PayloadLen;
    frame->ControlWord.value = TZBoxHtonl(frame->ControlWord.value);
    gSend(protocol, pipe, dstIA, (uint8_t*)frame, (int)sizeof(DComFrame) + payloadLen);
    frame->ControlWord.value = TZBoxNtohl(frame->ControlWord.value);
}

// DComBlockSend 块传输发送数据
void DComBlockSend(int protocol, uint64_t pipe, uint64_t dstIA, DComBlockFrame* frame) {
    if (frame == NULL || gSend == NULL || gIsAllowSend == NULL) {
        return;
    }
    if (gIsAllowSend(pipe) == false) {
        DComLogWarn("block send failed!pipe:0x%x is not allow send.token:%d", pipe, frame->ControlWord.bit.Token);
        return;
    }

    DComLogInfo("block send frame.token:%d protocol:%d pipe:0x%x dst ia:0x%x offset:%d", frame->ControlWord.bit.Token,
        protocol, pipe, dstIA, frame->BlockHeader.Offset);

    int payloadLen = frame->ControlWord.bit.PayloadLen;
    frame->ControlWord.value = TZBoxHtonl(frame->ControlWord.value);
    frame->BlockHeader.Crc16 = TZBoxHtons(frame->BlockHeader.Crc16);
    frame->BlockHeader.Total = TZBoxHtons(frame->BlockHeader.Total);
    frame->BlockHeader.Offset = TZBoxHtons(frame->BlockHeader.Offset);
    
    gSend(protocol, pipe, dstIA, (uint8_t*)frame, (int)sizeof(DComControlWord) + payloadLen);
    frame->ControlWord.value = TZBoxNtohl(frame->ControlWord.value);
    frame->BlockHeader.Crc16 = TZBoxNtohs(frame->BlockHeader.Crc16);
    frame->BlockHeader.Total = TZBoxNtohs(frame->BlockHeader.Total);
    frame->BlockHeader.Offset = TZBoxNtohs(frame->BlockHeader.Offset);
}

// DComIsAllowSend 是否允许发送
bool DComIsAllowSend(uint64_t pipe) {
    if (gIsAllowSend == NULL) {
        return false;
    }
    return gIsAllowSend(pipe);
}

// DComSendRstFrame 发送错误码
// controlWord 当前会话控制字
// 返回true时发送成功
bool DComSendRstFrame(int protocol, uint64_t pipe, uint64_t dstIA, int errorCode, int rid, int token) {
    DComLogInfo("send rst frame:0x%x!token:%d protocol:%d pipe:0x%x dst ia:0x%x", errorCode, token, protocol, pipe,
		dstIA);
    DComFrame* frame = (DComFrame*)TZMalloc(DComMid, (int)sizeof(DComFrame) + 1);
    if (frame == NULL) {
        return false;
    }
    frame->ControlWord.bit.Code = DCOM_CODE_RST;
    frame->ControlWord.bit.BlockFlag = 0;
    frame->ControlWord.bit.Rid = (uint32_t)rid;
    frame->ControlWord.bit.Token = (uint32_t)token;
    frame->ControlWord.bit.PayloadLen = 1;
    frame->Payload[0] = (uint8_t)errorCode | 0x80;
    DComSend(protocol, pipe, dstIA, frame);
    TZFree(frame);
    return true;
}
