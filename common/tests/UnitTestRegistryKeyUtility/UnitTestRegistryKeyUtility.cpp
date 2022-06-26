#include "UnitTestRegistryKeyUtility.h"

#include <boost/algorithm/string/trim.hpp>

#include "RegistryKeyUtility/exceptions/ExtractorException.h"
#include "RegistryKeyUtility/command/GetCommand.h"
#include "RegistryKeyUtility/command/SetCommand.h"
#include "RegistryKeyUtility/command/RemoveCommand.h"
#include "RegistryKeyUtility/command/RenameCommand.h"
#include "RegistryKeyUtility/exceptions/CommandException.h"

TEST_P(TestExtractorParamPositive, TestExtractorPositive)
{
	auto &&param = GetParam();
	CommandParams &&res = param.extractor->Extract(param.argc, param.argv);

	EXPECT_STREQ(res.GetOptions().backend.c_str(), param.params.GetOptions().backend.c_str());
	EXPECT_STREQ(res.GetOptions().root.c_str(), param.params.GetOptions().root.c_str());

	auto i = res.GetCommands().cbegin();
	auto j = param.params.GetCommands().cbegin();
	for (; i != res.GetCommands().cend() && j != param.params.GetCommands().cend(); ++i, ++j)
	{
		EXPECT_STREQ(i->nameCommand.c_str(), j->nameCommand.c_str());
		EXPECT_TRUE(std::equal(i->commandArgs.cbegin(), i->commandArgs.cend(), j->commandArgs.begin()));
	}
	EXPECT_TRUE((i == res.GetCommands().cend() && j == param.params.GetCommands().cend()));
}

TEST_P(TestExtractorParamNegative, TestExtractorNegative)
{
	auto &&param = GetParam();
	ASSERT_THROW((param.extractor->Extract(param.argc, param.argv)), ExtractorException);
}


TEST_P(TestExtractorParamHelp, TestExtractorPositive)
{
	auto &&param = GetParam();
	auto &&res = param.extractor->Extract(param.argc, param.argv);

	ASSERT_TRUE(!res.GetOptions().help.empty());
}


TEST_P(ExtractorValueKeyTest, ValueKeyExtactor)
{
	auto &&param = GetParam();
	ValueKey &&res = param.extractor->Extract(param.value, param.type, param.name, param.offset);

	EXPECT_STREQ(param.expect.GetName().keyName.c_str(), res.GetName().keyName.c_str());
	EXPECT_STREQ(param.expect.GetName().valueName.c_str(), res.GetName().valueName.c_str());
	EXPECT_TRUE(param.expect.GetType() == res.GetType());
	EXPECT_STREQ(param.expect.GetValue().c_str(), res.GetValue().c_str());
}

TEST_P(TestValidatorParams, ValidatorParamsTest)
{
	EXPECT_TRUE(GetParam().validator->Validation(GetParam().params).IsEmpty());
}


TEST_P(TestStorage, Storage)
{
	const StorageWrapper &param = GetParam();
	ASSERT_TRUE(param.storage->OpenStorage(param.pathStorage));

	param.storage->SaveToStorage(param.valueKey);
	auto &&res = param.storage->LoadFromStorage();

	auto &&i = res.cbegin();
	auto &&j = param.valueKey.cbegin();
	for (; i != res.cend() && j != param.valueKey.cend(); ++i, ++j)
	{
		EXPECT_STREQ(i->GetName().keyName.c_str(), j->GetName().keyName.c_str());
		EXPECT_STREQ(i->GetName().valueName.c_str(), j->GetName().valueName.c_str());
		EXPECT_TRUE(i->GetType() == j->GetType());
		EXPECT_STREQ(i->GetValue().c_str(), j->GetValue().c_str());
	}
	EXPECT_TRUE((i == res.cend() && j == param.valueKey.cend()));
	EXPECT_TRUE(param.storage->CloseStorage());
}

//////////////////////////////////////////////////////////////////////////

TEST_P(SetCommandTest, TestSetCommand)
{
	const SetCommandWrapper &param = GetParam();
	auto &&command = param.commandItem;

	ASSERT_TRUE(command->ParseCommand(param.commands.cbegin()).IsEmpty());
	command->Execute();
	command->CompletedExecute();

	VS_RegistryKey key{ false, param.name.first };
	ASSERT_TRUE(key.IsValid());

	std::unique_ptr<void, free_deleter> value;
	RegistryVT type;
	int32_t offset = key.GetValueAndType(value, type, param.name.second.c_str());
	ASSERT_GE(offset, 0);

	ValueKey &&res = param.extractor.extractor->Extract(value.get(), type, name_extracrt{param.name.first, param.name.second }, offset);
	EXPECT_STREQ(param.name.first.c_str(), res.GetName().keyName.c_str());
	EXPECT_STREQ(param.name.second.c_str(), res.GetName().valueName.c_str());
	ASSERT_STREQ(res.GetValue().c_str(), param.expectValue.c_str());
}

//////////////////////////////////////////////////////////////////////////

TEST_P(GetCommandTest, TestGetCommand)
{
	const GetCommandWrapper &param = GetParam();

	ASSERT_TRUE(param.setCommand->ParseCommand(param.setArgs.cbegin()).IsEmpty());
	ASSERT_NO_THROW(param.setCommand->Execute());
	param.setCommand->CompletedExecute();

	ASSERT_TRUE(param.commandItem->ParseCommand(param.commands.cbegin()).IsEmpty());
	ASSERT_NO_THROW(param.commandItem->Execute());
	param.commandItem->CompletedExecute();

	std::string res = { Stream.str() };
	boost::trim_right(res);

	ASSERT_STREQ(res.c_str(), param.expectValue.c_str());
}

//////////////////////////////////////////////////////////////////////////

static const char REMOVE_SET_KNAME[] = "remove";
static const char REMOVE_SET_VNAME[] = "remove_name";
static const char REMOVE_SET_TYPE[] = "str";
static const char REMOVE_SET_VALUE[] = "remove_val";

TEST_F(CommandTest, RemoveCommandTest)
{
	SetCommand set_command;
	CommandParams::CommandParamsItem::args_t args_set = { REMOVE_SET_KNAME, REMOVE_SET_VNAME, REMOVE_SET_TYPE, REMOVE_SET_VALUE };
	ASSERT_TRUE(set_command.ParseCommand(args_set.cbegin()).IsEmpty());
	set_command.Execute();
	set_command.CompletedExecute();

	GetCommand get_command(Stream, EXTRACTOR_VAL_KEY);
	CommandParams::CommandParamsItem::args_t args_name = { REMOVE_SET_KNAME, REMOVE_SET_VNAME };
	ASSERT_TRUE(get_command.ParseCommand(args_name.cbegin()).IsEmpty());
	ASSERT_NO_THROW(get_command.Execute());
	get_command.CompletedExecute();

	RemoveCommand remove_command;
	CommandParams::CommandParamsItem::args_t remove_args{ REMOVE_SET_KNAME };
	ASSERT_TRUE(remove_command.ParseCommand(args_name.cbegin()).IsEmpty());
	ASSERT_NO_THROW(remove_command.Execute());
	remove_command.CompletedExecute();

	ASSERT_TRUE(get_command.ParseCommand(args_name.cbegin()).IsEmpty());
	ASSERT_THROW(get_command.Execute(), CommandException);

}

////////////////////////////////////////////////////////////////////////////////

static const char RENAME_SET_KNAME[] = "rename";
static const char RENAME_SET_VNAME[] = "rename_name";
static const char RENAME_KNAME[] = "rename";
static const char RENAME_NEW_KNAME[] = "new_name";
static const char RENAME_GET_NEW_KNAME[] = "new_name";
static const char RENAME_GET_NEW_VNAME[] = "rename_name";
static const char RENAME_SET_TYPE[] = "str";
static const char RENAME_SET_VALUE[] = "rename_val";

TEST_F(CommandTest, RenameCommandTest)
{
	SetCommand set_command;
	CommandParams::CommandParamsItem::args_t args_set = { RENAME_SET_KNAME, RENAME_SET_VNAME, RENAME_SET_TYPE, RENAME_SET_VALUE };
	ASSERT_TRUE(set_command.ParseCommand(args_set.cbegin()).IsEmpty());
	ASSERT_NO_THROW(set_command.Execute());
	set_command.CompletedExecute();

	GetCommand get_command(Stream, EXTRACTOR_VAL_KEY);
	CommandParams::CommandParamsItem::args_t args_name = { RENAME_SET_KNAME,  RENAME_SET_VNAME };
	ASSERT_TRUE(get_command.ParseCommand(args_name.cbegin()).IsEmpty());
	ASSERT_NO_THROW(get_command.Execute());
	get_command.CompletedExecute();
	ASSERT_STREQ(Stream.str().c_str(), RENAME_SET_VALUE);

	CommandTest::TearDownTestCase();

	RenameCommand rename_command;
	CommandParams::CommandParamsItem::args_t args_rename = { RENAME_KNAME, RENAME_NEW_KNAME };
	ASSERT_TRUE(rename_command.ParseCommand(args_rename.cbegin()).IsEmpty());
	ASSERT_NO_THROW(rename_command.Execute());
	rename_command.CompletedExecute();

	CommandParams::CommandParamsItem::args_t args_get_new_name = { RENAME_GET_NEW_KNAME, RENAME_GET_NEW_VNAME };
	ASSERT_TRUE(get_command.ParseCommand(args_get_new_name.cbegin()).IsEmpty());
	ASSERT_NO_THROW(get_command.Execute());
	get_command.CompletedExecute();

	ASSERT_STREQ(Stream.str().c_str(), RENAME_SET_VALUE);
}

//////////////////////////////////////////////////////////////////////////


TEST_P(StoreLoadCommandTest, TestStoreLoadCommand)
{
	auto &&param = GetParam();

	SetCommand set_command;
	for (auto &&item : param.setArgs)
	{
		ASSERT_TRUE(set_command.ParseCommand(item.cbegin()).IsEmpty());
		ASSERT_NO_THROW(set_command.Execute());
		set_command.CompletedExecute();
	}

	VS_RegistryKey::TEST_DumpData();
	std::vector<std::string> args_store = { "store", STORE_LOAD_FILE };
	ASSERT_TRUE(param.storeCommand->ParseCommand(args_store.cbegin()).IsEmpty());
	ASSERT_NO_THROW(param.storeCommand->Execute());
	param.storeCommand->CompletedExecute();

	std::vector<std::string> args_load = { "load", STORE_LOAD_FILE };
	ASSERT_TRUE(param.loadCommand->ParseCommand(args_load.cbegin()).IsEmpty());
	ASSERT_NO_THROW(param.loadCommand->Execute());
	param.loadCommand->CompletedExecute();

	GetCommand get_command(Stream, EXTRACTOR_VAL_KEY);

	for (auto &&item : param.expectStorage)
	{
		CommandTest::TearDownTestCase();
		std::vector<std::string> args = { item.GetName().keyName, item.GetName().valueName };
		ASSERT_TRUE(get_command.ParseCommand(args.cbegin()).IsEmpty());
		ASSERT_NO_THROW(get_command.Execute());
		get_command.CompletedExecute();

		EXPECT_STREQ(Stream.str().c_str(), item.GetValue().c_str());
	}

}

//////////////////////////////////////////////////////////////////////////
