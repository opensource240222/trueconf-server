#pragma once

#include "std/cpplib/VS_RegistryKey.h"

#include "RegistryKeyUtility/extractor/Extractor.h"
#include "RegistryKeyUtility/entity/CommandParams.h"
#include "RegistryKeyUtility/extractor/ExtractorParams.h"
#include "RegistryKeyUtility/constants/Constants.h"
#include "RegistryKeyUtility/command/RegistryKeyCommand.h"
#include "RegistryKeyUtility/extractor/ExtractorValueKey.h"
#include "RegistryKeyUtility/validator/Validator.h"
#include "RegistryKeyUtility/validator/ValidatorImpl.h"
#include "RegistryKeyUtility/storage/Storage.h"
#include "RegistryKeyUtility/storage/StorageCSV.h"
#include "RegistryKeyUtility/command/SetCommand.h"
#include "RegistryKeyUtility/command/GetCommand.h"
#include "RegistryKeyUtility/command/StoreCommand.h"
#include "RegistryKeyUtility/command/LoadCommand.h"
#include "RegistryKeyUtility/entity/ValueKey.h"

#include <gtest/gtest.h>

#include "std-generic/compat/iterator.h"
#include "std-generic/compat/memory.h"

//////////////////////////////////////////////////////////////////////////

template<typename... Args>
struct ExtractorWrapper
{
	explicit ExtractorWrapper(std::shared_ptr<const Extractor<Args...>> extractor)
		: extractor(std::move(extractor))
	{
	}
	std::shared_ptr<const Extractor<Args...>> extractor;
};

struct ExtractorParamsWrapper : ExtractorWrapper<CommandParams, const int, const char* const[]>
{
	ExtractorParamsWrapper(const char* const argv[], const std::size_t argc,
		const std::shared_ptr<const Extractor<CommandParams, const int, const char* const[]>>& extractor)
		: ExtractorWrapper<CommandParams, const int, const char* const[]>(extractor),
		argv(argv),
		argc(argc)
	{
	}

	const char *const *argv;
	const std::size_t argc;
};


struct ExtractorParamExpectWrapper : ExtractorParamsWrapper
{
	ExtractorParamExpectWrapper(const char* const argv[], const std::size_t argc,
		const std::shared_ptr<const Extractor<CommandParams, const int, const char* const[]>> &extractor, CommandParams params)
		: ExtractorParamsWrapper(argv, argc, extractor),
		params(std::move(params))
	{
	}

	CommandParams params;
};
class TestExtractorParamNegative : public ::testing::TestWithParam<ExtractorParamsWrapper> {};
class TestExtractorParamPositive : public ::testing::TestWithParam<ExtractorParamExpectWrapper> {};

class TestExtractorParamHelp : public ::testing::TestWithParam<ExtractorParamsWrapper> {};


static const char TEST_ROOT[] = "Test\\Test";
static const char TEST_BACKEND[] = "TestBackend";
static const char TEST_KNAME[] = "test_kname";
static const char TEST_VNAME[] = "test_vname";
static const char TEST_VALUE[] = "test_value";

static const char TEST_BACKEND_FILE[] = "file_backend";
static const char TEST_BACKEND_FILE_PATH[] = "test_tc_server.cfg";

const char *const ARGS_1[] = { "0", "-r", TEST_ROOT, "-b", TEST_BACKEND };
const char *const ARGS_5[] = { "0", "-b", TEST_BACKEND, SET, TEST_KNAME, TEST_VNAME, TYPE_I32, TEST_VALUE, GET, TEST_KNAME, TEST_VNAME };
const char *const ARGS_10[] = { "0", "-c", TEST_BACKEND_FILE_PATH, "-r", TEST_ROOT,
	SET, TEST_KNAME, TEST_VNAME, TYPE_I32, TEST_VALUE, GET, TEST_KNAME, TEST_VNAME };

INSTANTIATE_TEST_CASE_P(Default, TestExtractorParamPositive,
	::testing::Values(
		ExtractorParamExpectWrapper
		{ ARGS_1, vs::size(ARGS_1), std::make_shared<const ExtractorParams>(), ([]()
			{
				CommandParams obj;
				obj.SetRoot(TEST_ROOT);
				obj.SetBackend(TEST_BACKEND);
				return obj;
			})()
		},
		ExtractorParamExpectWrapper
			{ ARGS_5, vs::size(ARGS_5), std::make_shared<const ExtractorParams>(), ([]()
				{
					CommandParams obj;
					obj.SetRoot(DEFAULT_ROOT);
					obj.SetBackend(TEST_BACKEND);
					obj.AddCommand({ SET,{ TEST_KNAME, TEST_VNAME, TYPE_I32, TEST_VALUE } });
					obj.AddCommand({ GET,{ TEST_KNAME, TEST_VNAME } });
					return obj;
				})()
			},
				ExtractorParamExpectWrapper
				{ ARGS_10, vs::size(ARGS_10), std::make_shared<const ExtractorParams>(), ([]()
					{
						CommandParams obj;
						obj.SetRoot(TEST_ROOT);
						obj.SetBackend(TEST_BACKEND_FILE);
						obj.AddCommand({ SET,{ TEST_KNAME, TEST_VNAME, TYPE_I32, TEST_VALUE } });
						obj.AddCommand({ GET,{ TEST_KNAME, TEST_VNAME } });
						return obj;
				   })()
				}
));


const char * const ARGS_6[] = { "0", "-b", TEST_BACKEND, "-h" , "-h", SET, TEST_KNAME, TEST_VNAME , TYPE_I32, TEST_VALUE, GET, TEST_KNAME, TEST_VNAME };
const char * const ARGS_7[] = { "0", "-b", TEST_BACKEND, SET, TEST_KNAME, TEST_VNAME , TYPE_I32, TEST_VALUE, TEST_VALUE, GET, TEST_KNAME, TEST_VNAME };
const char * const ARGS_8[] = { "0", "-", TEST_BACKEND, SET, TEST_KNAME, TEST_VNAME , TYPE_I32, TEST_VALUE, GET, TEST_KNAME, TEST_VNAME };
const char * const ARGS_9[] = { "0", "-z", TEST_BACKEND, SET, TEST_KNAME, TEST_VNAME , TYPE_I32, TEST_VALUE, GET, TEST_KNAME, TEST_VNAME };


INSTANTIATE_TEST_CASE_P(Default, TestExtractorParamNegative,
	::testing::Values(
		ExtractorParamsWrapper{ ARGS_6, vs::size(ARGS_6), std::make_shared<ExtractorParams>() },
		ExtractorParamsWrapper{ ARGS_7, vs::size(ARGS_7), std::make_shared<ExtractorParams>() },
		ExtractorParamsWrapper{ ARGS_8, vs::size(ARGS_8), std::make_shared<ExtractorParams>() },
		ExtractorParamsWrapper{ ARGS_9, vs::size(ARGS_9), std::make_shared<ExtractorParams>() }
));


static const char *const ARGS_2[] = { "0", "-h" };
static const char *const ARGS_3[] = { "0", "-r", TEST_ROOT, "-h" , GET, TEST_KNAME, TEST_VNAME };
static const char *const ARGS_4[] = { "0", "-h", "-r", TEST_ROOT, GET, TEST_KNAME, TEST_VNAME };


INSTANTIATE_TEST_CASE_P(Default, TestExtractorParamHelp,
	::testing::Values(
		ExtractorParamsWrapper{ ARGS_2, vs::size(ARGS_2), std::make_shared<const ExtractorParams>() },
		ExtractorParamsWrapper{ ARGS_3, vs::size(ARGS_3), std::make_shared<const ExtractorParams>() },
		ExtractorParamsWrapper{ ARGS_4, vs::size(ARGS_4), std::make_shared<const ExtractorParams>() }
));

//////////////////////////////////////////////////////////////////////////

struct ExtractorRegistryKeyCommandWrapper : ExtractorWrapper<ValueKey, const void*, const RegistryVT, name_extracrt,
	const std::size_t>
{
	explicit ExtractorRegistryKeyCommandWrapper(
		const std::shared_ptr<const Extractor<ValueKey, const void*, const RegistryVT, name_extracrt, const std::size_t>>
		&extractor)
		: ExtractorWrapper<ValueKey, const void*, const RegistryVT, name_extracrt,
		const std::size_t>(extractor)
	{
	}
};

struct ExtractorValueKeyExpectWrapper : ExtractorRegistryKeyCommandWrapper
{
	ExtractorValueKeyExpectWrapper(
		const std::shared_ptr < const Extractor < ValueKey, const void*, const RegistryVT, name_extracrt, const std::size_t >>
		&extractor, const void* value, const RegistryVT type, name_extracrt name, const std::size_t offset,
		ValueKey expect)
		: ExtractorRegistryKeyCommandWrapper(extractor),
		value(value),
		type(type),
		name(std::move(name)),
		offset(offset),
		expect(std::move(expect))
	{
	}

	const void *value;
	const RegistryVT type;
	const name_extracrt name;
	const std::size_t offset;

	ValueKey expect;
};


class ExtractorValueKeyTest : public ::testing::TestWithParam<ExtractorValueKeyExpectWrapper> {};

static const char VALUE1[] = "TEST_TEST";
static const int32_t  VALUE2 = 3;
static const int64_t  VALUE3 = 3;
static const char VALUE4[] = "123,,,";

INSTANTIATE_TEST_CASE_P(Default, ExtractorValueKeyTest,
	::testing::Values(
		ExtractorValueKeyExpectWrapper
		{ std::make_shared<const ExtractorValueKey>(), VALUE1,
			RegistryVT::VS_REG_STRING_VT, name_extracrt{ TEST_KNAME, TEST_VNAME }, vs::size(VALUE1),
			ValueKey { ValueKey::ValueName{ TEST_KNAME, TEST_VNAME }, ValueKey::ValueType::str, VALUE1 }
		},
		ExtractorValueKeyExpectWrapper
		{ std::make_shared<const ExtractorValueKey>(), &VALUE2,
			 RegistryVT::VS_REG_INTEGER_VT, name_extracrt{ TEST_KNAME, TEST_VNAME }, sizeof(VALUE2),
			ValueKey { ValueKey::ValueName{ TEST_KNAME, TEST_VNAME }, ValueKey::ValueType::i32,  std::to_string(VALUE2) }
		},
		ExtractorValueKeyExpectWrapper
		{ std::make_shared<const ExtractorValueKey>(), &VALUE3, RegistryVT::VS_REG_INT64_VT, name_extracrt{ TEST_KNAME, TEST_VNAME }, sizeof(VALUE3),
			ValueKey{ ValueKey::ValueName{ TEST_KNAME, TEST_VNAME }, ValueKey::ValueType::i64, std::to_string(VALUE3) }
		},
		ExtractorValueKeyExpectWrapper
		{ std::make_shared<const ExtractorValueKey>(), VALUE4, RegistryVT::VS_REG_BINARY_VT, name_extracrt{ TEST_KNAME, TEST_VNAME }, vs::size(VALUE4),
			ValueKey { ValueKey::ValueName{ TEST_KNAME, TEST_VNAME }, ValueKey::ValueType::b64, VALUE4 }
		}
));

//////////////////////////////////////////////////////////////////////////

struct ValidatorWrapper
{
	std::shared_ptr<const Validator<CommandParams>> validator;
	const CommandParams params;
};

class TestValidatorParams : public ::testing::TestWithParam<ValidatorWrapper> {};

INSTANTIATE_TEST_CASE_P(Default, TestValidatorParams, ::testing::Values(
	ValidatorWrapper{ std::make_shared<const ValidatorImpl>(), ([]()
	{
		CommandParams obj;
		obj.SetRoot(TEST_ROOT);
		obj.SetBackend(TEST_BACKEND);
		return obj;
	})() }
));

//////////////////////////////////////////////////////////////////////////

struct StorageWrapper
{
	std::shared_ptr<Storage> storage;
	const char *pathStorage;
	const Storage::storage_args valueKey;
};

class TestStorage : public ::testing::TestWithParam<StorageWrapper> {};

static const char PATH_STORAGE[] = "test_storage.csv";

INSTANTIATE_TEST_CASE_P(Default, TestStorage, ::testing::Values(
	StorageWrapper{ std::make_shared<StorageCSV>(true), PATH_STORAGE, ([]()
	{
		ValueKey obj;
		obj.SetName({ "Test", "Test" });
		obj.SetType(ValueKey::ValueType::str);
		obj.SetValue("TEST_DATA");
		return Storage::storage_args{ obj };
	})() }
));

//////////////////////////////////////////////////////////////////////////

static std::ostringstream Stream;
static const std::shared_ptr<const ExtractorValueKey> EXTRACTOR_VAL_KEY = std::make_shared<ExtractorValueKey>();

class CommandTest : public ::testing::Test
{
public:
	static void TearDownTestCase()
	{
		Stream.str("");
		Stream.clear();
	}
protected:
	void SetUp() override
	{
		TearDownTestCase();
	}
};

struct CommandWrapper
{
	CommandWrapper(const std::shared_ptr<CommandItem>& commandItem,
		const CommandParams::CommandParamsItem::args_t& basicStrings, const std::pair<std::string, std::string>& name,
		const std::string& expectValue)
		: commandItem(commandItem),
		  commands(basicStrings),
		  name(name),
		  expectValue(expectValue)
	{
	}

	const std::shared_ptr<CommandItem> commandItem;
	const CommandParams::CommandParamsItem::args_t commands;
	const std::pair<std::string, std::string> name;
	const std::string expectValue;
};

struct SetCommandWrapper : CommandWrapper
{
	SetCommandWrapper(const std::shared_ptr<CommandItem>& commandItem,
		const CommandParams::CommandParamsItem::args_t& basicStrings, const std::pair<std::string, std::string>& name,
		const ExtractorRegistryKeyCommandWrapper& extractor, const std::string& expectValue)
		: CommandWrapper(commandItem, basicStrings, name, expectValue),
		extractor(extractor)
	{
	}

	const ExtractorRegistryKeyCommandWrapper extractor;
};

class SetCommandTest : public CommandTest, public ::testing::WithParamInterface<SetCommandWrapper> {};

INSTANTIATE_TEST_CASE_P(Default, SetCommandTest, ::testing::Values(
	SetCommandWrapper{ std::make_shared<SetCommand>(), CommandParams::CommandParamsItem::args_t{ "set", "set_1", "i32", "123" },
		std::make_pair("set", "set_1"), ExtractorRegistryKeyCommandWrapper{ std::make_shared<ExtractorValueKey>() }, "123"
	},

	SetCommandWrapper{ std::make_shared<SetCommand>(), CommandParams::CommandParamsItem::args_t{ "set", "set_2", "i64", "321" },
		std::make_pair("set", "set_2"), ExtractorRegistryKeyCommandWrapper{ std::make_shared<ExtractorValueKey>() }, "321"
	},

	SetCommandWrapper{ std::make_shared<SetCommand>(), CommandParams::CommandParamsItem::args_t{ "set", "set_3", "str", "value_3" },
		std::make_pair("set", "set_3"), ExtractorRegistryKeyCommandWrapper{ std::make_shared<ExtractorValueKey>() }, "value_3"
	},
	SetCommandWrapper{ std::make_shared<SetCommand>(), CommandParams::CommandParamsItem::args_t{ "set", "set_4", "bin", "value_4" },
		std::make_pair("set", "set_4"), ExtractorRegistryKeyCommandWrapper{ std::make_shared<ExtractorValueKey>() }, "value_4"
	},

	SetCommandWrapper{ std::make_shared<SetCommand>(), CommandParams::CommandParamsItem::args_t{ "set", "set_5", "b64", "dmFsdWVfNQ==" },
		std::make_pair("set", "set_5"), ExtractorRegistryKeyCommandWrapper{ std::make_shared<ExtractorValueKey>() } ,"value_5"
	}
));

//////////////////////////////////////////////////////////////////////////


struct GetCommandWrapper : CommandWrapper
{
	GetCommandWrapper(const std::shared_ptr<CommandItem>& commandItem,
		const CommandParams::CommandParamsItem::args_t& basicStrings, const std::pair<std::string, std::string> &name,
		const std::string& expectValue, const std::shared_ptr<CommandItem>& commandItem1,
		const CommandParams::CommandParamsItem::args_t& basicStrings1)
		: CommandWrapper(commandItem, basicStrings, name, expectValue),
		setCommand(commandItem1),
		setArgs(basicStrings1)
	{
	}

	const std::shared_ptr<CommandItem> setCommand;
	const CommandParams::CommandParamsItem::args_t setArgs;
};

class GetCommandTest : public CommandTest, public ::testing::WithParamInterface<GetCommandWrapper> {};


INSTANTIATE_TEST_CASE_P(Default, GetCommandTest, ::testing::Values(
	GetCommandWrapper{ std::make_shared<GetCommand>(Stream, EXTRACTOR_VAL_KEY),
		CommandParams::CommandParamsItem::args_t{ "get", "get_1" }, std::make_pair("get", "get_1"), "123",
		std::make_shared<SetCommand>(), CommandParams::CommandParamsItem::args_t { "get", "get_1", "i32", "123"}
	},

	GetCommandWrapper{ std::make_shared<GetCommand>(Stream, EXTRACTOR_VAL_KEY),
		CommandParams::CommandParamsItem::args_t{ "get", "get_2" }, std::make_pair("get", "get_2"), "321",
		std::make_shared<SetCommand>(), CommandParams::CommandParamsItem::args_t{ "get", "get_2", "i64", "321" }
	},

	GetCommandWrapper{ std::make_shared<GetCommand>(Stream, EXTRACTOR_VAL_KEY),
		CommandParams::CommandParamsItem::args_t{ "get","get_3" }, std::make_pair("get", "get_3"), "str_val",
		std::make_shared<SetCommand>(), CommandParams::CommandParamsItem::args_t{ "get", "get_3", "str", "str_val" }
	},

	GetCommandWrapper{ std::make_shared<GetCommand>(Stream, EXTRACTOR_VAL_KEY),
		CommandParams::CommandParamsItem::args_t{ "get", "get_4" }, std::make_pair("get", "get_4"), "YmluX3ZhbA==",
		std::make_shared<SetCommand>(), CommandParams::CommandParamsItem::args_t{ "get", "get_4", "bin", "bin_val" }
	},

	GetCommandWrapper{ std::make_shared<GetCommand>(Stream, EXTRACTOR_VAL_KEY),
		CommandParams::CommandParamsItem::args_t{ "get", "get_5" }, std::make_pair("get", "get_5"), "YjY0X3ZhbA==",
		std::make_shared<SetCommand>(), CommandParams::CommandParamsItem::args_t{ "get","get_5", "b64", "YjY0X3ZhbA==" }
	}

));

//////////////////////////////////////////////////////////////////////////

struct StoreLoadCommandWrapper
{
	const std::shared_ptr<CommandItem> storeCommand;
	const std::shared_ptr<CommandItem> loadCommand;
	const std::vector<CommandParams::CommandParamsItem::args_t> setArgs;
	const Storage::storage_args expectStorage;
};

class StoreLoadCommandTest : public CommandTest, public ::testing::WithParamInterface<StoreLoadCommandWrapper> {};

static const char STORE_LOAD_FILE[] = "store_load_file.csv";

INSTANTIATE_TEST_CASE_P(Default, StoreLoadCommandTest, ::testing::Values(
	StoreLoadCommandWrapper{
		std::make_shared<StoreCommand>(vs::make_unique<StorageCSV>(true), EXTRACTOR_VAL_KEY),
		std::make_shared<LoadCommand>(vs::make_unique<StorageCSV>()),
		{{"store","store_1", "i32", "32"}, {"store","store_2", "i64", "64" }, {"store","store_3", "str", "str" }},
		Storage::storage_args {
			ValueKey{{"load","store_1"}, ValueKey::ValueType::i32, "32"},
			ValueKey{{"load","store_2"}, ValueKey::ValueType::i64, "64" },
			ValueKey{{"load","store_3"}, ValueKey::ValueType::str, "str"},
		}
	}
));

//////////////////////////////////////////////////////////////////////////
