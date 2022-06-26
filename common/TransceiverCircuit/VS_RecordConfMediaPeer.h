#pragma once
#include "VS_MediaPeerBase.h"
#include "../Transcoder/VSVideoFileWriter.h"
#include "../std/cpplib/VS_Lock.h"
#include "../std/cpplib/VS_MediaFormat.h"


class VS_RecordConfMediaPeer : public VS_MediaPeerBase
{
public:
	VS_RecordConfMediaPeer(const char *peer_id, const char *part_id, const char *file_name, VS_MediaFormat *mf);
	virtual ~VS_RecordConfMediaPeer();
	// call it after PeerConnect
	void ApplyMediaFormat();
	void Stop();

	virtual void PutVideo(unsigned char *pFrame, unsigned long size, bool isKey, unsigned int timestamp) override;
	virtual void PutCompressedAudio(unsigned char *buf, unsigned long sz, unsigned int samples, unsigned int timestamp) override;
	virtual void PutUncompressedAudio(unsigned char *buf, unsigned long sz, unsigned int timestamp) override {};
	virtual void ReceiveAudio(const char *peer_name, const unsigned char *buf, int sz, unsigned int timestamp) override {};
	virtual void ChangeAudioRcvPayload(const char *plname, const int plfreq) {};
	virtual void ReceiveVideo(const char *peer_name, const char *stream_id, const unsigned char *pFrame, int size, bool isKey, unsigned int timestamp) {};
	virtual void ChangeRcvFrameResolution(const char *peer_name, const char *stream_id, const char *plname, uint8_t pltype, unsigned short width, unsigned short height) override {};
	void Pause();
	void Resume();
	uint64_t GetFileSize() const;

private:
	VSVideoFileWriter	m_mkv;
	VS_MediaFormat	m_mf;
	uint64_t		m_fsize = 0;

	unsigned long	m_StartTimestamp = 0;

	unsigned int	m_ConfRecordTimeLimit; // in milliseconds
	bool			m_IsTimeLimitReached;

	bool			m_Paused = false;
	unsigned int	m_FirstPausedTimestamp = 0;
	unsigned int	m_LastPausedTimestamp = 0;
	unsigned int	m_TimeInPause = 0;
	bool			m_WaitKeyframe = false;

	unsigned long ToLocalTimestamp(unsigned long timestamp);
	void CheckTimeLimit();
	void PausedTimestamp(unsigned long timestamp);
};
