/**
 **************************************************************************
 * \file utf8.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief This code implements UTF8<->UCS2 recode functions.
 *
 * \b Project Standart Libraries
 * \author StasS
 * \date 17.02.04
 *
 * $Revision: 1 $
 *
 * $History: utf8.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/clib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/clib
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 30.03.06   Time: 17:12
 * Updated in $/VS/std/clib
 * added switchable AB to reg storage
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 30.12.04   Time: 14:12
 * Updated in $/VS/std/clib
 * files headers
 ****************************************************************************/
#pragma once

/****************************************************************************
 * Includes
 ****************************************************************************/
#include <cstddef>

/****************************************************************************
 * Functions
 ****************************************************************************/
size_t  VS_UTF8_CharLength(const char* ustr);
size_t  VS_Wide_UTF8Length(const wchar_t* wstr);

size_t VS_UTF8ToUCS(const char* ustr,wchar_t* wstr,size_t maxlen);
wchar_t* VS_UTF8ToUCS(const char* ustr);

size_t VS_UCSToUTF8(const wchar_t* wstr,char* ustr,size_t maxlen);
char* VS_UCSToUTF8(const wchar_t* wstr);

size_t VS_StrToUCS(const char* ustr,wchar_t* wstr,size_t maxlen);
wchar_t* VS_StrToUCS(const char* ustr);

char* VS_UCSToStr(const wchar_t* wstr);
size_t VS_UCSToStr(const wchar_t* wstr, char* str,size_t maxlen);
