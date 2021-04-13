// Copyright 2021-2021 The jdh99 Authors. All rights reserved.
// 日志模块
// Authors: jdh99 <jdh821@163.com>

#include "lagan.h"
#include "dcom.h"

#include <stdio.h>
#include <stdarg.h>

#define TAG "dcom"

static LaganLevel filterLevel = LAGAN_LEVEL_WARN;

// DComLogSetFilterLevel 设置日志过滤级别
void DComLogSetFilterLevel(LaganLevel level) {
    filterLevel = level;
}

// DComLogDebug 打印debug信息
void DComLogDebug(char *format, ...) {
    if (filterLevel == LAGAN_LEVEL_OFF || LAGAN_LEVEL_DEBUG < filterLevel) {
        return;
    }

    va_list args;
	va_start(args, format);

    char buf[LAGAN_RECORD_MAX_SIZE_DEFAULT] = {0};
    int len = vsnprintf(buf, LAGAN_RECORD_MAX_SIZE_DEFAULT - 1, format, args);
    if (len > LAGAN_RECORD_MAX_SIZE_DEFAULT || len < 0) {
        len = LAGAN_RECORD_MAX_SIZE_DEFAULT;
    }
    LaganPrint(TAG, LAGAN_LEVEL_DEBUG, "%s", buf);

    va_end(args);
}

// DComLogInfo 打印info信息
void DComLogInfo(char *format, ...) {
    if (filterLevel == LAGAN_LEVEL_OFF || LAGAN_LEVEL_INFO < filterLevel) {
        return;
    }

    va_list args;
	va_start(args, format);

    char buf[LAGAN_RECORD_MAX_SIZE_DEFAULT] = {0};
    int len = vsnprintf(buf, LAGAN_RECORD_MAX_SIZE_DEFAULT - 1, format, args);
    if (len > LAGAN_RECORD_MAX_SIZE_DEFAULT || len < 0) {
        len = LAGAN_RECORD_MAX_SIZE_DEFAULT;
    }
    LaganPrint(TAG, LAGAN_LEVEL_INFO, "%s", buf);

    va_end(args);
}

// DComLogWarn 打印warn信息
void DComLogWarn(char *format, ...) {
    if (filterLevel == LAGAN_LEVEL_OFF || LAGAN_LEVEL_WARN < filterLevel) {
        return;
    }

    va_list args;
	va_start(args, format);

    char buf[LAGAN_RECORD_MAX_SIZE_DEFAULT] = {0};
    int len = vsnprintf(buf, LAGAN_RECORD_MAX_SIZE_DEFAULT - 1, format, args);
    if (len > LAGAN_RECORD_MAX_SIZE_DEFAULT || len < 0) {
        len = LAGAN_RECORD_MAX_SIZE_DEFAULT;
    }
    LaganPrint(TAG, LAGAN_LEVEL_WARN, "%s", buf);

    va_end(args);
}

// DComLogError 打印error信息
void DComLogError(char *format, ...) {
    if (filterLevel == LAGAN_LEVEL_OFF || LAGAN_LEVEL_ERROR < filterLevel) {
        return;
    }

    va_list args;
	va_start(args, format);

    char buf[LAGAN_RECORD_MAX_SIZE_DEFAULT] = {0};
    int len = vsnprintf(buf, LAGAN_RECORD_MAX_SIZE_DEFAULT - 1, format, args);
    if (len > LAGAN_RECORD_MAX_SIZE_DEFAULT || len < 0) {
        len = LAGAN_RECORD_MAX_SIZE_DEFAULT;
    }
    LaganPrint(TAG, LAGAN_LEVEL_ERROR, "%s", buf);

    va_end(args);
}
