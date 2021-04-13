// Copyright 2021-2021 The jdh99 Authors. All rights reserved.
// 日志模块
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMLOG_H
#define DCOMLOG_H

#include <stdint.h>

// DComLogDebug 打印debug信息
void DComLogDebug(char *format, ...);

// DComLogInfo 打印info信息
void DComLogInfo(char *format, ...);

// DComLogWarn 打印warn信息
void DComLogWarn(char *format, ...);

// DComLogError 打印error信息
void DComLogError(char *format, ...);

#endif
