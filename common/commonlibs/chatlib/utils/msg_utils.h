#pragma once
#include "chatlib/chat_defs.h"
#include "chatlib/chatinfo/info_types.h"
#include "chatlib/msg/ChatMessage.h"

#include "std-generic/compat/set.h"
#include "std/cpplib/numerical.h"

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace rj = rapidjson;

namespace chat
{
namespace msg
{
class HandlerBase : public rj::BaseReaderHandler<rj::UTF8<>, HandlerBase>
{
protected:
	string_view m_paramName;
	bool m_keyIsFound;
	bool m_isCompleted;
public:
	HandlerBase(string_view paramName)
		: m_paramName{paramName},
		m_keyIsFound{false},
		m_isCompleted{false}
	{}
	bool IsCompleted() const
	{
		return m_isCompleted;
	}
	bool Key(const char* str, rj::SizeType length, bool)
	{
		if (m_paramName.length() == length
			&& m_paramName.compare(0, length, str) == 0)
			m_keyIsFound = true;
		return true;
	}
	bool Default()
	{
		if (m_keyIsFound)
			return false;
		return true;
	}
};
template<typename ValueType, typename Enabled = void>
class Handler : public HandlerBase
{
	static_assert(!std::is_same_v<Enabled, void>, "Error: There is no template specialization for ValueType.");
};
template<typename ValueType>
class Handler<ValueType, std::enable_if_t<
	std::is_same_v<ValueType, int32_t> ||
	std::is_same_v<ValueType, uint32_t> ||
	std::is_same_v<ValueType, int64_t> ||
	std::is_same_v<ValueType, uint64_t>>> : public HandlerBase
{
	ValueType m_value;
public:
	Handler(string_view paramName)
		: HandlerBase{paramName},
		m_value{0}
	{}
	bool Int(int val)
	{
		if (m_keyIsFound)
		{
			if (numeric_less(val, std::numeric_limits<ValueType>::min()))
				return false;
			m_value = static_cast<ValueType>(val);
			m_isCompleted = true;
		}
		return true;
	}
	bool Uint(unsigned val)
	{
		if (m_keyIsFound)
		{
			if (numeric_less(std::numeric_limits<ValueType>::max(), val))
				return false;
			m_value = static_cast<ValueType>(val);
			m_isCompleted = true;
		}
		return true;
	}
	bool Int64(int64_t val)
	{
		if (m_keyIsFound)
		{
			if (numeric_less(val, std::numeric_limits<ValueType>::min()) ||
				numeric_less(std::numeric_limits<ValueType>::max(), val))
				return false;
			m_value = static_cast<ValueType>(val);
			m_isCompleted = true;
		}
		return true;
	}
	bool Uint64(uint64_t val)
	{
		if (m_keyIsFound)
		{
			if (numeric_less(std::numeric_limits<ValueType>::max(), val))
				return false;
			m_value = static_cast<ValueType>(val);
			m_isCompleted = true;
		}
		return true;
	}
	bool GetValue(ValueType& value)
	{
		if (!m_isCompleted || !m_keyIsFound)
			return false;
		value = m_value;
		return true;
	}
};
template<typename ValueType>
class Handler<ValueType, std::enable_if_t<std::is_same_v<ValueType, bool>>> : public HandlerBase
{
	bool m_value;
public:
	Handler(string_view paramName)
		: HandlerBase{paramName},
		m_value{false}
	{}
	bool Bool(bool val)
	{
		if (m_keyIsFound)
		{
			m_value = val;
			m_isCompleted = true;
		}
		return true;
	}
	bool GetValue(bool& value)
	{
		if (!m_isCompleted || !m_keyIsFound)
			return false;
		value = m_value;
		return true;
	}
};
template<typename ValueType>
class Handler<ValueType,
	std::enable_if_t<
	std::is_same_v<ValueType, std::string>>> : public HandlerBase
{
	std::string m_value;
public:
	Handler(string_view paramName)
		: HandlerBase{paramName}
	{}
	bool String(const char* str, rj::SizeType length, bool)
	{
		if (m_keyIsFound)
		{
			m_value = std::string(str, length);
			m_isCompleted = true;
		}
		return true;
	}
	bool GetValue(std::string& value)
	{
		if (!m_isCompleted || !m_keyIsFound)
			return false;
		value = std::move(m_value);
		return true;
	}
};
template<typename ValueType>
std::enable_if_t<!std::is_enum_v<ValueType>, bool>
GetParamFromMsgContent(const msg::ChatMessagePtr& msg, string_view paramName, ValueType& result)
{
	Handler<ValueType> h(paramName);
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::StringStream inStream(content.c_str());
	rj::Reader reader;
	reader.IterativeParseInit();
	while (!reader.IterativeParseComplete() && !h.IsCompleted())
	{
		if (!reader.IterativeParseNext<rj::kParseDefaultFlags>(inStream, h))
			return false;
	}
	if (!h.GetValue(result))
		return false;
	return true;
}
template<typename ValueType>
std::enable_if_t<std::is_enum_v<ValueType>, bool>
GetParamFromMsgContent(const msg::ChatMessagePtr& msg, string_view paramName, ValueType& result)
{
	auto res = static_cast<int32_t>(result);
	if (!GetParamFromMsgContent(msg, paramName, res))
		return false;
	result = static_cast<ValueType>(res);
	return true;
}
std::string GetParamStrFromMsgContent(const msg::ChatMessagePtr& msg, string_view paramName);
bool InsertParamStrIntoMsgContent(const msg::ChatMessagePtr& msg, string_view paramName, string_view value);
}
}
