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

// DComHtons 2�ֽ�������ת��Ϊ������
uint16_t DComHtons(uint16_t n) {
    return (uint16_t)(((n & 0xff) << 8) | ((n & 0xff00) >> 8));
}

// DComNtohs 2�ֽ�������ת��Ϊ������
uint16_t DComNtohs(uint16_t n) {
    return DComHtons(n);
}

// DComHtonl 4�ֽ�������ת��Ϊ������
uint32_t DComHtonl(uint32_t n) {
    return ((n & 0xff) << 24) |
        ((n & 0xff00) << 8) |
        ((n & 0xff0000UL) >> 8) |
        ((n & 0xff000000UL) >> 24);
}

// DComNtohl 4�ֽ�������ת��Ϊ������
uint32_t DComNtohl(uint32_t n) {
    return DComHtonl(n);
}

// DComHtonll 8�ֽ�������ת��Ϊ������
uint64_t DComHtonll(uint64_t n) {
    return ((n & 0xff) << 56) |
        ((n & 0xff00) << 40) |
        ((n & 0xff0000) << 24) |
        ((n & 0xff000000) << 8) |
        ((n & 0xff00000000ULL) >> 8) |
        ((n & 0xff0000000000ULL) >> 24) |
        ((n & 0xff000000000000ULL) >> 40) |
        ((n & 0xff00000000000000ULL) >> 56);
}

// UtzNtohll 8�ֽ�������ת��Ϊ������
uint64_t DComNtohll(uint64_t n) {
    return DComHtonll(n);
}
