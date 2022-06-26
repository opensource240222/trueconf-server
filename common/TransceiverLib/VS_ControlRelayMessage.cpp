#include "VS_ControlRelayMessage.h"
#include "../streams/Protocol.h"
#include "../streams/VS_StreamsDefinitions.h"
#include "../std/cpplib/VS_MediaFormat.h"
#include "std-generic/cpplib/string_view.h"

#include <cstdint>
#include <cstring>

VS_ControlRelayMessage::VS_ControlRelayMessage(){}
VS_ControlRelayMessage::~VS_ControlRelayMessage(){}

bool VS_ControlRelayMessage::IsValid() const
{
	switch(GetMessageType()){
		case e_transmit_frame:
			if(!GetConferenceName() || !GetParticipantName() || !GetFrameHead() || !GetFrameData()) return false;
			break;
		case e_start_conference:
			if(!GetConferenceName()) return false;
			break;
		case e_stop_conference:
			if(!GetConferenceName()) return false;
			break;
		case e_part_connect:
			if(!GetConferenceName() || !GetParticipantName()) return false;
			break;
		case e_part_disconenct:
			if(!GetConferenceName() || !GetParticipantName()) return false;
			break;
		case e_set_caps:
			{
				size_t temp_l;
				if(!GetConferenceName() || !GetParticipantName() || !GetCaps(temp_l)) return false;
			}
			break;
		case e_restrict_btr_svc:
			{
				int32_t v_btr = 0;
				int32_t btr = 0;
				int32_t o_btr = 0;
				if(!GetConferenceName() || !GetParticipantName() || !GetSVCBitrate(v_btr,btr,o_btr)) return false;
				break;
			}
		case e_request_key_frame:
			{
				if(!GetConferenceName() || !GetParticipantName()) return false;
				break;
			}
		case e_live555_info:
			return GetLive555Port() != 0;
		default:
			return false;
	}
	return true;
}

bool VS_ControlRelayMessage::MakeConferenceStart(const char *conference_name)
{
	Clear();
	if(strlen(conference_name)>VS_STREAMS_MAX_SIZE_CONFERENCE_NAME)
		return false;
	m_mess->reserve(strlen(conference_name) + 1 +sizeof(RelayMessFixedPart));
	m_mess->resize(sizeof(RelayMessFixedPart));
	RelayMessFixedPart *header = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	header->mess_type = e_start_conference;
	header->body_len = strlen(conference_name) + 1;
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(conference_name),&(reinterpret_cast<const unsigned char*>(conference_name))[strlen(conference_name)+1]);
	m_mess_size = m_mess->size();
	m_isComplete = true;
	return true;
}
bool VS_ControlRelayMessage::MakeConferenceStart(const char *conference_name, const char *owner_name, VS_Conference_Type type, VS_GroupConf_SubType sub_type)
{
	Clear();
	if(strlen(conference_name)>VS_STREAMS_MAX_SIZE_CONFERENCE_NAME)
		return false;
	unsigned long mess_size = strlen(conference_name) + 1 + strlen(owner_name) + 1 + sizeof(RelayMessFixedPart) + 8;
	m_mess->reserve(mess_size);
	m_mess->resize(sizeof(RelayMessFixedPart));
	RelayMessFixedPart *header = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	header->mess_type = e_start_conference;
	header->body_len = strlen(conference_name) + 1 + strlen(owner_name) + 1 + 8;
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(conference_name),&(reinterpret_cast<const unsigned char*>(conference_name))[strlen(conference_name)+1]);
	{
		const int32_t val = type;
		m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(&val),&(reinterpret_cast<const unsigned char*>(&val))[4]);
	}
	{
		const int32_t val = sub_type;
		m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(&val),&(reinterpret_cast<const unsigned char*>(&val))[4]);
	}
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(owner_name),&(reinterpret_cast<const unsigned char*>(owner_name))[strlen(owner_name)+1]);
	m_mess_size = m_mess->size();
	m_isComplete = true;
	return true;
}
bool VS_ControlRelayMessage::MakeConferencsStop(const char *conference_name)
{
	Clear();
	if(strlen(conference_name)>VS_STREAMS_MAX_SIZE_CONFERENCE_NAME)
		return false;
	m_mess->reserve(strlen(conference_name) + 1 +sizeof(RelayMessFixedPart));
	m_mess->resize(sizeof(RelayMessFixedPart));
	RelayMessFixedPart *header = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	header->mess_type = e_stop_conference;
	header->body_len = strlen(conference_name) + 1;
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(conference_name),&(reinterpret_cast<const unsigned char*>(conference_name))[strlen(conference_name)+1]);
	m_mess_size = m_mess->size();
	m_isComplete = true;
	return true;
}
bool VS_ControlRelayMessage::MakeParticipantConnect(const char *conference_name, const char *part_name)
{
	Clear();
	if(strlen(conference_name)>VS_STREAMS_MAX_SIZE_CONFERENCE_NAME || strlen(part_name) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
		return false;
	unsigned long mess_size = sizeof(RelayMessFixedPart) + strlen(conference_name) + 1 + strlen(part_name) + 1;
	m_mess->reserve(mess_size);
	m_mess->resize(sizeof(RelayMessFixedPart));
	RelayMessFixedPart *header = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	header->mess_type = e_part_connect;
	header->body_len = mess_size - sizeof(RelayMessFixedPart);
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(conference_name),&(reinterpret_cast<const unsigned char*>(conference_name))[strlen(conference_name)+1]);
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(part_name),&(reinterpret_cast<const unsigned char*>(part_name))[strlen(part_name)+1]);
	m_mess_size = m_mess->size();
	m_isComplete = true;
	return true;
}
bool VS_ControlRelayMessage::MakeParticipantDisconnect(const char *conference_name, const char *part_name)
{
	Clear();
	if(strlen(conference_name)>VS_STREAMS_MAX_SIZE_CONFERENCE_NAME || strlen(part_name) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
		return false;
	unsigned long mess_size = sizeof(RelayMessFixedPart) + strlen(conference_name) + 1 + strlen(part_name) + 1;
	m_mess->reserve(mess_size);
	m_mess->resize(sizeof(RelayMessFixedPart));
	RelayMessFixedPart *header = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	header->mess_type = e_part_disconenct;
	header->body_len = mess_size - sizeof(RelayMessFixedPart);
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(conference_name),&(reinterpret_cast<const unsigned char*>(conference_name))[strlen(conference_name)+1]);
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(part_name),&(reinterpret_cast<const unsigned char*>(part_name))[strlen(part_name)+1]);
	m_mess_size = m_mess->size();
	m_isComplete = true;
	return true;
}
bool VS_ControlRelayMessage::MakeTransmitFrame(const char *conference_name, const char *part_name, const stream::FrameHeader *frame_head, const void *frame_data)
{
	Clear();
	if(strlen(conference_name)>VS_STREAMS_MAX_SIZE_CONFERENCE_NAME || strlen(part_name) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME ||
		!frame_head || frame_head->length>VS_STREAM_MAX_SIZE_FRAME)
		return false;
	unsigned long mess_size = sizeof(RelayMessFixedPart) + strlen(conference_name) + 1 + strlen(part_name) + 1 + sizeof(stream::FrameHeader) + frame_head->length;
	m_mess->reserve(mess_size);
	m_mess->resize(sizeof(RelayMessFixedPart));
	RelayMessFixedPart *header = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	header->mess_type = e_transmit_frame;
	header->body_len = mess_size - sizeof(RelayMessFixedPart);
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(conference_name),&(reinterpret_cast<const unsigned char*>(conference_name))[strlen(conference_name)+1]);
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(part_name),&(reinterpret_cast<const unsigned char*>(part_name))[strlen(part_name)+1]);
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(frame_head),&(reinterpret_cast<const unsigned char*>(frame_head))[sizeof(stream::FrameHeader)]);
	if(frame_head->length>0)
		m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(frame_data),&(reinterpret_cast<const unsigned char*>(frame_data))[frame_head->length]);
	m_mess_size = m_mess->size();
	m_isComplete = true;
	return true;
}
bool VS_ControlRelayMessage::MakeRestrictBitrateSVC(const char* conference_name, const char* part_name, int32_t v_bitrate, int32_t bitrate, int32_t old_bitrate)
{
	Clear();
	if(strlen(conference_name)>VS_STREAMS_MAX_SIZE_CONFERENCE_NAME || strlen(part_name) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
		return false;
	unsigned long mess_size = sizeof(RelayMessFixedPart) + strlen(conference_name) + 1 + strlen(part_name) + 1 + 12;
	m_mess->reserve(mess_size);
	m_mess->resize(sizeof(RelayMessFixedPart));
	RelayMessFixedPart *header = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	header->mess_type = e_restrict_btr_svc;
	header->body_len = mess_size - sizeof(RelayMessFixedPart);
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(conference_name),&(reinterpret_cast<const unsigned char*>(conference_name))[strlen(conference_name)+1]);
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(part_name),&(reinterpret_cast<const unsigned char*>(part_name))[strlen(part_name)+1]);
	m_mess->insert(m_mess->end(), reinterpret_cast<const unsigned char*>(&v_bitrate), reinterpret_cast<const unsigned char*>(&v_bitrate) + sizeof(int32_t));
	m_mess->insert(m_mess->end(), reinterpret_cast<const unsigned char*>(&bitrate), reinterpret_cast<const unsigned char*>(&bitrate) + sizeof(int32_t));
	m_mess->insert(m_mess->end(), reinterpret_cast<const unsigned char*>(&old_bitrate), reinterpret_cast<const unsigned char*>(&old_bitrate) + sizeof(int32_t));
	m_mess_size = m_mess->size();
	m_isComplete = true;
	return true;
}
bool VS_ControlRelayMessage::MakeRequestKeyFrame(const char *conferenceName, const char *participantName)
{
	if(!conferenceName || !participantName)
		return false;
	Clear();
	string_view conf_name = conferenceName;
	string_view part_name = participantName;
	if(conf_name.length() > VS_STREAMS_MAX_SIZE_CONFERENCE_NAME || part_name.length() > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
		return false;
	auto mess_sz = sizeof(RelayMessFixedPart) + conf_name.length() +1 + part_name.length() +1;
	m_mess->reserve(mess_sz);
	m_mess->resize(sizeof(RelayMessFixedPart));
	RelayMessFixedPart *header = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	header->mess_type = e_request_key_frame;
	header->body_len = mess_sz - sizeof(RelayMessFixedPart);
	m_mess->insert(m_mess->end(), conf_name.begin(), conf_name.end());
	m_mess->push_back('\0');
	m_mess->insert(m_mess->end(), part_name.begin(), part_name.end());
	m_mess->push_back('\0');
	m_mess_size = m_mess->size();
	m_isComplete = true;
	return true;
}
bool VS_ControlRelayMessage::MakeSetPartCaps(const char* conference_name, const char* participant_name, const void* caps_buf, size_t buf_sz)
{
	Clear();
	if(strlen(conference_name)>VS_STREAMS_MAX_SIZE_CONFERENCE_NAME || strlen(participant_name) > VS_STREAMS_MAX_SIZE_PARTICIPANT_NAME)
		return false;

	unsigned long mess_size = sizeof(RelayMessFixedPart) + strlen(conference_name) + 1 + strlen(participant_name) + 1 + 4 /*sizeof(buf_sz)*/ + buf_sz;
	m_mess->reserve(mess_size);
	m_mess->resize(sizeof(RelayMessFixedPart));
	RelayMessFixedPart *header = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	header->mess_type = e_set_caps;
	header->body_len = mess_size - sizeof(RelayMessFixedPart);
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(conference_name),&(reinterpret_cast<const unsigned char*>(conference_name))[strlen(conference_name)+1]);
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(participant_name),&(reinterpret_cast<const unsigned char*>(participant_name))[strlen(participant_name)+1]);
	uint32_t buf_sz_val = buf_sz;
	m_mess->insert(m_mess->end(), reinterpret_cast<const unsigned char*>(&buf_sz_val), reinterpret_cast<const unsigned char*>(&buf_sz_val) + sizeof(uint32_t));
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(caps_buf),&(reinterpret_cast<const unsigned char*>(caps_buf))[buf_sz]);

	m_mess_size = m_mess->size();
	m_isComplete = true;
	return true;
}
bool VS_ControlRelayMessage::MakeLive555Info(net::port port, string_view secret)
{
	Clear();

	const size_t mess_size = sizeof(RelayMessFixedPart) + sizeof(uint16_t) + secret.size() + 1;
	m_mess->resize(mess_size);
	auto p = m_mess->data();

	const auto header = reinterpret_cast<RelayMessFixedPart*>(p);
	header->mess_type = e_live555_info;
	header->body_len = mess_size - sizeof(RelayMessFixedPart);
	p += sizeof(RelayMessFixedPart);

	*reinterpret_cast<uint16_t*>(p) = port;
	p += sizeof(uint16_t);

	memcpy(p, secret.data(), secret.size());
	p += secret.size();
	*p++ = '\0';

	m_mess_size = m_mess->size();
	m_isComplete = true;
	return true;
}

const char *VS_ControlRelayMessage::GetConferenceName() const
{
	unsigned long pos = GetConfNameIndex();
	if(pos==-1)
		return 0;
	return reinterpret_cast<const char*>(&(*m_mess)[0]+pos);
}

bool VS_ControlRelayMessage::GetConferenceTypes(VS_Conference_Type &type, VS_GroupConf_SubType &subtype) const
{
	unsigned long pos = GetConfTypesIndex();
	if(-1 == pos)
		return false;
	type = (VS_Conference_Type)*reinterpret_cast<const int32_t *>(&(*m_mess)[0] + pos);
	subtype = (VS_GroupConf_SubType)*reinterpret_cast<const int32_t *>(&(*m_mess)[0] + pos + 4);
	return true;
}

const char *VS_ControlRelayMessage::GetOwnerName() const
{
	unsigned long pos = GetOwnerNameIndex();
	if(pos==-1)
		return 0;
	return reinterpret_cast<const char*>(&(*m_mess)[0]+pos);
}

const char *VS_ControlRelayMessage::GetParticipantName() const
{
	unsigned long pos(0);
	pos = GetPartNameIndex();
	if(pos==-1)
		return 0;
	return reinterpret_cast<const char *>(&(*m_mess)[0] + pos);

}
bool VS_ControlRelayMessage::GetSVCBitrate(int32_t& v_btr, int32_t& btr, int32_t& o_btr) const
{
	unsigned long pos = GetSVCBitrateIndex();
	if(pos==-1)
		return 0;
	if(m_mess->size()!=pos+12)
		return false;
	v_btr = *reinterpret_cast<const int32_t*>(m_mess->data() + pos);
	btr = *reinterpret_cast<const int32_t*>(m_mess->data() + pos + 4);
	o_btr = *reinterpret_cast<const int32_t*>(m_mess->data() + pos + 8);
	return true;
}
const void* VS_ControlRelayMessage::GetCaps(size_t& caps_buf_ln) const
{
	unsigned long pos = GetCapsLnIndex();
	if(pos == -1)
		return 0;
	caps_buf_ln = *reinterpret_cast<const uint32_t*>(m_mess->data() + pos);
	if(!caps_buf_ln)
		return 0;
	if(-1 == (pos = GetCapsBufIndex()))
		return 0;
	return reinterpret_cast<const void*>(&(*m_mess)[0] + pos);
}
const stream::FrameHeader *VS_ControlRelayMessage::GetFrameHead() const
{
	unsigned long pos = GetFrameHeadIndex();
	if(pos==-1)
		return 0;
	return reinterpret_cast<const stream::FrameHeader *>(&(*m_mess)[0] + pos);
}
const void *VS_ControlRelayMessage::GetFrameData() const
{
	unsigned long pos = GetFrameDataIndex();
	if(pos==-1)
		return 0;
	return reinterpret_cast<void*>(&(*m_mess)[0] + pos);
}
net::port VS_ControlRelayMessage::GetLive555Port() const
{
	const auto pos = GetLive555PortIndex();
	if (pos==-1)
		return 0;
	return *reinterpret_cast<const uint16_t*>(m_mess->data() + pos);
}
const char* VS_ControlRelayMessage::GetLive555Secret() const
{
	const auto pos = GetLive555SecretIndex();
	if (pos==-1)
		return 0;
	return reinterpret_cast<const char*>(m_mess->data() + pos);
}

unsigned long VS_ControlRelayMessage::GetConfNameIndex() const
{
	if(!IsComplete())
		return -1;
	return sizeof(RelayMessFixedPart);
}
unsigned long VS_ControlRelayMessage::GetPartNameIndex() const
{
	unsigned long pos(0);
	switch(GetMessageType())
	{
	case e_transmit_frame:
	case e_part_connect:
	case e_part_disconenct:
	case e_set_caps:
	case e_restrict_btr_svc:
	case e_request_key_frame:
		pos = GetConfNameIndex();
		if(m_mess->size()>pos)
			return pos + strlen(reinterpret_cast<const char*>(&(*m_mess)[0]+pos)) + 1;
	}
	return -1;
}
unsigned long VS_ControlRelayMessage::GetSVCBitrateIndex() const
{
	if(GetMessageType() != e_restrict_btr_svc)
		return -1;
	unsigned long pos = GetPartNameIndex();
	if(m_mess->size()>pos)
		return pos + strlen(reinterpret_cast<const char*>(&(*m_mess)[0]+pos)) + 1;
	return -1;
}
unsigned long VS_ControlRelayMessage::GetConfTypesIndex() const
{
	if(GetMessageType() != e_start_conference)
		return -1;
	unsigned long pos = GetConfNameIndex();
	if(m_mess->size()<=pos)
		return -1;
	if(m_mess->size() >= strlen(reinterpret_cast<const char*>(&(*m_mess)[0]+pos)) + 9 + sizeof(RelayMessFixedPart))
		return pos + strlen(reinterpret_cast<const char*>(&(*m_mess)[0]+pos)) + 1;
	else
		return -1;
}
unsigned long VS_ControlRelayMessage::GetOwnerNameIndex() const
{
	if(GetMessageType() != e_start_conference)
		return -1;
	unsigned long pos = GetConfTypesIndex() + 2 * sizeof(int);
	if(m_mess->size()<=pos)
		return -1;
	if(m_mess->size() >= strlen(reinterpret_cast<const char*>(&(*m_mess)[0]+pos)) + 1 + pos)
		return pos;
	else
		return -1;
}

unsigned long VS_ControlRelayMessage::GetCapsLnIndex() const
{
	if(GetMessageType() != e_set_caps)
		return -1;
	unsigned long pos = GetPartNameIndex();
	if(m_mess->size()<=pos)
		return -1;
	return pos + strlen(reinterpret_cast<const char*>(&(*m_mess)[0]+pos)) + 1;
}

unsigned long VS_ControlRelayMessage::GetCapsBufIndex() const
{
	unsigned long pos = GetCapsLnIndex();
	if(pos == -1)
		return pos;
	return pos + 4;
}

unsigned long VS_ControlRelayMessage::GetFrameHeadIndex() const
{
	if(GetMessageType()!=e_transmit_frame)
		return -1;
	unsigned long pos = GetPartNameIndex();
	if(m_mess->size()<=pos)
		return -1;
	return pos + strlen(reinterpret_cast<const char*>(&(*m_mess)[0]+pos)) + 1;
}
unsigned long VS_ControlRelayMessage::GetFrameDataIndex() const
{
	const stream::FrameHeader *h = GetFrameHead();
	if(!h || !h->length)
		return -1;
	return GetFrameHeadIndex() + sizeof(stream::FrameHeader);
}
unsigned long VS_ControlRelayMessage::GetLive555PortIndex() const
{
	if (GetMessageType() != e_live555_info)
		return -1;
	return sizeof(RelayMessFixedPart);
}
unsigned long VS_ControlRelayMessage::GetLive555SecretIndex() const
{
	unsigned long pos = GetLive555PortIndex();
	if(pos == -1)
		return pos;
	return pos + sizeof(uint16_t);
}