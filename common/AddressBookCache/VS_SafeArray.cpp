#include "VS_SafeArray.h"

VS_SafeArray::VS_SafeArray(size_t len)
	: m_psa(true), m_cpsa(0), m_len(len), m_back(0), m_front(0)
{
	m_cpsa = SafeArrayCreateVector(VT_VARIANT, 0, len);
	var.Clear();
	var.parray = m_cpsa;
	var.vt = VT_ARRAY | VT_VARIANT;
}

VS_SafeArray::VS_SafeArray(VARIANT *pVar)
	: m_psa(false), m_cpsa(0), m_len(0), m_back(0), m_front(0)
{
	var = pVar;
	m_cpsa = var.parray; elem.Clear();
	LONG l(0), u(0);
	SafeArrayGetLBound(m_cpsa, 1, &l);
	SafeArrayGetUBound(m_cpsa, 1, &u);
	m_len = u - l;
}


VS_SafeArray::~VS_SafeArray()
{
//	if (m_psa) SafeArrayDestroy(m_cpsa);
}

bool VS_SafeArray::put(size_t index, const _variant_t &elem)
{
	LONG _i(index); VARIANT vr = elem;
	return (S_OK == SafeArrayPutElement(m_cpsa, &_i, &vr));
}

const _variant_t& VS_SafeArray::get(size_t index)
{
	LONG _i(index); VARIANT vr;
	HRESULT hr = SafeArrayGetElement(m_cpsa, &_i, &vr);
	if (S_OK == hr)
	{
		elem.Attach(vr);
	} else {
		elem.Clear();
	}
	return elem;
}

size_t VS_SafeArray::len()
{
	return m_len;
}

bool VS_SafeArray::push_back(const _variant_t &elem)
{
	return put(m_back++, elem);
}

const _variant_t& VS_SafeArray::pop_front()
{
	return get(m_front++);
}

void VS_SafeArray::reset()
{
	m_back = m_front = 0;
}

VARIANT &VS_SafeArray::collection()
{
	return var.GetVARIANT();
}
