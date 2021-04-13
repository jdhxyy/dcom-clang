// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// �鴫�䷢��ģ��ͷ�ļ�
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMBLOCKTX_H
#define DCOMBLOCKTX_H

#include "dcomprotocol.h"

// DComBlockTxLoad ģ������
void DComBlockTxLoad(void);

// DComBlockTxRun ģ������
// ����ֵ��PT�к�,���ô���
int DComBlockTxRun(void);

// DComBlockTx �鴫�䷢��
void DComBlockTx(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token, uint8_t* data, int dataLen);

// DComBlockRxBackFrame ���յ�BACK֡ʱ������
void DComBlockRxBackFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame);

// DComBlockTxDealRstFrame �鴫�䷢��ģ�鴦��λ����֡
void DComBlockTxDealRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame);

// DComBlockRemove �鴫�䷢���Ƴ�����
void DComBlockRemove(int protocol, uint64_t pipe, uint64_t dstIA, int code, int rid, int token);

#endif
