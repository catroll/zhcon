// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          zhcon.h  -  description
                             -------------------
    begin                : Fri Mar 23 2001
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

#ifndef ZHCON_H
#define ZHCON_H

/**
 *@author ejoy
 */
#include <termios.h>
#include <sys/ioctl.h>
#include <string>
#include "global.h"
#include "cmdline.h"

using namespace std;

const char FBDEVICE[] = "/dev/fb0";
const char ASCIIFONT[] = "asc16";
const char GB2312FONT[] = "gb16";
const char GBKFONT[] = "gbk16";
const char BIG5FONT[] = "big516";
const char JISFONT[] = "jis16";
const char KSCFONT[] = "ksc16";

class HzDecoder;
class Console;
#include "inputmanager.h"
class ConfigServer;
class ConfigFile;
class Zhcon {
    friend void SignalHandle(int signo);
    friend void SignalChild(int signo);
    friend void SignalAlarm(int signo);

    friend class InputManager;
    friend class ConfigServer;

    public:
    static void SignalVtLeave(int signo);
    static void SignalVtEnter(int signo);
    static Zhcon* mpZhcon;
    void VtDoLeave();
    void VtDoEnter();

    public:
    Zhcon(int argc, char* argv[]);
    ~Zhcon();
    void Run();
    void Init();
    string GetEncode();

    private:
    void ForkPty();
    void InstallSignal();
    void CleanUp();

    void InitTty();
    static char  mCapBuf[512];
    static char* mpCapClearScr;
    static char* mpCapCursorOff;
    static char* mpCapCursorOn;

    void SetEncode(Encode e,Encode font);

    void VtSignalSet(int mode);
    void VtSignalClean() {
        VtSignalSet(0);
    }
    void VtSignalLeave() {
        VtSignalSet(1);
    }
    void VtSignalEnter() {
        VtSignalSet(2);
    }

    void TextMode();
    void GraphMode();

    void DoInputEvt(InputEvt &evt);

    void InitLocale(ConfigFile& f);
    void InitGraphDev(ConfigFile& f);
    void InitInputManager(ConfigFile& f);
    void InitCon(ConfigFile& f);
    void InitMisc(ConfigFile& f);
    void StartupMsg();

    enum STATE {STOP,RUNNING,TERMINATE};
    int mConFd;
    int mTtyFd;
    static int mTtyPid;
    static STATE mState;
    Console* mpCon;
    InputManager* mpInputManager;
    struct termios mOldTermios;
    struct winsize mOldWinSize;
    Encode mEncode;
    Encode mFontEncode;
    Encode mDefaultEncode;
    Encode mPreferEncode; //used to detect GB->Big5 or Big5->GB
    string mASCIIFont;
    string mGB2312Font;
    string mGBKFont;
    string mBIG5Font;
    string mJISFont;
    string mKSCFont;
    string mOldLocale;
    enum AutoEncodeType {AUTO,AUTO_GB,AUTO_BIG5,MANUAL};
    AutoEncodeType mAutoEncode;
    gengetopt_args_info mArgs;
};
#endif
