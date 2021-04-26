// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// ����ģ�����ļ�
// Authors: jdh99 <jdh821@163.com>

#include "dcomcommon.h"
#include "dcom.h"

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

// AddrToPipe �����ַת��Ϊ�ܵ���
// ת������Ϊ����˿�+ip��ַ.�������
uint64_t DComAddrToPipe(uint8_t* ip, uint16_t port) {
    uint64_t pipe = 0;
    pipe = (uint64_t)((ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3]);
    pipe |= ((((uint64_t)port >> 8) & 0xff) << 40) + (((uint64_t)port & 0xff) << 32);
    return pipe;
}

// PipeToAddr �ܵ���ת��Ϊ�����ַ
// ת������Ϊ����˿�+ip��ַ.�������
void DComPipeToAddr(uint64_t pipe, uint8_t* ip, uint16_t* port) {
    ip[0] = (uint8_t)(pipe >> 24);
    ip[1] = (uint8_t)(pipe >> 16);
    ip[2] = (uint8_t)(pipe >> 8);
    ip[3] = (uint8_t)pipe;
    *port = (uint16_t)(pipe >> 32);
}
