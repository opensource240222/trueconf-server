#pragma once

#include <boost/signals2/connection.hpp>

#include "std-generic/compat/map.h"
#include "std-generic/cpplib/synchronized.h"

#include "VS_H281Frame.h"

class RTPPacket;
class VS_H224Handler;
class VS_H224H281Handler;

class OpalH224Handler : public std::enable_shared_from_this<OpalH224Handler>
{
public:

	virtual ~OpalH224Handler();

	std::shared_ptr<RTPPacket> MakeStartAction(VS_H281Frame::ePanDirection panDirection,
		VS_H281Frame::eTiltDirection tiltDirection,
		VS_H281Frame::eZoomDirection zoomDirection,
		VS_H281Frame::eFocusDirection focusDirection);
	std::shared_ptr<RTPPacket> MakeStopAction();

	void CreateHandlers();
	void DeleteHandlers();

	std::shared_ptr<RTPPacket> MakeClientListRTP();
	std::shared_ptr<RTPPacket> MakeExtraCapabilitiesRTP();
	std::shared_ptr<RTPPacket> MakeExtraCapabilitiesMessageRTP(unsigned char clientID, unsigned char* data,
		unsigned long length);
	std::shared_ptr<RTPPacket> SendExtraCapabilitiesCommand();
	std::shared_ptr<RTPPacket> MakeRTP(VS_H224Frame& frame);

	virtual std::shared_ptr<RTPPacket> OnReceivedFrame(VS_H224Frame & frame);
	virtual std::shared_ptr<RTPPacket> OnReceivedCMEMessage(VS_H224Frame & frame);
	virtual void OnReceivedClientList(VS_H224Frame & frame);
	virtual std::shared_ptr<RTPPacket> OnReceivedClientListCommand();
	virtual void OnReceivedExtraCapabilities(VS_H224Frame & frame);
	virtual std::shared_ptr<RTPPacket> OnReceivedExtraCapabilitiesCommand();

	VS_Q922Frame::CodecType GetQ922CodecType() const { return m_codec_type; }
	void SetQ922CodecType(VS_Q922Frame::CodecType ct) { m_codec_type = ct; }

	unsigned char GetPT() const { return m_payload_type; }
	void SetPT(unsigned char pt) { m_payload_type = pt; }

	std::shared_ptr<VS_H224Handler> GetHandler(unsigned char clientID);
//	Since there is only one client ID being used right now, we take clientID handling upon ourselves
	std::shared_ptr<VS_H224H281Handler> GetH224H281Handler();

#ifdef H323_H235
	void AttachSecureChannel(H323SecureChannel * channel);
#endif

	bool OnReadFrame(RTPPacket & frame);
	bool OnWriteFrame(RTPPacket & frame);

protected:

	uint32_t m_seq_num;// Sequence number

	std::chrono::time_point<std::chrono::high_resolution_clock> transmitStartTime;

	vs::Synchronized<vs::map<unsigned char, std::shared_ptr<VS_H224Handler>>> m_h224Handlers;

	unsigned char m_payload_type;
	VS_Q922Frame::CodecType m_codec_type;

#ifdef H323_H235
	H323SecureChannel * secChannel;
#endif

protected:
	OpalH224Handler(unsigned char pt,
		VS_Q922Frame::CodecType ct = VS_Q922Frame::ExtendedCodec,
		uint32_t seq_num = 0);

private:
	boost::signals2::connection m_onRecvDataSigConn;

};
