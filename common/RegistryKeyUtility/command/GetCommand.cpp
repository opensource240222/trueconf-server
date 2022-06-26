#include "GetCommand.h"

#include "std/cpplib/VS_RegistryKey.h"
#include "../entity/ErrorHolder.h"
#include "../exceptions/CommandException.h"
#include "../constants/Constants.h"
#include "../utils/utils.h"

#include <cstdlib>

GetCommand::GetCommand(std::ostream& out,
	std::shared_ptr<const Extractor<ValueKey, const void*, const RegistryVT, name_extracrt, const std::size_t>> extractor)
	: extractor_(std::move(extractor)), out_(out)
{
}

void GetCommand::ExecuteImpl()
{
	VS_RegistryKey reg{ false, name_.keyName };
	if (reg.IsValid())
	{
		std::unique_ptr<void, free_deleter> value;
		RegistryVT type;
		int32_t offset;
		if ((offset = reg.GetValueAndType(value, type, name_.valueName.c_str())) > 0)
		{
			auto obj = extractor_->Extract(value.get(), type, { name_.keyName, name_.valueName }, offset);
			obj.StreamShowData(out_);
		}
	}
	else
	{
		throw CommandException(__LINE__, __FILE__, "Error Key root:" + VS_RegistryKey::GetDefaultRoot()
			+ ", key name: " + name_.keyName + ", valeue name: " + name_.valueName);
	}
}

const char* GetCommand::GetNameCommnad() const
{
	return GET;
}

ErrorHolder GetCommand::ParseCommandImpl(command_t first)
{
	ErrorHolder error;
	if (!first->empty())
	{
		if (is_valid_key_name(*first))
		{
			auto str_tmp = cut_front_end_delimeter(*first);
			if (!str_tmp.empty())
			{
				name_.keyName = std::string(str_tmp);
			}
		}
		else
		{
			error.AddError(KNAME, std::string{ INPUT_NOT_SUPPORTED_NAME_ERROR_MESSAGE } +": " + *first);
		}
	}

	++first;
	if (!first->empty())
	{
		name_.valueName = std::move(*first);
	}

	return error;
}
