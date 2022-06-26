/****************************************************************************
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
*
* Project: TransCoder
*
* $History: VS_BitStreamBuff.cpp $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Transcoder
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Transcoder
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 24.08.04   Time: 19:53
 * Updated in $/VS/Transcoder
 * in GateWay Client added audion spliting onto 195 Byte
*
* *****************  Version 1  *****************
* User: Smirnov      Date: 17.05.04   Time: 17:40
* Created in $/VS/Transcoder
* added RTP foming audio buffers
* added Visicron buffers
*
****************************************************************************/

/****************************************************************************
* \file VS_VS_Buffers.cpp
* \brief Visicron Stream buffers
****************************************************************************/

/****************************************************************************
* Includes
****************************************************************************/
#include "VS_BitStreamBuff.h"
#include <malloc.h>
#include <string.h>
#include <utility>

/****************************************************************************
* Classes
****************************************************************************/
/****************************************************************************
* VS_BitStreamBuff
****************************************************************************/
/****************************************************************************
* Constructor
****************************************************************************/
VS_BitStreamBuff::VS_BitStreamBuff()
{
	m_Size = 0x100;
	m_Buff = (unsigned char *)malloc(m_Size);
	memset(m_Buff, 0, m_Size);
	m_bits = 0;
}

/****************************************************************************
* Destructor
****************************************************************************/
VS_BitStreamBuff::~VS_BitStreamBuff()
{
	free(m_Buff);
}

VS_BitStreamBuff::VS_BitStreamBuff(VS_BitStreamBuff &&rhs) {
	m_Size = rhs.m_Size;
	m_Buff = rhs.m_Buff;
	m_bits = rhs.m_bits;

	rhs.m_Size = 0;
	rhs.m_Buff = nullptr;
	rhs.m_bits = 0;
}

VS_BitStreamBuff &VS_BitStreamBuff::operator=(VS_BitStreamBuff &&rhs) {
	if (this != &rhs) {
		free(m_Buff);

		m_Size = rhs.m_Size;
		m_Buff = rhs.m_Buff;
		m_bits = rhs.m_bits;

		rhs.m_Size = 0;
		rhs.m_Buff = nullptr;
		rhs.m_bits = 0;
	}

	return *this;
}

/****************************************************************************
* Add bits from buff begining from start of length of len
****************************************************************************/
bool VS_BitStreamBuff::Add(const unsigned char* buff, int start, int len)
{
	int totalbits = len;
	if ((m_bits+len+1)/8 > m_Size) {
		int old_size = m_Size;
		m_Size = (m_bits+len+0x100)/8;
		m_Buff = (unsigned char*)realloc(m_Buff, m_Size);
		memset(m_Buff + old_size, 0, m_Size-old_size);
	}
	while (totalbits) {
		int align = 8 - (m_bits&0x7);		// align internal buff to LSB
		const unsigned char* Start = buff + (start>>3);
		int shift = start&0x7;
		if (totalbits>16) {
			if (shift==0 && align==8) {
				int copy_bits = totalbits&(~0x7);
				memcpy(m_Buff+(m_bits>>3), Start, copy_bits>>3);
				start+=copy_bits;
				m_bits+=copy_bits;
				totalbits-=copy_bits;
			}
			else {
				unsigned short val = ((*Start)<<8)|(*(Start+1));
				val <<= shift;			// cut left bits
				val >>=(16-align);		// cut rigth bits, aligned to LSB
				unsigned char bval = (unsigned char)val;
				*(m_Buff+(m_bits>>3))|= bval;
				start+=align;
				m_bits+=align;
				totalbits-=align;
			}
		}
		else {
			unsigned char bval = *Start;
			bval <<= shift;			// move to begin of byte
			bval &= 0x80;			// select MSB only
			bval >>=(8-align);		// move to need position
			*(m_Buff+(m_bits>>3))|= bval;
			start++;
			m_bits++;
			totalbits--;
		}
	}
	return true;
}

/****************************************************************************
* Return number of bits
****************************************************************************/
int VS_BitStreamBuff::Bits() const
{
	return m_bits;
}

/****************************************************************************
* Return pointer to buffer in case of any bits in buff
****************************************************************************/
unsigned char* VS_BitStreamBuff::Buff()
{
	return m_bits ? m_Buff : 0;
}

const unsigned char* VS_BitStreamBuff::Buff() const
{
	return m_bits ? m_Buff : 0;
}

/****************************************************************************
* Clear internal buffer
****************************************************************************/
void VS_BitStreamBuff::Reset()
{
	m_bits = 0; memset(m_Buff, 0, m_Size);
}



/****************************************************************************
* VS_AudioBuff
****************************************************************************/
VS_AudioBuff::VS_AudioBuff()
{
	m_Size = 0;
	m_Buff = 0;
	m_Bytes = 0;
}

VS_AudioBuff::~VS_AudioBuff()
{
	if (m_Buff) free(m_Buff);
}

void VS_AudioBuff::Add(const unsigned char* buff, int size)
{
	if (size+m_Bytes > m_Size) {
		m_Size = (size+m_Bytes+0x100);
		m_Buff = (unsigned char*)realloc(m_Buff, m_Size);
	}
	memcpy(m_Buff+m_Bytes, buff, size);
	m_Bytes+=size;
}

void VS_AudioBuff::AddConstBytes(char byte, int size)
{
	if (size+m_Bytes > m_Size) {
		m_Size = (size+m_Bytes+0x100);
		m_Buff = (unsigned char*)realloc(m_Buff, m_Size);
	}
	memset(m_Buff+m_Bytes, byte, size);
	m_Bytes+=size;
}

void VS_AudioBuff::TruncLeft(int size)
{
	if (size>m_Bytes)
		size = m_Bytes;
	memmove(m_Buff, m_Buff+size, m_Bytes-size);
	m_Bytes-=size;
}

int VS_AudioBuff::Bytes()
{
	return m_Bytes;
}

unsigned char* VS_AudioBuff::Buff()
{
	return m_Buff;
}

void VS_AudioBuff::Reset()
{
	m_Bytes = 0;
}