// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          inputserver.h  -  description
                             -------------------
    begin                : Sun Sep 9 2001
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

#ifndef INPUTSERVER_H
#define INPUTSERVER_H


/**
  *@author ejoy
  */
using namespace std;

#include <string>
#include "global.h"

struct ImmInfo {
public:
    bool operator==(const ImmInfo& r) {
        return (r.mName == mName && r.mModule == mModule
                && r.mTable == mTable && r.mType == mType);
    }
    string mName;
    string mModule;
    string mTable;
    Encode mEncode;
    string mType;
};

struct Candilist;

class InputServer {
    public:
        InputServer();
        virtual ~InputServer();
        bool IsFullComma();
        bool IsFullChar();
        virtual void SetFullComma(bool value) = 0;
        virtual void SetFullChar(bool value) = 0;
        virtual bool LoadImm(ImmInfo& rModule) = 0;
        ImmInfo GetImmInfo();
        virtual bool ProcessKey(char key, string& rBuf) = 0;
        virtual void GetCandilist(Candilist& rList) = 0;
        virtual void GetInputBuf(char* pBuf, int len) = 0;
        virtual string GetServerType() = 0;
        void SetClientBufLen(int len);
    protected:
        ImmInfo mImmInfo;
        bool mIsFullChar;
        bool mIsFullComma;
        int mClientBufLen;//max chars a client can handle
};

#endif
