/**
 **************************************************************************
 * \file utf8.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief This code implements UTF8<->UCS2 recode functions.
 *
 * \b Project Standart Libraries
 * \author StasS
 * \date 17.02.04
 *
 * $Revision: 1 $
 *
 * $History: utf8.cpp $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/clib
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 13.04.07   Time: 12:43
 * Updated in $/VS2005/std/clib
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 3.04.07    Time: 15:12
 * Updated in $/VS2005/std/clib
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 3.04.07    Time: 15:08
 * Updated in $/VS2005/std/clib
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 3.04.07    Time: 12:31
 * Updated in $/VS2005/std/clib
 * fixed utf8 length safety
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/clib
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 30.03.06   Time: 17:12
 * Updated in $/VS/std/clib
 * added switchable AB to reg storage
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 30.12.04   Time: 14:12
 * Updated in $/VS/std/clib
 * files headers
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "utf8_old.h"
#include "std-generic/attributes.h"

#include <cstdlib>
#include <cstring>
#include <cwchar>

/**
 **************************************************************************
 * \brief calculates length of UTF8 string in characters
 ****************************************************************************/
size_t  VS_UTF8_CharLength(const char* ustr_in)
{
	const unsigned char* ustr=(const unsigned char*)ustr_in;

	if(ustr==NULL)
    return 0;

	size_t len=0;
	int shift;
// if this is wrong string,
	while(*ustr)
	{
		if (*ustr < 0x80)
		  shift=1;
		else if (*ustr < 0xC0)
		  return 0; //error
		else if (*ustr < 0xE0)
		{
			if(!ustr[1]) return 0;
			shift=2;
		}
		else if (*ustr < 0xF0)
		{
			if(!ustr[1] || !ustr[2]) return 0;
			shift=3;
		}
		else if (*ustr < 0xF8)
		{
			if(!ustr[1] || !ustr[2] || !ustr[3]) return 0;
			shift=4;
		}
		else
			return 0;

		ustr+=shift;
		len++;
	};

	return len;
};

/**
 **************************************************************************
 * \brief calculates length of UTF8 string in bytes w/o terminating null
 ****************************************************************************/
size_t  VS_Wide_UTF8Length(const wchar_t* wstr)
{
  if(wstr==0)
    return 0;

  size_t len=0;
  int shift;

  while(*wstr)
  {
    if (*wstr < 0x80)
      shift=1;
    else if (*wstr < 0x800)
      shift=2;
    else
      shift=3;

    wstr++;
    len+=shift;
  };

  return len;
};

static unsigned long mask[4]=
{ 0x00,0x3080,0xC2080,0x3082080 };

size_t VS_UTF8ToUCS(const char* ustr_in,wchar_t* wstr,size_t maxlen)
{
	const unsigned char* ustr=(const unsigned char*)ustr_in;

	if(ustr==NULL || wstr==NULL || maxlen==0 )
    return 0;

	size_t len=0;
	wchar_t wch;
	int shift;

	while(*ustr && len+1<maxlen)
	{
		if (*ustr < 0x80)
		  shift=1;
		else if (*ustr < 0xC0)
		{ wstr[0]=0;  return 0; }
		else if (*ustr < 0xE0)
		  shift=2;
		else if (*ustr < 0xF0)
		  shift=3;
		else if (*ustr < 0xF8)
		  shift=4;
		else
		{ wstr[0]=0;  return 0; }

		wch=0;
		switch (shift)
		{ // !fallback switch
		case 4:
		if(!*ustr) goto invalid_string;
		  wch+= *ustr++;
		  wch<<= 6;
			VS_FALLTHROUGH;
		case 3:
		if(!*ustr) goto invalid_string;
		  wch+= *ustr++;
		  wch<<= 6;
			VS_FALLTHROUGH;
		case 2:
		if(!*ustr) goto invalid_string;
		  wch+= *ustr++;
		  wch<<= 6;
			VS_FALLTHROUGH;
		case 1:
		if(!*ustr) goto invalid_string;
				wch += *ustr++;
		}

		wch-=(wchar_t)mask[shift-1];
		*wstr=wch;

		wstr++;
		len++;
	};

invalid_string:
	*wstr=0;
	return len;
};


/**
 **************************************************************************
 * \brief with memory allocation - string should be freed later!
 ****************************************************************************/
wchar_t* VS_UTF8ToUCS(const char* ustr)
{
	if (!ustr)
		return NULL;
	if (!*ustr)
	{
		wchar_t* wstr=(wchar_t*)malloc(sizeof(wchar_t));
		*wstr = 0;
		return wstr;
	}

	size_t len=VS_UTF8_CharLength(ustr);
	if(len==0)
		return NULL;
	len++; //add terminating 0
	wchar_t* wstr=(wchar_t*)malloc(len*sizeof(wchar_t));
	if(VS_UTF8ToUCS(ustr,wstr,len)>0)
		return wstr;

	free(wstr);
	return NULL;
};

static unsigned char prefix[4]=
{ 0x00,0xC0,0xE0,0xF0 };

size_t VS_UCSToUTF8(const wchar_t* wstr,char* ustr_in,size_t maxlen)
{
	unsigned char* ustr=(unsigned char*)ustr_in;

  if(ustr==NULL || wstr==NULL)
    return 0;

  size_t len=0;
  wchar_t wch;
  int shift;

  while(*wstr && len+1<maxlen)
  {
		wch=*wstr;

    if (wch < 0x80)
      shift=1;
    else if (wch < 0x800)
      shift=2;
    else
      shift=3;

	if(len+shift>=maxlen)
		break;

	ustr+=shift;

    switch (shift)
    { // !fallback switch
    case 4:
      *--ustr=(wch|0x80) & 0xBF;
      wch>>=6;
	  VS_FALLTHROUGH;
    case 3:
      *--ustr=(wch|0x80) & 0xBF;
      wch>>=6;
	  VS_FALLTHROUGH;
    case 2:
      *--ustr=(wch|0x80) & 0xBF;
      wch>>=6;
	  VS_FALLTHROUGH;
    case 1:
      *--ustr=wch|prefix[shift-1];
    }

    ustr+=shift;
    wstr++;
    len+=shift;
  };

  *ustr=0;
	return len;
};

/**
 **************************************************************************
 * \brief with memory allocation - string should be freed later!
 ****************************************************************************/
char* VS_UCSToUTF8(const wchar_t* wstr)
{
  size_t len=VS_Wide_UTF8Length(wstr);
  if(len<=0)
    return NULL;
	len++; //add terminating 0
  char* ustr=(char*)malloc(len*sizeof(char));
  if(VS_UCSToUTF8(wstr,ustr,len)>0)
    return ustr;

  free(ustr);
  return NULL;
};

size_t VS_StrToUCS(const char* str,wchar_t* wstr,size_t maxlen)
{
	if(str==NULL)
	{
		if(maxlen>0)
			*wstr=0;
		return 0;
	};

	wchar_t			*p;
	unsigned char	*q;
	size_t			i;
	for(i=0,p=wstr,q=(unsigned char*)str;i<maxlen && *q;p++,q++,i++)
		*p=*q;
	*p=0;

	return i;
}

wchar_t* VS_StrToUCS(const char* str)
{
	if(str==NULL)
		return NULL;
	size_t size=strlen(str)+1;
	wchar_t* wstr=(wchar_t*)malloc(size*sizeof(wchar_t));
	wchar_t*		p;
	unsigned char*	q;
	for(p=wstr,q=(unsigned char*)str;*q;p++,q++)
		*p=*q;
	*p=0;

	return wstr;
}

char* VS_UCSToStr(const wchar_t* wstr)
{
		if(!wstr)
			return NULL;

    size_t size=wcslen(wstr)+1;
    char* outstr=(char*)malloc(size);
	const wchar_t	*p;
	unsigned char	*q;
	for(p=wstr,q=(unsigned char*)outstr;*p;p++,q++)
		*q=(unsigned char)*p;
	*q=0;
	return outstr;
};

size_t VS_UCSToStr(const wchar_t* wstr, char* str,size_t maxlen)
{
	if(wstr==NULL)
	{
		if(maxlen>0)
			*str=0;
		return 0;
	};

	const wchar_t	*p;
	unsigned char	*q;
	size_t			i;

	for(i=0,p=wstr,q=(unsigned char*)str;i<maxlen && *p;p++,q++,i++)
		*q=(unsigned char)*p;
	*q=0;

	return i;
}
