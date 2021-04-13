// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 参数管理模块主文件
// Authors: jdh99 <jdh821@163.com>

#include "dcomparam.h"

static int gBlockRetryInterval = 0;
static int gBlockRetryMaxNum = 0;

// DComParamSetBlockRetryInterval 设置块传输帧重试间隔.单位:ms
void DComParamSetBlockRetryInterval(int interval) {
    gBlockRetryInterval = interval;
}

// DComParamGetBAckRetryInterval 读取块传输帧重试间隔.单位:ms
int DComParamGetBlockRetryInterval(void) {
    return gBlockRetryInterval;
}

// DComParamSetBlockRetryMaxNum 设置块传输帧重试最大次数
void DComParamSetBlockRetryMaxNum(int interval) {
    gBlockRetryMaxNum = interval;
}

// DComParamGetBlockRetryMaxNum 读取块传输帧重试最大次数
int DComParamGetBlockRetryMaxNum(void) {
    return gBlockRetryMaxNum;
}
