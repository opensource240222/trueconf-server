/****************************************************************************
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
*
* Project: TransCoder
*
* $History: VS_VS_Buffers.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Transcoder
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Transcoder
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 27.12.06   Time: 19:40
 * Updated in $/VS/transcoder
 * - key frame marker for vs_ input buffer
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 2.12.05    Time: 19:59
 * Updated in $/VS/Transcoder
 * new Buffers and keyFrames
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 28.09.04   Time: 22:01
 * Updated in $/VS/transcoder
 * output buffers in transcoders now are barrel
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 30.06.04   Time: 18:04
 * Updated in $/VS/Transcoder
 * added sender and reciever audio support
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 28.06.04   Time: 20:18
 * Updated in $/VS/Transcoder
 * write avi class
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 28.06.04   Time: 17:46
 * Updated in $/VS/Transcoder
 * addded time support for video
 * added set-get media format support
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 21.06.04   Time: 12:11
 * Updated in $/VS/Transcoder
 * write in stream video interval
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 10.06.04   Time: 14:58
 * Updated in $/VS/Transcoder
 * rtp to visicron stream transcoder
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 17.05.04   Time: 17:40
 * Created in $/VS/Transcoder
 * added RTP foming audio buffers
 * added Visicron buffers
*
****************************************************************************/

/****************************************************************************
* \file VS_VS_Buffers.h
* \brief Visicron Stream buffers
****************************************************************************/
#ifndef VS_VS_BUFFERS_H
#define VS_VS_BUFFERS_H

/****************************************************************************
* Includes
****************************************************************************/
#include "VS_BitStreamBuff.h"
#include "../streams/Protocol.h"

/****************************************************************************
* Structures
****************************************************************************/

/****************************************************************************
* Visicron Video Frame Header
****************************************************************************/
struct visicron_xcc_header
{
	visicron_xcc_header(unsigned char* buff) {
		unsigned short header = buff[0]|((int)buff[1]<<8);
		type	= ((header>>13)&7);
		w		= (((header>>7)&63)+1)*16;
		h		= (((header>>1)&63)+1)*16;
		intra	= header&1;
	}
	unsigned int intra;
	unsigned int h;
	unsigned int w;
	unsigned int type;
};


/****************************************************************************
* Classes
****************************************************************************/
/****************************************************************************
* Visicron Input Buffer
****************************************************************************/
class VS_VS_InputBuffer
{
	VS_BitStreamBuff	m_Vbuff;
	VS_BitStreamBuff	m_Abuff;
	unsigned int		m_sections;
	unsigned char		m_counter;
	bool				m_frame_complete;
	bool				m_waitKey;
public:
	VS_VS_InputBuffer();
	~VS_VS_InputBuffer();
	// add stream format data
	bool Add(const unsigned char* buff, unsigned long size, stream::Track track);
	// get codec compatible data
	int Get(unsigned char* buff, unsigned long& size, stream::Track& track, unsigned long& VideoInterval, bool* key = 0);
	// get reference
	stream::Track GetRef(unsigned char*& buff, unsigned long &size, unsigned long &VideoInterval, bool & key);
	unsigned long NextGetSize() const;
	void Reset() {
		m_Vbuff.Reset();
		m_Abuff.Reset();
		m_sections = 0;
		m_frame_complete = false;
		m_waitKey = true;
		m_counter = 0;
	}
	void Reset(stream::Track type) {
		if (type == stream::Track::video) {
			m_Vbuff.Reset();
			m_frame_complete = false;
		}
		else if (type == stream::Track::audio)
			m_Abuff.Reset();
	}
};

/****************************************************************************
* Visicron Input Buffer
****************************************************************************/
class VS_VS_OutputBuffer
{
	static const int		VS_VS_MAXBUFF = 100;
	static const int		VS_VS_VIDEOMAXLEN = 1200;
	VS_BitStreamBuff		m_buff[VS_VS_MAXBUFF];
	int						m_NumberEnd;
	int						m_NumberStart;
	stream::SliceHeader m_sh;
	void Clean();
public:
	VS_VS_OutputBuffer();
	~VS_VS_OutputBuffer();
	// add codec  data
	bool Add(const unsigned char* buff, unsigned long size, stream::Track track, unsigned long VideoInterval, unsigned char key);
	// get stream data
	bool Get(unsigned char* buff, unsigned long &size, stream::Track& track);
	unsigned long NextGetSize() const;
	int PacketsNum() const { return m_NumberEnd-m_NumberStart; }
};

#endif
