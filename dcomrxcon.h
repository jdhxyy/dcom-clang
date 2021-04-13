// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 接收到连接处理头文件
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMRXCON_H
#define DCOMRXCON_H

#include "dcomprotocol.h"

// DComRxCon 接收到连接帧时处理函数
// payloadLen是载荷长度,超长后就需要启动块传输发送
void DComRxCon(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen);

#endif
