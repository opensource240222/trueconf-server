/**
 **************************************************************************
 * \file VS_ClientCaps.cpp
 * (c) 2002-2006 Visicron Inc.  http://www.visicron.net/
 * \brief Capabilities for client Implementation
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \date 04.05.06
 *
 * $Revision: 6 $
 *
 * $History: VS_ClientCaps.cpp $
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/std/cpplib
 * - were added auto stereo mode detect
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 9.04.11    Time: 23:59
 * Updated in $/VSNA/std/cpplib
 * - Set mediaformat depend on type of conference
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 31.03.11   Time: 13:18
 * Updated in $/VSNA/std/cpplib
 * - fps in mediaformat
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 18.10.10   Time: 19:47
 * Updated in $/VSNA/std/cpplib
 * - HQ detect alfa
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 25.03.09   Time: 15:03
 * Updated in $/VSNA/std/cpplib
 * 5.5 PVC enhancments:
 * - added "adaptive data decode" capability
 * - new bitrate control for data
 *
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 6.05.06    Time: 16:34
 * Created in $/VS/std/cpplib
 * -"'desired" receiver medaiformat capability
 *
 ****************************************************************************/


/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_ClientCaps.h"

#include <cstring>

/****************************************************************************
 * Static
 ****************************************************************************/
static const char VSCC_INTCAPS[]		= "IntCaps";	 // long
static const char VSCC_STREAMS_DIRCON[]	= "StrmDirC";	 // long
static const char VSCC_VIDEO_RCV[]		= "VideoRcv";	 // long
static const char VSCC_VIDEO_SND[]		= "VideoSnd";	 // long
static const char VSCC_AUDIO_RCV[]		= "AudioRcv";	 // long
static const char VSCC_AUDIO_SND[]		= "AudioSnd";	 // long
static const char VSCC_RATING[]			= "Rating";		 // long
static const char VSCC_LEVEL[]			= "Level";		 // long
static const char VSCC_LEVEL_GROUP[]	= "LevelGroup";	 // long
static const char VSCC_SCREEN_WIDTH[]	= "ScreenWidth"; // long
static const char VSCC_SCREEN_HEIGHT[]	= "ScreenHeight";// long
static const char VSCC_CLIENTTYPE[]		= "ClientType";	 // long
static const char VSCC_MEDIAFORMAT[]	= "MediaFmt";	 // void*
static const char VSCC_MEDIAFORMATRCV[]	= "MdFmtRcv";	 // void*
static const char VSCC_BANDW_RCV[]		= "BandWRcv";	 // long
static const char VSCC_AUDIO_CODECS[]	= "AudioCdc";	 // void*
static const char VSCC_VIDEO_CODECS[]	= "VideoCdc";	 // void*
static const char VSCC_CLIENT_FLAGS[]	= "ClientFlags"; // enum ClientFlags

/****************************************************************************
 * Defines
 ****************************************************************************/
#define VSCC_BANDW_DEFAULT		128 //-1;

enum VSCC_InternalCaps {
	INTCAPS_NONE  = 0,
	INTCAPS_MFRCV = 1
};

VS_ClientCaps::VS_ClientCaps()
{
	ReSet();
	m_int_caps = INTCAPS_MFRCV;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ClientCaps::ReSet()
{
	m_int_caps = INTCAPS_NONE;
	m_streams_dc = VSCC_STREAM_NONE;
	m_bandw_rcv = VSCC_BANDW_DEFAULT;
	m_video_rcv = VSCC_VIDEO_DEFAULT;
	m_video_snd = VSCC_VIDEO_DEFAULT;
	m_audio_rcv = VSCC_AUDIO_DEFAULT;
	m_audio_snd = VSCC_AUDIO_DEFAULT;
	m_rating = 0;
	m_level = 0;
	m_level_group = 0;
	m_screen_width = 0;
	m_screen_height = 0;
	m_client_type = 0;
	m_client_flags = ClientFlags::NONE;
	m_mediaformat.Empty();
	m_mediaformat_rcv.Empty();
	m_audio_codecs.Empty();
	m_video_codecs.Empty();
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ClientCaps::Set(const void* buff, size_t size)
{
	ReSet();
	if (!buff || !size) return;
	VS_Container cnt;
	cnt.Deserialize(buff, size);
	cnt.GetValueI32(VSCC_INTCAPS, m_int_caps);
	cnt.GetValueI32(VSCC_STREAMS_DIRCON, m_streams_dc);
	cnt.GetValueI32(VSCC_VIDEO_RCV, m_video_rcv);
	cnt.GetValueI32(VSCC_VIDEO_SND, m_video_snd);
	cnt.GetValueI32(VSCC_AUDIO_RCV, m_audio_rcv);
	cnt.GetValueI32(VSCC_AUDIO_SND, m_audio_snd);
	cnt.GetValueI32(VSCC_RATING, m_rating);
	cnt.GetValueI32(VSCC_LEVEL, m_level);
	cnt.GetValueI32(VSCC_LEVEL_GROUP, m_level_group);
	cnt.GetValueI32(VSCC_SCREEN_WIDTH, m_screen_width);
	cnt.GetValueI32(VSCC_SCREEN_HEIGHT, m_screen_height);
	cnt.GetValueI32(VSCC_CLIENTTYPE, m_client_type);
	cnt.GetValueI32(VSCC_CLIENT_FLAGS, m_client_flags);
	const void* Buff;
	size_t Size;
	Buff = cnt.GetBinValueRef(VSCC_MEDIAFORMAT, Size);
	m_mediaformat.Set(Buff, Size);
	Buff = cnt.GetBinValueRef(VSCC_MEDIAFORMATRCV, Size);
	m_mediaformat_rcv.Set(Buff, Size);
	cnt.GetValueI32(VSCC_BANDW_RCV, m_bandw_rcv);
	Buff = cnt.GetBinValueRef(VSCC_AUDIO_CODECS, Size);
	m_audio_codecs.Set(Buff, Size);
	Buff = cnt.GetBinValueRef(VSCC_VIDEO_CODECS, Size);
	m_video_codecs.Set(Buff, Size);
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_ClientCaps::Get(void* &buff, size_t &size) const
{
	VS_Container cnt;
	cnt.AddValue(VSCC_INTCAPS, static_cast<int32_t>(m_int_caps));
	cnt.AddValue(VSCC_STREAMS_DIRCON, static_cast<int32_t>(m_streams_dc));
	cnt.AddValue(VSCC_VIDEO_RCV, static_cast<int32_t>(m_video_rcv));
	cnt.AddValue(VSCC_VIDEO_SND, static_cast<int32_t>(m_video_snd));
	cnt.AddValue(VSCC_AUDIO_RCV, static_cast<int32_t>(m_audio_rcv));
	cnt.AddValue(VSCC_AUDIO_SND, static_cast<int32_t>(m_audio_snd));
	cnt.AddValue(VSCC_RATING, static_cast<int32_t>(m_rating));
	cnt.AddValue(VSCC_LEVEL, static_cast<int32_t>(m_level));
	cnt.AddValue(VSCC_LEVEL_GROUP, static_cast<int32_t>(m_level_group));
	cnt.AddValue(VSCC_SCREEN_WIDTH, static_cast<int32_t>(m_screen_width));
	cnt.AddValue(VSCC_SCREEN_HEIGHT, static_cast<int32_t>(m_screen_height));
	cnt.AddValue(VSCC_CLIENTTYPE, static_cast<int32_t>(m_client_type));
	cnt.AddValueI32(VSCC_CLIENT_FLAGS, m_client_flags);
	cnt.AddValue(VSCC_MEDIAFORMAT, m_mediaformat.Buffer(), m_mediaformat.Size());
	cnt.AddValue(VSCC_MEDIAFORMATRCV, m_mediaformat_rcv.Buffer(), m_mediaformat_rcv.Size());
	cnt.AddValue(VSCC_BANDW_RCV, static_cast<int32_t>(m_bandw_rcv));
	cnt.AddValue(VSCC_AUDIO_CODECS, m_audio_codecs.Buffer(), m_audio_codecs.Size());
	cnt.AddValue(VSCC_VIDEO_CODECS, m_video_codecs.Buffer(), m_video_codecs.Size());
	return cnt.SerializeAlloc(buff, size);
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ClientCaps::GetMediaFormat(VS_MediaFormat &fmt) const
{
	if (m_mediaformat.IsValid())
		fmt = *static_cast<const VS_MediaFormat*>(m_mediaformat.Buffer());
	else
		fmt.ReSet();
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_ClientCaps::IsFixedRcvMediaFormat(const VS_ClientCaps &from) const
{
	return ((m_int_caps & INTCAPS_MFRCV) && from.m_mediaformat_rcv.IsValid());
}

/**
 **************************************************************************
 ****************************************************************************/
VS_MediaFormat VS_ClientCaps::GetFmtFromToMe(const VS_ClientCaps &from) const
{
	VS_MediaFormat myFormat, fromFormat, fmt, rcvfmt;
	from.GetMediaFormat(fromFormat);
	GetMediaFormat(myFormat);
	bool brcvmf = from.m_int_caps&INTCAPS_MFRCV && m_mediaformat_rcv.IsValid();
	if (brcvmf)
		rcvfmt = *static_cast<const VS_MediaFormat*>(m_mediaformat_rcv.Buffer());
	// audio
	fmt.SetAudio(0, 0, 0);		// set not valid
	if (fromFormat.IsAudioValid() && from.GetAudioSnd()!=VSCC_AUDIO_NONE && GetAudioRcv()!=VSCC_AUDIO_NONE) {
		// "from" format is valid, "from" at list VSCC_AUDIO_DEFAULT, "i" at list VSCC_AUDIO_DEFAULT
		// freq
		uint32_t freq = 0;
		// codec
		uint32_t codec = 0;
		if (brcvmf) {
			if (from.GetAudioSnd()&VSCC_AUDIO_ANYFREQ)
				freq = rcvfmt.dwAudioSampleRate;
			else if (rcvfmt.dwAudioSampleRate == fromFormat.dwAudioSampleRate)
				freq = rcvfmt.dwAudioSampleRate;

			if (from.GetAudioSnd()&VSCC_AUDIO_ANYFREQ && from.FindAudioCodec(rcvfmt.dwAudioCodecTag))
				codec = rcvfmt.dwAudioCodecTag;
			else if (rcvfmt.dwAudioCodecTag == fromFormat.dwAudioCodecTag)
				codec = rcvfmt.dwAudioCodecTag;
		}
		else {
			if (GetAudioRcv()&VSCC_AUDIO_ANYFREQ)
				freq = fromFormat.dwAudioSampleRate;
			else {
				if (from.GetAudioSnd()&VSCC_AUDIO_ANYFREQ)
					freq = myFormat.dwAudioSampleRate;
				else if (myFormat.dwAudioSampleRate == fromFormat.dwAudioSampleRate)
					freq = myFormat.dwAudioSampleRate;
			}

			if (GetAudioRcv()&VSCC_AUDIO_ANYFREQ) {
				if (FindAudioCodec(fromFormat.dwAudioCodecTag))
					codec = fromFormat.dwAudioCodecTag;
				else if (from.GetAudioSnd()&VSCC_AUDIO_ANYFREQ) {
					if (from.FindAudioCodec(myFormat.dwAudioCodecTag))
						codec = myFormat.dwAudioCodecTag;
					else
						codec = from.FindInAudioCodecs(m_audio_codecs);
				}
			}
			else {
				if (from.GetAudioSnd() & VSCC_AUDIO_ANYFREQ && from.FindAudioCodec(myFormat.dwAudioCodecTag))
					codec = myFormat.dwAudioCodecTag;
				else if (myFormat.dwAudioCodecTag == fromFormat.dwAudioCodecTag)
					codec = myFormat.dwAudioCodecTag;
			}
		}
		fmt.SetAudio(freq, codec);
		// bufflen
		if (brcvmf && from.GetAudioSnd()&VSCC_AUDIO_ANYBLEN)
			fmt.dwAudioBufferLen = rcvfmt.dwAudioBufferLen;
		else if (GetAudioRcv()&VSCC_AUDIO_ANYBLEN)
			fmt.dwAudioBufferLen = fromFormat.dwAudioBufferLen;
		else if (from.GetAudioSnd()&VSCC_AUDIO_ANYBLEN)
			; // do nothing, save default value
		else if (fmt.dwAudioBufferLen == fromFormat.dwAudioBufferLen)
			; // do nothing, save default value
		else
			fmt.dwAudioBufferLen = 0; // set not valid
	}

	// video
	fmt.SetVideo(0, 0, 0, 0, 0, 0, 0);		// set not valid
	if (fromFormat.IsVideoValid() && from.GetVideoSnd()!=VSCC_VIDEO_NONE && GetVideoRcv()!=VSCC_VIDEO_NONE) {
		// "from" format is valid, "from" at list VSCC_VIDEO_DEFAULT, "i" at list VSCC_VIDEO_DEFAULT
		// dimensions
		uint32_t width = 0;
		uint32_t height = 0;
		// codec
		uint32_t codec = 0;
		// fps
		uint32_t fps = 0;
		// stereo
		uint32_t stereo = 0;
		// hw enc
		uint32_t hwenc = 0;
		if (brcvmf) {
			if (from.GetVideoSnd()&VSCC_VIDEO_ANYSIZE)
			{
				width = rcvfmt.dwVideoWidht;
				height = rcvfmt.dwVideoHeight;
			}
			else if (rcvfmt.dwVideoWidht==fromFormat.dwVideoWidht && rcvfmt.dwVideoHeight==fromFormat.dwVideoHeight)
			{
				width = rcvfmt.dwVideoWidht;
				height = rcvfmt.dwVideoHeight;
			}

			if (from.GetVideoSnd()&VSCC_VIDEO_ANYCODEC && from.FindVideoCodec(rcvfmt.dwVideoCodecFCC))
				codec = rcvfmt.dwVideoCodecFCC;
			else if (rcvfmt.dwVideoCodecFCC == fromFormat.dwVideoCodecFCC)
				codec = rcvfmt.dwVideoCodecFCC;

			fps = rcvfmt.dwFps;
			stereo = rcvfmt.dwStereo;
		}
		else {
			if (GetVideoRcv()&VSCC_VIDEO_ANYSIZE)
			{
				width = fromFormat.dwVideoWidht;
				height = fromFormat.dwVideoHeight;
			}
			else {
				if (from.GetVideoSnd()&VSCC_VIDEO_ANYSIZE)
				{
					width = myFormat.dwVideoWidht;
					height = myFormat.dwVideoHeight;
				}
				else if (myFormat.dwVideoWidht==fromFormat.dwVideoWidht && myFormat.dwVideoHeight==fromFormat.dwVideoHeight)
				{
					width = myFormat.dwVideoWidht;
					height = myFormat.dwVideoHeight;
				}
			}

			if (GetVideoRcv()&VSCC_VIDEO_ANYCODEC) {
				if (FindVideoCodec(fromFormat.dwVideoCodecFCC))
					codec = fromFormat.dwVideoCodecFCC;
				else if (from.GetVideoSnd()&VSCC_VIDEO_ANYCODEC) {
					if (from.FindVideoCodec(myFormat.dwVideoCodecFCC))
						codec = myFormat.dwVideoCodecFCC;
					else
						codec = from.FindInVideoCodecs(m_video_codecs);
				}
			}
			else {
				if (from.GetVideoSnd()&VSCC_VIDEO_ANYCODEC && from.FindVideoCodec(myFormat.dwVideoCodecFCC))
					codec = myFormat.dwVideoCodecFCC;
				else if (myFormat.dwVideoCodecFCC == fromFormat.dwVideoCodecFCC)
					codec = myFormat.dwVideoCodecFCC;
			}

			fps = fromFormat.dwFps;
			if (GetVideoRcv()&VSCC_VIDEO_RCVSTEREO) stereo = fromFormat.dwStereo;
			if (stereo != fromFormat.dwStereo) codec = VS_VCODEC_VPX;
		}
		fmt.SetVideo(width, height, codec, fps, stereo, 0, hwenc);
	}
	return fmt;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ClientCaps::GetAudioCodecs(uint16_t* codecs, size_t& number) const
{
	if (codecs && number >= m_audio_codecs.Size() / sizeof(uint16_t)) {
		memcpy(codecs, m_audio_codecs.Buffer(), m_audio_codecs.Size());
	}
	number = m_audio_codecs.Size() / sizeof(uint16_t);
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_ClientCaps::FindAudioCodec(uint16_t codec) const
{
	auto p = static_cast<const uint16_t*>(m_audio_codecs.Buffer());
	for (unsigned int i = 0; i < m_audio_codecs.Size() / sizeof(uint16_t); i++) {
		if (p[i]==codec)
			return true;
	}
	return false;
}

/**
 **************************************************************************
 ****************************************************************************/
uint16_t VS_ClientCaps::FindInAudioCodecs(const VS_BinBuff &codecs) const
{
	auto my = static_cast<const uint16_t*>(m_audio_codecs.Buffer());
	auto other = static_cast<const uint16_t*>(codecs.Buffer());
	for (unsigned int i = 0; i < m_audio_codecs.Size() / sizeof(uint16_t); i++) {
		for (unsigned int j = 0; j < codecs.Size() / sizeof(uint16_t); j++) {
			if (my[i]==other[j])
				return other[j];
		}
	}
	return 0;
}

/**
 **************************************************************************
 ****************************************************************************/
void VS_ClientCaps::GetVideoCodecs(uint32_t* codecs, size_t& number) const
{
	if (codecs && number >= m_video_codecs.Size() / sizeof(uint32_t)) {
		memcpy(codecs, m_video_codecs.Buffer(), m_video_codecs.Size());
	}
	number = m_video_codecs.Size() / sizeof(uint32_t);
}

/**
 **************************************************************************
 ****************************************************************************/
bool VS_ClientCaps::FindVideoCodec(uint32_t codec) const
{
	auto p = static_cast<const uint32_t*>(m_video_codecs.Buffer());
	for (unsigned int i = 0; i < m_video_codecs.Size() / sizeof(uint32_t); i++) {
		if (p[i]==codec)
			return true;
	}
	return false;
}

/**
 **************************************************************************
 ****************************************************************************/
uint32_t VS_ClientCaps::FindInVideoCodecs(const VS_BinBuff &codecs) const
{
	auto my = static_cast<const uint32_t*>(m_video_codecs.Buffer());
	auto other = static_cast<const uint32_t*>(codecs.Buffer());
	for (unsigned int i = 0; i < m_video_codecs.Size() / sizeof(uint32_t); i++) {
		for (unsigned int j = 0; j < codecs.Size() / sizeof(uint32_t); j++) {
			if (my[i]==other[j])
				return other[j];
		}
	}
	return 0;
}

bool operator==(const VS_ClientCaps& l, const VS_ClientCaps& r)
{
	return l.m_int_caps == r.m_int_caps
	    && l.m_streams_dc == r.m_streams_dc
	    && l.m_bandw_rcv == r.m_bandw_rcv
	    && l.m_video_rcv == r.m_video_rcv
	    && l.m_video_snd == r.m_video_snd
	    && l.m_audio_rcv == r.m_audio_rcv
	    && l.m_audio_snd == r.m_audio_snd
	    && l.m_rating == r.m_rating
	    && l.m_level == r.m_level
	    && l.m_level_group == r.m_level_group
	    && l.m_screen_width == r.m_screen_width
	    && l.m_screen_height == r.m_screen_height
	    && l.m_client_type == r.m_client_type
	    && l.m_mediaformat == r.m_mediaformat
	    && l.m_mediaformat_rcv == r.m_mediaformat_rcv
	    && l.m_audio_codecs == r.m_audio_codecs
	    && l.m_video_codecs == r.m_video_codecs
	;
}
