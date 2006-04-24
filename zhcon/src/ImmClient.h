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
 *       ImmClient.h  ====    Method Modules Interface 
 *
 ***************************************************************************/
#ifndef __IMMCLIENT_H__
#define __IMMCLIENT_H__

#include <stdlib.h>
#include <Phrase.h>
#include <ImmDefs.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define MAX_PHRASE_BUFFER     256
typedef long IMM_HANDLE;
typedef long ImmServer_T;

typedef struct __IMM_STRUCTURE__
{
    IMM_HANDLE handle;             /* Client Handle */
    /* Tempory Buffer for commnunication between 
       Server and Client */
    ImmServer_T pCImmServer;       /* Server Handler */
    char szMethod[32];
} IMM;

/* type defination */
#define CCE                    0
#define TLC                    1

/* Module initialize and release */
/* return ImmServer handle */
ImmServer_T IMM_OpenClient (char *szIpAddr, u_short port);
int IMM_CloseClient (ImmServer_T ImmServer);

/* Phrase Table File Operation */
IMM *IMM_OpenInput (ImmServer_T ImmServer, 
                    char *szImmModule, 
                    char *szImmTable, 
                    u_long type);
int  IMM_CloseInput (IMM *p);

/* Input Method Operations */
int  IMM_KeyFilter (IMM *p,          // return value:
                    u_char key,      // 2 -- have filtered and translated
                    char *buf,       // 1 -- have filtered 
                    int *len);       // 0 -- not filtered
                                     // < 0 -- error code

int  IMM_ResetInput (IMM *p);        /* 0 -- fail, 1 -- success */
int  IMM_SetInputMode (IMM *p,       /* 0 -- fail, 1 -- success */
                       long mode);   // see ImmDefs.h  

/* Input Area Configuration & Operation */
int  IMM_ConfigInputArea (IMM *pImm, /* Set the Client's Selection Length */
                          int SelectionLen); 
int  IMM_GetInputDisplay (IMM *pImm, /* Get Inputs from Server */ 
                          char *buf, 
                          long buflen);      
int  IMM_GetSelectDisplay (IMM *pImm, /* Get Selections from Server */
                           char *buf, 
                           long buflen);

/* User Phrase Interface */
int  IMM_AddUserPhrase (IMM *pImm,    /* Add a new phrase to server */
                        char *szCode,
                        char *szPhrase,
                        u_long freq);
/* Add a new phrase to server */
int  IMM_FlushUserPhrase (IMM *pImm); /* Flush server so that the   */
                                      /* new phrase will take effect */
int  IMM_ChangePhraseItem(IMM *pImm, /* change a phrase of server */
                        u_long n,
                        char *szCode,
                        char *szPhrase,
                        u_long freq);

#ifdef  __cplusplus
}
#endif

#endif  /* IMMCLIENT_H__ */

