// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Thu Mar 15 11:07:16 GMT-5 2001
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
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#if defined(linux)
    #include <linux/kd.h>
#elif defined(__FreeBSD__)
    //#include <machine/console.h>
    #include <sys/consio.h>
#endif
#include "zhcon.h"
#include "global.h"
using namespace std;

GraphDev *gpScreen = NULL;
HzDecoder *gpDecoder = NULL;
Zhcon *gpZhcon = NULL;

int main(int argc, char* argv[]) {
    try {
        Zhcon con(argc, argv);
        gpZhcon = &con;
        con.Init();
        con.Run();
    } catch (runtime_error & e) {
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "unknown exception caught" << endl;
    }

    return EXIT_SUCCESS;
}

//do some clean up when serious signal caught
void SignalHandle(int signo) {
    Zhcon::mState = Zhcon::TERMINATE;
}

void SignalChild(int signo) {
    int status;
    if (waitpid(Zhcon::mTtyPid, &status, WNOHANG))
        Zhcon::mState = Zhcon::TERMINATE;
}

bool gIsBeepOn = false;
void Beep(void) {
    if (gIsBeepOn)
        ioctl(0, KDMKTONE, 0x200637);
}
void Beep(bool onoff) {
    gIsBeepOn = onoff;
}
