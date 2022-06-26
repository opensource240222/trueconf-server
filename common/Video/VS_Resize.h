/**
 **************************************************************************
 * \file VS_Resize.h
 * \brief Declare API for scaling
 *
 * (c) 2006 Visicron Corp.  http://www.visicron.com/
 *
 * \b Project: High Quality Resampling
 * \author Smirnov Konstatntin
 * \author Sergey Anufriev
 * \date 24.07.2006
 *
 * $Revision: 1 $
 * $History: VS_Resize.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Video
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Video
 *
 * *****************  Version 1  *****************
 * User: Sanufriev    Date: 27.07.06   Time: 12:33
 * Created in $/VCR_Scaling/ResamplingYUV
 * - added Fast Mode & HQ algorithm for YUV420 images
 ***************************************************************************/

#ifndef  __VCR_HQRESIZE_H__
#define __VCR_HQRESIZE_H__

/// Error status
enum VS_ErrorStatus {
	VS_ErrSts_NoError	=  0,	///< no error
	VS_ErrSts_BadArg	= -1,	///< arguments passed to function are out of range or wrong
	VS_ErrSts_NoMem		= -2,	///< memory allocation error
	VS_ErrSts_NotInit	= -3	///< initialisation error
};

/// Processor(code) type.
enum VS_ProcType {
	VS_PrType_Common		= 0,  ///< C implementation
	VS_PrType_MMX			= 1,  ///< MMX implementation
	VS_PrType_SSE2			= 2   ///< SSE2 implementation
};

/// Processor(code) type.
enum VS_AlgMode {
	VS_FastMode				= 0,  ///< C implementation
	VS_HQMode				= 1   ///< MMX implementation
};

struct VS_InitStruct;

class VS_HQResampling
{
public:
	VS_HQResampling(VS_ProcType prType = VS_PrType_Common, VS_AlgMode algMode = VS_FastMode);
	~VS_HQResampling();
	VS_ErrorStatus			Init(int src_width, int src_height, int dst_width, int dst_height);
	VS_ErrorStatus			ResamplingYUV(unsigned char* src, unsigned char* dst);
	void					Release();
private:
	bool					m_Init;
	VS_ProcType				m_prType;
	VS_AlgMode				m_algMode;
	VS_InitStruct			*m_Str;
	void					HQResampling_C(unsigned char* src, unsigned char* dst);
	void					HQResampling_MMX(unsigned char* src, unsigned char* dst);
	void					HQResampling_SSE2(unsigned char* src, unsigned char* dst);
	void					(VS_HQResampling::*HQResampling)(unsigned char* src, unsigned char* dst);
};

#endif /*__VS_HQRESIZE_H__*/