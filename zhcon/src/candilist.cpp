// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          candilist.cpp  -  description
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

#include "candilist.h"

Candilist::Candilist(){
    Reset();
}
Candilist::~Candilist(){
}

void Candilist::Reset(){
    mCount = 0;
    mHaveNext = mHavePrev = false;
}

