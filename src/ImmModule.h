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

/*****************************************************************************
 *
 *       ImmModule.h  ====    Defination of Method Modules Interface 
 *
 ***************************************************************************/
#ifndef __IMMMODULE_H__
#define __IMMMODULE_H__

#include <stdlib.h>
#include <Phrase.h>
#include <ImmDefs.h>

typedef struct  __IMM_CLIENT__
{
    /* ClientData for recording states */
    void *pImmClientData;

    /* Buffer for the PhraseItem */
    PhraseItem m;
    char buf[512];

} IMM_CLIENT;

struct ImmOperation
{
    char     *name;
    char     *version;
    char     *comments;
    u_long   type;               /* eg. IMM_BIG5 << 24 | IMM_CCE */

    /* File I/O Operation */
    IMM_CLIENT *(*open) (char *szFileName, long type);
    int (*save)  (IMM_CLIENT *p, char *szFileName);
    int (*close) (IMM_CLIENT *p);

    /* Indepent Modules support */
    int (*KeyFilter) (IMM_CLIENT *p, u_char key, char *buf, int *len);
    int (*ResetInput) (IMM_CLIENT *p);

    /* Input Area Configuration & Operation */
    int (*ConfigInputArea) (IMM_CLIENT *pImm, int SelectionLen);
    int (*GetInputDisplay) (IMM_CLIENT *pImm, char *buf, long buflen);
    int (*GetSelectDisplay) (IMM_CLIENT *pImm, char *buf, long buflen);
    
    /* Phrase Operation */
    PhraseItem * (*pGetItem) (IMM_CLIENT *p, u_long n);
    int    (*AddPhrase) (IMM_CLIENT *pClient, PhraseItem *p);
    int    (*ModifyPhraseItem) (IMM_CLIENT *p, long n, PhraseItem *pItem);
    int    (*Flush) (IMM_CLIENT *p);
};

#endif

