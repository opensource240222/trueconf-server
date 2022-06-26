#include "ExtractorValueKey.h"

ValueKey ExtractorValueKey::Extract(const void* value, const RegistryVT type,
	name_extracrt name, const std::size_t offset) const
{
	std::string result_str;
	result_str.reserve(offset);
	const auto res_type = RegistryKeyCommand::RegistryVtToValueType(type);

	switch (res_type)
	{
	case ValueKey::ValueType::i32:
	{
		result_str = std::to_string(*static_cast<const int32_t*>(value));
		break;
	}
	case ValueKey::ValueType::i64:
	{
		result_str = std::to_string(*static_cast<const int64_t*>(value));
		break;
	}
	case ValueKey::ValueType::str:
	{
		result_str = std::string{ static_cast<const char *>(value), offset - 1 };
		break;
	}
	default:
	{
		result_str = std::string{ static_cast<const char *>(value), offset };
		break;
	}
	}
	return { ValueKey::ValueName{ std::string(name.first), std::string(name.second) },
		res_type, std::move(result_str) };
}