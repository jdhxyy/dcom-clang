// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 公共模块主文件
// Authors: jdh99 <jdh821@163.com>

#include "dcomcommon.h"

static int gToken = 0;

// DComGetToken 获取token
// token范围:0-1023
int DComGetToken(void) {
    gToken++;
    if (gToken > 1023) {
        gToken = 0;
    }
    return gToken;
}
