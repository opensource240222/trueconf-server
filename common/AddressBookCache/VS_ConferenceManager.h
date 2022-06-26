#ifndef VS_CONFERENCEMANAGER_H
#define VS_CONFERENCEMANAGER_H

#include "VS_AbstractManager.h"
#include <string>

namespace conference {
	enum type {
		call = 1,
		group = 2,
		multicast = 3,
		role = 4,
		joinGroup = 5,
		invalid // add any required type before this value
	};
}

class VS_ConferenceManager : public VS_AbstractManager
{
private:
	std::string lastPeerName;
	std::string lastPeerDn;
	std::string lastConfName;
	std::string lastConfPass;
	conference::type lastConfType;
	void setCacheConference(const char *peerName, const char *peerDn, const char *confName, const char *confPass, conference::type type);
	void clearCacheConference();

	void parseConfCreated(VS_Container &cnt);
	void parseConfDeleted(VS_Container &cnt);
	void parseInvite(VS_Container &cnt);
	void parseInviteToMulti(VS_Container &cnt);
	void parseAccept(VS_Container &cnt);
	void parseReject(VS_Container &cnt);
	void parseJoin(VS_Container &cnt);
	void parseRegistrationInfo(VS_Container &cnt);
	void parseReceiverConnected(VS_Container &cnt);
	void parseSenderConnected(VS_Container &cnt);
	void parseReqInvite(VS_Container &cnt);
    void parseRoleEvent(VS_Container &cnt);
    void parsePartList(VS_Container &cnt);

protected:
	// callback on conference create event
	virtual void OnConferenceCreated(const char *confName, long result) = 0;
	// callback on conference delete event
	virtual void OnConferenceDeleted(const char *confName, long result) = 0;
	// callback on invite from peer to local user
	virtual void OnInvite(const char *peerName, const char *peerDn, const char *confName, const char *confPass, conference::type type) = 0;
	// callback on accept from peer to local invite request
	virtual void OnReqInvite(const char *peerName, const char *peerDn, const char *confName, const char *confPass, conference::type type) = 0;
	// callback on accept from peer to local invite request
	virtual void OnAccept(const char *peerName, const char *peerDn, const char *confName) = 0;
	// callback on reject from peer to local invite request
	virtual void OnReject(const char *peerName, const char *peerDn, const char *confName, long cause) = 0;
	// callback on conference join
	virtual void OnJoin(const char *confName, const char *confFriendlyName) = 0;
	// callback incoming registration info
	virtual void OnRegistrationInfo(const char *peerName, const char *peerUserName, const char *peerDn) = 0;
	// callback receiver connected
	virtual void OnReceiverConnected(const char *peerName, long result, bool audioOnly) = 0;
	// callback sender connected
	virtual void OnSenderConnected(long result) = 0;
    // callback on peers arrived
	virtual void OnPeersListUpdate(VS_Container &cnt) = 0;

public:
	VS_ConferenceManager();
	virtual ~VS_ConferenceManager();
	virtual bool ParseIncomimgMessage(VS_Container &cnt);
};

#endif