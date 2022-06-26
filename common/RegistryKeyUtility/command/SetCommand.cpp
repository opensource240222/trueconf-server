#include "SetCommand.h"

#include <boost/lexical_cast.hpp>
#include "std/cpplib/VS_RegistryKey.h"

#include "../constants/Constants.h"
#include "../entity/CommandParams.h"
#include "../entity/ErrorHolder.h"
#include "../exceptions/CommandException.h"
#include "../utils/utils.h"


ErrorHolder SetCommand::ParseCommandImpl(command_t first)
{
	ErrorHolder errors;

	if (!first->empty())
	{
		string_view str = *first;
		if (is_valid_key_name(*first))
		{
			str = cut_front_end_delimeter(str);
			if (!str.empty())
			{
				this->valueKey_.SetKeyName(std::string(str));
			}
		}
		else
		{
			errors.AddError(KNAME, std::string{ INPUT_NOT_SUPPORTED_NAME_ERROR_MESSAGE } +": " + *first);
		}

		if (!str.empty())
		{
			this->valueKey_.SetKeyName(std::string(str));
		}
	}

	++first;
	if(!first->empty())
	{
		this->valueKey_.SetValueName(std::move(*first));
	}


	++first;
	const auto type = ValueKey::StrToValueType(first->c_str());
	if (ValueKey::ValueType::non_type == type)
	{
		errors.AddError(TYPE, std::string{ TYPE_NOT_SUPPORTED_ERROR_MESSAGE } +": " + *first);
	}
	this->valueKey_.SetType(type);

	++first;
	if (ValueKey::ValueType::i64 == type || ValueKey::ValueType::i32 == type)
	{
		if (!is_int_numbers(first->c_str()))
		{
			errors.AddError(VALUE, std::string(NUMBER_ERROR_MESSAGE) + ": " + *first);
		}
	}
	else if (ValueKey::ValueType::b64 == type && !is_base64(first->c_str()))
	{
		errors.AddError(VALUE, NOT_BASE64_ERROR_MESSAGE + std::string(": ") + *first);
	}

	if (!first->empty())
	{
		this->valueKey_.SetValue(std::move(*first));
	}
	return errors;
}

void SetCommand::ExecuteImpl()
{
	VS_RegistryKey key(false, this->valueKey_.GetName().keyName, false, true);
	if (key.IsValid())
	{
		if (!SetValue(this->valueKey_, key))
		{
			throw CommandException(__LINE__, __FILE__, "Error SetValue value: " + std::string(valueKey_.GetValue()) + " type: "
				+ std::string(ValueKey::ValueTypeToStr(valueKey_.GetType())) + "root: " + VS_RegistryKey::GetDefaultRoot()
				+ ", key: " + valueKey_.GetName().keyName + ", value name: " + valueKey_.GetName().valueName);
		}
		return;
	}
	throw CommandException(__LINE__, __FILE__, "RegistryKey is not valid");
}

const char* SetCommand::GetNameCommnad() const
{
	return SET;
}
