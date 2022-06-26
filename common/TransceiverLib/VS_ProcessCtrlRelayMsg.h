#pragma once
#include "VS_MainRelayMessage.h"
#include "VS_EnvelopeRelayMessage.h"

class VS_ProcessCtrlRelayMsg : public VS_EnvelopeRelayMessage
{
public:
	enum ProcessCtrlMsgType
	{
		e_stop = 1
	};

	VS_ProcessCtrlRelayMsg();
	virtual ~VS_ProcessCtrlRelayMsg();
	uint32_t GetProcCtrlMessageType();
	bool MakeStop();

	const static char module_name[];

};