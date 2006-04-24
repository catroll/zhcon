// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          nativebarclient.cpp  -  description
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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "global.h"
#include "debug.h"
#include "console.h"
#include "candilist.h"
#include "inputserver.h"
#include "nativebarclient.h"
#include "zhcon.h"

NativeBarClient::NativeBarClient(string& sColors)
: Window(0, 0, mpCon->MaxCols() * gpScreen->BlockWidth() - 1,
               gpScreen->BlockHeight() + 4 - 1,
               WS_CHILD | WS_FRAMETHIN),
InputClient() {
    SetColor(sColors);
    SetFgColor(InputClient::mFgColor);
    SetBgColor(InputClient::mBgColor);
    SetFrameColor(InputClient::mPixLight, InputClient::mPixDark);

    int BottomSpc;
    BottomSpc = gpScreen->Height() % gpScreen->BlockHeight();
    if ( BottomSpc > 4)
        mOveredRows = 1;
    else
        mOveredRows = 2;
    MoveTo(0, gpScreen->Height() - Height());
    /* inner space cleared by Console::VtSizeDelta
    int InnerSpc;
    InnerSpc = GetY1() % gpScreen->BlockHeight();
    if (InnerSpc > 0) {
        gpScreen->FillRect(0, GetY1() - InnerSpc,
                           gpScreen->Width() - 1, GetY1() - 1,
                           0);
    }
    */
}

NativeBarClient::~NativeBarClient() {}


void NativeBarClient::Show() {
    Draw();
    InputClient::mVisible = true;
}

void NativeBarClient::Hide() {
    Window::Hide();
    int BottomSpc;
    BottomSpc = gpScreen->Height() % gpScreen->BlockHeight();
    if (BottomSpc > 0) {
        gpScreen->FillRect(0, gpScreen->Height() - BottomSpc - 1,
                           gpScreen->Width() - 1, gpScreen->Height() - 1,
                           0);
    }
    InputClient::mVisible = false;
}

//update window's content
//only update display when visible
void NativeBarClient::Update() {
    if (mpServer == NULL) {
        //prompt mode
        mTitle = GetPromptStr();
        mKey = "";
        mList.Reset();
    } else {
        mpServer->GetCandilist(mList);

//        mTitle = _("[");
//        mTitle += (mpServer->IsFullChar() ? _("Full") : _("Half"));
//        mTitle += (mpServer->IsFullComma() ? _("Ch") : _("En"));
//        mTitle += _("]") +
        mTitle = (mpServer->GetImmInfo()).mName;
        mTitle += " ";

        char buf[64];
        buf[0] = '\0';
        mpServer->GetInputBuf(buf, 63);
        mKey = buf;
        mKey.resize(12, ' ');
        mKey += " ";
    }
    if (!InputClient::mVisible) return ;

    Draw();
}

void NativeBarClient::PutTitle() {
    PutStr(0, 0, mTitle, InputClient::mColor1);
}

void NativeBarClient::PutKey() {
    if (mTitle.size() >= (unsigned)mMaxCols) return;
    PutStr(mTitle.size(), 0, mKey, InputClient::mColor1);
}

void NativeBarClient::PutList() {
    mShownWords = 0;
    int col = mTitle.size() + mKey.size();
    int lvlen = mMaxCols - col;
    string str;
    char key;
    if (mList.mCount) {
        //draw candilist
        for (int i = 0; i < mList.mCount; i++) {
            lvlen -= 2 + mList.mList[i].mText.size();
            key = mList.mList[i].mKey;
            if (key != '\0')
                lvlen--;
            if (lvlen < 0)
                break;

            str = ((i == 9) ? '0' : '1' + i);
            str += '.';
            PutStr(col, 0, str, mColor2);
            col += 2;

            PutStr(col, 0, mList.mList[i].mText);
            col += mList.mList[i].mText.size();

            if (key != '\0') {
                str = key;
                PutStr(col, 0, str, mColor1);
                col++;
            }
            mShownWords++;

            if (lvlen == 0)
                break;

            PutStr(col, 0, " ");
            col += 1;
            lvlen--;
        }
    }
    if (col < mMaxCols) {
        str.assign(mMaxCols - col, ' ');
        PutStr(col, 0, str);
    }
}

void NativeBarClient::Draw() {
    PutTitle();
    PutKey();
    PutList();

    if (!Window::Visible())
        Window::Show();
    else
        Window::Redraw();
}

void NativeBarClient::VtSizeDelta(int &ColDelta, int &RowDelta) {
    ColDelta = 0;
    RowDelta = mOveredRows;
}

void NativeBarClient::Connect(InputServer *pServer) {
    mpServer = pServer;
    pServer->SetClientBufLen(mMaxCols - 40);
}

string NativeBarClient::GetPromptStr() {
    string encode = gpZhcon->GetEncode();
    string s;
    s = "  ";
    s += PACKAGE;
    s += " ";
    s += VERSION;
    s.resize(mMaxCols - encode.size() - 4, ' ');
    s += _("[");
    s += encode;
    s += _("]");
    return s;
}
