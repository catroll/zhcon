// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          kscdecoder.h  -  description
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

#ifndef KSCDECODER_H
#define KSCDECODER_H

#include <hzdecoder.h>

/**
 *@author ejoy
 */

class KSCDecoder : public HzDecoder {
    public:
        KSCDecoder();
        ~KSCDecoder();

        virtual bool IsCode2(char c);
        virtual bool IsCode1(char c);
        virtual unsigned int Index(char c1,char c2);
};
#endif
