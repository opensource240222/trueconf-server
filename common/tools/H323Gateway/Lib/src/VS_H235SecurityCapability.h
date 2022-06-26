#pragma once

#include <vector>
#include <cstdint>
#include <tuple>
#include "std-generic/cpplib/VS_Container.h"

#include "VS_H235Enumerations.h"

struct VS_H235SecurityCapability{
	VS_H235SecurityCapability(){}
	VS_H235SecurityCapability(std::vector<uint8_t> &_h235_sessionKey, encryption_mode _m, unsigned _syncFlag) : h235_sessionKey(_h235_sessionKey), m(_m), syncFlag(_syncFlag){}
	VS_H235SecurityCapability(encryption_mode _m, unsigned _syncFlag) : m(_m), syncFlag(_syncFlag){}

	bool Serialize(VS_Container& cnt, const char* name) const{
		VS_Container sub_cnt;
		if (!sub_cnt.AddValue("encryption_mode", static_cast<int32_t>(m))) return false;

		if (m != encryption_mode::no_encryption){
			if (!sub_cnt.AddValue("syncFlag", static_cast<int32_t>(syncFlag))) return false;
			sub_cnt.AddValue("h235_sessionKey", static_cast<const void *>(h235_sessionKey.data()), h235_sessionKey.size());
		}
		return cnt.AddValue(name, sub_cnt);
	}
	bool Deserialize(const VS_Container& cnt, const char* name){
		VS_Container sub_cnt;
		if (!(name ? cnt.GetValue(name, sub_cnt) : cnt.GetValue(sub_cnt)))
			return false;

		int32_t m_value;
		if (!sub_cnt.GetValue("encryption_mode", m_value)) return false;
		m = decltype(m)(m_value);

		if (m != encryption_mode::no_encryption)
		{
			int32_t syncFlag_value;
			const uint8_t *h235_key(nullptr);
			std::size_t key_size(0);
			if (!sub_cnt.GetValue("syncFlag", syncFlag_value))	return false;
			syncFlag = decltype(syncFlag)(syncFlag_value);
			h235_key = static_cast<const uint8_t *>(sub_cnt.GetBinValueRef("h235_sessionKey", key_size));
			if (h235_key) h235_sessionKey.assign(h235_key, h235_key + key_size);
		}

		return true;
	}

	bool operator==(const VS_H235SecurityCapability& other)const{
		return std::tie(this->h235_sessionKey, this->m, this->syncFlag) == std::tie(other.h235_sessionKey, other.m, other.syncFlag);
	}

	std::vector<uint8_t>	h235_sessionKey;
	encryption_mode m = encryption_mode::no_encryption;
	unsigned syncFlag = 0;
};