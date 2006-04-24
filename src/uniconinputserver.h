// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          uniconinputserver.h  -  description
                             -------------------
    begin                : Mon Sep 17 2001
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

#ifndef UNICONINPUTSERVER_H
#define UNICONINPUTSERVER_H

#include <inputserver.h>
#include "ImmDefs.h"
#include "ImmClient.h"


/**
  *@author ejoy
  */

//interface for unicon
extern "C" {
    int LibOpen();
    int LibRelease();
}

class UniconInputServer : public InputServer {
    public:
        UniconInputServer();
        ~UniconInputServer();
        void SetFullComma(bool value);
        void SetFullChar(bool value);
        bool ProcessKey(char key, string & rBuf);
        bool LoadImm(ImmInfo & rModule);
        string GetServerType();
        void GetInputBuf(char * pBuf, int len);
        void GetCandilist(Candilist & rList);
        void SetClientBufLen(int len);

        static bool SetDataPath(string datapath);

    private:
        static string mDataPath;
        void SetInputMode();
        ImmServer_T mImmServer;
        IMM *mpImm;
        char mSelectBuf[128];
};

#endif
