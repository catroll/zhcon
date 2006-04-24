// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          gb2big5decoder.h  -  description
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

#ifndef GB2BIG5DECODER_H
#define GB2BIG5DECODER_H

#include <gbdecoder.h>

/**
  *@author ejoy
  */

class GB2BIG5Decoder : public GBDecoder  {
    public:
        GB2BIG5Decoder();
        ~GB2BIG5Decoder();
        virtual unsigned int Index(char c1, char c2);
    private:
        static char GtoB[];
};

#endif
