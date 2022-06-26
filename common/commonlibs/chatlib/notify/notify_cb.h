#pragma once
#include "chatlib/callbacks.h"
namespace chat
{
namespace notify
{
using OnChatCreateCallBack = std::function<void(
	const GlobalContext&)>;
using OnPartAddedCallBack = std::function<void(
	ChatIDRef, CallIDRef,
	ChatMessageIDRef, CallIDRef)>;
using OnPartRemoveCallBack = std::function<void(
	ChatIDRef, CallIDRef, ChatMessageIDRef)>;
using OnPersonalContextUpdatedCallBack = std::function<void(
	const chat::PersonalContext&)>;
using OnGlobalContextUpdatedCallBack = std::function<void(
	const chat::GlobalContext&)>;
using OnPersonalContextsFetchedCallBack = std::function<void()>;
using OnErrMessageMalformedCallBack = std::function<void(
	const msg::ChatMessagePtr&, string_view)>;
using OnErrorCallBack = std::function<void(string_view, string_view)>;
using OnChainUpdatedCallBack = std::function<void(
	const msg::ChatMessagePtr&)>;
using OnMsgDeliveredCallBack = std::function<void(
	ChatMessageIDRef, CallIDRef)>;
using OnMsgReadCallBack = std::function<void(
	ChatIDRef, ChatMessageIDRef, CallIDRef)>;
using OnSyncStartCallBack = std::function<void(
	ChatIDRef, CallIDRef)>;
enum class SyncResult
{
	success,
	reset_by_sync_race,
	reset_by_update,
	reset_by_peer,
	reset_by_timeout,
	failed
};
using OnSyncEndCallBack = std::function<void(
	SyncResult, ChatIDRef, CallIDRef)>;
}
}