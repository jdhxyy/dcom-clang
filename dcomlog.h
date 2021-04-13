// Copyright 2021-2021 The jdh99 Authors. All rights reserved.
// ��־ģ��
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMLOG_H
#define DCOMLOG_H

#include <stdint.h>

// DComLogDebug ��ӡdebug��Ϣ
void DComLogDebug(char *format, ...);

// DComLogInfo ��ӡinfo��Ϣ
void DComLogInfo(char *format, ...);

// DComLogWarn ��ӡwarn��Ϣ
void DComLogWarn(char *format, ...);

// DComLogError ��ӡerror��Ϣ
void DComLogError(char *format, ...);

#endif
