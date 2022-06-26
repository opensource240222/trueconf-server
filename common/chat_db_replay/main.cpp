#include "chatlib/chain/ChainContainer.h"
#include "chatlib/msg/attr.h"
#include "chatlib/storage/make_chat_storage.h"

#include "std-generic/sqlite/CppSQLite3.h"
#include <cppdb/frontend.h>

#include <iostream>

std::string get_dsn(const std::string & in_file_name)
{
	return std::string("sqlite3:db=") + in_file_name;
}

std::vector<std::string> get_message_history(const std::string& in_file_name, const std::string& chat_id)
{
	std::vector<std::string> res;
	cppdb::session inp(get_dsn(in_file_name));
	cppdb::result rs = inp << "SELECT id FROM messages WHERE chat_id = ? ORDER BY db_timestamp"
		<< chat_id;
	while (rs.next()) {
		std::string id;
		rs >> id;
		res.emplace_back(std::move(id));
	}
	return res;
}

int main(int argc, const char *argv[])
{
	sqlite3_config(SQLITE_CONFIG_URI, 1);
	if (argc < 4) {
		std::cerr << "Usage: " << argv[0]
			<< " [input db filename]"
			<< " [output db filename]"
			<< " [chat_id]"
			<< "\n"
			<< "This utility copies messages with given chat_id from input db to output db\n"
			"in chronological order until (and excluding) the message\n"
			"breaking correct order_in_chain sequence\n";
		return 1;
	}
	try {
		std::string in_file_name = argv[1];
		std::string out_file_name = argv[2];
		std::string chat_id = argv[3];
		std::string tmp_name = "file:tmp.sqlite?mode=memory&cache=shared";

		auto history = get_message_history(in_file_name, chat_id);

		auto src = chat::make_chat_storage(get_dsn(in_file_name));

		CppSQLite3DB tmp_raw;
		tmp_raw.open(tmp_name.c_str());
		auto tmp = chat::make_chat_storage(get_dsn(tmp_name));
		chat::chain::ChainContainer cc;

		bool match = true;
		for (const auto& msg_id : history) {
			if (match) {
				tmp_raw.copyTo(out_file_name.c_str());
			}
			else {
				std::clog << "FAILURE\n";
				return 1;
			}

			auto msg = src->GetMessage(msg_id, {});

			auto parent_message_id = msg->GetParamStrRef(chat::attr::PREVIOUS_MESSAGE_ID_paramName);
			auto id = msg->GetParamStrRef(chat::attr::MESSAGE_ID_paramName);
			chat::ChatMessageTimestamp timestamp;
			msg->GetParam(chat::attr::TIMESTAMP_paramName, timestamp);

			cc.insert(id, parent_message_id, timestamp);
			tmp->SaveChatMessage(msg, "test", false);

			size_t msg_count = tmp->CountMessagesInChat(chat_id);
			auto db_msgs = tmp->GetLastMessages(chat_id, msg_count);

			match = std::equal(cc.begin(), cc.end(), db_msgs.begin(), db_msgs.end(),
				[](const auto& a, const auto& b) { return a.id == b.msg_id; });
		}
		tmp_raw.copyTo(out_file_name.c_str());
		std::clog << "SUCCESS\n";
		return 0;
	}
	catch (std::exception const &e) {
		std::cerr << "ERROR: " << e.what() << '\n';
		return 1;
	}
}