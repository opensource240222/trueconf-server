
#ifndef _VS_ENDPOINT_ID_H_
#define _VS_ENDPOINT_ID_H_

#include "VS_Endpoint.h"

#include <cstdint>
#include <cstring>

#if defined(_WIN32)
#	include <comutil.h>
#endif

class VS_EndpointId
{
	uint64_t m_int;
	char				m_char[16];
public:
	// consttuctors
	VS_EndpointId() {
		Empty();
	}
	VS_EndpointId(const VS_EndpointId &src) {
		*this = src;
	}
	VS_EndpointId(const uint64_t id) {
		*this = id;
	}
	VS_EndpointId(const char* id) {
		*this = id;
	}
#if defined(_WIN32)
	VS_EndpointId(const _variant_t& id) {
		*this = id;
	}
#endif
	// assign operators
	VS_EndpointId& operator=(const uint64_t id) {
		m_int = id;
		VS_ConvertEndpoint(m_int, m_char); m_char[15] = 0;
		return *this;
	}
	VS_EndpointId& operator=(const char* id) {
		return operator =(VS_ConvertEndpoint(id));
	}
#if defined(_WIN32)
	VS_EndpointId& operator=(const _variant_t& id) {
		unsigned __int64 i;
		if		(id.vt==VT_DECIMAL)
			i = id.decVal.Lo64;
		else if (id.vt==VT_EMPTY || id.vt==VT_NULL)
			i = 0;
		else
			i = V_UI8(&id);
		return operator =(i);
	}
#endif
	// typecast operators
	operator const void*() const {
		return this;
	}
	operator const char*() const {
		return m_char;
	}
	operator uint64_t() const {
		return m_int;
	}
#if defined(_WIN32)
	inline operator _variant_t() const {
		DECIMAL z;
		z.Hi32 = 0;
		z.signscale = 0;
		z.Lo64 = m_int;
		return _variant_t (z);
	}
#endif
	// boolean operators
	bool operator! () const							{ return m_int == 0;	 }
	bool operator< (const VS_EndpointId& id) const 	{ return m_int<id.m_int; }
	bool operator> (const VS_EndpointId& id) const	{ return m_int>id.m_int; }
	bool operator!=(const VS_EndpointId& id) const 	{ return m_int!=id.m_int;}
	bool operator==(const VS_EndpointId& id) const	{ return m_int==id.m_int;}
	// methods
	void Empty() {
		m_int = 0;
		memset(m_char, 0, sizeof(m_char));
	}
	// increment
	inline VS_EndpointId& operator++(int) {
		m_int++;
		return operator =(m_int);
	}

	// allocator, deallocator
	static void* Factory(const void* p)	{
		return new VS_EndpointId(*static_cast<const VS_EndpointId*>(p));
	}
	static void  Destructor(void* p) {
		delete static_cast<VS_EndpointId*>(p);
	}
	static int Predicate(const void* id1, const void* id2) {
		auto l = static_cast<const VS_EndpointId*>(id1);
		auto r = static_cast<const VS_EndpointId*>(id2);
		return *l == *r ? 0 : *l < *r ? -1 : 1;
	}
};

#endif /*_VS_ENDPOINT_ID_H_*/