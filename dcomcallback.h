// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 回调模块头文件
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMCALLBACK_H
#define DCOMCALLBACK_H

#include <stdint.h>

// DComCallback 回调资源号rid对应的函数
int DComCallback(int protocol, uint64_t pipe, uint64_t srcIA, int rid, uint8_t* req, int reqLen, uint8_t** resp, int* respLen);

#endif
