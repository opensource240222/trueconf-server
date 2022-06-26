/**
 **************************************************************************
 * \file VS_FileTime.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief definition of FILETIME operating class
 *
 * \b Project Standart Libraries
 * \author StasS
 * \date 05.09.03
 *
 * $Revision: 7 $
 *
 * $History: VS_FileTime.h $
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 22.06.11   Time: 21:20
 * Updated in $/VSNA/std/cpplib
 * - bugfix#9213
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 9.06.11    Time: 12:27
 * Updated in $/VSNA/std/cpplib
 * - don't use opertator=() from _variant_t, because he add TimeZone
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 11.11.10   Time: 17:54
 * Updated in $/VSNA/std/cpplib
 *  - GetTimeInSec return unsigned long
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 14.10.10   Time: 16:16
 * Updated in $/VSNA/std/cpplib
 *  - 7231
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 28.07.10   Time: 16:28
 * Updated in $/VSNA/std/cpplib
 * - VS_FileTime::operator variant_t() doesn't add TimeZone now
 * - Use another func to return variant with TimeZone
 *
 * *****************  Version 2  *****************
 * User: Ktrushnikov  Date: 21.03.08   Time: 19:42
 * Updated in $/VSNA/std/cpplib
 * - support for Current stats  (not Average stats): send from AS to SM
 * and saved to Registry::Servers
 * - VS_FileTime: RUS_FMT added: dd.mm.yyyy hh:mm:ss
 * - struct VS_AppServerStats added
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 7.02.07    Time: 12:56
 * Updated in $/VS2005/std/cpplib
 * fixed warning
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 5.02.07    Time: 18:43
 * Updated in $/VS2005/std/cpplib
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 6.12.06    Time: 12:29
 * Updated in $/VS/std/cpplib
 * support for null variant
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 18.05.06   Time: 20:05
 * Updated in $/VS/std/cpplib
 * added <= >=
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
****************************************************************************/
#ifndef VS_FILETIME_H
#define VS_FILETIME_H


#ifdef _WIN32

/****************************************************************************
* Includes
****************************************************************************/
#include <windows.h>
//#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <comutil.h>
#include <oleauto.h>
#include <ctime>
#include <utility>
#include <chrono>

#include "std-generic/cpplib/TimeUtils.h"
/**
**************************************************************************
* \brief Simple FILETIME Class
****************************************************************************/
#pragma warning(disable:4996)

class VS_FileTime
{
public:
	FILETIME m_filetime;
	static const char *MONTH[];
	// Constructor/Destructor
	VS_FileTime() {	Empty(); }
	VS_FileTime(const FILETIME* ft)
	{
		if (ft == NULL)
			Empty();
		else {
			m_filetime.dwLowDateTime = ft->dwLowDateTime;
			m_filetime.dwHighDateTime = ft->dwHighDateTime;
		} // end if
	}
	explicit VS_FileTime(uint32_t time)
	{
		if (!time)
		{
			Empty();
		}
		else
		{
			LONGLONG time64 = Int32x32To64(time, 10000000) + 116444736000000000;
			m_filetime.dwLowDateTime = (DWORD)time64;
			m_filetime.dwHighDateTime = time64 >> 32;
		}
	}
	explicit VS_FileTime(uint64_t time)
	{
		if (!time) {
			Empty();
		}
		else {
			m_filetime.dwLowDateTime = (DWORD)time;
			m_filetime.dwHighDateTime = time >> 32;
		}
	}
	explicit VS_FileTime(__time64_t time) :VS_FileTime((uint64_t)tu::UnixSecondsToWindowsTicks(std::chrono::system_clock::from_time_t(time))) {}
	explicit VS_FileTime(__time32_t time) :VS_FileTime((uint32_t)tu::UnixSecondsToWindowsTicks(std::chrono::system_clock::from_time_t(time))) {}

	explicit VS_FileTime(const std::chrono::system_clock::time_point tp) :VS_FileTime(std::chrono::system_clock::to_time_t(tp)){}
	VS_FileTime(const VS_FileTime& src) { src.Copy(&m_filetime); }
	VS_FileTime(VS_FileTime&&src)
	{
		*this = std::move(src);
	}

	VS_FileTime(const _variant_t& time)
	{
		this->operator=(time);
	}

	~VS_FileTime()	{ Empty(); }
	// Operator=
	VS_FileTime& operator=(const VS_FileTime& src)	{
		src.Copy(&m_filetime);
		return *this;
	}
	VS_FileTime & operator=(VS_FileTime&&src)
	{
		m_filetime = src.m_filetime;
		src.m_filetime.dwLowDateTime = 0;
		src.m_filetime.dwHighDateTime = 0;
		return *this;
	}
	VS_FileTime& operator=(const FILETIME* ft)	{
		if (ft == NULL)
			Empty();
		else {
			m_filetime.dwLowDateTime = ft->dwLowDateTime;
			m_filetime.dwHighDateTime = ft->dwHighDateTime;
		} // end if
		return *this;
	}
	VS_FileTime& operator=(const _variant_t& time)
	{
		return FromVariant(time, true);
	}

	VS_FileTime& FromVariant(const _variant_t& time, bool withTz)
	{
		if(time.vt==VT_NULL || time.vt==VT_EMPTY)
		{
			Empty();
			return *this;
		}

		SYSTEMTIME st;
		FILETIME ft;
		if (time.vt!=VT_DATE)
		{
			_variant_t tmp(time);
			tmp.ChangeType(VT_DATE);
			VariantTimeToSystemTime(tmp.dblVal, &st);
		}
		else
			VariantTimeToSystemTime(time.dblVal, &st);
		if (withTz)
		{
			SystemTimeToFileTime(&st,&ft);
			LocalFileTimeToFileTime(&ft,&m_filetime);
		}else{
			SystemTimeToFileTime(&st,&m_filetime);
		}
		return *this;
	}

	VS_FileTime& FromVariant_NoTZ(const _variant_t& time)
	{
		return FromVariant(time, false);
	}

	void Now ()
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		SystemTimeToFileTime(&st, &m_filetime);
	}
	// Type conversion
	operator FILETIME() const { return m_filetime; }
	explicit operator time_t() const
	{
		ULARGE_INTEGER ull;
		ull.LowPart = m_filetime.dwLowDateTime;
		ull.HighPart = m_filetime.dwHighDateTime;
		return ull.QuadPart / 10000000ULL - 11644473600ULL;
	}
	explicit operator std::chrono::system_clock::time_point() { return std::chrono::system_clock::from_time_t(operator time_t()); }

private:
	variant_t ToVariant(bool withTz = false) const
	{
		DATE date;
		SYSTEMTIME st;
		FileTimeToSystemTime(&m_filetime, &st);
		if (withTz)
			SystemTimeToTzSpecificLocalTime(NULL,&st,&st);
		SystemTimeToVariantTime(&st, &date);
		return variant_t(date);
	}
public:
	operator variant_t() const {
		return ToVariant();
	}
	variant_t ToVariant_WithTZ() const {
		return ToVariant(true);
	}
	FILETIME* operator&() { return &m_filetime; }
	const FILETIME* operator&() const { return &m_filetime; }
	// Copying
	void Copy(FILETIME* ft) const
	{
		if (ft != NULL) {
			ft->dwLowDateTime = m_filetime.dwLowDateTime;
			ft->dwHighDateTime = m_filetime.dwHighDateTime;
		} // end if
	}
	void Assign(const FILETIME* ft) { operator=(ft); }
	unsigned long GetTimeInSec() {
		ULARGE_INTEGER sec;
		sec.LowPart = m_filetime.dwLowDateTime;
		sec.HighPart = m_filetime.dwHighDateTime;
		sec.QuadPart/=10000000;
		return sec.HighPart==0?sec.LowPart:0;
	}
	std::chrono::system_clock::time_point chrono_system_clock_time_point()const
	{
		ULARGE_INTEGER ull;
		ull.LowPart = m_filetime.dwLowDateTime;
		ull.HighPart = m_filetime.dwHighDateTime;
		std::chrono::milliseconds ms(ull.QuadPart / 10000ULL - 11644473600000ULL);
		return std::chrono::system_clock::time_point(ms);
	}

	// Internal buffer
	void Empty()
	{
		m_filetime.dwLowDateTime = 0;
		m_filetime.dwHighDateTime = 0;
	}
	// Append
	VS_FileTime& operator+=(const VS_FileTime& src)
	{
		operator+=(&src.m_filetime);
		return *this;
	}
	VS_FileTime& operator+=(ULONGLONG ns100)
	{
		ULARGE_INTEGER sum;
		sum.LowPart		= m_filetime.dwLowDateTime;
		sum.HighPart	= m_filetime.dwHighDateTime;
		sum.QuadPart += ns100;
		m_filetime.dwLowDateTime = sum.LowPart;
		m_filetime.dwHighDateTime = sum.HighPart;

		return *this;
	}
	VS_FileTime& operator+=(const FILETIME* ft)
	{
		if (ft != NULL) {
			ULARGE_INTEGER sum, sec;
			sum.LowPart = m_filetime.dwLowDateTime;
			sum.HighPart = m_filetime.dwHighDateTime;
			sec.LowPart = ft->dwLowDateTime;
			sec.HighPart = ft->dwHighDateTime;
			sum.QuadPart += sec.QuadPart;
			m_filetime.dwLowDateTime = sum.LowPart;
			m_filetime.dwHighDateTime = sum.HighPart;
		} // end if
		return *this;
	}
	VS_FileTime& operator-=(const VS_FileTime& src)
	{
		operator-=(&src.m_filetime);
		return *this;
	}
	VS_FileTime& operator-=(const FILETIME* ft)
	{
		if (ft != NULL) {
			ULARGE_INTEGER sum, sec;
			sum.LowPart = m_filetime.dwLowDateTime;
			sum.HighPart = m_filetime.dwHighDateTime;
			sec.LowPart = ft->dwLowDateTime;
			sec.HighPart = ft->dwHighDateTime;
			sum.QuadPart -= sec.QuadPart;
			m_filetime.dwLowDateTime = sum.LowPart;
			m_filetime.dwHighDateTime = sum.HighPart;
		} // end if
		return *this;
	}
	// Conversion
#define GLOBAL_TIME_FMT "%d %02d/%02d %02d:%02d:%02d GMT"
#define LOCAL_TIME_FMT "%d %02d/%02d %02d:%02d:%02d"
#define NEUTRAL_TIME_FMT "%d/%02d/%02d %02d.%02d.%02d GMT"
#define RUS_TIME_FMT "%02d.%02d.%04d %02d:%02d:%02d"
#define ISO8601_Z_TIME_FMT "%04d-%02d-%02dT%02d:%02d:%02dZ"
	const char* ToNStr (char* str) const
	{
		if (str == NULL)
			return NULL;
		if (operator!=(NULL)) {
			SYSTEMTIME st;
			FileTimeToSystemTime(&m_filetime, &st);
			sprintf(str, NEUTRAL_TIME_FMT, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		} else
			strcpy(str, "none");
		return str;
	}
	const char* ToLFStr (char* str) const
	{
		if (str == NULL)
			return NULL;
		SYSTEMTIME st; memset(&st, 0, sizeof(SYSTEMTIME));
		TIME_ZONE_INFORMATION tz; memset(&tz, 0, sizeof(TIME_ZONE_INFORMATION));
		if (operator!=(NULL)) {
			FileTimeToSystemTime(&m_filetime, &st);
			SystemTimeToTzSpecificLocalTime(NULL, &st, &st);
			GetTimeZoneInformation(&tz);
		}
		sprintf(str, "%02d/%s/%04d:%02d:%02d:%02d %+03ld%02ld", st.wDay,
			MONTH[st.wMonth], st.wYear, st.wHour, st.wMinute, st.wSecond, -(tz.Bias/60), tz.Bias%60);
		return str;
	}
	const char* ToStr (char* str, bool local = false) const
	{
		if (str == NULL)
			return NULL;
		if (operator!=(NULL)) {
			SYSTEMTIME st;
			FileTimeToSystemTime(&m_filetime, &st);
			if (local)
				if (!SystemTimeToTzSpecificLocalTime(NULL, &st, &st))
					local = false;
			sprintf(str, (local? LOCAL_TIME_FMT: GLOBAL_TIME_FMT), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		} else
			strcpy(str, "none");
		return str;
	}
	const char* ToStr (char* str, size_t len, bool local = false) const
	{
		if ((str == NULL) || (len  == 0))
			return NULL;
		if (operator!=(NULL)) {
			SYSTEMTIME st;
			FileTimeToSystemTime(&m_filetime, &st);
			if (local)
				SystemTimeToTzSpecificLocalTime(NULL, &st, &st);
			_snprintf(str, len, (local? LOCAL_TIME_FMT: GLOBAL_TIME_FMT), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		} else
			strncpy(str, "none", len);
		return str;
	}
	const char* ToRUSStr (char* str) const
	{
		if (str == NULL)
			return NULL;
		SYSTEMTIME st; memset(&st, 0, sizeof(SYSTEMTIME));
		TIME_ZONE_INFORMATION tz; memset(&tz, 0, sizeof(TIME_ZONE_INFORMATION));
		if (operator!=(NULL)) {
			FileTimeToSystemTime(&m_filetime, &st);
			SystemTimeToTzSpecificLocalTime(NULL, &st, &st);
			GetTimeZoneInformation(&tz);
		} else
			strcpy(str, "none");
		sprintf(str, RUS_TIME_FMT, st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
		return str;
	}
	const char* ToStr_ISO8601_Z(char* str) const
	{
		if (str == NULL)
			return NULL;
		if (operator!=(NULL)) {
			SYSTEMTIME st;
			FileTimeToSystemTime(&m_filetime, &st);
			sprintf(str, ISO8601_Z_TIME_FMT, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		} else
			strcpy(str, "none");
		return str;
	}
	// Comparison
	bool operator!() const { return (m_filetime.dwLowDateTime == 0 && m_filetime.dwHighDateTime == 0);}
	bool operator<(const VS_FileTime& sec) const
	{
		if (m_filetime.dwHighDateTime < sec.m_filetime.dwHighDateTime)
			return true;
		if ((m_filetime.dwHighDateTime == sec.m_filetime.dwHighDateTime) &&
			(m_filetime.dwLowDateTime < sec.m_filetime.dwLowDateTime))
			return true;
		return false;
	}
	bool operator<=(const FILETIME* ft) const
	{
		if (m_filetime.dwHighDateTime < ft->dwHighDateTime)
			return true;
		if ((m_filetime.dwHighDateTime == ft->dwHighDateTime) &&
			(m_filetime.dwLowDateTime <= ft->dwLowDateTime))
			return true;
		return false;
	}
	bool operator<=(const VS_FileTime& sec) const
  {
    return operator<=(&sec);
  }

	bool operator> (const FILETIME* ft) const     { return !operator<=(ft); }
	bool operator> (const VS_FileTime& sec) const { return !operator<=(&sec); }
	bool operator>=(const FILETIME* ft) const     { return !operator<(ft); }
	bool operator>=(const VS_FileTime& sec) const { return !operator<(&sec); }

	bool operator!=(const VS_FileTime& sec) const { return !operator==(sec); }
	bool operator!=(const FILETIME* ft) const { return !operator==(ft); }
	bool operator!=(int nNull) const { return !operator==(nNull); }
	bool operator==(const VS_FileTime& sec) const
	{
		if ((m_filetime.dwHighDateTime == sec.m_filetime.dwHighDateTime) &&
			(m_filetime.dwLowDateTime == sec.m_filetime.dwLowDateTime))
			return true;
		return false;
	}
	bool operator==(const FILETIME* ft) const { return operator==(VS_FileTime(ft)); }
	bool operator==(int nNull) const { return operator!(); }
};

LONGLONG operator-(const VS_FileTime& a,const FILETIME* b);
inline LONGLONG operator-(const VS_FileTime& a,const VS_FileTime& b)
{return a-&b;};

#pragma warning(default:4996)

#endif //_WIN32

#endif // VS_FILETIME_H
