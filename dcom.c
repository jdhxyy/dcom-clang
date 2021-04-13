// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// dcom接口文件
// Authors: jdh99 <jdh821@163.com>

#include "dcom.h"
#include "dcomparam.h"
#include "dcomtx.h"
#include "dcomblockrx.h"
#include "dcomblocktx.h"
#include "dcomrx.h"
#include "dcomwaitlist.h"

// DComMid tzmalloc用户id
int DComMid = 0; 

// DComLoad 模块载入
void DComLoad(DComLoadParam param) {
    DComMid = param.Mid;
    DComParamSetBlockRetryInterval(param.BlockRetryInterval);
    DComParamSetBlockRetryMaxNum(param.BlockRetryMaxNum);
    DComSetSendFunction(param.IsAllowSend, param.Send);

    DComBlockRxLoad();
    DComBlockTxLoad();
    DComRxLoad();
    DComWaitlistLoad();
}

// DComRun 模块运行
void DComRun(void) {
    DComBlockRxRun();
    DComBlockTxRun();
    DComWaitlistRun();
}
