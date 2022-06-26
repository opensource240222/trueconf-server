#pragma once
#include "chatlib/storage/ChatStorage.h"
namespace chat
{
std::shared_ptr<ChatStorage> make_chat_storage(string_view config);
}