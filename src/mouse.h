// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          mouse.h  -  description
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

#ifndef MOUSE_H
#define MOUSE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_GPM_H
    #include <gpm.h>
#elif defined(__FreeBSD__)
    #if __FreeBSD__ < 5
        #include <machine/mouse.h>
    #else
        #include <sys/mouse.h>
    #endif

    #include <sys/consio.h>
    //#include <machine/console.h>
    #include <sys/time.h>
#endif

#include "console.h"

class Mouse {
    public:
#if defined(__FreeBSD__)		
		static void SignalProcess(int signo);
		static Mouse* pMouse;
#endif		
        Mouse();
        ~Mouse();

        bool Open(Console* pCon, int confd, int ttyno, int ttyfd);
        void Close();
        bool IsOpen() {
            if (mFd < 0) return false;
            return true;
        }
        int  GetButtons() {
            return mButtons;
        }
        void Process();

    public:
        int mFd;

    private:
#ifdef HAVE_GPM_H
        Gpm_Connect mConn;
#elif defined(__FreeBSD__)
        struct mouse_info mMiPrev;
        int mClicks;
        struct timeval mTV1;
        struct timeval mTV2;
#endif
        int mButtons;
        Console *mpCon;
        int mConFd;  // FreeBSD ioctl
        int mTtyFd;  // for paste
        
        int mX1, mY1, mX2, mY2;
};

#endif
