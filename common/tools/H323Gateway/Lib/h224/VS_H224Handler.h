#pragma once

#include "VS_H224Frame.h"
#include <memory>

class OpalH224Handler;
class RTPPacket;

class VS_H224Handler
{
public:
	explicit VS_H224Handler(const std::string & name);
	virtual ~VS_H224Handler();

	virtual bool IsActive() const { return true; } // may be used to disable some handlers at runtime

	void AttachH224Handler(const std::shared_ptr<OpalH224Handler> &h224Handler);

	virtual unsigned char GetClientID() const = 0;
	virtual void SetRemoteSupport() = 0;
	virtual void SetLocalSupport() = 0;
	virtual void OnRemoteSupportDetected() = 0;
	virtual std::shared_ptr<RTPPacket> MakeExtraCapabilitiesRTP() const = 0;
	virtual void OnReceivedExtraCapabilities(const unsigned char *capabilities, unsigned long size) = 0;
	virtual void OnReceivedMessage(const VS_H224Frame & message) = 0;

	static std::shared_ptr<VS_H224Handler>  CreateHandler(unsigned char clientId);
protected:
	std::weak_ptr<OpalH224Handler>        m_h224Handler;
	std::string              m_h224Display;
};

