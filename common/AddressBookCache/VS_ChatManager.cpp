#include "VS_ChatManager.h"

VS_ChatManager::VS_ChatManager()
{
}

VS_ChatManager::~VS_ChatManager()
{
}

bool VS_ChatManager::ParseIncomimgMessage(VS_Container &cnt)
{
	if (!cnt.IsValid()) return false;

	VS_SimpleStr Method = cnt.GetStrValueRef(METHOD_PARAM);
	DTRACE(VSTM_PRTCL, "chat method    = %20s", Method);
	unsigned long dwRet(ERR_OK);

    if (Method == SENDCOMMAND_METHOD) {
        parseSendCommand(cnt);
    } else {
        parseChat(cnt);
    }
	return (dwRet == ERR_OK);
}

void VS_ChatManager::parseSendCommand(VS_Container &cnt) {
	VS_WideStr mess;
	mess.AssignUTF8(cnt.GetStrValueRef(MESSAGE_PARAM));
	const char* from = cnt.GetStrValueRef(FROM_PARAM);
	const char* to = cnt.GetStrValueRef(TO_PARAM);
	OnCommand(from, to, mess);
}

void VS_ChatManager::parseChat(VS_Container &cnt) {
	VS_WideStr dn, mess;
	dn.AssignUTF8(cnt.GetStrValueRef(DISPLAYNAME_PARAM));
	mess.AssignUTF8(cnt.GetStrValueRef(MESSAGE_PARAM));
	const char* from = cnt.GetStrValueRef(FROM_PARAM);
	const char* to = cnt.GetStrValueRef(TO_PARAM);
	OnChat(from, dn, mess, to);
}
