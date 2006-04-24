// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          candilist.h  -  description
                             -------------------
    begin                : Thu Sep 13 2001
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

#ifndef CANDILIST_H
#define CANDILIST_H


/**
  *@author ejoy
  */
#include <string>
using namespace std;

struct Candilist {
public:
    Candilist();
    ~Candilist();
    void Reset();

    struct {
        string mText;
        char mKey;
    }
    mList[10];
    int mCount;
    bool mHavePrev;
    bool mHaveNext;
};

#endif
