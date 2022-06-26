/****************************************************************************
 * (c) 2002 Visicron Inc.  http://www.visicron.net/
 *
 * Project: VSVideo processing
 *
 * $Revision: 11 $
 * $History: VSVideoProc.cpp $
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 30.05.12   Time: 19:35
 * Updated in $/VSNA/Video
 * - fix preprocessing modules on IPP for non SSE2 cpu
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 12.05.12   Time: 11:57
 * Updated in $/VSNA/Video
 * - were added mirror self view video
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 19.03.12   Time: 18:47
 * Updated in $/VSNA/Video
 * - improve csc input format (ipp wrap)
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 21.10.11   Time: 19:01
 * Updated in $/VSNA/Video
 * - support correct SD/HQ capture on HD PTZ cameras
 * - in video proc were add ResampleCropI420_IPP based on IPP
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/Video
 * - were added auto stereo mode detect
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 29.11.10   Time: 11:16
 * Updated in $/VSNA/Video
 * - VideoProc: SSE2 improve
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 22.10.10   Time: 16:19
 * Updated in $/VSNA/Video
 * - optimization resampling functions
 * - optimization deinterlacing
 * - clean VideoProc class
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
 * *****************  Version 34  *****************
 * User: Smirnov      Date: 27.07.06   Time: 13:44
 * Updated in $/VS/Video
 * - added HQ Resize
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 14.10.05   Time: 17:28
 * Updated in $/VS/Video
 * - coversion to 16 bit rgb
 *
 * *****************  Version 32  *****************
 * User: Smirnov      Date: 28.09.05   Time: 18:55
 * Updated in $/VS/Video
 * - another bug whith proportional scaling resolved
 *
 * *****************  Version 31  *****************
 * User: Smirnov      Date: 26.09.05   Time: 16:17
 * Updated in $/VS/Video
 * - I420 for mutitranscoder
 *
 * *****************  Version 30  *****************
 * User: Sanufriev    Date: 23.09.05   Time: 16:13
 * Updated in $/VS/Video
 * - added 8-bit image processing, C & MMX version
 *
 * *****************  Version 29  *****************
 * User: Sanufriev    Date: 4.07.05    Time: 16:04
 * Updated in $/VS/Video
 * - added 32-bit image processing, C & MMX version
 *
 * *****************  Version 28  *****************
 * User: Smirnov      Date: 22.06.05   Time: 20:13
 * Updated in $/VS/Video
 * - new files in Video project
 * - bicubic minification embeedded in videoproc class
 *
 * *****************  Version 27  *****************
 * User: Smirnov      Date: 1.06.05    Time: 21:03
 * Updated in $/VS/Video
 * bilinear scaling integration in Video project
 *
 * *****************  Version 26  *****************
 * User: Smirnov      Date: 12.04.05   Time: 18:10
 * Updated in $/VS/Video
 * new cliping-expand schema from source image
 *
 * *****************  Version 25  *****************
 * User: Smirnov      Date: 11.01.05   Time: 19:39
 * Updated in $/VS/Video
 * added amd mmx suppotr for ipp library
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 26.11.04   Time: 19:23
 * Updated in $/VS/Video
 * mode for resize
 *
 * *****************  Version 23  *****************
 * User: Smirnov      Date: 26.07.04   Time: 19:35
 * Updated in $/VS/Video
 * added media format initialization
 * added clipping/expand support (pan&scan)
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 23.03.04   Time: 15:54
 * Updated in $/VS/Video
 * added dithering
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 18.03.04   Time: 12:08
 * Updated in $/VS/Video
 * ippInit
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 17.03.04   Time: 18:47
 * Updated in $/VS/Video
 * added dib bilinear
 *
 * *****************  Version 19  *****************
 * User: Smirnov      Date: 16.03.04   Time: 20:26
 * Updated in $/VS/Video
 * new ipp LIB
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 16.03.04   Time: 14:20
 * Updated in $/VS/Video
 * new capture frame func
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 26.02.04   Time: 17:22
 * Updated in $/VS/Video
 * added Clipping Support
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 17.02.04   Time: 19:47
 * Updated in $/VS/Video
 * scaling tested
 * test util
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 13.02.04   Time: 14:51
 * Updated in $/VS/Video
 * rewroten videoProc
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 12.02.04   Time: 17:01
 * Updated in $/VS/Video
 *
 * *****************  Version 13  *****************
 * User: Smirnov      Date: 11.04.03   Time: 21:31
 * Updated in $/VS/Video
 * rgb - old variant
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 11.04.03   Time: 20:20
 * Updated in $/VS/Video
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 11.04.03   Time: 20:14
 * Updated in $/VS/Video
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 11.04.03   Time: 19:40
 * Updated in $/VS/Video
 * rewrited all rendering,
 * added support YUY2, UYVY
 * History: 25.11.02 Created
 *
 ****************************************************************************/
/**
 * \file VSVideoProc.cpp
 * \brief Contain color conversions and image resampling methods
 */

#include "VSVideoDefines.h"
#include "VSVideoProc.h"
#include "bilinear.h"
#include "bicubic.h"

#include <cstdlib>

// table to use with CLIP() in this library
BYTE cipping_table[1024];
/**
 ******************************************************************************
 * Constructor. Init static cliping table and set proc type.
 *
 *  \date    25-11-2002		Copied from XCcodec project
 ******************************************************************************
 */
CVSVideoProc::CVSVideoProc()
{
	int i = 0;
	for (i = 0; i < 2; i++) {
		m_resize_st[i] = (VS_ResizeStruct*)malloc(sizeof(VS_ResizeStruct));
		memset(m_resize_st[i], 0, sizeof(VS_ResizeStruct));
	}
	m_tbuff = 0;
	for (int i = 0; i< sizeof(cipping_table); i++)
	{
		if (i<512) cipping_table[i] = 0;
		else if (i<(512+256)) cipping_table[i] = i-512;
		else cipping_table[i] = 255;
	}
}

CVSVideoProc::~CVSVideoProc()
{
	int i = 0;
	ReleaseResize();
	for (i = 0; i < 2; i++) {
		if (m_resize_st[i]) free(m_resize_st[i]); m_resize_st[i] = 0;
	}
}

bool CVSVideoProc::InitResize(int w, int h, int new_w, int new_h)
{
	int k;
	double factor = (double)(w - 1) / (new_w - 1);

	ReleaseResize();

	m_tbuff = (unsigned char*)_aligned_malloc(h*new_w*4, 16);

	for (k = 0; k < 2; k++) {
		/* TO DO: почему не дает приемущества в кленте?
		m_resize_st[k]->x_pos_8bit = (int*)malloc(new_w*sizeof(int));
		m_resize_st[k]->x_pos_24bit = (int*)malloc(new_w*sizeof(int));
		m_resize_st[k]->x_pos_32bit = (int*)malloc(new_w*sizeof(int));
		m_resize_st[k]->y_pos = (int*)malloc(new_h*sizeof(int));
		m_resize_st[k]->dirx_cf = (int*)malloc(new_w*sizeof(int));
		m_resize_st[k]->invx_cf = (int*)malloc(new_w*sizeof(int));
		m_resize_st[k]->diry_cf = (int*)malloc(new_h*sizeof(int));
		m_resize_st[k]->invy_cf = (int*)malloc(new_h*sizeof(int));

		/// x dir bilinear coef
		for (i = 0; i < new_w; i++) {
			x = factor * i;
			m_resize_st[k]->x_pos_8bit[i] = (int)(factor * i);
			m_resize_st[k]->x_pos_24bit[i] = (int)(factor * i) * 3;
			m_resize_st[k]->x_pos_32bit[i] = (int)(factor * i) * 4;
			x -= (double)(int)x;
			m_resize_st[k]->dirx_cf[i] = (int)((1 - x) * (1 << 8) + 0.5);
			m_resize_st[k]->invx_cf[i] = (1 << 8) - m_resize_st[k]->dirx_cf[i];
		}
		/// y dir bilinear coef
		factor = (double)(h - 1) / (new_h - 1);
		for (j = 1; j < new_h - 1; j++) {
			x = factor * j;
			m_resize_st[k]->y_pos[j] = (int)(x);
			x -= (double)(int)x;
			m_resize_st[k]->diry_cf[j] = (int)((1 - x) * (1 << 8) + 0.5);
			m_resize_st[k]->invy_cf[j] = (1 << 8) - m_resize_st[k]->diry_cf[j];
		}
		*/
		/// bicubic coef
		m_resize_st[k]->bicubic_cf_len_x = genTableAlloc(w, new_w, m_resize_st[k]->bicubic_cf_x);
		m_resize_st[k]->bicubic_cf_len_y = genTableAlloc(h, new_h, m_resize_st[k]->bicubic_cf_y);

		m_resize_st[k]->srcW = w;
		m_resize_st[k]->srcH = h;
		m_resize_st[k]->dstW = new_w;
		m_resize_st[k]->dstH = new_h;

		w /= 2;
		h /= 2;
		new_w /= 2;
		new_h /= 2;
	}
	return true;
}

void CVSVideoProc::ReleaseResize()
{
	int i = 0;
	for (i = 0; i < 2; i++) {
		if (m_resize_st[i]->x_pos_8bit) free(m_resize_st[i]->x_pos_8bit);
		if (m_resize_st[i]->x_pos_24bit) free(m_resize_st[i]->x_pos_24bit);
		if (m_resize_st[i]->x_pos_32bit) free(m_resize_st[i]->x_pos_32bit);
		if (m_resize_st[i]->y_pos) free(m_resize_st[i]->y_pos);
		if (m_resize_st[i]->dirx_cf) free(m_resize_st[i]->dirx_cf);
		if (m_resize_st[i]->invx_cf) free(m_resize_st[i]->invx_cf);
		if (m_resize_st[i]->diry_cf) free(m_resize_st[i]->diry_cf);
		if (m_resize_st[i]->invy_cf) free(m_resize_st[i]->invy_cf);
		if (m_resize_st[i]->bicubic_cf_x) delete [] m_resize_st[i]->bicubic_cf_x;
		if (m_resize_st[i]->bicubic_cf_x_8bit) delete [] m_resize_st[i]->bicubic_cf_x_8bit;
		if (m_resize_st[i]->bicubic_cf_y) delete [] m_resize_st[i]->bicubic_cf_y;
		/// SIMD
		if (m_resize_st[i]->x_add_reg) _aligned_free(m_resize_st[i]->x_add_reg);
		if (m_resize_st[i]->y_add_reg) _aligned_free(m_resize_st[i]->y_add_reg);
		if (m_resize_st[i]->dirx_cf_reg) _aligned_free(m_resize_st[i]->dirx_cf_reg);
		if (m_resize_st[i]->invx_cf_reg) _aligned_free(m_resize_st[i]->invx_cf_reg);
		if (m_resize_st[i]->diry_cf_reg) _aligned_free(m_resize_st[i]->diry_cf_reg);
		if (m_resize_st[i]->invy_cf_reg) _aligned_free(m_resize_st[i]->invy_cf_reg);
		///
		memset(m_resize_st[i], 0, sizeof(VS_ResizeStruct));
	}
	if (m_tbuff) _aligned_free(m_tbuff); m_tbuff = 0;
}

bool CVSVideoProc::ConvertYV12ToI420 (BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV,
									  int srcW, int h, int dstW)
{
	int size = srcW*h;
	memcpy(dstY, src, size);
	src+=size; size/=4;
	memcpy(dstV, src, size);
	src+=size;
	memcpy(dstU, src, size);
	return true;
}


/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to YUY2 format. C version.
 * \return true if all OK or false in case of bad srcWidth.
 *
 * \note srcWidth must be multiple of 4
 *
 * \param srcY				[in]  - pointer to Y component of source image;
 * \param srcU				[in]  - pointer to U component of source image;
 * \param srcV				[in]  - pointer to V component of source image;
 * \param dst				[out] - pointer to destination YUYV image;
 * \param srcWidth			[in]  - width  of source image;
 * \param height			[in]  - height of source image;
 * \param dstWidth			[in]  - width of destination image, can be greater than defined one;
 *
 *  \date    25-11-2002		Copied from XCcodec project
 ******************************************************************************
 */
bool CVSVideoProc::ConvertI420ToYUY2(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									 BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;
	for(int i = 0; i<height; i++) {
		for(int j=0; j<(srcWidth>>1); j++) {
			dst[j*4 + 0] = srcY[j*2 + 0];
			dst[j*4 + 1] = srcU[j   + 0];
			dst[j*4 + 2] = srcY[j*2 + 1];
			dst[j*4 + 3] = srcV[j   + 0];
		}
		srcY+=srcWidth;
		dst+= dstWidth;
		if(i&1)	srcU+= srcWidth/2, srcV+= srcWidth/2;
	}
	return true;
}
/// direct conversion
bool CVSVideoProc::ConvertYUY2ToI420 (BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV,
									  int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x3 || height&1) return false;
	BYTE *src2, *dstY2;

	for(int i = 0; i<height; i+=2) {
		src2 = src+srcWidth;
		dstY2 = dstY+dstWidth;
		for(int j=0; j<srcWidth/4; j++)	{
			dstY [j*2+0] = src [j*4+0];
			dstY [j*2+1] = src [j*4+2];
			dstY2[j*2+0] = src2[j*4+0];
			dstY2[j*2+1] = src2[j*4+2];
			dstU [j    ] = ((int)src[j*4+1] + (int)src2[j*4+1])/2;
			dstV [j    ] = ((int)src[j*4+3] + (int)src2[j*4+3])/2;
		}
		src+= srcWidth*2;
		dstY+=dstWidth*2;
		dstU+=dstWidth/2;
		dstV+=dstWidth/2;
	}
	return true;
}

bool CVSVideoProc::ConvertI42SToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcWidth, int height, int dstWidth)
{
	if (dstWidth != srcWidth) return false;

	bool ret = true;
	int s = srcWidth * height * 2;
	memcpy(dstY, src, dstWidth * height);
	memcpy(dstU, src + s, dstWidth * height / 4);
	memcpy(dstV, src + 5 * s / 4, dstWidth * height / 4);

	return ret;
}

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to UYVY format. C version.
 * \return true if all OK or false in case of bad srcWidth.
 *
 * \note srcWidth must be multiple of 4
 *
 * \param srcY				[in]  - pointer to Y component of source image;
 * \param srcU				[in]  - pointer to U component of source image;
 * \param srcV				[in]  - pointer to V component of source image;
 * \param dst				[out] - pointer to destination YUYV image;
 * \param srcWidth			[in]  - width  of source image;
 * \param height			[in]  - height of source image;
 * \param dstWidth			[in]  - width of destination image, can be greater than defined one;
 *
 *  \date    25-11-2002		Copied from XCcodec project
 ******************************************************************************
 */
bool CVSVideoProc::ConvertI420ToUYVY(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									 BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;
	for(int i = 0; i<height; i++) {
		for(int j=0; j<(srcWidth>>1); j++)	{
			dst[j*4 + 0] = srcU[j   + 0];
			dst[j*4 + 1] = srcY[j*2 + 0];
			dst[j*4 + 2] = srcV[j   + 0];
			dst[j*4 + 3] = srcY[j*2 + 1];
		}
		srcY+=srcWidth;
		dst+= dstWidth;
		if(i&1)	srcU+= srcWidth/2, srcV+= srcWidth/2;
	}
	return true;
}
/// direct conversion
bool CVSVideoProc::ConvertUYVYToI420 (BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV,
									  int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x3 || height&1) return false;
	BYTE *src2, *dstY2;
	for(int i = 0; i<height; i+=2) {
		src2 = src+srcWidth;
		dstY2 = dstY+dstWidth;
		for(int j=0; j<srcWidth/4; j++)	{
			dstY [j*2+0] = src [j*4+1];
			dstY [j*2+1] = src [j*4+3];
			dstY2[j*2+0] = src2[j*4+1];
			dstY2[j*2+1] = src2[j*4+3];
			dstU [j    ] = ((int)src[j*4+0] + (int)src2[j*4+0])/2;
			dstV [j    ] = ((int)src[j*4+2] + (int)src2[j*4+2])/2;
		}
		src+= srcWidth*2;
		dstY+=dstWidth*2;
		dstU+=dstWidth/2;
		dstV+=dstWidth/2;
	}
	return true;
}

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to 16 bitmap. C version.
 * \return true if all OK or false else.
 *
 * \param srcY				[in]  - pointer to Y component of source image;
 * \param srcU				[in]  - pointer to U component of source image;
 * \param srcV				[in]  - pointer to V component of source image;
 * \param dst				[out] - pointer to destination RGB 16-bit image;
 * \param srcWidth			[in]  - width  of source image;
 * \param height			[in]  - height of source image;
 * \param dstWidth			[in]  - width of destination image in bytes;
 *
 * \date    14-10-2005
 *******************************************************************************/
bool CVSVideoProc::ConvertI420ToBMF16(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									  BYTE *dst, int srcWidth, int height, int dstWidth)
{
	int i ,j;
	int r ,g, b, u, v, y;
	int rr, gg, bb, yy;
	BYTE * pRGB;

#define MAKERGB565 ((r>>3)<<11)|((g>>2)<<5)|(b>>3)

	if (height<0) {
		height = -height;
		dst = dst + dstWidth*height - dstWidth;
		dstWidth = -dstWidth;
	}

	for (i=0; i<height; i+=2) {
		pRGB = dst + dstWidth*(height-1-i);
		for (j=0; j<srcWidth/2 ;j++) {
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j*2+0]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			*(short*)(pRGB+0) = MAKERGB565;

			y = srcY[j*2+1]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			*(short*)(pRGB+2) = MAKERGB565;

			y = srcY[j*2+srcWidth+0]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			*(short*)(pRGB-dstWidth) = MAKERGB565;

			y = srcY[j*2+srcWidth+1]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			*(short*)(pRGB-dstWidth+2) = MAKERGB565;

			pRGB+=4;
		}
		srcY += srcWidth*2;
		srcU += srcWidth/2;
		srcV += srcWidth/2;
	}
	return true;
}

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to bitmap-24 format. C version.
 * \return true if all OK or false else.
 *
 * \note
 *
 * \param srcY				[in]  - pointer to Y component of source image;
 * \param srcU				[in]  - pointer to U component of source image;
 * \param srcV				[in]  - pointer to V component of source image;
 * \param dst				[out] - pointer to destination YUYV image;
 * \param srcWidth			[in]  - width  of source image;
 * \param height			[in]  - height of source image;
 * \param dstWidth			[in]  - width of destination image, can be greater than defined one;
 *
 *  \date    25-11-2002		Copied from XCcodec project, fixed
 ******************************************************************************
 */
bool CVSVideoProc::ConvertI420ToBMF24(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									  BYTE *dst, int srcWidth, int height, int dstWidth)
{
	int i ,j;
	int r ,g, b, u, v, y;
	int rr, gg, bb, yy;

	srcY+=srcWidth*(height-1);
	srcU+=srcWidth/2*(height/2-1);
	srcV+=srcWidth/2*(height/2-1);

	for (i=0; i<height; i+=2)
	{
		for (j=0; j<srcWidth/2 ;j++)
		{
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j*2+0]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[0] = b;
			dst[1] = g;
			dst[2] = r;

			y = srcY[j*2+1]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[3] = b;
			dst[4] = g;
			dst[5] = r;

			y = srcY[j*2-srcWidth+0]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[0+dstWidth] = b;
			dst[1+dstWidth] = g;
			dst[2+dstWidth] = r;

			y = srcY[j*2-srcWidth+1]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[3+dstWidth] = b;
			dst[4+dstWidth] = g;
			dst[5+dstWidth] = r;

			dst+= 6;
		}

		dst+= (dstWidth*2-3*srcWidth);
		srcY-= srcWidth*2;
		srcU-= srcWidth/2;
		srcV-= srcWidth/2;
	}
	return true;
}

/**
 ******************************************************************************
 * Convert image from BMF 24 (Windows BGR 24 bit) to I420. C version.
 * \return none
 *
 * \param Src			[in]  - input BGR image
 * \param DstY			[out] - Y component of output image
 * \param DstU			[out] - U component of output image
 * \param DstV			[out] - V component of output image
 * \param srcW			[in]  - width in bytes of input image
 * \param h				[in]  - height in lines of input image
 * \param dstW			[in]  - width in bytes of output image
 *
 *  \date    31-06-2002
 ******************************************************************************
 */

void  ColorTrans_In_bmp24(
		BYTE* pBGR24,
		BYTE* pY,
		BYTE* pU,
		BYTE* pV,
		int width, int height,
		int w_org,int h_org,
		int pitch
		)
{
    int i, j, ofs;
	int r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;
	int y1, u1, v1, y2, y3, y4;

	//if(width!=w_org || height!=h_org)
	//{
	//	memset(pY,0,width*height);
	//	memset(pU,0x80,width*height/4);
	//	memset(pV,0x80,width*height/4);
	//}

	pY+=(h_org-1)*width;
	pU+=(h_org/2-1)*width/2;
	pV+=(h_org/2-1)*width/2;

    for(i = 0; i<(h_org>>1); i++)
	{
		ofs = 0;
		for(j = 0; j<(w_org>>1); j++)
		{
            int ra,ga,ba;

			b1 = *(pBGR24 + ofs +		 0);
			g1 = *(pBGR24 + ofs +		 1);
			r1 = *(pBGR24 + ofs +		 2);
			b2 = *(pBGR24 + ofs +		 3);
			g2 = *(pBGR24 + ofs +		 4);
			r2 = *(pBGR24 + ofs +		 5);

			b3 = *(pBGR24 + ofs + pitch + 0);
			g3 = *(pBGR24 + ofs + pitch + 1);
			r3 = *(pBGR24 + ofs + pitch + 2);
			b4 = *(pBGR24 + ofs + pitch + 3);
			g4 = *(pBGR24 + ofs + pitch + 4);
			r4 = *(pBGR24 + ofs + pitch + 5);

			countY(r1, g1, b1, y1);
			countY(r2, g2, b2, y2);
			countY(r3, g3, b3, y3);
			countY(r4, g4, b4, y4);

            ra = r1+r2+r3+r4;
            ga = g1+g2+g3+g4;
            ba = b1+b2+b3+b4;

			pY[j*2-width+0	] = CLIP(y3);
			pY[j*2-width+1	] = CLIP(y4);
			pY[j*2+0		] = CLIP(y1);
			pY[j*2+1		] = CLIP(y2);

			countU4(ra, ga, ba, u1);
			countV4(ra, ga, ba, v1);
			pU[j			] = CLIP(u1);
			pV[j			] = CLIP(v1);

			ofs += 6;
        }

		pBGR24 += 2*pitch;
		pY -= 2*width;
		pU -= width/2;
		pV -= width/2;
    }
}

/**
 ******************************************************************************
 * Convert image from RGB 24 to I420. C version.
 * \return none
 *
 * \param Src			[in]  - input BGR image
 * \param DstY			[out] - Y component of output image
 * \param DstU			[out] - U component of output image
 * \param DstV			[out] - V component of output image
 * \param srcW			[in]  - width in bytes of input image
 * \param h				[in]  - height in lines of input image
 * \param dstW			[in]  - width in bytes of output image
 *
 *  \date    07-11-2018
 ******************************************************************************
 */
void  ColorTrans_In_rgb24(
	BYTE* pRGB24,
	BYTE* pY,
	BYTE* pU,
	BYTE* pV,
	int width, int height,
	int w_org, int h_org,
	int pitch
)
{
	int i, j, ofs;
	int r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;
	int y1, u1, v1, y2, y3, y4;

	//if(width!=w_org || height!=h_org)
	//{
	//	memset(pY,0,width*height);
	//	memset(pU,0x80,width*height/4);
	//	memset(pV,0x80,width*height/4);
	//}

	pY += (h_org - 1)*width;
	pU += (h_org / 2 - 1)*width / 2;
	pV += (h_org / 2 - 1)*width / 2;

	for (i = 0; i < (h_org >> 1); i++)
	{
		ofs = 0;
		for (j = 0; j < (w_org >> 1); j++)
		{
			int ra, ga, ba;

			r1 = *(pRGB24 + ofs + 0);
			g1 = *(pRGB24 + ofs + 1);
			b1 = *(pRGB24 + ofs + 2);
			r2 = *(pRGB24 + ofs + 3);
			g2 = *(pRGB24 + ofs + 4);
			b2 = *(pRGB24 + ofs + 5);

			r3 = *(pRGB24 + ofs + pitch + 0);
			g3 = *(pRGB24 + ofs + pitch + 1);
			b3 = *(pRGB24 + ofs + pitch + 2);
			r4 = *(pRGB24 + ofs + pitch + 3);
			g4 = *(pRGB24 + ofs + pitch + 4);
			b4 = *(pRGB24 + ofs + pitch + 5);

			countY(r1, g1, b1, y1);
			countY(r2, g2, b2, y2);
			countY(r3, g3, b3, y3);
			countY(r4, g4, b4, y4);

			ra = r1 + r2 + r3 + r4;
			ga = g1 + g2 + g3 + g4;
			ba = b1 + b2 + b3 + b4;

			pY[j * 2 - width + 0] = CLIP(y3);
			pY[j * 2 - width + 1] = CLIP(y4);
			pY[j * 2 + 0] = CLIP(y1);
			pY[j * 2 + 1] = CLIP(y2);

			countU4(ra, ga, ba, u1);
			countV4(ra, ga, ba, v1);
			pU[j] = CLIP(u1);
			pV[j] = CLIP(v1);

			ofs += 6;
		}

		pRGB24 += 2 * pitch;
		pY -= 2 * width;
		pU -= width / 2;
		pV -= width / 2;
	}
}


bool CVSVideoProc::ConvertBMF24ToI420(BYTE* Src, BYTE* DstY, BYTE* DstU, BYTE *DstV,
									  int srcW, int h, int dstW)
{
	ColorTrans_In_bmp24(Src, DstY, DstU, DstV, dstW, h, dstW, h, srcW);
	/*
    int i, j;
	int r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;
	int y1, y2, y3, y4, u1, v1;
	BYTE *dstY, *dstU, *dstV, *src;

    for(i = 0; i<h; i+=2) {
		dstY = DstY -(h-2-i)*dstW;
		dstU = DstU -(h-2-i)*dstW/4;
		dstV = DstV -(h-2-i)*dstW/4;
		src  = Src + i*srcW;
		for(j = 0; j<srcW; j+=6) {
            int ra, ga, ba;

			b1 = src[		0];	g1 = src[		1];	r1 = src[		2];
			b2 = src[		3];	g2 = src[		4];	r2 = src[		5];
			b3 = src[srcW + 0];	g3 = src[srcW + 1];	r3 = src[srcW + 2];
			b4 = src[srcW + 3];	g4 = src[srcW + 4];	r4 = src[srcW + 5];

			countY(r3, g3, b3, y3);
			countY(r2, g2, b2, y2);
			countY(r1, g1, b1, y1);
			countY(r4, g4, b4, y4);
            ra = r1+r2+r3+r4;
            ga = g1+g2+g3+g4;
            ba = b1+b2+b3+b4;
			countU4(ra, ga, ba, u1);
			countV4(ra, ga, ba, v1);

			dstY[		0] = CLIP(y3);
			dstY[		1] = CLIP(y4);
			dstY[dstW + 0] = CLIP(y1);
			dstY[dstW + 1] = CLIP(y2);
			dstU[		0] = CLIP(u1);
			dstV[		0] = CLIP(v1);

			src +=6;	dstY+=2;	dstU++;		dstV++;
        }
    }
	*/
	return true;
}

bool CVSVideoProc::ConvertRGB24ToI420(BYTE* Src, BYTE* DstY, BYTE* DstU, BYTE *DstV,
	int srcW, int h, int dstW)
{
	ColorTrans_In_rgb24(Src, DstY, DstU, DstV, dstW, h, dstW, h, srcW);
	return true;
}


/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to bitmap-32 format. C version.
 * \return true if all OK or false else.
 *
 * \note
 *
 * \param srcY				[in]  - pointer to Y component of source image;
 * \param srcU				[in]  - pointer to U component of source image;
 * \param srcV				[in]  - pointer to V component of source image;
 * \param dst				[out] - pointer to destination YUYV image;
 * \param srcWidth			[in]  - width  of source image;
 * \param height			[in]  - height of source image;
 * \param dstWidth			[in]  - width of destination image, can be greater than defined one;
 *
 *  \date    25-11-2002		Copied from XCcodec project, fixed
 ******************************************************************************
 */
bool CVSVideoProc::ConvertI420ToBMF32(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									  BYTE *dst, int srcWidth, int height, int dstWidth)
{
	int i ,j;
	int r ,g, b, u, v, y;
	int rr, gg, bb, yy;

	srcY+=srcWidth*(height-1);
	srcU+=srcWidth/2*(height/2-1);
	srcV+=srcWidth/2*(height/2-1);

	for (i=0; i<height; i+=2)
	{
		for (j=0; j<srcWidth/2 ;j++)
		{
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j*2+0]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[0] = b;
			dst[1] = g;
			dst[2] = r;

			y = srcY[j*2+1]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[4] = b;
			dst[5] = g;
			dst[6] = r;

			y = srcY[j*2-srcWidth+0]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[0+dstWidth] = b;
			dst[1+dstWidth] = g;
			dst[2+dstWidth] = r;

			y = srcY[j*2-srcWidth+1]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[4+dstWidth] = b;
			dst[5+dstWidth] = g;
			dst[6+dstWidth] = r;

			dst+= 8;
		}

		dst+= (dstWidth*2-4*srcWidth);
		srcY-= srcWidth*2;
		srcU-= srcWidth/2;
		srcV-= srcWidth/2;
	}
	return true;
}

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to bitmap-32 format. C version. Vertical flip
 * \return true if all OK or false else.
 *
 * \note
 *
 * \param srcY				[in]  - pointer to Y component of source image;
 * \param srcU				[in]  - pointer to U component of source image;
 * \param srcV				[in]  - pointer to V component of source image;
 * \param dst				[out] - pointer to destination YUYV image;
 * \param srcWidth			[in]  - width  of source image;
 * \param height			[in]  - height of source image;
 * \param dstWidth			[in]  - width of destination image, can be greater than defined one;
 *
 *  \date    25-11-2002		Copied from XCcodec project, fixed
 ******************************************************************************
 */
bool CVSVideoProc::ConvertI420ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV,
											BYTE *dst, int srcWidth, int height, int dstWidth)
{
	int i ,j;
	int r ,g, b, u, v, y;
	int rr, gg, bb, yy;

	for (i=0; i<height; i+=2)
	{
		for (j=0; j<srcWidth/2 ;j++)
		{
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j*2+0]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[0] = b;
			dst[1] = g;
			dst[2] = r;

			y = srcY[j*2+1]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[4] = b;
			dst[5] = g;
			dst[6] = r;

			y = srcY[j*2-srcWidth+0]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[0+dstWidth] = b;
			dst[1+dstWidth] = g;
			dst[2+dstWidth] = r;

			y = srcY[j*2-srcWidth+1]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[4+dstWidth] = b;
			dst[5+dstWidth] = g;
			dst[6+dstWidth] = r;

			dst+= 8;
		}

		dst+= (dstWidth*2-4*srcWidth);
		srcY+= srcWidth*2;
		srcU+= srcWidth/2;
		srcV+= srcWidth/2;
	}
	return true;
}

bool CVSVideoProc::ConvertYUV444ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV,
											BYTE *dst, int srcWidth, int height, int dstWidth)
{
	int i ,j;
	int r ,g, b, u, v, y;
	int rr, gg, bb, yy;

	for (i=0; i<height; i++)
	{
		for (j=0; j<srcWidth ;j++)
		{
			u = srcU[j+0]-128;
			v = srcV[j+0]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j+0]-16;
			makeRGB(y, rr, gg, bb, r, g, b, yy);
			dst[0] = b;
			dst[1] = g;
			dst[2] = r;
			dst[3] = 0;

			dst+= 4;
		}

		dst+= (dstWidth-4*srcWidth);
		srcY+= srcWidth;
		srcU+= srcWidth;
		srcV+= srcWidth;
	}
	return true;
}

/**
 ******************************************************************************
 * Convert image from BMF 32 (Windows RGB 32 bit) to I420. C version.
 * \return none
 *
 * \param Src			[in]  - input rgb image
 * \param DstY			[out] - Y component of output image
 * \param DstU			[out] - U component of output image
 * \param DstV			[out] - V component of output image
 * \param srcW			[in]  - width in bytes of input image
 * \param h				[in]  - height in lines of input image
 * \param dstW			[in]  - width in bytes of output image
 *
 *  \date    31-06-2002
 ******************************************************************************
 */

void  ColorTrans_In_bmp32(
		BYTE* pBGR24,
		BYTE* pY,
		BYTE* pU,
		BYTE* pV,
		int width, int height,
		int w_org,int h_org,
		int pitch
		)
{
    int i, j, ofs;
	int r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;
	int y1, u1, v1, y2, y3, y4;

	if(width!=w_org || height!=h_org)
	{
		memset(pY,0,width*height);
		memset(pU,0x80,width*height/4);
		memset(pV,0x80,width*height/4);
	}

	pY+=(h_org-1)*width;
	pU+=(h_org/2-1)*width/2;
	pV+=(h_org/2-1)*width/2;

    for(i = 0; i<(h_org>>1); i++)
	{
		ofs = 0;
		for(j = 0; j<(w_org>>1); j++)
		{
            int ra,ga,ba;

			b1 = *(pBGR24 + ofs +		 0);
			g1 = *(pBGR24 + ofs +		 1);
			r1 = *(pBGR24 + ofs +		 2);
			b2 = *(pBGR24 + ofs +		 4);
			g2 = *(pBGR24 + ofs +		 5);
			r2 = *(pBGR24 + ofs +		 6);

			b3 = *(pBGR24 + ofs + pitch + 0);
			g3 = *(pBGR24 + ofs + pitch + 1);
			r3 = *(pBGR24 + ofs + pitch + 2);
			b4 = *(pBGR24 + ofs + pitch + 4);
			g4 = *(pBGR24 + ofs + pitch + 5);
			r4 = *(pBGR24 + ofs + pitch + 6);

			countY(r1, g1, b1, y1);
			countY(r2, g2, b2, y2);
			countY(r3, g3, b3, y3);
			countY(r4, g4, b4, y4);

            ra = r1+r2+r3+r4;
            ga = g1+g2+g3+g4;
            ba = b1+b2+b3+b4;

			pY[j*2-width+0	] = CLIP(y3);
			pY[j*2-width+1	] = CLIP(y4);
			pY[j*2+0		] = CLIP(y1);
			pY[j*2+1		] = CLIP(y2);

			countU4(ra, ga, ba, u1);
			countV4(ra, ga, ba, v1);
			pU[j			] = CLIP(u1);
			pV[j			] = CLIP(v1);

			ofs += 8;
        }

		pBGR24 += 2*pitch;
		pY -= 2*width;
		pU -= width/2;
		pV -= width/2;
    }
}


bool CVSVideoProc::ConvertBMF32ToI420(BYTE* Src, BYTE* DstY, BYTE* DstU, BYTE *DstV,
									  int srcW, int h, int dstW)
{
	ColorTrans_In_bmp32(Src, DstY, DstU, DstV, dstW, h, dstW, h, srcW);
	/*
    int i, j;
	int r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;
	int y1, y2, y3, y4, u1, v1;
	BYTE *dstY, *dstU, *dstV, *src;

    for(i = 0; i<h; i+=2) {
		dstY = DstY -(h-2-i)*dstW;
		dstU = DstU -(h-2-i)*dstW/4;
		dstV = DstV -(h-2-i)*dstW/4;
		src  = Src + i*srcW;
		for(j = 0; j<srcW; j+=8) {
            int ra, ga, ba;

			b1 = src[		0];	g1 = src[		1];	r1 = src[		2];
			b2 = src[		4];	g2 = src[		5];	r2 = src[		6];
			b3 = src[srcW + 0];	g3 = src[srcW + 1];	r3 = src[srcW + 2];
			b4 = src[srcW + 4];	g4 = src[srcW + 5];	r4 = src[srcW + 6];

			countY(r3, g3, b3, y3);
			countY(r2, g2, b2, y2);
			countY(r1, g1, b1, y1);
			countY(r4, g4, b4, y4);
            ra = r1+r2+r3+r4;
            ga = g1+g2+g3+g4;
            ba = b1+b2+b3+b4;
			countU4(ra, ga, ba, u1);
			countV4(ra, ga, ba, v1);

			dstY[		0] = CLIP(y3);
			dstY[		1] = CLIP(y4);
			dstY[dstW + 0] = CLIP(y1);
			dstY[dstW + 1] = CLIP(y2);
			dstU[		0] = CLIP(u1);
			dstV[		0] = CLIP(v1);

			src +=8;	dstY+=2;	dstU++;		dstV++;
        }
    }
	*/
	return true;
}

bool CVSVideoProc::ConvertI420ToYUV444(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE *dst, int srcWidth, int height, int dstWidth)
{
	bool ret = true;
	// Y don't interpolate
	memcpy(dst, srcY, srcWidth * height);
	// U
	dst += dstWidth * height;
	ret&= ResizeBilinear(srcU, dst, srcWidth / 2, height / 2, srcWidth, height, dstWidth, 0, 1);
	if (!ret) return ret;
	// V
	dst += dstWidth * height;
	ret&= ResizeBilinear(srcV, dst, srcWidth / 2, height / 2, srcWidth, height, dstWidth, 0, 1);
	return ret;
}

/**
 ******************************************************************************
 * Saturate chroma component of I420 image. C version.
 *
 * \note
 *
 * \param src				[in, out]  - pointer to row of source chroma component if image;
 * \param srcWidth			[in]  - width  of source chroma icomponent of image;
 * \param srcHeight			[in]  - height of source chroma icomponent of image;
 * \param dwSaturation		[in]  - Saturation value in percents;
 *
 *  \date    07-02-2003		Created
 ******************************************************************************
 */
bool CVSVideoProc::SaturateI420Chroma(BYTE* src, int srcWidth, int srcHeight, DWORD dwSaturation)
{
	int i, j;
	int sat = dwSaturation*256/100;
	for (j = 0; j<srcHeight; j++) {
		for (i = 0; i<srcWidth; i++)
			src[i]= CLIP((((src[i]-128)*sat+128)>>8)+128);
		src+=srcWidth;
	}
	return true;
}

/******************************************************************************
 * Saturate I420 image
 *
 * \param src				[in, out]  - pointer to I420 image;
 * \param srcWidth			[in]  - width of image;
 * \param srcHeight			[in]  - height of source chroma icomponent of image;
 * \param dwSaturation		[in]  - Saturation value in percents;
 *
 *  \date    07-02-2003		Created
 ******************************************************************************/
bool CVSVideoProc::SaturateI420(BYTE* src, int srcWidth, int srcHeight, DWORD dwSaturation)
{
	if (dwSaturation==100) return true;
	SaturateI420Chroma(src+srcWidth*srcHeight, srcWidth/2, srcHeight/2, dwSaturation);
	SaturateI420Chroma(src+srcWidth*srcHeight*5/4, srcWidth/2, srcHeight/2, dwSaturation);
	return true;
}

/******************************************************************************
 * Mirror I420 image
 *
 * \param src				[in]  - pointer to I420 image;
 * \param dst				[out] - pointer to destination I420 image;
 * \param srcWidth			[in]  - width of image;
 * \param srcHeight			[in]  - height of source chroma icomponent of image;
 * \param mode				[in]  - mirror mode;
 *
 *  \date    07-02-2003		Created
 ******************************************************************************/
bool CVSVideoProc::MirrorI420(BYTE* src, BYTE* dst, int srcWidth, int srcHeight, int mode)
{
	int i, j = 0;
	unsigned char *pSrc = src, *pDst = dst;
	/// Y
	for (j = 0; j < srcHeight; j++) {
		for (i = 0; i < srcWidth; i++) {
			pDst[srcWidth-1-i] = pSrc[i];
		}
		pSrc += srcWidth;
		pDst += srcWidth;
	}
	srcWidth >>= 1;
	srcHeight >>= 1;
	/// U
	for (j = 0; j < srcHeight; j++) {
		for (i = 0; i < srcWidth; i++) {
			pDst[srcWidth-1-i] = pSrc[i];
		}
		pSrc += srcWidth;
		pDst += srcWidth;
	}
	/// V
	for (j = 0; j < srcHeight; j++) {
		for (i = 0; i < srcWidth; i++) {
			pDst[srcWidth-1-i] = pSrc[i];
		}
		pSrc += srcWidth;
		pDst += srcWidth;
	}
	return true;
}

/******************************************************************************
 * Clip I420 image (dstW <= srcW, dstH <= srcH)
 *
 * \param src				[in]  - pointer to input I420 image;
 * \param src				[out] - pointer to clipped ouput I420 image;
 * \param srcW, srcW		[in]  - dimensions of input image;
 * \param dstW, dstW		[in]  - dimensions of ouput  image;
 * \param mode				[in]  - cliping mode;
 *
 *  \date    26-02-2004		Created
 ******************************************************************************/
bool CVSVideoProc::ClipI420(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int mode)
{

	//Y
	if (ClipExpandPlain(src, dst, srcW, srcH, dstW, dstH, mode, 0, 1)) {
		// U
		src+= srcW*srcH;
		dst+= dstW*dstH;
		srcW>>=1; srcH>>=1; dstW>>=1; dstH>>=1;
		if (ClipExpandPlain(src, dst, srcW, srcH, dstW, dstH, mode, 128, 2)) {
			// V
			src+= srcW*srcH;
			dst+= dstW*dstH;
			return  ClipExpandPlain(src, dst, srcW, srcH, dstW, dstH, mode, 128, 2);
		}
	}
	return false;
}

/******************************************************************************
 * Expand or Clip RGB24 image (dstW != srcW, dstH != srcH)
 *
 * \param src				[in]  - pointer to input RGB24 image;
 * \param src				[out] - pointer to clipped ouput RGB24 image;
 * \param srcW, srcW		[in]  - dimensions of input image;
 * \param dstW, dstW		[in]  - dimensions of ouput  image;
 * \param mode				[in]  - cliping mode; [1,...9]
 *
 *  \date    26-07-2004		Created
 ******************************************************************************/
bool CVSVideoProc::ExpandRGB24(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int mode)
{
	return ClipExpandPlain(src, dst, srcW*3, srcH, dstW*3, dstH, mode, 0, 0);
}



int CopyLine(BYTE* dst, BYTE* src, int w, int srcPitch, int type)
{
	// type = scaleCol*4 | scaleRow*2 | nearest;
	switch(type)
	{
	case 0:
	case 1:
		memcpy(dst, src, w);
		return srcPitch;
	case 2:
		for (int i = 0; i<w; i++)
			dst[i] = (src[i*2] + src[i*2+1] + 1)>>1;
		return srcPitch;
	case 3:
		for (int i = 0; i<w; i++)
			dst[i] = src[i*2];
		return srcPitch;
	case 4:
		for (int i = 0; i<w; i++)
			dst[i] = (src[i] + src[i+srcPitch] + 1)>>1;
		return srcPitch*2;
	case 5:
		memcpy(dst, src, w);
		return srcPitch*2;
	case 6:
		for (int i = 0; i<w; i++)
			dst[i] = (src[i*2] + src[i*2+1] + src[i*2+srcPitch] + src[i*2+srcPitch+1] + 2)>>2;
		return srcPitch*2;
	case 7:
		for (int i = 0; i<w; i++)
			dst[i] = src[i*2];
		return srcPitch*2;
	}
	return 0;
}



bool CVSVideoProc::ClipExpandPlain(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int mode, int fillcolor, int ExpMode)
{
	// ExpMode: no scaling=0, nearest=1, belinear=2;
	if (!src || !dst || (srcW|srcH|dstW|dstH)&1) return false;
	// defaults
	bool nearest = ExpMode==1;
	bool scaleRow = false;
	bool scaleCol = false;

	int LeftOffset = 0;
	int UpOffset = 0;
	if (dstW <=srcW/2 && ExpMode!=0) {// scale up rows by 2
		LeftOffset = dstW - srcW/2;
		scaleRow = true;
	}
	else
		LeftOffset = dstW - srcW;
	if (dstH <= srcH/2 && ExpMode!=0) {
		UpOffset = dstH - srcH/2;
		scaleCol = true;
	}
	else
		UpOffset = dstH - srcH;
	int type = scaleCol*4+scaleRow*2+nearest;

	int Mode;
	BYTE *s = 0, *d = 0;

	Mode = (mode-1)/3;
	UpOffset = Mode==0 ? UpOffset : (Mode==1 ? UpOffset/2 : 0);
	Mode = (mode-1)%3;
	LeftOffset = Mode==2 ? LeftOffset : (Mode==1 ? LeftOffset/2 : 0);

	if		(UpOffset>0 && LeftOffset>0) {
		memset(dst, fillcolor, dstW*UpOffset);
		s = src;
		d = dst+dstW*UpOffset;
		for (int i = 0; i<srcH; i++) {
			memset(d, fillcolor, LeftOffset);
			s+=CopyLine(d+LeftOffset, s, srcW, srcW, type);
			memset(d+LeftOffset+srcW, fillcolor, dstW-LeftOffset-srcW);
			d+=dstW;
		}
		memset(d, fillcolor, dstW*(dstH-UpOffset-srcH));
	}
	else if (UpOffset>0 && LeftOffset<=0) {
		memset(dst, fillcolor, dstW*UpOffset);
		s = src-LeftOffset*(1+scaleRow);
		d = dst+dstW*UpOffset;
		for (int i = 0; i<srcH; i++) {
			s+=CopyLine(d, s, dstW, srcW, type);;
			d+=dstW;
		}
		memset(d, fillcolor, dstW*(dstH-UpOffset-srcH));
	}
	else if (UpOffset<=0 && LeftOffset>0) {
		s = src - UpOffset*srcW*(1+scaleCol);
		d = dst;
		for (int i = 0; i<dstH; i++) {
			memset(d, fillcolor, LeftOffset);
			s+=CopyLine(d+LeftOffset, s, srcW, srcW, type);
			memset(d+LeftOffset+srcW, fillcolor, dstW-LeftOffset-srcW);
			d+=dstW;
		}
	}
	else if (UpOffset<=0 && LeftOffset<=0) {
		s = src - UpOffset*srcW*(1+scaleCol) - LeftOffset*(1+scaleRow);
		d = dst;
		for (int i = 0; i<dstH; i++) {
			s+=CopyLine(d, s, dstW, srcW, type);
			d+=dstW;
		}
	}
	return true;
}


/**
 ******************************************************************************
 * Bicubic interpolation by factor 1.5 of the image in hor dim. C version.
 *
 * \note
 *
 * \param src				[in]  - pointer to row of source image component;
 * \param dst				[out] - pointer to interpolated row of destination image component;
 * \param w					[in]  - width  of source image;
 * \param h					[in]  - height of source image;
 * \param Luma				[in]  - true if luma component;
 *
 *  \date    25-11-2002		Created
 ******************************************************************************
 */
void CVSVideoProc::InterpolateHor1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma) // real param
{
	int i, j;
	int BIC_A0, BIC_A1, BIC_A2, BIC_A3;
	if (Luma)
	{
		BIC_A0 = BIC_LA0;
		BIC_A1 = BIC_LA1;
		BIC_A2 = BIC_LA2;
		BIC_A3 = BIC_LA3;
	}
	else
	{
		BIC_A0 = BIC_CA0;
		BIC_A1 = BIC_CA1;
		BIC_A2 = BIC_CA2;
		BIC_A3 = BIC_CA3;
	}

	for (j = 0; j<h; j++)
	{
		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[0] + BIC_A1*src[0] + BIC_A2*src[1] + BIC_A3*src[2] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst[2] = CLIP((BIC_A3*src[0] + BIC_A2*src[1] + BIC_A1*src[2] + BIC_A0*src[3] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst+=3; src+=2;

		for (i = 0; i<(w/2-2); i++)
		{
			dst[0] = src[0];
			dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A2*src[1] + BIC_A3*src[2] + (1<<(RADIX_1-1)))>>RADIX_1);
			dst[2] = CLIP((BIC_A3*src[ 0] + BIC_A2*src[1] + BIC_A1*src[2] + BIC_A0*src[3] + (1<<(RADIX_1-1)))>>RADIX_1);
			dst+=3; src+=2;
		}

		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A2*src[1] + BIC_A3*src[1] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst[2] = CLIP((BIC_A3*src[ 0] + BIC_A2*src[1] + BIC_A1*src[1] + BIC_A0*src[1] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst+=3; src+=2;
	}
}


/**
 ******************************************************************************
 * Bicubic interpolation by factor 1.5 of the image in ver dim. C version.
 *
 * \note
 *
 * \param src				[in]  - pointer to row of source image component;
 * \param dst				[out] - pointer to interpolated row of destination image component;
 * \param w					[in]  - width  of source image;
 * \param h					[in]  - height of source image;
 * \param Luma				[in]  - true if luma component;
 *
 *  \date    25-11-2002		Created.
 ******************************************************************************
 */
void CVSVideoProc::InterpolateVer1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma)
{
	int i, j;
	int BIC_A0, BIC_A1, BIC_A2, BIC_A3;
	if (Luma)
	{
		BIC_A0 = BIC_LA0;
		BIC_A1 = BIC_LA1;
		BIC_A2 = BIC_LA2;
		BIC_A3 = BIC_LA3;
	}
	else
	{
		BIC_A0 = BIC_CA0;
		BIC_A1 = BIC_CA1;
		BIC_A2 = BIC_CA2;
		BIC_A3 = BIC_CA3;
	}

	for (j = 0; j<w; j++)
	{
		dst[0  ] = src[0];
		dst[w  ] = CLIP((BIC_A0*src[0] + BIC_A1*src[0] + BIC_A2*src[w  ] + BIC_A3*src[2*w] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst[2*w] = CLIP((BIC_A3*src[0] + BIC_A2*src[w] + BIC_A1*src[2*w] + BIC_A0*src[3*w] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst++; src++;
	}
	dst+=2*w; src+=w;

	for (i = 0; i<(h/2-2); i++)
	{
		for (j = 0; j<w; j++)
		{
			dst[0  ] = src[0];
			dst[w  ] = CLIP((BIC_A0*src[-w] + BIC_A1*src[0] + BIC_A2*src[w  ] + BIC_A3*src[2*w] + (1<<(RADIX_1-1)))>>RADIX_1);
			dst[2*w] = CLIP((BIC_A3*src[0 ] + BIC_A2*src[w] + BIC_A1*src[2*w] + BIC_A0*src[3*w] + (1<<(RADIX_1-1)))>>RADIX_1);
			dst++; src++;
		}
		dst+=2*w; src+=w;
	}

	for (j = 0; j<w; j++)
	{
		dst[0  ] = src[0];
		dst[w  ] = CLIP((BIC_A0*src[-w] + BIC_A1*src[0] + BIC_A2*src[w] + BIC_A3*src[w] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst[2*w] = CLIP((BIC_A3*src[0 ] + BIC_A2*src[w] + BIC_A1*src[w] + BIC_A0*src[w] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst++; src++;
	}
}

/**
 ******************************************************************************
 * Bicubic interpolation by factor 1.5 of whole image.
 *
 * \note
 *
 * \param src				[in]  - source image;
 * \param dst				[out] - destination image;
 * \param width				[in]  - width  of source image;
 * \param height			[in]  - height of source image;
 *
 *  \date    25-11-2002		Created
 ******************************************************************************
 */
bool CVSVideoProc::ResampleUp_1d5(BYTE* src, BYTE* dst,  int width, int height)
{
	BYTE *temp = new BYTE[width*height*3/2];

	InterpolateVer1_5(src, temp, width, height, true);
	InterpolateHor1_5(temp, dst, width, height*3/2, true);
	src+=width*height; dst+=width*height*9/4;
	width/=2; height/=2;

	InterpolateVer1_5(src, temp, width, height, false);
	InterpolateHor1_5(temp, dst, width, height*3/2, false);
	src+=width*height; dst+=width*height*9/4;

	InterpolateVer1_5(src, temp, width, height, false);
	InterpolateHor1_5(temp, dst, width, height*3/2, false);

	delete[] temp;
	return true;
}


/******************************************************************************
 * Bicubic interpolation by factor 2 of whole image.
 *
 * \param src				[in]  - source image;
 * \param dst				[out] - destination image;
 * \param width				[in]  - width  of source image;
 * \param height			[in]  - height of source image;
 *
 *  \date    12-02-2004		Created
 ******************************************************************************/
bool CVSVideoProc::ResampleUp_2(BYTE* src, BYTE* dst,  int width, int height)
{
	BYTE *temp = new BYTE[width*height*2];

	InterpolateVer2(src, temp, width, height, true);
	InterpolateHor2(temp, dst, width, height*2, true);
	src+=width*height; dst+=width*height*4;
	width/=2; height/=2;

	InterpolateVer2(src, temp, width, height, false);
	InterpolateHor2(temp, dst, width, height*2, false);
	src+=width*height; dst+=width*height*4;

	InterpolateVer2(src, temp, width, height, false);
	InterpolateHor2(temp, dst, width, height*2, false);

	delete[] temp;
	return true;
}


/******************************************************************************
 * Bicubic interpolation by factor 2 of the image in hor dim. C version.
 *
 * \param src				[in]  - pointer to row of source image component;
 * \param dst				[out] - pointer to interpolated row of destination image component;
 * \param w					[in]  - width  of source image;
 * \param h					[in]  - height of source image;
 * \param Luma				[in]  - true if luma component;
 *
 *  \date    12-02-2004		Created
 ******************************************************************************/
void CVSVideoProc::InterpolateHor2(BYTE *src, BYTE *dst, int w, int h, bool Luma) // real param
{
	int i, j;
	int BIC_A0, BIC_A1;
	if (Luma) {
		BIC_A0 = UP2_LA0;
		BIC_A1 = UP2_LA1;
	}
	else {
		BIC_A0 = UP2_CA0;
		BIC_A1 = UP2_CA1;
	}

	for (j = 0; j<h; j++) {
		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[0] + BIC_A1*src[0] + BIC_A1*src[1] + BIC_A0*src[2] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst+=2; src++;

		for (i = 0; i<(w-3); i++) {
			dst[0] = src[0];
			dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A1*src[1] + BIC_A0*src[2] + (1<<(RADIX_1-1)))>>RADIX_1);
			dst+=2; src++;
		}

		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A1*src[1] + BIC_A0*src[1] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst+=2; src++;

		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A1*src[0] + BIC_A0*src[0] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst+=2; src++;
	}
}


/*******************************************************************************
 * Bicubic interpolation by factor 2 of the image in ver dim. C version.
 *
 * \param src				[in]  - pointer to row of source image component;
 * \param dst				[out] - pointer to interpolated row of destination image component;
 * \param w					[in]  - width  of source image;
 * \param h					[in]  - height of source image;
 * \param Luma				[in]  - true if luma component;
 *
 *  \date    12-02-2004		Created
 ******************************************************************************/
void CVSVideoProc::InterpolateVer2(BYTE *src, BYTE *dst, int w, int h, bool Luma)
{
	int i, j;
	int BIC_A0, BIC_A1;
	if (Luma) {
		BIC_A0 = UP2_LA0;
		BIC_A1 = UP2_LA1;
	}
	else {
		BIC_A0 = UP2_CA0;
		BIC_A1 = UP2_CA1;
	}

	for (j = 0; j<w; j++) {
		dst[0  ] = src[0];
		dst[w  ] = CLIP((BIC_A0*src[0] + BIC_A1*src[0] + BIC_A1*src[w  ] + BIC_A0*src[2*w] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst++; src++;
	}
	dst+=w; src+=0;

	for (i = 0; i<(h-3); i++) {
		for (j = 0; j<w; j++) {
			dst[0  ] = src[0];
			dst[w  ] = CLIP((BIC_A0*src[-w] + BIC_A1*src[0] + BIC_A1*src[w  ] + BIC_A0*src[2*w] + (1<<(RADIX_1-1)))>>RADIX_1);
			dst++; src++;
		}
		dst+=w; src+=0;
	}

	for (j = 0; j<w; j++) {
		dst[0  ] = src[0];
		dst[w  ] = CLIP((BIC_A0*src[-w] + BIC_A1*src[0] + BIC_A1*src[w] + BIC_A0*src[w] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst++; src++;
	}
	dst+=w; src+=0;

	for (j = 0; j<w; j++) {
		dst[0  ] = src[0];
		dst[w  ] = CLIP((BIC_A0*src[-w] + BIC_A1*src[0] + BIC_A1*src[0] + BIC_A0*src[0] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst++; src++;
	}
}

/*******************************************************************************
 * Bilinear free interpolation of I420 image.
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 *
 *  \date    16-03-2004		Created
 ******************************************************************************/
bool CVSVideoProc::ResampleI420(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH)
{
	bool ret = true;
	// Y
	ret&= ResizeBilinear(src, dst, srcW, srcH, dstW, dstH, dstW, 0, 0);
	if (!ret) return ret;
	// U
	src+= srcW*srcH;
	dst+= dstW*dstH;
	srcW/=2; srcH/=2; dstW/=2; dstH/=2;
	ret&= ResizeBilinear(src, dst, srcW, srcH, dstW, dstH, dstW, 0, 1);
	if (!ret) return ret;
	// V
	src+= srcW*srcH;
	dst+= dstW*dstH;
	ret&= ResizeBilinear(src, dst, srcW, srcH, dstW, dstH, dstW, 0, 1);
	return ret;
}

/*******************************************************************************
 * Bicubic downsampling (8 times) of RGB8.
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 *
 *  \date    16-03-2004		Created
 ******************************************************************************/
bool CVSVideoProc::ResampleDown_8d1(BYTE* src, BYTE *dst, int srcW, int srcH)
{
	bool ret = true;
	ret &= ResizeBilinear(src, dst, srcW, srcH, srcW / 8, srcH / 8, srcW / 8, 4, 0);
	return ret;
}

/*******************************************************************************
 * Bicubic resample with crop, slow dummy for MMX or C
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 *
 *  \date    21-10-2011		Created
 ******************************************************************************/
bool CVSVideoProc::ResampleCropI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW, int dstH, int dstStep,
								    int srcWR, int srcHR, int srcOffsetW, int srcOffsetH, double factorW, double factorH, int mode)
{
	int j;
	unsigned char *pTmp = (unsigned char*)malloc(srcWR*srcHR*3/2);
	unsigned char *pY, *pU, *pV;
	unsigned char *pPlane = pTmp;
	/// copy Y
	pY = pSrc[0] + srcOffsetW + srcOffsetH * srcW;
	for (j = 0; j < srcHR; j++) {
		memcpy(pPlane, pY, srcWR);
		pPlane += srcWR;
		pY += srcW;
	}
	/// copy U
	pU = pSrc[1] + srcOffsetW / 2 + srcOffsetH * srcW / 4;
	for (j = 0; j < srcHR / 2; j++) {
		memcpy(pPlane, pU, srcWR / 2);
		pPlane += srcWR / 2;
		pU += srcW / 2;
	}
	/// copy V
	pV = pSrc[2] + srcOffsetW / 2 + srcOffsetH * srcW / 4;
	for (j = 0; j < srcHR / 2; j++) {
		memcpy(pPlane, pV, srcWR / 2);
		pPlane += srcWR / 2;
		pV += srcW / 2;
	}
	/// Resample
	ResampleI420(pTmp, pDst[0], srcWR, srcHR, dstW, dstH);
	free(pTmp);
	return true;
}

/*******************************************************************************
 * Bicubic resample without crop, slow dummy for MMX or C
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 *
 *  \date    21-10-2011		Created
 ******************************************************************************/
bool CVSVideoProc::ResampleInscribedI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW, int dstH, int dstStep,
											 int srcOffsetW, int srcOffsetH, double factorW, double factorH, int mode)
{
	bool ret = true;

	memset(pDst[0], 0, dstStep * dstH);
	memset(pDst[1], 0x80, (dstStep >> 1) * (dstH >> 1));
	memset(pDst[2], 0x80, (dstStep >> 1) * (dstH >> 1));

	int newW = srcW * factorW;
	int newH = srcH * factorH;
	if (newW > dstW) newW = dstW;
	if (newH > dstH) newH = dstH;

	unsigned char *pOut[3];
	pOut[0] = pDst[0] + srcOffsetW + srcOffsetH * dstStep;
	pOut[1] = pDst[1] + (srcOffsetW >> 1) + (srcOffsetH >> 1) * (dstStep >> 1);
	pOut[2] = pDst[2] + (srcOffsetW >> 1) + (srcOffsetH >> 1) * (dstStep >> 1);
	// Y
	ret &= ResizeBilinear(pSrc[0], pOut[0], srcW, srcH, newW, newH, dstStep, 0, 0);
	if (!ret) return ret;
	// U
	ret &= ResizeBilinear(pSrc[1], pOut[1], srcW >> 1, srcH >> 1, newW >> 1, newH >> 1, dstStep >> 1, 0, 1);
	if (!ret) return ret;
	// V
	ret &= ResizeBilinear(pSrc[2], pOut[2], srcW >> 1, srcH >> 1, newW >> 1, newH >> 1, dstStep >> 1, 0, 1);

	return ret;
}

/*******************************************************************************
 * Bilinear free interpolation of RGB32 image.
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 * \param dstStep			[in]  - destination line length in bytes;
 *
 * \date    17-03-2004		Created
 ******************************************************************************/
bool CVSVideoProc::ResampleRGB32(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep)
{
	return ResizeBilinear(src, dst, srcW, srcH, dstW, dstH, dstStep, 2, 0);
}

/*******************************************************************************
 * Bilinear free interpolation of RGB24 image.
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 * \param dstStep			[in]  - destination line length in bytes;
 *
 * \date    17-03-2004		Created
 ******************************************************************************/
bool CVSVideoProc::ResampleRGB24(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep)
{
	return ResizeBilinear(src, dst, srcW, srcH, dstW, dstH, dstStep, 1, 0);
}

/*******************************************************************************
 * Bilinear free interpolation of RGB8 image.
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 * \param dstStep			[in]  - destination line length in bytes;
 *
 * \date    17-03-2004		Created
 ******************************************************************************/
bool CVSVideoProc::ResampleRGB8(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep)
{
	return ResizeBilinear(src, dst, srcW, srcH, dstW, dstH, dstStep, 0, 0);
}

bool CVSVideoProc::ResampleRGB565(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep)
{
	return ResizeBilinear(src, dst, srcW, srcH, dstW, dstH, dstStep, 3, 0);
}

/******************************************************************************
 * Make Dithering other the channel, depends from the Brightness
 * \return none
 *
 * \param Src				[IN, OUT]	- processing image
 * \param W, H				[IN]		- image width and heith
 * \param Bits				[IN]		- Dithering Depth
 *
 *  \date    23-03-2004
 ******************************************************************************/
bool CVSVideoProc::DitherI420(BYTE* Src, int W, int H, int Bits)
{
	static int cnt[8]={ 0,  1,  1,  0,  1,  1,  1,  0 };
	static int ThresholdBrightness[3] = {0, 448, 568};
	int cnt0;
	int cnt1;
	int cnt2;
	int cnt3;
	int cnt4;
	int cnt5;
	int cnt6;
	int cnt7;
	int i, j;
	BYTE *src;

	if (Bits>2) Bits = 2;
	if (Bits<0) Bits = 0;

	cnt0 = cnt[0];
	cnt1 = cnt[1];
	cnt2 = cnt[2];
	cnt3 = cnt[3];
	cnt4 = cnt[4];
	cnt5 = cnt[5];
	cnt6 = cnt[6];
	cnt7 = cnt[7];

	for (j = 0; j<H; j++) {
		src = Src+W*j;
		for (i = 0; i<W/8; i++) {
			int nmask0, mask0, nmask5, mask5;
			int	brightness;
			int bits, t;

			bits = Bits;
			brightness = (src[0]+src[1]+src[2]+src[3]+src[4]+src[5]+src[6]+src[7]);
			if (brightness<ThresholdBrightness[bits])
				bits--;

			nmask0  = 0xFF<<(bits);
			mask0   = ~nmask0;
			nmask5  =  nmask0<<1;
			mask5   = ~nmask5;

			t = CLIP(src[0] + cnt0); cnt0 = t&mask0; src[0] = t&nmask0;
			t = CLIP(src[1] + cnt1); cnt1 = t&mask5; src[1] = t&nmask5;
			t = CLIP(src[2] + cnt2); cnt2 = t&mask0; src[2] = t&nmask0;
			t = CLIP(src[3] + cnt3); cnt3 = t&mask5; src[3] = t&nmask5;
			t = CLIP(src[4] + cnt4); cnt4 = t&mask0; src[4] = t&nmask0;
			t = CLIP(src[5] + cnt5); cnt5 = t&mask5; src[5] = t&nmask5;
			t = CLIP(src[6] + cnt6); cnt6 = t&mask0; src[6] = t&nmask0;
			t = CLIP(src[7] + cnt7); cnt7 = t&mask5; src[7] = t&nmask5;

			src+=8;
		}
	}

	cnt[0] = cnt0;
	cnt[1] = cnt1;
	cnt[2] = cnt2;
	cnt[3] = cnt3;
	cnt[4] = cnt4;
	cnt[5] = cnt5;
	cnt[6] = cnt6;
	cnt[7] = cnt7;
	return true;
}

/*******************************************************************************
 * Bilinear free interpolation of 8, 24 and 32 bits per pixel image.
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 * \param dstStep			[in]  - destination line length in bytes;
 * \param mode				[in]  - type of image, 0 - 8bit, 1 - 24bit, 2 - 32bit, 3 - rgb565, 4 - bicubic 8 times minification;
 *
 * \return  false if input parametrs are out of range.
 *
 * \date    01-06-2005		Created
 ******************************************************************************/
bool CVSVideoProc::ResizeBilinear(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep, int mode, int isUVPlane)
{
	if (!src || !dst || srcW<9 || srcH<9 || dstW<9 || dstH <9 ||srcW>16000 || srcH>16000 || dstW>16000 || dstH>16000)
		return false;
	if (m_resize_st[isUVPlane]->srcW != srcW || m_resize_st[isUVPlane]->srcH != srcH || m_resize_st[isUVPlane]->dstW != dstW || m_resize_st[isUVPlane]->dstH != dstH) {
		if (isUVPlane == 0)
			InitResize(srcW, srcH, dstW, dstH);
		else
			InitResize(srcW * 2, srcH * 2, dstW, dstH);
	}
	if (mode==0) {
		if (dstStep < dstW)
			return false;
		if (dstW < srcW)
			bic_resize_x1(src, m_tbuff, srcW, srcH, dstW, m_resize_st[isUVPlane]->bicubic_cf_x, m_resize_st[isUVPlane]->bicubic_cf_len_x);
		else
			b_resize_x1(src, m_tbuff, srcW, srcH, dstW);
		if (dstH < srcH)
			bic_resize_y1(m_tbuff, dst, dstW, srcH, dstH, dstStep, m_resize_st[isUVPlane]->bicubic_cf_y, m_resize_st[isUVPlane]->bicubic_cf_len_y);
		else
			b_resize_y1(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==1) {
		if (dstStep < dstW*3)
			return false;
		if (dstW < srcW)
			bic_resize_x3(src, m_tbuff, srcW, srcH, dstW, m_resize_st[isUVPlane]->bicubic_cf_x, m_resize_st[isUVPlane]->bicubic_cf_len_x);
		else
			b_resize_x3(src, m_tbuff, srcW, srcH, dstW);
		if (dstH < srcH)
			bic_resize_y3(m_tbuff, dst, dstW, srcH, dstH, dstStep, m_resize_st[isUVPlane]->bicubic_cf_y, m_resize_st[isUVPlane]->bicubic_cf_len_y);
		else
			b_resize_y3(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==2) {
		if (dstStep < dstW*4)
			return false;
		if (dstW < srcW)
			bic_resize_x3_4(src, m_tbuff, srcW, srcH, dstW, m_resize_st[isUVPlane]->bicubic_cf_x, m_resize_st[isUVPlane]->bicubic_cf_len_x);
		else
			b_resize_x3_4(src, m_tbuff, srcW, srcH, dstW);
		if (dstH < srcH)
			bic_resize_y3_4(m_tbuff, dst, dstW, srcH, dstH, dstStep, m_resize_st[isUVPlane]->bicubic_cf_y, m_resize_st[isUVPlane]->bicubic_cf_len_y);
		else
			b_resize_y3_4(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==3) {
		if (dstStep < dstW*2)
			return false;
		if (dstW<srcW)
			bic_resize_x_565(src, m_tbuff, srcW, srcH, dstW);
		else
			b_resize_x_565(src, m_tbuff, srcW, srcH, dstW);
		if (dstH<srcH)
			bic_resize_y_565(m_tbuff, dst, dstW, srcH, dstH, dstStep);
		else
			b_resize_y_565(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==4) {
		if (dstStep < dstW)
			return false;
		bic_resize_8_x1(src, m_tbuff, srcW, srcH);
		bic_resize_8_y1(m_tbuff, dst, dstW, srcH, dstStep);
	} else
		return false;
	return true;
}

#define USE_IPP_WRAP

VS_VideoProc::VS_VideoProc(bool NotUseMMX)
{
	cpu = VS_GetCPUType();
#ifndef USE_IPP_WRAP
	if (!NotUseMMX) {
		if (cpu&VS_CPU_SSE2)
			vp = new CVSVideoProc_SSE2;
		else if (cpu&VS_CPU_MMX)
			vp = new CVSVideoProc_MMX;
		else
			vp = new CVSVideoProc;
	}
	else
		vp = new CVSVideoProc;
#else
	if (cpu & VS_CPU_SSE2)
		vp = new CVSVideoProc_IPP;
	else if (cpu & VS_CPU_MMX)
		vp = new CVSVideoProc_MMX;
	else
		vp = new CVSVideoProc;
#endif
}

VS_VideoProc::~VS_VideoProc()
{
	delete vp;
}

int VS_VideoProc::Cpu()
{
	return cpu;
}

bool VS_VideoProc::ConvertI420ToYUY2(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW)
{
	return vp->ConvertI420ToYUY2(srcY, srcU, srcV, dst, srcW, h, dstW);
}

bool VS_VideoProc::ConvertI420ToUYVY(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW)
{
	return vp->ConvertI420ToUYVY(srcY, srcU, srcV, dst, srcW, h, dstW);
}

bool VS_VideoProc::ConvertI420ToBMF16(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW)
{
	return vp->ConvertI420ToBMF16(srcY, srcU, srcV, dst, srcW, h, dstW);
}

bool VS_VideoProc::ConvertI420ToBMF24(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW)
{
	return vp->ConvertI420ToBMF24(srcY, srcU, srcV, dst, srcW, h, dstW);
}

bool VS_VideoProc::ConvertI420ToBMF32(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW)
{
	return vp->ConvertI420ToBMF32(srcY, srcU, srcV, dst, srcW, h, dstW);
}

bool VS_VideoProc::ConvertI420ToYUV444(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW)
{
	return vp->ConvertI420ToYUV444(srcY, srcU, srcV, dst, srcW, h, dstW);
}

bool VS_VideoProc::ConvertI420ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW)
{
	return vp->ConvertI420ToBMF32_Vflip(srcY, srcU, srcV, dst, srcW, h, dstW);
}

bool VS_VideoProc::ConvertYUV444ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV, BYTE* dst, int srcW, int h, int dstW)
{
	return vp->ConvertYUV444ToBMF32_Vflip(srcY, srcU, srcV, dst, srcW, h, dstW);
}

bool VS_VideoProc::ConvertYV12ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW)
{
	return vp->ConvertYV12ToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool VS_VideoProc::ConvertYUY2ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW)
{
	return vp->ConvertYUY2ToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool VS_VideoProc::ConvertUYVYToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW)
{
	return vp->ConvertUYVYToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool VS_VideoProc::ConvertBMF24ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW)
{
	return vp->ConvertBMF24ToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool VS_VideoProc::ConvertBMF32ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW)
{
	return vp->ConvertBMF32ToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool VS_VideoProc::ConvertRGB24ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW)
{
	return vp->ConvertRGB24ToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool VS_VideoProc::ConvertI42SToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW)
{
	return vp->ConvertI42SToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool VS_VideoProc::ConvertNV12ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE* dstV, int srcW, int h, int dstW)
{
	return vp->ConvertNV12ToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool VS_VideoProc::ResampleI420(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH)
{
	return vp->ResampleI420(src, dst, srcW, srcH, dstW, dstH);
}

bool VS_VideoProc::ResampleRGB32(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int dstStep)
{
	return vp->ResampleRGB32(src, dst, srcW, srcH, dstW, dstH, dstStep);
}

bool VS_VideoProc::ResampleRGB24(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int dstStep)
{
	return vp->ResampleRGB24(src, dst, srcW, srcH, dstW, dstH, dstStep);
}

bool VS_VideoProc::ResampleRGB8(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int dstStep)
{
	return vp->ResampleRGB8(src, dst, srcW, srcH, dstW, dstH, dstStep);
}

bool VS_VideoProc::ResampleRGB565(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int dstStep)
{
	return vp->ResampleRGB565(src, dst, srcW, srcH, dstW, dstH, dstStep);
}

bool VS_VideoProc::ResampleUp_2(BYTE* src, BYTE* dst, int width, int height)
{
	return vp->ResampleUp_2(src, dst, width, height);
}

bool VS_VideoProc::ResampleUp_1d5(BYTE* src, BYTE* dst, int width, int height)
{
	return vp->ResampleUp_1d5(src, dst, width, height);
}

bool VS_VideoProc::ResampleDown_8d1(BYTE* src, BYTE* dst, int width, int height)
{
	return vp->ResampleDown_8d1(src, dst, width, height);
}

bool VS_VideoProc::ResampleCropI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW, int dstH,
	int dstStep, int srcWR, int srcHR, int srcOffsetW, int srcOffsetH, double factorW,
	double factorH, int mode)
{
	return vp->ResampleCropI420(pSrc, pDst, srcW, srcH, srcStep, dstW, dstH, dstStep, srcWR, srcHR, srcOffsetW,
		srcOffsetH, factorW, factorH, mode);
}

bool VS_VideoProc::ResampleInscribedI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW,
	int dstH, int dstStep, int srcOffsetW, int srcOffsetH, double factorW,
	double factorH, int mode)
{
	return vp->ResampleInscribedI420(pSrc, pDst, srcW, srcH, srcStep, dstW, dstH, dstStep, srcOffsetW, srcOffsetH,
		factorW, factorH, mode);
}

bool VS_VideoProc::SaturateI420(BYTE* src, int srcWidth, int srcHeight, DWORD dwSaturation)
{
	return vp->SaturateI420(src, srcWidth, srcHeight, dwSaturation);
}

bool VS_VideoProc::ClipI420(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int mode)
{
	return vp->ClipI420(src, dst, srcW, srcH, dstW, dstH, mode);
}

bool VS_VideoProc::ExpandRGB24(BYTE* src, BYTE* dst, int srcW, int srcH, int dstW, int dstH, int mode)
{
	return vp->ExpandRGB24(src, dst, srcW, srcH, dstW, dstH, mode);
}

bool VS_VideoProc::DitherI420(BYTE* src, int srcW, int srcH, int Bits)
{
	return vp->DitherI420(src, srcW, srcH, Bits);
}

bool VS_VideoProc::MirrorVertical(BYTE* src, BYTE* dst, int srcW, int srcH)
{
	return vp->MirrorI420(src, dst, srcW, srcH, 1);
}
