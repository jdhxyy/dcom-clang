// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 参数管理模块头文件
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMPARAM_H
#define DCOMPARAM_H

#include <stdint.h>

// DComParamSetBlockRetryInterval 设置块传输帧重试间隔.单位:ms
void DComParamSetBlockRetryInterval(int interval);

// DComParamGetBAckRetryInterval 读取块传输帧重试间隔.单位:ms
int DComParamGetBlockRetryInterval(void);

// DComParamSetBlockRetryMaxNum 设置块传输帧重试最大次数
void DComParamSetBlockRetryMaxNum(int interval);

// DComParamGetBlockRetryMaxNum 读取块传输帧重试最大次数
int DComParamGetBlockRetryMaxNum(void);

#endif
