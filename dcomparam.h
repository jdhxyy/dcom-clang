// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// ��������ģ��ͷ�ļ�
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMPARAM_H
#define DCOMPARAM_H

#include <stdint.h>

// DComParamSetBlockRetryInterval ���ÿ鴫��֡���Լ��.��λ:ms
void DComParamSetBlockRetryInterval(int interval);

// DComParamGetBAckRetryInterval ��ȡ�鴫��֡���Լ��.��λ:ms
int DComParamGetBlockRetryInterval(void);

// DComParamSetBlockRetryMaxNum ���ÿ鴫��֡����������
void DComParamSetBlockRetryMaxNum(int interval);

// DComParamGetBlockRetryMaxNum ��ȡ�鴫��֡����������
int DComParamGetBlockRetryMaxNum(void);

#endif
