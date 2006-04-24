// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          winime.h  -  description
                             -------------------
    begin                : Wed Apr 4 2001
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

#ifndef WINIME_H
#define WINIME_H

/**
 *@author ejoy
 */
using namespace std;

#include <string>

class Candilist;
struct WinImeHead {
    char mName[12];
    int mMaxCodes;
    char mCodeSet[47];
    char mWildChar;
};

class WinIme {
    public:
        WinIme(const char* arg);
        ~WinIme();

        bool IsWildChar(char c) const {
            return c == mHead.mWildChar;
        }
        string GetName();
        bool InCodeSet(char c) const;
        int Search(string& s, int start);
        void SetCandilist(Candilist* p,unsigned len = 1000) {
            mpList = p;
            mCandilistBufLen = len;
        }
        void Reset();
        void SetGBKOut(bool v) {
            mGBKOut = v;
        }
        bool GetGBKOut() {
            return mGBKOut;
        }
        int mNum;        //number of chars processed in code queue
    private:

        bool IsHzCode1(char c) {
            return (c > 0x80 && c < 0xFF);
        }
        bool IsGB2312_1(char c) const {
            return c >= 0xA1 && c <= 0xF7;
        }
        bool IsGB2312_2(char c) const {
            return c >= 0xA1 && c <= 0xFE;
        }

        void AddCandilist(char*& p,unsigned& buflen);
        void SkipNext(char*& rp);
        int MatchWord(char* p, int len, int offset);
        bool IsGB2312(char* p);
        int Index(char c);
        int Search(int start);
        int Search(char c);

        char mInput[12 + 1];
        bool mGBKOut;
        Candilist* mpList;
        char* mpOffset[12];
        int mFd;
        char* mpBuf;
        char** mpIndex1;
        char** mpIndex2;
        char* mpText;
        char* mpCur;                  //current search position
        unsigned int mBufSize;
        unsigned int mCandilistBufLen;
        WinImeHead mHead;
};
#endif
