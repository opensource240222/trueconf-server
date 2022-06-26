#ifndef VS_OLEPROTOHELPER_H
#define VS_OLEPROTOHELPER_H

//#include "../AddressBookCache/VZOchat7.h"

#include <windows.h>
class VSClient;

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : Warning : "

#include "VS_ConferenceManager.h"

#include <vector>
#include <string>
#include <map>

// role events namespace
namespace role {
	enum status {
		invalid,
		general,
		leader,
		speaker,
		reporter,
		remark
	};

	enum type {
		inquiry,
		answer,
		confirm,
		notify
	};

	enum result {
		incorrect = -1,
		ok,
		unknown,
		roleBusy,
		cancel,
		userBusy,
		timeout
	};
};

typedef std::vector<std::string> oleStrArray;
class VS_AviWriter;

// Delphi code compability
class VS_OleProtoHelper
{
private:
	VSClient *m_client;
	oleStrArray m_cameras;
	oleStrArray m_microphones;
	oleStrArray m_speakers;
	std::map<std::string, unsigned long> m_peerIdMap;
	CRITICAL_SECTION m_lockPeerIdMap;

	//* avi writer delegate
	bool AviCreate(const char *callId, const char *filename);
	bool AviResume();
	bool AviPause();
	bool AviClose();
    bool SetEndpoint(const char * address, const unsigned short port);

public:
	explicit VS_OleProtoHelper(VSClient *parent);
	virtual ~VS_OleProtoHelper();
	int AcceptCall();
	void RejectCall();
	void RejectCall(char *confName, char * peerName, long cause);
	void Hangup(bool force);
	bool Call(const char *peerName, const char *password = 0, conference::type type = conference::call, int maxPeers = 6, bool nonpublic = true);
	bool Invite(const char *peerName);
	bool InviteReply(int code = 0);
	bool Kick(const char *peerName);
	bool ConnectSender(HWND notify, int type = 0, int mediaFilter = 0);
	bool DisconnectSender();
	bool SetSelfBroadcast(int enable);
	const char *ConnectReceiver(HWND canvas, HWND notify, unsigned long UID = 0);
	bool DisconnectReceiver(const char *peerName);
	bool InitCameraConnect(HWND canvas);
	bool InitMicrophoneConnect();
	bool SetMicrophoneSensitivity(unsigned long sensitivity);
	bool SetTestPlaybackSensitivity(unsigned long sensitivity);
	bool FreeCameraConnect();
	bool FreeMicrophoneConnect();
	bool StartCameraCapture(const wchar_t *cameraName, oleStrArray &pins, bool & modes);
	bool StopCameraCapture();
	bool GetCameraPinsAndModes(oleStrArray &pins, bool & modes);
	bool SetCameraPin(int pin);
	bool SetCameraMode(int mode);
	void SetCameraFlip(bool);
	bool SetSendAudioVideo(bool audio, bool video);
	bool GetSendAudioVideo(bool & audio, bool & video);
	bool StartAudioCapture(const wchar_t *microphoneName);
	bool StopAudioCapture();
	bool StartMicrophoneTest(const std::string &speakerName, const std::string & micName);
	bool StartAudioRenderTest(const std::string &speakerName);
	bool StopAudioRenderTest();
	bool StartAudioPlayer(const wchar_t *speakersName);
	bool StopAudioPlayer();
	bool SetPeerVolume(const char *peerName, unsigned long volume);
	bool SetPeerAV(const char *peerName, bool enabledAudio, bool enabledVideo);
	const char *GetPeerByEventId(unsigned long eventId);
	unsigned long GetEventIdByPeer(const char *peer);
	void RemoveEventIdByPeer(const char *peer);
	bool CreateConference(const char *confName = 0, const char *password = 0, conference::type type = conference::group, int maxPeers = 6, bool nonpublic = true);
	VS_AviWriter *RecordPeer(const char *peerName, const char *filename);
	const oleStrArray& GetCameras();
	const oleStrArray& GetMicrophones();
	const oleStrArray& GetSpeakers();
	// quick support for login interface with autologin, etc
	bool Login(char *login, char *password, bool autologin, bool encrypt);
	bool Logout(bool clearAutoLogin = false);
	// quick support of group chat
	bool ChatSend(char *peerName, char *message);
	bool SetEchoCancellation(bool enable);
	bool SetEnableAGC(bool enable);
	const char *GetProperty(const char *name);
	// quick support for an assymetric conferences
	VS_Container* GetContainerByEventId(unsigned long eventId);
	bool UMCCommand(const role::type type, char *peerName, const role::status role, const role::result result = role::incorrect);
    bool ConnectToServer(const char * address, const unsigned short port);
    bool SetInputBandwidth(const unsigned long bandwidth);
    bool SetOutputBandwidth(const unsigned long bandwidth);
	bool SetSystemVolume(unsigned long volume);
	bool SetSystemMute(bool mute);
    long GetConferenceType();
	bool isMultiConference();
    bool InitAudioSystem();
    bool AcceptRecordRequest(char * peerName);
    bool RejectRecordRequest(char * peerName);
	//quick support for current user rights
	long GetRights();
	long GetTarifRestrictions();
	VS_SimpleStr GetTarifPlan();
	void SetDiscoveryService(char *);

	//current user display name
	VS_WideStr GetMyDisplayName();

	void GetOtherName(char *UserName, char *FirstName, char *LastName);

	bool SendCommand(char * command, char * to);

	friend class VS_AviWriter;
};

#endif