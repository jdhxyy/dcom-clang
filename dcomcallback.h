// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// �ص�ģ��ͷ�ļ�
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMCALLBACK_H
#define DCOMCALLBACK_H

#include <stdint.h>

// DComCallback �ص���Դ��rid��Ӧ�ĺ���
int DComCallback(int protocol, uint64_t pipe, uint64_t srcIA, int rid, uint8_t* req, int reqLen, uint8_t** resp, int* respLen);

#endif
