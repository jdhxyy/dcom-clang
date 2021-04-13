// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// �ȴ�����ͷ�ļ�
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMWAITLIST_H
#define DCOMWAITLIST_H

#include "dcomprotocol.h"

// DComWaitlistLoad ģ������
void DComWaitlistLoad(void);

// DComWaitlistRun ģ������
int DComWaitlistRun(void);

// DComRxAckFrame ���յ�ACK֡ʱ������
// payloadLen���غɳ���,�˲����ɼ��ݿ鴫��ĳ�֡
void DComRxAckFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen);

// DComRxRstFrame ���յ�RST֡ʱ������
void DComRxRstFrame(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame);

#endif
