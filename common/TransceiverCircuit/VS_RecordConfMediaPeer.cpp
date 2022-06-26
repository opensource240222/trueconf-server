#include "VS_RecordConfMediaPeer.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/utf8.h"
#include <limits>
#include <string>

VS_RecordConfMediaPeer::VS_RecordConfMediaPeer(const char *peer_id, const char *part_id, const char *file_name, VS_MediaFormat *mf)
	: VS_MediaPeerBase(peer_id,part_id)
{
	m_type = vs_media_peer_type_record;

	VS_RegistryKey cfg(false, CONFIGURATION_KEY);

	m_ConfRecordTimeLimit = std::numeric_limits<unsigned long>::max() / 1000; // max time
	unsigned long val = 0;
	if (cfg.GetValue(&val, 4, VS_REG_INTEGER_VT, "ConfRecordTimeLimit") > 0) {
		if (val < m_ConfRecordTimeLimit)
			m_ConfRecordTimeLimit = val; // seconds
	}
	m_ConfRecordTimeLimit *= 1000; // ms
	m_IsTimeLimitReached = false;

	VSVideoFile::SAudioInfo audioInfo;
	VSVideoFile::SVideoInfo videoInfo;

	videoInfo.Width = mf->dwVideoWidht;
	videoInfo.Height = mf->dwVideoHeight;
	videoInfo.FPS = 30;
	videoInfo.PixFormat = VSVideoFile::PF_YUV420;
	videoInfo.CodecID = mf->dwVideoCodecFCC == VS_VCODEC_H264 ? VSVideoFile::VCODEC_ID_H264 : VSVideoFile::VCODEC_ID_VP8;

	audioInfo.BitsPerSample = 16;
	audioInfo.NumChannels = 1;
	audioInfo.SampleRate = mf->dwAudioSampleRate;
	if (mf->dwAudioCodecTag == VS_ACODEC_AAC) {
		audioInfo.CodecID = VSVideoFile::ACODEC_ID_AAC;
		audioInfo.BitsPerSample = 32; // because aac encoder use float as internal sample format
	}
	else {
		audioInfo.CodecID = VSVideoFile::ACODEC_ID_PCM;
	}
	m_mf = *mf;

	m_mkv.Init(file_name);
	m_mkv.SetAudioFormat(audioInfo);
	m_mkv.SetVideoFormat(videoInfo);
	m_mkv.WriteHeader();
}

void VS_RecordConfMediaPeer::ApplyMediaFormat()
{
	m_observer->ChangeVideoSendPayload(m_part_id.c_str(), m_peer_id.c_str(), m_type, m_mf);
	if (m_mf.dwAudioCodecTag == VS_ACODEC_AAC)
		m_observer->ChangeAudioSendPayload(m_part_id.c_str(), m_peer_id.c_str(), "AAC", m_mf.dwAudioSampleRate);
	else if (m_mf.dwAudioCodecTag == VS_ACODEC_PCM)
		m_observer->ChangeAudioSendPayload(m_part_id.c_str(), m_peer_id.c_str(), "L16", m_mf.dwAudioSampleRate);

	m_observer->KeyFrameRequestForPeer(m_part_id.c_str(), m_peer_id.c_str());
}

void VS_RecordConfMediaPeer::Stop()
{
	m_fsize = m_mkv.Release();
}

VS_RecordConfMediaPeer::~VS_RecordConfMediaPeer()
{
	m_mkv.Release();
}

void VS_RecordConfMediaPeer::PutVideo(unsigned char *pFrame, unsigned long size, bool isKey, unsigned int timestamp)
{
	if (m_IsTimeLimitReached)
		return;

	if (m_Paused)
	{
		PausedTimestamp(timestamp);
		return;
	}

	if (m_WaitKeyframe)
	{
		if (isKey)
			m_WaitKeyframe = false;
		else
			return;
	}

	m_mkv.WriteVideoTimeAbs((char*)pFrame, size, isKey, ToLocalTimestamp(timestamp));

	CheckTimeLimit();
}

void VS_RecordConfMediaPeer::PutCompressedAudio(unsigned char *buf, unsigned long sz, unsigned int samples, unsigned int timestamp)
{
	if (m_IsTimeLimitReached)
		return;

	if (m_Paused)
	{
		PausedTimestamp(timestamp);
		return;
	}

	m_mkv.WriteAudioTimeAbs((char*)(buf + sizeof(int32_t)), sz, ToLocalTimestamp(static_cast<uint32_t>(timestamp)));

	CheckTimeLimit();
}

unsigned long VS_RecordConfMediaPeer::ToLocalTimestamp(unsigned long timestamp)
{
	if (!m_StartTimestamp)
		m_StartTimestamp = timestamp;

	return timestamp - m_StartTimestamp - m_TimeInPause;
}

void VS_RecordConfMediaPeer::CheckTimeLimit()
{
	if (m_mkv.GetCurrentAudioTime() > m_ConfRecordTimeLimit || m_mkv.GetCurrentVideoTime() > m_ConfRecordTimeLimit)
	{
		m_IsTimeLimitReached = true;
		m_fsize = m_mkv.Release();
	}
}

void VS_RecordConfMediaPeer::PausedTimestamp(unsigned long timestamp)
{
	if (!m_FirstPausedTimestamp)
		m_FirstPausedTimestamp = timestamp;

	m_LastPausedTimestamp = timestamp;
}

void VS_RecordConfMediaPeer::Pause()
{
	m_Paused = true;
}

void VS_RecordConfMediaPeer::Resume()
{
	m_Paused = false;
	m_WaitKeyframe = true;
	m_TimeInPause += m_LastPausedTimestamp - m_FirstPausedTimestamp;
	m_FirstPausedTimestamp = 0;
	m_LastPausedTimestamp = 0;
	m_observer->KeyFrameRequestForPeer(m_part_id.c_str(), m_peer_id.c_str());
}

uint64_t VS_RecordConfMediaPeer::GetFileSize() const
{
	uint64_t fsize = m_mkv.GetSize();
	return fsize != 0 ? fsize : m_fsize;
}
