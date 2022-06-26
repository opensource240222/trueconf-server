#include "ValueKey.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "../constants/Constants.h"
#include <boost/lexical_cast.hpp>
#include "std/cpplib/base64.h"

#include "std-generic/compat/memory.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////////
static const struct
{
	const char *nameType;
	ValueKey::ValueType type;
}STR_TYPE[] =
{
	{ TYPE_I32, ValueKey::ValueType::i32 },
	{ TYPE_I64, ValueKey::ValueType::i64 },
	{ TYPE_STR, ValueKey::ValueType::str },
	{ TYPE_BIN, ValueKey::ValueType::bin },
	{ TYPE_B64, ValueKey::ValueType::b64 }
};

//////////////////////////////////////////////////////////////////////////

ValueKey::ValueKey(ValueKey::ValueName name, const ::ValueKey::ValueType type, std::string value)
	: name_(std::move(name)),
	type_(type),
	value_(std::move(value))
{
}

const char* ValueKey::ValueTypeToStr(const ValueType type)
{
	const int index = static_cast<std::underlying_type<ValueKey::ValueType>::type>(type);
	if (index > -1)
	{
		return TYPES[index];
	}
	return EMPTY_STR;
}


ValueKey::ValueType ValueKey::StrToValueType(const char* type)
{
	for (auto &&item : STR_TYPE)
	{
		if (strcmp(type, item.nameType) == 0)
		{
			return item.type;
		}
	}
	return ValueType::non_type;
}

inline void show(std::ostream& stream, const std::string &value)
{
	stream.write(value.c_str(), value.length());
}

void ValueKey::StreamShowData(std::ostream &stream) const
{
	switch (type_)
	{
	case ValueType::i32:
	case ValueType::i64:
	case ValueType::str: show(stream, value_); break;
	case ValueType::b64:
	{
		std::size_t output_len = 0;
		base64_encode(value_.c_str(), value_.length(), nullptr, output_len);
		auto output = vs::make_unique_default_init<char[]>(output_len);

		bool encode_success = base64_encode(value_.c_str(), value_.length(), output.get(), output_len);
		assert(encode_success);
		(void)encode_success;

		stream.write(output.get(), output_len);
	}
	break;
	default: assert(0);  break;
	}
}

const ValueKey::ValueName& ValueKey::GetName() const
{
	return name_;
}

void ValueKey::SetName(ValueName name)
{
	name_ = std::move(name);
}

const ValueKey::ValueType& ValueKey::GetType() const
{
	return type_;
}

void ValueKey::SetType(const ValueType type)
{
	type_ = type;
}

const std::string& ValueKey::GetValue() const
{
	return value_;
}

void ValueKey::SetValue(std::string value)
{
	value_ = std::move(value);
}

void ValueKey::SetKeyName(std::string kName)
{
	this->name_.keyName = std::move(kName);
}

void ValueKey::SetValueName(std::string vName)
{
	this->name_.valueName = std::move(vName);
}
