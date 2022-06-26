#include "LoadCommand.h"
#include "std/cpplib/VS_RegistryKey.h"

#include "../constants/Constants.h"
#include "../storage/Storage.h"
#include "../entity/ErrorHolder.h"
#include "../exceptions/CommandException.h"
#include "../exceptions/StorageException.h"
#include "../utils/utils.h"

#include <algorithm>

LoadCommand::LoadCommand(std::unique_ptr<Storage> storage)
	: storage_(std::move(storage))
{
}

LoadCommand::~LoadCommand()
{
	LoadCommand::CompletedExecute();
}

void LoadCommand::ExecuteImpl()
{
	Storage::storage_args objs;
	try
	{
		objs = this->storage_->LoadFromStorage();
	}
	catch (const StorageException &ex)
	{
		throw CommandException(__LINE__, __FILE__, EXECUTE_LAOD_COMMAND_ERROR_MESSAGE, ex);
	}
	if (!objs.empty())
	{
		std::sort(objs.begin(), objs.end(),
			[](const ValueKey &a, const ValueKey &b) -> bool
		{
			return a.GetName().keyName < b.GetName().keyName;
		});

		std::string name;
		VS_RegistryKey key;
		for (auto &item : objs)
		{
			if (!key.IsValid() || item.GetName().keyName != name)
			{
				name.reserve(keyName_.length() + 1 + item.GetName().keyName.length());
				name.clear();
				name += keyName_;
				if (!keyName_.empty() && !item.GetName().keyName.empty())
					name += DELIMITER;
				name += item.GetName().keyName;
				key = VS_RegistryKey{ false, name, false ,true };
			}
			if (!SetValue(ValueKey{ { EMPTY_STR,  item.GetName().valueName },
				item.GetType(), item.GetValue() }, key))
			{
				throw CommandException(__LINE__, __FILE__,
					std::string{ SET_VALUE_TO_ERROR_MESSAGE } +": " + name + ", value: " + item.GetValue());
			}
		}
	}
}

ErrorHolder LoadCommand::ParseCommandImpl(command_t first)
{
	ErrorHolder errors;

	if (!first->empty())
	{
		if (is_valid_key_name(*first))
		{
			auto &&tmp_str = cut_front_end_delimeter(*first);
			if (!tmp_str.empty())
			{
				keyName_ = std::string(tmp_str);
			}
		}
		else
		{
			errors.AddError(KNAME, std::string{ INPUT_NOT_SUPPORTED_NAME_ERROR_MESSAGE } +": " + *first);
		}
	}

	++first;
	if (!storage_->OpenStorage(first->c_str(), true))
	{
		errors.AddError(FILE_STR, std::string(FILE_NOT_FOUNT_OR_BAD_ERROR_MESSAGE) + ": " + *first);
	}
	return errors;
}

const char* LoadCommand::GetNameCommnad() const
{
	return LOAD;
}

void LoadCommand::CompletedExecute()
{
	if (IsValid() && storage_->IsValidStorage())
	{
		storage_->CloseStorage();
	}
}

