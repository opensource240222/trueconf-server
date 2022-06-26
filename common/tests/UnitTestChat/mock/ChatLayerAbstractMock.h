#pragma once
#include "tests/UnitTestChat/SetLayerHelperSimple.h"
#include "tests/common/GMockOverride.h"
#include "std-generic/compat/map.h"
#include "std-generic/cpplib/StrCompare.h"

#include <gmock/gmock.h>

namespace chat
{
namespace notify
{
class ChatEventsFuncs;
}
}

class LayerInterface_MockAdapter : public chat::LayerInterface
{
private:
	void ForwardBelowMessage(chat::msg::ChatMessagePtr&&ptr,
		std::vector<chat::ParticipantDescr>&& parts) override final
	{
		ForwardBelowMessage(&ptr, &parts);
	}
public:
	virtual void ForwardBelowMessage(chat::msg::ChatMessagePtr*,
		std::vector<chat::ParticipantDescr>*) = 0;
};

class SetLayerHelper_MockAdapter : public chat_test::SetLayerHelperSimple
{
private:
	void OnChatMessageArrived(
		chat::msg::ChatMessagePtr&& m,
		chat::CallIDRef sender) override final
	{
		OnChatMessageArrived(&m,sender);
	}
public:
	virtual void OnChatMessageArrived(
		chat::msg::ChatMessagePtr*,
		chat::CallIDRef sender) = 0;
};


class LayerInterfaceAgregator_Mock : public LayerInterface_MockAdapter,
		public SetLayerHelper_MockAdapter
{
	void ShutDown() override
	{
		SetLayerHelper_MockAdapter::ShutDown();
	}
};

class SharedLeakDetect_Mock
{
public:
	MOCK_METHOD0(MethodForMock, void());
};
class LayerInterfaceAgregator_Stub : public LayerInterfaceAgregator_Mock
{
	void ForwardBelowMessage(chat::msg::ChatMessagePtr*,
		std::vector<chat::ParticipantDescr>*) override;
	void OnChatMessageArrived(
		chat::msg::ChatMessagePtr*,
		chat::CallIDRef sender) override;

	std::shared_ptr<SharedLeakDetect_Mock> leak_control_
		= std::make_shared<SharedLeakDetect_Mock>();
public:
	LayerInterfaceAgregator_Stub();
};
class LayerInterface_Mock : public LayerInterfaceAgregator_Mock
{
public:
	LayerInterface_Mock()
	{}
	explicit LayerInterface_Mock(const std::shared_ptr<LayerInterfaceAgregator_Mock> &fake)
		: default_handler_(fake)
	{}
	MOCK_METHOD2_OVERRIDE(ForwardBelowMessage,
		void(chat::msg::ChatMessagePtr* msg,
			std::vector<chat::ParticipantDescr>* parts));
	MOCK_METHOD2_OVERRIDE(OnChatMessageArrived,
		void(chat::msg::ChatMessagePtr* msg, chat::CallIDRef));
	void DelegateToDefault()
	{
		ON_CALL(*this, ForwardBelowMessage(::testing::_, ::testing::_))
			.WillByDefault(::testing::Invoke(
				[this](auto msg, auto parts)
		{
			if (default_handler_)
				default_handler_->ForwardBelowMessage(msg, parts);
		}));
		ON_CALL(*this, OnChatMessageArrived(::testing::_, ::testing::_))
			.WillByDefault(::testing::Invoke(
				[this](
					chat::msg::ChatMessagePtr*msg,
					chat::CallIDRef sender){
			if (default_handler_)
				default_handler_->OnChatMessageArrived(msg, sender);
		}));
	}
	void ClearDefault()
	{
		default_handler_.reset();
	}
private:
	std::shared_ptr<LayerInterfaceAgregator_Mock> default_handler_;
};