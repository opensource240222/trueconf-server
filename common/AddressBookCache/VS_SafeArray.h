#ifndef VS_SAFEARRAY_H
#define VS_SAFEARRAY_H

#include <windows.h>
#include <comutil.h>

class VS_SafeArray
{
private:
	bool m_psa;
	SAFEARRAY *m_cpsa;
	size_t m_len, m_back, m_front;
	_variant_t var, elem;

public:
	VS_SafeArray(size_t len);
	VS_SafeArray(VARIANT *pVar);
	bool put(size_t index, const _variant_t &elem);
	const _variant_t& get(size_t index);
	bool push_back(const _variant_t &elem);
	const _variant_t& pop_front();
	void reset();
	VARIANT &collection();
	size_t len();
	~VS_SafeArray();
};

#endif