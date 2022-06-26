/****************************************************************************
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
*
* Project: TransCoder
*
* $History: VS_BitStreamBuff.h $
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
* \file VS_BitStreamBuff.h
* \brief Bit Stream container
****************************************************************************/
#ifndef VS_BITSTREAMBUFF_H
#define VS_BITSTREAMBUFF_H

/****************************************************************************
* Classes
****************************************************************************/
/****************************************************************************
* VS_BitStreamBuff
****************************************************************************/
class VS_BitStreamBuff
{
	unsigned char*	m_Buff;
	int 			m_bits;
	int				m_Size;
public:
	VS_BitStreamBuff();
	~VS_BitStreamBuff();
	VS_BitStreamBuff(VS_BitStreamBuff &&rhs);
	VS_BitStreamBuff &operator=(VS_BitStreamBuff &&rhs);
	bool Add(const unsigned char* buff, int start, int len);
	int Bits() const;
	unsigned char* Buff();
	const unsigned char* Buff() const;
	void Reset();
};

/****************************************************************************
* VS_AudioBuff
****************************************************************************/
class VS_AudioBuff
{
	unsigned char*	m_Buff;
	int				m_Bytes;
	int				m_Size;
public:
	VS_AudioBuff();
	~VS_AudioBuff();
	void Add(const unsigned char* buff, int size);
	void AddConstBytes(char byte, int size);
	void TruncLeft(int size);
	int Bytes();
	unsigned char* Buff();
	void Reset();
};

#endif
