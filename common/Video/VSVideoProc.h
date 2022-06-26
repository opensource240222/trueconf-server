/**
 **************************************************************************
 * \file VSVideoProc.h
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief Contain color conversions, image resampling methods, deinterlacing
 *
 * \b Project Video
 * \author SMirnovK
 * \date 25.11.2002
 *
 * $Revision: 16 $
 *
 * $History: VSVideoProc.h $
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 30.05.12   Time: 19:35
 * Updated in $/VSNA/Video
 * - fix preprocessing modules on IPP for non SSE2 cpu
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 12.05.12   Time: 11:57
 * Updated in $/VSNA/Video
 * - were added mirror self view video
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 12.04.12   Time: 17:08
 * Updated in $/VSNA/Video
 * - add ipp nv12 -> i420 csc
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 19.03.12   Time: 18:47
 * Updated in $/VSNA/Video
 * - improve csc input format (ipp wrap)
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 21.10.11   Time: 19:01
 * Updated in $/VSNA/Video
 * - support correct SD/HQ capture on HD PTZ cameras
 * - in video proc were add ResampleCropI420_IPP based on IPP
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/Video
 * - were added auto stereo mode detect
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 30.11.10   Time: 17:48
 * Updated in $/VSNA/Video
 * - SSE2 ench VideoProc
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 29.11.10   Time: 11:16
 * Updated in $/VSNA/Video
 * - VideoProc: SSE2 improve
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 8.11.10    Time: 19:05
 * Updated in $/VSNA/video
 * - SSE2 optimization VideoProc
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 22.10.10   Time: 16:19
 * Updated in $/VSNA/Video
 * - optimization resampling functions
 * - optimization deinterlacing
 * - clean VideoProc class
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 20.10.10   Time: 17:06
 * Updated in $/VSNA/Video
 * - fix sse2 VideoProc
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 15.10.10   Time: 16:58
 * Updated in $/VSNA/Video
 * - enable SSE2 in VideoProc
 * - added SSE2 implementation deinterlace (up to 25% performance of MMX)
 * - remove FAST algorithm of deinterlace
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 14.10.10   Time: 14:39
 * Updated in $/VSNA/Video
 * - improved deinterlacing
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 4.06.10    Time: 17:35
 * Updated in $/VSNA/Video
 * - Direct3D Render implementation
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 6.05.08    Time: 12:29
 * Updated in $/VSNA/Video
 * - fixed MMX ConvertI420ToBMF16
 * - were add RGB565 resampling functions
 * - were add TestVideo project
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Video
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Video
 *
 * *****************  Version 27  *****************
 * User: Smirnov      Date: 31.07.06   Time: 20:51
 * Updated in $/VS/Video
 * - added SSE2 videoproc class (turned off now)
 * - align impruvements for 8 bit resampling MMX methods
 *
 * *****************  Version 26  *****************
 * User: Smirnov      Date: 27.07.06   Time: 13:44
 * Updated in $/VS/Video
 * - added HQ Resize
 *
 * *****************  Version 25  *****************
 * User: Smirnov      Date: 24.07.06   Time: 16:56
 * Updated in $/VS/Video
 * - added deintrlacing algorithm
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 14.10.05   Time: 17:28
 * Updated in $/VS/Video
 * - coversion to 16 bit rgb
 *
 * *****************  Version 23  *****************
 * User: Sanufriev    Date: 23.09.05   Time: 16:13
 * Updated in $/VS/Video
 * - added 8-bit image processing, C & MMX version
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 1.06.05    Time: 21:03
 * Updated in $/VS/Video
 * bilinear scaling integration in Video project
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 12.04.05   Time: 18:10
 * Updated in $/VS/Video
 * new cliping-expand schema from source image
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 26.11.04   Time: 19:23
 * Updated in $/VS/Video
 * mode for resize
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 26.07.04   Time: 19:35
 * Updated in $/VS/Video
 * added media format initialization
 * added clipping/expand support (pan&scan)
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 23.03.04   Time: 15:54
 * Updated in $/VS/Video
 * added dithering
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 17.03.04   Time: 18:47
 * Updated in $/VS/Video
 * added dib bilinear
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 16.03.04   Time: 20:26
 * Updated in $/VS/video
 * new ipp LIB
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 16.03.04   Time: 14:20
 * Updated in $/VS/Video
 * new capture frame func
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 26.02.04   Time: 17:22
 * Updated in $/VS/Video
 * added Clipping Support
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 24.02.04   Time: 13:56
 * Updated in $/VS/Video
 * added no Use MMX
 * Intrpolation Up2 times n MMX
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 13.02.04   Time: 21:02
 * Updated in $/VS/Video
 * codec and camrera working now whith I420 only!!!
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 13.02.04   Time: 14:51
 * Updated in $/VS/Video
 * rewroten videoProc
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 12.02.04   Time: 17:01
 * Updated in $/VS/Video
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 11.04.03   Time: 19:40
 * Updated in $/VS/Video
 * rewrited all rendering,
 * added support YUY2, UYVY
 * History: 25.11.02 Created
 *
 ****************************************************************************/
#ifndef VS_VIDEO_PROC_H
#define VS_VIDEO_PROC_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include <stdio.h>
#include <windows.h>

#include "std/cpplib/VS_Cpu.h"
#include "IppLib2/VSVideoProcessingIpp.h"

struct VS_ResizeStruct {
	int *x_pos_8bit, *x_pos_24bit, *x_pos_32bit;
	int *y_pos;
	int *dirx_cf;
	int *invx_cf;
	int *diry_cf;
	int *invy_cf;
	short *bicubic_cf_x, *bicubic_cf_x_8bit;
	short *bicubic_cf_y;
	int bicubic_cf_len_x, bicubic_cf_len_x_8bit;
	int bicubic_cf_len_y;
	// SIMD
	unsigned short *x_add_reg;
	unsigned short *y_add_reg;
	unsigned short *dirx_cf_reg;
	unsigned short *invx_cf_reg;
	unsigned short *diry_cf_reg;
	unsigned short *invy_cf_reg;
	// Dimensions
	int srcW, srcH;
	int dstW, dstH;
};

/**
**************************************************************************
 * \brief base class for videoprocessing, contain C version of func.
 ****************************************************************************/
class CVSVideoProc
{
protected:
	VS_ResizeStruct *m_resize_st[2];
	unsigned char *m_tbuff;
public:
	CVSVideoProc();
	virtual ~CVSVideoProc();
	/// reverce color conversions
	virtual bool ConvertI420ToYUY2 (BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToUYVY (BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToBMF16(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToBMF24(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToBMF32(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
			bool ConvertI420ToYUV444(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertYUV444ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	/// direct color conversions
	virtual bool ConvertYV12ToI420 (BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
	virtual bool ConvertYUY2ToI420 (BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
	virtual bool ConvertUYVYToI420 (BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
	virtual bool ConvertBMF24ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
	virtual bool ConvertBMF32ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
	virtual bool ConvertRGB24ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
	virtual bool ConvertI42SToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
	virtual bool ConvertNV12ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW) { return false; }
	/// resampling
	virtual bool ResampleI420	 (BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH);
	bool ResampleRGB32	 (BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep);
	bool ResampleRGB24	 (BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep);
	bool ResampleRGB8	 (BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep);
	bool ResampleRGB565	 (BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep);
	bool ResampleUp_2	 (BYTE* src, BYTE *dst, int srcW, int srcH);
	bool ResampleUp_1d5	 (BYTE* src, BYTE *dst, int srcW, int srcH);
	bool ResampleDown_8d1(BYTE* src, BYTE *dst, int width, int height);
	virtual bool ResampleCropI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW, int dstH, int dstStep,
								  int srcWR, int srcHR, int srcOffsetW, int srcOffsetH, double factorW, double factorH, int mode);
	virtual bool ResampleInscribedI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW, int dstH, int dstStep,
									   int srcOffsetW, int srcOffsetH, double factorW, double factorH, int mode);
	/// image process
	bool ClipI420			(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int mode);
	bool ExpandRGB24		(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int mode);
	bool SaturateI420		(BYTE* src, int srcW, int srcH, DWORD dwSaturation);
	virtual bool DitherI420	(BYTE* src, int srcW, int srcH, int Bits);
	virtual bool MirrorI420	(BYTE* src, BYTE* dst, int srcW, int srcH, int mode);
protected:
	virtual bool InitResize(int w, int h, int new_w, int new_h);
	virtual void ReleaseResize();
	bool ClipExpandPlain	(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int mode, int fillcolor, int ExpMode);
	//mode =0 - 8 bit, 1 - 24 bit, 2 - 32 bit
	virtual bool ResizeBilinear(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep, int mode, int isUVPlane);
	/// saturate chroma component
	virtual bool SaturateI420Chroma(BYTE* src, int srcW, int srcH, DWORD dwSaturation);
	/// bicubic interpolation additional methods
	virtual void InterpolateHor2(BYTE *src, BYTE *dst, int w, int h, bool Luma);
	virtual void InterpolateVer2(BYTE *src, BYTE *dst, int w, int h, bool Luma);
	virtual void InterpolateHor1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma);
	virtual void InterpolateVer1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma);
};

/**
**************************************************************************
 * \brief derived class for videoprocessing, MMX version of func.
 ****************************************************************************/
class CVSVideoProc_MMX: public CVSVideoProc
{
public:
	CVSVideoProc_MMX() {};
	virtual ~CVSVideoProc_MMX() {};
	/// reverce color conversions
	virtual bool ConvertI420ToYUY2 (BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToUYVY (BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	bool ConvertI420ToBMF16(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToBMF24(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToBMF32(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertYUV444ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool DitherI420(BYTE* src, int srcW, int srcH, int Bits);
	/// direct color conversions
	virtual bool ConvertUYVYToI420 (BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
	virtual bool ConvertBMF24ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
	virtual bool ConvertBMF32ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
protected:
	virtual bool InitResize(int w, int h, int new_w, int new_h);
	virtual bool ResizeBilinear(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep, int mode, int isUVPlane);
	/// saturate chroma component
	virtual bool SaturateI420Chroma(BYTE* src, int srcWidth, int srcHeight, DWORD dwSaturation);
	/// bicubic interpolation additional methods
	virtual void InterpolateHor2(BYTE *src, BYTE *dst, int w, int h, bool Luma);
	virtual void InterpolateVer2(BYTE *src, BYTE *dst, int w, int h, bool Luma);
	virtual void InterpolateHor1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma);
	virtual void InterpolateVer1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma);
};

/**
**************************************************************************
 * \brief derived class for videoprocessing, SSE2 version of func.
 ****************************************************************************/
class CVSVideoProc_SSE2: public CVSVideoProc_MMX
{
public:
	CVSVideoProc_SSE2() {};
	~CVSVideoProc_SSE2() {};
	virtual bool DitherI420(BYTE* src, int srcW, int srcH, int Bits);
	/// reverce color conversions
	virtual bool ConvertI420ToYUY2 (BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToUYVY (BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToBMF24(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToBMF32(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertI420ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	virtual bool ConvertYUV444ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcW, int h, int dstW);
	/// direct color conversions
	virtual bool ConvertUYVYToI420 (BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
	virtual bool ConvertBMF24ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
	virtual bool ConvertBMF32ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW);
protected:
	virtual bool InitResize(int w, int h, int new_w, int new_h);
	virtual bool ResizeBilinear(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep, int mode, int isUVPlane);
	/// saturate chroma component
	virtual bool SaturateI420Chroma(BYTE* src, int srcWidth, int srcHeight, DWORD dwSaturation);
	/// bicubic interpolation additional methods
	virtual void InterpolateHor2(BYTE *src, BYTE *dst, int w, int h, bool Luma);
	virtual void InterpolateVer2(BYTE *src, BYTE *dst, int w, int h, bool Luma);
	virtual void InterpolateHor1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma);
	virtual void InterpolateVer1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma);
};

class CVSVideoProc_IPP: public CVSVideoProc_SSE2
{
public:
	/// direct color conversions
	bool ConvertYUY2ToI420 (BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW) override;
	bool ConvertUYVYToI420 (BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW) override;
	bool ConvertBMF24ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW) override;
	bool ConvertBMF32ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW) override;
	bool ConvertNV12ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW) override;
	/// mirror
	bool MirrorI420(BYTE* src, BYTE* dst, int srcW, int srcH, int mode) override;
	/// AR resize
	bool ResampleI420(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH) override;
	bool ResampleCropI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW, int dstH, int dstStep,
								  int srcWR, int srcHR, int srcOffsetW, int srcOffsetH, double factorW, double factorH, int mode) override;
	bool ResampleInscribedI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW, int dstH, int dstStep,
									   int srcOffsetW, int srcOffsetH, double factorW, double factorH, int mode) override;

private:
	VSVideoProcessingIpp m_videoProcessingIpp;
};

/**
**************************************************************************
 * VS_VideoProc - videoprocessing class wrapper.
 ****************************************************************************/
class VS_VideoProc {
	CVSVideoProc	* vp;
	int				cpu;
public:
	VS_VideoProc(bool NotUseMMX = false);

	~VS_VideoProc();
	int Cpu();
	/************************************************************************/
	bool ConvertI420ToYUY2(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW);

	bool ConvertI420ToUYVY(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW);

	bool ConvertI420ToBMF16(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW);

	bool ConvertI420ToBMF24(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW);

	bool ConvertI420ToBMF32(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW);

	bool ConvertI420ToYUV444(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW);

	bool ConvertI420ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW);

	bool ConvertYUV444ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW);
	/************************************************************************/
	bool ConvertYV12ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW);

	bool ConvertYUY2ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW);

	bool ConvertUYVYToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW);

	bool ConvertBMF24ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW);

	bool ConvertBMF32ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW);

	bool ConvertRGB24ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW);


	bool ConvertI42SToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW);

	bool ConvertNV12ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW);
	/************************************************************************/
	bool ResampleI420(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH);

	bool ResampleRGB32(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int dstStep);

	bool ResampleRGB24(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int dstStep);

	bool ResampleRGB8(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int dstStep);

	bool ResampleRGB565(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int dstStep);

	bool ResampleUp_2(BYTE* src, BYTE* dst, int width, int height);

	bool ResampleUp_1d5(BYTE* src, BYTE* dst, int width, int height);

	bool ResampleDown_8d1(BYTE* src, BYTE* dst, int width, int height);

	bool ResampleCropI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW, int dstH,
	                      int dstStep,
	                      int srcWR, int srcHR, int srcOffsetW, int srcOffsetH, double factorW, double factorH,
	                      int mode = IPPI_INTER_CUBIC);

	bool ResampleInscribedI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW, int dstH,
	                           int dstStep,
	                           int srcOffsetW, int srcOffsetH, double factorW, double factorH,
	                           int mode = IPPI_INTER_CUBIC);
	/************************************************************************/
	bool SaturateI420(BYTE* src, int srcWidth, int srcHeight, DWORD dwSaturation);

	bool ClipI420(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int mode);

	bool ExpandRGB24(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int mode);

	bool DitherI420(BYTE* src, int srcW, int srcH, int Bits);

	bool MirrorVertical(BYTE* src, BYTE* dst, int srcW, int srcH);
};

#endif /*VS_VIDEO_PROC_H*/