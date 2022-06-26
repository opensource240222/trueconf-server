#pragma once
#include "chatlib/layers/ChatLayerAbstract.h"

namespace chat
{
template<typename LayersTraits>
class SystemChatLayer
	: public LayerInterface
	, public LayerHelper<LayersTraits>
{
	void ForwardBelowMessage(msg::ChatMessagePtr &&msg,
		std::vector<ParticipantDescr>&& dstParts) override;
	void OnChatMessageArrived(
		msg::ChatMessagePtr &&msg,
		CallIDRef sender) override;
	void ShutDown() override;
public:
	template<typename ...Args>
	SystemChatLayer(const GlobalConfigPtr& cfg, Args&&... args);
private:
	void HandleOnAddPart(ChatIDRef chatId, CallIDRef partId,
		ChatMessageID&& msgId, CallID&& author);
	void HandleOnRemovePart(ChatIDRef chatId,
		CallIDRef part, ChatMessageID&& msgId);
	void HandleOnChatCreated(GlobalContext&& chatInfo);
	template<typename CallBack>
	void GetSysChatForSend(CallIDRef part, ChatIDRef chatWhere, CallBack&& cb);
	void SendPartAddedToChatMessage(ChatIDRef sysChatId, CallIDRef to,
		const GlobalContext& chatInfo, ChatMessageIDRef triggerMsg,
		CallIDRef addedPart, CallIDRef whoAdd, CallIDRef p2pPart);
	GlobalConfigPtr GetCfg() const;
	std::weak_ptr<GlobalConfigInterface> m_cfg;
};
}