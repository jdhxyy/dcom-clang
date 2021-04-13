// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// dcom协议
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMPROTOCOL_H
#define DCOMPROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

// CODE码
#define DCOM_CODE_CON 0
#define DCOM_CODE_NON 1
#define DCOM_CODE_ACK 2
#define DCOM_CODE_RST 3
#define DCOM_CODE_BACK 4

// 单帧最大字节数.超过此字节数需要块传输
#define DCOM_SINGLE_FRAME_SIZE_MAX 255

#pragma pack(1)

// DComControlWord dcom控制字
typedef union {
    struct {
        uint32_t PayloadLen:8;
        uint32_t Token:10;
        uint32_t Rid:10;
        uint32_t BlockFlag:1;
        uint32_t Code:3;
    } bit;
    uint32_t value;
} DComControlWord;

// DComFrame dcom帧
typedef struct {
    DComControlWord ControlWord;
    uint8_t Payload[];
} DComFrame;

// BlockHeader 块传输头部
typedef struct {
    uint16_t Crc16;
    uint16_t Total;
    uint16_t Offset;
} DComBlockHeader;

// DComBlockFrame 块传输帧.重定义了dcom帧的载荷
typedef struct {
    DComControlWord ControlWord;
    DComBlockHeader BlockHeader;
    uint8_t Payload[];
} DComBlockFrame;

#pragma pack()

#endif
