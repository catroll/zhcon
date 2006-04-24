// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          jisdecoder.cpp  -  description
                             -------------------
    begin                : Wed May 16 2001
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

#include "jisdecoder.h"

JISDecoder::JISDecoder() {}

JISDecoder::~JISDecoder() {}

bool JISDecoder::IsCode1(char c) {
    if (c < 0x80)
        return false;
    c = c & 0x7f;
    return (c >= 0x21 && c <= 0x74);
}

bool JISDecoder::IsCode2(char c) {
    if (c < 0x80)
        return false;
    c = c & 0x7f;
    return (c >= 21 && c <= 0x7e);
}

unsigned int JISDecoder::Index(char left, char right) {
    left = left & 0x7f;
    right = right & 0x7f;
    return ((left - 0x21) * (0x7e - 0x21 + 1) + right - 0x21);
}
