/****************************************************************************
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 *
 * Project: TransCoder
 *
 * $History: AudioCodec.cpp $
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 19.06.12   Time: 19:42
 * Updated in $/VSNA/Transcoder
 * - sanufriev : audio system
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 19.06.12   Time: 17:00
 * Updated in $/VSNA/Transcoder
 * - add g.722.1 24kbps default settings
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 16.07.10   Time: 11:02
 * Updated in $/VSNA/Transcoder
 * - were fix system acodec: AV out memory range
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 29.06.10   Time: 19:25
 * Updated in $/VSNA/Transcoder
 * - fix AwiWriter
 * - fix System AudioCodec Enumerate
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 4.08.09    Time: 20:28
 * Updated in $/VSNA/Transcoder
 * - fix DS for avi player
 * - add AudioCodecSystem class, change acmDriverEnumCallback function
 * - avi player support system audio codecs
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Transcoder
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Transcoder
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 5.07.04    Time: 14:39
 * Updated in $/VS/Transcoder
 * new sinc shema
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 28.06.04   Time: 17:46
 * Updated in $/VS/Transcoder
 * addded time support for video
 * added set-get media format support
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 21.04.04   Time: 17:02
 * Updated in $/VS/Transcoder
 * g723 implemented OK
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 16.04.04   Time: 18:17
 * Updated in $/VS/Transcoder
 * Codec are wave datf Aligned
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 15.04.04   Time: 20:18
 * Created in $/VS/Transcoder
 * Transcoder audio
 *
 ****************************************************************************/

/****************************************************************************
 * \file AudioCodec.cpp
 * \brief Implementation of AudioCodec
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "AudioCodec.h"
#include "stdio.h"

#include <algorithm>

const int AudioCodec::m_SIZE = 96000; // 1sec of mono data 44.1 kHz

/******************************************************************************
* Constructor.
* \param	outTag			- codec to use (defined in mmreg.h)
* \param	coder			- coder or decoder functionality
* \date    16-04-2004
******************************************************************************/
AudioCodec::AudioCodec(uint32_t outTag, bool coder, uint32_t GRAN)
	: m_coder(coder)
	, m_Tag(outTag)
	, m_valid(false)
	, m_pcmfmt(nullptr)
	, m_cdcfmt(nullptr)
	, m_GRAN(GRAN)
	, m_inBuff(nullptr)
	, m_outBuff(nullptr)
	, m_DataPointer(nullptr)
{
	memset(&m_ash, 0, sizeof(ACMSTREAMHEADER));
}

/******************************************************************************
* Destructor. Release resources
* \date    16-04-2004
******************************************************************************/
AudioCodec::~AudioCodec()
{
}

/******************************************************************************
* Convert wave data from/to codec comressed format
* \param	in				- input wave data to convert
* \param	out				- converted data
* \param	insize			- input data length in bytes
* \date    16-04-2004
******************************************************************************/
int	AudioCodec::Convert(uint8_t *in, uint8_t *out, uint32_t insize)
{
	if (!in || !out) return -1;	// error param!
	if (insize==0) return 0;	// nothing to convert

	uint32_t incopied = 0;
	int converted = 0;

	while (true) {
		uint32_t datalost = insize-incopied;
		uint32_t datainbuff = (uint32_t)(m_DataPointer-m_inBuff);
		int alignbytes = (std::min<int>(datalost+datainbuff, m_SIZE)/m_GRAN)*m_GRAN - datainbuff;// align
		if (alignbytes<=0) { // not enough input data...
			memcpy(m_DataPointer, in+incopied, datalost);
			m_DataPointer+= datalost;
			break;
		}
		else {
			memcpy(m_DataPointer, in+incopied, alignbytes);
			incopied+= alignbytes;
			m_ash.cbSrcLength = datainbuff+alignbytes;
			int mRes = ConvertFunction();
			if (mRes != 0) {
				return -1;
			}
			memcpy(out+converted, m_outBuff, m_ash.cbDstLengthUsed);
			converted+= m_ash.cbDstLengthUsed;
			m_DataPointer = m_inBuff;	// no data in buff now
		}
	}
	return converted;
}
