// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          inputserver.cpp  -  description
                             -------------------
    begin                : Sun Sep 9 2001
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

#include "inputserver.h"

InputServer::InputServer()
:mIsFullChar(false),
mIsFullComma(false),
mClientBufLen(0) {}

InputServer::~InputServer() {}

ImmInfo InputServer::GetImmInfo() {
    return mImmInfo;
}

bool InputServer::IsFullChar() {
    return mIsFullChar;
}

bool InputServer::IsFullComma() {
     return mIsFullComma;
}
void InputServer::SetClientBufLen(int len){
    mClientBufLen = len;
}
