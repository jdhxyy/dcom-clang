// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// ��������ģ�����ļ�
// Authors: jdh99 <jdh821@163.com>

#include "dcomparam.h"

static int gBlockRetryInterval = 0;
static int gBlockRetryMaxNum = 0;

// DComParamSetBlockRetryInterval ���ÿ鴫��֡���Լ��.��λ:ms
void DComParamSetBlockRetryInterval(int interval) {
    gBlockRetryInterval = interval;
}

// DComParamGetBAckRetryInterval ��ȡ�鴫��֡���Լ��.��λ:ms
int DComParamGetBlockRetryInterval(void) {
    return gBlockRetryInterval;
}

// DComParamSetBlockRetryMaxNum ���ÿ鴫��֡����������
void DComParamSetBlockRetryMaxNum(int interval) {
    gBlockRetryMaxNum = interval;
}

// DComParamGetBlockRetryMaxNum ��ȡ�鴫��֡����������
int DComParamGetBlockRetryMaxNum(void) {
    return gBlockRetryMaxNum;
}
