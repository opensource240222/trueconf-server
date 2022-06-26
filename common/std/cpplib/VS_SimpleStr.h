/**
**************************************************************************
* \file VS_SimpleStr
* \brief Simple string Class
*
* \b Project Standart Libraries
* \author Petrovichev
* \author SMirnovK
* \author StasS
* \date 11.03.03
*
****************************************************************************/

#ifndef VS_SIMPLESTR_H
#define VS_SIMPLESTR_H

/****************************************************************************
* Includes
****************************************************************************/
#include "std-generic/clib/strcasecmp.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <utility>
#include "std-generic/cpplib/string_view.h"
#include "std-generic/attributes.h"

#if defined(_WIN32)
#	define strdup _strdup
#endif

/**
**************************************************************************
* \brief String class
****************************************************************************/
class VS_SimpleStr
{
public:
	char* m_str;
	// Constructor/Destructor
	VS_SimpleStr() : m_str(nullptr) {}
	explicit VS_SimpleStr(int nSize) {
		if (nSize == 0)	m_str = NULL;
		else {
			m_str = (char*)malloc(nSize);
			memset(m_str, 0, nSize);
		} // end if
	}
	VS_SimpleStr(int nSize, const char* sz)	{
		if (nSize == 0)	m_str = NULL;
		else {
			m_str = (char*)malloc(nSize+1);
			strncpy(m_str, sz, nSize);
			m_str[nSize] = '\0';
		} // end if
	}
	VS_SimpleStr(const char* sz) {
		if (sz == NULL)		m_str = NULL;
		else				m_str = strdup(sz);
	}
	VS_SimpleStr(const VS_SimpleStr& src) {	m_str = src.Copy(); }
	VS_SimpleStr(VS_SimpleStr&& src) noexcept
		: m_str(src.m_str)
	{
		src.m_str = nullptr;
	}
	~VS_SimpleStr()	{ Empty(); }

	// Operator=
	VS_SimpleStr& operator=(VS_SimpleStr&& src) noexcept
	{
		if (this == &src)
			return *this;
		::free(m_str);
		m_str = src.m_str;
		src.m_str = nullptr;
		return *this;
	}
	VS_SimpleStr& operator=(const VS_SimpleStr& src) {
		if (m_str != src.m_str) {
			Empty();
			m_str = src.Copy();
		} // end if
		return *this;
	}
	VS_SimpleStr& operator=(const char* sz)	{
		if (sz != m_str) {
			Empty();
			if (sz != NULL)		m_str = strdup(sz);
			else				m_str = NULL;
		} // end if
		return *this;
	}

	// Length
	unsigned int Length() const { return (unsigned int)((m_str == NULL)? 0 : strlen(m_str)); }
	bool IsEmpty() const { return nullptr == m_str || 0 == *m_str; }
	unsigned int ByteLength() const { return (unsigned int)((m_str == NULL)? 0 : strlen(m_str) + 1); }
	// Type conversion
	operator char*() { return m_str; }
	operator const char*() const { return m_str; }
	// Copying
	char* Copy() const	{
		if (m_str == NULL)	return NULL;
		else				return strdup(m_str);
	}
	void CopyTo(char** sz)	{
		if (sz != NULL)		*sz = Copy();
	}
	void Assign(const char* sz) { operator=(sz); }
	// Attach/Detach
	void Attach(char* sz)	{
		if (m_str != sz) {
			Empty();
			m_str = sz;
		} // end if
	}
	char* Detach()	{
		char* sz = m_str;
		m_str = NULL;
		return sz;
	}
	// Internal buffer
	void Empty() {
		if (m_str != NULL) {
			free(m_str);
			m_str = NULL;
		} // end if
	}
	// Append
	void Append(const VS_SimpleStr& simpleSrc) { Append(simpleSrc.m_str); }
	void Append(const char* p) {
		size_t lenNew;
		if ((p != NULL) && ((lenNew = strlen(p)) != 0)) {
			if (m_str == 0)		m_str = strdup(p);
			else {
				lenNew += Length() + 1;
				m_str = (char*)realloc(m_str, lenNew);
				strcat(m_str, p);
			} // end if
		} // end if
	}
	void Append(const char* sz, int nLen)
	{
		size_t lenNew;
		if ((sz != NULL) && (nLen > 0) && ((lenNew = strlen(sz)) != 0)) {
			if (lenNew < (size_t)nLen)  nLen = (int)lenNew;
			else						lenNew = nLen;
			// Copying
			if (m_str == 0) {
				m_str = (char*)malloc(lenNew + 1);
				strncpy(m_str, sz, nLen);
				m_str[nLen] = '\0';
			} else {
				lenNew += Length() + 1;
				m_str = (char*)realloc(m_str, lenNew);
				strncat(m_str, sz, nLen);
				m_str[lenNew - 1] = '\0';
			} // end if
		} // end if
	}
	VS_SimpleStr& operator+=(const VS_SimpleStr& simpleSrc)	{
		Append(simpleSrc.m_str);
		return *this;
	}
	VS_SimpleStr& operator+=(const char* sz)	{
		Append(sz);
		return *this;
	}
	// Conversion
	void ToLower()
	{
		if (!m_str)
			return;
		for (auto p = m_str; *p; ++p)
			*p = static_cast<char>(::tolower(*p));
	}
	void ToUpper()
	{
		if (!m_str)
			return;
		for (auto p = m_str; *p; ++p)
			*p = static_cast<char>(::toupper(*p));
	}
	// Comparison
	bool operator!() const { return m_str==NULL || *m_str==0; }
	bool operator<(const VS_SimpleStr& simpleSrc) const {
		if (IsEmpty())
			return (!simpleSrc.IsEmpty());
		else if (simpleSrc.IsEmpty())
			return false;
		return strcmp(m_str, simpleSrc.m_str) < 0;
	}
	bool operator<(const char* sz) const { return operator<(VS_SimpleStr(sz)); }
	bool operator>(const VS_SimpleStr& simpleSrc) const throw()	{
		if (simpleSrc.IsEmpty())
			return (!IsEmpty());
		else if (IsEmpty())
			return false;
		return strcmp(m_str, simpleSrc.m_str) > 0;
	}
	bool operator>(const char* sz) const { return operator>(VS_SimpleStr(sz)); }
	bool operator!=(const VS_SimpleStr& simpleSrc) const { return !operator==(simpleSrc); }
	bool operator!=(const char* sz) const { return !operator==(sz); }
	bool operator!=(int nNull) const { return !operator==(nNull); }
	bool operator==(const VS_SimpleStr& simpleSrc) const {
		if (IsEmpty())
			return (simpleSrc.IsEmpty());
		else if (simpleSrc.IsEmpty())
			return false;
		return strcmp(m_str, simpleSrc.m_str) == 0;
	}
	bool operator==(const char* sz) const {
		bool i_am_null = ( m_str==NULL || *m_str==0 );
		bool party_null= ( sz==NULL || *sz==0 );
		if(i_am_null)
			return party_null;
		else if(party_null)
			return false;
		return strcmp(m_str,sz) == 0;
	}

	bool iCompare(const VS_SimpleStr& simpleSrc) const {
		if (IsEmpty())
			return (simpleSrc.IsEmpty());
		else if (simpleSrc.IsEmpty())
			return false;
		return strcasecmp(m_str, simpleSrc.m_str) == 0;
	}

	bool iCompare(const char* sz) const {
		bool i_am_null = (m_str == NULL || *m_str == 0);
		bool party_null = (sz == NULL || *sz == 0);
		if (i_am_null)
			return party_null;
		else if (party_null)
			return false;
		return strcasecmp(m_str, sz) == 0;
	}

	bool operator==(char* sz) const { return operator ==((const char*)sz); }

	bool operator!=(char* sz) const { return operator !=((const char*)sz); }

	bool operator==(int) const { return m_str==NULL || *m_str==0; }

	static inline void* Factory ( const void* str ) {
		if (!str) return 0;
		char*	newStr = (char*)malloc(strlen((char*)str) + 1);
		strcpy(newStr, (char*)str);
		return newStr;
	}
	static inline void Destructor ( void* str )	{	if (str) free(str); }
	static inline int Predicate ( const void* str1, const void* str2 )	{
		return strcmp((char*)str1, (char*)str2);
	}

  void Resize(int nSize)
	{
	  Empty();

 		if (nSize > 0)
		{
			m_str = (char*)malloc(nSize*sizeof(char));
			*m_str=0;
		}
	}

};

VS_DEPRECATED inline string_view SimpleStrToStringView(const VS_SimpleStr& s) noexcept { return { s.m_str, (size_t)s.Length() }; }
VS_DEPRECATED inline string_view SimpleStrToStringView(const char*) noexcept = delete;

VS_DEPRECATED inline VS_SimpleStr StringViewToSimpleStr(string_view s) { return { (int)s.length(), s.data() }; }
VS_DEPRECATED inline VS_SimpleStr StringViewToSimpleStr(const char*) = delete;

#endif // VS_SIMPLESTR_H
