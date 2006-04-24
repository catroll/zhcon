// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          configserver.cpp  -  description
                             -------------------
    begin                : Fri Sep 28 2001
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

#include <cassert>
#include "global.h"
#include "zhcon.h"
#include "candilist.h"
#include "inputmanager.h"
#include "configserver.h"

ConfigServer::MenuItem ConfigServer::mSysMenu[] = {
    {gettext_noop("input option"), NULL, &ConfigServer::MenuHandleIme},
    {gettext_noop("Select Encode"), &ConfigServer::GetTextEncode, &ConfigServer::MenuHandleEncode},
    {gettext_noop("Encode-Detect Method"), NULL, &ConfigServer::MenuHandleDetectEncode},
    {gettext_noop("Quit"), NULL, &ConfigServer::MenuQuit},
    {NULL, NULL, NULL}
};
ConfigServer::MenuItem ConfigServer::mDetectEncodeMenu[] = {
    {gettext_noop("Current Encode-Detect Method:"), &ConfigServer::GetTextDetectEncode, NULL},
    {"Auto-GB", NULL, &ConfigServer::MenuHandleSetAutoEncodeGB},
    {"Auto-BIG5", NULL, &ConfigServer::MenuHandleSetAutoEncodeBIG5},
    {"Auto", NULL, &ConfigServer::MenuHandleSetAutoEncodeAUTO},
    {"Manual", NULL, &ConfigServer::MenuHandleSetAutoEncodeMANUAL},
    {gettext_noop("Back"), NULL, &ConfigServer::MenuHandleGotoSysMenu},
    {NULL, NULL, NULL}
};
ConfigServer::MenuItem ConfigServer::mImeMenu[] = {
    {gettext_noop("Input Option:"), NULL, NULL},
    {gettext_noop("Auto Select Unique Word"), NULL,
     &ConfigServer::MenuHandleAutoSelectUnique},
    {gettext_noop("Input Encode"), NULL,
     &ConfigServer::MenuHandleInputEncode},
    {gettext_noop("Back"), NULL, &ConfigServer::MenuHandleGotoSysMenu},
    {NULL, NULL, NULL}
};
ConfigServer::MenuItem ConfigServer::mEncodeMenu[] = {
    {gettext_noop("Please Select Encode:"), NULL, NULL},
    {"GB2312", NULL, &ConfigServer::MenuHandleSetEncodeGB2312},
    {"GBK", NULL, &ConfigServer::MenuHandleSetEncodeGBK},
    {"BIG5", NULL, &ConfigServer::MenuHandleSetEncodeBIG5},
    {"JIS", NULL, &ConfigServer::MenuHandleSetEncodeJIS},
    {"KSC", NULL, &ConfigServer::MenuHandleSetEncodeKSC},
    {gettext_noop("Back"), NULL, &ConfigServer::MenuHandleGotoSysMenu},
    {NULL, NULL, NULL}
};

ConfigServer::ConfigServer() {
    mImmInfo.mName = _("System Menu");
    mpCurMenu = mSysMenu;
}
ConfigServer::~ConfigServer() {}

void ConfigServer::GetCandilist(Candilist & rList) {
    rList.Reset();
    int i = 0;
    for (MenuItem *p = mpCurMenu; p->mpText; p++) {
        rList.mList[i].mKey = '\0';
        rList.mList[i].mText = (p->mpGetText ? (this->*(p->mpGetText)) () : _(p->mpText));
        i++;
    }
    rList.mCount = i;
    assert(i <= 9);   //menu should has only one page,max 9 items
}

void ConfigServer::GetInputBuf(char * pBuf, int len) {
    pBuf = '\0';
}

string ConfigServer::GetServerType() {
    return "system";
}

bool ConfigServer::ProcessKey(char key, string & rBuf) {
    assert(mpCurMenu);
    rBuf = "";
    if (key == 033) { //Esc pressed
        return false;
    }

    int count = 0;
    MenuItem *p;
    for (p = mpCurMenu; p->mpText; p++)
        count++;
    assert(count);
    if (key < '1' || key >= '1' + count)
        return true;

    p = &mpCurMenu[key - '1'];
    if (p->mpFun == &ConfigServer::MenuQuit) {
        return false;
    }
    
    (this->*(p->mpFun)) ();
    return true;
}

bool ConfigServer::LoadImm(ImmInfo & rModule) {
    assert("Could not reach here!");
    return false;
}

void ConfigServer::MenuQuit() {
}

void ConfigServer::MenuHandleIme() {
    mpCurMenu = mImeMenu;
}

void ConfigServer::MenuHandleEncode() {
    mpCurMenu = mEncodeMenu;
}

string ConfigServer::GetTextEncode() {
    string s = _("Current Encode:");
    //s += '%';
    //s += (char) mColor1;
    switch (gpZhcon->mEncode) {
        case ASCII:
            s += "ASCII";
            break;
        case GB2312:
            s += "GB2312";
            break;
        case GBK:
            s += "GBK";
            break;
        case BIG5:
            s += "BIG5";
            break;
        case JIS:
            s += "JIS";
            break;
        case KSC:
            s += "KSC";
            break;
        default:
            assert(!"Error in encode!");
    }
    //s += '%';
    //s += (char) 255;
    return s;
}

void ConfigServer::MenuHandleSetEncodeGB2312() {
    gpZhcon->SetEncode(GB2312, GB2312);
}
void ConfigServer::MenuHandleSetEncodeGBK() {
    gpZhcon->SetEncode(GBK, GBK);
}
void ConfigServer::MenuHandleSetEncodeBIG5() {
    gpZhcon->SetEncode(BIG5, BIG5);
}
void ConfigServer::MenuHandleSetEncodeJIS() {
    gpZhcon->SetEncode(JIS, JIS);
}
void ConfigServer::MenuHandleSetEncodeKSC() {
    gpZhcon->SetEncode(KSC, KSC);
}

string ConfigServer::GetTextDetectEncode() {
    string s = _("Current Encode-Detect Method:");
    //    s += '%';
    //   s += (char) mColor1;
    switch (gpZhcon->mAutoEncode) {
        case Zhcon::AUTO:
            s += "Auto";
            break;
        case Zhcon::AUTO_GB:
            s += "Auto-GB";
            break;
        case Zhcon::AUTO_BIG5:
            s += "Auto-BIG5";
            break;
        case Zhcon::MANUAL:
            s += "Manual";
            break;
    }
    // s += '%';
    // s += (char) 255;
    //s += ']';
    return s;
}

void ConfigServer::MenuHandleSetAutoEncodeGB() {
    gpZhcon->mAutoEncode = Zhcon::AUTO_GB;
}
void ConfigServer::MenuHandleSetAutoEncodeBIG5() {
    gpZhcon->mAutoEncode = Zhcon::AUTO_BIG5;
}
void ConfigServer::MenuHandleSetAutoEncodeMANUAL() {
    gpZhcon->mAutoEncode = Zhcon::MANUAL;
}
void ConfigServer::MenuHandleSetAutoEncodeAUTO() {
    gpZhcon->mAutoEncode = Zhcon::AUTO;
}

void ConfigServer::MenuHandleDetectEncode() {
    mpCurMenu = mDetectEncodeMenu;
}

string ConfigServer::ConfigServer::GetTextInputEncode() {
    string s = _("Current Input Encode(native only)");
    //    s += '%';
    //s += (char) mColor1;
    /*    if (mpCurIme) {
            if (mpCurIme->GetGBKOut())
                s += "GBK";
            else
                s += "GB2312";
        } else
            s += "UNKNOWN";*/
    //    s += '%';
    //s += (char) 255;
    return s;
}

void ConfigServer::MenuHandleInputEncode() {
    /*
        if (mpCurIme)
            mpCurIme->SetGBKOut(!mpCurIme->GetGBKOut());
        else
            Beep();*/
}

string ConfigServer::GetTextAutoSelectUnique() {
    string s = _("Auto Select Unique Word");
    /*
    s += '%';
    s += (char) mColor1;
    s += (mAutoSelectUnique ? "Yes"  :  "No");
    s += '%';
    s += (char) 255;*/
    return s;
}


void ConfigServer::MenuHandleAutoSelectUnique() {
    /*
        mAutoSelectUnique = !mAutoSelectUnique;*/
}

void ConfigServer::MenuHandleGotoSysMenu() {
    mpCurMenu = mSysMenu;
}


