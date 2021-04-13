// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// dcom接口头文件
// Authors: jdh99 <jdh821@163.com>

#ifndef DCOMPORT_H
#define DCOMPORT_H

#include "lagan.h"

#include <stdint.h>
#include <stdbool.h>

// 版本
#define DCOM_VERSION_NAME "1.0"

// 系统错误码
// 正确值
#define DCOM_OK 0
// 接收超时
#define DCOM_SYSTEM_ERROR_CODE_RX_TIMEOUT 0x10
// 发送超时
#define DCOM_SYSTEM_ERROR_CODE_TX_TIMEOUT 0x11
// 内存不足
#define DCOM_SYSTEM_ERROR_NOT_ENOUGH_MEMORY 0x12
// 没有对应的资源ID
#define DCOM_SYSTEM_ERROR_CODE_INVALID_RID 0x13
// 块传输校验错误
#define DCOM_SYSTEM_ERROR_CODE_WRONG_BLOCK_CHECK 0x14
// 块传输偏移地址错误
#define DCM_SYSTEM_ERROR_CODE_WRONG_BLOCK_OFFSET 0x15
// 参数错误
#define DCOM_SYSTEM_ERROR_PARAM_INVALID 0x16

// 运行间隔.单位:us
#define INTERVAL 1000

// DComCallbackFunc 注册DCOM服务回调函数
// 遵循谁调用谁释放原则,resp需要由回调函数使用TZMalloc开辟空间,DCom负责释放空间
// 返回值为0表示回调成功,否则是错误码
typedef int (*DComCallbackFunc)(uint64_t pipe, uint64_t srcIA, uint8_t* req, int reqLen, uint8_t** resp, int* respLen);

// DComAckCallback DCOM应答回调函数
// 应答数据保存在resp中,使用完毕需要释放
// error是应答错误码.非DCOM_OK表示应答失败
typedef void (*DComAckCallback)(uint8_t* resp, int respLen, int error);

// DComIsAllowSendFunc 是否允许发送函数类型
typedef bool (*DComIsAllowSendFunc)(uint64_t pipe);

// DComSendFunc 向指定管道发送函数类型
typedef void (*DComSendFunc)(int protocol, uint64_t pipe, uint64_t dstIA, uint8_t* bytes, int size);

// DComLoadParam 载入参数
typedef struct {
    // tzmalloc内存管理中的内存id
    int Mid;
    // 块传输帧重试间隔.单位:ms
    int BlockRetryInterval;
    // 块传输帧重试最大次数
    int BlockRetryMaxNum;

    // API接口
    // 是否允许发送
    DComIsAllowSendFunc IsAllowSend;
    // 发送的是DCOM协议数据
    DComSendFunc Send;
} DComLoadParam;

// DComMid tzmalloc用户id
extern int DComMid;

// DComLoad 模块载入
void DComLoad(DComLoadParam param);

// DComRun 模块运行
void DComRun(void);

// DComRpcCreateHandle 创建RPC调用句柄
// protocol是协议号
// pipe是通信管道
// timeout是超时时间,单位:ms
// resp为NULL,respLen为NULL,timeout为0,有一个条件满足就表示不需要应答
// 本函数调用后需调用DComRpcCallCoroutine进行RPC通信,调用结果result中存储的是错误码.非DCOM_OK表示调用失败
// 调用成功后,应答数据保存在resp中,注意要释放
// 返回句柄.非0表示创建成功
intptr_t DComRpcCreateHandle(int protocol, uint64_t pipe, uint64_t dstIA, int rid, int timeout, uint8_t* req, int reqLen, 
    uint8_t** resp, int* respLen, int* result);

// DComCall 通过协程的方式进行DCOM的RPC同步调用
// 注意本函数是PT协程方式调用的,需要使用PT_WAIT_THREAD等待函数调用结束
// 返回值是PT行号
int DComCall(intptr_t handle);

// DComCallAsync RPC异步调用
// protocol是协议号
// pipe是通信管道
// ackCallback是应答回调
// timeout为0,ackCallback为NULL,有一个条件满足就表示不需要应答
// 返回调用结果.非DCOM_OK表示调用失败
int DComCallAsync(int protocol, uint64_t pipe, uint64_t dstIA, int rid, int timeout, uint8_t* req, int reqLen, 
    DComAckCallback ackCallback);

// DComRegister 注册服务回调函数
// 如果rid对应的回调已经存在,则会使用新的替换旧的
// 如果注册失败,原因是内存不足
bool DComRegister(int protocol, int rid, DComCallbackFunc callback);

// DComReceive 接收数据
// 应用模块接收到数据后需调用本函数
// 本函数接收帧的格式为DCOM协议数据
void DComReceive(int protocol, uint64_t pipe, uint64_t srcIA, uint8_t* bytes, int size);

// DComLogSetFilterLevel 设置日志过滤级别
void DComLogSetFilterLevel(LaganLevel level);

#endif
