// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          nativebarclient.h  -  description
                             -------------------
    begin                : Tue Oct 16 2001
    copyright            : (C) 2001 by huyong
    email                : ccpaging@online.sh.cn
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NATIVEBARLIENT_H
#define NATIVEBARCLIENT_H

#include <inputclient.h>
#include <window.h>

class Candilist;

class NativeBarClient : public Window,public InputClient {
    public:
        NativeBarClient(string& sColors);
        ~NativeBarClient();
        void Update();
        void Hide();
        void Show();
        void VtSizeDelta(int &ColDelta, int &RowDelta);
        void Connect(InputServer *pServer);
    private:
        int mOveredRows;
        string mTitle;
        string mKey;
        int mShownWords;
        void PutTitle();
        void PutKey();
        void PutList();
        void Draw();
  string GetPromptStr();
        Candilist mList;
};

#endif
