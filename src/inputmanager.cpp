// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          inputmanager.cpp  -  description
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

using namespace std;

#include <unistd.h>
#include <sys/time.h>
#include <string>

#include "global.h"
#include "debug.h"
#include "keymap.h"
#include "console.h"
#include "zhcon.h"
#include "configfile.h"
#include "inputserver.h"
#include "inputclient.h"
#include "nativeinputserver.h"
#ifdef HAVE_UNICON_LIB
    #include "uniconinputserver.h"
#endif
#include "nativebarclient.h"
#include "overspotclient.h"
#include "inputmanager.h"
#include "encfilter.h"
#include "iconv_string.h"

int InputManager::mConFd = -1;
int InputManager::mConNo = -1;
int InputManager::mTtyFd = -1;

void InputManager::SetTty(int confd, int ttyno, int ttyfd) {
    mConFd = confd;
    mConNo = ttyno;
    mTtyFd = ttyfd;
}

InputManager::InputManager( Console * pCon,
    InputStyle style, string& OverSpotColors, string& NativeBarColors)
        :
        mInputRead(0), 
        mOutputRead(0), 
        mpCon(pCon),
        mpInputServer(NULL),
        mpInputClient(NULL),
        mActive(false),
        mClientVisible(false),
        mpCurImm(NULL),
        mStyle(style),
        msOverSpotColors(OverSpotColors),
        msNativeBarColors(NativeBarColors),
        mpHelpWin(NULL) {

    assert(mTtyFd >= 0);
    if (mConNo >= 0) mMouse.Open(mpCon, mConFd, mConNo, mTtyFd);
    
    InputClient::SetConsole(pCon);
    if (mStyle == OverSpot)
        mpInputClient = new OverSpotClient(msOverSpotColors);
    else // if (mStyle == NativeBar)
        mpInputClient = new NativeBarClient(msNativeBarColors);
        
    KDSysSet();
    Active();
    // set VT size
    SetVtSize();
}

InputManager::~InputManager() {
    mMouse.Close();
    //HelpHide(); may cause core dump if mpConsole is deleted first
    delete mpHelpWin;
    delete mpInputServer;
    delete mpInputClient;
    KDSysRestore();
    KDInputRestore();
}

void InputManager::Process(InputEvt &evt) {
    evt.oper = InputEvt::Nothing;
    evt.sub = 0;
    
    struct timeval tv;
    int rcved, shift;

    shift = GetShiftState();
    if (shift != 0) {
        //printf("Shift stat %d\r\n", s);
    }

    // must use SetTty after construct
    assert(mConFd >= 0 && mTtyFd >= 0);

    FD_ZERO(&mFdSet);
    FD_SET(mConFd, &mFdSet);
    FD_SET(mTtyFd, &mFdSet);
#ifdef HAVE_GPM_H  
    if (mMouse.IsOpen()) FD_SET(mMouse.mFd, &mFdSet);
#endif 
               
    tv.tv_sec = 0;
    tv.tv_usec = 100000;                  /* 0.1 sec */
    rcved = select(FD_SETSIZE, &mFdSet, NULL, NULL, &tv);
    if (rcved <= 0)
        return;

#ifdef HAVE_GPM_H
    if (mMouse.IsOpen()) {
        if (FD_ISSET(mMouse.mFd, &mFdSet)) {
            mMouse.Process();
        }
    }
#endif

    if (FD_ISSET(mConFd, &mFdSet)) {
        mOutputRead += read(mConFd, mOutputBuf+mOutputRead, BUFSIZE-mOutputRead);
        if (mOutputRead > 0) {
#ifdef HAVE_ICONV
            /* XXX: under utf8 mode the system hotkey (single char) will be converted into
             * some crazy 3-byte sequence. To avoid this problem, an ad hoc workaround is to only
             * perform encoding conversion on buffer with more than 1 byte.
             */
            if (mOutputRead > 1 && UseEncodingFilter) {
                mOutputRead = DoEncodingFilter(CONVERT_TO_UTF8_FILTER, mOutputBuf, mOutputRead);
                if (EncodingFilterLen > 0) {
                    for (size_t i = 0; i < EncodingFilterLen; i++)
                        ProcessKey(EncodingFilterBuff[i], evt);
                }
            }
            else
#endif
            {
                for (int i = 0; i < mOutputRead; i++)
                    ProcessKey(mOutputBuf[i], evt);
                mOutputRead = 0;
            }
        }
    }

   if (FD_ISSET(mTtyFd, &mFdSet)) {
        mInputRead += read(mTtyFd, mInputBuf+mInputRead, BUFSIZE-mInputRead);
        if (mInputRead > 0) {
#ifdef HAVE_ICONV
            if (UseEncodingFilter) { /* all the utf8 data go through filter to system encoding */
                mInputRead = DoEncodingFilter(CONVERT_FROM_UTF8_FILTER, mInputBuf, mInputRead);
                if (EncodingFilterLen > 0) {
                    mpCon->Write(EncodingFilterBuff, EncodingFilterLen);
                }
            }
            else
#endif
            {
                mpCon->Write(mInputBuf, mInputRead);
                mInputRead = 0;
            }
        }
    }
}

int InputManager::GetShiftState()
{
   int shift = 0;
#if defined(linux)
   int arg = 6;
   if (ioctl(mConFd, TIOCLINUX, &arg) != -1)
   {
       shift = arg;
   }
#endif
   return shift;
}

void InputManager::ProcessKey(char c, InputEvt &evt) {
    if (c == ALT_SPACE) { // always useful
        if (mActive)
            DisActive();
        else
            Active();
        return ;
    }
    if (ProcessSysKey(c, evt)) // is sys keyb
        return ;

    HelpHide();
    if (!mActive) { // not input mode, only Alt_Space
        WriteClient(c);
        return ;
    }

    if (ProcessInputKey(c)) // is sys keyb
        return ;

    if (mpInputClient->Visible() && mpInputServer) {
        bool isContinue;
        string s;
        isContinue = mpInputServer->ProcessKey(c, s);
        if (!isContinue) { // speical for ConfigServer
            DoCtrlSpace();
            return;
        }
        if (!s.empty()) {
            if (UseEncodingFilter) {
                /* need to convert output from input method into UTF-8 */
                /* TODO: need a better way to detech UseEncodingFilter */
                char* result = NULL;
                size_t len;
                int rc;
                rc = iconv_string ("UTF-8", "GBK", s.c_str(), s.c_str()+s.length()+1, &result, &len);
                if (rc == 0)
                    s = result;
                if (result)
                    free(result);
            }
            WriteClient(s);
        }
        mpInputClient->Update();
        return ;
    }
    WriteClient(c); //no imm active or prompt mode
}

//return true if a sys-key is processed
//return false:no sys-key found!
bool InputManager::ProcessSysKey(char c, InputEvt &evt) {
    bool r = true;
    switch (c) {
        case CTRL_F1:
            evt.oper = InputEvt::SetEnco;
            evt.sub = 1;
            break;
        case CTRL_F2:
            evt.oper = InputEvt::SetEnco;
            evt.sub = 2;
            break;
        case CTRL_F3:
            evt.oper = InputEvt::SetEnco;
            evt.sub = 3;
            break;
        case CTRL_F4:
            evt.oper = InputEvt::SetEnco;
            evt.sub = 4;
            break;
        case CTRL_F5:
            evt.oper = InputEvt::SetEnco;
            evt.sub = 5;
            break;
        case CTRL_F7:
            switch (mStyle) {
                case OverSpot:
                    ChangeStyle(NativeBar);
                    break;
                case NativeBar:
                    ChangeStyle(OverSpot);
                    break;
            }
            break;
        case CTRL_F8:
            //ChangeStyle(OverSpot);

            break;
        case CTRL_F9:
            evt.oper = InputEvt::AutoEncoSwitch;
            evt.sub = 0;
            break;
        case SHIFT_PAGEUP:
            mpCon->ScrollDelta(Console::PAGE_UP);
            break;
        case SHIFT_PAGEDOWN:
            mpCon->ScrollDelta(Console::PAGE_DOWN);
            break;
        case SHIFT_ARROWUP:
            mpCon->ScrollDelta(Console::LINE_UP);
            break;
        case SHIFT_ARROWDOWN:
            mpCon->ScrollDelta(Console::LINE_DOWN);
            break;
        case CTRL_ALT_H:
            if (mpHelpWin)
                HelpHide();
            else
                HelpShow();
            break;
        default:
            r = false;
    }//switch
    return r;
}

//return true if a sys-key is processed
//return false:no sys-key found!
bool InputManager::ProcessInputKey(char c) {
    bool r = true;
    switch (c) {
        case CTRL_ALT_0:
            PromptMode();
            break;
        case CTRL_ALT_1:       //fall through
        case CTRL_ALT_2:
        case CTRL_ALT_3:
        case CTRL_ALT_4:
        case CTRL_ALT_5:
        case CTRL_ALT_6:
        case CTRL_ALT_7:
        case CTRL_ALT_8:
        case CTRL_ALT_9:
            size_t i;
            i = c - CTRL_ALT_0;
            if (mImmTable.size() >= i)
                if (LoadImm(mImmTable[i - 1]) == false)
                    Beep();      //need more error feedback here?
            break;
            //case CTRL_F7:
            //break;
        case CTRL_SPACE:
        case 0: // CTRL_SPACE under GGI-X
            DoCtrlSpace();
            break;
        case CTRL_SHIFT:
            DoCtrlShift();
            break;
        case CTRL_PERIOD:
            if (mpInputServer) {
                mpInputServer->SetFullComma(!mpInputServer->IsFullComma());
                mpInputClient->Update();
            } else
                r = false;
            break;
        case CTRL_COMMA:
            if (mpInputServer) {
                mpInputServer->SetFullChar(!mpInputServer->IsFullChar());
                mpInputClient->Update();
            } else
                r = false;
            break;
        default:
            r = false;
    }//switch
    return r;
}

void InputManager::WriteClient(char c) {
    write(mTtyFd, &c, 1);
}

void InputManager::WriteClient(string & s) {
    write(mTtyFd, s.c_str(), s.size());
}

void InputManager::Active() {
    mActive = true;
    if (mClientVisible)
        mpInputClient->Show();
    // kb map set
    KDInputSet();
}

//hide all input win and restore some keymap
void InputManager::DisActive() {
    mActive = false;
    mClientVisible = mpInputClient->Visible();
    if (mpInputClient->Visible())
        mpInputClient->Hide();
    // kb map restore
    KDInputRestore();
}

//adjust vt size according to inputclient
void InputManager::SetVtSize() {
    int ColDelta, RowDelta;
    ColDelta = RowDelta = 0;
    mpInputClient->VtSizeDelta(ColDelta, RowDelta);
    mpCon->VtSizeDelta(ColDelta, RowDelta);

    int cols, rows;
    mpCon->GetVtSize(cols, rows);
    struct winsize ws;
    ws.ws_col = cols;
    ws.ws_row = rows;
    // debug<<"Change VT size "<<cols<<","<<rows<<endl;
    ioctl(mTtyFd, TIOCSWINSZ, &ws);
}

//circle through available input methods
void InputManager::DoCtrlShift() {
    if (!mpInputClient->Visible() || mpCurImm == NULL)
        return ;
    for (size_t i = 0; i < mImmTable.size(); i++)
        if (mImmTable[i] == *mpCurImm) {
            if (++i == mImmTable.size())
                i = 0;
            LoadImm(mImmTable[i]);
            break;
        }
}

bool InputManager::LoadImm(ImmInfo& rInfo) {
    if (mpCurImm && rInfo == *mpCurImm)
        return true;
    ImmInfo* pOld = mpCurImm;
    if (mpInputServer == NULL || mpInputServer->GetServerType() != rInfo.mType) {
        delete mpInputServer;
        if (rInfo.mType == "native")
            mpInputServer = new NativeInputServer();
#ifdef HAVE_UNICON_LIB
        else if (rInfo.mType == "unicon")
            mpInputServer = new UniconInputServer();
#endif
        else
            assert(!"unknown input server found!");
        mpInputClient->Connect(mpInputServer);
    }
    if (mpInputServer->LoadImm(rInfo)) {
        mpCurImm = &rInfo;
        if (rInfo.mEncode != gpZhcon->mEncode)
            gpZhcon->SetEncode(rInfo.mEncode, rInfo.mEncode);
        mpInputClient->Update();
        return true;
    } else {
        //restore old imm
        if (pOld) {
            mpCurImm = NULL; //force reload
            bool r = LoadImm(*pOld);
            assert(r);
            r = true; /* remove warning of unused r */
        }
        return false;
    }
}

void InputManager::LoadImmInfo(ConfigFile& f) {
    ImmInfo t;
    string s;
    unsigned int ParmNo, Pos;
    unsigned int NextPos;
    string sVal;
    vector < string > v;
    f.GetOptions(string("ime"), v);
    for (unsigned i = 0; i < v.size(); i++) {
        s = v[i];
        ParmNo = 0;
        Pos = 0;
        while ( Pos < s.length() ) {
            NextPos = s.find(',', Pos);
            if (NextPos == (unsigned int)string::npos)
                NextPos = s.length();
            sVal = s.substr(Pos, NextPos - Pos);
            switch (ParmNo) {
                case 0:
                    t.mName = sVal;
                    break;
                case 1:
                    t.mModule = sVal;
                    break;
                case 2:
                    t.mTable = sVal;
                    break;
                case 3:
                    if (sVal == "gb2312")
                        t.mEncode = GB2312;
                    else if (sVal == "gbk")
                        t.mEncode = GBK;
                    else if (sVal == "big5")
                        t.mEncode = BIG5;
                    else
                        assert(!"unsupport imm encode type found in config file");
                    break;
                case 4:
                    t.mType = sVal;
                    break;
                default:
                    break;
            }
            ParmNo++;
            Pos = NextPos + 1;
        }
#ifndef HAVE_UNICON_LIB
        if (t.mType == "unicon")
            continue;
#endif
        mImmTable.push_back(t);
    }
}

void InputManager::DoCtrlSpace() {
    if (mpCurImm == NULL && mImmTable.size() > 0) {
        if (LoadImm(mImmTable[0]) == false) {
            Beep();
            return ;
        }
        mpInputClient->Show();
        mpInputClient->Update();
        return ;
    }
    if (mpInputClient->Visible()) {
        //        if (dynamic_cast<NativeBarClient *>(mpInputClient))
        //            PromptMode();
        //        else
        mpInputClient->Hide();
    } else {
        mpInputClient->Show();
        mpInputClient->Update();
    }
}

#if 0
void InputManager::MenuMode() {
    if (mpInputServer)
        delete mpInputServer;
    mpInputServer = new ConfigServer();
    mpCurImm = NULL;
    mpInputClient->Connect(mpInputServer);
    mpInputClient->Show();
    mpInputClient->Update();
}
#endif

void InputManager::Show() {
    if (mActive && mClientVisible) {
        mpInputClient->Update();
        mpInputClient->Show();
    }
    KDSysSet();
    KDInputSet();
}

void InputManager::Hide() {
    //because mClientVisible has been updated in DisActive()
    //so we only update mClientVisible when active
    if (mActive)
        mClientVisible = mpInputClient->Visible();
    mpInputClient->Hide();
    KDSysRestore();
    KDInputRestore();
}

void InputManager::Redraw() {
    mpInputClient->Update();
}

bool InputManager::mKDSysSaved = false;
bool InputManager::mKDInputSaved = false;

#if defined(linux)
#include <linux/keyboard.h>
#include <linux/kd.h>
InputManager::KeyMap InputManager::mKDSysMap[] = {
    //ALT_X
    {1 << KG_ALT, 57, 0, K(KT_LATIN, ALT_SPACE)},
    //CTRL_X
    {1 << KG_CTRL, 59, 0, K(KT_LATIN, CTRL_F1)},
    {1 << KG_CTRL, 60, 0, K(KT_LATIN, CTRL_F2)},
    {1 << KG_CTRL, 61, 0, K(KT_LATIN, CTRL_F3)},
    {1 << KG_CTRL, 62, 0, K(KT_LATIN, CTRL_F4)},
    {1 << KG_CTRL, 63, 0, K(KT_LATIN, CTRL_F5)},
    {1 << KG_CTRL, 65, 0, K(KT_LATIN, CTRL_F7)},
    {1 << KG_CTRL, 66, 0, K(KT_LATIN, CTRL_F8)},
    {1 << KG_CTRL, 67, 0, K(KT_LATIN, CTRL_F9)},
    {1 << KG_CTRL, 68, 0, K(KT_LATIN, CTRL_F10)},
    //SHIFT_X
    {1 << KG_SHIFT, 103, 0, K(KT_LATIN, SHIFT_ARROWUP)},
    {1 << KG_SHIFT, 104, 0, K(KT_LATIN, SHIFT_PAGEUP)},
    {1 << KG_SHIFT, 108, 0, K(KT_LATIN, SHIFT_ARROWDOWN)},
    {1 << KG_SHIFT, 109, 0, K(KT_LATIN, SHIFT_PAGEDOWN)},
    //CTRL_ALT_X
    {(1 << KG_CTRL) + (1 << KG_ALT), 35, 0, K(KT_LATIN, CTRL_ALT_H)},
};

InputManager::KeyMap InputManager::mKDInputMap[] = {
    //CTRL_ALT_X
    {(1 << KG_CTRL) + (1 << KG_ALT), 11, 0, K(KT_LATIN, CTRL_ALT_0)},
    {(1 << KG_CTRL) + (1 << KG_ALT), 2, 0, K(KT_LATIN, CTRL_ALT_1)},
    {(1 << KG_CTRL) + (1 << KG_ALT), 3, 0, K(KT_LATIN, CTRL_ALT_2)},
    {(1 << KG_CTRL) + (1 << KG_ALT), 4, 0, K(KT_LATIN, CTRL_ALT_3)},
    {(1 << KG_CTRL) + (1 << KG_ALT), 5, 0, K(KT_LATIN, CTRL_ALT_4)},
    {(1 << KG_CTRL) + (1 << KG_ALT), 6, 0, K(KT_LATIN, CTRL_ALT_5)},
    {(1 << KG_CTRL) + (1 << KG_ALT), 7, 0, K(KT_LATIN, CTRL_ALT_6)},
    {(1 << KG_CTRL) + (1 << KG_ALT), 8, 0, K(KT_LATIN, CTRL_ALT_7)},
    {(1 << KG_CTRL) + (1 << KG_ALT), 9, 0, K(KT_LATIN, CTRL_ALT_8)},
    {(1 << KG_CTRL) + (1 << KG_ALT), 10, 0, K(KT_LATIN, CTRL_ALT_9)},
    //ALT_X
    {1 << KG_ALT, 11, 0, K(KT_LATIN, ALT_0)},
    {1 << KG_ALT, 2, 0, K(KT_LATIN, ALT_1)},
    {1 << KG_ALT, 3, 0, K(KT_LATIN, ALT_2)},
    {1 << KG_ALT, 4, 0, K(KT_LATIN, ALT_3)},
    {1 << KG_ALT, 5, 0, K(KT_LATIN, ALT_4)},
    {1 << KG_ALT, 6, 0, K(KT_LATIN, ALT_5)},
    {1 << KG_ALT, 7, 0, K(KT_LATIN, ALT_6)},
    {1 << KG_ALT, 8, 0, K(KT_LATIN, ALT_7)},
    {1 << KG_ALT, 9, 0, K(KT_LATIN, ALT_8)},
    {1 << KG_ALT, 10, 0, K(KT_LATIN, ALT_9)},
    //CTRL_X
    {1 << KG_CTRL, 42, 0, K(KT_LATIN, CTRL_SHIFT)},
    {1 << KG_CTRL, 57, 0, K(KT_LATIN, CTRL_SPACE)},
    {1 << KG_CTRL, 52, 0, K(KT_LATIN, CTRL_PERIOD)},
    {1 << KG_CTRL, 51, 0, K(KT_LATIN, CTRL_COMMA)},
};

void InputManager::KDSysSet() {
    struct kbentry kb;
    for (unsigned i = 0; i < sizeof(mKDSysMap) / sizeof(KeyMap); i++) {
        kb.kb_table = mKDSysMap[i].mTable;
        kb.kb_index = mKDSysMap[i].mIndex;
        if (!mKDSysSaved) {
            ioctl(0, KDGKBENT, &kb);
            mKDSysMap[i].mOldValue = kb.kb_value;
        }
        kb.kb_value = mKDSysMap[i].mNewValue;
        ioctl(0, KDSKBENT, &kb);
    }
    mKDSysSaved = true;
}

void InputManager::KDSysRestore() {
    if (!mKDSysSaved)
        return ;
    struct kbentry kb;
    for (unsigned i = 0; i < sizeof(mKDSysMap) / sizeof(KeyMap); i++) {
        kb.kb_table = mKDSysMap[i].mTable;
        kb.kb_index = mKDSysMap[i].mIndex;
        kb.kb_value = mKDSysMap[i].mOldValue;
        ioctl(0, KDSKBENT, &kb);
    }
//    mKDSysSaved = true;
}

void InputManager::KDInputSet() {
    struct kbentry kb;
    for (unsigned i = 0; i < sizeof(mKDInputMap) / sizeof(KeyMap); i++) {
        kb.kb_table = mKDInputMap[i].mTable;
        kb.kb_index = mKDInputMap[i].mIndex;
        if (!mKDInputSaved) {
            ioctl(0, KDGKBENT, &kb);
            mKDInputMap[i].mOldValue = kb.kb_value;
        }
        kb.kb_value = mKDInputMap[i].mNewValue;
        ioctl(0, KDSKBENT, &kb);
    }
    mKDInputSaved = true;
}

void InputManager::KDInputRestore() {
    if (!mKDInputSaved)
        return ;
    struct kbentry kb;
    for (unsigned i = 0; i < sizeof(mKDInputMap) / sizeof(KeyMap); i++) {
        kb.kb_table = mKDInputMap[i].mTable;
        kb.kb_index = mKDInputMap[i].mIndex;
        kb.kb_value = mKDInputMap[i].mOldValue;
        ioctl(0, KDSKBENT, &kb);
    }
}

#elif defined(__FreeBSD__)
#include <termios.h>
//#include <machine/console.h>
#include <sys/consio.h>

keymap_t InputManager::mKDSysOld;
keymap_t InputManager::mKDSysNew;
keymap_t InputManager::mKDInputOld;
keymap_t InputManager::mKDInputNew;

void InputManager::KDSysSet() {
    if (!mKDSysSaved) {
        ioctl(0, GIO_KEYMAP, &mKDSysOld);
        mKDSysNew = mKDSysOld;

        // ALT_X
        mKDSysNew.key[57].map[4] = ALT_SPACE;

        // SHIFT_X
        mKDSysNew.key[95].map[1] = SHIFT_ARROWUP;
        mKDSysNew.key[96].map[1] = SHIFT_PAGEUP;
        mKDSysNew.key[100].map[1] = SHIFT_ARROWDOWN;
        mKDSysNew.key[101].map[1] = SHIFT_PAGEDOWN;

        // CTRL_X
        mKDSysNew.key[59].map[2] = CTRL_F1;
        mKDSysNew.key[60].map[2] = CTRL_F2;
        mKDSysNew.key[61].map[2] = CTRL_F3;
        mKDSysNew.key[62].map[2] = CTRL_F4;
        mKDSysNew.key[63].map[2] = CTRL_F5;
        //mKDSysNew.key[64].map[2] = CTRL_F6;
        mKDSysNew.key[65].map[2] = CTRL_F7;
        mKDSysNew.key[66].map[2] = CTRL_F8;
        mKDSysNew.key[67].map[2] = CTRL_F9;
        mKDSysNew.key[68].map[2] = CTRL_F10;
    }
    // Write Keyboard Keymap now;
    ioctl(0, PIO_KEYMAP, &mKDSysNew);
    mKDSysSaved = true;
}

void InputManager::KDSysRestore() {
    if (!mKDSysSaved)
        return ;
    ioctl(0, PIO_KEYMAP, &mKDSysOld);
}

void InputManager::KDInputSet() {
    if (!mKDInputSaved) {
        ioctl(0, GIO_KEYMAP, &mKDInputOld);
        mKDInputNew = mKDInputOld;

        // CTRL_ALT_X
        mKDInputNew.key[2].map[6] = CTRL_ALT_1;
        mKDInputNew.key[3].map[6] = CTRL_ALT_2;
        mKDInputNew.key[4].map[6] = CTRL_ALT_3;
        mKDInputNew.key[5].map[6] = CTRL_ALT_4;
        mKDInputNew.key[6].map[6] = CTRL_ALT_5;
        mKDInputNew.key[7].map[6] = CTRL_ALT_6;
        mKDInputNew.key[8].map[6] = CTRL_ALT_7;
        mKDInputNew.key[9].map[6] = CTRL_ALT_8;
        mKDInputNew.key[10].map[6] = CTRL_ALT_9;
        mKDInputNew.key[11].map[6] = CTRL_ALT_0;

        // ALT_X
        mKDInputNew.key[2].map[4] = ALT_1;
        mKDInputNew.key[3].map[4] = ALT_2;
        mKDInputNew.key[4].map[4] = ALT_3;
        mKDInputNew.key[5].map[4] = ALT_4;
        mKDInputNew.key[6].map[4] = ALT_5;
        mKDInputNew.key[7].map[4] = ALT_6;
        mKDInputNew.key[8].map[4] = ALT_7;
        mKDInputNew.key[9].map[4] = ALT_8;
        mKDInputNew.key[10].map[4] = ALT_9;
        mKDInputNew.key[11].map[4] = ALT_0;

        // CTRL_X
        mKDInputNew.key[42].map[2] = CTRL_SHIFT;
        mKDInputNew.key[57].map[2] = CTRL_SPACE;
        mKDInputNew.key[52].map[2] = CTRL_PERIOD;
        mKDInputNew.key[51].map[2] = CTRL_COMMA;
    }
    // Write Keyboard Keymap now;
    ioctl(0, PIO_KEYMAP, &mKDInputNew);
    mKDInputSaved = true;
}

void InputManager::KDInputRestore() {
    if (!mKDInputSaved)
        return ;
    ioctl(0, PIO_KEYMAP, &mKDInputOld);
}

#endif

void InputManager::ChangeStyle(InputStyle style) {
    if (style == mStyle) return ;
    mStyle = style;
    bool visible = mpInputClient->Visible();
    mpInputClient->Hide();
    delete mpInputClient;

    if (mStyle == NativeBar)
        mpInputClient = new NativeBarClient(msNativeBarColors);
    else if (mStyle == OverSpot)
        mpInputClient = new OverSpotClient(msOverSpotColors);
    else
        throw runtime_error("unknown input style.");

    if (mpInputServer)
        mpInputClient->Connect(mpInputServer);

    // set VT size
    SetVtSize();
    //force api fresh screen
    write(mTtyFd, "\0", 1);

    if (visible) {
        mpInputClient->Update();
        mpInputClient->Show();
    }
    Beep();
}

void InputManager::PromptMode() {
    assert(mpInputClient);
    mpInputClient->DisConnect();
    delete mpInputServer;
    mpCurImm = NULL;
    mpInputServer = NULL;
    mpInputClient->Update();
    mpInputClient->Show();
}

void InputManager::HelpShow() {
    if (mpHelpWin != NULL)
        return ;

    int c1, r1, c2, r2;
    c1 = (mpCon->MaxCols() - 60) / 2;
    c2 = c1 + 61;
    r1 = (mpCon->MaxRows() - 18) / 2;
    r2 = r1 + 18;
    mpHelpWin = new Window(c1 * gpScreen->BlockWidth(),
                           r1 * gpScreen->BlockHeight(),
                           c2 * gpScreen->BlockWidth() - 1,
                           r2 * gpScreen->BlockHeight() - 1,
                           WS_CHILD | WS_FRAMETHICK);
    mpHelpWin->SetFrameColor(0, 12);
    mpHelpWin->SetFgColor(15);
    mpHelpWin->SetBgColor(4);
    mpHelpWin->Clear();

    mpHelpWin->PutStr(16, 0,  _("Press CTRL-ALT-H to exit help"));
    mpHelpWin->PutStr(1, 1,   _("ALT -SPACE  Open/Close CJK mode"));
    mpHelpWin->PutStr(1, 2,   _("CTRL-SPACE  Open/Close input method"));
    mpHelpWin->PutStr(1, 3,   _("SHIFT-PageUp/PageDown/Up/Down   Scroll & Display History"));

    mpHelpWin->PutStr(1, 5,   _("CTRL key:"));
    mpHelpWin->PutStr(1, 6,   _("F1  GB2312"));
    mpHelpWin->PutStr(15, 6,  _("F2  GBK"));
    mpHelpWin->PutStr(30, 6,  _("F3  BIG5"));
    mpHelpWin->PutStr(45, 6,  _("F4  JIS"));
    mpHelpWin->PutStr(1, 7,   _("F5  KSC"));
    mpHelpWin->PutStr(15, 7,  _("F9  Switch & Auto Detect"));
    // mpHelpWin->PutStr(45, 7,  _("F10 Sys Menu"));
    mpHelpWin->PutStr(1, 8,   _("F7  Input Style"));
    mpHelpWin->PutStr(45, 8,  _("D   Exit zhcon"));

    mpHelpWin->PutStr(1, 10,  _("Chinese/English Mode:"));
    mpHelpWin->PutStr(1, 11,  _("CTRL-,  Full/Half Char"));
    mpHelpWin->PutStr(1, 12,  _("CTRL-.  Full/Half Comma"));

    mpHelpWin->PutStr(30, 10, _("CTRL-ALT-NUM:"));
    mpHelpWin->PutStr(30, 11, _("0      Prompt Mode"));
    mpHelpWin->PutStr(30, 12, _("1..9   Select IME 1..9"));

    mpHelpWin->PutStr(1, 14,  _("Special Input Key:"));
    mpHelpWin->PutStr(1, 15,  _("CTRL-SHIFT  Next IME"));
    mpHelpWin->PutStr(30, 15, _("SPACE       Select Number 0"));
    mpHelpWin->PutStr(1, 16,  _("+/-         Next/Prev page"));
    mpHelpWin->PutStr(30, 16, _("ESC         Reset Input Area"));

    mpHelpWin->Show();
}

void InputManager::HelpHide() {
    if (mpHelpWin == NULL)
        return ;

    mpHelpWin->Hide();
    delete mpHelpWin;
    mpHelpWin = NULL;
}


