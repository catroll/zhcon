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
 * ImmComm.h == Protocols of Communication 
 *
 ****************************************************************************/

/* command that send to Server 
   Frame defination:


  len     COMMAND            input1       input2        input3        return
 (short)  (long)
  -----------------------------------------------------------------------------
          OPEN_SERVER_IMM    ModuleName   ImmTableName  type(u_long)  handle 
          CLOSE_SERVER_IMM   handle 
          CLEAR_USER_INPUT   handle  
          ADD_KEY            handle       u_char key  
          BACKSPACE_KEY      handle
          GET_PHRASE_ITEM    handle       u_long nItem
          SET_PHRASE_ITEM    handle       PhraseItem *buf
          ADD_USER_PHRASE    handle       PhraseItem *buf
          FLUSH_USER_PHRASE  handle   
          SHAKEHAND
 */
 
#define IMM_OPEN_SERVER            0
#define IMM_CLOSE_SERVER           1

#define IMM_RESET_INPUT            2

#define IMM_KEY_PRESSED            3
#define IMM_KEY_FILTER             20
#define IMM_SET_INPUTMODE          21

#define IMM_CONFIG_INPUT_AREA      13
#define IMM_GET_INPUT_DISPLAY      14
#define IMM_GET_SELECT_DISPLAY     15

#define IMM_SET_PHRASE_ITEM        8
#define IMM_ADD_USER_PHRASE        9
#define IMM_FLUSH_USER_PHRASE      10
#define IMM_SHAKEHAND              11

/* command that send to Client 
   Frame defination:
   len    COMMAND           State     option
   --------------------------------------------------- 
          NEW_SELECTION     State     total selection 
          PHRASE_ITEM       State     PhraseItem *buf
          NEW_CONNECTION    State     
 */

#define NEW_SELECTION        0x40 
#define PHRASE_ITEM          0x41
#define NEW_CONNECTION       0x42
#define CLIENT_ID            0x43

/* error defination */
#define IMM_NORMAL           0
#define IMM_BAD_REQUEST      1
#define IMM_NO_SUCH_PHRASE   2
#define IMM_NO_MEMORY        3
#define IMM_BAD_PHRASE_FILE  4
#define IMM_ILEGAL_FILE      5
#define IMM_NO_SUCH_IMM      6
#define IMM_IMM_NOTOPEN      7


