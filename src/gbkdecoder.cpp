// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          gbkdecoder.cpp  -  description
                             -------------------
    begin                : Sun Apr 22 2001
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

#include "gbkdecoder.h"

GBKDecoder::GBKDecoder() {}

GBKDecoder::~GBKDecoder() {}

bool GBKDecoder::IsCode1(char c) {
    return c >= 0x81 && c <= 0xfe;
}

bool GBKDecoder::IsCode2(char c) {
    return c >= 0x40 && c <= 0xff;
}

unsigned int GBKDecoder::Index(char c1, char c2) {
    int n;
    n = (c1 - 0x81) * 192;
    if (c2 <= 0xff && c2 >= 0x40)
        n += (c2 - 0x40);
    return n;
}
