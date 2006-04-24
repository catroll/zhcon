// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          mouse.cpp  -  description
                             -------------------
    begin                : Sat Mar 2 2002
    copyright            : (C) 2002 by huyong
    email                : ccpaging@online.sh.cn
**************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cassert>
#include "debug.h"
#include "mouse.h"
#include <sys/signal.h>

#if defined(__FreeBSD__)
Mouse* Mouse::pMouse = NULL;
void Mouse::SignalProcess(int signo) {
    if (Mouse::pMouse != NULL)  pMouse->Process();
}
#endif

Mouse::Mouse() {
#if defined(__FreeBSD__)
    Mouse::pMouse = this;
#endif   
    mFd = -1;
    mButtons = 2;
    mpCon = NULL;
    mTtyFd = -1;
    
    mX1 = mY1 = 0;
}

Mouse::~Mouse() {
    Close();
#if defined(__FreeBSD__)
    Mouse::pMouse = NULL;
#endif
}

bool Mouse::Open(Console* pCon, int confd, int ttyno, int ttyfd) {
    assert(pCon);
    assert(ttyfd >= 0);
    mpCon = pCon;
    mConFd = confd;
    mTtyFd = ttyfd;

#ifdef HAVE_GPM_H
    mConn.eventMask   = ~0;         // get everything
    mConn.defaultMask = ~GPM_HARD;  // pass everything unused
    mConn.minMod      = 0;          // run always
    mConn.maxMod      = ~0;         // with any modifier
    if (Gpm_Open(&mConn, ttyno)==-1) {
      //printf("Can't open mouse connection at %d\r\n", mMouseConn.vc);
      //printf("pid=%d, vc=%d\r\n",mMouseConn.pid, mMouseConn.vc);
      return false;
    }

    mFd = gpm_fd;
    //printf("pid=%d, vc=%d\r\n",mMouseConn.pid, mMouseConn.vc);

    // Get mouse button, based on mev.c in gpm
    Gpm_Event event;
    int i=Gpm_GetSnapshot(&event);
    if (-1 == i) {
       printf("Warning: cannot get snapshot!\n");
       printf("Maybe in xterm or before connecting?\n");
       return true;
    }
    mButtons = i;

    /*
    printf("Mouse has %d buttons\n",i);
    printf("Currently sits at (%d,%d)\n",event.x,event.y);
    printf("The window is %d columns by %d rows\n",event.dx,event.dy);
    s=Gpm_GetLibVersion(&i);
    printf("The library is version \"%s\" (%i)\n",s,i);
    s=Gpm_GetServerVersion(&i);
    printf("The daemon is version \"%s\" (%i)\n",s,i);
    printf("The current console is %d, with modifiers 0x%02x\n",
        event.vc,event.modifiers);
    printf("The button mask is 0x%02X\n",event.buttons);
    */
#elif defined(__FreeBSD__)
    memset(&mMiPrev, 0, sizeof(mMiPrev));
    memset(&mTV1, 0, sizeof(mTV1));
    memset(&mTV2, 0, sizeof(mTV2));
    mClicks = 0;
    struct mouse_info mi;
    mi.operation = MOUSE_MODE;
    mi.u.mode.signal = SIGUSR2;
    if (ioctl(mConFd, CONS_MOUSECTL, &mi) < 0) {
        //err(1, "CONS_MOUSECTL");
        return false;
    }
    signal(SIGUSR2, Mouse::SignalProcess);
#endif
    return true;
}

void Mouse::Close() {
#ifdef HAVE_GPM_H
    if (mFd <= 0)
        return;
    while (Gpm_Close()); // close all mouse stack
    mFd = -1;
#elif defined(__FreeBSD__)
    signal(SIGUSR2, SIG_DFL);
#endif
}

enum MouseType {
    MOUSE_MOVE = 1,
    MOUSE_DRAG = 2,
    MOUSE_DOWN = 4,
    MOUSE_UP = 8
};

#define MOUSE_LEFT     4
#define MOUSE_MIDDLE   2
#define MOUSE_RIGHT        1

// based on function do_selection in gpm.c
#define DEF_PTRDRAG          1    /* double or triple click */

#define GET_TIME(tv)       (gettimeofday(&tv, (struct timezone *)NULL))
#define DIF_TIME(t1, t2)   ((t2.tv_sec  - t1.tv_sec) * 1000 + \
                           (t2.tv_usec - t1.tv_usec) / 1000)
#define DEF_TIME           250 // time interval (ms) for multiple clicks
void Mouse::Process() {
    assert(mpCon);
    int maxX, maxY;
    mpCon->GetVtSize(maxX, maxY);
   
#ifdef HAVE_GPM_H
    if (mFd <= 0)
        return;

    Gpm_Event event;
    if (Gpm_GetEvent(&event) <= 0)
        return;

    mX2 = event.x - 1;
    mY2 = event.y - 1;
    switch(GPM_BARE_EVENTS(event.type)) {
        case GPM_MOVE:
            if (mX2 < 0)
                mX2 = 0;
            else if (mX2 > maxX - 1)
                mX2 = maxX - 1;
            else;
            if (mY2 < 0)
                mY2 = 0;
            else if (mY2 > maxY - 1)
                mY2 = maxY - 1;
            else;
            mpCon->SelCopy(mX2, mY2, mX2, mY2,3); /* just highlight pointer */
            return;

        case GPM_DRAG:
            if (event.buttons==GPM_B_LEFT) {
                if (event.margin) { /* fix margins */
                    switch(event.margin) {
                        case GPM_TOP:
                            mX2 = 0;
                            mY2++;
                            break;
                        case GPM_BOT:
                            mX2 = maxX - 1;
                            mY2--;
                            break;
                        case GPM_RGT:
                            mX2--;
                            break;
                        case GPM_LFT:
                            if (mY2 <= mY1)
                               mX2++;
                            else {
                               mX2 = maxX - 1;
                               mY2--;
                            }
                            break;
                    }
                }
                mpCon->SelCopy(mX1, mY1, mX2, mY2, event.clicks);
                if (event.clicks >= DEF_PTRDRAG && !event.margin) /* pointer */
                    mpCon->SelCopy(mX2, mY2, mX2, mY2, 3);
            } /* if */
            return;

        case GPM_DOWN:
            switch (event.buttons) {
                case GPM_B_LEFT:
                    mX1 = mX2;
                    mY1 = mY2;
                    mpCon->SelCopy(mX1, mY1, mX2, mY2, event.clicks);  /* start selection */
                    return;

                case GPM_B_MIDDLE:
                    mpCon->SelPaste(mTtyFd);
                    return;

                case GPM_B_RIGHT:
                    if (mButtons == 3)
                        mpCon->SelCopy(mX1, mY1, mX2, mY2, event.clicks);
                    else
                        mpCon->SelPaste(mTtyFd);
                    return;
            }
    } /* switch above */
#elif defined(__FreeBSD__)
    struct mouse_info mi;
    mi.operation = MOUSE_GETINFO;
    if (ioctl(mConFd, CONS_MOUSECTL, &mi) < 0) {
        //err(1, "CONS_MOUSECTL");
        return;
    }

    //debug << mi.u.data.x << "," << mi.u.data.y << endl;
    /*
    if (mMiPrev.u.data.x == mi.u.data.x
        && mMiPrev.u.data.y == mi.u.data.y
        && mMiPrev.u.data.buttons == mi.u.data.buttons) {
        return;
    }
    */
   
    int x = (mi.u.data.x * mpCon->MaxCols()) / mpCon->Width();
    int y = (mi.u.data.y * mpCon->MaxRows()) / mpCon->Height();
    //debug<<"Mouse pointer ("<<x<<","<<y<<")";
    /*
    if (mX2 == x && mY2 == y
        && mMiPrev.u.data.buttons == mi.u.data.buttons) {
        return;
    }
    */
    mX2 = x;
    mY2 = y;

    // based on gpm, gpm.c, function processMouse
    MouseType evtype;
    if (mMiPrev.u.data.buttons == mi.u.data.buttons) {
        if (mi.u.data.buttons > 0) {
            evtype = MOUSE_DRAG;
            //debug << " drag";
        } else {
            evtype = MOUSE_MOVE;
            //debug << " move";
        }
    } else {
        if (mi.u.data.buttons > mMiPrev.u.data.buttons) {
            evtype = MOUSE_DOWN;
            //debug << " down";
        } else {
            evtype = MOUSE_UP;
            //mi.u.data.buttons ^= mMiPrev.u.data.buttons;
            //debug << " up";
        }
    }
    
    int buttons = 0;
    if (mi.u.data.buttons & MOUSE_BUTTON1DOWN) {
        buttons = MOUSE_LEFT;
        //debug << " left";
    }
    if (mi.u.data.buttons & MOUSE_BUTTON2DOWN) {
        buttons = MOUSE_MIDDLE;
        //debug << " middle";
    }
    if (mi.u.data.buttons & MOUSE_BUTTON3DOWN) {
        buttons = MOUSE_RIGHT;
        //debug << " right";
    }

    int clicks;
    switch (evtype) {
        case MOUSE_DOWN:
            GET_TIME(mTV2);
            // check first click
            if (mTV1.tv_sec && DIF_TIME(mTV1, mTV2) < DEF_TIME) {
                mClicks++;
                mClicks %= 3;
            } else {
                mClicks = 0;
            }
            break;
        case MOUSE_UP:
            GET_TIME(mTV1);
            break;
        case MOUSE_DRAG:
            break;

        case MOUSE_MOVE:
            mClicks = 0;
        default:
            break;
    }
    clicks = mClicks;
    //debug << " clicks " << clicks << endl;
   
    switch( evtype ) {
        case MOUSE_MOVE:
            mpCon->SelCopy(mX2, mY2, mX2, mY2,3); /* just highlight pointer */
            break;
        case MOUSE_DRAG:
            if (buttons == MOUSE_LEFT) {
                mpCon->SelCopy(mX1, mY1, mX2, mY2, clicks);
                if (clicks >= DEF_PTRDRAG) /* pointer */
                    mpCon->SelCopy(mX2, mY2, mX2, mY2, 3);
            } /* if */
            break;
        case MOUSE_DOWN:
            switch (buttons) {
                case MOUSE_LEFT:
                    mX1 = mX2;
                    mY1 = mY2;
                    mpCon->SelCopy(mX1, mY1, mX2, mY2, clicks);  /* start selection */
                    break;

                case MOUSE_MIDDLE:
                    mpCon->SelPaste(mTtyFd);
                    break;

                case MOUSE_RIGHT:
                    if (mButtons == 3)
                        mpCon->SelCopy(mX1, mY1, mX2, mY2, clicks);
                    else
                        mpCon->SelPaste(mTtyFd);
                    break;
            }
       default:
           break;
    } /* switch above */
    memcpy(&mMiPrev, &mi, sizeof(mMiPrev));
#endif
    return;
}
