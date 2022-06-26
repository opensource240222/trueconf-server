#include "VS_ConferenceManager.h"

VS_ConferenceManager::VS_ConferenceManager()
{
	clearCacheConference();
}

VS_ConferenceManager::~VS_ConferenceManager()
{
}

void VS_ConferenceManager::setCacheConference(const char *peerName, const char *peerDn, const char *confName, const char *confPass, conference::type type)
{
	lastPeerName = peerName ? peerName : "";
	lastPeerDn = peerDn ? peerDn : "";
	lastConfName = confName ? confName : "";
	lastConfPass = confPass ? confPass : "";
	lastConfType = type;
}

void VS_ConferenceManager::clearCacheConference()
{
	lastConfType = conference::invalid;
	lastPeerName.clear();
	lastPeerDn.clear();
	lastConfName.clear();
	lastConfPass.clear();
}

void VS_ConferenceManager::parseConfCreated(VS_Container &cnt)
{
	long result(0);
	cnt.GetValue(RESULT_PARAM, result);
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	OnConferenceCreated(name, result);
}

void VS_ConferenceManager::parseConfDeleted(VS_Container &cnt)
{
	long result(0);
	cnt.GetValue(RESULT_PARAM, result);
	const char *name = cnt.GetStrValueRef(NAME_PARAM);
	OnConferenceDeleted(name, result);
}

void VS_ConferenceManager::parseInvite(VS_Container &cnt)
{
	const char *user = cnt.GetStrValueRef(NAME_PARAM);
	const char *dn   = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	const char *confName = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *confPass = cnt.GetStrValueRef(PASSWORD_PARAM);

	setCacheConference(user, dn ? dn : user, confName, confPass, conference::call);
	OnInvite(user, dn ? dn : user, confName, confPass, conference::call);
}

void VS_ConferenceManager::parseInviteToMulti(VS_Container &cnt)
{
	const char *peerName     = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *peerDn       = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	const char *confName = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *confPass = cnt.GetStrValueRef(PASSWORD_PARAM);

	long _type = CT_MULTISTREAM;
	cnt.GetValue(TYPE_PARAM, _type);
	conference::type type = (CT_MULTISTREAM == _type) ? conference::group : conference::multicast;
	setCacheConference(peerName, peerDn ? peerDn : peerName, confName, confPass, type);
	OnInvite(peerName, peerDn ? peerDn : peerName, confName, confPass, type);
}

void VS_ConferenceManager::parseAccept(VS_Container &cnt)
{
	const char *confName = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *peerName = cnt.GetStrValueRef(NAME_PARAM);
	const char *peerDn   = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	OnAccept(peerName, peerDn ? peerDn : peerName, confName);
}

void VS_ConferenceManager::parseReject(VS_Container &cnt)
{
	const char *confName = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *peerName = cnt.GetStrValueRef(NAME_PARAM);
	if (!peerName) peerName = cnt.GetStrValueRef(CALLID_PARAM); // TODO: bugfix it in server
	const char *peerDn   = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	long cause(0); cnt.GetValue(CAUSE_PARAM, cause);
	OnReject(peerName, peerDn ? peerDn : peerName, confName, cause);
}

void VS_ConferenceManager::parseJoin(VS_Container &cnt)
{
	const char *confName = cnt.GetStrValueRef(CONFERENCE_PARAM);
	const char *confFriendlyName = cnt.GetStrValueRef(NAME_PARAM);
	OnJoin(confName, confFriendlyName);
}

void VS_ConferenceManager::parseRegistrationInfo(VS_Container &cnt)
{
	const char *peerName = cnt.GetStrValueRef(CALLID_PARAM);
	const char *peerUserName = cnt.GetStrValueRef(USERNAME_PARAM);
	const char *peerDn = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	OnRegistrationInfo(peerName, peerUserName, peerDn);
}

void VS_ConferenceManager::parseReceiverConnected(VS_Container &cnt)
{
	long result(0); cnt.GetValue(RESULT_PARAM, result);
	const char *peerName = cnt.GetStrValueRef(USERNAME_PARAM);
	long media(0);
	cnt.GetValue(MEDIAFILTR_PARAM,media);
	bool audioOnly=false;
	if (media == 0x900)
		audioOnly=true;
	//if (media == 0xF00) then it has video
	OnReceiverConnected(peerName, result, audioOnly);
}

void VS_ConferenceManager::parseSenderConnected(VS_Container &cnt)
{
	long result(0); cnt.GetValue(RESULT_PARAM, result);
	OnSenderConnected(result);
}

void VS_ConferenceManager::parseReqInvite(VS_Container &cnt)
{
	const char *user = cnt.GetStrValueRef(CALLID_PARAM);
	const char *dn   = cnt.GetStrValueRef(DISPLAYNAME_PARAM);
	setCacheConference(user, dn ? dn : user, 0, 0, conference::group);
	OnReqInvite(user, dn ? dn : user, 0, 0, conference::group);
}

void VS_ConferenceManager::parseRoleEvent(VS_Container &cnt)
{
}

void VS_ConferenceManager::parsePartList(VS_Container &cnt)
{
    OnPeersListUpdate(cnt);
}

bool VS_ConferenceManager::ParseIncomimgMessage(VS_Container &cnt)
{
	if (!cnt.IsValid()) return false;

	VS_SimpleStr Method = cnt.GetStrValueRef(METHOD_PARAM);
	DTRACE(VSTM_PRTCL, "conf7 method    = %20s", Method);
	unsigned long dwRet(ERR_OK);
	if (Method==CONFERENCECREATED_METHOD)
		parseConfCreated(cnt);
	else if (Method == CONFERENCEDELETED_METHOD)
		parseConfDeleted(cnt);
	else if (Method == USERREGISTRATIONINFO_METHOD)
		parseRegistrationInfo(cnt);
	//else if (Method == CONFIGURATIONUPDATED_METHOD)
	//	dwRet = Method_ConfigurationUpdated(cnt);
	else if (Method == INVITE_METHOD)
		parseInvite(cnt);
	else if (Method == INVITETOMULTI_METHOD)
		parseInviteToMulti(cnt);
	//else if (Method == INVITEREPLY_METHOD)
	//	dwRet = Method_InviteReply(cnt);
	else if (Method == ACCEPT_METHOD)
		parseAccept(cnt);
	else if (Method == REJECT_METHOD)
		parseReject(cnt);
	else if (Method == JOIN_METHOD)
		parseJoin(cnt);
	else if (Method == RECEIVERCONNECTED_METHOD)
		parseReceiverConnected(cnt);
	else if (Method == SENDERCONNECTED_METHOD)
		parseSenderConnected(cnt);
	else if (Method == REQINVITE_METHOD)
		parseReqInvite(cnt);
	else if (Method == ROLEEVENT_METHOD)
		parseRoleEvent(cnt);
    else if (Method == SENDPARTSLIST_METHOD)
        parsePartList(cnt);
		/*
	else if (Method == LISTENERSFLTR_METHOD)
		dwRet = Method_ListenersFltr(cnt);
	else if (Method == SENDCOMMAND_METHOD)
		dwRet = Method_SendCommand(cnt);
	else if (Method == DEVICESTATUS_METHOD)
		dwRet = Method_DeviceStatus(cnt);*/
	//make this later after tests
    else
        dwRet = VSTRCL_ERR_METHOD;
	return (dwRet == ERR_OK);
}