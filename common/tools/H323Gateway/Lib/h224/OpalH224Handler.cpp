#include "OpalH224Handler.h"

#include "VS_H224H281Handler.h"
#include "Transcoder/RTPPacket.h"

#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

OpalH224Handler::OpalH224Handler(unsigned char pt,
	VS_Q922Frame::CodecType ct,
	uint32_t seq_num)
	: m_seq_num(seq_num), m_payload_type(pt), m_codec_type(ct)
#ifdef H323_H235
	, secChannel(NULL)
#endif
	{}


OpalH224Handler::~OpalH224Handler()
{
	m_onRecvDataSigConn.disconnect();
}

std::shared_ptr<RTPPacket> OpalH224Handler::MakeStartAction(VS_H281Frame::ePanDirection panDirection,
	VS_H281Frame::eTiltDirection tiltDirection,
	VS_H281Frame::eZoomDirection zoomDirection,
	VS_H281Frame::eFocusDirection focusDirection)
{
	std::shared_ptr<VS_H224H281Handler> h224281handler = GetH224H281Handler();
	if (h224281handler)
		return h224281handler->MakeStartActionRTP(panDirection, tiltDirection, zoomDirection, focusDirection);
	return {};
}

std::shared_ptr<RTPPacket> OpalH224Handler::MakeStopAction()
{
	std::shared_ptr<VS_H224H281Handler> h224281handler = GetH224H281Handler();
	if (h224281handler)
		return h224281handler->MakeStopActionRTP();
	return {};
}

void OpalH224Handler::CreateHandlers()
{
	std::vector<unsigned char> handlers{ VS_H281_CLIENT_ID };

	for (const auto &handler_id : handlers) {
		auto handler = VS_H224Handler::CreateHandler(handler_id);
		if (handler)
			handler->AttachH224Handler(shared_from_this());
		else continue;

		m_h224Handlers->emplace(std::pair<unsigned char, std::shared_ptr<VS_H224Handler> >(handler->GetClientID(), handler));
	}
}

int CalculateClientListSize(const std::map<unsigned char, std::shared_ptr<VS_H224Handler> > &handlers)
{
	int i = 3;
	for (auto it = handlers.begin(); it != handlers.end(); ++it) {
		unsigned char clientID = it->first;
		if (clientID == 0x7e) { // extended client ID
			i += 2;
		}
		else if (clientID == 0x7f) { // non-standard client ID
			i += 6;
		}
		else { // standard client ID
			i++;
		}
	}
	return i;
}

std::shared_ptr<RTPPacket> OpalH224Handler::MakeClientListRTP()
{
	int size;
	int count;

	{ // lock handlers
		auto locked_handlers = m_h224Handlers.lock();

		count = locked_handlers->size();
		if (!count)
			return {};

		size = CalculateClientListSize(*locked_handlers);
	} // unlock handlers

	VS_H224Frame h224Frame(size);
	h224Frame.SetHighPriority(true);
	h224Frame.SetDestinationTerminalAddress(VS_H224_BROADCAST);
	h224Frame.SetSourceTerminalAddress(VS_H224_BROADCAST);

	// CME frame
	h224Frame.SetClientID(0x00);

	// Begin and end of sequence
	h224Frame.SetBS(true);
	h224Frame.SetES(true);
	h224Frame.SetC1(false);
	h224Frame.SetC0(false);
	h224Frame.SetSegmentNumber(0);

	unsigned char *ptr = h224Frame.GetClientDataPtr();

	ptr[0] = 0x01; // Client list code
	ptr[1] = 0x00; // Message code
	ptr[2] = static_cast<unsigned char>(count); // client count

	{ // lock handlers
		int i = 3;
		auto locked_handlers = m_h224Handlers.lock();
		for (auto it = locked_handlers->begin(); it != locked_handlers->end(); ++it) {
			if (it->second->IsActive()) {
				unsigned char clientID = it->first;
				ptr[i] = static_cast<unsigned char>(0x80 | clientID);
				if (clientID == 0x7e) { // extended client ID
					i += 2;
				}
				else if (clientID == 0x7f) { // non-standard client ID
					i += 6;
				}
				else { // standard client ID
					i++;
				}
			}
		}
	} // unlock handlers

	dprint4("H224 \tSend Client List");
	return MakeRTP(h224Frame);
}

std::shared_ptr<RTPPacket> OpalH224Handler::MakeExtraCapabilitiesRTP()
{
	auto locked_handlers = m_h224Handlers.lock();
	for (auto it = locked_handlers->begin(); it != locked_handlers->end(); ++it) {
		if (it->second->IsActive())
			return it->second->MakeExtraCapabilitiesRTP();
	}

	return {};
}

std::shared_ptr<RTPPacket> OpalH224Handler::MakeExtraCapabilitiesMessageRTP(unsigned char clientID,
	unsigned char* data, unsigned long length)
{
	if (!data)
		return {};

	VS_H224Frame h224Frame = VS_H224Frame(length + 3);
	h224Frame.SetHighPriority(true);
	h224Frame.SetDestinationTerminalAddress(VS_H224_BROADCAST);
	h224Frame.SetSourceTerminalAddress(VS_H224_BROADCAST);

	// use clientID zero to indicate a CME frame
	h224Frame.SetClientID(0x00);

	// Begin and end of sequence, rest is zero
	h224Frame.SetBS(true);
	h224Frame.SetES(true);
	h224Frame.SetC1(false);
	h224Frame.SetC0(false);
	h224Frame.SetSegmentNumber(0);

	unsigned char *ptr = h224Frame.GetClientDataPtr();

	ptr[0] = 0x02; // Extra Capabilities code
	ptr[1] = 0x00; // Response Code
	ptr[2] = (0x80 | clientID); // EX CAPS and ClientID

	memcpy(ptr + 3, data, length);

	dprint4("H224 \tSend Extra Capabilities Message");
	return MakeRTP(h224Frame);
}

std::shared_ptr<RTPPacket> OpalH224Handler::OnReceivedFrame(VS_H224Frame & frame)
{
	if (frame.GetDestinationTerminalAddress() != VS_H224_BROADCAST) {
		// only broadcast frames are handled at the moment
		dprint4("H224 \tReceived frame with non-broadcast address (IGNORING)");
		return {};
	}
	unsigned char clientID = frame.GetClientID();

	if (clientID == 0x00) {
		return OnReceivedCMEMessage(frame);
	}
	dprint4("H224 \tReceived frame for ClientID %d", clientID);

	return {};
}

std::shared_ptr<RTPPacket> OpalH224Handler::OnReceivedCMEMessage(VS_H224Frame & frame)
{
	unsigned char *data = frame.GetClientDataPtr();

	if (data[0] == 0x01) { // Client list code

		if (data[1] == 0x00) { // Message
			dprint4("H224 \tReceived Client List");
			OnReceivedClientList(frame);
		}
		else if (data[1] == 0xff) { // Command
			dprint4("H224 \tReceived Client List Command");
			return OnReceivedClientListCommand();
		}

	}
	else if (data[0] == 0x02) { // Extra Capabilities code

		if (data[1] == 0x00) { // Message
			dprint4("H224 \tReceived Extra Capabilities");
			OnReceivedExtraCapabilities(frame);
		}
		else if (data[1] == 0xff) {// Command
			dprint4("H224 \tReceived Extra Capabilities Command");
			return OnReceivedExtraCapabilitiesCommand();
		}
	}

	return {};
}

void OpalH224Handler::OnReceivedClientList(VS_H224Frame & frame)
{
	unsigned char *data = frame.GetClientDataPtr();

	unsigned char numberOfClients = data[2];

	unsigned long i = 3;

	while (numberOfClients > 0) {

		unsigned char clientID = static_cast<unsigned char>(data[i] & 0x7f);

		auto locked_handlers = m_h224Handlers.lock();
		for (auto it = locked_handlers->begin(); it != locked_handlers->end(); ++it) {
			if (clientID == it->first) {
				it->second->SetRemoteSupport();
				break;
			}
		}

		if (clientID == 0x7e) { // extended client ID
			i += 2;
		}
		else if (clientID == 0x7f) { // non-standard client ID
			i += 6;
		}
		else { // standard client ID
			i++;
		}
		numberOfClients--;
	}
}

std::shared_ptr<RTPPacket> OpalH224Handler::OnReceivedClientListCommand()
{
	return MakeClientListRTP();
}

void OpalH224Handler::OnReceivedExtraCapabilities(VS_H224Frame & frame)
{
	unsigned char *data = frame.GetClientDataPtr();

	unsigned char clientID = (data[2] & 0x7f);

	auto locked_handlers = m_h224Handlers.lock();
	for (auto it = locked_handlers->begin(); it != locked_handlers->end(); ++it) {
		if (clientID == it->first) {
			it->second->OnReceivedExtraCapabilities((data + 3), frame.GetClientDataSize() - 3);
			break;
		}
	}
}

std::shared_ptr<RTPPacket> OpalH224Handler::OnReceivedExtraCapabilitiesCommand()
{
	return MakeExtraCapabilitiesRTP();
}

std::shared_ptr<VS_H224Handler> OpalH224Handler::GetHandler(unsigned char clientID)
{
	auto locked_handlers = m_h224Handlers.lock();
	auto handler = locked_handlers->find(clientID);
	if (handler != locked_handlers->end())
		return handler->second;
	return nullptr;
}

std::shared_ptr<VS_H224H281Handler> OpalH224Handler::GetH224H281Handler()
{
	return std::static_pointer_cast<VS_H224H281Handler>(GetHandler(VS_H281_CLIENT_ID));
}

std::shared_ptr<RTPPacket> OpalH224Handler::MakeRTP(VS_H224Frame& frame)
{
	std::shared_ptr<RTPPacket> result = std::make_shared<RTPPacket>();

	result->PayloadType(m_payload_type);

	unsigned long size = frame.GetEncodedSize();
	unsigned char buffer[500];

	if (!frame.Encode(buffer, size, m_codec_type)) {
		dprint3("H224 \tFailed to encode H.224 frame");
		return {};
	}

	// determining correct timestamp
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> timePassed = currentTime - transmitStartTime;
	result->Timestamp(static_cast<uint32_t>(timePassed.count()) * 8);

	result->SetData(buffer, size, true);
	result->Marker(false);
	result->SeqNo(m_seq_num++);
	return result;
}

#ifdef H323_H235
void OpalH224Handler::AttachSecureChannel(H323SecureChannel * channel)
{
	secChannel = channel;
}
#endif

bool OpalH224Handler::OnReadFrame(RTPPacket & frame)
{
#ifdef H323_H235
	if (secChannel)
		return secChannel->ReadFrame(frame);
	else
#endif
		return true;
}

bool OpalH224Handler::OnWriteFrame(RTPPacket & frame)
{
#ifdef H323_H235
	if (secChannel)
		return secChannel->WriteFrame(frame);
	else
#endif
		return true;
}
