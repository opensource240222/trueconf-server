#pragma once
#include "VS_NetworkRelayMessage.h"

namespace
{
#pragma pack(1)
	struct RelayMessFixedPart
	{
		unsigned short	mess_type;
		int32_t			body_len;
	};
#pragma pack ( )
	enum EControlMessageState
	{
		e_start_st,
		e_wait_header_st,
		e_wait_body_st,
		e_error_st
	};
}

/**
	TODO: maybe divide to ReadMessage and WriteMessage
**/
class VS_MainRelayMessage : public VS_NetworkRelayMessageBase
{
	unsigned	m_state;
	bool CheckHeader();
public:
	enum Type
	{
		e_bad_message,
		e_is_not_complete,

		e_transmit_frame,
		e_start_conference,
		e_stop_conference,
		e_part_connect,
		e_part_disconenct,
		e_set_caps,
		e_restrict_btr_svc,
		e_request_key_frame,
		e_envelope,
		e_live555_info,
	};

	char *GetModuleName() const;
	VS_MainRelayMessage();
	virtual ~VS_MainRelayMessage()
	{
	};
	unsigned char* GetBufToRead(unsigned long& buf_sz) override;
	void SetReadBytes(const unsigned long received_bytes) override;
	void Clear() override;
	unsigned long GetMessageType() const;
protected:
	unsigned long GetModuleNameIndex() const;
};