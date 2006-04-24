// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          configserver.h  -  description
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

#ifndef CONFIGSERVER_H
#define CONFIGSERVER_H

#include <inputserver.h>

/**Provide system menu through InputServer interface.
  *@author ejoy
  */

class ConfigServer : public InputServer {
    public:
        ConfigServer();
        ~ConfigServer();
        string GetServerType();
        void GetInputBuf(char * pBuf, int len);
        void GetCandilist(Candilist & rList);
        bool ProcessKey(char key, string & rBuf);  // return false to quit
                                                   // special for ConfigServer
        void SetFullComma(bool value) {}
        void SetFullChar(bool value) {}
        bool LoadImm(ImmInfo & rModule);
    private:
        void MenuQuit();
        void MenuHandleEncode();
        void MenuHandleDetectEncode();
        void MenuHandleSetAutoEncodeGB();
        void MenuHandleSetAutoEncodeBIG5();
        void MenuHandleSetAutoEncodeMANUAL();
        void MenuHandleSetAutoEncodeAUTO();
        void MenuHandleSetEncodeGB2312();
        void MenuHandleSetEncodeGBK();
        void MenuHandleSetEncodeBIG5();
        void MenuHandleSetEncodeJIS();
        void MenuHandleSetEncodeKSC();
        void MenuHandleGotoSysMenu();
        void MenuHandleIme();
        string GetTextEncode();
        string GetTextDetectEncode();
        void MenuHandleInputEncode();
        void MenuHandleAutoSelectUnique();
        string GetTextAutoSelectUnique();
        string GetTextInputEncode();

        struct MenuItem {
            char *mpText;
            string(ConfigServer:: *mpGetText) ();
            void (ConfigServer:: *mpFun) ();
        };
        struct MenuItem *mpCurMenu;
        static MenuItem mSysMenu[];
        static MenuItem mDetectEncodeMenu[];
        static MenuItem mImeMenu[];
        static MenuItem mEncodeMenu[];
};
#endif
