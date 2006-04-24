// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          nativeinputserver.h  -  description
                             -------------------
    begin                : Mon Sep 10 2001
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

#ifndef NATIVEINPUTSERVER_H
#define NATIVEINPUTSERVER_H

//Native table based input server
//use Windows98 table file(converted by win2mb)
//and Ucdos *.dic file(converted by uc2win)

#include <inputserver.h>
#include <stack>
#include "candilist.h"
/**
  *@author ejoy
  */

class WinIme;

class NativeInputServer : public InputServer {
    public:
        NativeInputServer();
        ~NativeInputServer();
        void SetFullComma(bool value);
        void SetFullChar(bool value);
        bool ProcessKey(char key, string & rBuf);
        void GetInputBuf(char * pBuf, int len);
        void GetCandilist(Candilist & rList);
        bool LoadImm(ImmInfo & rModule);
        string GetServerType();
        void SetClientBufLen(int len);

        static bool SetDataPath(string datapath);

    private:
        static string mDataPath;
        string GetFullSymbol(char c);
        string Select(int n);
        bool IsSymbol(char c);
        void OutChar(char c, string& buf);

        WinIme *mpIme;
        Candilist mList;
        string mInput;
        stack < int > mStack;   //save last search position
        int mWordOffset;
        bool mAutoSelectUnique;
        static struct Symbol {
            char mKey;
            char *mpSymbol;
        }
        mFullSymbolTable[];
        static char mFullCharTable[];
        bool mFirstComma;
        bool mFirstQuote;
};

#endif
