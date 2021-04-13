// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 等待队列头文件
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMWAITLIST_H
#define DCOMWAITLIST_H

#include "dcomprotocol.h"

// DComWaitlistLoad 模块载入
void DComWaitlistLoad(void);

// DComWaitlistRun 模块运行
int DComWaitlistRun(void);

// DComRxAckFrame 接收到ACK帧时处理函数
// payloadLen是载荷长度,此参数可兼容块传输的长帧
void DComRxAckFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen);

// DComRxRstFrame 接收到RST帧时处理函数
void DComRxRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame);

#endif
