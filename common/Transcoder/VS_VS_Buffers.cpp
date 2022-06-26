/****************************************************************************
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
*
* Project: TransCoder
*
* $History: VS_VS_Buffers.cpp $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Transcoder
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Transcoder
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 27.12.06   Time: 19:40
 * Updated in $/VS/Transcoder
 * - key frame marker for vs_ input buffer
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 19.01.06   Time: 18:37
 * Updated in $/VS/Transcoder
 * - fixed lost frames detection in viscron format streams
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 2.12.05    Time: 19:59
 * Updated in $/VS/Transcoder
 * new Buffers and keyFrames
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 24.11.05   Time: 18:45
 * Updated in $/VS/Transcoder
 * - request Keyframe transcoding (to VS)
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 8.12.04    Time: 20:26
 * Updated in $/VS/Transcoder
 * new video-audio sinc
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 28.09.04   Time: 22:01
 * Updated in $/VS/Transcoder
 * output buffers in transcoders now are barrel
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 13.07.04   Time: 17:48
 * Updated in $/VS/Transcoder
 * added musor track filtr
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 30.06.04   Time: 18:04
 * Updated in $/VS/Transcoder
 * added sender and reciever audio support
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 28.06.04   Time: 20:18
 * Updated in $/VS/Transcoder
 * write avi class
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 28.06.04   Time: 17:46
 * Updated in $/VS/Transcoder
 * addded time support for video
 * added set-get media format support
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 21.06.04   Time: 12:11
 * Updated in $/VS/Transcoder
 * write in stream video interval
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 21.06.04   Time: 11:52
 * Updated in $/VS/Transcoder
 * visicron frame format
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 18.06.04   Time: 20:59
 * Updated in $/VS/Transcoder
 * bugfix
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 11.06.04   Time: 18:14
 * Updated in $/VS/Transcoder
 * transcoder from rtp to visicron now is working
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
* \file VS_BitStreamBuff.cpp
* \brief Bit Stream container
****************************************************************************/

/****************************************************************************
* Includes
****************************************************************************/
#include "VS_BitStreamBuff.h"
#include "VS_VS_Buffers.h"
#include "../streams/Protocol.h"

#include <cassert>
#include <string.h>

/****************************************************************************
* Structures
****************************************************************************/


/****************************************************************************
* Classes
****************************************************************************/
/****************************************************************************
* VS_VS_InputBuffer
****************************************************************************/
/****************************************************************************
* Constructor
****************************************************************************/
VS_VS_InputBuffer::VS_VS_InputBuffer()
{
	m_sections = 0;
	m_counter = 0;
	m_frame_complete = false;
	m_waitKey = true;
}

/****************************************************************************
* Destructor
****************************************************************************/
VS_VS_InputBuffer::~VS_VS_InputBuffer()
{
}

/****************************************************************************
* Add buufer and merge it if need...
****************************************************************************/
bool VS_VS_InputBuffer::Add(const unsigned char* buff, unsigned long size, stream::Track track)
{
	if      (track == stream::Track::audio) {
		m_Abuff.Add(buff, 0, size*8);
	}
	else if (track == stream::Track::video) {
		const auto& sh = *reinterpret_cast<const stream::SliceHeader*>(buff + size - sizeof(stream::SliceHeader));
		if (sh.id == sh.first_id) {	// new frame
			m_counter++;
			if (m_Vbuff.Bits()>0)				// previous frame was not retrived
				m_waitKey = true;
			m_frame_complete = false;
			m_Vbuff.Reset();

			bool IsKey = !!buff[0];
			if (IsKey)
				m_waitKey = false;				// allow recive frames
			else if (m_counter != sh.frame_counter)// not key, lost frames, wait key
				m_waitKey = true;
			m_counter = sh.frame_counter;		// new start counter value
			m_sections = sh.first_id;		// new setions strat value
		}
		else {									// part of frame or part from other frame
			if (m_counter != sh.frame_counter)		// not part of current frame
				m_waitKey = true;
			else if (m_sections != sh.id)// part of farme but not consequtive
				m_waitKey = true;
		}
		if (m_waitKey)
			return false;

		size -= sizeof(stream::SliceHeader);
		m_Vbuff.Add(buff, 0, size*8);
		m_frame_complete = sh.id == 0 && m_sections == 0;
		m_sections--;
	}
	return true;
}

/****************************************************************************
* Fill buffer and size, return num of lost buffers
****************************************************************************/
int VS_VS_InputBuffer::Get(unsigned char* buff, unsigned long &size, stream::Track& track, unsigned long &VideoInterval, bool* key)
{
	size = 0;
	track = {};
	VideoInterval = 0;
	int ret = -1;
	if (m_Abuff.Bits()) {
		size = m_Abuff.Bits()/8;
		if (size) {
			memcpy(buff, m_Abuff.Buff(), size);
			track = stream::Track::audio;
			ret++;
		}
		m_Abuff.Reset();
	}
	if (ret==-1) {				// prev was empy
		if (m_frame_complete) {
			size = m_Vbuff.Bits()/8;
			if (size >5) {
				if (key) *key = *m_Vbuff.Buff()!=0;
				VideoInterval = *reinterpret_cast<const uint32_t*>(m_Vbuff.Buff() + 1);
				size-=5;
				memcpy(buff, m_Vbuff.Buff()+5, size);
				track = stream::Track::video;
				ret++;
			}
			m_Vbuff.Reset();
			m_frame_complete = false;
		}
	}
	return ret;
}

stream::Track VS_VS_InputBuffer::GetRef(unsigned char *& buff, unsigned long & size, unsigned long & VideoInterval, bool& key)
{
	buff = 0;
	size = 0;
	VideoInterval = 0;
	stream::Track type(stream::Track::garbage);
	if (m_Abuff.Bits()) {
		size = m_Abuff.Bits() / 8;
		buff = m_Abuff.Buff();
		type = stream::Track::audio;
	}
	if (type == stream::Track::garbage) { // prev was empy
		if (m_frame_complete) {
			size = m_Vbuff.Bits() / 8;
			if (size > 5) {
				key = *m_Vbuff.Buff() != 0;
				VideoInterval = *reinterpret_cast<const uint32_t*>(m_Vbuff.Buff() + 1);
				size -= 5;
				buff = m_Vbuff.Buff() + 5;
				type = stream::Track::video;
			}
		}
	}
	return type;
}

/****************************************************************************
* Return size of data that next call to Get() will return
****************************************************************************/
unsigned long VS_VS_InputBuffer::NextGetSize() const
{
	unsigned long size(0);
	size = m_Abuff.Bits()/8;
	if (size > 0)
		return size;
	size = m_Vbuff.Bits()/8;
	if (size > 5 && m_frame_complete)
		return size-5;
	return 0;
}

/****************************************************************************
* VS_VS_InputBuffer
****************************************************************************/
/****************************************************************************
* Constructor
****************************************************************************/
VS_VS_OutputBuffer::VS_VS_OutputBuffer()
{
	m_NumberEnd = m_NumberStart = 0;
}

/****************************************************************************
* Destructor
****************************************************************************/
VS_VS_OutputBuffer::~VS_VS_OutputBuffer()
{
}

/****************************************************************************
* Clean all buffers
****************************************************************************/
void VS_VS_OutputBuffer::Clean()
{
	for (int i = 0; i<VS_VS_MAXBUFF; i++)
		m_buff[i].Reset();
	m_NumberEnd = m_NumberStart = 0;
}

/****************************************************************************
* Add buufer and split it if need...
****************************************************************************/
bool VS_VS_OutputBuffer::Add(const unsigned char* buff, unsigned long size, stream::Track track, unsigned long VideoInterval, unsigned char key)
{
	if (size==0) return false;

	VS_BitStreamBuff* sb = &m_buff[m_NumberEnd%VS_VS_MAXBUFF];
	uint8_t track_value = id(track);
	sb->Add((unsigned char*)&track_value, 0, 8);

	if      (track != stream::Track::video) {
		sb->Add(buff, 0, size*8);
		m_NumberEnd++;
		if (PacketsNum()==VS_VS_MAXBUFF) {
			m_buff[m_NumberStart%VS_VS_MAXBUFF].Reset();
			m_NumberStart++;
		}
	}
	else if (track == stream::Track::video) {
		const unsigned n_slices = (size + VS_VS_VIDEOMAXLEN - 1) / VS_VS_VIDEOMAXLEN;
		assert(n_slices > 0);
		assert(n_slices <= 256);
		m_sh.frame_counter++;
		m_sh.id = m_sh.first_id = n_slices - 1;
		sb->Add(&key, 0, 8);
		sb->Add((unsigned char*)&VideoInterval, 0, 32);
		do {
			sb->Add(buff, 0, VS_VS_VIDEOMAXLEN*8);
			sb->Add((unsigned char*)&m_sh, 0, sizeof(m_sh)*8);
			buff+=VS_VS_VIDEOMAXLEN;
			size-=VS_VS_VIDEOMAXLEN;
			m_NumberEnd++;
			if (PacketsNum()==VS_VS_MAXBUFF) {
				m_buff[m_NumberStart%VS_VS_MAXBUFF].Reset();
				m_NumberStart++;
			}
			sb = &m_buff[m_NumberEnd%VS_VS_MAXBUFF];	// point to new stream frame
			sb->Add((unsigned char*)&track_value, 0, 8);		// set the type of new frame
		} while (m_sh.id--);
		sb->Add(buff, 0, size*8);
		sb->Add((unsigned char*)&m_sh, 0, sizeof(m_sh)*8);
		m_NumberEnd++;
		if (PacketsNum()==VS_VS_MAXBUFF) {
			m_buff[m_NumberStart%VS_VS_MAXBUFF].Reset();
			m_NumberStart++;
		}
	}
	return true;
}

/****************************************************************************
* Fill buffer and size, return num of lost buffers
****************************************************************************/
bool VS_VS_OutputBuffer::Get(unsigned char* buff, unsigned long &size, stream::Track& track)
{
	size = 0;
	if (PacketsNum()>0) {
		int BuffNum = m_NumberStart%VS_VS_MAXBUFF;
		size = m_buff[BuffNum].Bits()/8-1;
		track = static_cast<stream::Track>(m_buff[BuffNum].Buff()[0]);
		memcpy(buff, m_buff[BuffNum].Buff()+1, size);
		m_buff[BuffNum].Reset();
		m_NumberStart++;
		return true;
	}
	else
		return false;
}

/****************************************************************************
* Return size of data that next call to Get() will return
****************************************************************************/
unsigned long VS_VS_OutputBuffer::NextGetSize() const
{
	if (PacketsNum()>0) {
		int BuffNum = m_NumberStart%VS_VS_MAXBUFF;
		return m_buff[BuffNum].Bits()/8-1;
	}
	else
		return 0;
}