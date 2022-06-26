#pragma once
#include <string>

enum VS_MediaPeer_Type
{
	vs_media_peer_type_webrtc,
	vs_media_peer_type_record,
	vs_media_peer_type_rtsp,
	vs_media_peer_type_mcu,
};

class VS_MediaFormat;

class VS_PeerPoolItemObserver
{
public:
	virtual void KeyFrameRequestForPeer(const char *part_id, const char *peer_id) = 0;
	virtual void ChangeAudioSendPayload(const char *part_id, const char *peer_id, const char *plname, int plfreq) = 0;
	virtual void ChangeVideoSendPayload(const char *part_id, const char *peer_id, VS_MediaPeer_Type peer_type, const VS_MediaFormat& mf) = 0;
	virtual void ChangeVideoSendPayload(const char *part_id, const char *peer_id, const VS_MediaFormat& mf, VS_MediaFormat &out, bool forceFormat) = 0;
	virtual void SetRates(const char *part_id, const char *peer_name, unsigned int bitrate, unsigned int frame_rate) = 0;
	virtual void SetSndFrameResolution(const char *part_id, const char *peer_name, const char *plname, uint8_t pltype, unsigned short width, unsigned short height) = 0;
	virtual void ReadyToDie(const char *part_id, const char *peer_id) = 0;
};

namespace ffl {
	class SourceMediaPeer;
}

class VS_MediaPeerInterface {
public:
	virtual void PutVideo(unsigned char *pFrame, unsigned long size, bool isKey, unsigned timestamp) = 0;
	virtual void PutCompressedAudio(unsigned char *buf, unsigned long sz, unsigned int samples, unsigned int timestamp) = 0;
	virtual void PutUncompressedAudio(unsigned char *buf, unsigned long sz, unsigned int timestamp) = 0;
	virtual void ReceiveAudio(const char *peer_name, const unsigned char *buf, int sz, unsigned int timestamp) = 0;
	virtual void ChangeAudioRcvPayload(const char *plname, const int plfreq) = 0;
	virtual void ReceiveVideo(const char *peer_name, const char *stream_id, const unsigned char *pFrame, int size, bool isKey, unsigned int timestamp) = 0;
	virtual void ChangeRcvFrameResolution(const char *peer_name, const char *stream_id, const char *plname, uint8_t pltype, unsigned short width, unsigned short height) = 0;

	virtual void SetPeerBitrate(int /*bitrate*/) {};
	virtual void RequestKeyFrameFromPeer() {};

	virtual ~VS_MediaPeerInterface() {}
};

class VS_MediaPeerBase : public VS_MediaPeerInterface
{
	friend class ffl::SourceMediaPeer;
protected:
	std::string m_peer_id;
	std::string m_part_id;
	VS_PeerPoolItemObserver *m_observer;
	bool m_isClosed;
	int64_t m_payload_key;
	unsigned int m_atimestamp;
	VS_MediaPeer_Type m_type;
public:
	VS_MediaPeerBase(std::string&& peer_id, std::string&& part_id)
		: m_peer_id(std::move(peer_id))
		, m_part_id(std::move(part_id))
		, m_observer(0)
		, m_isClosed(false)
		, m_payload_key(0)
		, m_atimestamp(0)
		, m_type(vs_media_peer_type_webrtc)
	{}
	VS_MediaPeerBase(const char *peer_id, const char *part_id)
		: VS_MediaPeerBase((!!peer_id) ? peer_id : std::string(), (!!part_id) ? part_id : std::string())
	{}


	void SetObserver(VS_PeerPoolItemObserver *observer)
	{
		m_observer = observer;
	}
	void Close()
	{
		m_isClosed = true;
	}

	const std::string& GetPeerId() const
	{
		return m_peer_id;
	}
	const std::string& GetPartId() const
	{
		return m_part_id;
	}
	void SetPayloadKey(unsigned int twocc, unsigned int plfreq)
	{
		m_payload_key = ((int64_t)(twocc) << 32) | ((int64_t)(plfreq));
	}
	int64_t GetPayloadKey()
	{
		return m_payload_key;
	}
	void UpdateAudioTimestamp(unsigned int dt)
	{
		m_atimestamp += dt;
	}
	unsigned int GetAudioTimestamp()
	{
		return m_atimestamp;
	}
	VS_MediaPeer_Type GetPeerType()
	{
		return m_type;
	}
};
