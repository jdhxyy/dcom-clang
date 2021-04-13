// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// ����ģ��ͷ�ļ�
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMCOMMON_H
#define DCOMCOMMON_H

#include <stdint.h>

// DComGetToken ��ȡtoken
// token��Χ:0-1023
int DComGetToken(void);

// DComHtons 2�ֽ�������ת��Ϊ������
uint16_t DComHtons(uint16_t n);

// DComNtohs 2�ֽ�������ת��Ϊ������
uint16_t DComNtohs(uint16_t n);

// DComHtonl 4�ֽ�������ת��Ϊ������
uint32_t DComHtonl(uint32_t n);

// DComNtohl 4�ֽ�������ת��Ϊ������
uint32_t DComNtohl(uint32_t n);

// DComHtonll 8�ֽ�������ת��Ϊ������
uint64_t DComHtonll(uint64_t n);

// UtzNtohll 8�ֽ�������ת��Ϊ������
uint64_t DComNtohll(uint64_t n);

#endif
