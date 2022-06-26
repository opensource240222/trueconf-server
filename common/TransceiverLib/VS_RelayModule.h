#pragma once

#include <boost/weak_ptr.hpp>
#include "std-generic/cpplib/StrCompare.h"

#include "std-generic/compat/map.h"
#include <memory>
#include <string>

class VS_NetworkRelayMessageBase;
class VS_RelayMessageSenderInterface;
class VS_RelayModule
{
public:
	explicit VS_RelayModule(const char *module_name):m_moduleName(module_name)
	{}
	virtual ~VS_RelayModule() {}
	virtual bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess) = 0;
	virtual bool SendMsg(const boost::shared_ptr<VS_NetworkRelayMessageBase>& mess, string_view senderKey = {});
	virtual void SetMessageSender(const std::shared_ptr<VS_RelayMessageSenderInterface>& sender, string_view senderKey = {});
	virtual bool HasSender(string_view senderKey);
	virtual void RemoveMessageSender(string_view senderKey);

	const std::string& GetModuleName() const { return m_moduleName; }
protected:
	vs::map<std::string, std::weak_ptr<VS_RelayMessageSenderInterface>, vs::str_less> m_message_senders;
private:
	const std::string	m_moduleName;
};
