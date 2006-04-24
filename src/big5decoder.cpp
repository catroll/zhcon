// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          big5decoder.cpp  -  description
                             -------------------
    begin                : Mon Apr 23 2001
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

#include "big5decoder.h"

BIG5Decoder::BIG5Decoder() {}

BIG5Decoder:: ~ BIG5Decoder() {}

bool BIG5Decoder::IsCode1(char c) {
    return c >= 0xa1 && c <= 0xfa;
}

bool BIG5Decoder::IsCode2(char c) {
    return c >= 40 && c <= 0xff;
}

unsigned int BIG5Decoder::Index(char c1, char c2) {
    return (c1 - 0xa1) * (0xff - 0x40) + c2 - 0x40;
}
