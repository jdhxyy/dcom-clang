// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 公共模块头文件
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMCOMMON_H
#define DCOMCOMMON_H

#include <stdint.h>

// DComGetToken 获取token
// token范围:0-1023
int DComGetToken(void);

// DComHtons 2字节主机序转换为网络序
uint16_t DComHtons(uint16_t n);

// DComNtohs 2字节网络序转换为主机序
uint16_t DComNtohs(uint16_t n);

// DComHtonl 4字节主机序转换为网络序
uint32_t DComHtonl(uint32_t n);

// DComNtohl 4字节网络序转换为主机序
uint32_t DComNtohl(uint32_t n);

// DComHtonll 8字节主机序转换为网络序
uint64_t DComHtonll(uint64_t n);

// UtzNtohll 8字节网络序转换为主机序
uint64_t DComNtohll(uint64_t n);

#endif
