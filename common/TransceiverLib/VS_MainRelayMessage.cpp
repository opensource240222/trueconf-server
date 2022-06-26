#include "VS_MainRelayMessage.h"
#include "../transport/VS_TransportDefinitions.h"

VS_MainRelayMessage::VS_MainRelayMessage(): m_state(e_start_st)
{}

unsigned char* VS_MainRelayMessage::GetBufToRead(unsigned long &buf_sz)
{
	/**
		state 0
			wait header  return to pos 0
		state 1
			wait body return next pos
	**/
	unsigned long pos(0);
	unsigned long sz(0);
	do
	{
		pos = 0;
		sz = 0;
		RelayMessFixedPart *head(0);
		switch(m_state)
		{
		case e_start_st:
			pos = 0;
			m_state = e_wait_header_st;
			sz = sizeof(RelayMessFixedPart);
			break;
		case e_wait_header_st:
			if(m_mess->size()<sizeof(RelayMessFixedPart))
			{
				pos = m_mess->size();
				sz = sizeof(RelayMessFixedPart) - pos;
			}
			else
			{
				m_state = e_wait_body_st;
				head = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
				if(!CheckHeader())
				{
					m_state = e_error_st;
					buf_sz = 0;
					return 0;
				}
				sz = head->body_len;
				pos = sizeof(RelayMessFixedPart);
			}
			break;
		case e_wait_body_st:
			head = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
			if(m_mess->size() < head->body_len + sizeof(RelayMessFixedPart))
			{
				//read more
				pos = m_mess->size();
				sz = head->body_len + sizeof(RelayMessFixedPart) - pos;
			}
			else
			{
				m_mess_size = 0;
				m_state = e_start_st;
			}
			break;
		default:
			buf_sz = 0;
			assert(false);
			return 0;
		};
	}while(e_start_st == m_state);
	m_mess->resize(sz+pos);
	buf_sz = sz;
	assert(sz != 0);
	return &(*m_mess)[0] + pos;
}
void VS_MainRelayMessage::SetReadBytes(const unsigned long received_bytes)
{
	VS_NetworkRelayMessageBase::SetReadBytes(received_bytes);
	/**
		Check for complete message
	*/
	if(m_mess->size()<sizeof(RelayMessFixedPart))
	{
		m_isComplete = false;
		return;
	}
	RelayMessFixedPart *h = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	if(CheckHeader() && h->body_len+sizeof(RelayMessFixedPart)==m_mess->size())
		m_isComplete = true;
	else
		m_isComplete = false;
}
void VS_MainRelayMessage::Clear()
{
	VS_NetworkRelayMessageBase::Clear();
	m_state = e_start_st;
}
bool VS_MainRelayMessage::CheckHeader()
{
	if(m_mess_size < sizeof(RelayMessFixedPart))
		return false;
	RelayMessFixedPart *head = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	switch(head->mess_type)
	{
	case e_transmit_frame:
	case e_start_conference:
	case e_stop_conference:
	case e_part_connect:
	case e_part_disconenct:
	case e_set_caps:
	case e_restrict_btr_svc:
	case e_request_key_frame:
	case e_envelope:
	case e_live555_info:
		if(head->body_len <= VS_TRANSPORT_MAX_SIZE_BODY)
			return true;
	};
	return false;
}

unsigned long VS_MainRelayMessage::GetMessageType() const
{
	if(!IsComplete())
		return e_is_not_complete;
	else if(m_state == e_error_st)
		return e_bad_message;
	return reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0])->mess_type;
}
unsigned long VS_MainRelayMessage::GetModuleNameIndex() const
{
	if(!IsComplete())
		return -1;
	if (GetMessageType()!=e_envelope)
		return -1;
	return sizeof(RelayMessFixedPart);
}

char *VS_MainRelayMessage::GetModuleName() const
{
	unsigned long pos = GetModuleNameIndex();
	if(pos==-1)
		return 0;
	return reinterpret_cast<char*>(&(*m_mess)[0]+pos);
}