// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 块传输发送模块头文件
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMBLOCKTX_H
#define DCOMBLOCKTX_H

#include "dcomprotocol.h"

// DComBlockTxLoad 模块载入
void DComBlockTxLoad(void);

// DComBlockTxRun 模块运行
// 返回值是PT行号,不用处理
int DComBlockTxRun(void);

// DComBlockTx 块传输发送
void DComBlockTx(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token, uint8_t* data, int dataLen);

// DComBlockRxBackFrame 接收到BACK帧时处理函数
void DComBlockRxBackFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame);

// DComBlockTxDealRstFrame 块传输发送模块处理复位连接帧
void DComBlockTxDealRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame);

// DComBlockRemove 块传输发送移除任务
void DComBlockRemove(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token);

#endif
