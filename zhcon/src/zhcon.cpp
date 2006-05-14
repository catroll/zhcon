// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          zhcon.cpp  -  description
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <sys/ioctl.h>

#if defined(linux)
    #include <linux/limits.h>
    #include <linux/kd.h>
    #include <linux/vt.h>
    #include <pty.h>
#elif defined(__FreeBSD__)
    #include <termios.h>
    //#include <machine/console.h>
    #include <sys/consio.h>
    #include <libutil.h>
    #define TCSETA TIOCSETA
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <stdexcept>
#include <algorithm>
#include <locale.h>

#include "global.h"
#include "console.h"

#include "encfilter.h"
#include "inputclient.h"
#include "inputmanager.h"
#include "nativeinputserver.h"
#ifdef HAVE_UNICON_LIB
    #include "uniconinputserver.h"
#endif

// for termcap, must include after gpm.h, why?
#include <term.h>

#include "gbkdecoder.h"
#include "gbdecoder.h"
#include "big5decoder.h"
#include "gb2big5decoder.h"
#include "big52gbdecoder.h"
#include "jisdecoder.h"
#include "kscdecoder.h"
#include "configfile.h"
//#include "fade.h"
#include "zhcon.h"
#include "basefont.h"
#include "encfilter.h"
#include "cmdline.h"
//#include "popwin.h"
BaseFont* gpHzFont;
BaseFont* gpAscFont;

#ifndef NDEBUG
#include "debug.h"
ofstream debug("debug");
#endif

Zhcon* Zhcon::mpZhcon = NULL;
int Zhcon::mTtyPid = 0;
Zhcon::STATE Zhcon::mState = STOP;

void Zhcon::SignalVtLeave(int signo) {
    if(Zhcon::mpZhcon != NULL) mpZhcon->VtDoLeave();
}

void Zhcon::SignalVtEnter(int signo) {
    if(Zhcon::mpZhcon != NULL) mpZhcon->VtDoEnter();
}

Zhcon::Zhcon(int argc, char* argv[])
:mpCon(NULL),
mpInputManager(NULL)
{
    Zhcon::mpZhcon = this;
    mConFd = mTtyFd = -1;

    /* let's call our CMDLINE Parser */
    if (cmdline_parser(argc, argv, &mArgs) != 0) {
        cmdline_parser_print_help();
        throw runtime_error("fail to parse command line arguments");
    }

    if (mArgs.utf8_flag)
            UseEncodingFilter = 1;
}

// will be called on exception
Zhcon::~Zhcon() {
    CleanUp();
    Zhcon::mpZhcon = NULL;
}

void Zhcon::Init() {
    //reading config file
    string cfgfile = getenv("HOME");
    cfgfile += "/.zhconrc";
    if (access(cfgfile.c_str(), R_OK) != 0)
        cfgfile = PREFIX "/etc/zhcon.conf";

    //for debug,a pause enable us to attach zhcon's pid in gdb
    //char c;cin>>c;
    ConfigFile f(cfgfile.c_str());
    //the InitXXX sequence is important,do not change
    //unless you know what you are doing
    InitTty();
    // set blank line height, must before init font
    InitGraphDev(f);
    GraphMode();
	seteuid(getuid());
    InitLocale(f); // include init font
    InitCon(f);
    InitMisc(f);
    VtSignalLeave();
    InstallSignal();
    ForkPty();
    // if setlocale run before forkpty, it cause FreeBSD hang
    SetEncode(mDefaultEncode, mDefaultEncode);
    InitInputManager(f);
    if (f.GetOption("startupmsg",true))
        StartupMsg();
}

void Zhcon::Run() {
    InputEvt evt;
    Encode e;

    int t = 0;
//    fadeout();
//    fadein();
    mState = RUNNING;
    while (mState == RUNNING) {
        mpCon->CursorBlink();
        mpInputManager->Process(evt);
        gpScreen->Update(); //   force ggi to update screen
        if (evt.oper != InputEvt::Nothing) {
            DoInputEvt(evt);
            continue;
        }

        if (++t < 10 || mAutoEncode == MANUAL)
            continue;
            
        t = 0;
        e = mpCon->DetectBufferEncode();
        switch (e) {
            case ASCII:
                SetEncode(mDefaultEncode, mDefaultEncode);
                break;
            case GB2312:
                SetEncode(GB2312,
                          mAutoEncode ==
                          AUTO_BIG5 ? BIG5 : GB2312);
                break;
            case BIG5:
                SetEncode(BIG5,
                          mAutoEncode ==
                          AUTO_GB ? GB2312 : BIG5);
                break;
            default:
               assert(!"Wrong encode!");
        }
    } //while
}

void Zhcon::DoInputEvt(InputEvt &evt) {
    switch (evt.oper) {
        case InputEvt::SetEnco:
            switch (evt.sub) {
                case 1:
                    SetEncode(GB2312, GB2312);
                    break;
                case 2:
                    SetEncode(GBK, GBK);
                    break;
                case 3:
                    SetEncode(BIG5, BIG5);
                    break;
                case 4:
                    SetEncode(JIS, JIS);
                    break;
                case 5:
                    SetEncode(KSC, KSC);
                    break;
                default:
                    break;
            }
            break;
        case InputEvt::AutoEncoSwitch:
            switch (mAutoEncode) {
                case AUTO:
                    mAutoEncode = AUTO_GB;
                    break;
                case AUTO_GB:
                    mAutoEncode = AUTO_BIG5;
                    break;
                case AUTO_BIG5:
                    mAutoEncode = MANUAL;
                    break;
                case MANUAL:
                    mAutoEncode = AUTO;
                    break;
            }
            mpInputManager->Redraw();
            Beep();
            break;
        default:
            break;
    }
}

//fork a new pty to run user's shell
void Zhcon::ForkPty() {
    char name[50];
    mTtyPid = forkpty(&mTtyFd, name, NULL, NULL);
    if (mTtyPid == -1)
        throw runtime_error("forkpty fail!");

    ioctl(0, TIOCSCTTY, 0);
    if (mTtyPid == 0) {
        //child
        struct passwd *userpd;

        if ((userpd = getpwuid(getuid())) == NULL)
            throw runtime_error("can not get user's shell!");

        /* close all opened file */
        int open_max = sysconf(_SC_OPEN_MAX);
        for (int i = 3; i < open_max; i++)
            close(i);
        setsid();
        // seteuid(getuid());   /* for security */

        //TODO: handle error condition
        if (mArgs.inputs_num > 0) {
            // the second arg of execvp must be terminated by a NULL pointer
            // we have to create a new one with NULL pointer at the end
            char** args = (char**)malloc(sizeof(char*)*(mArgs.inputs_num+1));
            for (size_t i = 0; i < mArgs.inputs_num; ++i)
                args[i] = mArgs.inputs[i];
            args[mArgs.inputs_num] = NULL;
            cout << "Executing [";
            for (size_t i = 0; i < mArgs.inputs_num; ++i)
                cout << ' ' << args[i] << ' ';
            cout << ']' << endl;
            execvp(args[0], args);
        } else
            execlp(userpd->pw_shell, userpd->pw_shell, (char *) 0);

        exit(0);
    }
}

void Zhcon::InstallSignal() {
    struct sigaction act;
    act.sa_handler = SignalHandle;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGABRT, &act, NULL);
    sigaction(SIGIOT, &act, NULL);
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    act.sa_handler = &SignalChild;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, NULL);

    //    act.sa_handler = &SignalAlarm;
    //    sigemptyset(&act.sa_mask);
    //    act.sa_flags = 0;
    //    sigaction(SIGALRM, &act, NULL);
}

// as CleanUp() is also called on exception, care should be made so that
// only full inititlized part are cleaned
void Zhcon::CleanUp() {
    struct sigaction act;
    /* done in procress and block all serious signal to prevent interrupt */
    act.sa_handler = SIG_IGN;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGILL, &act, NULL);
    sigaction(SIGABRT, &act, NULL);
    sigaction(SIGIOT, &act, NULL);
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGFPE, &act, NULL);

    VtSignalClean();
#if defined(__FreeBSD__)
    ioctl(0, VT_RELDISP, 1);
#endif

    GraphDev::Close();
    delete gpAscFont;
    delete gpHzFont;
    delete gpDecoder;
    delete mpCon;
    delete mpInputManager;
    if (mConFd != -1) {
        // need restore signal ?
        tcsetattr(mConFd, TCSAFLUSH, &mOldTermios);
        // restore console blanking
#if defined(linux)
        write(mConFd, "\033[9;10]",7);  // default 10 minutes
#elif defined(__FreeBSD__) 
        int BlankTime = 5;  // default 5 minutes
        ioctl(mConFd, CONS_BLANKTIME, &BlankTime);
#endif
        ioctl(mConFd, KDSETMODE, KD_TEXT);  // special for vga
        ioctl(mConFd, TIOCSWINSZ, &mOldWinSize);

        // for mouse, keep text mode clean
        // write(mConFd, "\n", 1);

        // make visible cursor, use termcap "ve" instead
        //write(mConFd, "\E[?25h\E[?0c", 11);
        if (mpCapCursorOn) write(mConFd, mpCapCursorOn, strlen(mpCapCursorOn));

        close(mConFd);
    }

    if (mArgs.utf8_flag)
        CleanupEncodingFilter();

    setenv("LC_ALL", mOldLocale.c_str(), 1);
}

char  Zhcon::mCapBuf[512] = {0};
char* Zhcon::mpCapClearScr = NULL;
char* Zhcon::mpCapCursorOff = NULL;
char* Zhcon::mpCapCursorOn = NULL;
void Zhcon::InitTty() {
    // Using throw cause core dump when call destruct
    if (!isatty(fileno(stdout))) {
        printf("This is an interactive api, don't redirect stdout.\r\n");
        exit(1);
    }

    char *TtyName = ttyname(fileno(stdout));
    if (!TtyName) {
        printf("Can not get current tty name.\r\n");
        exit(1);
    }
    
    if (!(strncmp("/dev/tty", TtyName, 8) == 0 ||
        strncmp("/dev/vc/", TtyName, 8) == 0)) {
        fprintf(stderr, "warning!!!\n");
        fprintf(stderr, "%s is not real tty or vc, are your running under X-Window?\n", TtyName);
#ifndef HAVE_GGI_LIB
        fprintf(stderr, "libggi support not complied in, can not run under X-Window, now quit\n");
        exit(1);
#endif
    }
    
    mConFd = open(TtyName, O_RDWR);
    if (mConFd == -1)
        throw runtime_error("Can not open console!");

    if (tcgetattr(mConFd, &mOldTermios) < 0)
        throw runtime_error("Can't get console termios.");

    struct termios t;
    t = mOldTermios;
    t.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG | NOFLSH);
    t.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON);
    t.c_cflag &= ~(CSIZE | PARENB);
    t.c_cflag |= CS8;
    t.c_oflag &= ~(OPOST);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
#if defined(linux)
    t.c_line = 0;
#elif defined(__FreeBSD__)
    t.c_cc[VDISCARD] = _POSIX_VDISABLE;
    t.c_cc[VLNEXT] = _POSIX_VDISABLE;
    t.c_cc[VSTART] = _POSIX_VDISABLE;
    t.c_cc[VSTOP] = _POSIX_VDISABLE;
    t.c_cc[VINTR] = _POSIX_VDISABLE;
    t.c_cc[VSUSP] = _POSIX_VDISABLE;
    t.c_cc[VDSUSP] = _POSIX_VDISABLE;
    t.c_cc[VQUIT] = _POSIX_VDISABLE;
    setenv("TERM","linux",1);
#endif
    if (tcsetattr(mConFd, TCSAFLUSH, &t) < 0)
        throw runtime_error("Can't set console termios\n");
    ioctl(mConFd, TIOCGWINSZ, &mOldWinSize);
    
    setenv("PTY_SLAVE", TtyName, 1);

    // disable console auto blanking to prevent screen redraw
    // problem when waking up
#if defined(linux)
    // see kernel's console.c, function setterm_command
    write(mConFd, "\033[9;0]",6);
#elif defined(__FreeBSD__) 
    // man splash
    // see /usr/src/sys/dev/syscons/syscons.c
    int BlankTime = 0;
    ioctl(mConFd, CONS_BLANKTIME, &BlankTime);
#endif

    // for mouse, keep text mode clean
    char* TermType = getenv("TERM");
    if (TermType == NULL) {
        // printf("can't find the TERM environment variable.");
        return;
    }
    int rc = tgetent(NULL, TermType);
    if (rc == -1) {
        //printf("can't find the termcap database.\n");
        //printf("check your termcap environment variable ...\n");
        return;
    }
    if (rc == 0) {
        //printf("can't find the terminal type in the termcap database.\n");
        return;
    }
    char *pCap = mCapBuf;
    mpCapClearScr = tgetstr("cl", &pCap);
    mpCapCursorOff = tgetstr("vi", &pCap);
    mpCapCursorOn  = tgetstr("ve",  &pCap);

    if (mpCapClearScr) write(mConFd, mpCapClearScr, strlen(mpCapClearScr));
    if (mpCapCursorOff) write(mConFd, mpCapCursorOff, strlen(mpCapCursorOff));
    // clear screen and home cursor, use termcap "cl" instead
    //write(mConFd, "\E[H\E[J", 6);
    // invisible cursor even in text mode, use termcap "vi" instead
    //write(mConFd, "\E[?25l\E[?1c", 11);
}

void Zhcon::SetEncode(Encode e, Encode font) {
    if (mEncode == e && mFontEncode == font)
        return;
    try {
        BaseFont *f = NULL;
        HzDecoder *h = NULL;
        string locale;
        switch (e) {
            case GB2312:
                assert(font == GB2312 || font == BIG5);
                if (font == BIG5) {
                    f = new BaseFont(mBIG5Font, 16, 16);
                    h = new GB2BIG5Decoder();
                } else {
                    f = new BaseFont(mGB2312Font, 16, 16);
                    h = new GBDecoder();
                }
                locale = "zh_CN.GB2312";
                break;
            case GBK:
                assert(font == GBK);
                f = new BaseFont(mGBKFont, 16, 16);
                h = new GBKDecoder();
                locale = "zh_CN.GBK";
                break;
            case BIG5:
                assert(font == GB2312 || font == BIG5);
                if (font == GB2312) {
                    f = new BaseFont(mGB2312Font, 16, 16);
                    h = new BIG52GBDecoder();
                } else {
                    f = new BaseFont(mBIG5Font, 16, 16);
                    h = new BIG5Decoder();
                }
                locale = "zh_TW.Big5";
                break;
            case JIS:
                assert(font == e);
                f = new BaseFont(mJISFont, 16, 16);
                h = new JISDecoder();
                locale = "ja.JIS";
                break;
            case KSC:
                assert(font == e);
                f = new BaseFont(mKSCFont, 16, 16);
                h = new KSCDecoder();
                locale = "ko";
                break;
            default:
                assert(!"unknown encode and font type");
        }
        delete gpHzFont;
        gpHzFont = f;
        gpScreen->SetDblFont(gpHzFont);
        delete gpDecoder;
        gpDecoder = h;
        mEncode = e;
        mFontEncode = font;
        //i18n locale support
        setlocale(LC_ALL,locale.c_str());
        textdomain(PACKAGE);
        //Fix me:how to change child's shell env?
        if (mpCon) {
            mpCon->SelClear();
            mpCon->ResetFlagAll();
            mpCon->Redraw(true);
        }
        if (mpInputManager)
            mpInputManager->Redraw();
        Beep();
    }//try
    catch (runtime_error & e) {}
}

void Zhcon::VtSignalSet(int mode)
{
    // Signal handing only apply to console mode (fb or vga), after GraphDev being correctly initialized, 
    // not applied to ggi
    if (GraphDev::mpGraphDev == NULL || GraphDev::mpGraphDev->Name() == "ggi")
        return;

    vt_mode vtm;
    if (ioctl(mConFd, VT_GETMODE, &vtm))
        throw runtime_error("in Zhcon::VtSignalSet() ioctl VT_GETMODE failed!");

    switch (mode) {
        case 1:  // leave
            signal(SIGUSR1, Zhcon::SignalVtLeave);
            vtm.mode = VT_PROCESS;
            vtm.relsig = SIGUSR1;
            vtm.acqsig = SIGUSR1; // for FreeBSD, see syscons.c, 0 < signal < NSIG
            vtm.frsig = SIGUSR1;
            break;
        case 2:  // enter
            signal(SIGUSR1, Zhcon::SignalVtEnter);
            vtm.mode = VT_PROCESS;
            vtm.relsig = SIGUSR1; 
            vtm.acqsig = SIGUSR1;
            vtm.frsig = SIGUSR1;
            break;
        case 0:
        default:  // clean
            signal(SIGUSR1, SIG_DFL);
            vtm.mode = VT_AUTO;
            break;
    }
    
    if (ioctl(mConFd, VT_SETMODE, &vtm))
        throw runtime_error("in Zhcon::VtSignalSet() ioctl VT_SETMODE failed!");
}

void Zhcon::VtDoLeave() {
    mpCon->SelClear();
    mpCon->CursorHide();  // when window has cursor, put in hide()
    mpCon->Hide();
    mpInputManager->Hide();

    // not needed, may change color platte when libggi used
    // TextMode();
    gpScreen->SwitchToText();
    ioctl(mConFd, VT_RELDISP, 1);

    VtSignalEnter();
}

void Zhcon::VtDoEnter() {
    // GraphMode();
    gpScreen->SwitchToGraph();
    gpScreen->ClearScr();
    mpCon->Show();
    mpCon->CursorShow();  // when window has cursor, put in show()
    mpInputManager->Show();
    ioctl(mConFd, VT_RELDISP, VT_ACKACQ);

    VtSignalLeave();
}

void Zhcon::GraphMode() {
    // for mouse, keep text mode
    //if (ioctl(mConFd, KDSETMODE, KD_GRAPHICS))
    //    throw runtime_error("ioctl KDSETMODE failed!");
    ioctl(mConFd, TIOCCONS, NULL);
}

void Zhcon::TextMode() {
    // for mouse, keep text mode
    //if (ioctl(mConFd, KDSETMODE, KD_TEXT))
    //    throw runtime_error("ioctl KDSETMODE failed!");
    ioctl(mConFd, TIOCCONS, NULL);
}

//set encode,locale then load appropriate fonts
void Zhcon::InitLocale(ConfigFile& f){
    bindtextdomain(PACKAGE,NULL);
    if (getenv("LC_ALL"))
        mOldLocale = getenv("LC_ALL");

    string prefix = PREFIX"/lib/zhcon/";
    mASCIIFont = prefix + f.GetOption(string("ascfont"), string(ASCIIFONT));
    mGB2312Font = prefix + f.GetOption(string("gbfont"), string(GB2312FONT));
    mGBKFont = prefix + f.GetOption(string("gbkfont"), string(GBKFONT));
    mBIG5Font = prefix + f.GetOption(string("big5font"), string(BIG5FONT));
    mJISFont = prefix + f.GetOption(string("jisfont"), string(JISFONT));
    mKSCFont = prefix + f.GetOption(string("kscfont"), string(KSCFONT));

    gpAscFont = new BaseFont(mASCIIFont, 8, 16);
    gpScreen->SetAscFont(gpAscFont);
    //dblfont loaded in SetEncode()

    string s;
    s = f.GetOption(string("defaultencode"), string("gb2312"));
    if (s == "gb2312") {
        //SetEncode(GB2312,GB2312);
        //FreeBsd use zh_CN.EUC instead of zh_CN.GB2312
        //a broken locale? workaround it
#if defined (__FreeBSD__)
        setenv("LC_ALL", "zh_CN.EUC", 1);
#else
        if (mArgs.utf8_flag)
            setenv("LC_ALL", "zh_CN.UTF-8", 1);
        else
            setenv("LC_ALL", "zh_CN.GB2312", 1);
#endif
        mDefaultEncode = GB2312;
    } else if (s == "gbk") {
        if (mArgs.utf8_flag)
            setenv("LC_ALL", "zh_CN.UTF-8", 1);
        else
            setenv("LC_ALL", "zh_CN.GBK", 1);
        mDefaultEncode = GBK;
    } else if (s == "big5") {
        if (mArgs.utf8_flag)
            setenv("LC_ALL", "zh_TW.UTF-8", 1);
        else
            setenv("LC_ALL", "zh_TW.Big5", 1);
        mDefaultEncode = BIG5;
    } else if (s == "jis") {
        //SetEncode(JIS,JIS);
        setenv("LC_ALL", "ja.JIS", 1);
        mDefaultEncode = JIS;
    } else if (s == "ksc") {
        //SetEncode(KSC,KSC);
        setenv("LC_ALL", "ko", 1);
        mDefaultEncode = GBK;
    } else {
        throw runtime_error("unable to set default encode!");
    }

    if (mArgs.utf8_flag) {
        SetupEncodingFilter(s.c_str());
    }

    s = f.GetOption(string("autoencode"), string("manual"));
    if (s == "auto")
        mAutoEncode = AUTO;
    else if (s == "auto-gb")
        mAutoEncode = AUTO_GB;
    else if (s == "auto-big5")
        mAutoEncode = AUTO_BIG5;
    else
        mAutoEncode = MANUAL;
}

void Zhcon::InitGraphDev(ConfigFile& f){
    bool r;
#if defined(linux)
    r = GraphDev::Open(mArgs.drv_arg);
#elif defined(__FreeBSD__)
    int xres = f.GetOption(string("x_resolution"), 640);
    int yres = f.GetOption(string("y_resolution"), 480);
    int depth = f.GetOption(string("color_depth"), 4);
    r = GraphDev::Open(xres, yres, depth);
#endif
    if (!r)
        throw(runtime_error(
            "\n============== I'm really sorry, but... ================\n"
            "I can not open graphical device on this machine, this can happen when:\n"
            "1. your kernel does not have framebuffer device enabled, check the output from `dmesg|grep vesa`\n"
            "2. you are running on a non-i386 machine so no VGA support\n"
            "3. you are running under X-Window but libggi is not compiled in (required for running zhcon under X-Window)\n"
            "\n"
            "Don't be panic by this message, thousands of people have run zhcon successfully, surely you can!\n"
            "I suggest you visit http://zhcon.sourceforge.net for more information, or send an email to zhcon-users@lists.sourceforge.net\n"
            "I'm pretty sure your problem will be solved very quickly\n"
            "You can subscribe to the list on https:// lists.sourceforge.net/lists/listinfo/zhcon-users\n"
            "\n"
            "Good Luck!\n"
            ));
    gpScreen = GraphDev::mpGraphDev;
    GraphDev::mBlankLineHeight = f.GetOption(string("blanklineheight"), 0);
    if (GraphDev::mBlankLineHeight < 0)
        GraphDev::mBlankLineHeight = 0;
    // debug << gpScreen->Width() << "," << gpScreen->Height() << endl;
    gpScreen->ClearScr();
}

void Zhcon::InitCon(ConfigFile& f){
    /*
    int BarHeight;
    if (gpScreen->BlockHeight()/2 < 4)
        BarHeight = gpScreen->BlockHeight() + 4;
    else
        BarHeight = gpScreen->BlockHeight() + gpScreen->BlockHeight()/2;
    mpCon = new Console(0, 0, gpScreen->Width() - 1,
                        gpScreen->Height() - BarHeight - 1);
    */
    mpCon = new Console(0, 0, gpScreen->Width() - 1,
                        gpScreen->Height() - 1);
    int CursorType;
    CursorType = f.GetOption(string("cursortype"), CUR_DEF);
    mpCon->SetCursorType(CursorType);

    // for mouse support
    struct winsize ws;
    ws.ws_col = mpCon->MaxCols();
    ws.ws_row = mpCon->MaxRows();
    // debug << "tty size " << mConFd << " " << ws.ws_col << "," << ws.ws_row << endl;
    ioctl(mConFd, TIOCSWINSZ, &ws);
}

void Zhcon::InitInputManager(ConfigFile& f){
    assert(mConFd >= 0); // set by InitTty()
    char *TtyName = ttyname(mConFd);
    int ttyno = atoi(&TtyName[8]);  // must be /dev/tty? style
    assert(mTtyFd >= 0); // set by ForkPty()
    InputManager::SetTty(mConFd, ttyno, mTtyFd);
    
    string s;
    s = f.GetOption(string("zhconpath"), string(PREFIX"/lib/zhcon/"));
    NativeInputServer::SetDataPath(s);
#ifdef HAVE_UNICON_LIB
    s = f.GetOption(string("uniconpath"), string("/usr/lib/unicon/"));
    UniconInputServer::SetDataPath(s);
#endif
    string sOverSpot, sNativeBar;
    sOverSpot = f.GetOption(string("overspotcolor"), string("0,7,1,1,15,8"));
    sNativeBar = f.GetOption(string("nativebarcolor"), string("15,4,11,14,0,12"));
    s = f.GetOption(string("inputstyle"),string("overspot"));
    if (s == "overspot")
        mpInputManager = new InputManager(mpCon, InputManager::OverSpot, sOverSpot, sNativeBar);
    else // others as default, "nativebar")
        mpInputManager = new InputManager(mpCon, InputManager::NativeBar, sOverSpot, sNativeBar);

    mpInputManager->LoadImmInfo(f);
    mpInputManager->Show();
    if (f.GetOption("promptmode",true))
        mpInputManager->PromptMode();
}

void Zhcon::InitMisc(ConfigFile& f){
    Beep(f.GetOption(string("beep"),true));
}

void Zhcon::StartupMsg(){
    #include "logo.h"
    string scr = screendata;
    string s = PLATFORM;
    s.resize(8,' ');
    scr.replace(scr.find("PLATFORM"),8,s);
    s = VERSION;
    s.resize(7,' ');
    scr.replace(scr.find("VERSION"),7,s);
    char buf[1024];

    sprintf(buf, "screen resolution: %dx%d [col=%d, row=%d]\r\ndefault encoding: %s %s\r\n", 
            gpScreen->Width(), gpScreen->Height(), mpCon->MaxCols(), mpCon->MaxRows(), 
            GetEncode().c_str(), (mArgs.utf8_flag?" [UTF-8]":""));
    scr += buf;
    mpCon->Write(scr.c_str(),scr.size());
}

string Zhcon::GetEncode(){
    string s;
    switch (mAutoEncode) {
        case Zhcon::AUTO:
            s = "A:";
            break;
        case Zhcon::AUTO_GB:
            s = "A-GB:";
            break;
        case Zhcon::AUTO_BIG5:
            s = "A-B5:";
            break;
        case Zhcon::MANUAL:
            s = "M:";
            break;
    }
    switch (mEncode) {
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
            s += "Unknown";
    }
    return s;
}
