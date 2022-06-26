/**
 **************************************************************************
 * \file VS_RTP_Buffers.h
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief Input and Output RTP buffers for video and audio data
 *
 * \b Project: TransCoder
 * \author SMirnovK
 * \date 14.05.2004
 *
 * $Revision: 3 $
 *
 * $History: VS_RTP_Buffers.h $
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 13.06.12   Time: 18:34
 * Updated in $/VSNA/Transcoder
 * - frame size in rtp packet
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:01
 * Updated in $/VSNA/Transcoder
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 29.09.07   Time: 13:03
 * Updated in $/VS2005/Transcoder
 * - added h.263/h.264 bitsream parsing (get frame resolution from
 * bitstream)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Transcoder
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 14.12.05   Time: 17:42
 * Updated in $/VS/Transcoder
 * - added g722 audiocodec supporting
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 2.12.05    Time: 19:59
 * Updated in $/VS/Transcoder
 * new Buffers and keyFrames
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 19.10.05   Time: 19:36
 * Updated in $/VS/Transcoder
 * - h263+ input buffers now backward compatible with h263
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 21.09.05   Time: 14:13
 * Updated in $/VS/Transcoder
 * - added RTP buffers for H264
 * - added base class for all video input buffers
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 19.09.05   Time: 18:06
 * Updated in $/VS/Transcoder
 * - added RTP H261 buffers
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 16.09.05   Time: 12:04
 * Updated in $/VS/Transcoder
 * - added g7221_24 support in gateway
 * - added h.263+ RTP support
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 19.08.05   Time: 15:37
 * Updated in $/VS/Transcoder
 * - metall voise in Polycom fixed for g728
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 18.08.05   Time: 20:34
 * Updated in $/VS/Transcoder
 * - added support for g729a
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 10.06.05   Time: 19:27
 * Updated in $/VS/Transcoder
 *  - new base rtp buffer
 *  - g728 codec are embeded
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 25.02.05   Time: 16:08
 * Updated in $/VS/Transcoder
 * g711 -> g711a
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 24.02.05   Time: 19:29
 * Updated in $/VS/Transcoder
 * added g711
 * added format initaalization
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 28.09.04   Time: 22:01
 * Updated in $/VS/Transcoder
 * output buffers in transcoders now are barrel
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 24.09.04   Time: 20:22
 * Updated in $/VS/Transcoder
 * mode formin packets
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 10.09.04   Time: 19:12
 * Updated in $/VS/Transcoder
 * lost frames detection
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 23.07.04   Time: 14:59
 * Updated in $/VS/Transcoder
 * returned video interval correction
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 23.07.04   Time: 14:31
 * Updated in $/VS/Transcoder
 * video frames interval
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 23.07.04   Time: 12:20
 * Updated in $/VS/transcoder
 * set same rtp ssrc
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 28.06.04   Time: 17:46
 * Updated in $/VS/Transcoder
 * addded time support for video
 * added set-get media format support
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 17.05.04   Time: 17:40
 * Updated in $/VS/Transcoder
 * added RTP foming audio buffers
 * added Visicron buffers
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 14.05.04   Time: 19:07
 * Created in $/VS/Transcoder
 * added RTP H263 buffers
*
****************************************************************************/
#ifndef VS_RTP_BUFFERS_H
#define VS_RTP_BUFFERS_H

/****************************************************************************
* Includes
****************************************************************************/
#include "RTPPacket.h"
#include "RtpPayload.h"
#include "VS_BitStreamBuff.h"
#include "tools/Server/CommonTypes.h"

#include <boost/circular_buffer.hpp>

#include <chrono>
#include <queue>
#include <vector>

/**
**************************************************************************
* \brief Base Rtp Input buffer class
****************************************************************************/
class VS_RTP_InputBuffer
{
protected:
	unsigned long		m_time;
	unsigned short		m_seq;
public:
	VS_RTP_InputBuffer();
	virtual ~VS_RTP_InputBuffer();
	virtual int Add(RTPPacket *rtp) = 0;
	virtual int Get(unsigned char* in, unsigned long &size, unsigned long &VideoInterval, char &key) = 0;
	virtual unsigned long NextGetSize() const = 0;
	virtual int GetResolution(unsigned char* in, int size, int &width, int &height) {return -1;}
};

/**
**************************************************************************
* \brief Base Rtp Ouput buffer class
****************************************************************************/
class VS_RTP_OutputBuffer
{
protected:
	static const int	VS_RTP_MAXLEN = 1400;
	VS_BitStreamBuff	m_buff;
	unsigned long		m_time;
	unsigned short		m_seq;
	unsigned char		m_ptype;
	boost::circular_buffer<RTPPacket> m_packets;
	void Clean();
	void NewPacket(unsigned char *buff, int len, unsigned long timestamp, bool marker);
public:
	unsigned long		m_ssrc;
	VS_RTP_OutputBuffer(unsigned char ptype = RTP_PT_AUTO);
	virtual ~VS_RTP_OutputBuffer(){};
	virtual void Add(const void* buff, unsigned long buffsize) = 0;
	virtual bool Get(unsigned char* in, unsigned long &size);
	virtual unsigned long NextGetSize() const;
	size_t PacketsNum() const { return m_packets.size(); }

	unsigned short GetSeq() const { return m_seq; }
	void SetSeq(unsigned short seq) { m_seq = seq; }
};

/****************************************************************************
* Video.
****************************************************************************/
/**
**************************************************************************
* \brief RTP Audio Input Buffer common class
****************************************************************************/
class VS_RTP_InputBufferVideo : public VS_RTP_InputBuffer
{
protected:
	VS_BitStreamBuff	m_buff[2];
	int					m_curr_buff;
	bool				m_frame_complete[2];
	uint32_t			m_timestamp;
	virtual void AddRTPData(RTPPacket *rtp);
	virtual bool IsKey(unsigned char *data, size_t size){return false;}
public:
	VS_RTP_InputBufferVideo();
	VS_RTP_InputBufferVideo(VS_RTP_InputBufferVideo &&rhs);
	VS_RTP_InputBufferVideo &operator=(VS_RTP_InputBufferVideo &&rhs);
	virtual ~VS_RTP_InputBufferVideo(){};
	int Add(RTPPacket *rtp);
	int Get(unsigned char* in, unsigned long &size, unsigned long &VideoInterval, char &key);
	unsigned long NextGetSize() const;
	uint32_t GetVideoTimestamp() const;

	VS_BitStreamBuff *GetCurrentBitStreamBuff()
	{
		return &m_buff[m_curr_buff];
	}
};


/**
**************************************************************************
* \brief RTP H263 Input Buffer
****************************************************************************/
class VS_RTP_H263InputBuffer : public VS_RTP_InputBufferVideo
{
protected:
	bool IsKey(unsigned char *data, size_t size) override;
	void AddRTPData(RTPPacket *rtp) override;
public:
	VS_RTP_H263InputBuffer() {}
	VS_RTP_H263InputBuffer(VS_RTP_H263InputBuffer &&rhs) : VS_RTP_InputBufferVideo(std::move(rhs)) {}
	VS_RTP_H263InputBuffer &operator=(VS_RTP_H263InputBuffer &&rhs) {
		VS_RTP_InputBufferVideo::operator=(std::move(rhs));
		return *this;
	}
	int GetResolution(unsigned char* in, int size, int &width, int &height);
};

/**
**************************************************************************
* \brief RTP H263+ Input Buffer
****************************************************************************/
class VS_RTP_H263PlusInputBuffer : public VS_RTP_H263InputBuffer
{
	void AddRTPData(RTPPacket *rtp) override;
};

/**
**************************************************************************
* \brief RTP H261 Input Buffer
****************************************************************************/
class VS_RTP_H261InputBuffer : public VS_RTP_InputBufferVideo
{
	bool IsKey(unsigned char *data, size_t size) override;
	void AddRTPData(RTPPacket *rtp) override;
};

/**
**************************************************************************
* \brief RTP H264 Input Buffer
****************************************************************************/
class VS_RTP_H264InputBuffer : public VS_RTP_H263InputBuffer
{
	enum NAL_Unit_Type {
		NAL_UT_RESERVED	 = 0x00, // Reserved
		NAL_UT_SLICE	 = 0x01, // Coded Slice - slice_layer_no_partioning_rbsp
		NAL_UT_DPA		 = 0x02, // Coded Data partition A - dpa_layer_rbsp
		NAL_UT_DPB		 = 0x03, // Coded Data partition A - dpa_layer_rbsp
		NAL_UT_DPC		 = 0x04, // Coded Data partition A - dpa_layer_rbsp
		NAL_UT_IDR_SLICE = 0x05, // Coded Slice of a IDR Picture - slice_layer_no_partioning_rbsp
		NAL_UT_SEI		 = 0x06, // Supplemental Enhancement Information - sei_rbsp
		NAL_UT_SPS		 = 0x07, // Sequence Parameter Set - seq_parameter_set_rbsp
		NAL_UT_PPS		 = 0x08, // Picture Parameter Set - pic_parameter_set_rbsp
		NAL_UT_PD		 = 0x09, // Picture Delimiter - pic_delimiter_rbsp
		NAL_UT_FD		 = 0x0a, // Filler Data - filler_data_rbsp
		NAL_FU_A		 = 0x1c,
		NAL_FU_B		 = 0x1d,
		NAL_STAP_A		 = 0x18,
		NAL_STAP_B		 = 0x19,
		NAL_MTAP16		 = 0x1A,
		NAL_MTAP24		 = 0x1B,


	};

	void AddRTPData(RTPPacket *rtp) override;
	bool IsKey(unsigned char *data, size_t size) override;

	void InitStartSequence(unsigned char header);

	unsigned char *	m_nalBuff;
	unsigned char * m_posInBuff;

	bool m_DropThisUnit;
	int  m_seqNum;

	static const int SIZE_OF_BUFFER = 600 * 1024;
public:
	VS_RTP_H264InputBuffer() {
		m_seqNum = 0; m_DropThisUnit = true;
		m_posInBuff = m_nalBuff =  new unsigned char[SIZE_OF_BUFFER];}
	~VS_RTP_H264InputBuffer() {if (m_nalBuff) delete[] m_nalBuff;}

	VS_RTP_H264InputBuffer(VS_RTP_H264InputBuffer &&rhs)
		: VS_RTP_H263InputBuffer(static_cast<VS_RTP_H263InputBuffer&&>(rhs))
		, m_nalBuff(rhs.m_nalBuff)
		, m_posInBuff(rhs.m_posInBuff)
		, m_DropThisUnit(rhs.m_DropThisUnit)
		, m_seqNum(rhs.m_seqNum)
	{
		rhs.m_seqNum = 0; rhs.m_DropThisUnit = true;
		rhs.m_posInBuff = rhs.m_nalBuff = 0;
	}
	VS_RTP_H264InputBuffer &operator=(VS_RTP_H264InputBuffer &&rhs) {
		VS_RTP_H263InputBuffer::operator=(static_cast<VS_RTP_H263InputBuffer&&>(rhs));

		if (this != &rhs) {
			if (m_nalBuff) delete[] m_nalBuff;

			m_seqNum = rhs.m_seqNum;
			m_DropThisUnit = rhs.m_DropThisUnit;
			m_posInBuff = rhs.m_posInBuff;
			m_nalBuff = rhs.m_nalBuff;

			rhs.m_seqNum = 0; rhs.m_DropThisUnit = true;
			rhs.m_posInBuff = rhs.m_nalBuff = 0;
		}

		return *this;
	}

	int GetResolution(unsigned char* in, int size, int &width, int &height);
};

/**
**************************************************************************
* \brief RTP H263 Output Buffer
****************************************************************************/
class VS_RTP_H263OutputBuffer : public VS_RTP_OutputBuffer
{
protected:
	RTPPayload			m_payload;
	bool FormNewRTPPacket(int sbits, int ebits, unsigned long timestamp, bool last, RTPMedia_Mode mode);
public:
	VS_RTP_H263OutputBuffer(unsigned char ptype = RTP_PT_H263) : VS_RTP_OutputBuffer(ptype ) {m_payload.Clean();}
	void Add(const void* buff, unsigned long buffsize) override;
};


/**
**************************************************************************
* \brief RTP H263+ Output Buffer
****************************************************************************/
class VS_RTP_H263PlusOutputBuffer : public VS_RTP_OutputBuffer
{
public:
	VS_RTP_H263PlusOutputBuffer(unsigned char ptype): VS_RTP_OutputBuffer(ptype){};
	void Add(const void* buff, unsigned long buffsize) override;
};

/**
**************************************************************************
* \brief RTP H261 Output Buffer
****************************************************************************/
class VS_RTP_H261OutputBuffer : public VS_RTP_H263OutputBuffer
{
public:
	VS_RTP_H261OutputBuffer(): VS_RTP_H263OutputBuffer(RTP_PT_H261){
		m_payload.h261.intra_frame = 0;
		m_payload.h261.motion_vector = 1;
	};
};

/**
**************************************************************************
* \brief RTP H264 Output Buffer
****************************************************************************/
class VS_RTP_H264OutputBuffer : public VS_RTP_OutputBuffer
{
	std::vector<uint8_t> m_buffer;
public:
	VS_RTP_H264OutputBuffer(unsigned char ptype): VS_RTP_OutputBuffer(ptype){};
	void Add(const void* buff, unsigned long buffsize) override;
};



/****************************************************************************
* Audio.
****************************************************************************/

/**
**************************************************************************
* \brief RTP Audio Input Buffer common class
****************************************************************************/
class VS_RTP_InputBufferAudio: public VS_RTP_InputBuffer
{
	VS_BitStreamBuff	m_buff;
	Rtp_PayloadType		m_ptype;
	bool m_reverse_bytes;
public:
	VS_RTP_InputBufferAudio(Rtp_PayloadType ptype, bool reverse_bytes = false){m_reverse_bytes = reverse_bytes; m_ptype = ptype;}
	~VS_RTP_InputBufferAudio(){};
	int Add(RTPPacket *rtp);
	int Get(unsigned char* in, unsigned long &size, unsigned long &VideoInterval, char &key);
	unsigned long NextGetSize() const;
};

/**
**************************************************************************
* \brief RTP Audio Output Buffer Common class
****************************************************************************/
class VS_RTP_OutputBufferAudio : public VS_RTP_OutputBuffer
{
protected:
	unsigned long 		m_splitSize, m_timeStep;
	unsigned long		m_sampleRate; /// in kHz
	VS_AudioBuff		m_abuff;
	bool m_reverse_bytes;
	std::chrono::steady_clock::time_point m_last_add_pack_time;

	static const std::chrono::steady_clock::duration silence_duration_before_marker;

	bool GetMarkerFlag();

public:
	VS_RTP_OutputBufferAudio(const unsigned char ptype, const int splitSize, const int timeStep, const unsigned long sampleRate, bool reverse_bytes = false)
		: VS_RTP_OutputBuffer(ptype) {
		m_splitSize = splitSize;
		m_timeStep= timeStep;
		m_sampleRate = sampleRate;
		m_reverse_bytes = reverse_bytes;
	}
	void Add(const void* buff, unsigned long buffsize) override;
};

/**
**************************************************************************
* \brief RTP Audio Input Buffer for MPEG audio codec
****************************************************************************/
class VS_RTP_InputBufferMPA : public VS_RTP_InputBuffer
{
	std::queue<std::vector<char>> m_queue;
	std::vector<char> m_fragment;

public:
	VS_RTP_InputBufferMPA();
	int Add(RTPPacket *rtp) override;
	int Get(unsigned char* in, unsigned long &size, unsigned long &VideoInterval, char &key) override;
	unsigned long NextGetSize() const override;
};

/**
**************************************************************************
* \brief RTP Audio Output Buffer for MPEG audio codec
****************************************************************************/
class VS_RTP_OutputBufferMPA : public VS_RTP_OutputBuffer
{
	unsigned char m_assemble_buff[VS_RTP_MAXLEN];

public:
	VS_RTP_OutputBufferMPA();
	void Add(const void* buff, unsigned long buffsize) override;
};

/**
**************************************************************************
* \brief RTP Audio Output Buffer for Speex/16000 codec
****************************************************************************/
class VS_RTP_OutputBufferAudioSpeex : public VS_RTP_OutputBufferAudio
{
//	unsigned long 		m_splitSize, m_timeStep;
//	VS_AudioBuff		m_abuff;
public:
	VS_RTP_OutputBufferAudioSpeex(const unsigned char ptype, const int splitSize, const int timeStep, const unsigned long sampleRate)
		: VS_RTP_OutputBufferAudio(ptype, splitSize, timeStep, sampleRate)
	{
//			m_splitSize = splitSize;
//			m_timeStep= timeStep;
	}
	void Add(const void* buff, unsigned long buffsize) override;
};

class VS_RTP_OutputBufferAudioOpus : public VS_RTP_OutputBufferAudio {
public:
	VS_RTP_OutputBufferAudioOpus(const unsigned char ptype, const int splitSize, const int timeStep, const unsigned long sampleRate)
		: VS_RTP_OutputBufferAudio(ptype, splitSize, timeStep, sampleRate) {
	}
	void Add(const void* buff, unsigned long buffsize) override;
};

/****************************************************************************
 * Coder and decoder declaration macros
 ****************************************************************************/
#define DECLARE_RTPAUDIO_INPUT(name, ptype) \
class VS_RTP_InputBuffer##name: public VS_RTP_InputBufferAudio { \
public: \
	VS_RTP_InputBuffer##name() : VS_RTP_InputBufferAudio(ptype){} \
};

#define DECLARE_RTPAUDIO_OUTPUT(name, ptype, split, time,sampleRate) \
class VS_RTP_OutputBuffer##name: public VS_RTP_OutputBufferAudio { \
public: \
	VS_RTP_OutputBuffer##name() : VS_RTP_OutputBufferAudio (ptype, split, time,sampleRate){} \
};

/**
**************************************************************************
RFC 3551 audio codecs characteristic
 name of                              sampling              default
   encoding  sample/frame  bits/sample      rate  ms/frame  ms/packet
   __________________________________________________________________
   DVI4      sample        4                var.                   20
   G722      sample        8              16,000                   20
   G723      frame         N/A             8,000        30         30
   G726-40   sample        5               8,000                   20
   G726-32   sample        4               8,000                   20
   G726-24   sample        3               8,000                   20
   G726-16   sample        2               8,000                   20
   G728      frame         N/A             8,000       2.5         20
   G729      frame         N/A             8,000        10         20
   G729D     frame         N/A             8,000        10         20
   G729E     frame         N/A             8,000        10         20
   GSM       frame         N/A             8,000        20         20
   GSM-EFR   frame         N/A             8,000        20         20
   L8        sample        8                var.                   20
   L16       sample        16               var.                   20
   LPC       frame         N/A             8,000        20         20
   MPA       frame         N/A              var.      var.
   PCMA      sample        8                var.                   20
   PCMU      sample        8                var.                   20
   QCELP     frame         N/A             8,000        20         20
   VDVI      sample        var.             var.                   20

   Table 1: Properties of Audio Encodings (N/A: not applicable; var.:
            variable)

 ****************************************************************************/
DECLARE_RTPAUDIO_INPUT(G711a, RTP_PT_G711_ALAW)
DECLARE_RTPAUDIO_INPUT(G711mu, RTP_PT_G711_ULAW)
DECLARE_RTPAUDIO_INPUT(G723, RTP_PT_G723)
DECLARE_RTPAUDIO_INPUT(G728, RTP_PT_G728)
DECLARE_RTPAUDIO_INPUT(G729, RTP_PT_G729A)
DECLARE_RTPAUDIO_INPUT(G722, RTP_PT_G722)

DECLARE_RTPAUDIO_OUTPUT(G711a, RTP_PT_G711_ALAW, 160, 20, 8)
////DECLARE_RTPAUDIO_OUTPUT(G711a, RTP_PT_G711_ALAW, 320, 40)
DECLARE_RTPAUDIO_OUTPUT(G711mu, RTP_PT_G711_ULAW, 160, 20, 8)
///DECLARE_RTPAUDIO_OUTPUT(G711mu, RTP_PT_G711_ULAW, 320, 40)
///DECLARE_RTPAUDIO_OUTPUT(G723, RTP_PT_G723, 48, 60)
DECLARE_RTPAUDIO_OUTPUT(G723, RTP_PT_G723, 24, 30, 8)
//DECLARE_RTPAUDIO_OUTPUT(G728, RTP_PT_G728, 80, 40)
DECLARE_RTPAUDIO_OUTPUT(G728, RTP_PT_G728, 40, 20, 8)
////DECLARE_RTPAUDIO_OUTPUT(G729, RTP_PT_G729A, 40, 40)
DECLARE_RTPAUDIO_OUTPUT(G729, RTP_PT_G729A, 20, 20, 8)
//DECLARE_RTPAUDIO_OUTPUT(G722, RTP_PT_G722, 320, 40)
DECLARE_RTPAUDIO_OUTPUT(G722, RTP_PT_G722, 160, 20, 8)

/**
**************************************************************************
* \brief RTP Input Buffer for MPEG4 Elementary Streams (RFC3640)
****************************************************************************/
class VS_RTP_InputBufferMPEG4ES : public VS_RTP_InputBuffer
{
protected:
	VS_MPEG4ESConfiguration m_conf;
	std::vector<char> m_fau; // Fragmented AU
	unsigned int m_fau_size;

	virtual void AddAU(const void* data, size_t size) = 0;

public:
	VS_RTP_InputBufferMPEG4ES(const VS_MPEG4ESConfiguration& conf);
	virtual ~VS_RTP_InputBufferMPEG4ES();
	int Add(RTPPacket *rtp) override;
};

/**
**************************************************************************
* \brief RTP Input Buffer for AAC (RFC3640)
****************************************************************************/
class VS_RTP_InputBufferAAC : public VS_RTP_InputBufferMPEG4ES
{
protected:
	unsigned m_aot; // MPEG-4 Audio Object Type
	unsigned m_sfi; // MPEG-4 Sampling Frequency Index
	unsigned m_cc; // MPEG-4 Channel Configuration
	std::queue<std::vector<char>> m_queue;

	void AddAU(const void* data, size_t size) override;

public:
	VS_RTP_InputBufferAAC(const VS_MPEG4ESConfiguration& conf);
	virtual ~VS_RTP_InputBufferAAC();
	int Get(unsigned char* in, unsigned long &size, unsigned long &VideoInterval, char &key) override;
	unsigned long NextGetSize() const override;
};

#include <boost/container/flat_map.hpp>

class VS_RTP_InputBufferXH264UC : public VS_RTP_InputBuffer {
	boost::container::flat_map<unsigned, VS_RTP_H264InputBuffer> m_temporal_layers;
	//std::map<unsigned, VS_RTP_H264InputBuffer> m_temporal_layers;
	uint32_t m_timestamp;
public:
	VS_RTP_InputBufferXH264UC() : m_timestamp(0) {};
	virtual ~VS_RTP_InputBufferXH264UC() {};
	int Add(RTPPacket *rtp) override;
	int Get(unsigned char* in, unsigned long &size, unsigned long &VideoInterval, char &key) override;
	unsigned long NextGetSize() const override;
	int GetResolution(unsigned char* in, int size, int &width, int &height) override;
};

#endif
