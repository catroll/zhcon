// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          gbdecoder.h  -  description
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

#ifndef GBDECODER_H
#define GBDECODER_H

#include <hzdecoder.h>

/**
 *@author ejoy
 */

class GBDecoder : public HzDecoder {
    public:
        GBDecoder();
        ~GBDecoder();

        virtual bool IsCode2(char c);
        virtual bool IsCode1(char c);
        virtual unsigned int Index(char c1,char c2);
};
#endif
