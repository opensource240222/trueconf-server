#pragma once

#include "std-generic/cpplib/VS_Container.h"

#include <cstdint>
#include <ctime>
#include <functional>
#include <memory>
#include <string>

class ChatDBInterface {
public:
	typedef std::function<bool(
		const int64_t &mID,
		const VS_Container *cnt,
		const char *message,
		const std::time_t *timestamp,
		const bool *is_offline,
		const char *from_call_id,
		const char *from_display_name,
		const char *to_call_id,
		const char *conference_id)> ProcessMessageFunc;
public:
	static std::unique_ptr<ChatDBInterface> Create(void);

	virtual ~ChatDBInterface() {}
	virtual bool Init(const std::string& db_conn_str) = 0;

	virtual bool AddNewMessage(int64_t &out_mid,
		const char *message,
		const std::time_t *timestamp = nullptr,
		const char *from_call_id = nullptr,
		const char *from_display_name = nullptr,
		const char *to_call_id = nullptr,
		const char *conference_id = nullptr) = 0;

	virtual bool ChangeMessage(const int64_t mid,
		const VS_Container *cnt = nullptr,
		const char *message = nullptr,
		const std::time_t *timestamp = nullptr,
		const bool *is_offline = nullptr,
		const char *from_call_id = nullptr,
		const char *from_display_name = nullptr,
		const char *to_call_id = nullptr,
		const char *conference_id = nullptr) = 0;

	virtual bool MarkRoamingMessageAsDelivered(const char *from_call_id,
		const char *from_display_name,
		const char *to_call_id,
		const char *message) = 0;

	virtual bool ProcessUserOfflineMessages(const ProcessMessageFunc &func, const char *user_id, const size_t count = 80) = 0;

	virtual bool ProcessRoamingOfflineMessages(const ProcessMessageFunc &func, const char *our_server_id, const size_t count = 80) = 0;
};
