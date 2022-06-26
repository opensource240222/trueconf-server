#pragma once
#include <boost/shared_ptr.hpp>
class VS_MessageData;
class VS_MessResult;

class VS_MessageHandler
{
public:
	virtual ~VS_MessageHandler() {};
	virtual void HandleMessage(const boost::shared_ptr<VS_MessageData> &message) = 0;
	virtual void HandleMessageWithResult(const boost::shared_ptr<VS_MessageData> &/*message*/, const boost::shared_ptr<VS_MessResult> &/*res*/) {}
};