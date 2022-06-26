#include "std/cpplib/VS_Protocol.h"
#include "VS_SystemMessage.h"

void VS_SystemMessage::SendCommandMessage(const char* message, const char * user, const char* server)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SENDCOMMAND_METHOD);
	rCnt.AddValue(FROM_PARAM, "");
	rCnt.AddValue(MESSAGE_PARAM, message);
	rCnt.AddValue(TO_PARAM, "");
	PostRequest(server, user, rCnt, 0, CHAT_SRV, 30000);
}

void VS_SystemMessage::SendSystemMessage(const char* message, const char * user, const char* server)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SENDMESSAGE_METHOD);
	rCnt.AddValue(FROM_PARAM, "");
	rCnt.AddValue(DISPLAYNAME_PARAM, "SYSTEM MESSAGE");
	rCnt.AddValue(MESSAGE_PARAM, message);
	rCnt.AddValue(TO_PARAM, "");
	rCnt.AddValueI32(TYPE_PARAM, MSG_SYSTEM);
	PostRequest(server, user, rCnt, 0, CHAT_SRV, 30000);
}

#ifdef _WIN32
void VS_SystemMessage::SendSystemMessageByCid(const wchar_t* message, const char * cid)
{
	VS_Container rCnt;
	rCnt.AddValue(METHOD_PARAM, SENDMESSAGE_METHOD);
	rCnt.AddValue(FROM_PARAM, "");
	rCnt.AddValue(DISPLAYNAME_PARAM, "SYSTEM MESSAGE");
	rCnt.AddValue(MESSAGE_PARAM, message);
	rCnt.AddValue(TO_PARAM, "");
	rCnt.AddValueI32(TYPE_PARAM, MSG_SYSTEM);
	PostUnauth(cid, rCnt, 0, CHAT_SRV);
}
#endif

