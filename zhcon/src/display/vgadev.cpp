// vi:ts=4:shiftwidth=4:expandtab
/***************************************************************************
                          vgadev.cpp  -  description
                             -------------------
    begin                : Wed Sept 05 2001
    copyright            : (C) 2001 by rick, huyong
    email                : rick@chinaren.com
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef USING_VGA
#if defined(linux)

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/io.h>
#include <cassert>
#include "debug.h"
#include "vgadev.h"
#include "lrmi.h"

#define GRAPH_BASE 0xA0000
#define GRAPH_ADDR  0x3ce
#define GRAPH_DATA  0x3cf

char *VGADev::mpBuf = NULL;
int VGADev::mBufLen = 0;
unsigned long VGADev::mNextLine = 0; // offset to one line below
int VGADev::mFd = -1;

bool VGADev::TryOpen() {
    if (!LRMI_init()) {
        throw (runtime_error("LRMI_init() failed.\n"));
        return false;
    }

    EnableIOPerm();
    if (!SetVideoMode(0x12)) {
        DisableIOPerm();
        throw (runtime_error("LRMI_int() failed, can't change video mode.\n"));
        return false;
    }

    if ((mFd = open("/dev/mem", O_RDWR)) < 0) {
        DisableIOPerm();
        throw(runtime_error("Cannot open /dev/mem.\n"));
        return false;
    }

    mXres = 640;
    mYres = 480;
    mBufLen = mXres * mYres / 8;
    mpBuf = static_cast<char *>(mmap((__ptr_t)0, mBufLen, PROT_READ | PROT_WRITE, MAP_SHARED,
                                mFd, GRAPH_BASE));
    if (mpBuf == MAP_FAILED) {
        DisableIOPerm();
        throw(runtime_error("mmap() failed!"));
    }

    mpGraphDev = new VGADev;
    return true;
}

VGADev::VGADev() {
    mNextLine = mXres >> 3;
    EnableIOPerm();
    SetDefaultMode();
}

VGADev::~VGADev() {
    if (mpBuf != NULL)
        munmap(mpBuf, mBufLen);
    if (mFd >= 0)
        close(mFd);
    SetVideoMode(0x3);
    DisableIOPerm();
}

void VGADev::EnableIOPerm()
{
    ioperm(GRAPH_ADDR, 1, 1);
    ioperm(GRAPH_DATA, 1, 1);
    ioperm(0, 0x400, 1);
    iopl(3);
}

void VGADev::DisableIOPerm()
{
    ioperm(GRAPH_ADDR, 1, 0);
    ioperm(GRAPH_DATA, 1, 0);
    ioperm(0, 0x400, 0);
    iopl(3);
}

inline bool VGADev::SetVideoMode(int mode)
{
    struct LRMI_regs r;
    memset(&r, 0, sizeof(r));
    r.eax = mode;
    return LRMI_int(0x10, &r);
}

inline void VGADev::SetDefaultMode() {
    SetWriteMode(3);
    SetOper(0);
    SetBitMask(0xff);
}

inline void VGADev::SetBitMask(char mask) {
    outb(9, GRAPH_ADDR);
    outb(mask, GRAPH_DATA);
}

inline void VGADev::SetOper(int op) {
    outb(3, GRAPH_ADDR);
    outb(op, GRAPH_DATA);
}

inline void VGADev::SetWriteMode(int mode) {
    outb(5, GRAPH_ADDR);
    outb(mode, GRAPH_DATA);
}

inline void VGADev::SetColor(int color) {
    outb(0, GRAPH_ADDR);
    outb(color, GRAPH_DATA);
}

void VGADev::PutPixel(int x,int y,int color) {
    static int prevColor = -1;
    if (prevColor != color) {
        prevColor = color;
        SetColor(color);
    }

    __u8* dest = (__u8*)mpBuf + mNextLine * y + (x>>3);
    fb_readb(dest);
    *dest = 0x80 >> (x & 7);
}

void VGADev::FillRect(int x1,int y1,int x2,int y2,int color) {
    assert( x1 >= 0 && x1 < mXres && y1 >=0 && y1 < mYres);
    assert( x2 >= 0 && x2 < mXres && y2 >=0 && y2 < mYres);
    assert(x1 <= x2 && y1 <= y2);

#ifndef NDEBUG
    //debug<<"FillRect ("<<x1<<","<<y1<<")-("<<x2<<","<<y2<<")"<<endl;
#endif
    SetColor(color);

    __u8* destorig = (__u8*)mpBuf + mNextLine * y1 + (x1>>3);
    __u8* dest = destorig;

    int width = x2 - x1 + 1;
    int left, right, midbytes;
    left = 8 - (x1 & 7);
    // left maybe 8
    if (left == 8) left = 0;

    if (left >= width) {
        right = 0;
        midbytes = 0;
    } else {
        right = (x1 + width) & 7;
        midbytes = (width - left - right) / 8;
    }
#ifndef NDEBUG
    if (left + right > width || midbytes < 0) {
        debug<<"FillRect x1="<<x1<<" width="<<width<<" left="<<left;
        debug<<" midbytes="<<midbytes<<" right="<<right<<endl;
    }
#endif
    
    int height;
    if (left) {
        __u8 leftmask = 0xff >> (8 - left);
        if (left > width) leftmask &= 0xff << (left - width);
        for (height = y2 - y1 + 1; height--; dest += mNextLine) {
            fb_readb(dest); // fill latches
            *dest = leftmask;
        }
        destorig++;
    }
    if (midbytes) {
        dest = destorig;
        for (height = y2 - y1 + 1; height--; dest += mNextLine) {
            memset(dest, 0xff, midbytes);
        }
        destorig += midbytes;
    }
    if (right) {
        dest = destorig;
        __u8 rightmask = 0xff << (8 - right);
        for (height = y2 - y1 + 1; height--; dest += mNextLine) {
            fb_readb(dest); // fill latches
            *dest = rightmask;
        }
    }
}

void VGADev::RevRect(int x1,int y1,int x2,int y2) {
    assert( x1 >= 0 && x1 < Width() && y1 >=0 && y1 < Height());
    assert( x2 >= 0 && x2 < Width() && y2 >=0 && y2 < Height());
    assert(x1 <= x2 && y1 <= y2);

    __u8* dest = (__u8*)mpBuf + mNextLine * y1 + (x1>>3);
    __u8* dest8;

    SetWriteMode(3);
    SetOper(0x18);
    SetColor(0xf);
    SetBitMask(0xff);
    
    int width = x2 - x1 + 1;
    int left, right, midbytes;
    left = 8 - (x1 & 7);
    // left maybe 8
    if (left == 8) left = 0;

    if (left >= width) {
        right = 0;
        midbytes = 0;
    } else {
        right = (x1 + width) & 7;
        midbytes = (width - left - right) / 8;
    }

    __u8 leftmask = 0xff >> (8 - left);
    if (left > width) leftmask &= 0xff << (left - width);
    __u8 rightmask = 0xff << (8 - right);
    int cnt;
    for(int height = y2 - y1 + 1; height--; dest += mNextLine) {
        dest8 = dest;
        if (left) {
            fb_readb(dest8);
            fb_writeb(leftmask, dest8++);
        }
        if (midbytes) {
            for(cnt = midbytes; cnt--; ) {
                fb_readb(dest8);
                fb_writeb(0xff, dest8++);
            }
        }
        if (right) {
            fb_readb(dest8);
            fb_writeb(rightmask, dest8);
        }
    }
    SetDefaultMode();
}

void VGADev::DrawChar(int x,int y,int fg,int bg,struct CharBitMap* pFont) {
    __u8* destorig = (__u8*)mpBuf + mNextLine * y + (x >> 3);
    __u8* dest;
    __u8* dest8;

    int LeftShift = x & 7;
    int FillBytes = (LeftShift + pFont->w + 7) >> 3;
    // maxium 24 pixel width font
    if (FillBytes > 3) FillBytes = 3;

    __u8* cdat8;
    int rows, cnt;
    union {
        char ch[4];
        __u32 l;
    } UniLine;

    UniLine.l = (0xffffffff >> LeftShift)
                & (0xffffffff << (32 - LeftShift - pFont->w));

    SetColor(bg);
    dest = destorig;
    for (rows = mBlockHeight; rows-- ; dest += mNextLine) {
        dest8 = dest;
        cdat8 = (__u8*)&UniLine.ch[3];
        for (cnt = FillBytes; cnt--;) {
            fb_readb(dest8);
            fb_writeb(*cdat8--, dest8++);
        }
    }
        
    SetColor(fg);
    dest = destorig;
    char *pFontLine = pFont->pBuf;
    if (LeftShift == 0) {
        for (rows = mBlockHeight; rows-- ; dest += mNextLine) {
            dest8 = dest;
            UniLine.l = 0;
            memcpy(&UniLine, pFontLine, pFont->wBytes);
            cdat8 = (__u8*)&UniLine;
            for (cnt = FillBytes; cnt--;) {
                fb_readb(dest8);
                fb_writeb(*cdat8++, dest8++);
            }
            pFontLine += pFont->wBytes;
        }
        return;
    }

    for (rows = mBlockHeight; rows-- ; dest += mNextLine) {
        dest8 = dest;

        // for little endian
        UniLine.l = 0;
        cdat8 = (__u8*)pFontLine;
        for (cnt = 0; cnt < pFont->wBytes; cnt++) {
            UniLine.ch[3 - cnt] = *cdat8++;
        }
        UniLine.l >>= LeftShift;
        cdat8 = (__u8*)&UniLine.ch[3];
        for (cnt = FillBytes; cnt--;) {
             fb_readb(dest8);
             fb_writeb(*cdat8--, dest8++);
        }
        pFontLine += pFont->wBytes;
    }
}

void VGADev::SwitchToGraph() {
    SetVideoMode(0x12);
    SetDefaultMode();
}

void VGADev::SwitchToText() {
    SetVideoMode(0x3);
}
#endif

#endif //USING_VGA
