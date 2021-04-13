// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// dcom�ӿ�ͷ�ļ�
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMPORT_H
#define DCOMPORT_H

#include "lagan.h"

#include <stdint.h>
#include <stdbool.h>

// �汾
#define DCOM_VERSION_NAME "1.0"

// ϵͳ������
// ��ȷֵ
#define DCOM_OK 0
// ���ճ�ʱ
#define DCOM_SYSTEM_ERROR_CODE_RX_TIMEOUT 0x10
// ���ͳ�ʱ
#define DCOM_SYSTEM_ERROR_CODE_TX_TIMEOUT 0x11
// �ڴ治��
#define DCOM_SYSTEM_ERROR_NOT_ENOUGH_MEMORY 0x12
// û�ж�Ӧ����ԴID
#define DCOM_SYSTEM_ERROR_CODE_INVALID_RID 0x13
// �鴫��У�����
#define DCOM_SYSTEM_ERROR_CODE_WRONG_BLOCK_CHECK 0x14
// �鴫��ƫ�Ƶ�ַ����
#define DCM_SYSTEM_ERROR_CODE_WRONG_BLOCK_OFFSET 0x15
// ��������
#define DCOM_SYSTEM_ERROR_PARAM_INVALID 0x16

// ���м��.��λ:us
#define INTERVAL 1000

// DComCallbackFunc ע��DCOM����ص�����
// ��ѭ˭����˭�ͷ�ԭ��,resp��Ҫ�ɻص�����ʹ��TZMalloc���ٿռ�,DCom�����ͷſռ�
// ����ֵΪ0��ʾ�ص��ɹ�,�����Ǵ�����
typedef int (*DComCallbackFunc)(uint64_t pipe, uint64_t srcIA, uint8_t* req, int reqLen, uint8_t** resp, int* respLen);

// DComAckCallback DCOMӦ��ص�����
// Ӧ�����ݱ�����resp��,ʹ�������Ҫ�ͷ�
// error��Ӧ�������.��DCOM_OK��ʾӦ��ʧ��
typedef void (*DComAckCallback)(uint8_t* resp, int respLen, int error);

// DComIsAllowSendFunc �Ƿ������ͺ�������
typedef bool (*DComIsAllowSendFunc)(uint64_t pipe);

// DComSendFunc ��ָ���ܵ����ͺ�������
typedef void (*DComSendFunc)(int protocol, uint64_t pipe, uint64_t dstIA, uint8_t* bytes, int size);

// DComLoadParam �������
typedef struct {
    // tzmalloc�ڴ�����е��ڴ�id
    int Mid;
    // �鴫��֡���Լ��.��λ:ms
    int BlockRetryInterval;
    // �鴫��֡����������
    int BlockRetryMaxNum;

    // API�ӿ�
    // �Ƿ�������
    DComIsAllowSendFunc IsAllowSend;
    // ���͵���DCOMЭ������
    DComSendFunc Send;
} DComLoadParam;

// DComMid tzmalloc�û�id
extern int DComMid;

// DComLoad ģ������
void DComLoad(DComLoadParam param);

// DComRun ģ������
void DComRun(void);

// DComRpcCreateHandle ����RPC���þ��
// protocol��Э���
// pipe��ͨ�Źܵ�
// timeout�ǳ�ʱʱ��,��λ:ms
// respΪNULL,respLenΪNULL,timeoutΪ0,��һ����������ͱ�ʾ����ҪӦ��
// ���������ú������DComRpcCallCoroutine����RPCͨ��,���ý��result�д洢���Ǵ�����.��DCOM_OK��ʾ����ʧ��
// ���óɹ���,Ӧ�����ݱ�����resp��,ע��Ҫ�ͷ�
// ���ؾ��.��0��ʾ�����ɹ�
intptr_t DComRpcCreateHandle(int protocol, uint64_t pipe, uint64_t dstIA, int rid, int timeout, uint8_t* req, int reqLen, 
    uint8_t** resp, int* respLen, int* result);

// DComCall ͨ��Э�̵ķ�ʽ����DCOM��RPCͬ������
// ע�Ȿ������PTЭ�̷�ʽ���õ�,��Ҫʹ��PT_WAIT_THREAD�ȴ��������ý���
// ����ֵ��PT�к�
int DComCall(intptr_t handle);

// DComCallAsync RPC�첽����
// protocol��Э���
// pipe��ͨ�Źܵ�
// ackCallback��Ӧ��ص�
// timeoutΪ0,ackCallbackΪNULL,��һ����������ͱ�ʾ����ҪӦ��
// ���ص��ý��.��DCOM_OK��ʾ����ʧ��
int DComCallAsync(int protocol, uint64_t pipe, uint64_t dstIA, int rid, int timeout, uint8_t* req, int reqLen, 
    DComAckCallback ackCallback);

// DComRegister ע�����ص�����
// ���rid��Ӧ�Ļص��Ѿ�����,���ʹ���µ��滻�ɵ�
// ���ע��ʧ��,ԭ�����ڴ治��
bool DComRegister(int protocol, int rid, DComCallbackFunc callback);

// DComReceive ��������
// Ӧ��ģ����յ����ݺ�����ñ�����
// ����������֡�ĸ�ʽΪDCOMЭ������
void DComReceive(int protocol, uint64_t pipe, uint64_t srcIA, uint8_t* bytes, int size);

// DComLogSetFilterLevel ������־���˼���
void DComLogSetFilterLevel(LaganLevel level);

#endif
