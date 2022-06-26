#include "chatlib/msg/attr.h"
#include "chatlib/msg/chat_messages_construct.h"
#include "chatlib/utils/chat_utils.h"
#include "chatlib/utils/msg_utils.h"

#include <gtest/gtest.h>

using namespace chat;

namespace
{
const auto testChatType = ChatType::symmetric;
const std::string testTitle = "Chat Title";
const std::string nonExistingKeyName = "non_existing_key";
} // anonymous namespace
namespace chat_test
{
TEST(MsgUtils, GetParamFromMsgContent)
{
	auto chatId = GenerateChatID();
	std::string user = "user@trueconf.com";
	auto userInstance = user + "/123";
	auto userType = vs::CallIDType::client;
	auto msg = msg::CreateChatMessage(
		chatId, current_chat_version,
		testChatType, testTitle,
		user, userInstance, userType, {}).AcquireMessage();
	// get and check existing params
	Version verFromContent;
	EXPECT_TRUE(msg::GetParamFromMsgContent(msg, msg::versionKeyName, verFromContent));
	EXPECT_EQ(current_chat_version, verFromContent);
	ChatType typeFromContent;
	EXPECT_TRUE(msg::GetParamFromMsgContent(msg, msg::typeKeyName, typeFromContent));
	EXPECT_EQ(testChatType, typeFromContent);
	std::string titleFromContent;
	EXPECT_TRUE(msg::GetParamFromMsgContent(msg, msg::titleKeyName, titleFromContent));
	EXPECT_EQ(testTitle, titleFromContent);
	CallID instanceFromContent;
	EXPECT_TRUE(msg::GetParamFromMsgContent(msg, msg::fromInstanceKeyName, instanceFromContent));
	EXPECT_EQ(userInstance, instanceFromContent);
	// try to get non-existing param
	std::string nonExistingParam;
	EXPECT_FALSE(msg::GetParamFromMsgContent(msg, nonExistingKeyName, nonExistingParam));
}
TEST(MsgUtils, InsertParamStrIntoMsgContent)
{
	using namespace msg;
	auto chatId = GenerateChatID();
	std::string user = "user@trueconf.com";
	auto userInstance = user + "/123";
	auto userType = vs::CallIDType::client;
	auto msg = msg::CreateChatMessage(
		chatId, current_chat_version,
		testChatType, testTitle,
		user, userInstance, userType, {}).AcquireMessage();
	// insert new params
	std::string toUser = "user_test@trueconf.com";
	EXPECT_TRUE(msg::InsertParamStrIntoMsgContent(msg, msg::toKeyName, toUser));
	std::string text = "Test message";
	EXPECT_TRUE(msg::InsertParamStrIntoMsgContent(msg, msg::contentKeyName, text));
	// rewrite existing param
	std::string userInstanceNew = user + "/321";
	EXPECT_TRUE(msg::InsertParamStrIntoMsgContent(msg, msg::fromInstanceKeyName, userInstanceNew));
	// parse and check content
	auto content = msg->GetParamStrRef(attr::MESSAGE_CONTENT_paramName);
	rj::Document doc;
	EXPECT_FALSE(doc.Parse(content.data(), content.length()).HasParseError() || !doc.IsObject());
	auto verIt = doc.FindMember(versionKeyName.c_str());
	auto typeIt = doc.FindMember(typeKeyName.c_str());
	auto titleIt = doc.FindMember(titleKeyName.c_str());
	auto userInstanceIt = doc.FindMember(fromInstanceKeyName.c_str());
	auto toIt = doc.FindMember(toKeyName.c_str());
	auto textIt = doc.FindMember(contentKeyName.c_str());
	EXPECT_FALSE(verIt == doc.MemberEnd());
	EXPECT_FALSE(typeIt == doc.MemberEnd());
	EXPECT_FALSE(titleIt == doc.MemberEnd());
	EXPECT_FALSE(userInstanceIt == doc.MemberEnd());
	EXPECT_FALSE(toIt == doc.MemberEnd());
	EXPECT_FALSE(textIt == doc.MemberEnd());
	EXPECT_TRUE(verIt->value.IsString());
	EXPECT_TRUE(typeIt->value.IsUint());
	EXPECT_TRUE(titleIt->value.IsString());
	EXPECT_TRUE(userInstanceIt->value.IsString());
	EXPECT_TRUE(toIt->value.IsString());
	EXPECT_TRUE(textIt->value.IsString());
	auto verFromMsg = Version{verIt->value.GetString(), verIt->value.GetStringLength()};
	auto typeFromMsg = static_cast<ChatType>(typeIt->value.GetUint());
	auto titleFromMsg = std::string{titleIt->value.GetString(), titleIt->value.GetStringLength()};
	auto userInstanceFromMsg = std::string{userInstanceIt->value.GetString(), userInstanceIt->value.GetStringLength()};
	auto toFromMsg = std::string{toIt->value.GetString(), toIt->value.GetStringLength()};
	auto textFromMsg = std::string{textIt->value.GetString(), textIt->value.GetStringLength()};
	EXPECT_EQ(current_chat_version, verFromMsg);
	EXPECT_EQ(testChatType, typeFromMsg);
	EXPECT_EQ(testTitle, titleFromMsg);
	EXPECT_EQ(userInstanceNew, userInstanceFromMsg);
	EXPECT_EQ(toUser, toFromMsg);
	EXPECT_EQ(text, textFromMsg);
}
} // namespace chat_test
