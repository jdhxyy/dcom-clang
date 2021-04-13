// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// dcom�ӿ��ļ�
// Authors: jdh99 <jdh821@163.com>

#include "dcom.h"
#include "dcomparam.h"
#include "dcomtx.h"
#include "dcomblockrx.h"
#include "dcomblocktx.h"
#include "dcomrx.h"
#include "dcomwaitlist.h"

// DComMid tzmalloc�û�id
int DComMid = 0; 

// DComLoad ģ������
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

// DComRun ģ������
void DComRun(void) {
    DComBlockRxRun();
    DComBlockTxRun();
    DComWaitlistRun();
}
