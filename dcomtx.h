// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// ����ģ��ͷ�ļ�
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMTX_H
#define DCOMTX_H

#include "dcomprotocol.h"
#include "dcom.h"

// DComSetSendFunction ���÷��ͺ���
void DComSetSendFunction(DComIsAllowSendFunc isAllowSend, DComSendFunc send);

// DComSend ��������
void DComSend(int protocol, uint64_t pipe, uint64_t dstIA, DComFrame* frame);

// DComBlockSend �鴫�䷢������
void DComBlockSend(int protocol, uint64_t pipe, uint64_t dstIA, DComBlockFrame* frame);

// DComIsAllowSend �Ƿ�������
bool DComIsAllowSend(uint64_t pipe);

// DComSendRstFrame ���ʹ�����
// controlWord ��ǰ�Ự������
// ����trueʱ���ͳɹ�
bool DComSendRstFrame(int protocol, uint64_t pipe, uint64_t dstIA, int errorCode, int rid, int token);

#endif
