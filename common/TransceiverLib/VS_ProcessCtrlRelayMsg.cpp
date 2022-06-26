#include "VS_ProcessCtrlRelayMsg.h"

const char VS_ProcessCtrlRelayMsg::module_name[] = "ProcessControlModule";

VS_ProcessCtrlRelayMsg::VS_ProcessCtrlRelayMsg() : VS_EnvelopeRelayMessage(module_name)
{
}

VS_ProcessCtrlRelayMsg::~VS_ProcessCtrlRelayMsg()
{
}

bool VS_ProcessCtrlRelayMsg::MakeStop()
{
	ClearContainer();
	if(!SetParam("ProcessCtrlMsgType",(int32_t)e_stop)) return false;
	return Make();
}

uint32_t VS_ProcessCtrlRelayMsg::GetProcCtrlMessageType()
{
	int32_t ProcessCtrlMsgType;
	if(GetParam("ProcessCtrlMsgType",ProcessCtrlMsgType))
		return (uint32_t)ProcessCtrlMsgType;
	return -1;
}
