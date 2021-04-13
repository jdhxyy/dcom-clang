// Copyright 2020-2021 The jdh99 Authors. All rights reserved.
// 回调模块主文件
// Authors: jdh99 <jdh821@163.com>

#include "dcomcallback.h"
#include "dcomlog.h"
#include "dcomprotocol.h"
#include "dcom.h"

#include "tzlist.h"
#include "tzmalloc.h"

#include <string.h>

#pragma pack(1)

typedef struct {
    int rid;
    DComCallbackFunc callback;
} tItem;

#pragma pack()

static intptr_t gList = 0;

static tItem* getItem(int rid);
static TZListNode* createNode(void);

// DComRegister 注册服务回调函数
// 如果rid对应的回调已经存在,则会使用新的替换旧的
// 如果注册失败,原因是内存不足
bool DComRegister(int protocol, int rid, DComCallbackFunc callback) {
    DComLogInfo("register.protocol:%d rid:%d", protocol, rid);
    rid += protocol << 16;

    if (callback == NULL) {
        return false;
    }
    if (gList == 0) {
        gList = TZListCreateList(DComMid);
        if (gList == 0) {
            return false;
        }
    }

    tItem* item = getItem(rid);
    if (item != NULL) {
        item->callback = callback;
        return true;
    }

    TZListNode* node = createNode();
    if (node == NULL) {
        return false;
    }
    item = (tItem*)node->Data;
    item->rid = rid;
    item->callback = callback;
    TZListAppend(gList, node);
    return true;
}

static tItem* getItem(int rid) {
    TZListNode* node = TZListGetHeader(gList);
    tItem* item = NULL;
    for (;;) {
        if (node == NULL) {
            break;
        }
        item = (tItem*)node->Data;
        if (item->rid == rid) {
            return item;
        }
        node = node->Next;
    }
    return NULL;
}

static TZListNode* createNode(void) {
    TZListNode* node = TZListCreateNode(gList);
    if (node == NULL) {
        return NULL;
    }
    node->Data = TZMalloc(DComMid, sizeof(tItem));
    if (node->Data == NULL) {
        TZFree(node);
        return NULL;
    }
    return node;
}

// DComCallback 回调资源号rid对应的函数
int DComCallback(int protocol, uint64_t pipe, uint64_t srcIA, int rid, uint8_t* req, int reqLen, uint8_t** resp, int* respLen) {
    if (gList == 0) {
        return DCOM_SYSTEM_ERROR_CODE_INVALID_RID;
    }

    DComLogInfo("service callback.rid:%d", rid);
    rid += protocol << 16;

    tItem* item = getItem(rid);
    if (item == NULL) {
        DComLogWarn("service callback failed!can not find new rid:%d", rid);
        return DCOM_SYSTEM_ERROR_CODE_INVALID_RID;
    }
    return item->callback(pipe, srcIA, req, reqLen, resp, respLen);
}
