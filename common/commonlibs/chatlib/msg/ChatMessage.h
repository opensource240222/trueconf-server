#pragma once
#include "chatlib/callbacks.h"
#include "chatlib/chat_defs.h"

#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/cpplib/macro_utils.h"

#include <functional>
#include <list>
#include <vector>

namespace chat
{
namespace msg
{
class ChatMessage // access to container
{
	struct ChainUpdateResSt
	{
		cb::ProcessingResult res
			= cb::ProcessingResult::undef;
		cb::MsgIdAndOrderInChain updatedOrder;
	};
public:
	ChatMessage();
	explicit ChatMessage(const VS_Container&cnt);
	~ChatMessage();
	template <typename T>
	bool SetParam(string_view param, const T&val){
		return m_cnt.AddValue(param, val);
	}
	template<typename T>
	bool SetParamI32(string_view param, const T&v)
	{
		return m_cnt.AddValueI32(param, v);
	}
	template<typename T>
	bool SetParamI64(string_view param, const T&v)
	{
		return m_cnt.AddValueI64(param, v);
	}
	bool SetParam(string_view param, int64_t val){
		return m_cnt.AddValueI64(param, val);
	}
	bool SetParam(string_view name, const void* val, const size_t size)
	{
		return m_cnt.AddValue(name, val, size);
	}
	bool DeleteParam(string_view param)
	{
		return m_cnt.DeleteValue(param);
	}
	template <typename T>
	bool GetParam(const char *param, T&out) const
	{
		return m_cnt.GetValue(param, out);
	}
	bool GetParam(const char *param, uint32_t &out) const
	{
		return m_cnt.GetValueI32(param, out);
	}
	template <typename T>
	bool GetParamI32(const char *param, T& out) const
	{
		return m_cnt.GetValueI32(param, out);
	}
	template <typename T>
	bool GetParamI64(const char *param, T& out) const
	{
		return m_cnt.GetValueI64(param, out);
	}
	bool GetParam(const char *param, std::string &out) const;
	bool GetParam(const char *param, ChatMessageTimestamp &out) const;
	const void * GetParamBinRef(const char *param, size_t &size) const
	{
		return m_cnt.GetBinValueRef(param, size);
	}
	bool SetParam(string_view param, const std::string &val);
	void OnChainUpdateByMsg(
		cb::ProcessingResult res,
		cb::MsgIdAndOrderInChain&& updatedOrder);
	const ChainUpdateResSt &
		GetMsgInChainresult() const
	{
		return m_chainUpdateRes;
	}
	void OnMsgIsStored(cb::ProcessingResult res);
	void AddOnChainUpdateByMsgCallBack(const cb::OnChainUpdateByMsgCb& cb);
	void AddOnMsgIsStoredCallBack(const cb::OnMessageIsStoredCallBack &cb);
	size_t count() const;
	void Clear()
	{
		m_cnt.Clear();
	}
	cstring_view GetParamStrRef(const char *paramName) const;
	std::string GetParamStr(const char* paramName) const;
	const VS_Container &GetContainer() const;
protected:
	VS_Container m_cnt;
	vs::Signal<void(
		cb::ProcessingResult,
		ChatMessageIDRef,
		const cb::MsgIdAndOrderInChain&)>
		m_onChainUpdateByMsg;
	vs::Signal<void(
		cb::ProcessingResult,
		ChatIDRef,
		ChatMessageIDRef)> m_onMsgStored;
private:
	ChainUpdateResSt m_chainUpdateRes;
	cb::ProcessingResult m_onMsgStoredRes = cb::ProcessingResult::undef;
};
class ChatMessageKeeper
{
protected:
	ChatMessagePtr m_message;
public:
	ChatMessageKeeper();
	explicit ChatMessageKeeper(ChatMessagePtr&& msg);
	ChatMessagePtr AcquireMessage();
};


}
}