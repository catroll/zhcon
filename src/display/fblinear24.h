// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          fblinear24.h  -  description
                             -------------------
    begin                : Fri July 20 2001
    copyright            : (C) 2001 by ejoy, huyong
    email                : ejoy@users.sourceforge.net
                           ccpaging@online.sh.cn
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FBLINEAR24_H
#define FBLINEAR24_H

#include "fbdev.h"

/**
 *@author huyong
 */
class FBLinear24 : public FBDev {
    public:
        FBLinear24();

        void PutPixel(int x,int y,int color);
        void FillRect(int x1,int y1,int x2,int y2,int color);
        void RevRect(int x1,int y1,int x2,int y2);
        void DrawChar(int x,int y,int fg,int bg,struct CharBitMap* pFont);

    private:
        void store4pixels(__u32 d1, __u32 d2, __u32 d3, __u32 d4, __u32 *dest);
        void rectfill(__u8 *dest, int width, int height, __u32 data,int linesize);

    private:
        void InitColorMap();
        __u32 cfb24[16];
};
#endif

