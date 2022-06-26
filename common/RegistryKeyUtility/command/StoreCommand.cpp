#include "StoreCommand.h"

#include "std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/scope_exit.h"

#include "../constants/Constants.h"
#include "../storage/Storage.h"
#include "../entity/ErrorHolder.h"
#include "../exceptions/CommandException.h"
#include "../exceptions/StorageException.h"
#include "../utils/utils.h"

#include <cstdlib>

StoreCommand::StoreCommand(std::unique_ptr<Storage> storage,
	std::shared_ptr<const Extractor<ValueKey, const void*, const RegistryVT, name_extracrt,
	const std::size_t>> extractor)
	: storage_(std::move(storage)), extractor_(std::move(extractor))
{
}


void StoreCommand::ExecuteImpl()
{
	VS_RegistryKey reg_key{ false, keyName_ };
	if (reg_key.IsValid())
	{
		const auto res_val = GetData(reg_key);
		try {
			storage_->SaveToStorage(res_val);
		}
		catch (const StorageException &ex)
		{
			throw CommandException(__LINE__, __FILE__, "Error Execute StoreCommand", ex);
		}
	}
	else
	{
		throw CommandException(__LINE__, __FILE__, "Error store key: " + keyName_);
	}
}

StoreCommand::~StoreCommand()
{
	StoreCommand::CompletedExecute();
}


ErrorHolder StoreCommand::ParseCommandImpl(command_t first)
{
	ErrorHolder errors;
	if (!first->empty())
	{
		string_view tmp_str = *first;
		if (is_valid_key_name(*first))
		{
			tmp_str = cut_front_end_delimeter(tmp_str);
		}
		else
		{
			errors.AddError(KNAME, std::string{ INPUT_NOT_SUPPORTED_NAME_ERROR_MESSAGE } +": " + *first);
		}

		if (!tmp_str.empty())
		{
			this->keyName_ = std::string(tmp_str);
		}
	}

	++first;
	if (!storage_->OpenStorage(first->c_str()))
	{
		errors.AddError(STORE, std::string(FILE_NOT_FOUNT_OR_BAD_ERROR_MESSAGE) + ": " + *first);
	}
	return errors;
}

void StoreCommand::CompletedExecute()
{
	if (IsValid() && storage_->IsValidStorage())
	{
		storage_->CloseStorage();
	}
}

const char* StoreCommand::GetNameCommnad() const
{
	return STORE;
}

Storage::storage_args StoreCommand::GetData(VS_RegistryKey& key, string_view name) const
{
	Storage::storage_args data;

	std::unique_ptr<void, free_deleter> value;
	RegistryVT type;
	std::string name_val;
	int32_t offset;

	key.ResetValues();
	while ((offset = key.NextValueAndType(value, type, name_val)) > 0)
	{
		data.push_back(extractor_->Extract(value.get(), type, name_extracrt{ name.substr(0, name.empty() ? 0 : name.length() - 1),
			std::move(name_val) }, offset));
	}

	VS_RegistryKey sub_key;
	key.ResetKey();

	std::string new_name;
	while (key.NextKey(sub_key))
	{
		const auto len_sub_key = std::char_traits<char>::length(sub_key.GetName());
		new_name.reserve(name.size() + len_sub_key + 1);
		new_name = std::string(name);

		new_name += sub_key.GetName();
		new_name += DELIMITER;

		auto tmp = GetData(sub_key, new_name);
		if (!tmp.empty())
		{
			data.reserve(data.size() + tmp.size());
			std::move(std::begin(tmp), std::end(tmp), std::back_inserter(data));
		}
	}
	return data;
}