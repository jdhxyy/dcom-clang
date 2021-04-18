// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// ����ģ�����ļ�
// Authors: jdh99 <jdh821@163.com>

#include "dcomcommon.h"

static int gToken = 0;

// DComGetToken ��ȡtoken
// token��Χ:0-1023
int DComGetToken(void) {
    gToken++;
    if (gToken > 1023) {
        gToken = 0;
    }
    return gToken;
}
