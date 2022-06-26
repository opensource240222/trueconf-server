/**
 **************************************************************************
 * \file VS_ClientCaps.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Capabilities for client
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \date 25.02.04
 *
 * $Revision: 8 $
 *
 * $History: VS_ClientCaps.h $
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 10.02.12   Time: 10:13
 * Updated in $/VSNA/std/cpplib
 * - update svc server implementation
 * - add svc server capability
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/std/cpplib
 * - were added auto stereo mode detect
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 31.05.11   Time: 17:11
 * Updated in $/VSNA/std/cpplib
 * - VS_ClientCaps::ClearMediaFomatRcv() added
 * - CVSTrClientProc::PrepareCaps(): call ClearMediaFomatRcv() for
 * groupconf
 * SingleGW:
 * - 4CIF on
 * - FakeVideo is VGA (was QVGA)
 * - if ForceTrans: select min from H.323 mediaformat and 640x480
 * - text "there are no parts in gconf" fixed for 640x480
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 27.04.11   Time: 19:14
 * Updated in $/VSNA/std/cpplib
 * - were added auto change media format
 * - were added info media format command
 * - wait time reduced to 1000 ms in EventManager
 * - were added new capability : dynamic change media format
 * - capture : unblock SetFps and GetFrame
 * - receivers can dynamic change media format
 * - were added auto check media format from bitstream in receivers
 * - change scheme BtrVsFPS for vpx
 * - change AviWriter for auto change media format
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 18.10.10   Time: 19:47
 * Updated in $/VSNA/std/cpplib
 * - HQ detect alfa
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 31.03.09   Time: 18:15
 * Updated in $/VSNA/std/cpplib
 * - stream symmetric crypt support
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
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 6.05.06    Time: 16:34
 * Updated in $/VS/std/cpplib
 * -"'desired" receiver medaiformat capability
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 16.11.05   Time: 15:46
 * Updated in $/VS/std/cpplib
 * -new video caps
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 28.10.05   Time: 18:53
 * Updated in $/VS/std/cpplib
 * - find video codecs bug fix
 *
 * *****************  Version 13  *****************
 * User: Melechko     Date: 23.06.05   Time: 13:40
 * Updated in $/VS/std/cpplib
 * Change default bandwidth
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 5.05.05    Time: 20:31
 * Updated in $/VS/std/cpplib
 * bug fix with caps exchange for voice
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 19.01.05   Time: 16:57
 * Updated in $/VS/std/cpplib
 * bug whith new caps (Any Buf len) fixed
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 18.01.05   Time: 14:32
 * Updated in $/VS/std/cpplib
 * added new caps - Any Audio Buffer Len
 ****************************************************************************/
#ifndef VS_CLIENT_CAPS_H
#define VS_CLIENT_CAPS_H


/****************************************************************************
 * Includes
 ****************************************************************************/
#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/cpplib/macro_utils.h"
#include "VS_MediaFormat.h"

/**
 **************************************************************************
 * \brief Capabilities for video, backward compatibility order
 ****************************************************************************/
enum VSCC_Video{
	VSCC_VIDEO_NONE			 =   0,	///< no video
	VSCC_VIDEO_DEFAULT		 =   1,	///< fixed dimension, xc02
	VSCC_VIDEO_ANYSIZE		 =   2,	///< any size, xc02
	VSCC_VIDEO_ANYCODEC		 =   4,	///< fixed dimension, any (supported) codec
	VSCC_VIDEO_DYNCHANGE	 =   8,	///< dynamic change video mode
	VSCC_VIDEO_RCVSTEREO	 =  16,	///< stereo video mode
	VSCC_VIDEO_MULTIPLICITY8 =  32,	///< can rcv w & h multiplicity of 8
	VSCC_VIDEO_H264PORTRETFIX = 64, ///< fix intel hw h.264 decoder for portret mode
	VSCC_VIDEO_MULTIPLICITY4  = 128,///< can rcv w & h multiplicity of 4
	VSCC_VIDEO_MULTIPLICITY2  = 256,///< can rcv w & h multiplicity of 2
};

/**
 **************************************************************************
 * \brief Capabilities for audio, backward compatibility order
 ****************************************************************************/
enum VSCC_Audio{
	VSCC_AUDIO_NONE		= 0,	///< no audio
	VSCC_AUDIO_DEFAULT	= 1,	///< fixed freq, fixed codec
	VSCC_AUDIO_ANYFREQ	= 2,	///< any freq, any codec
	VSCC_AUDIO_ANYBLEN	= 4		///< variable size of capturing buffer
};

/**
 **************************************************************************
 * \brief Capabilities for streans
 ****************************************************************************/
enum VSCC_Stream {
	VSCC_STREAM_NONE					= 0,	///< default
	VSCC_STREAM_ADAPTIVE_DATA_DECODE	= 1,	///< can decode not compressed data track
	VSCC_STREAM_CAN_DECODE_SSL			= 2,	///< can decode ssl crypted stream
	VSCC_STREAM_CAN_USE_SVC				= 4,	///< can use svc
	VSCC_STREAM_CAN_CHANGE_MF_RCV		= 8		///< can dynamic change rcv media format
};

/**
 **************************************************************************
 * \brief Container for Capabilities, filled on client start
 ****************************************************************************/
class VS_ClientCaps
{
public:
	enum class ClientFlags : uint32_t
	{
		NONE						= 0,
		HAS_HANGUP_FLAGS			= 1 << 0,		// client will add HangupFlags::BY_USER at HANGUP_METHOD
		CONFERECNCE_PASS			= 1 << 1,		// client supports CONFERENCE_PASSWORD_REQUIRED
		MULTILOGIN_DIRECTCONNECT	= 1 << 2,		// client will support DirectConnect if server is in MultiLogin mode
													// (old clients has more strict check for username when connect streams)
		MULTILOGIN_REQINVITE		= 1 << 3,		// ClientLib_v28 understands ReqInvite+InviteToMulti with instance (not only call_id)
	};

private:
	int32_t m_int_caps;
	int32_t m_streams_dc;
	int32_t m_bandw_rcv;
	int32_t m_video_rcv;
	int32_t m_video_snd;
	int32_t m_audio_rcv;
	int32_t m_audio_snd;
	int32_t m_rating;
	int32_t m_level;
	int32_t m_level_group;
	int32_t m_screen_width;
	int32_t m_screen_height;
	int32_t m_client_type;
	VS_BinBuff m_mediaformat;
	VS_BinBuff m_mediaformat_rcv;
	VS_BinBuff m_audio_codecs;
	VS_BinBuff m_video_codecs;
	ClientFlags m_client_flags;

	void ReSet();

	uint16_t FindInAudioCodecs(const VS_BinBuff &codecs) const;
	uint32_t FindInVideoCodecs(const VS_BinBuff &codecs) const;
public:
	VS_ClientCaps();
	VS_ClientCaps(const void* buff, size_t size) { Set(buff, size); }
	void Set(const void* buff, size_t size);
	bool Get(void* &buff, size_t &size) const;
	/*******************************************************************/
	int32_t GetStreamsDC() const { return m_streams_dc; }
	void SetStreamsDC(int32_t val) { m_streams_dc = val; }

	int32_t GetBandWRcv() const { return m_bandw_rcv; }
	void SetBandWRcv(int32_t val) { m_bandw_rcv = val; }

	int32_t GetVideoRcv() const { return m_video_rcv; }
	void SetVideoRcv(int32_t val) { m_video_rcv = val; }

	int32_t GetVideoSnd() const { return m_video_snd; }
	void SetVideoSnd(int32_t val) { m_video_snd = val; }

	int32_t GetAudioRcv() const { return m_audio_rcv; }
	void SetAudioRcv(int32_t val) { m_audio_rcv = val; }

	int32_t GetAudioSnd() const { return m_audio_snd; }
	void SetAudioSnd(int32_t val) { m_audio_snd = val; }

	int32_t GetRating() const { return m_rating; }
	void SetRating(int32_t val) { m_rating = val; }

	int32_t GetLevel() const { return m_level; }
	void SetLevel(int32_t val) { m_level = val; }

	int32_t GetLevelGroup() const { return m_level_group; }
	void SetLevelGroup(int32_t val) { m_level_group = val; }

	int32_t GetScreenWidth() const { return m_screen_width; }
	void SetScreenWidth(int32_t val) { m_screen_width = val; }

	int32_t GetScreenHeight() const { return m_screen_height; }
	void SetScreenHeight(int32_t val) { m_screen_height = val; }

	int32_t GetClientType() const { return m_client_type; }
	void SetClientType(int32_t ct) { m_client_type = ct; }

	ClientFlags GetClientFlags() const { return m_client_flags; }
	void SetClientFlags(ClientFlags f) { m_client_flags = f; }
	/*******************************************************************/
	void GetMediaFormat(VS_MediaFormat &fmt) const;
	void SetMediaFormat(const VS_MediaFormat &fmt) {m_mediaformat.Set(&fmt, fmt.GetSize());}
	void SetMediaFormatRcv(const VS_MediaFormat &fmt) {m_mediaformat_rcv.Set(&fmt, fmt.GetSize());}
	void ClearMediaFormatRcv() { m_mediaformat_rcv.Empty(); }
	bool IsFixedRcvMediaFormat(const VS_ClientCaps &from) const;

	VS_MediaFormat GetFmtFromToMe(const VS_ClientCaps &from) const;
	/*******************************************************************/
	void GetAudioCodecs(uint16_t* codecs, size_t& number) const;
	void SetAudioCodecs(const uint16_t* codecs, size_t number) { m_audio_codecs.Set(codecs, number * sizeof(uint16_t)); }

	void GetVideoCodecs(uint32_t* codecs, size_t& number) const;
	void SetVideoCodecs(const uint32_t* codecs, size_t number) { m_video_codecs.Set(codecs, number * sizeof(uint32_t)); }
	/*******************************************************************/
	bool FindVideoCodec(uint32_t codec) const;
	bool FindAudioCodec(uint16_t codec) const;

	friend bool operator==(const VS_ClientCaps& l, const VS_ClientCaps& r);
	friend bool operator!=(const VS_ClientCaps& l, const VS_ClientCaps& r) { return !(l == r); }
};

VS_ENUM_BITOPS(VS_ClientCaps::ClientFlags, uint32_t);

#endif
