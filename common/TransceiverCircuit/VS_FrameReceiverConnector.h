#pragma once

#include <memory>

class VS_TransmitFrameInterface;

/**
	TODO:
	impliment ConnectToTransmitFrame and DisconnectFromTransmitFrame in this class;
**/
class VS_FrameReceiverConnector
{
public:
	virtual ~VS_FrameReceiverConnector(){}
	virtual bool ConnectToTransmitFrame(const char * conf_name, const std::shared_ptr<VS_TransmitFrameInterface> &trans_fr_cb) = 0;
	virtual bool DisconnectFromTransmitFrame(const char *conf_name, const std::shared_ptr<VS_TransmitFrameInterface> &trans_fr_cb) = 0;
};