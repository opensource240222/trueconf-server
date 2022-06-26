#pragma once
#include "chatlib/storage/helpers.h"
#include <string>
namespace chat
{
namespace pg_qry
{
using ScriptDescr = chat::detail::ScriptDescr;
using ExecType = chat::detail::ExecType;
const std::vector<ScriptDescr> addMessage = {
	{"select * from chat.add_message(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);", ExecType::query, true} };
/*
	$1 = message_id
	$2 = chat_id
	$3 = time_stamp
	$4 = bucket_number
	$5 = type
	$6 = sender
	$7 = sender_type
	$8 = parent_message_id
	$9 = original_message_id
	$10 = content
	$11 = container
*/

const std::vector<ScriptDescr> updateMessageChain = {
	{"select * from chat.update_message_chain(?);", ExecType::query, true}};
/*
	$1 = msg_id;
*/
const std::vector<ScriptDescr> createGlobalContext =
{ {"select * from chat.create_global_context(?, ?, ?, ?, ?);", ExecType::query, true} };
/*
	$1 = message_id
	$2 = chat_id
	$3 = type
	$4 = sender
	$5 = content
*/
const std::vector<ScriptDescr> createPersonalContext = {
	{"select * from chat.create_personal_context(?, ?, ?);", ExecType::query, true}};
/*
	$1 = message_id
	$2 = type
	$3 = content
*/
const std::vector<ScriptDescr> getParticipants = {
	{"select * from chat.get_participants(?);", ExecType::query, true}};
;
/*
	$1 = chat_id
*/
const std::vector<ScriptDescr> hasMessage = {
	{"select chat.has_message(?);", ExecType::query, true}};
; // $1 - msg_id
const std::vector<ScriptDescr> getLastBucketNumber = {
	{"select * from chat.get_last_bucket_number(?);", ExecType::query, true}};
; // $1 - chat_id
const std::vector<ScriptDescr> getLastMessages = {
	{"select * from chat.get_last_messages(?, ?);", ExecType::query, true}};
;
/*
	$1 - chat_id
	$2 - tail len
*/
const std::vector<ScriptDescr> getMessage = {
	{"select * from chat.get_message(?, ?, ?);", ExecType::query, true}};
;
/*
	$1 - _message_id uuid
	$2 - _user varchar
	$3 - _is_active boolean
*/
const std::vector<ScriptDescr> getMessages = {
	{"select * from chat.get_messages(?, ?, ?, ?);", ExecType::query, true}};
;
/*
	$1 - _chat_id varchar,
	$2 - _user varchar,
	$3 - _cnt int,
	$4 - _to_message_id uuid = null,
*/
const std::vector<ScriptDescr> getMessagesByBucketNumber = {
	{"select * from chat.get_messages_by_bucket_number(?,?);", ExecType::query, true}};
;
/*
	$1 - chat_id
	$2 - bucker_number
*/
const std::vector<ScriptDescr> getGlobalContext = {
	{"select * from chat.get_global_context(?);", ExecType::query, true}};
; // $1 - chat_id
const std::vector<ScriptDescr> getUserPersonalContexts = {
	{"select * from chat.get_user_personal_contexts(?, ?, ?);", ExecType::query, true}};
 // $1 = call_id
 // $2 = cnt
 // $3 = page
const std::vector<ScriptDescr> countChatMessages = {
	{"select * from chat.count_chat_messages(?);", ExecType::query, true}};
; // $1 - chat_id
const std::vector<ScriptDescr> getMessagesToRetransmit = {
	{"select * from chat.get_messages_to_retransmit(?, ?, ?);", ExecType::query, true}
};
/*
	$1 - _chat_id varchar
	$2 - _buckets jsonb
	$3 - _messages jsonb

*/
const std::vector<ScriptDescr> addFirstVisibleMessage = {
	{"select * from chat.add_first_visible_message(?, ?);", ExecType::query, true}
};
/*
	$1 - _message_id uuid
	$2 - _user varchar
*/
const std::vector<ScriptDescr> addGlobalContext = {
	{"select * from chat.add_global_context(?, ?, ?, ?, ?, ?, ?, ?, ?, ?);", ExecType::query, true}};
;
/*
	$1 - chat_id
	$2 - chat_type
	$3 - chat_version
	$4 - chat_creator
	$5 - chat_created_at
	$6 - context_created_at
	$7 - message_id
	$8 - chat_title
	$9 - participants
	$10 - ban_list
*/
const std::vector<ScriptDescr> addPersonalContext = {
	{"select * from chat.add_personal_context(?,?,?,?,?,?,?,?,?,?,?,?,?);", ExecType::query, true}};
;
/*
	$1 - _owner
	$2 - _chat_id
	$3 - _chat_type
	$4 - _chat_version
	$5 - _chat_creator
	$6 - _chat_created_at
	$7 - _context_created_at
	$8 - _message_id
	$9 - _is_deleted
	$10 - _get_notifications
	$11 - _chat_title
	$12 - _unread_messages
	$13 - _draft
*/
const std::vector<ScriptDescr> getPersonalContext = {
	{"select * from chat.get_personal_context(?,?);", ExecType::query, true}};
; //$1 - chat_id; $2 - owner
const std::vector<ScriptDescr> addUndeliveredMessage = {
	{"select * from chat.add_undelivered_message(?);", ExecType::query, true}};
; //$1 - msgId;
const std::vector<ScriptDescr> removeUndeliveredMessage = {
	{"select * from chat.remove_undelivered_message(?, ?);", ExecType::query, true}};
;
/*
	$1 - msgId
	$2 - partName
*/
const std::vector<ScriptDescr> getUndeliveredMessagesByChatId = {
	{"select * from chat.get_undelivered_messages_by_chat_id(?);", ExecType::query, true}};
;
// $1 - chat_id
const std::vector<ScriptDescr> getAllUndeliveredMessages = {
	{"select * from chat.get_all_undelivered_messages();", ExecType::query, true}};
;
const std::vector<ScriptDescr> getMessagesForGlobalContext = {
	{"select * from chat.get_messages_for_global_context(?);", ExecType::query, true}};
;
// $1 - msg_id
const std::vector<ScriptDescr> getMessagesForPersonalContext = {
	{"select * from chat.get_messages_for_personal_context(?);", ExecType::query, true}};
;
// $1 - msg_id

}
}