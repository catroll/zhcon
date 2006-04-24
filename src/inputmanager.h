// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          inputmanager.h  -  description
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

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H


/**
  *@author ejoy
  */
using namespace std;
#include <cassert>
#include <vector>

#include "mouse.h"
#if defined(__FreeBSD__)
    #include <sys/kbio.h>
#endif

#define BUFSIZE 8192
class Zhcon;
class Console;
class InputServer;
class InputClient;
class ImmInfo;
class ConfigFile;
class Window;

typedef struct InputEvt_t {
    enum Oper {Nothing, SetEnco, AutoEncoSwitch};
    Oper oper;
    int  sub;
} InputEvt;

class InputManager {
    friend class ConfigServer;
    public:
    enum InputStyle {NativeBar,OverSpot};
    InputManager(Console * pCon, InputStyle style,
            string& OverSpotColors, string& NativeBarColors);
    ~InputManager();
    static void SetTty(int confd, int ttyno, int ttyfd);
    void Process(InputEvt &evt);

    void LoadImmInfo(ConfigFile& f);
    void Show();
    void Hide();
    void Redraw();
    void PromptMode();
    private:
    void ProcessKey(char c, InputEvt &evt);
    int  GetShiftState();
    bool ProcessSysKey(char c, InputEvt &evt);
    bool ProcessInputKey(char c);
    class Mouse mMouse;

    void WriteClient(char c);
    void WriteClient(string & s);
    void OutChar(char c);
    void Active();
    void DisActive();
    void SetVtSize();
    void DoCtrlShift();
    bool LoadImm(ImmInfo& rInfo);
    void DoCtrlSpace();
    void MenuMode();

    void KDSysSet();
    void KDSysRestore();
    void KDInputSet();
    void KDInputRestore();
    void ChangeStyle(InputStyle style);
    void HelpShow();
    void HelpHide();

    static int mConFd, mConNo, mTtyFd;
    fd_set mFdSet;

    char mInputBuf[BUFSIZE];
    char mOutputBuf[BUFSIZE];
    int mInputRead;
    int mOutputRead;
#if defined(linux)
    struct KeyMap {
        unsigned char mTable;
        unsigned char mIndex;
        unsigned short mOldValue;
        unsigned short mNewValue;
    };
    static KeyMap mKDSysMap[];
    static KeyMap mKDInputMap[];
#elif defined(__FreeBSD__)
    static keymap_t mKDSysOld, mKDSysNew;
    static keymap_t mKDInputOld, mKDInputNew;
#endif
    static bool mKDSysSaved;
    static bool mKDInputSaved;
    Zhcon *mpOwner;
    Console *mpCon;
    InputServer *mpInputServer;
    InputClient *mpInputClient;
    int mClientFd;
    bool mActive;
    bool mClientVisible;
    ImmInfo* mpCurImm;
    vector < ImmInfo > mImmTable;
    InputStyle mStyle;
    string msOverSpotColors;
    string msNativeBarColors;
    Window *mpHelpWin;
};

#endif
