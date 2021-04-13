// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// ���յ����Ӵ���ͷ�ļ�
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMRXCON_H
#define DCOMRXCON_H

#include "dcomprotocol.h"

// DComRxCon ���յ�����֡ʱ������
// payloadLen���غɳ���,���������Ҫ�����鴫�䷢��
void DComRxCon(int protocol, uint64_t pipe, uint64_t srcIA, DComFrame* frame, int payloadLen);

#endif
