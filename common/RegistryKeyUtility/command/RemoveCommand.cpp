#include "RemoveCommand.h"

#include "std/cpplib/VS_RegistryKey.h"
#include "../entity/ErrorHolder.h"
#include "../exceptions/CommandException.h"
#include "../constants/Constants.h"
#include "../utils/utils.h"


ErrorHolder RemoveCommand::ParseCommandImpl(command_t first)
{
	ErrorHolder error;
	if (first->c_str())
	{
		if (is_valid_key_name(*first))
		{
			auto str = cut_front_end_delimeter(*first);
			if (!str.empty())
			{
				keyName_ = std::string(str);
			}
		}
		else
		{
			error.AddError(KNAME, std::string{ INPUT_NOT_SUPPORTED_NAME_ERROR_MESSAGE } +": " + *first);
		}
	}
	return error;
}


void RemoveCommand::ExecuteImpl()
{
	VS_RegistryKey reg_key{ false, EMPTY_STR, false };
	if (reg_key.IsValid())
	{
		if (!reg_key.RemoveKey(keyName_))
		{
			throw CommandException(__LINE__, __FILE__, "Error remove key: " + keyName_);
		}
	}
	else
	{
		throw CommandException(__LINE__, __FILE__, "Error open root key");
	}
}

const char* RemoveCommand::GetNameCommnad() const
{
	return REMOVE;
}
