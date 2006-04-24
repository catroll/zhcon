// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          nativeinputserver.cpp  -  description
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdexcept>
#include <cassert>
#include <algorithm>

#include "global.h"
#include "candilist.h"
#include "winime.h"
#include "nativeinputserver.h"
#include "keymap.h"

string NativeInputServer::mDataPath = PREFIX"/lib/zhcon/";

NativeInputServer::Symbol NativeInputServer::mFullSymbolTable[] = {
    {'.', "¡£"}
    ,
    {',', "£¬"}
    ,
    {';', "£»"}
    ,
    {':', "£º"}
    ,
    {'?', "£¿"}
    ,
    {'\"', "¡°¡±"}
    ,
    {'\'', "¡®¡¯"}
    ,
    {'(', "£¨"}
    ,
    {')', "£©"}
    ,
    {'<', "¡¶"}
    ,
    {'>', "¡·"}
    ,
    {'^', "¡­¡­"}
    ,
    {'_', "¡ª¡ª"}
    ,
    {'\\', "¡¢"}
    ,
    {'@', "¡¤"}
    ,
    {'&', "¡ª"}
    ,
    {'$', "£¤"}
};

char NativeInputServer::mFullCharTable[] =
    "¡¡£¡¡±££¡ç£¥£¦¡¯£¨£©£ª£«£¬£­£®£¯£°£±£²£³£´£µ£¶£·£¸£¹£º£»£¼£½£¾£¿"
    "£À£Á£Â£Ã£Ä£Å£Æ£Ç£È£É£Ê£Ë£Ì£Í£Î£Ï£Ð£Ñ£Ò£Ó£Ô£Õ£Ö£×£Ø£Ù£Ú¡²£Ü¡³£Þ¡õ"
    "¡®£á£â£ã£ä£å£æ£ç£è£é£ê£ë£ì£í£î£ï£ð£ñ£ò£ó£ô£õ£ö£÷£ø£ù£ú£û£ü£ý¡«  ";

bool NativeInputServer::SetDataPath(string datapath) {
    mDataPath = datapath;
    return true;
}

NativeInputServer::NativeInputServer()
: mpIme(NULL),
mWordOffset(0),
mAutoSelectUnique(true),  //always false,may delete this member some day
mFirstComma(true),
mFirstQuote(true) {
    mList.Reset();
}
NativeInputServer::~NativeInputServer() {
    delete mpIme;
}

void NativeInputServer::GetCandilist(Candilist &rList) {
    rList = mList;
}

void NativeInputServer::GetInputBuf(char * pBuf, int len) {
    assert(len);
    int n = min((size_t)(len - 1), mInput.size());
    mInput.copy(pBuf, n);
    pBuf[n] = '\0';
}

//note:mList.mHaveNext is updated in ime->Search();
//while mList.mHavePrev is updated in this routine
bool NativeInputServer::ProcessKey(char c, string & rBuf) {
    int count;
    string s;
    rBuf = "";
    assert(mpIme);
    bool flag = true;
    //wIldchar should not occur in 1st char so check mCount
    if (mpIme->InCodeSet(c)
            || (!mInput.empty() && mpIme->IsWildChar(c))) {
        flag = false;
        while (!mStack.empty())
            mStack.pop();
        mWordOffset = 0;
        if (mList.mCount == 1 && mList.mList[0].mKey == '\0')
            s = mList.mList[0].mText; 	  //save exact match for autoselect function
        mInput += c;
        count = mpIme->Search(mInput, 0);
        if (count > 0) {
            if (count == 1 && mList.mList[0].mKey == '\0' && mAutoSelectUnique) {
                //auto select unique word
                rBuf = Select(0);
                mpIme->Reset();
                mInput = "";
                mList.Reset();
            }
        }
        else {
            if(c >= '0' && c <= '9')  // for big5-phone
                flag = true;
            if (!s.empty()) {
                rBuf = s;
                mpIme->Reset();
                mInput = c;
                mList.Reset();
                if (mpIme->IsWildChar(c))
                    count = 0;
                else
                    count = mpIme->Search(mInput, 0);
                if (count > 0) {
                    //&&auto
                    if (count == 1 && mList.mList[0].mKey == '\0' && mAutoSelectUnique) {
                        rBuf = Select(0);
                        mpIme->Reset();
                        mInput = "";
                        mList.Reset();
                    }
                } else {
                    mInput = "";
                    Beep();
                }
            }
            else {
                mInput.erase(mInput.end() - 1);
                Beep();
            }
        }
    }
    if(flag == true) {
        if (mList.mCount > 0) {
            switch (c) {
                case 033:
                    mpIme->Reset();
                    mInput = "";
                    mList.Reset();
                    break;
                case ' ':
                case 015: 	  //enter
                    s = Select(0);
                    rBuf = s;
                    mpIme->Reset();
                    mInput = "";
                    mList.Reset();
                    break;
                case '0':   //fall through
                    c = '1' + 9;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    s = Select(c - '1');
                    if (!s.empty()) {
                        rBuf = s;
                        mpIme->Reset();
                        mInput = "";
                        mList.Reset();
                    }
                    else
                        Beep();
                    break;
                case ALT_0:   //fall through
                    c += 10;
                case ALT_1:
                case ALT_2:
                case ALT_3:
                case ALT_4:
                case ALT_5:
                case ALT_6:
                case ALT_7:
                case ALT_8:
                case ALT_9:
                   //unfinished
                    assert(!"ALT_? key unsupport in this server!\n\
                           remove them from keymap.");
                    c -= (ALT_0 + 1);
                    s = Select(c);
                    if (!s.empty()) {
                        rBuf = s;
                        mpIme->Reset();
                        mInput = "";
                        mList.Reset();
                    } else
                        Beep();
                    break;
                case '+':
                case '=':
                    if (mList.mHaveNext) {
                        int n = mList.mCount;
                        mpIme->Search(mInput, mWordOffset + n);
                        mStack.push(n);
                        mList.mHavePrev = true;
                        mWordOffset += n;
                    }
                    else
                        Beep();
                    break;
                case '-':
                case '_':
                    if (!mStack.empty()) {
                        assert(mList.mHavePrev);
                        mpIme->Search(mInput, mWordOffset -=
                                          mStack.top());
                        mStack.pop();
                    }
                    else
                        Beep();
                    break;
                case 010: 		/* BackSpace Ctrl+H */
                case 0177: 		/* BackSpace */
                    assert(!mInput.empty());
                    mInput.erase(mInput.end() - 1);
                    mList.Reset();
                    mpIme->Search(mInput, 0);
                    break;
                default:
                    if (mIsFullComma && IsSymbol(c)) {
                        s = Select(0);
                        s += GetFullSymbol(c);
                        rBuf = s;
                        mpIme->Reset();
                        mInput = "";
                        mList.Reset();
                    }
                    else
                        OutChar(c, rBuf);
            }//switch
        }
        else
            OutChar(c, rBuf);  //other char or 1st char is wildchar so out it
    }
    return true;
}

void NativeInputServer::SetFullChar(bool value) {
    mIsFullChar = value;
}

void NativeInputServer::SetFullComma(bool value) {
    mIsFullComma = value;
}

bool NativeInputServer::LoadImm(ImmInfo & rModule) {
    if (mpIme) {
        delete mpIme;
        mpIme = NULL;
    }

    try {
        string s;
        s = mDataPath + rModule.mTable;
        mpIme = new WinIme(s.c_str());
        //reset
        mList.Reset();
        mInput = "";
        while (!mStack.empty())
            mStack.pop();
        mpIme->SetCandilist(&mList,mClientBufLen);
        mImmInfo = rModule;
        return true;
    } catch (runtime_error & e) {
        mpIme = NULL;
        return false;
    }
}

string NativeInputServer::GetFullSymbol(char c) {
    Symbol *p;
    string s;
    for (p = mFullSymbolTable; p != mFullSymbolTable +
            sizeof(mFullSymbolTable) / sizeof(Symbol); p++)
        if (p->mKey == c) {
            switch (c) {
                case '\"':
                    if (mFirstComma) {
                        s += p->mpSymbol[0];
                        s += p->mpSymbol[1];
                    } else {
                        s += p->mpSymbol[2];
                        s += p->mpSymbol[3];
                    }
                    mFirstComma = !mFirstComma;
                    return s;
                case '\'':
                    if (mFirstQuote) {
                        s += p->mpSymbol[0];
                        s += p->mpSymbol[1];
                    } else {
                        s += p->mpSymbol[2];
                        s += p->mpSymbol[3];
                    }
                    mFirstQuote = !mFirstQuote;
                    return s;
                default:
                    return p->mpSymbol;
            }
        }
    assert(!"could not reach here!");
    return s;
}

//return selected hzcode
string NativeInputServer::Select(int n) {
    if (n >= 0 && n < mList.mCount)
        return mList.mList[n].mText;
    else
        return "";
}

bool NativeInputServer::IsSymbol(char c) {
    Symbol *p;
    for (p = mFullSymbolTable; p != mFullSymbolTable +
            sizeof(mFullSymbolTable) / sizeof(Symbol); p++)
        if (p->mKey == c)
            break;
    return p != mFullSymbolTable + sizeof(mFullSymbolTable) / sizeof(Symbol);
}

//string NativeInputServer::GetFullChar(char c){
//    int n;
//    switch (mImmInfo.mEncode)
//    {
//        case GB2312:
//        case GBK:
//            n = ASCII_CONVERTOR_GB;
//            break;
//        case BIG5:
//            n = ASCII_CONVERTOR_BIG5;
//            break;
//        default:
//            assert(!"Could not reach here in non Chinese env!");
//            return string();
//    }
//    char *s = pCAsciiConvertor[n]->szFullCharKeyStroke (c);
//    assert(s);
//    return string(s);
//}

//string NativeInputServer::GetSymbolFilter(char c){
//    int n;
//    switch (mImmInfo.mEncode)
//    {
//        case GB2312:
//        case GBK:
//            n = ASCII_CONVERTOR_GB;
//            break;
//        case BIG5:
//            n = ASCII_CONVERTOR_BIG5;
//            break;
//        default:
//            assert(!"Could not reach here in non Chinese env!");
//            return string();
//    }
//    char *s = pCAsciiConvertor[n]->szFullSymbolKeyStroke (c);
//    assert(s);
//    return string(s);
//}

//process normal char then copy it to buf
void NativeInputServer::OutChar(char c, string& buf) {
    if (mIsFullChar && c >= ' ' && c <= 126) {
        int key = (c - ' ') << 1;
        buf = mFullCharTable[key];
        buf += mFullCharTable[key + 1];
    } else if (mIsFullComma && IsSymbol(c)) {
        buf = GetFullSymbol(c);
    } else
        buf = c;
}

string NativeInputServer::GetServerType() {
    return string("native");
}
void NativeInputServer::SetClientBufLen(int len){
    mClientBufLen = len;
    if (mpIme)
        mpIme->SetCandilist(&mList,len);
}
