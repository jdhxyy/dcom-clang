// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// ����ģ��ͷ�ļ�
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMRX_H
#define DCOMRX_H

#include <stdint.h>

// ����FIFO��С
#define DCOM_RX_FIFO_LEN 2

// DComRxLoad ģ������
void DComRxLoad(void);

// DComRxLoad ģ������
int DComRxRun(void);

#endif
