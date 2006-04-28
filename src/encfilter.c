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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "encfilter.h"

#ifdef HAVE_ICONV

#ifndef ICONV_CONST
#error ICONV_CONST not defined
#endif /* ICONV_CONST */

#define BUFSIZ 8192
int UseEncodingFilter;  /* use filter such as UTF-8 */
char EncodingFilterBuff[BUFSIZ];   /* global buffer used by ConsoleHandler() and TextCopy() */
size_t EncodingFilterLen; 

static iconv_t FilterCD[2]; /* 0 is from filter to target, 1 is reverse */
 
void CleanupEncodingFilter(void)
{
	if (FilterCD[0] && FilterCD[0] != (iconv_t)-1) iconv_close(FilterCD[0]);
	if (FilterCD[1] && FilterCD[1] != (iconv_t)-1) iconv_close(FilterCD[1]);
	FilterCD[0] = FilterCD[1] = NULL;
}

int SetupEncodingFilter(const char* sysCoding)
{
    assert(UseEncodingFilter);

/*    printf("setting utf8 filter for %s\n",  sysCoding);*/

	CleanupEncodingFilter(); /* close old ones */

	UseEncodingFilter = 0;

	if ((FilterCD[CONVERT_FROM_UTF8_FILTER] = iconv_open(sysCoding, "UTF-8")) == (iconv_t)-1)
	{
		fprintf(stderr, "Failed to do iconv_open(%s,%s), filter disabled.\r\n", sysCoding, "UTF-8");
		return -1;
	}
	
	if ((FilterCD[CONVERT_TO_UTF8_FILTER] = iconv_open("UTF-8", sysCoding)) == (iconv_t)-1)
	{
		iconv_close(FilterCD[0]);
		FilterCD[0] = NULL;
		return -1;
	}
	
/*    fprintf(stderr, "Using encoding filter %s for %s.\r\n", "UTF-8", sysCoding);*/

	UseEncodingFilter = 1;
	return 0;
}

/* return the number of bytes remained in inbuf */
int DoEncodingFilter(int dir, char *inbuf, size_t inlen)
{
	size_t ret;
	ICONV_CONST char *oldinbuf = (ICONV_CONST char*)inbuf;
	char *outbuf = EncodingFilterBuff;

	if (!inbuf || inlen <= 0) return 0;

	EncodingFilterLen = sizeof(EncodingFilterBuff);

	ret = iconv(FilterCD[dir], &inbuf, &inlen, &outbuf, &EncodingFilterLen);

	EncodingFilterLen = sizeof(EncodingFilterBuff) - EncodingFilterLen; /* converted  */

	if (ret == -1)  
	{
		switch(errno)
		{
		case EINVAL:  /* incomplete, inbuf is pointing to incomplete buffer */
		case E2BIG:  /* output room is not enough */
			memcpy(oldinbuf, inbuf, inlen);  /* save them to the beginning */
			return inlen;
			
		case EILSEQ:  /* illegal sequence */
		default:
			return 0; 
		}
	}

	return 0; /* inlen should be 0 since everything is converted */
}

#endif /* HAVE_ICONV */

