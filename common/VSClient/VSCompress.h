/**
**************************************************************************
* \file VSCompress.h
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
* \brief Video coding classes
*
* \b Project Client
* \author Melechko Ivan
* \date 07.10.2002
*
* $Revision: 2 $
*
* $History: VSCompress.h $
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 25.03.09   Time: 15:03
 * Updated in $/VSNA/VSClient
 * 5.5 PVC enhancments:
 * - added "adaptive data decode" capability
 * - new bitrate control for data
 *
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 8.11.06    Time: 16:53
 * Updated in $/VS/VSClient
 * - NHP headers added
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 26.05.06   Time: 14:26
 * Updated in $/VS/VSClient
 * - Nhp new format
 * - send stat in Nhp
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 28.10.05   Time: 18:52
 * Updated in $/VS/VSClient
 * - system (VFW) video codecs support
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
****************************************************************************/
#ifndef _VSCOMPRESS_
#define _VSCOMPRESS_

/****************************************************************************
* Includes
****************************************************************************/
class VS_MediaFormat;
class VS_VideoCodecManager;
class VideoCodec;

#include "../std/cpplib/VS_VideoLevelCaps.h"

/**
**************************************************************************
* \brief Video Compressor class. Use own thread
****************************************************************************/
class CVideoCompressor : public CVSThread
{
protected:
	unsigned char *			m_pBuffer;		///< buffer for compressed data
	VS_VideoCodecManager *	m_pManager;		///< video codec
	int						m_Bitrate;		///< current bitrate (correspondent to 10 fps)
	HANDLE					m_hEvRequest;	///< compress request event
	HANDLE					m_hEvAnswer;	///< compress complete event
	bool					m_bKey;			///< key Frame indicator
	bool					m_bSuccess;		///< data compress result
	DWORD					m_Marker;		///< reseved data
	DWORD					m_ComprSize;	///< last compressed data size
	unsigned char *			m_pInpBuffer;	///< pointer to data queued for compression
	unsigned char 			m_FrameCounter;	///< Compressed frame counter
	int						m_baseWidth;
	int						m_baseHeight;
	eHardwareEncoder		m_eHardwareType;///< codec hardware type
private:
	void CompressingFrame();
public:
	/// Thread procedure
	DWORD Loop(LPVOID lpParameter);
	/// create codec, create events
	CVideoCompressor();
	/// delete codec and events
	~CVideoCompressor();

	/// connect certain coder pointed by pvmf
	int ConnectToVideoCompressor(VS_MediaFormat *pvmf);
	/// disconect coder, free recources
	int DisconnectToVideoCompressor();
	/// queue video data for compression
	int CompressFrame(unsigned char *pBuffer,bool &bKey, DWORD Marker);
	/// get compression result
	bool GetResult(){return m_bSuccess;};
	/// set bitrate
	int SetBitrate(int baseBitrate, int maxBitrate, int framerate);
	/// get bitrate
	int GetBitrate(){return m_Bitrate;};
	/// Get compress complete event
	HANDLE GetEventEx(){return m_hEvAnswer;}
	/// Get compressed data and it attribute
	unsigned char *GetCompressedData(int *pSize, bool *pbKey, int *pw, int *ph);
};

/**
**************************************************************************
* \brief Video Decompresssor class.
****************************************************************************/
class CVideoDecompressor
{
protected:
	VideoCodec * m_pCodec;		/// video codec
public:
	/// create codec
	CVideoDecompressor();
	/// delete codec, free recources
	~CVideoDecompressor();
	/// connect certain decoder pointed by pvmf
	int ConnectToVideoDecompressor(VS_MediaFormat *pvmf);
	/// disconect decoder, free recources
	int DisconnectToVideoDecompressor();
	/// decompress frame
	int DecompressFrame(unsigned char *in, int size, bool bKey, unsigned char *out);
};

#endif