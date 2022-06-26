#include "RenameCommand.h"
#include "std/cpplib/VS_RegistryKey.h"

#include "../constants/Constants.h"
#include "../entity/ErrorHolder.h"
#include "../exceptions/CommandException.h"
#include "../utils/utils.h"

void RenameCommand::ExecuteImpl()
{
	VS_RegistryKey key{ false, EMPTY_STR, false };
	if (key.IsValid())
	{
		if (!key.RenameKey(keyName_, newKeyName_))
		{
			throw CommandException(__LINE__, __FILE__, std::string{ RENAME_KEY_ERROR_MESSAGE }
			+": old - " + keyName_ + " new - " + newKeyName_);
		}
	}
	else
	{
		throw CommandException(__LINE__, __FILE__, "Error open key: " + keyName_);
	}
}


ErrorHolder RenameCommand::ParseCommandImpl(command_t first)
{
	ErrorHolder errors;
	if (!first->empty())
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
			errors.AddError(KNAME, std::string{ INPUT_NOT_SUPPORTED_NAME_ERROR_MESSAGE } +": " + *first);
		}
	}
	else
	{
		errors.AddError(KNAME, INPUT_NOT_SUPPORTED_EMPTY_STR_ERROR_MESSAGE);
	}

	++first;
	if (!first->empty())
	{
		if (is_valid_key_name(*first))
		{
			auto str_tmp = cut_front_end_delimeter(*first);
			if (!str_tmp.empty())
			{
				newKeyName_ = std::string(str_tmp);
			}
		}
		else
		{
			errors.AddError(NEW_KNAME, std::string{ INPUT_NOT_SUPPORTED_NAME_ERROR_MESSAGE } +": " + *first);
		}
	}
	else
	{
		errors.AddError(NEW_KNAME, INPUT_NOT_SUPPORTED_EMPTY_STR_ERROR_MESSAGE);
	}
	return errors;
}

const char* RenameCommand::GetNameCommnad() const
{
	return RENAME;
}
