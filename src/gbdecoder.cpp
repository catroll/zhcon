// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          gbdecoder.cpp  -  description
                             -------------------
    begin                : Thu Mar 22 2001
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

#include "gbdecoder.h"

GBDecoder::GBDecoder() {}

GBDecoder::~GBDecoder() {}

//check if c is byte1 of a gbcode
bool GBDecoder::IsCode1(char c) {
    return (c >= 0xA1 && c <= 0xF7);
}

//check if c is byte2 of a gbcode
bool GBDecoder::IsCode2(char c) {
    return (c >= 0xA1 && c <= 0xFE);
}

unsigned int GBDecoder::Index(char c1, char c2) {
    return (c1 - 0xa1) * 94 + c2 - 0xa1;
}
