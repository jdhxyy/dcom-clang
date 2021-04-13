// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 块传输接收模块头文件
// Authors: jdh99 <jdh821@163.com>

#ifndef TZBLOCKRX_H
#define TZBLOCKRX_H

#include "dcomprotocol.h"

// DComBlockRecvFunc 块传输接收函数类型
// 注意:使用payloadLen来标识载荷长度而不是通过frame中的载荷长度字段
typedef void (*DComBlockRecvFunc)(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen);

// DComBlockRxLoad 模块载入
void DComBlockRxLoad(void);

// DComBlockRxRun 模块运行
// 返回值是PT行号,不用处理
int DComBlockRxRun(void);

// DComBlockRxSetCallback 设置接收回调函数
// 回调函数载荷长度在DComBlockHeader.total字段标识.DComControlWord.PayloadLen字段无效
void DComBlockRxSetCallback(DComBlockRecvFunc recvFunc);

// DComBlockRxReceive 块传输接收数据
void DComBlockRxReceive(int protocol, uint64_t pipe, uint64_t srcIA, DComBlockFrame* frame);

// DComBlockRxDealRstFrame 块传输接收模块处理复位连接帧
void DComBlockRxDealRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame);

#endif
