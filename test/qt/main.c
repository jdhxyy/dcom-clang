#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dcom.h"
#include "tzmalloc.h"
#include "tztime.h"
#include "pt.h"
#include "lagan.h"

#define RAM_INTERNAL 0

#pragma pack(1)

typedef struct {
    uint8_t a;
    uint8_t b;
    uint8_t c;
} tRid1;

typedef struct {
    uint8_t d;
    uint8_t e;
} tRid1Ack;

#pragma pack()

static int gMid = -1;

static void print(uint8_t* bytes, int size);
static LaganTime getLaganTime(void);

static uint64_t getTime(void);
static bool isAllowSend(uint64_t pipe);
static void pipeSend(int protocol, uint64_t pipe, uint64_t dstIA, uint8_t* bytes, int size);

static void case0(void);

static void case1(void);
static int callback1(uint64_t pipe, uint64_t srcIA, uint8_t* req, int reqLen, uint8_t** resp, int* respLen);
static int callback2(uint64_t pipe, uint64_t srcIA, uint8_t* req, int reqLen, uint8_t** resp, int* respLen);
static int callback3(uint64_t pipe, uint64_t srcIA, uint8_t* req, int reqLen, uint8_t** resp, int* respLen);

static int case2(void);

static void case3(void);
static void dealAck(uint8_t* resp, int respLen, int result);

static int case4(void);

static int case5(void);

int main() {
    LaganLoad(print, getLaganTime);

    TZTimeLoad(getTime);
    TZMallocLoad(RAM_INTERNAL, 20, 100 * 1024, malloc(100 * 1024));

    gMid = TZMallocRegister(RAM_INTERNAL, "dcom", 4096);

    DComLoadParam param;
    param.Mid = gMid;
    param.BlockRetryInterval = 100;
    param.BlockRetryMaxNum = 5;
    param.IsAllowSend = isAllowSend;
    param.Send = pipeSend;
    DComLoad(param);

    DComLogSetFilterLevel(LAGAN_LEVEL_DEBUG);

    struct pt pt;

    case0();

    case1();
    //case2();
    //case3();
    case4();
    //case5();

    for (int i = 0; i < 1000; i++) {
        DComRun();
    }
    case4();
    //case5();
    return 0;
}

static void print(uint8_t* bytes, int size) {
    printf("%s", bytes);
}

static LaganTime getLaganTime(void) {
    static int now = 0;

    LaganTime time;
    time.Year = 0;
    time.Month = 0;
    time.Day = 0;

    int temp = now;
    int hour = temp / 3600;
    temp = temp % 3600;
    int minute = temp / 60;
    int second = temp % 60;
    now++;

    time.Hour = hour;
    time.Minute = minute;
    time.Second = second;
    time.Us = 0;
    return time;
}

static uint64_t getTime(void) {
    static uint64_t time = 0;
    time += 100;
    return time;
}

static bool isAllowSend(uint64_t pipe) {
    return true;
}

static void pipeSend(int protocol, uint64_t pipe, uint64_t dstIA, uint8_t* bytes, int size) {
    printf("pipeSend:0x%x %p %x\n", (uint32_t)dstIA, (void*)bytes, size);
    for (int i = 0; i < size; i++) {
        printf("%02x ", bytes[i]);
    }
    printf("\n");

    DComReceive(protocol, pipe, dstIA, bytes, size);
}

static void case0(void) {
    uint8_t ip[4] = {0x12, 0x34, 0x56, 0x78};
    uint16_t port = 0x2345;
    uint64_t pipe = DComAddrToPipe(ip, port);
    printf("pipe=0x%llx\n", pipe);

    DComPipeToAddr(pipe, ip, &port);
    printf("ip=0x%x 0x%x 0x%x 0x%x,port=0x%x\n", ip[0], ip[1], ip[2], ip[3], port);
}

static void case1(void) {
    DComRegister(0, 1, callback1);
    DComRegister(0, 2, callback2);
    DComRegister(0, 3, callback3);
}

static int callback1(uint64_t pipe, uint64_t srcIA, uint8_t* req, int reqLen, uint8_t** resp, int* respLen) {
    printf("callback1 start.pipe:%lld srcIA:0x%llx\n", pipe, srcIA);
    tRid1* rid = (tRid1*)req;
    printf("recv:%d %d %d\n", rid->a, rid->b, rid->c);

    tRid1Ack* ack = (tRid1Ack*)TZMalloc(gMid, sizeof(tRid1Ack));
    ack->d = 5;
    ack->e = 6;

    *resp = (uint8_t*)ack;
    *respLen = sizeof(tRid1Ack);

    return DCOM_OK;
}

static int callback2(uint64_t pipe, uint64_t srcIA, uint8_t* req, int reqLen, uint8_t** resp, int* respLen) {
    printf("callback2 start.pipe:%lld srcIA:0x%llx\n", pipe, srcIA);
    printf("recv:len=%d %d %d\n", reqLen, req[0], req[1]);

    tRid1Ack* ack = (tRid1Ack*)TZMalloc(gMid, sizeof(tRid1Ack));
    ack->d = 5;
    ack->e = 6;

    *resp = (uint8_t*)ack;
    *respLen = sizeof(tRid1Ack);

    return DCOM_OK;
}

static int callback3(uint64_t pipe, uint64_t srcIA, uint8_t* req, int reqLen, uint8_t** resp, int* respLen) {
    printf("callback3 start.pipe:%lld srcIA:0x%llx\n", pipe, srcIA);
    tRid1* rid = (tRid1*)req;
    printf("recv:len=%d %d %d %d\n", reqLen, rid->a, rid->b, rid->c);

    uint8_t* arr = TZMalloc(gMid, 256);
    for (int i = 0; i < 256; i++) {
        arr[i] = i + 10;
    }

    *resp = arr;
    *respLen = 256;

    return DCOM_OK;
}
static int case2(void) {
    static struct pt pt;
    static uint8_t* resp = NULL;
    static int respLen = 0;
    static int result = 0;

    PT_BEGIN(&pt);

    tRid1 rid1;
    rid1.a = 1;
    rid1.b = 2;
    rid1.c = 3;

    intptr_t handle = DComCallCreateHandle(0, 0, 1, 1, 5000, (uint8_t*)&rid1, sizeof(tRid1), &resp, &respLen, &result);
    PT_WAIT_THREAD(&pt, DComCall(handle));

    printf("result:0x%x\n", result);
    if (result == DCOM_OK && resp != NULL) {
        tRid1Ack* ack = (tRid1Ack*)resp;
        printf("%d %d\n", ack->d, ack->e);
        TZFree(resp);
    }

    printf("case2 end!");

    PT_END(&pt);
}

static void case3(void) {
    printf("case3 start\n");

    tRid1 rid1;
    rid1.a = 1;
    rid1.b = 2;
    rid1.c = 3;
    uint8_t* resp = NULL;
    int respLen = 0;
    int result = 0;
    int ret = DComCallAsync(0, 0, 1, 1, 5000, (uint8_t*)&rid1, sizeof(tRid1), dealAck);
    printf("call result:%d\n", ret);
    TZFree(resp);
}

static void dealAck(uint8_t* resp, int respLen, int result) {
    printf("dealAck start:result:0x%x %d\n", result, respLen);
    if (result != DCOM_OK) {
        return;
    }
    tRid1Ack* ack = (tRid1Ack*)resp;
    printf("%d %d\n", ack->d, ack->e);
}

static int case4(void) {
    static struct pt pt;
    static uint8_t arr[256] = {0};
    static uint8_t* resp = NULL;
    static int respLen = 0;
    static int result = 0;

    PT_BEGIN(&pt);

    for (int i = 0; i < 256; i++) {
        arr[i] = i + 1;
    }

    intptr_t handle = DComCallCreateHandle(0, 0, 1, 2, 5000, arr, 256, &resp, &respLen, &result);
    PT_WAIT_THREAD(&pt, DComCall(handle));

    if (resp != NULL) {
        tRid1Ack* ack = (tRid1Ack*)resp;
        printf("%d %d\n", ack->d, ack->e);
        TZFree(resp);
    }

    printf("case2 end!");

    PT_END(&pt);
}

static int case5(void) {
    static struct pt pt;

    PT_BEGIN(&pt);

    tRid1 rid1;
    rid1.a = 1;
    rid1.b = 2;
    rid1.c = 3;
    uint8_t* resp = NULL;
    int respLen = 0;
    int result = 0;
    intptr_t handle = DComCallCreateHandle(0, 0, 1, 3, 5000, (uint8_t*)&rid1, sizeof(tRid1), &resp, &respLen, &result);
    PT_WAIT_THREAD(&pt, DComCall(handle));

//    tRid1Ack* ack = (tRid1Ack*)resp;
//    if (ack != NULL) {
//        printf("%d %d\n", ack->d, ack->e);
//    } else {
//        printf("ack is null\n");
//    }
    printf("resp:0x%x respLen:%d\n", resp, respLen);

    printf("case5 end!");
    TZFree(resp);

    PT_END(&pt);
}
