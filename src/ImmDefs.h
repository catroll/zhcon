// vi:ts=4:shiftwidth=4:expandtab
/*
 *
 * UNICON - The Console Chinese & I18N
 * Copyright (c) 1999-2000
 *
 * This file is part of UNICON, a console Chinese & I18N
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * See the file COPYING directory of this archive
 * Author: see CREDITS
 */

#ifndef __IMM_DEF_H__
#define __IMM_DEF_H__

/* Define Input Mode */
#define IMM_DOUBLE_BYTE_MODE          0x1
#define IMM_SINGLE_BYTE_MODE          0x0

#define IMM_FULL_SYMBOL_MODE          0x2
#define IMM_HALF_SYMBOL_MODE          0x0

#define IMM_FULL_CHAR_MODE            0x4
#define IMM_HALF_CHAR_MODE            0x0

#define IMM_FULL_ASCII_MODE           0x80  /* Combine all */

/* define support module type */
#define IMM_CCE                       1
#define IMM_FREE_PINYIN               2
#define IMM_TLC                       3
#define IMM_XCIN                      4
#define IMM_UNKNOWN                   0xffffff

/* define local language code support */
#define IMM_LC_GB2312                 1
#define IMM_LC_JISX0208               2
#define IMM_LC_KSC5601                3
#define IMM_LC_JISX0212               4
#define IMM_LC_BIG5                   5
#define IMM_LC_GBK                    6
#define IMM_LC_ALL                    0xff

/* define the const for the Phrase that user made */ 
#define USER_PHRASE_FREQ              1024

#endif

