#include "RegistryKeyCommand.h"

#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/base64.h"
#include "std/cpplib/numerical.h"

#include "../constants/Constants.h"

#include "std-generic/compat/memory.h"
#include <cassert>
#include <cstdlib>

bool RegistryKeyCommand::SetValue(const ValueKey &value, VS_RegistryKey& regKey) const
{
	switch (value.GetType())
	{
	case ValueKey::ValueType::i32:
	{
		const auto& str = value.GetValue();
		int32_t val32;
		char* end = nullptr;
		if (str[0] == '-')
			val32 = clamp_cast<int32_t>(std::strtol(str.c_str(), &end, 10));
		else
			val32 = clamp_cast<uint32_t>(std::strtoul(str.c_str(), &end, 10));
		if (end == str)
			return false;
		return regKey.SetValue(&val32, sizeof(val32), VS_REG_INTEGER_VT, value.GetName().valueName.c_str());
	}
	break;
	case ValueKey::ValueType::i64:
	{
		const auto& str = value.GetValue();
		int64_t val64;
		char* end = nullptr;
		if (str[0] == '-')
			val64 = clamp_cast<int64_t>(std::strtoll(str.c_str(), &end, 10));
		else
			val64 = clamp_cast<uint64_t>(std::strtoull(str.c_str(), &end, 10));
		if (end == str)
			return false;
		return regKey.SetValue(&val64, sizeof(val64), VS_REG_INT64_VT, value.GetName().valueName.c_str());
	}
	break;
	case ValueKey::ValueType::str:
		return regKey.SetString(value.GetValue().c_str(), value.GetName().valueName.c_str());
	case ValueKey::ValueType::bin:
		return regKey.SetValue(value.GetValue().c_str(),
			value.GetValue().length(), VS_REG_BINARY_VT, value.GetName().valueName.c_str());
	case ValueKey::ValueType::b64:
	{
		const std::size_t input_len = value.GetValue().length();
		std::size_t output_len;
		base64_decode(value.GetValue().c_str(), input_len, nullptr, output_len);
		if (output_len > 0)
		{
			auto output = vs::make_unique_default_init<char[]>(output_len);

			if (base64_decode(value.GetValue().c_str(), input_len, output.get(), output_len))
			{
				return regKey.SetValue(output.get(), output_len,
					VS_REG_BINARY_VT, value.GetName().valueName.c_str());
			}
			else
			{
				assert(0);
				return false;
			}
		}
		else
		{
			return regKey.SetValue(EMPTY_STR, 0, VS_REG_BINARY_VT, value.GetName().valueName.c_str());
		}
	}
	break;
	}
	return false;
}

void RegistryKeyCommand::CompletedExecute()
{
}

//////////////////////////////////////////////////////////////////////////

ValueKey::ValueType RegistryKeyCommand::RegistryVtToValueType(const RegistryVT vt)
{
	switch (vt)
	{
	case VS_REG_INTEGER_VT: return ValueKey::ValueType::i32;
	case VS_REG_INT64_VT:	return ValueKey::ValueType::i64;
	case VS_REG_STRING_VT:	return ValueKey::ValueType::str;
	case VS_REG_BINARY_VT:	return ValueKey::ValueType::b64;
	default:				return ValueKey::ValueType::non_type;
	}
}

//////////////////////////////////////////////////////////////////////////