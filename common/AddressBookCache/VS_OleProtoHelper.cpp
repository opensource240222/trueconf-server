#include "VS_OleProtoHelper.h"
#include "../ClientInterface/ClientInterface.h"
#include "../ClientInterface/VSClient.h"
#include "../VSClient/VSThinkClient.h"
#include "../net/EndpointRegistry.h"

#include "VS_SafeArray.h"
#include "VS_AviWriter.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include "../std/cpplib/VS_RcvFunc.h"

VS_OleProtoHelper::VS_OleProtoHelper(VSClient *parent)
	: m_client(parent)
{
	InitializeCriticalSectionAndSpinCount(&m_lockPeerIdMap, 2048);
    InitAudioSystem();
}

VS_OleProtoHelper::~VS_OleProtoHelper()
{
	DeleteCriticalSection(&m_lockPeerIdMap);
}

int VS_OleProtoHelper::AcceptCall()
{
	return m_client->AcceptProtocolConnect();
}

void VS_OleProtoHelper::RejectCall()
{
	m_client->RejectProtocolConnect();
}

void VS_OleProtoHelper::RejectCall(char *confName, char * peerName, long cause)
{
	m_client->GetProtocol()->Reject(confName, peerName, cause);
}

bool VS_OleProtoHelper::Call(const char *peerName, const char *password, conference::type type, int maxPeers, bool nonpublic)
{
	if (!m_client || !peerName || !*peerName) return false;
	VS_SafeArray data(5);
	data.push_back(type == conference::call ? MCT_PRIVATE_CALL : MCT_REQINVITE);
	data.push_back(peerName);
	data.push_back(password);
	data.push_back(maxPeers);
	data.push_back(nonpublic ? 1 : 0);
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Protocol\\Call", &data.collection()));
}

bool VS_OleProtoHelper::Invite(const char *peerName)
{
	if (!m_client || !peerName || !*peerName) return false;
	_variant_t var = peerName;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Protocol\\Invite", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::InviteReply(int code)
{
	if (!m_client) return false;
	_variant_t var = code;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Protocol\\InviteReply", &var.GetVARIANT()));
}

void VS_OleProtoHelper::Hangup(bool force)
{
	m_client->HangupProtocolConnect(force ? 1 : 0);
}

bool VS_OleProtoHelper::ConnectSender(HWND notify, int type, int mediaFilter)
{
	if (!m_client) return false;
	m_client->ConnectSender(notify, type);
	return true;
	/* role conference
	_variant_t var = mediaFilter;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Protocol\\ConnectSender", &var.GetVARIANT()));
	*/
}

bool VS_OleProtoHelper::DisconnectSender()
{
	if (!m_client) return false;
	m_client->DisconnectSender();
	/*_variant_t var = 0;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Protocol\\ConnectSender", &var.GetVARIANT()));*/
	return true;
}

bool VS_OleProtoHelper::SetSelfBroadcast(int enable)
{
	if (!m_client) return false;
	_variant_t var = enable;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Protocol\\ConnectSender", &var.GetVARIANT()));
}

const char *VS_OleProtoHelper::ConnectReceiver(HWND canvas, HWND notify, unsigned long UUID)
{
	static char peerName[MAX_PATH+1]; peerName[0] = 0;
	if (!m_client /*|| (UUID == 0xffffffff)*/) return peerName;
	VS_SafeArray data(3);
	data.push_back(reinterpret_cast<int>(canvas));
	data.push_back(reinterpret_cast<int>(notify));
	data.push_back(static_cast<int>(UUID));
	if (VS_INTERFACE_OK != m_client->Process(RUN_COMMAND, "Receivers\\ConnectReceiver", &data.collection())) return peerName;
	data.reset();
	_bstr_t _otherName = data.get(2);
	strncpy(peerName, _otherName.operator const char *(), MAX_PATH);
	return peerName;
}

bool VS_OleProtoHelper::DisconnectReceiver(const char *peerName)
{
	if (!m_client || !peerName || !*peerName) return false;
	_variant_t var = peerName;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Receivers\\DisconnectReceiver", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::InitCameraConnect(HWND canvas)
{
	if (!m_client) return false;
	VS_SafeArray data(2);
	data.push_back("VideoCaptureSlot");
	data.push_back(reinterpret_cast<int>(canvas));
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Sender\\AddVideoCapture", &data.collection()));
}

bool VS_OleProtoHelper::InitMicrophoneConnect()
{
	if (!m_client) return false;
	_variant_t var = "AudioCaptureSlot";
	return VS_INTERFACE_OK == m_client->Process(RUN_COMMAND,"Sender\\AddAudioCapture", &var.GetVARIANT());
}

bool VS_OleProtoHelper::SetMicrophoneSensitivity(unsigned long sensitivity)
{
	if (!m_client) return false;
	_variant_t var = (sensitivity * 65535 /*0xffff*/) / 100;
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, "Sender\\AudioCaptureSlot\\AudioCapture\\MicrophoneLevel", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::SetTestPlaybackSensitivity(unsigned long sensitivity)
{
	if (!m_client) return false;
	_variant_t var = (sensitivity * 65535 /*0xffff*/) / 100;
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, "Receivers\\AudioPlayback\\TestPlayback"/*"Sender\\AudioCaptureSlot\\AudioCapture\\TestPlayback"*/, &var.GetVARIANT()));
}

bool VS_OleProtoHelper::FreeCameraConnect()
{
	if (!m_client) return false;
	_variant_t var = "VideoCaptureSlot";
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Sender\\RemoveVideoCapture", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::FreeMicrophoneConnect()
{
	if (!m_client) return false;
	_variant_t var = "AudioCaptureSlot";
	return VS_INTERFACE_OK == m_client->Process(RUN_COMMAND,"Sender\\RemoveAudioCapture", &var.GetVARIANT());
}

bool VS_OleProtoHelper::StartCameraCapture(const wchar_t *cameraName, oleStrArray &pins, bool & modes)
{
	if (!m_client) return false;
	_variant_t var = cameraName ? (*cameraName ? cameraName : L"none") : L"none";
	m_client->Process(RUN_COMMAND, "Sender\\VideoCaptureSlot\\Connect", &var.GetVARIANT());
    bool result;
    if (wcscmp(cameraName,L"none") != 0)
        result = (var.lVal == 0);
    else
        result = false;

	if (result) {
		GetCameraPinsAndModes(pins, modes);
	}
	return result;
}

bool VS_OleProtoHelper::GetCameraPinsAndModes(oleStrArray &pins, bool & modes)
{
	bool res = true;
	_variant_t var = false;
	pins.clear();
	_variant_t vdata;

	m_client->Process(GET_PARAM, "Sender\\VideoCaptureSlot\\CameraPropertyPage", &var.GetVARIANT());
	int curPin = m_client->Process(GET_PARAM, "Sender\\VideoCaptureSlot\\CameraPins", &var.GetVARIANT());
	if (VS_INTERFACE_OK == (curPin & 0xFFFF) )
	{
		VS_SafeArray data(&var.GetVARIANT());
		while (VT_EMPTY != (vdata = data.pop_front()).vt)
		{
			unsigned int tp = vdata.vt;
			pins.push_back(vdata.operator _bstr_t().operator const char *());
		}
	}
	else
		res = false;

	var = 0;
	modes = false;
	if (VS_INTERFACE_OK == m_client->Process(GET_PARAM, "Sender\\VideoCaptureSlot\\CameraMode", &var.GetVARIANT()))
	{
		modes = var;
	}
	else
		res = false;

	return res;
}

bool VS_OleProtoHelper::StopCameraCapture()
{
	if (!m_client) return false;
	_variant_t var;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Sender\\VideoCaptureSlot\\Disconnect", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::SetCameraPin(int pin)
{
	_variant_t var;
	var = pin;
	return m_client->Process(RUN_COMMAND, "Sender\\VideoCaptureSlot\\\CameraPins", &var.GetVARIANT()) == VS_INTERFACE_OK;
}

bool VS_OleProtoHelper::SetCameraMode(int mode)
{
	_variant_t var;
	var = mode;
	m_client->Process(RUN_COMMAND, "Sender\\VideoCaptureSlot\\\CameraMode", &var.GetVARIANT());
	return true;
}

bool VS_OleProtoHelper::StartAudioCapture(const wchar_t *microphoneName)
{
	if (!m_client) return false;
	_variant_t var = microphoneName ? (*microphoneName ? microphoneName : L"none") : L"none";
	m_client->Process(RUN_COMMAND, "Sender\\AudioCaptureSlot\\AudioCapture\\Connect", &var.GetVARIANT());

    bool result;
    if (wcscmp(microphoneName,L"none") != 0)
        result = (var.lVal == 0);
    else
        result = false;
	return result;
}

bool VS_OleProtoHelper::StopAudioCapture()
{
	if (!m_client) return false;
	_variant_t var;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Sender\\AudioCaptureSlot\\AudioCapture\\Disconnect", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::StartMicrophoneTest(const std::string &speakerName, const std::string & micName)
{
	if (!m_client) return false;

	int AudioCaptureIndex = 0;
	int AudioRIndex = 0;

	oleStrArray speakers;
	_variant_t vr, vdata;
	m_client->Process(RUN_COMMAND, "Receivers\\AudioPlayback\\AudioRenderList", &vr.GetVARIANT());
	if (VS_INTERFACE_OK == m_client->Process(GET_PARAM, "Receivers\\AudioPlayback\\AudioRenderList", &vr.GetVARIANT())) {
		VS_SafeArray data(&vr.GetVARIANT());
		while (VT_EMPTY != (vdata = data.pop_front()).vt) speakers.push_back(vdata.operator _bstr_t().operator const char *());
	}


	for (int i=0; i!=speakers.size(); ++i)
	{
		if (speakers[i] == speakerName)
		{
			AudioRIndex = i;
		}
	}

	oleStrArray mics;
	//_variant_t vr, vdata;
	m_client->Process(RUN_COMMAND, "Sender\\AudioCaptureDevices\\AudioCaptureList", &vr.GetVARIANT());
	if (VS_INTERFACE_OK == m_client->Process(GET_PARAM, "Sender\\AudioCaptureDevices\\AudioCaptureList", &vr.GetVARIANT())) {
		VS_SafeArray data(&vr.GetVARIANT());
		while (VT_EMPTY != (vdata = data.pop_front()).vt) mics.push_back(vdata.operator _bstr_t().operator const char *());
	}

	for (int i=0; i!=mics.size(); ++i)
	{
		if (mics[i] == micName)
		{
			AudioCaptureIndex = i;
		}
	}

	if (AudioCaptureIndex == -1)
	{
		return false;
	}

	_variant_t var;
	_variant_t vars[3];
	vars[0] = 11025;
	vars[1] = AudioRIndex;
	vars[2] = AudioCaptureIndex;
	m_client->CombineVars(&var.GetVARIANT(), vars, 3);


	//_variant_t var = speakersName ? (*speakersName ? speakersName : L"none") : L"none";
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Receivers\\AudioPlayback\\TestPlayback", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::StartAudioRenderTest(const std::string &speakerName)
{

	if (!m_client) return false;


	int AudioCaptureIndex = -1;
	int AudioRIndex = 0;//index;

	oleStrArray speakers;
	_variant_t vr, vdata;
	m_client->Process(RUN_COMMAND, "Receivers\\AudioPlayback\\AudioRenderList", &vr.GetVARIANT());
	if (VS_INTERFACE_OK == m_client->Process(GET_PARAM, "Receivers\\AudioPlayback\\AudioRenderList", &vr.GetVARIANT())) {
		VS_SafeArray data(&vr.GetVARIANT());
		while (VT_EMPTY != (vdata = data.pop_front()).vt) speakers.push_back(vdata.operator _bstr_t().operator const char *());
	}


	for (int i=0; i!=speakers.size(); ++i)
	{
		if (speakers[i] == speakerName)
		{
			AudioRIndex = i;
		}
	}

	_variant_t var;
	_variant_t vars[3];
	vars[0] = 11025;
	vars[1] = AudioRIndex;
	vars[2] = AudioCaptureIndex;
	m_client->CombineVars(&var.GetVARIANT(), vars, 3);


	//_variant_t var = speakersName ? (*speakersName ? speakersName : L"none") : L"none";
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Receivers\\AudioPlayback\\TestPlayback", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::StopAudioRenderTest()
{
	if (!m_client) return false;
	_variant_t var = false;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Receivers\\AudioPlayback\\TestPlayback", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::StartAudioPlayer(const wchar_t *speakersName)
{
	if (!m_client) return false;
	_variant_t var = speakersName ? (*speakersName ? speakersName : L"none") : L"none";
	m_client->Process(SET_PARAM, "Receivers\\PlaybackDevice", &var.GetVARIANT());
    bool result;
    if (wcscmp(speakersName,L"none") != 0)
        result = (var.lVal >= 0);
    else
        result = false;
	return result;
}

bool VS_OleProtoHelper::StopAudioPlayer()
{
	if (!m_client) return false;
	_variant_t var = L"none";
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, "Receivers\\PlaybackDevice", &var.GetVARIANT()));
}

const char *VS_OleProtoHelper::GetPeerByEventId(unsigned long eventId)
{
	static char iPeerName[MAX_PATH+1]; iPeerName[0] = 0;
	if (!m_client) return iPeerName;
	_variant_t var = eventId;
	if (VS_INTERFACE_OK == m_client->Process(GET_PARAM, "Protocol\\ActionUser", &var.GetVARIANT()))
	{
		strncpy(iPeerName, var.operator _bstr_t().operator const char *(), MAX_PATH);
		if (*iPeerName) {
			EnterCriticalSection(&m_lockPeerIdMap);
			m_peerIdMap[iPeerName] = eventId;
			LeaveCriticalSection(&m_lockPeerIdMap);
		}
	}
	return iPeerName;
}

unsigned long VS_OleProtoHelper::GetEventIdByPeer(const char *peer)
{
	if (!m_client || !peer || !*peer) return 0;

	EnterCriticalSection(&m_lockPeerIdMap);
	std::map<std::string, unsigned long>::iterator it = m_peerIdMap.find(peer);
	unsigned long ret(0);
	if (m_peerIdMap.end() != it) {
		ret = (*it).second;
		m_peerIdMap.erase(it);
	}
	LeaveCriticalSection(&m_lockPeerIdMap);
	return ret;
}

void VS_OleProtoHelper::RemoveEventIdByPeer(const char *peer)
{
	EnterCriticalSection(&m_lockPeerIdMap);
	std::map<std::string, unsigned long>::iterator it = m_peerIdMap.find(peer);
	if (m_peerIdMap.end() != it) { m_peerIdMap.erase(it); }
	LeaveCriticalSection(&m_lockPeerIdMap);
}

const oleStrArray& VS_OleProtoHelper::GetCameras()
{
	m_cameras.clear();
	_variant_t var, vdata;
	//to retreive updated list
	m_client->Process(RUN_COMMAND, "Sender\\VideoCaptureDevices\\VideoCaptureList", &var.GetVARIANT());
	if (VS_INTERFACE_OK == m_client->Process(GET_PARAM, "Sender\\VideoCaptureDevices\\VideoCaptureList", &var.GetVARIANT())) {
		VS_SafeArray data(&var.GetVARIANT());
		while (VT_EMPTY != (vdata = data.pop_front()).vt) m_cameras.push_back(vdata.operator _bstr_t().operator const char *());
	}
	m_cameras.push_back("none");
	return m_cameras;
}

const oleStrArray& VS_OleProtoHelper::GetMicrophones()
{
	m_microphones.clear();
	_variant_t var, vdata;
	var = "AudioCaptureSlot";
	if (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND,"Sender\\AddAudioCapture", &var.GetVARIANT())) {
		var.Clear();
		m_client->Process(RUN_COMMAND, "Sender\\AudioCaptureDevices\\AudioCaptureList", &var.GetVARIANT());
		if (VS_INTERFACE_OK == m_client->Process(GET_PARAM, "Sender\\AudioCaptureDevices\\AudioCaptureList", &var.GetVARIANT())) {
			VS_SafeArray data(&var.GetVARIANT());
			while (VT_EMPTY != (vdata = data.pop_front()).vt) m_microphones.push_back(vdata.operator _bstr_t().operator const char *());
		}
	}
	m_microphones.push_back("none");
	return m_microphones;
}

const oleStrArray& VS_OleProtoHelper::GetSpeakers()
{
	m_speakers.clear();
	_variant_t var, vdata;
	m_client->Process(RUN_COMMAND, "Receivers\\AudioPlayback\\AudioRenderList", &var.GetVARIANT());
	if (VS_INTERFACE_OK == m_client->Process(GET_PARAM, "Receivers\\AudioPlayback\\AudioRenderList", &var.GetVARIANT()))
	{
		VS_SafeArray data(&var.GetVARIANT());
		while (VT_EMPTY != (vdata = data.pop_front()).vt) m_speakers.push_back(vdata.operator _bstr_t().operator const char *());
	}
	m_speakers.push_back("none");
	return m_speakers;
}

bool VS_OleProtoHelper::CreateConference(const char *confName, const char *password, conference::type type, int maxPeers, bool nonpublic)
{
	if (!m_client) return false;
	VS_SafeArray data(5);
	int confType = MCT_PRIVATE_CALL;
	switch (type)
	{
	case conference::group:
		confType = MCT_CREATEMULTI;
		break;
	case conference::multicast :
		confType = MCT_ONE2MANY;
		break;
	case conference::role :
		confType = MCT_ROLE;
		break;
	}
	data.push_back(confType/*type == conference::call ? MCT_PRIVATE_CALL : MCT_CREATEMULTI*/);
	data.push_back(confName);
	data.push_back(password);
	data.push_back(maxPeers);
	data.push_back(nonpublic ? 1 : 0);
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Protocol\\Call", &data.collection()));
}

bool VS_OleProtoHelper::AviCreate(const char *callId, const char *filename)
{
	if (!m_client) return false;
	VS_SafeArray data(5);
	data.push_back(filename);
	data.push_back(callId);
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, "Receivers\\AviWriter", &data.collection()));
}

bool VS_OleProtoHelper::AviResume()
{
	if (!m_client) return false;
	_variant_t var = 1;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Receivers\\AviWriter", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::AviPause()
{
	if (!m_client) return false;
	_variant_t var = 2;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Receivers\\AviWriter", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::AviClose()
{
	if (!m_client) return false;
	_variant_t var = 0;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Receivers\\AviWriter", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::SetEndpoint(const char * address, const unsigned short port)
{
	if(!address || !*address) return false;
    char * server_name = new char [MAX_PATH];
    _snprintf(server_name, MAX_PATH, "%s%s", address, "#vcs");

	net::endpoint::ClearAllConnectTCP(server_name, true);

	net::endpoint::AddConnectTCP({ server_name, port, "TCP" }, server_name, true);
    delete [] server_name;
    return true;
}

VS_AviWriter *VS_OleProtoHelper::RecordPeer(const char *peerName, const char *filename)
{
	VS_AviWriter *_avi = new VS_AviWriter(this);
	if (_avi->Create(peerName, filename)) return _avi;
	delete _avi;
	return 0;
}

bool VS_OleProtoHelper::Login(char *login, char *password, bool autologin, bool encrypt)
{
	if (!m_client || !login ||!*login || !password || !*password) return false;
	return (ERR_OK == m_client->LoginToServer(login, password, autologin ? 1 : 0, encrypt ? 1 : 0));
}

bool VS_OleProtoHelper::Logout(bool clearAutoLogin)
{
	if (!m_client) return false;
	m_client->GetProtocol()->LogoutUser(clearAutoLogin);
	return true;
}

bool VS_OleProtoHelper::ChatSend(char *peerName, char *message)
{
	if (!m_client) return false;
	m_client->GetProtocol()->ChatSend(message, peerName);
	return true;
}

bool VS_OleProtoHelper::SetSendAudioVideo(bool audio, bool video)
{
	if (!m_client) return false;
	_variant_t var;
	_variant_t vars[2];
	vars[0] = audio;
	vars[1] = video;
	m_client->CombineVars(&var.GetVARIANT(), vars, 2);
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, "Sender\\EnabledDevices", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::GetSendAudioVideo(bool & audio, bool & video)
{
	if (!m_client) return false;
    audio = m_client->GetThinkClient()->m_DeviceStatus.bUseAudio;
    video = m_client->GetThinkClient()->m_DeviceStatus.bUseVideo;
    return true;
}

bool VS_OleProtoHelper::SetPeerVolume(const char *peerName, unsigned long volume)
{
	if ((!m_client) || !peerName || !*peerName) return false;
	_variant_t var = (volume * 0xffff) / 100;
	char str[MAX_PATH] = {};
	sprintf(str, "Receivers\\%s>\\RenderAudio\\Volume", peerName);
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, str, &var.GetVARIANT()));
}

bool VS_OleProtoHelper::SetPeerAV(const char *peerName, bool enabledAudio, bool enabledVideo)
{
	if ((!m_client) || !peerName || !*peerName) return false;
	long df = VS_RcvFunc::FLTR_ALL_MEDIA;
	if (!enabledAudio) df ^= VS_RcvFunc::FLTR_RCV_AUDIO;
	if (!enabledVideo) df ^= VS_RcvFunc::FLTR_RCV_VIDEO;
	_variant_t var = df;
	char str[MAX_PATH] = {};
	sprintf(str, "Receivers\\%s>\\SetRsvFltr", peerName);
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, str, &var.GetVARIANT()));
}

bool VS_OleProtoHelper::SetEchoCancellation(bool enable)
{
	if (!m_client) return false;
	_variant_t var = enable ? 2 : 0;
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Sender\\AudioCaptureSlot\\AudioCapture\\UseXPAec", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::SetEnableAGC(bool enable)
{
	if (!m_client) return false;
	_variant_t var = enable ? 1 : 0;
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, "Sender\\AudioCaptureSlot\\AudioCapture\\EnableAGC'", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::Kick(const char *peerName)
{
	return ERR_OK == m_client->GetProtocol()->Kick(peerName);
}

const char *VS_OleProtoHelper::GetProperty(const char *name)
{
	static char prop[MAX_PATH+1];
	prop[0] = 0;
	if (!name || !*name) return prop;
	if (ERR_OK != m_client->GetProperty(name, prop))
		prop[0] = 0;
	return prop;
}

VS_Container* VS_OleProtoHelper::GetContainerByEventId(unsigned long eventId)
{
	return m_client->GetProtocol()->m_RoleEventContainers.GetList(eventId);
}

bool VS_OleProtoHelper::UMCCommand(const role::type type, char *peerName, const role::status role, const role::result result)
{
	if (!m_client) return false;
	const bool rIncorrect = (result == role::incorrect);
	VS_SafeArray data(rIncorrect ? 3 : 4);
    VARIANT v;
    VariantInit(&v);
    v.pcVal = peerName;
    data.push_back(type);
	data.push_back(&v);
    data.push_back(role);
	if (!rIncorrect) data.push_back(result);
	return (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Protocol\\RoleEvents", &data.collection()));
}

bool VS_OleProtoHelper::ConnectToServer(const char * address, const unsigned short port)
{
    //SetEndpoint(address, port);
	return (0 == m_client->GetProtocol()->ConnectServer(address));
}

bool VS_OleProtoHelper::SetInputBandwidth(const unsigned long bandwidth)
{
	if (!m_client) return false;
	_variant_t var = bandwidth;
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, "Protocol\\InputBandwidth", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::SetOutputBandwidth(const unsigned long bandwidth)
{
	if (!m_client) return false;
	_variant_t var = bandwidth;
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, "Sender\\Bandwidth", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::SetSystemVolume(unsigned long volume)
{
	if (!m_client) return false;
	_variant_t var = (volume * 65535/*0xffff*/) / 100;
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, "Receivers\\AudioPlayback\\SystemVolume", &var.GetVARIANT()));
}

bool VS_OleProtoHelper::SetSystemMute(bool mute)
{
	if (!m_client) return false;
	_variant_t var = mute? 1: 0;
	return (VS_INTERFACE_OK == m_client->Process(SET_PARAM, "Receivers\\AudioPlayback\\SystemMute", &var.GetVARIANT()));
}

long VS_OleProtoHelper::GetConferenceType()
{
    return m_client->GetProtocol()->m_Status.CurrConfInfo->ServerSubType;
}

bool VS_OleProtoHelper::isMultiConference()
{
    return m_client->GetProtocol()->m_Status.CurrConfInfo->ServerScope ? 1 : 0;
}

bool VS_OleProtoHelper::SendCommand(char * command, char * to)
{
    if (!m_client) return false;
	m_client->SendCommand(command, to);
    return true;
}

bool VS_OleProtoHelper::InitAudioSystem()
{
    if (!m_client) return false;
	_variant_t var1 = 0;
    bool result(VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Receivers\\AudioPlayback\\PrepareAudio", &var1.GetVARIANT()));
    result &= (VS_INTERFACE_OK == m_client->Process(RUN_COMMAND, "Receivers\\AudioPlayback\\SystemVolume", &var1.GetVARIANT()));
    return true;
}

bool VS_OleProtoHelper::AcceptRecordRequest(char * peerName) {
    return SendCommand("plugin:Record:Accept", peerName);
}

bool VS_OleProtoHelper::RejectRecordRequest(char * peerName) {
    return SendCommand("plugin:Record:Reject", peerName);
}

long VS_OleProtoHelper::GetRights()
{
	return m_client->GetProtocol()->m_Status.MyInfo.Rights;
}

long VS_OleProtoHelper::GetTarifRestrictions()
{
	return m_client->GetProtocol()->m_Status.MyInfo.TarifRestrictions;
}

VS_WideStr VS_OleProtoHelper::GetMyDisplayName()
{
	return m_client->GetProtocol()->m_Status.MyInfo.DisplayName;
}

void VS_OleProtoHelper::GetOtherName(char *UserName, char *FirstName, char *LastName)
{
	m_client->GetOtherName(UserName, FirstName, LastName);
}

VS_SimpleStr VS_OleProtoHelper::GetTarifPlan()
{
	return m_client->GetProtocol()->m_Status.MyInfo.TarifPlan;
}

void VS_OleProtoHelper::SetDiscoveryService(char * service)
{
	m_client->GetProtocol()->SetDiscoveryServise(service);
}

void VS_OleProtoHelper::SetCameraFlip( bool flip)
{
	if (!m_client) return;
	_variant_t var = flip ? 1: 0;
	m_client->Process(SET_PARAM, "Sender\\VideoCaptureSlot\\Render\\FlipFrame", &var.GetVARIANT());
}
