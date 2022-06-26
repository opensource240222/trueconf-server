#include "VS_RelayModule.h"
#include "VS_RelayMessageSenderInterface.h"


void VS_RelayModule::SetMessageSender(const std::shared_ptr<VS_RelayMessageSenderInterface>& sender, string_view senderKey)
{
	m_message_senders[std::string(senderKey)] = sender;
}

bool VS_RelayModule::HasSender(string_view senderKey)
{
	return m_message_senders.find(senderKey) != m_message_senders.end();
}

void VS_RelayModule::RemoveMessageSender(string_view senderKey)
{
	auto sender_it = m_message_senders.find(senderKey);
	if (sender_it == m_message_senders.end()) return;

	m_message_senders.erase(sender_it);
}

bool VS_RelayModule::SendMsg(const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess, string_view senderKey)
{
	auto sender_it = m_message_senders.find(senderKey);
	if (sender_it == m_message_senders.end()) return false;
	auto sender = sender_it->second.lock();
	if(!sender)
		return false;
	return sender->SendMsg(mess);
}