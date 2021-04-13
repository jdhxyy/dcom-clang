// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// �鴫�����ģ��ͷ�ļ�
// Authors: jdh99 <jdh821@163.com>

#ifndef TZBLOCKRX_H
#define TZBLOCKRX_H

#include "dcomprotocol.h"

// DComBlockRecvFunc �鴫����պ�������
// ע��:ʹ��payloadLen����ʶ�غɳ��ȶ�����ͨ��frame�е��غɳ����ֶ�
typedef void (*DComBlockRecvFunc)(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen);

// DComBlockRxLoad ģ������
void DComBlockRxLoad(void);

// DComBlockRxRun ģ������
// ����ֵ��PT�к�,���ô���
int DComBlockRxRun(void);

// DComBlockRxSetCallback ���ý��ջص�����
// �ص������غɳ�����DComBlockHeader.total�ֶα�ʶ.DComControlWord.PayloadLen�ֶ���Ч
void DComBlockRxSetCallback(DComBlockRecvFunc recvFunc);

// DComBlockRxReceive �鴫���������
void DComBlockRxReceive(int protocol, uint64_t pipe, uint64_t srcIA, DComBlockFrame* frame);

// DComBlockRxDealRstFrame �鴫�����ģ�鴦��λ����֡
void DComBlockRxDealRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame);

#endif
