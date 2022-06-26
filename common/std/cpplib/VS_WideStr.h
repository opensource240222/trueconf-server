/**
 **************************************************************************
 * \file VS_WideStr
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Wide char strings class
 *
 * \b Project Standart Libraries
 * \author StasS
 * \date 17.02.04
 *
 * $Revision: 5 $
 *
 * $History: VS_WideStr.h $
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 11.04.12   Time: 16:11
 * Updated in $/VSNA/std/cpplib
 * #11566: operator==(const wchar_t* sz) const didn't check sz for null
 * - VS_ConfigurationService::SetEpProperties_Method() back to 15
 * revision, because fix at VS_WideStr
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 24.09.10   Time: 18:13
 * Updated in $/VSNA/std/cpplib
 * Trim(): check str for null
 *
 * *****************  Version 3  *****************
 * User: Ktrushnikov  Date: 22.09.10   Time: 16:43
 * Updated in $/VSNA/std/cpplib
 * - new search from client by [name, call_id, email]
 * - sp_set_group_endpoint_properties: directX param size fixed for
 * PostgreSQL
 * - VS_WideStr::Trim() function
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 15.11.07   Time: 15:28
 * Updated in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 7.02.07    Time: 12:56
 * Updated in $/VS2005/std/cpplib
 * fixed warning
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 19.01.07   Time: 14:39
 * Updated in $/VS/std/cpplib
 * fixed conversion from lame bstr (ms bug)
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 30.03.06   Time: 17:12
 * Updated in $/VS/std/cpplib
 * added switchable AB to reg storage
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 1.03.06    Time: 20:05
 * Updated in $/VS/std/cpplib
 * fixed const variant interface
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 ****************************************************************************/
#ifndef VS_WIDESTR_H
#define VS_WIDESTR_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "std-generic/cpplib/utf8.h"
#include "std/cpplib/utf8_old.h"

#include <cstdlib>
#include <cstring>
#include <cwctype>

#if defined(_WIN32)
#	define wcsdup _wcsdup
#endif

/**
 **************************************************************************
 * \brief UCS-2 string Class
 ****************************************************************************/
class VS_WideStr
{
public:
	wchar_t* m_str;

	// Constructor/Destructor
	VS_WideStr() : m_str(NULL) {}

	explicit VS_WideStr(int nSize)
	{
		if (nSize == 0)
		{ m_str = NULL; }
		else
		{
			m_str = (wchar_t*)malloc(nSize*sizeof(wchar_t));
			memset(m_str, 0, nSize*sizeof(wchar_t));
		} // end if
	}

	VS_WideStr(int nSize, const wchar_t* sz)
	{
		if (nSize == 0)
		{ m_str = NULL; }
		else
		{
			m_str = (wchar_t*)malloc((nSize+1)*sizeof(wchar_t));
			wcsncpy(m_str, sz, nSize);
			m_str[nSize] = '\0';
		} // end if
	}

	VS_WideStr(const wchar_t* sz)
	{
		if (sz == NULL)   m_str = NULL;
		else              m_str = wcsdup(sz);
	}

	VS_WideStr(const VS_WideStr& src) { m_str = src.Copy(); }
	VS_WideStr(VS_WideStr&&src)
	{
		*this = std::move(src);
	}

	~VS_WideStr() { Empty(); }

	// Operator=
	VS_WideStr& operator=(VS_WideStr&&src)
	{
		m_str = src.m_str;
		src.m_str = nullptr;
		return *this;
	}
	VS_WideStr& operator=(const VS_WideStr& src)
	{
		if (m_str != src.m_str)
		{
			Empty();
			m_str = src.Copy();
		} // end if
		return *this;
	}

	VS_WideStr& operator=(const wchar_t* sz)
	{
		if (sz != m_str)
		{
			Empty();
			if (sz != NULL) m_str = wcsdup(sz);
			else            m_str = NULL;
		} // end if
		return *this;
	}

	bool IsEmpty() const { return nullptr == m_str || 0 == *m_str; }

	// Length
	unsigned int Length() const
	{ return (unsigned int)((m_str == NULL)? 0 : wcslen(m_str)); }
	unsigned int ByteLength() const
	{ return (unsigned int)((m_str == NULL)? 0 : (wcslen(m_str)+1)*sizeof(wchar_t)); }

	// Type conversion
	operator wchar_t*() { return m_str; }
	operator const wchar_t*() const { return m_str; }
	wchar_t** operator&() { return &m_str; }
	// Copying
	wchar_t* Copy() const  {
		if (m_str == NULL)  return NULL;
		else        return wcsdup(m_str);
	}
	void CopyTo(wchar_t** sz)
	{ if (sz != NULL)   *sz = Copy();  }

	VS_WideStr& Assign(const wchar_t* sz)
	{ return operator=(sz); }
	// Attach/Detach
	void Attach(wchar_t* sz)
	{
		if (m_str != sz)
		{
			Empty();
			m_str = sz;
		} // end if
	}
	wchar_t* Detach()
	{
		wchar_t* sz = m_str;
		m_str = NULL;
		return sz;
	}
	// Internal buffer
	void Empty()
	{
		if (m_str != NULL)
		{
			free(m_str);
			m_str = NULL;
		} // end if
	}
	// Append
	void Append(const VS_WideStr& simpleSrc) { Append(simpleSrc.m_str); }
	void Append(wchar_t* p)
	{
		size_t lenNew;
		if ((p != NULL) && ((lenNew = wcslen(p)) != 0))
		{
			if (m_str == NULL)   m_str = wcsdup(p);
			else
			{
				lenNew += Length() + 1;
				m_str = (wchar_t*)realloc(m_str, lenNew*sizeof(wchar_t));
				wcscat(m_str, p);
			} // end if
		} // end if
	}
	void Append(const wchar_t* sz, int nLen)
	{
		size_t lenNew;
		if ((sz != NULL) && (nLen > 0) && ((lenNew = wcslen(sz)) != 0))
		{
			if (lenNew < (size_t)nLen)  nLen = (int)lenNew;
			else            lenNew = nLen;
			// Copying
			if (m_str == 0)
			{
				m_str = (wchar_t*)malloc((lenNew+1)*sizeof(wchar_t));
				wcsncpy(m_str, sz, nLen);
				m_str[nLen] = '\0';
			}
			else
			{
				lenNew += Length()+1;
				m_str = (wchar_t*)realloc(m_str, lenNew * sizeof(wchar_t));
				wcsncat(m_str, sz, nLen);
				m_str[lenNew-1] = '\0';
			} // end if
		} // end if
	}
	VS_WideStr& operator+=(const VS_WideStr& simpleSrc)
	{
		Append(simpleSrc.m_str);
		return *this;
	}

	VS_WideStr& operator+=(const wchar_t* sz)
	{
		Append(sz);
		return *this;
	}
	// Conversion
	void ToLower()
	{
		if (!m_str)
			return;
		for (auto p = m_str; *p; ++p)
			*p = static_cast<wchar_t>(::towlower(*p));
	}
	void ToUpper()
	{
		if (!m_str)
			return;
		for (auto p = m_str; *p; ++p)
			*p = static_cast<wchar_t>(::towupper(*p));
	}
	// Comparison
	bool operator!() const { return m_str==NULL || *m_str==0; }
	bool operator<(const VS_WideStr& simpleSrc) const
	{
		if (IsEmpty())
			return (!simpleSrc.IsEmpty());
		else if (simpleSrc.IsEmpty())
			return false;
		return wcscmp(m_str, simpleSrc.m_str) < 0;
	}

	bool operator<(const wchar_t* sz) const { return operator<(VS_WideStr(sz)); }
	bool operator>(const VS_WideStr& simpleSrc) const throw()
	{
		if (simpleSrc.IsEmpty())
			return (!IsEmpty());
		else if (IsEmpty())
			return false;
		return wcscmp(m_str, simpleSrc.m_str) > 0;
	}

	bool operator>(const wchar_t* sz) const { return operator>(VS_WideStr(sz)); }
	bool operator!=(const VS_WideStr& simpleSrc) const { return !operator==(simpleSrc); }
	bool operator!=(const wchar_t* sz) const { return !operator==(sz); }
	bool operator!=(int nNull) const { return !operator==(nNull); }

	bool operator==(const wchar_t* sz) const
	{
		if (operator!())
		{ return (sz==NULL || *sz==0); }
		else if (!sz || (sz!=NULL && *sz==0))
			return false;

		return wcscmp(m_str, sz) == 0;
	}

	bool operator==(const VS_WideStr& simpleSrc) const
	{ return operator==(simpleSrc.m_str); }

	bool operator==(int) const { return m_str==NULL || *m_str==0; }

#if defined(_WIN32) // Not ported yet
	char* ToUTF8() const
	{
		if(operator!())
			return NULL;

		auto str_utf8 = vs::UTF16toUTF8Convert(m_str);
		if (!str_utf8.empty())
		{
			static_assert(sizeof(wchar_t) == sizeof(char16_t), "Conversion is not correct. wchar_t != char16_t");
			char* outstr = (char*)malloc(str_utf8.length() + 1);
			memcpy_s(outstr, str_utf8.length(), str_utf8.c_str(), str_utf8.length());
			*(outstr + str_utf8.length()) = 0;
			return outstr;
		}
		else
		{
			return VS_UCSToUTF8(m_str);
		}
	};

	bool ToUTF8(std::string &out) const
	{
		if (operator!())
			return false;

		out = vs::UTF16toUTF8Convert(m_str);
		if (out.empty()) {
			char* s = VS_UCSToUTF8(m_str);
			if (s) {
				out = s;
				free(s);
			}
		}
		return !out.empty();
	};

	VS_WideStr& AssignUTF8(const char* ustr)
	{
		Empty();
		if(ustr!=NULL)
		{
			auto str_utf16 = vs::UTF8toUTF16Convert(ustr);
			if (!str_utf16.empty())
			{
				static_assert(sizeof(wchar_t) == sizeof(char16_t), "Conversion is not correct. wchar_t != char16_t");
				m_str = wcsdup(reinterpret_cast<const wchar_t*>(str_utf16.c_str()));
			}
			else
			{
				m_str=VS_UTF8ToUCS(ustr);
			}
		}

		return *this;
	}
#endif

	///functions, allowing conversion from char*
	///dupes string, converting to char by cutting main wchar byte
	char* ToStr() const
	{
		if(operator!())
			return NULL;

		int size=Length()+1;
		char* outstr=(char*)malloc(size);
		wchar_t				*p;
		unsigned char	*q;
		for(p=m_str,q=(unsigned char*)outstr;*p;p++,q++)
			*q=(unsigned char)*p;
		*q=0;
		return outstr;
	};


	char* ToStr(char* outstr, int maxsize) const
	{
		if(operator!() || outstr==NULL)
			return NULL;

		wchar_t				*p;
		unsigned char	*q;
		for(p=m_str,q=(unsigned char*)outstr;*p && p-m_str<maxsize-1;p++,q++)
			*q=(unsigned char)*p;
		*q=0;
		return outstr;
	};

	VS_WideStr& AssignStr(const char* sz)
	{
		Empty();
		m_str=VS_StrToUCS(sz);
		return *this;
	}

  void Resize(int nSize)
	{
	  Empty();

 		if (nSize > 0)
		{
			m_str = (wchar_t*)malloc(nSize*sizeof(wchar_t));
			memset(m_str, 0, nSize*sizeof(wchar_t));
		}
	}


	/*char* ToStr(unsigned int CODEPAGE=CP_ACP)
	{
	int wsize=Length()+1;
	int usize=WideCharToMultiByte(CODEPAGE,0,m_str,wsize,NULL,0,NULL,NULL);
	char* sz=(char*)malloc(usize);
	if(WideCharToMultiByte(CODEPAGE,0,m_str,wsize,sz,usize,NULL,NULL)>0)
	return sz;
	else
	return NULL;
	};


	char* ToStr(char* buf,int maxsize,unsigned int CODEPAGE=CP_ACP)
	{
	if(WideCharToMultiByte(CODEPAGE,0,m_str,-1,buf,maxsize,NULL,NULL)>0)
	return buf;
	else
	return NULL;
	};*/


	// VS_Map Support
	static inline void* Factory ( const void* str )
	{
		if (!str) return 0;
		return wcsdup((wchar_t*)str);
	}
	static inline void Destructor ( void* str )
	{ if (str) free(str); }

	static inline int Predicate ( const void* str1, const void* str2 )
	{ return wcscmp((wchar_t*)str1, (wchar_t*)str2); }

	void Trim()
	{
		if (!m_str || !*m_str)
			return ;

		auto p = m_str;
		while (*p == L' ')
			++p;
		auto begin = p;
		auto end = begin;
		while (*p)
		{
			if (*p != L' ')
				end = p + 1;
			++p;
		}
		if (begin != m_str)
			memmove(m_str, begin, (end - begin) * sizeof(wchar_t));
		m_str[end - begin] = L'\0';
	}

};

#endif // VS_WIDESTR_H
