// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 发送模块头文件
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMTX_H
#define DCOMTX_H

#include "dcomprotocol.h"
#include "dcom.h"

// DComSetSendFunction 设置发送函数
void DComSetSendFunction(DComIsAllowSendFunc isAllowSend, DComSendFunc send);

// DComSend 发送数据
void DComSend(int protocol, uint64_t pipe, uint64_t dstIA, DComFrame* frame);

// DComBlockSend 块传输发送数据
void DComBlockSend(int protocol, uint64_t pipe, uint64_t dstIA, DComBlockFrame* frame);

// DComIsAllowSend 是否允许发送
bool DComIsAllowSend(uint64_t pipe);

// DComSendRstFrame 发送错误码
// controlWord 当前会话控制字
// 返回true时发送成功
bool DComSendRstFrame(int protocol, uint64_t pipe, uint64_t dstIA, int errorCode, int rid, int token);

#endif
