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

#ifndef __PHRASE_H__
#define __PHRASE_H__

#include <stdlib.h>

typedef u_char freq_t;
typedef u_char count_t;

typedef struct __PhraseItem
{
    char    *szKeys;
    u_char  *KeyLen;
    char    *szPhrase;
    freq_t  *frequency;
} PhraseItem;

#endif

