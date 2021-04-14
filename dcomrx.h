// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 接收模块头文件
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMRX_H
#define DCOMRX_H

#include <stdint.h>

// 接收FIFO大小
#define DCOM_RX_FIFO_LEN 2

// DComRxLoad 模块载入
void DComRxLoad(void);

// DComRxLoad 模块运行
int DComRxRun(void);

#endif
