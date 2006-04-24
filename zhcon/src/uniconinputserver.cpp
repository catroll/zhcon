// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          uniconinputserver.cpp  -  description
                             -------------------
    begin                : Mon Sep 17 2001
    copyright            : (C) 2001 by ejoy
    email                : ejoy@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_UNICON_LIB
#include <unistd.h>
#include <cassert>

#include "debug.h"
#include "candilist.h"
#include "uniconinputserver.h"

string UniconInputServer::mDataPath = "/usr/lib/unicon/";

bool UniconInputServer::SetDataPath(string datapath) {
    mDataPath = datapath;
    return true;
}

UniconInputServer::UniconInputServer()
:mImmServer(0),
mpImm(NULL) {
    LibOpen();
    mImmServer = IMM_OpenClient("127.0.0.1", 9010);
    if (!mImmServer)
        throw runtime_error("Unable to connect to Unicon Input Server");
}
UniconInputServer::~UniconInputServer() {
    if (mpImm)
		IMM_FlushUserPhrase(mpImm);
        IMM_CloseInput(mpImm);
    IMM_CloseClient(mImmServer);
    LibRelease();
}

void UniconInputServer::GetCandilist(Candilist & rList) {
    //from xl_hzinput.c it seems that return value of IMM_GetSelectDisplay
    //is the count of words found,but no comment found on this in ImmClient.h
    //maybe I'm wrong.
    rList.Reset();
    rList.mCount = IMM_GetSelectDisplay (mpImm, mSelectBuf, sizeof(mSelectBuf));
    if (rList.mCount == 0)
        return ;
    string s;
    char *p = mSelectBuf;
    while (*p != ' ')
        p++;
    if (*p == '<') {
        rList.mHavePrev = true;
        p++;
    }
    int i = 0;
    while (*p != '\0' && *p != '>') {
        if (*p == ' ')
            p++;
        else {
            p++;
            s = "";
            while (*p != ' ')
                s += *p++;
            assert(i < 10);
            rList.mList[i].mKey = '\0'; //unicon does not need key highlight
            rList.mList[i++].mText = s;
        }
    }
    if (*p == '>')
        rList.mHaveNext = true;
}

void UniconInputServer::GetInputBuf(char * pBuf, int len) {
    assert(len);
    IMM_GetInputDisplay (mpImm, pBuf, len);
}

string UniconInputServer::GetServerType() {
    return string("unicon");
}

bool UniconInputServer::LoadImm(ImmInfo & rModule) {
    if (mpImm) {
        IMM_CloseInput(mpImm);
        mpImm = NULL;
    }
//#warning hard coded path!

    u_long type;
    switch (rModule.mEncode) {
        case GB2312:
        case GBK:
            type = IMM_LC_GB2312;
            break;
        case BIG5:
            type = IMM_LC_BIG5;
            break;
        default:
            throw runtime_error("unsupport encode type found in Imm");
    }
    string module;
    module = mDataPath + rModule.mModule;
    string table;
    table = mDataPath + rModule.mTable;

    //unicon will fail if unable to open file
    //so check here to prevent from core dump
    if (access(module.c_str(), R_OK) || access(table.c_str(), R_OK))
        return false;
    mpImm = IMM_OpenInput (mImmServer, (char*)module.c_str(), (char*)table.c_str(), type);
    if (mpImm == NULL)
        return false;
    SetInputMode();
    assert(mClientBufLen);
    IMM_ConfigInputArea(mpImm, mClientBufLen);
    mImmInfo = rModule;
    return true;
}

bool UniconInputServer::ProcessKey(char key, string & rBuf) {
    char buf[64];
    int len = 63;
    buf[0] = '\0';
    int r = IMM_KeyFilter (mpImm,
                           key,       // 2 -- have filtered and translated
                           buf,        // 1 -- have filtered
                           &len);        // 0 -- not filtered
                                        // < 0 -- error code
    assert(r >= 0);
    switch (r) {
        case 2:
            rBuf = buf;
            break;
        case 1:
            rBuf = "";
            break;
        case 0:
            rBuf = key;
            break;
        default:
            assert(!"wrong return value got from unicon,it's not my fault!");
    }
    return true;
}

void UniconInputServer::SetFullChar(bool value) {
    mIsFullChar = value;
    SetInputMode();
}

void UniconInputServer::SetFullComma(bool value) {
    mIsFullComma = value;
    SetInputMode();
}

void UniconInputServer::SetInputMode() {
    long mode = IMM_DOUBLE_BYTE_MODE;
    if (mIsFullChar)
        mode |= IMM_FULL_CHAR_MODE;
    if (mIsFullComma)
        mode |= IMM_FULL_SYMBOL_MODE;
    IMM_SetInputMode(mpImm, mode);
}

void UniconInputServer::SetClientBufLen(int len){
    mClientBufLen = len;
    if (mpImm)
        IMM_ConfigInputArea(mpImm, mClientBufLen);
}
#endif //HAVE_UNICON_LIB
