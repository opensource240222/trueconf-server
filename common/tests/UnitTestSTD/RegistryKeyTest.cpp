#include "std/cpplib/VS_RegistryKey.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/utf8.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <cctype>
#include <cstring>
#include <locale>

#if defined(__has_include)
#	if __has_include("tests/RegistryKeyTest_config.h")
#		include "tests/RegistryKeyTest_config.h"
#	endif
#endif

namespace regkey_test {

const std::string test_root("TrueConf_RegistryKeyTest");

const char key_name_1[] = "Key1";
const char key_name_2[] = "KeyTwo";
const char key_name_3[] = "OtherKey";
const char key_u8name_1[] = "\xD0\x9A\xD0\xBB\xD1\x8E\xD1\x87\x31"; // "Ключ1"
const char key_u8name_2[] = "\xD0\x9A\xD0\xBB\xD1\x8E\xD1\x87\xD0\x94\xD0\xB2\xD0\xB0"; // "КлючДва"
const char key_u8name_3[] = "\xD0\x94\xD1\x80\xD1\x83\xD0\xB3\xD0\xBE\xD0\xB9\xD0\x9A\xD0\xBB\xD1\x8E\xD1\x87"; //  "ДругойКлюч"

const char value_name_1[] = "Value1";
const char value_name_2[] = "ValueTwo";
const char value_name_3[] = "OtherValue";
const char value_u8name_1[] = "\xD0\x97\xD0\xBD\xD0\xB0\xD1\x87\xD0\xB5\xD0\xBD\xD0\xB8\xD0\xB5\x31"; // "Значение1"
const char value_u8name_2[] = "\xD0\x97\xD0\xBD\xD0\xB0\xD1\x87\xD0\xB5\xD0\xBD\xD0\xB8\xD0\xB5\xD0\x94\xD0\xB2\xD0\xB0"; // "ЗначениеДва"
const char value_u8name_3[] = "\xD0\x94\xD1\x80\xD1\x83\xD0\xB3\xD0\xBE\xD0\xB5\xD0\x97\xD0\xBD\xD0\xB0\xD1\x87\xD0\xB5\xD0\xBD\xD0\xB8\xD0\xB5"; // "ДругоеЗначение"

const char u8value_1[] = "\xD0\x9F\xD0\xB5\xD1\x80\xD0\xB2\xD0\xB0\xD1\x8F \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0"; // "Первая строка"
const char u8value_2[] = "\xD0\x92\xD1\x82\xD0\xBE\xD1\x80\xD0\xB0\xD1\x8F \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0"; // "Вторая строка"
const char u8value_3[] = "\xD0\xA1\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0 \xD1\x82\xD1\x80\xD0\xB8"; // "Строка три"
#if defined(_WIN32) // Not ported yet
const wchar_t wvalue_1[] = L"\u041F\u0435\u0440\u0432\u0430\u044F \u0441\u0442\u0440\u043E\u043A\u0430"; // "Первая строка"
const wchar_t wvalue_2[] = L"\u0412\u0442\u043E\u0440\u0430\u044F \u0441\u0442\u0440\u043E\u043A\u0430"; // "Вторая строка"
const wchar_t wvalue_3[] = L"\u0421\u0442\u0440\u043E\u043A\u0430 \u0442\u0440\u0438"; // "Строка три"
#endif

template <class T>
class RegistryKeyTest : public ::testing::Test
{
public:
	static void SetUpTestCase()
	{
		backend = regkey::Backend::Create(T::get_configuration());
	}

	static void TearDownTestCase()
	{
		backend.reset();
	}

protected:
	virtual void SetUp() override
	{
		ASSERT_NE(nullptr, backend);
		EXPECT_TRUE(backend->Create({}, false, test_root, false, true)->IsValid());
	}

	virtual void TearDown() override
	{
		ASSERT_NE(nullptr, backend);
		auto root_key = backend->Create({}, false, string_view{}, false, true);
		EXPECT_TRUE(root_key->RemoveKey(test_root));
	}

	void CreateNonEmptyKey(string_view root, string_view name)
	{
		auto key = backend->Create(root, false, name, false, true);
		EXPECT_TRUE(key->IsValid());
		// Key has to have some values to exits
		int32_t value = 0;
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "TMP123"));
	}

	std::string InvertCase(string_view str)
	{
		std::string result;
		result.reserve(str.size());
		for (auto c: str)
			result += std::islower(c) ? std::toupper(c) : std::tolower(c);
		return result;
	}

	std::string InvertCase_UTF8(string_view str)
	{
		auto wstr = vs::UTF8ToWideCharConvert(str);
		EXPECT_FALSE(!str.empty() && wstr.empty());
		const auto& loc = vs::GetUnicodeLocale();
		for (auto& c: wstr)
			c = std::islower(c, loc) ? std::toupper(c, loc) : std::tolower(c, loc);
		auto result = vs::WideCharToUTF8Convert(wstr);
		EXPECT_FALSE(!wstr.empty() && result.empty());
		EXPECT_NE(str, result);
		return result;
	}

	static std::shared_ptr<regkey::Backend> backend;
};

template <class T>
std::shared_ptr<regkey::Backend> RegistryKeyTest<T>::backend;

struct memory   { static string_view get_configuration() { return "memory:"; } };
struct registry { static string_view get_configuration() { return "registry:force_lm=true"; } };
#if defined(DB_BACKEND)
struct db       { static string_view get_configuration() { return DB_BACKEND; } };
#endif

typedef ::testing::Types<memory
#if defined(_WIN32)
	, registry
#endif
#if defined(DB_BACKEND)
	, db
#endif
> TypeParams;
TYPED_TEST_CASE(RegistryKeyTest, TypeParams);

TYPED_TEST(RegistryKeyTest, Create)
{
	EXPECT_TRUE(this->backend->Create(test_root, false, key_name_1, false, true)->IsValid());
}

TYPED_TEST(RegistryKeyTest, Create_Existing)
{
	EXPECT_TRUE(this->backend->Create(test_root, false, key_name_1, false, true)->IsValid());
	EXPECT_TRUE(this->backend->Create(test_root, false, key_name_1, false, true)->IsValid());
}

TYPED_TEST(RegistryKeyTest, Create_InvalidKeyName)
{
	EXPECT_FALSE(this->backend->Create(test_root, false, "\\Leading slash", false, true)->IsValid());
	EXPECT_FALSE(this->backend->Create(test_root, false, "Trailing slash\\", false, true)->IsValid());
}

TYPED_TEST(RegistryKeyTest, Create_InvalidRoot)
{
	EXPECT_FALSE(this->backend->Create("\\Leading slash", false, key_name_1, false, true)->IsValid());
	EXPECT_FALSE(this->backend->Create("Trailing slash\\", false, key_name_1, false, true)->IsValid());
}

TYPED_TEST(RegistryKeyTest, Open)
{
	this->CreateNonEmptyKey(test_root, key_name_1);
	EXPECT_TRUE(this->backend->Create(test_root, false, key_name_1, true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, Open_Missing)
{
	EXPECT_FALSE(this->backend->Create(test_root, false, key_name_1, true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, Open_UTF8Name)
{
	this->CreateNonEmptyKey(test_root, key_u8name_1);
	EXPECT_TRUE(this->backend->Create(test_root, false, key_u8name_1, true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, Open_NonCaseSensetiveName)
{
	this->CreateNonEmptyKey(test_root, key_name_1);
	EXPECT_TRUE(this->backend->Create(test_root, false, this->InvertCase(key_name_1), true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, DISABLED_Open_NonCaseSensetiveName_UTF8Name)
{
	this->CreateNonEmptyKey(test_root, key_u8name_1);
	EXPECT_TRUE(this->backend->Create(test_root, false, this->InvertCase_UTF8(key_u8name_1), true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, Open_DuplicateSlash)
{
	this->CreateNonEmptyKey(test_root, "Two\\\\Slashes");
	EXPECT_TRUE(this->backend->Create(test_root, false, "Two\\Slashes", false)->IsValid());

	this->CreateNonEmptyKey(test_root, "One\\Slash");
	EXPECT_TRUE(this->backend->Create(test_root, false, "One\\\\\\Slash", false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, RootNameConcatenation)
{
	const int32_t value = 121418;
	{
		std::string root;
		root += test_root;
		root += '\\';
		root += key_name_1;

		auto key = this->backend->Create(root, false, key_name_2, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	{
		std::string name;
		name += key_name_1;
		name += '\\';
		name += key_name_2;

		auto key = this->backend->Create(test_root, false, name, false, true);
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, SetValue_ReadOnly)
{
	this->CreateNonEmptyKey(test_root, key_name_1);
	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	const int32_t value = 42;
	EXPECT_FALSE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
}

TYPED_TEST(RegistryKeyTest, SetValue_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, false);
	EXPECT_FALSE(key->IsValid());
	const int32_t value = 42;
	EXPECT_FALSE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_NonCaseSensetiveName)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 42;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	int32_t result;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, this->InvertCase(value_name_1).c_str()));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, DISABLED_SetGetValue_NonCaseSensetiveName_UTF8Name)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 42;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_u8name_1));
	int32_t result;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, this->InvertCase_UTF8(value_u8name_1).c_str()));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, SetGetValue_Int32)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 1234;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	int32_t result = 0;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, SetGetValue_Int32_SmallBuffer)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 1234;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	int32_t result = 0;
	EXPECT_EQ(-static_cast<int32_t>(sizeof(result)), key->GetValue(&result, sizeof(result) / 2, VS_REG_INTEGER_VT, value_name_1));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_Int32_Overwrite)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value_old = 1234;
	EXPECT_TRUE(key->SetValue(&value_old, sizeof(value_old), VS_REG_INTEGER_VT, value_name_1));
	const int32_t value = 4321;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	int32_t result = 0;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, SetGetValue_Int32_UTF8Name)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 1234;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_u8name_1));
	int32_t result = 0;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_u8name_1));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, GetValue_Int32_Missing)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 1234;
	int32_t result = value;
	EXPECT_EQ(0, key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_EQ(result, value) << "GetValue shouldn't modify buffer contents if value isn't found";
}

TYPED_TEST(RegistryKeyTest, GetValue_Int32_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	const int32_t value = 1234;
	int32_t result = value;
	EXPECT_EQ(0, key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_EQ(result, value) << "GetValue shouldn't modify buffer contents if the key is misssing";
}

TYPED_TEST(RegistryKeyTest, SetGetValue_Int64)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int64_t value = 1234567890123456;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INT64_VT, value_name_1));
	int64_t result = 0;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INT64_VT, value_name_1));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, SetGetValue_Int64_SmallBuffer)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int64_t value = 1234567890123456;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INT64_VT, value_name_1));
	int64_t result = 0;
	EXPECT_EQ(-static_cast<int32_t>(sizeof(result)), key->GetValue(&result, sizeof(result) / 2, VS_REG_INT64_VT, value_name_1));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_Int64_Overwrite)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int64_t value_old = 6543210987654321;
	EXPECT_TRUE(key->SetValue(&value_old, sizeof(value_old), VS_REG_INT64_VT, value_name_1));
	const int64_t value = 1234567890123456;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INT64_VT, value_name_1));
	int64_t result = 0;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INT64_VT, value_name_1));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, SetGetValue_Int64_UTF8Name)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int64_t value = 1234567890123456;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INT64_VT, value_u8name_1));
	int64_t result = 0;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INT64_VT, value_u8name_1));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, GetValue_Int64_Missing)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int64_t value = 1234567890123456;
	int64_t result = value;
	EXPECT_EQ(0, key->GetValue(&result, sizeof(result), VS_REG_INT64_VT, value_name_1));
	EXPECT_EQ(result, value) << "GetValue shouldn't modify buffer contents if value isn't found";
}

TYPED_TEST(RegistryKeyTest, GetValue_Int64_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	const int64_t value = 1234567890123456;
	int64_t result = value;
	EXPECT_EQ(0, key->GetValue(&result, sizeof(result), VS_REG_INT64_VT, value_name_1));
	EXPECT_EQ(result, value) << "GetValue shouldn't modify buffer contents if the key is misssing";
}

TYPED_TEST(RegistryKeyTest, SetGetValue_String)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "This is a string";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_STRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(value));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_String_SmallBuffer)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "This is a string";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(-static_cast<int32_t>(sizeof(result)), key->GetValue(&result, sizeof(result) / 2, VS_REG_STRING_VT, value_name_1));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_String_Overwrite)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value_old[] = "Old string";
	EXPECT_TRUE(key->SetValue(value_old, 0, VS_REG_STRING_VT, value_name_1));
	const char value[] = "This is a string";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_STRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(value));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_String_UTF8Name)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "This is a string";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_u8name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_STRING_VT, value_u8name_1));
	EXPECT_THAT(result, StrEq(value));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_String_UTF8)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "\xD0\xAD\xD1\x82\xD0\xBE \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0"; // "Это строка"
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_STRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(value));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_String_UTF8_UTF8Name)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "\xD0\xAD\xD1\x82\xD0\xBE \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0"; // "Это строка"
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_u8name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_STRING_VT, value_u8name_1));
	EXPECT_THAT(result, StrEq(value));
}

TYPED_TEST(RegistryKeyTest, GetValue_String_Missing)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "This is a string";
	char result[sizeof(value) / sizeof(value[0])];
	std::memcpy(result, value, sizeof(value));
	EXPECT_EQ(0, key->GetValue(&result, sizeof(result), VS_REG_STRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(value)) << "GetValue shouldn't modify buffer contents if value isn't found";
}

TYPED_TEST(RegistryKeyTest, GetValue_String_InvalidKey)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	const char value[] = "This is a string";
	char result[sizeof(value) / sizeof(value[0])];
	std::memcpy(result, value, sizeof(value));
	EXPECT_EQ(0, key->GetValue(&result, sizeof(result), VS_REG_STRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(value)) << "GetValue shouldn't modify buffer contents if the key is misssing";
}

#if defined(_WIN32) // Not ported yet
TYPED_TEST(RegistryKeyTest, SetGetValue_WString)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const wchar_t value[] = L"\u042D\u0442\u043E \u0441\u0442\u0440\u043E\u043A\u0430"; // "Это строка"
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_WSTRING_VT, value_name_1));
	wchar_t result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_WSTRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(value));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_WString_SmallBuffer)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const wchar_t value[] = L"\u042D\u0442\u043E \u0441\u0442\u0440\u043E\u043A\u0430"; // "Это строка"
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_WSTRING_VT, value_name_1));
	wchar_t result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(-static_cast<int32_t>(sizeof(result)), key->GetValue(&result, sizeof(result) / 2, VS_REG_WSTRING_VT, value_name_1));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_WString_Overwrite)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const wchar_t value_old[] = L"\u0421\u0442\u0430\u0440\u0430\u044F \u0441\u0442\u0440\u043E\u043A\u0430"; // "Старая строка"
	EXPECT_TRUE(key->SetValue(value_old, 0, VS_REG_WSTRING_VT, value_name_1));
	const wchar_t value[] = L"\u042D\u0442\u043E \u0441\u0442\u0440\u043E\u043A\u0430"; // "Это строка"
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_WSTRING_VT, value_name_1));
	wchar_t result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_WSTRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(value));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_WString_UTF8Name)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const wchar_t value[] = L"\u042D\u0442\u043E \u0441\u0442\u0440\u043E\u043A\u0430"; // "Это строка"
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_WSTRING_VT, value_u8name_1));
	wchar_t result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_WSTRING_VT, value_u8name_1));
	EXPECT_THAT(result, StrEq(value));
}

TYPED_TEST(RegistryKeyTest, GetValue_WString_Missing)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const wchar_t value[] = L"\u042D\u0442\u043E \u0441\u0442\u0440\u043E\u043A\u0430"; // "Это строка"
	wchar_t result[sizeof(value) / sizeof(value[0])];
	std::memcpy(result, value, sizeof(value));
	EXPECT_EQ(0, key->GetValue(&result, sizeof(result), VS_REG_WSTRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(value)) << "GetValue shouldn't modify buffer contents if value isn't found";
}

TYPED_TEST(RegistryKeyTest, GetValue_WString_InvalidKey)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	const wchar_t value[] = L"\u042D\u0442\u043E \u0441\u0442\u0440\u043E\u043A\u0430"; // "Это строка"
	wchar_t result[sizeof(value) / sizeof(value[0])];
	std::memcpy(result, value, sizeof(value));
	EXPECT_EQ(0, key->GetValue(&result, sizeof(result), VS_REG_WSTRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(value)) << "GetValue shouldn't modify buffer contents if the key is misssing";
}
#endif

TYPED_TEST(RegistryKeyTest, SetGetValue_Binary)
{
	using ::testing::ElementsAreArray;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = { 'S', 'o', 'm', 'e', '\x00', '\x01', '\x02', 'd', 'a', 't', 'a', '\xff', '\x03', '\x04', '\x80', 'h', 'e', 'r', 'e', '\x7f', };
	EXPECT_TRUE(key->SetValue(value, sizeof(value), VS_REG_BINARY_VT, value_name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_BINARY_VT, value_name_1));
	EXPECT_THAT(result, ElementsAreArray(value));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_Binary_SmallBuffer)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = { 'S', 'o', 'm', 'e', '\x00', '\x01', '\x02', 'd', 'a', 't', 'a', '\xff', '\x03', '\x04', '\x80', 'h', 'e', 'r', 'e', '\x7f', };
	EXPECT_TRUE(key->SetValue(value, sizeof(value), VS_REG_BINARY_VT, value_name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(-static_cast<int32_t>(sizeof(result)), key->GetValue(&result, sizeof(result) / 2, VS_REG_BINARY_VT, value_name_1));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_Binary_Overwrite)
{
	using ::testing::ElementsAreArray;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value_old[] = { 'S', 't', 'r', 'i', 'n', 'g', '\0', 'w', 'i', 't', 'h', '\0', 'n', 'u', 'l', 'l', 's', };
	EXPECT_TRUE(key->SetValue(value_old, sizeof(value_old), VS_REG_BINARY_VT, value_name_1));
	const char value[] = { 'S', 'o', 'm', 'e', '\x00', '\x01', '\x02', 'd', 'a', 't', 'a', '\xff', '\x03', '\x04', '\x80', 'h', 'e', 'r', 'e', '\x7f', };
	EXPECT_TRUE(key->SetValue(value, sizeof(value), VS_REG_BINARY_VT, value_name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_BINARY_VT, value_name_1));
	EXPECT_THAT(result, ElementsAreArray(value));
}

TYPED_TEST(RegistryKeyTest, SetGetValue_Binary_UTF8Name)
{
	using ::testing::ElementsAreArray;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = { 'S', 'o', 'm', 'e', '\x00', '\x01', '\x02', 'd', 'a', 't', 'a', '\xff', '\x03', '\x04', '\x80', 'h', 'e', 'r', 'e', '\x7f', };
	EXPECT_TRUE(key->SetValue(value, sizeof(value), VS_REG_BINARY_VT, value_u8name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_BINARY_VT, value_u8name_1));
	EXPECT_THAT(result, ElementsAreArray(value));
}

TYPED_TEST(RegistryKeyTest, GetValue_Binary_Missing)
{
	using ::testing::ElementsAreArray;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = { 'S', 'o', 'm', 'e', '\x00', '\x01', '\x02', 'd', 'a', 't', 'a', '\xff', '\x03', '\x04', '\x80', 'h', 'e', 'r', 'e', '\x7f', };
	char result[sizeof(value) / sizeof(value[0])];
	std::memcpy(result, value, sizeof(value));
	EXPECT_EQ(0, key->GetValue(&result, sizeof(result), VS_REG_BINARY_VT, value_name_1));
	EXPECT_THAT(result, ElementsAreArray(value)) << "GetValue shouldn't modify buffer contents if value isn't found";
}

TYPED_TEST(RegistryKeyTest, GetValue_Binary_InvalidKey)
{
	using ::testing::ElementsAreArray;

	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	const char value[] = { 'S', 'o', 'm', 'e', '\x00', '\x01', '\x02', 'd', 'a', 't', 'a', '\xff', '\x03', '\x04', '\x80', 'h', 'e', 'r', 'e', '\x7f', };
	char result[sizeof(value) / sizeof(value[0])];
	std::memcpy(result, value, sizeof(value));
	EXPECT_EQ(0, key->GetValue(&result, sizeof(result), VS_REG_BINARY_VT, value_name_1));
	EXPECT_THAT(result, ElementsAreArray(value)) << "GetValue shouldn't modify buffer contents if the key is misssing";
}

TYPED_TEST(RegistryKeyTest, GetValueAndType_NonCaseSensetiveName)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 1234;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	int32_t result = 0;
	RegistryVT type = static_cast<RegistryVT>(255);
	EXPECT_EQ(sizeof(result), key->GetValueAndType(&result, sizeof(result), type, this->InvertCase(value_name_1).c_str()));
	EXPECT_EQ(type, VS_REG_INTEGER_VT);
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, GetValueAndType_Int32)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 1234;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	int32_t result = 0;
	RegistryVT type = static_cast<RegistryVT>(255);
	EXPECT_EQ(sizeof(result), key->GetValueAndType(&result, sizeof(result), type, value_name_1));
	EXPECT_EQ(type, VS_REG_INTEGER_VT);
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, GetValueAndType_Int64)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int64_t value = 1234567890123456;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INT64_VT, value_name_1));
	int64_t result = 0;
	RegistryVT type = static_cast<RegistryVT>(255);
	EXPECT_EQ(sizeof(result), key->GetValueAndType(&result, sizeof(result), type, value_name_1));
	EXPECT_EQ(type, VS_REG_INT64_VT);
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, GetValueAndType_String)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "This is a string";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	RegistryVT type = static_cast<RegistryVT>(255);
	EXPECT_EQ(sizeof(result), key->GetValueAndType(result, sizeof(result), type, value_name_1));
	EXPECT_EQ(type, VS_REG_STRING_VT);
	EXPECT_THAT(result, StrEq(value));
}

// NOTE: There are no tests WString because different backends store data strings in different ways:
//   * DB store everything as a UTF-8 strings.
//   * Win registry stores all strings in UTF-16.

TYPED_TEST(RegistryKeyTest, GetValueAndType_Binary)
{
	using ::testing::ElementsAreArray;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = { 'S', 'o', 'm', 'e', '\x00', '\x01', '\x02', 'd', 'a', 't', 'a', '\xff', '\x03', '\x04', '\x80', 'h', 'e', 'r', 'e', '\x7f', };
	EXPECT_TRUE(key->SetValue(value, sizeof(value), VS_REG_BINARY_VT, value_name_1));
	char result[sizeof(value) / sizeof(value[0])] = {};
	RegistryVT type = static_cast<RegistryVT>(255);
	EXPECT_EQ(sizeof(result), key->GetValueAndType(result, sizeof(result), type, value_name_1));
	EXPECT_EQ(type, VS_REG_BINARY_VT);
	EXPECT_THAT(result, ElementsAreArray(value));
}

TYPED_TEST(RegistryKeyTest, GetValueAndType_InvalidKey)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	char result[] = "Old data";
	RegistryVT type = static_cast<RegistryVT>(255);
	EXPECT_EQ(0, key->GetValueAndType(result, sizeof(result), type, value_name_1));
	EXPECT_EQ(type, static_cast<RegistryVT>(255)) << "GetValueAndType shouldn't set value type if the key is misssing";;
	EXPECT_THAT(result, StrEq("Old data")) << "GetValueAndType shouldn't modify buffer contents if the key is misssing";
}

TYPED_TEST(RegistryKeyTest, GetString)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "This is a string";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	std::string result;
	EXPECT_TRUE(key->GetString(result, value_name_1));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, GetString_UTF8Name)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "This is a string";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_u8name_1));
	std::string result;
	EXPECT_TRUE(key->GetString(result, value_u8name_1));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, GetString_UTF8)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "\xD0\xAD\xD1\x82\xD0\xBE \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0"; // "Это строка"
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	std::string result;
	EXPECT_TRUE(key->GetString(result, value_name_1));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, GetString_UTF8_UTF8Name)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "\xD0\xAD\xD1\x82\xD0\xBE \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0"; // "Это строка"
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_u8name_1));
	std::string result;
	EXPECT_TRUE(key->GetString(result, value_u8name_1));
	EXPECT_EQ(result, value);
}

TYPED_TEST(RegistryKeyTest, GetString_Missing)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "This is a string";
	std::string result = value;
	EXPECT_FALSE(key->GetString(result, value_name_1));
	EXPECT_EQ(result, value) << "GetString shouldn't modify buffer contents if value isn't found";
}

TYPED_TEST(RegistryKeyTest, GetString_InvalidKey)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	const char value[] = "This is a string";
	std::string result = value;
	EXPECT_FALSE(key->GetString(result, value_name_1));
	EXPECT_EQ(result, value) << "GetString shouldn't modify buffer contents if the key is misssing";
}

TYPED_TEST(RegistryKeyTest, StringToInt32)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "123456";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	int32_t result = 0;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_EQ(result, 123456);
}

TYPED_TEST(RegistryKeyTest, StringToInt32_MinInt32)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "-2147483648";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	int32_t result = 0;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_EQ(result, INT32_MIN);
}

TYPED_TEST(RegistryKeyTest, StringToInt32_MaxInt32)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] =  "2147483647";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	int32_t result = 0;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_EQ(result, INT32_MAX);
}

TYPED_TEST(RegistryKeyTest, StringToInt32_MaxUInt32)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "4294967295";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	uint32_t result = 0;
	EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_EQ(result, UINT32_MAX);
}

TYPED_TEST(RegistryKeyTest, StringToInt32_Invalid)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value[] = "Not a number";
	EXPECT_TRUE(key->SetValue(value, 0, VS_REG_STRING_VT, value_name_1));
	int32_t result = 654321;
	EXPECT_EQ(0, key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_EQ(result, 654321) << "GetValue shouldn't modify buffer contents if data conversion fails";
}

TYPED_TEST(RegistryKeyTest, Int32ToString)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 654321;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	char result[7] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_STRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq("654321"));
}

TYPED_TEST(RegistryKeyTest, Int32ToString_MinInt32)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = INT32_MIN;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	char result[12] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_STRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq("-2147483648"));
}

TYPED_TEST(RegistryKeyTest, Int32ToString_MaxInt32)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = INT32_MAX;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	char result[11] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_STRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq("2147483647"));
}

#if defined(_WIN32) // Not ported yet
TYPED_TEST(RegistryKeyTest, StringToWString)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char u8value[] = "This is a string";
	const wchar_t wvalue[] = L"This is a string";
	EXPECT_TRUE(key->SetValue(u8value, sizeof(u8value), VS_REG_STRING_VT, value_name_1));
	wchar_t result[sizeof(wvalue) / sizeof(wvalue[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_WSTRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(wvalue));
}

TYPED_TEST(RegistryKeyTest, StringToWString_UTF8)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char u8value[] = "\xD0\xAD\xD1\x82\xD0\xBE \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0"; // "Это строка"
	const wchar_t wvalue[] = L"\u042D\u0442\u043E \u0441\u0442\u0440\u043E\u043A\u0430"; // "Это строка"
	EXPECT_TRUE(key->SetValue(u8value, sizeof(u8value), VS_REG_STRING_VT, value_name_1));
	wchar_t result[sizeof(wvalue) / sizeof(wvalue[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_WSTRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(wvalue));
}

TYPED_TEST(RegistryKeyTest, WStringToString)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char u8value[] = "This is a string";
	const wchar_t wvalue[] = L"This is a string";
	EXPECT_TRUE(key->SetValue(wvalue, sizeof(wvalue), VS_REG_WSTRING_VT, value_name_1));
	char result[sizeof(u8value) / sizeof(u8value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_STRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(u8value));
}

TYPED_TEST(RegistryKeyTest, WStringToString_UTF8)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char u8value[] = "\xD0\xAD\xD1\x82\xD0\xBE \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0"; // "Это строка"
	const wchar_t wvalue[] = L"\u042D\u0442\u043E \u0441\u0442\u0440\u043E\u043A\u0430"; // "Это строка"
	EXPECT_TRUE(key->SetValue(wvalue, sizeof(wvalue), VS_REG_WSTRING_VT, value_name_1));
	char result[sizeof(u8value) / sizeof(u8value[0])] = {};
	EXPECT_EQ(sizeof(result), key->GetValue(result, sizeof(result), VS_REG_STRING_VT, value_name_1));
	EXPECT_THAT(result, StrEq(u8value));
}
#endif

TYPED_TEST(RegistryKeyTest, HasValue)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 42;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_TRUE(key->HasValue(value_name_1));
}

TYPED_TEST(RegistryKeyTest, HasValue_Missing)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	EXPECT_FALSE(key->HasValue(value_name_1));
}

TYPED_TEST(RegistryKeyTest, HasValue_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	EXPECT_FALSE(key->HasValue(value_name_1));
}

TYPED_TEST(RegistryKeyTest, HasValue_UTF8Name)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 42;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_u8name_1));
	EXPECT_TRUE(key->HasValue(value_u8name_1));
}

TYPED_TEST(RegistryKeyTest, HasValue_NonCaseSensetiveName)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 42;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_TRUE(key->HasValue(this->InvertCase(value_name_1)));
}

TYPED_TEST(RegistryKeyTest, DISABLED_HasValue_NonCaseSensetiveName_UTF8Name)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 42;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_u8name_1));
	EXPECT_TRUE(key->HasValue(this->InvertCase_UTF8(value_u8name_1)));
}

TYPED_TEST(RegistryKeyTest, RemoveValue)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 42;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_TRUE(key->RemoveValue(value_name_1));
	EXPECT_FALSE(key->HasValue(value_name_1));
}

TYPED_TEST(RegistryKeyTest, RemoveValue_Missing)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	EXPECT_FALSE(key->RemoveValue(value_name_1));
}

TYPED_TEST(RegistryKeyTest, RemoveValue_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, false);
	EXPECT_FALSE(key->IsValid());
	EXPECT_FALSE(key->RemoveValue(value_name_1));
}

TYPED_TEST(RegistryKeyTest, RemoveValue_UTF8Name)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 42;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_u8name_1));
	EXPECT_TRUE(key->RemoveValue(value_u8name_1));
	EXPECT_FALSE(key->HasValue(value_u8name_1));
}

TYPED_TEST(RegistryKeyTest, RemoveValue_NonCaseSensetiveName)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 42;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_TRUE(key->RemoveValue(this->InvertCase(value_name_1)));
	EXPECT_FALSE(key->HasValue(value_name_1));
}

TYPED_TEST(RegistryKeyTest, DISABLED_RemoveValue_NonCaseSensetiveName_UTF8Name)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value = 42;
	EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_u8name_1));
	EXPECT_TRUE(key->RemoveValue(this->InvertCase_UTF8(value_u8name_1)));
	EXPECT_FALSE(key->HasValue(value_u8name_1));
}

TYPED_TEST(RegistryKeyTest, RemoveValue_CorrectKey)
{
	auto key1 = this->backend->Create(test_root, false, key_name_1, false, true);
	auto key2 = this->backend->Create(test_root, false, key_name_2, false, true);
	std::string key1_root;
	key1_root.append(test_root).append("\\").append(key_name_1);
	auto key1_subkey = this->backend->Create(key1_root, false, key_name_3, false, true);

	const int32_t value = 42;
	EXPECT_TRUE(key1->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_TRUE(key2->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_TRUE(key1_subkey->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));

	EXPECT_TRUE(key1->RemoveValue(value_name_1));
	EXPECT_FALSE(key1->HasValue(value_name_1));
	EXPECT_TRUE(key2->HasValue(value_name_1));
	EXPECT_TRUE(key1_subkey->HasValue(value_name_1));
}

TYPED_TEST(RegistryKeyTest, RemoveKey)
{
	this->CreateNonEmptyKey(test_root, key_name_1);
	std::string sub_key_name;
	sub_key_name += key_name_1;
	sub_key_name += '\\';
	sub_key_name += key_name_2;
	this->CreateNonEmptyKey(test_root, sub_key_name);
	std::string unrelated_key_name;
	unrelated_key_name += key_name_1;
	unrelated_key_name += '_'; // Not a subkey, just a key with a same name prefix
	unrelated_key_name += key_name_3;
	this->CreateNonEmptyKey(test_root, unrelated_key_name);
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RemoveKey(key_name_1));
	EXPECT_FALSE(this->backend->Create(test_root, false, key_name_1, true, false)->IsValid());
	EXPECT_FALSE(this->backend->Create(test_root, false, sub_key_name, true, false)->IsValid());
	EXPECT_TRUE(this->backend->Create(test_root, false, unrelated_key_name, true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, RemoveKey_ReadOnly)
{
	this->CreateNonEmptyKey(test_root, key_name_1);
	auto root_key = this->backend->Create({}, false, test_root, true, false);
	EXPECT_FALSE(root_key->RemoveKey(key_name_1));
	EXPECT_TRUE(this->backend->Create(test_root, false, key_name_1, true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, RemoveKey_Missing)
{
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RemoveKey(key_name_1));
}

TYPED_TEST(RegistryKeyTest, RemoveKey_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, false);
	EXPECT_FALSE(key->IsValid());
	EXPECT_FALSE(key->RemoveKey(key_name_2));
}

TYPED_TEST(RegistryKeyTest, RemoveKey_UTF8Name)
{
	this->CreateNonEmptyKey(test_root, key_u8name_1);
	std::string sub_key_name;
	sub_key_name += key_u8name_1;
	sub_key_name += '\\';
	sub_key_name += key_u8name_2;
	this->CreateNonEmptyKey(test_root, sub_key_name);
	std::string unrelated_key_name;
	unrelated_key_name += key_u8name_1;
	unrelated_key_name += '_'; // Not a subkey, just a key with a same name prefix
	unrelated_key_name += key_u8name_3;
	this->CreateNonEmptyKey(test_root, unrelated_key_name);
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RemoveKey(key_u8name_1));
	EXPECT_FALSE(this->backend->Create(test_root, false, key_u8name_1, true, false)->IsValid());
	EXPECT_FALSE(this->backend->Create(test_root, false, sub_key_name, true, false)->IsValid());
	EXPECT_TRUE(this->backend->Create(test_root, false, unrelated_key_name, true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, RemoveKey_NonCaseSensetiveName)
{
	this->CreateNonEmptyKey(test_root, key_name_1);
	std::string sub_key_name;
	sub_key_name += key_name_1;
	sub_key_name += '\\';
	sub_key_name += key_name_2;
	this->CreateNonEmptyKey(test_root, sub_key_name);
	std::string unrelated_key_name;
	unrelated_key_name += key_name_1;
	unrelated_key_name += '_'; // Not a subkey, just a key with a same name prefix
	unrelated_key_name += key_name_3;
	this->CreateNonEmptyKey(test_root, unrelated_key_name);
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RemoveKey(this->InvertCase(key_name_1)));
	EXPECT_FALSE(this->backend->Create(test_root, false, key_name_1, true, false)->IsValid());
	EXPECT_FALSE(this->backend->Create(test_root, false, sub_key_name, true, false)->IsValid());
	EXPECT_TRUE(this->backend->Create(test_root, false, unrelated_key_name, true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, DISABLED_RemoveKey_NonCaseSensetiveName_UTF8Name)
{
	this->CreateNonEmptyKey(test_root, key_u8name_1);
	std::string sub_key_name;
	sub_key_name += key_u8name_1;
	sub_key_name += '\\';
	sub_key_name += key_u8name_2;
	this->CreateNonEmptyKey(test_root, sub_key_name);
	std::string unrelated_key_name;
	unrelated_key_name += key_u8name_1;
	unrelated_key_name += '_'; // Not a subkey, just a key with a same name prefix
	unrelated_key_name += key_u8name_3;
	this->CreateNonEmptyKey(test_root, unrelated_key_name);
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RemoveKey(this->InvertCase_UTF8(key_u8name_1)));
	EXPECT_FALSE(this->backend->Create(test_root, false, key_u8name_1, true, false)->IsValid());
	EXPECT_FALSE(this->backend->Create(test_root, false, sub_key_name, true, false)->IsValid());
	EXPECT_TRUE(this->backend->Create(test_root, false, unrelated_key_name, true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, RenameKey)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RenameKey(key_name_1, key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, RenameKey_ReadOnly)
{
	this->CreateNonEmptyKey(test_root, key_name_1);
	auto root_key = this->backend->Create({}, false, test_root, true, false);
	EXPECT_FALSE(root_key->RenameKey(key_name_1, key_name_2));
	EXPECT_TRUE(this->backend->Create(test_root, false, key_name_1, true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, RenameKey_Missing)
{
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_FALSE(root_key->RenameKey(key_name_1, key_name_2));
}

TYPED_TEST(RegistryKeyTest, RenameKey_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, false);
	EXPECT_FALSE(key->IsValid());
	EXPECT_FALSE(key->RenameKey(key_name_2, key_name_3));
}

TYPED_TEST(RegistryKeyTest, RenameKey_MoveToSubKey)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	std::string dst_key_name;
	dst_key_name += key_name_2;
	dst_key_name += '\\';
	dst_key_name += key_name_3;
	EXPECT_TRUE(root_key->RenameKey(key_name_1, dst_key_name));
	{
		auto key = this->backend->Create(test_root, false, dst_key_name, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, RenameKey_MoveFromSubKey)
{
	std::string src_key_name;
	src_key_name += key_name_1;
	src_key_name += '\\';
	src_key_name += key_name_2;
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, src_key_name, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RenameKey(src_key_name, key_name_3));
	{
		auto key = this->backend->Create(test_root, false, key_name_3, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, RenameKey_MoveToOwnSubKey) // foo -> foo\bar
{
	this->CreateNonEmptyKey(test_root, key_name_1);
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	std::string dst_key_name;
	dst_key_name += key_name_1;
	dst_key_name += '\\';
	dst_key_name += key_name_2;
	EXPECT_FALSE(root_key->RenameKey(key_name_1, dst_key_name));
}

TYPED_TEST(RegistryKeyTest, RenameKey_MoveFromOwnSubKey) // foo\bar -> foo
{
	std::string src_key_name;
	src_key_name += key_name_1;
	src_key_name += '\\';
	src_key_name += key_name_2;
	this->CreateNonEmptyKey(test_root, src_key_name);
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_FALSE(root_key->RenameKey(src_key_name, key_name_1));
}

TYPED_TEST(RegistryKeyTest, RenameKey_UTF8Name)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_u8name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RenameKey(key_u8name_1, key_u8name_2));
	{
		auto key = this->backend->Create(test_root, false, key_u8name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, RenameKey_NonCaseSensetiveName)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RenameKey(this->InvertCase(key_name_1), key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, DISABLED_RenameKey_NonCaseSensetiveName_UTF8Name)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_u8name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RenameKey(this->InvertCase_UTF8(key_u8name_1), key_u8name_2));
	{
		auto key = this->backend->Create(test_root, false, key_u8name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, RenameKey_SubKeyMoved)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_name_1 + std::string("\\") + key_name_3, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RenameKey(key_name_1, key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_2 + std::string("\\") + key_name_3, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, RenameKey_RemoveOriginal)
{
	this->CreateNonEmptyKey(test_root, key_name_1);
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RenameKey(key_name_1, key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_1, true, false);
		EXPECT_FALSE(key->IsValid());
	}
}

TYPED_TEST(RegistryKeyTest, RenameKey_OverwriteExisting)
{
	const int32_t value_1 = 1029;
	const int32_t value_2 = 3847;
	{
		auto key = this->backend->Create(test_root, false, key_name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value_1, sizeof(value_1), VS_REG_INTEGER_VT, value_name_1));
	}
	{
		auto key = this->backend->Create(test_root, false, key_name_2, false, true);
		EXPECT_TRUE(key->SetValue(&value_2, sizeof(value_2), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RenameKey(key_name_1, key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value_1);
	}
}

TYPED_TEST(RegistryKeyTest, RenameKey_NoMerge)
{
	const int32_t value_1 = 9201;
	const int32_t value_2 = 7483;
	{
		auto key = this->backend->Create(test_root, false, key_name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value_1, sizeof(value_1), VS_REG_INTEGER_VT, value_name_1));
	}
	{
		auto key = this->backend->Create(test_root, false, key_name_2, false, true);
		EXPECT_TRUE(key->SetValue(&value_2, sizeof(value_2), VS_REG_INTEGER_VT, value_name_2));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->RenameKey(key_name_1, key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		EXPECT_FALSE(key->HasValue(value_name_2));
	}
}

TYPED_TEST(RegistryKeyTest, CopyKey)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->CopyKey(key_name_1, key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, CopyKey_ReadOnly)
{
	this->CreateNonEmptyKey(test_root, key_name_1);
	auto root_key = this->backend->Create({}, false, test_root, true, false);
	EXPECT_FALSE(root_key->CopyKey(key_name_1, key_name_2));
	EXPECT_TRUE(this->backend->Create(test_root, false, key_name_1, true, false)->IsValid());
}

TYPED_TEST(RegistryKeyTest, CopyKey_Missing)
{
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_FALSE(root_key->CopyKey(key_name_1, key_name_2));
}

TYPED_TEST(RegistryKeyTest, CopyKey_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, false);
	EXPECT_FALSE(key->IsValid());
	EXPECT_FALSE(key->CopyKey(key_name_2, key_name_3));
}

TYPED_TEST(RegistryKeyTest, CopyKey_CopyToSubKey)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	std::string dst_key_name;
	dst_key_name += key_name_2;
	dst_key_name += '\\';
	dst_key_name += key_name_3;
	EXPECT_TRUE(root_key->CopyKey(key_name_1, dst_key_name));
	{
		auto key = this->backend->Create(test_root, false, dst_key_name, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, CopyKey_CopyFromSubKey)
{
	std::string src_key_name;
	src_key_name += key_name_1;
	src_key_name += '\\';
	src_key_name += key_name_2;
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, src_key_name, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->CopyKey(src_key_name, key_name_3));
	{
		auto key = this->backend->Create(test_root, false, key_name_3, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, CopyKey_MoveToOwnSubKey) // foo -> foo\bar
{
	this->CreateNonEmptyKey(test_root, key_name_1);
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	std::string dst_key_name;
	dst_key_name += key_name_1;
	dst_key_name += '\\';
	dst_key_name += key_name_2;
	EXPECT_FALSE(root_key->CopyKey(key_name_1, dst_key_name));
}

TYPED_TEST(RegistryKeyTest, CopyKey_MoveFromOwnSubKey) // foo\bar -> foo
{
	std::string src_key_name;
	src_key_name += key_name_1;
	src_key_name += '\\';
	src_key_name += key_name_2;
	this->CreateNonEmptyKey(test_root, src_key_name);
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_FALSE(root_key->CopyKey(src_key_name, key_name_1));
}

TYPED_TEST(RegistryKeyTest, CopyKey_UTF8Name)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_u8name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->CopyKey(key_u8name_1, key_u8name_2));
	{
		auto key = this->backend->Create(test_root, false, key_u8name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, CopyKey_NonCaseSensetiveName)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->CopyKey(this->InvertCase(key_name_1), key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, DISABLED_CopyKey_NonCaseSensetiveName_UTF8Name)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_u8name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->CopyKey(this->InvertCase_UTF8(key_u8name_1), key_u8name_2));
	{
		auto key = this->backend->Create(test_root, false, key_u8name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, CopyKey_SubKeysCopied)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_name_1 + std::string("\\") + key_name_3, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->CopyKey(key_name_1, key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_2 + std::string("\\") + key_name_3, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, CopyKey_KeepOriginal)
{
	const int32_t value = 1029;
	{
		auto key = this->backend->Create(test_root, false, key_name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->CopyKey(key_name_1, key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_1, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value);
	}
}

TYPED_TEST(RegistryKeyTest, CopyKey_OverwriteExisting)
{
	const int32_t value_1 = 1029;
	const int32_t value_2 = 3847;
	{
		auto key = this->backend->Create(test_root, false, key_name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value_1, sizeof(value_1), VS_REG_INTEGER_VT, value_name_1));
	}
	{
		auto key = this->backend->Create(test_root, false, key_name_2, false, true);
		EXPECT_TRUE(key->SetValue(&value_2, sizeof(value_2), VS_REG_INTEGER_VT, value_name_1));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->CopyKey(key_name_1, key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_1));
		EXPECT_EQ(result, value_1);
	}
}

TYPED_TEST(RegistryKeyTest, CopyKey_Merge)
{
	const int32_t value_1 = 9201;
	const int32_t value_2 = 7483;
	{
		auto key = this->backend->Create(test_root, false, key_name_1, false, true);
		EXPECT_TRUE(key->SetValue(&value_1, sizeof(value_1), VS_REG_INTEGER_VT, value_name_1));
	}
	{
		auto key = this->backend->Create(test_root, false, key_name_2, false, true);
		EXPECT_TRUE(key->SetValue(&value_2, sizeof(value_2), VS_REG_INTEGER_VT, value_name_2));
	}
	auto root_key = this->backend->Create({}, false, test_root, false, true);
	EXPECT_TRUE(root_key->CopyKey(key_name_1, key_name_2));
	{
		auto key = this->backend->Create(test_root, false, key_name_2, true, false);
		EXPECT_TRUE(key->IsValid());
		int32_t result = 0;
		EXPECT_EQ(sizeof(result), key->GetValue(&result, sizeof(result), VS_REG_INTEGER_VT, value_name_2));
		EXPECT_EQ(result, value_2);
	}
}

TYPED_TEST(RegistryKeyTest, GetName)
{
	using ::testing::StrEq;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const auto key_name = key->GetName();
	EXPECT_NE(nullptr, key_name);
	EXPECT_THAT(key_name, StrEq(key_name_1));
}

TYPED_TEST(RegistryKeyTest, GetName_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	EXPECT_EQ(nullptr, key->GetName());
}

TYPED_TEST(RegistryKeyTest, GetValueCount)
{
	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value_1 = 1234;
	const char value_2[] = "Second string";
	EXPECT_TRUE(key->SetValue(&value_1, sizeof(value_1), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_TRUE(key->SetValue(value_2, 0, VS_REG_STRING_VT, value_name_2));

	// Values in this sub-key should be ignored.
	auto sub_key = this->backend->Create(test_root + '\\' + key_name_1, false, key_name_2, false, true);
	const int64_t value_3 = 1010101010101;
	EXPECT_TRUE(sub_key->SetValue(&value_3, sizeof(value_3), VS_REG_INT64_VT, value_name_3));
	EXPECT_TRUE(sub_key->SetValue(&value_1, sizeof(value_1), VS_REG_INTEGER_VT, value_name_1));

	EXPECT_EQ(2u, key->GetValueCount());
}

TYPED_TEST(RegistryKeyTest, GetValueCount_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	EXPECT_EQ(0u, key->GetValueCount());
}

TYPED_TEST(RegistryKeyTest, GetKeyCount)
{
	std::string root;
	root += test_root;
	root += '\\';
	root += key_name_1;
	this->CreateNonEmptyKey(root, key_name_1);
	this->CreateNonEmptyKey(root, key_name_2);
	this->CreateNonEmptyKey(root, key_name_3);

	// These sub-sub-keys should be ignored.
	this->CreateNonEmptyKey(root + '\\' + key_name_1, key_name_2);
	this->CreateNonEmptyKey(root + '\\' + key_name_2, key_name_3);

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	EXPECT_EQ(3u, key->GetKeyCount());
}

TYPED_TEST(RegistryKeyTest, GetKeyCount_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	EXPECT_EQ(0u, key->GetKeyCount());
}

TYPED_TEST(RegistryKeyTest, NextValue_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	key->ResetValues();
	std::unique_ptr<int32_t, free_deleter> result;
	std::string name;
	EXPECT_EQ(0u, key->NextValue(result, VS_REG_INTEGER_VT, name));
}

TYPED_TEST(RegistryKeyTest, NextValue_Int32)
{
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int32_t value_1 = 1234;
	const int32_t value_2 = 9876;
	const int32_t value_3 = 10101;
	EXPECT_TRUE(key->SetValue(&value_1, sizeof(value_1), VS_REG_INTEGER_VT, value_name_1));
	EXPECT_TRUE(key->SetValue(&value_2, sizeof(value_2), VS_REG_INTEGER_VT, value_name_2));
	EXPECT_TRUE(key->SetValue(&value_3, sizeof(value_3), VS_REG_INTEGER_VT, value_name_3));

	std::vector<std::pair<std::string, int32_t>> data;

	key->ResetValues();
	int32_t result_size = 0;
	std::unique_ptr<int32_t, free_deleter> result;
	std::string name;
	while ((result_size = key->NextValue(result, VS_REG_INTEGER_VT, name)) > 0)
	{
		VS_SCOPE_EXIT
		{
			result = nullptr;
			name.clear();
		};
		ASSERT_FALSE(name.empty());
		ASSERT_EQ(sizeof(int32_t), result_size);
		data.emplace_back(name, *result);
	}
	EXPECT_THAT(data, UnorderedElementsAre(
		Pair(StrEq(value_name_1), value_1),
		Pair(StrEq(value_name_2), value_2),
		Pair(StrEq(value_name_3), value_3)
	));
}

TYPED_TEST(RegistryKeyTest, NextValue_Int64)
{
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const int64_t value_1 = 123400006789;
	const int64_t value_2 = 987600004321;
	const int64_t value_3 = 1010101010101;
	EXPECT_TRUE(key->SetValue(&value_1, sizeof(value_1), VS_REG_INT64_VT, value_name_1));
	EXPECT_TRUE(key->SetValue(&value_2, sizeof(value_2), VS_REG_INT64_VT, value_name_2));
	EXPECT_TRUE(key->SetValue(&value_3, sizeof(value_3), VS_REG_INT64_VT, value_name_3));

	std::vector<std::pair<std::string, int64_t>> data;

	key->ResetValues();
	int32_t result_size = 0;
	std::unique_ptr<int64_t, free_deleter> result;
	std::string name;
	while ((result_size = key->NextValue(result, VS_REG_INT64_VT, name)) > 0)
	{
		VS_SCOPE_EXIT
		{
			result = nullptr;
			name.clear();
		};
		ASSERT_FALSE(name.empty());
		ASSERT_EQ(sizeof(int64_t), result_size);
		data.emplace_back(name, *result);
	}
	EXPECT_THAT(data, UnorderedElementsAre(
		Pair(StrEq(value_name_1), value_1),
		Pair(StrEq(value_name_2), value_2),
		Pair(StrEq(value_name_3), value_3)
	));
}

TYPED_TEST(RegistryKeyTest, NextValue_String)
{
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value_1[] = "First string";
	const char value_2[] = "Second string";
	const char value_3[] = "String three";
	EXPECT_TRUE(key->SetValue(value_1, 0, VS_REG_STRING_VT, value_name_1));
	EXPECT_TRUE(key->SetValue(value_2, 0, VS_REG_STRING_VT, value_name_2));
	EXPECT_TRUE(key->SetValue(value_3, 0, VS_REG_STRING_VT, value_name_3));

	std::vector<std::pair<std::string, std::string>> data;

	key->ResetValues();
	int32_t result_size = 0;
	std::unique_ptr<char, free_deleter> result;
	std::string name;
	while ((result_size = key->NextValue(result, VS_REG_STRING_VT, name)) > 0)
	{
		VS_SCOPE_EXIT
		{
			result = nullptr;
			name.clear();
		};
		ASSERT_FALSE(name.empty());
		ASSERT_NE(nullptr, result);
		EXPECT_EQ('\0', result.get()[result_size - 1]);
		data.emplace_back(name, result.get());
	}
	EXPECT_THAT(data, UnorderedElementsAre(
		Pair(StrEq(value_name_1), StrEq(value_1)),
		Pair(StrEq(value_name_2), StrEq(value_2)),
		Pair(StrEq(value_name_3), StrEq(value_3))
	));
}

TYPED_TEST(RegistryKeyTest, NextValue_String_UTF8)
{
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	EXPECT_TRUE(key->SetValue(u8value_1, 0, VS_REG_STRING_VT, value_name_1));
	EXPECT_TRUE(key->SetValue(u8value_2, 0, VS_REG_STRING_VT, value_name_2));
	EXPECT_TRUE(key->SetValue(u8value_3, 0, VS_REG_STRING_VT, value_name_3));

	std::vector<std::pair<std::string, std::string>> data;

	key->ResetValues();
	int32_t result_size = 0;
	std::unique_ptr<char, free_deleter> result;
	std::string name;
	while ((result_size = key->NextValue(result, VS_REG_STRING_VT, name)) > 0)
	{
		VS_SCOPE_EXIT
		{
			result = nullptr;
			name.clear();
		};
		ASSERT_FALSE(name.empty());
		ASSERT_NE(nullptr, result);
		EXPECT_EQ('\0', result.get()[result_size - 1]);
		data.emplace_back(name, result.get());
	}
	EXPECT_THAT(data, UnorderedElementsAre(
		Pair(StrEq(value_name_1), StrEq(u8value_1)),
		Pair(StrEq(value_name_2), StrEq(u8value_2)),
		Pair(StrEq(value_name_3), StrEq(u8value_3))
	));
}

TYPED_TEST(RegistryKeyTest, NextString)
{
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value_1[] = "First string";
	const char value_2[] = "Second string";
	const char value_3[] = "String three";
	EXPECT_TRUE(key->SetValue(value_1, 0, VS_REG_STRING_VT, value_name_1));
	EXPECT_TRUE(key->SetValue(value_2, 0, VS_REG_STRING_VT, value_name_2));
	EXPECT_TRUE(key->SetValue(value_3, 0, VS_REG_STRING_VT, value_name_3));

	std::vector<std::pair<std::string, std::string>> data;

	key->ResetValues();
	std::string result;
	std::string name;
	while (key->NextString(result, name))
		data.emplace_back(name, result);
	EXPECT_THAT(data, UnorderedElementsAre(
		Pair(StrEq(value_name_1), StrEq(value_1)),
		Pair(StrEq(value_name_2), StrEq(value_2)),
		Pair(StrEq(value_name_3), StrEq(value_3))
	));
}

TYPED_TEST(RegistryKeyTest, NextString_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	key->ResetValues();
	std::string result;
	std::string name;
	EXPECT_FALSE(key->NextString(result, name));
}

TYPED_TEST(RegistryKeyTest, NextString_UTF8)
{
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	EXPECT_TRUE(key->SetValue(u8value_1, 0, VS_REG_STRING_VT, value_u8name_1));
	EXPECT_TRUE(key->SetValue(u8value_2, 0, VS_REG_STRING_VT, value_u8name_2));
	EXPECT_TRUE(key->SetValue(u8value_3, 0, VS_REG_STRING_VT, value_u8name_3));

	std::vector<std::pair<std::string, std::string>> data;

	key->ResetValues();
	std::string result;
	std::string name;
	while (key->NextString(result, name))
		data.emplace_back(name, result);
	EXPECT_THAT(data, UnorderedElementsAre(
		Pair(StrEq(value_u8name_1), StrEq(u8value_1)),
		Pair(StrEq(value_u8name_2), StrEq(u8value_2)),
		Pair(StrEq(value_u8name_3), StrEq(u8value_3))
	));
}

#if defined(_WIN32) // Not ported yet
TYPED_TEST(RegistryKeyTest, NextValue_WString)
{
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	EXPECT_TRUE(key->SetValue(wvalue_1, 0, VS_REG_WSTRING_VT, value_name_1));
	EXPECT_TRUE(key->SetValue(wvalue_2, 0, VS_REG_WSTRING_VT, value_name_2));
	EXPECT_TRUE(key->SetValue(wvalue_3, 0, VS_REG_WSTRING_VT, value_name_3));

	std::vector<std::pair<std::string, std::wstring>> data;

	key->ResetValues();
	int32_t result_size = 0;
	std::unique_ptr<wchar_t, free_deleter> result;
	std::string name;
	while ((result_size = key->NextValue(result, VS_REG_WSTRING_VT, name)) > 0)
	{
		VS_SCOPE_EXIT
		{
			result = nullptr;
			name.clear();
		};
		ASSERT_FALSE(name.empty());
		ASSERT_NE(nullptr, result);
		EXPECT_EQ(L'\0', result.get()[result_size / sizeof(wchar_t) - 1]);
		data.emplace_back(name, result.get());
	}
	EXPECT_THAT(data, UnorderedElementsAre(
		Pair(StrEq(value_name_1), StrEq(wvalue_1)),
		Pair(StrEq(value_name_2), StrEq(wvalue_2)),
		Pair(StrEq(value_name_3), StrEq(wvalue_3))
	));
}
#endif

TYPED_TEST(RegistryKeyTest, NextValue_Binary)
{
	using ::testing::ElementsAreArray;
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value_1[] = { 'S', 'o', 'm', 'e', '\x00', '\x01', '\x02', 'd', 'a', 't', 'a', '\xff', '\x03', '\x04', '\x80', 'h', 'e', 'r', 'e', '\x7f', };
	const char value_2[] = { 'S', 't', 'r', 'i', 'n', 'g', '\0', 'w', 'i', 't', 'h', '\0', 'n', 'u', 'l', 'l', 's', };
	const char value_3[] = { '1', };
	EXPECT_TRUE(key->SetValue(&value_1, sizeof(value_1), VS_REG_BINARY_VT, value_name_1));
	EXPECT_TRUE(key->SetValue(&value_2, sizeof(value_2), VS_REG_BINARY_VT, value_name_2));
	EXPECT_TRUE(key->SetValue(&value_3, sizeof(value_3), VS_REG_BINARY_VT, value_name_3));

	std::vector<std::pair<std::string, std::vector<char>>> data;

	key->ResetValues();
	int32_t result_size = 0;
	std::unique_ptr<char, free_deleter> result;
	std::string name;
	while ((result_size = key->NextValue(result, VS_REG_BINARY_VT, name)) > 0)
	{
		VS_SCOPE_EXIT
		{
			result = nullptr;
			name.clear();
		};
		ASSERT_FALSE(name.empty());
		ASSERT_NE(nullptr, result);
		data.emplace_back(name, std::vector<char>(result.get(), result.get() + result_size));
	}
	EXPECT_THAT(data, UnorderedElementsAre(
		Pair(StrEq(value_name_1), ElementsAreArray(value_1)),
		Pair(StrEq(value_name_2), ElementsAreArray(value_2)),
		Pair(StrEq(value_name_3), ElementsAreArray(value_3))
	));
}

TYPED_TEST(RegistryKeyTest, NextValueAndType)
{
	using ::testing::ElementsAreArray;
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	const char value_string_1[] = "First string";
	const char value_binary_1[] = { 'S', 'o', 'm', 'e', '\x00', '\x01', '\x02', 'd', 'a', 't', 'a', '\xff', '\x03', '\x04', '\x80', 'h', 'e', 'r', 'e', '\x7f', };
	const char value_binary_2[] = { 'S', 't', 'r', 'i', 'n', 'g', '\0', 'w', 'i', 't', 'h', '\0', 'n', 'u', 'l', 'l', 's', };
	const int32_t value_int32_1 = 42;
	const int32_t value_int32_2 = 12345678;
	EXPECT_TRUE(key->SetValue(value_string_1, 0, VS_REG_STRING_VT, value_name_1));
	EXPECT_TRUE(key->SetValue(u8value_1, 0, VS_REG_STRING_VT, value_u8name_1));
	EXPECT_TRUE(key->SetValue(&value_binary_1, sizeof(value_binary_1), VS_REG_BINARY_VT, value_name_2));
	EXPECT_TRUE(key->SetValue(&value_binary_2, sizeof(value_binary_2), VS_REG_BINARY_VT, value_u8name_2));
	EXPECT_TRUE(key->SetValue(&value_int32_1, sizeof(value_int32_1), VS_REG_INTEGER_VT, value_name_3));
	EXPECT_TRUE(key->SetValue(&value_int32_2, sizeof(value_int32_2), VS_REG_INTEGER_VT, value_u8name_3));

	std::vector<std::pair<std::string, std::string>> data_string;
	std::vector<std::pair<std::string, std::vector<char>>> data_binary;
	std::vector<std::pair<std::string, int32_t>> data_int32;

	key->ResetValues();
	int32_t result_size = 0;
	std::unique_ptr<void, free_deleter> result;
	RegistryVT type = static_cast<RegistryVT>(255);
	std::string name;
	while ((result_size = key->NextValueAndType(result, type, name)) > 0)
	{
		VS_SCOPE_EXIT
		{
			result = nullptr;
			type = static_cast<RegistryVT>(255);
			name.clear();
		};
		ASSERT_FALSE(name.empty());
		ASSERT_NE(nullptr, result);
		switch (type)
		{
		case VS_REG_INTEGER_VT:
			EXPECT_EQ(sizeof(int32_t), result_size);
			data_int32.emplace_back(name, *static_cast<const int32_t*>(result.get()));
			break;
		case VS_REG_STRING_VT:
			EXPECT_EQ('\0', static_cast<const char*>(result.get())[result_size - 1]);
			data_string.emplace_back(name, static_cast<const char*>(result.get()));
			break;
		case VS_REG_BINARY_VT:
			data_binary.emplace_back(name, std::vector<char>(static_cast<const char*>(result.get()), static_cast<const char*>(result.get()) + result_size));
			break;
		default:
			ADD_FAILURE() << "Unexpected type: " << type;
			break;
		}
	}
	EXPECT_THAT(data_string, UnorderedElementsAre(
		Pair(StrEq(value_name_1), StrEq(value_string_1)),
		Pair(StrEq(value_u8name_1), StrEq(u8value_1))
	));
	EXPECT_THAT(data_binary, UnorderedElementsAre(
		Pair(StrEq(value_name_2), ElementsAreArray(value_binary_1)),
		Pair(StrEq(value_u8name_2), ElementsAreArray(value_binary_2))
	));
	EXPECT_THAT(data_int32, UnorderedElementsAre(
		Pair(StrEq(value_name_3), value_int32_1),
		Pair(StrEq(value_u8name_3), value_int32_2)
	));
}

TYPED_TEST(RegistryKeyTest, NextValueAndType_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	key->ResetValues();
	std::unique_ptr<void, free_deleter> result;
	RegistryVT type;
	std::string name;
	EXPECT_EQ(0u, key->NextValueAndType(result, type, name));
}

TYPED_TEST(RegistryKeyTest, NextKey)
{
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	std::string root;
	root += test_root;
	root += '\\';
	root += key_name_1;
	this->CreateNonEmptyKey(root, key_name_2);
	this->CreateNonEmptyKey(root, key_u8name_2);
	this->CreateNonEmptyKey(root, key_name_3);
	this->CreateNonEmptyKey(root, key_u8name_3);

	std::vector<std::string> data;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	key->ResetKey();
	regkey::key_ptr sub_key;
	while ((sub_key = key->NextKey()))
		data.emplace_back(sub_key->GetName());
	EXPECT_THAT(data, UnorderedElementsAre(
		StrEq(key_name_2),
		StrEq(key_u8name_2),
		StrEq(key_name_3),
		StrEq(key_u8name_3)
	));
}

TYPED_TEST(RegistryKeyTest, NextKey_InvalidKey)
{
	auto key = this->backend->Create(test_root, false, key_name_1, true, false);
	EXPECT_FALSE(key->IsValid());
	key->ResetKey();
	EXPECT_EQ(nullptr, key->NextKey());
}

TYPED_TEST(RegistryKeyTest, NextKey_UTF8Name)
{
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	std::string root;
	root += test_root;
	root += '\\';
	root += key_u8name_1;
	this->CreateNonEmptyKey(root, key_name_2);
	this->CreateNonEmptyKey(root, key_u8name_2);
	this->CreateNonEmptyKey(root, key_name_3);
	this->CreateNonEmptyKey(root, key_u8name_3);

	std::vector<std::string> data;

	auto key = this->backend->Create(test_root, false, key_u8name_1, false, true);
	key->ResetKey();
	regkey::key_ptr sub_key;
	while ((sub_key = key->NextKey()))
		data.emplace_back(sub_key->GetName());
	EXPECT_THAT(data, UnorderedElementsAre(
		StrEq(key_name_2),
		StrEq(key_u8name_2),
		StrEq(key_name_3),
		StrEq(key_u8name_3)
	));
}

TYPED_TEST(RegistryKeyTest, NextKey_NonCaseSensetiveName)
{
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	std::string root;
	root += test_root;
	root += '\\';
	root += key_name_1;
	this->CreateNonEmptyKey(root, key_name_2);
	this->CreateNonEmptyKey(root, key_u8name_2);
	this->CreateNonEmptyKey(root, key_name_3);
	this->CreateNonEmptyKey(root, key_u8name_3);

	std::vector<std::string> data;

	auto key = this->backend->Create(test_root, false, this->InvertCase(key_name_1), false, true);
	key->ResetKey();
	regkey::key_ptr sub_key;
	while ((sub_key = key->NextKey()))
		data.emplace_back(sub_key->GetName());
	EXPECT_THAT(data, UnorderedElementsAre(
		StrEq(key_name_2),
		StrEq(key_u8name_2),
		StrEq(key_name_3),
		StrEq(key_u8name_3)
	));
}

TYPED_TEST(RegistryKeyTest, DISABLED_NextKey_NonCaseSensetiveName_UTF8Name)
{
	using ::testing::Pair;
	using ::testing::StrEq;
	using ::testing::UnorderedElementsAre;

	std::string root;
	root += test_root;
	root += '\\';
	root += key_u8name_1;
	this->CreateNonEmptyKey(root, key_name_2);
	this->CreateNonEmptyKey(root, key_u8name_2);
	this->CreateNonEmptyKey(root, key_name_3);
	this->CreateNonEmptyKey(root, key_u8name_3);

	std::vector<std::string> data;

	auto key = this->backend->Create(test_root, false, this->InvertCase_UTF8(key_u8name_1), false, true);
	key->ResetKey();
	regkey::key_ptr sub_key;
	while ((sub_key = key->NextKey()))
		data.emplace_back(sub_key->GetName());
	EXPECT_THAT(data, UnorderedElementsAre(
		StrEq(key_name_2),
		StrEq(key_u8name_2),
		StrEq(key_name_3),
		StrEq(key_u8name_3)
	));
}

TYPED_TEST(RegistryKeyTest, NextKey_NoDuplicatesWithDirrerentCase)
{
	using ::testing::Pair;
	using ::testing::StrCaseEq;
	using ::testing::UnorderedElementsAre;

	{
		auto key = this->backend->Create(test_root, false, std::string(key_name_1) + '\\' + key_name_2, false, true);
		EXPECT_TRUE(key->IsValid());
		int32_t value = 0;
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_1));
	}

	{
		auto key = this->backend->Create(test_root, false, std::string(key_name_1) + '\\' + this->InvertCase(key_name_2), false, true);
		EXPECT_TRUE(key->IsValid());
		int32_t value = 1;
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_2));
	}

	{
		auto key = this->backend->Create(test_root, false, this->InvertCase(key_name_1) + '\\' + key_name_2, false, true);
		EXPECT_TRUE(key->IsValid());
		int32_t value = 2;
		EXPECT_TRUE(key->SetValue(&value, sizeof(value), VS_REG_INTEGER_VT, value_name_3));
	}

	std::vector<std::string> data;

	auto key = this->backend->Create(test_root, false, key_name_1, false, true);
	key->ResetKey();
	regkey::key_ptr sub_key;
	while ((sub_key = key->NextKey()))
		data.emplace_back(sub_key->GetName());
	EXPECT_THAT(data, UnorderedElementsAre(
		StrCaseEq(key_name_2)
	));
}

}
