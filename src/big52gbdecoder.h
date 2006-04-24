// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          big52gbdecoder.h  -  description
                             -------------------
    begin                : Thu Jul 19 2001
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

#ifndef BIG52GBDECODER_H
#define BIG52GBDECODER_H

#include <big5decoder.h>

/**
  *@author ejoy
  */

class BIG52GBDecoder : public BIG5Decoder {
    public:
        BIG52GBDecoder();
        ~BIG52GBDecoder();
        virtual unsigned int Index(char c1, char c2);
    private:
        static char BtoG[];
};
#endif
