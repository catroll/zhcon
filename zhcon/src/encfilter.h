/* vi: set sw=4 ts=4: */
/*
 * CCE - Console Chinese Environment -
 * Copyright (C) 1998-2003 Rui He (rhe@3eti.com)
 *
 * 2006-04-07: modified by ejoy <ejoy@users.sf.net> for use in zhcon to convert from/to UTF-8
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE TERRENCE R. LAMBERT BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 */

#ifndef __CCE_ENCFILTER_H__
#define __CCE_ENCFILTER_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_ICONV
#include	<iconv.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int UseEncodingFilter;
extern char EncodingFilterBuff[];
extern size_t EncodingFilterLen;  /* last converted len */

extern int  SetupEncodingFilter(const char* sysCoding); /* GB2312, GBK etc */
extern void CleanupEncodingFilter(void);	
extern int  DoEncodingFilter(int dir, char *inbuf, size_t inlen);

#define CONVERT_FROM_UTF8_FILTER  0
#define CONVERT_TO_UTF8_FILTER	 1

#ifdef __cplusplus
}
#endif

#endif /* HAVE_ICONV */
#endif /* __CCE_ENCFILTER_H__ */

