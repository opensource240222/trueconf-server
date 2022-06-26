#include "msg_utils.h"
#include "chatlib/msg/chat_messages_construct.h"

namespace chat
{
namespace msg
{
std::string GetParamStrFromMsgContent(const msg::ChatMessagePtr& msg, string_view paramName)
{
	std::string result;
	if (!GetParamFromMsgContent(msg, paramName, result))
		return {};
	return result;
}
bool InsertParamStrIntoMsgContent(const msg::ChatMessagePtr& msg, string_view paramName, string_view value)
{
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	if (doc.Parse(content.c_str()).HasParseError() || !doc.IsObject())
		return false;
	auto memberIt = doc.FindMember(paramName.data());
	if (memberIt != doc.MemberEnd())
		doc.RemoveMember(memberIt);
	doc.AddMember(rj::StringRef(paramName.data(), paramName.length()),
		rj::Value().SetString(value.data(), static_cast<rj::SizeType>(value.length())), doc.GetAllocator());
	rj::StringBuffer s;
	rj::Writer<rj::StringBuffer> writer(s);
	doc.Accept(writer);
	if (!msg->DeleteParam(attr::MESSAGE_CONTENT_paramName))
		return false;
	if (!msg->SetParam(attr::MESSAGE_CONTENT_paramName, std::string{s.GetString(), s.GetLength()}))
		return false;
	return true;
}
}
}
