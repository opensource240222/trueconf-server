#pragma once
namespace chat
{
namespace attr
{
extern const char MESSAGE_TYPE_paramName[]; //CreateChat, Invite, join and etc
extern const char CHAT_ID_paramName[];
extern const char TIMESTAMP_paramName[];
extern const char MESSAGE_ID_paramName[];
extern const char PREVIOUS_MESSAGE_ID_paramName[];
extern const char ORIGINAL_MESSAGE_ID_paramName[];
extern const char FROM_paramName[];
extern const char FROM_INSTANCE_paramName[];
extern const char SENDER_TYPE_paramName[];
extern const char RESPONSE_CODE_paramName[];
extern const char MESSAGE_CONTENT_paramName[];
extern const char GLOBAL_CONTEXT_paramName[];
extern const char PERSONAL_CONTEXTS_paramName[];
extern const char BLOB_paramName[];

extern const char TAIL_HASH_paramName[];
extern const char TAIL_LENGTH_paramName[];
extern const char MASTER_WEIGHT_paramName[];

extern const char BUCKET_paramName[];
extern const char BUCKETSYNC_METHOD_paramName[];
extern const char SYNC_ID_paramName[];
extern const char BUCKET_HASH_paramName[];

extern const char SYNC_BUCKET_START[];
extern const char SYNC_BUCKET_LIST_REQ[];
extern const char SYNC_BUCKET_LIST_RESP[];
extern const char GET_DIFF_MSG_REQ[];
extern const char GET_DIFF_MSG_RESP[];

extern const char SYNC_CHECK_REQ[];
extern const char SYNC_CHECK_RESP[];

extern const char RESET_SYNC_REQ[];
extern const char INSERT_MSG[];
extern const char INSERTED_MSG_ID_paramName[];

extern const char DST_ENDPOINT_paramName[];
extern const char DST_CALLID_paramName[];
extern const char SRC_ENDPOINT_paramName[];

extern const char REQ_ID_paramName[];
extern const char ERROR_CODE_paramName[];
extern const char RETRANSMITS_paramName[];
}
enum class MessageType
{
	undefined = 0,
	//x or xx service messages
	sync = 1,
	delivered = 2,
	fetch_all_personal_ctxs_req = 3,
	fetch_all_personal_ctxs_resp = 4,
	get_global_ctx_req = 5,
	get_global_ctx_resp = 6,
	get_tail_hash_req = 7,
	get_tail_hash_resp = 8,
	// 1xx chat control
	create_chat = 100,
	create_p2p_chat = 101,
	edit_chat_info = 102,
	lock_chat = 103,
	unlock_chat = 104,
	add_part = 110,
	remove_part = 111,
	remove_all_parts = 112,
	invite = 113,
	invite_response = 114,
	change_part_permission = 115,
	shared_chat_req = 130,
	shared_chat_resp = 131,
	// 2xx regular messages
	text = 200,
	filetransfer = 201,
	contact = 202,
	location = 203,
	forward = 220,
	del_msg_self = 221,
	del_msg_all = 222,
	clear_history_for_part = 223,
	clear_history_for_all = 224,
	// 3xx call history
	// p2p
	start_call = 300,
	end_call = 301,
	missed_call = 302,
	reject_call = 303,
	cancel_call = 304,
	// group conf
	invite_to_conf = 310,
	start_conf = 311,
	end_conf = 312,
	join_to_conf = 313,
	leave_from_conf = 314,
	// 4xx system chat (chat-of-chats)
	add_part_notification = 400,
	remove_part_notification = 401,
	mute = 402,
	unmute = 403,
	draft = 410,
	unread_msg = 420,
	read_msg = 421,
};
}