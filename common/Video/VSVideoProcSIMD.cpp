/****************************************************************************
 * (c) 2002 Visicron Inc.  http://www.visicron.net/
 *
 * Project: VSVideo processing
 *
 * $Revision: 12 $
 * $History: VSVideoProcSIMD.cpp $
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 30.05.12   Time: 19:35
 * Updated in $/VSNA/Video
 * - fix preprocessing modules on IPP for non SSE2 cpu
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 12.04.12   Time: 17:08
 * Updated in $/VSNA/Video
 * - add ipp nv12 -> i420 csc
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 19.03.12   Time: 18:47
 * Updated in $/VSNA/Video
 * - improve csc input format (ipp wrap)
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 17.12.10   Time: 20:01
 * Updated in $/VSNA/Video
 * - fix av in sse2 resampling x1.5
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 30.11.10   Time: 17:48
 * Updated in $/VSNA/Video
 * - SSE2 ench VideoProc
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 29.11.10   Time: 11:16
 * Updated in $/VSNA/Video
 * - VideoProc: SSE2 improve
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 8.11.10    Time: 19:05
 * Updated in $/VSNA/Video
 * - SSE2 optimization VideoProc
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 22.10.10   Time: 17:43
 * Updated in $/VSNA/Video
 * - optimisation deinterlacing (sse2 bicubic minification)
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 22.10.10   Time: 16:19
 * Updated in $/VSNA/Video
 * - optimization resampling functions
 * - optimization deinterlacing
 * - clean VideoProc class
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
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 16.10.06   Time: 15:56
 * Updated in $/VS/Video
 * - fixed bug with non 8 pixel-aligned color conversion (now 2
 * pixel-aligned)
 *
 * *****************  Version 17  *****************
 * User: Smirnov      Date: 31.07.06   Time: 20:51
 * Updated in $/VS/Video
 * - added SSE2 videoproc class (turned off now)
 * - align impruvements for 8 bit resampling MMX methods
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 27.07.06   Time: 13:44
 * Updated in $/VS/Video
 * - added HQ Resize
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 14.10.05   Time: 17:28
 * Updated in $/VS/Video
 * - coversion to 16 bit rgb
 *
 * *****************  Version 14  *****************
 * User: Sanufriev    Date: 23.09.05   Time: 16:13
 * Updated in $/VS/Video
 * - added 8-bit image processing, C & MMX version
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 4.07.05    Time: 16:04
 * Updated in $/VS/Video
 * - added 32-bit image processing, C & MMX version
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 27.06.05   Time: 18:51
 * Updated in $/VS/Video
 * bicubic downsampling, MMX version
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 22.06.05   Time: 20:13
 * Updated in $/VS/Video
 * - new files in Video project
 * - bicubic minification embeedded in videoproc class
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 1.06.05    Time: 21:03
 * Updated in $/VS/Video
 * bilinear scaling integration in Video project
 *
 * *****************  Version 9  *****************
 * User: Smirnov      Date: 23.03.04   Time: 15:54
 * Updated in $/VS/Video
 * added dithering
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 24.02.04   Time: 13:56
 * Updated in $/VS/Video
 * added no Use MMX
 * Intrpolation Up2 times n MMX
 *
 * *****************  Version 7  *****************
 * User: Smirnov      Date: 11.04.03   Time: 19:40
 * Updated in $/VS/Video
 * rewrited all rendering,
 * added support YUY2, UYVY
 * History: 25.11.02 Created
 *
 ****************************************************************************/
/**
 * \file VSVideoProcSIMD.cpp
 * \brief Contain color conversions and image resampling SIMD methods
 */

#include "VSVideoDefines.h"
#include "VSVideoProc.h"
#include "bilinear.h"
#include "bicubic.h"
#include <mmintrin.h>
#include <emmintrin.h>

extern BYTE cipping_table[];

#define DW_FILL_QW(x) _mm_set1_pi32(x)
#define W_FILL_QW(x)  _mm_set1_pi16(x)
#define DW_TO_QW(a, b) _mm_set_pi32((a),(b))
#define W_TO_QW(a, b, c, d) _mm_set_pi16((a),(b),(c),(d))

bool CVSVideoProc_MMX::InitResize(int w, int h, int new_w, int new_h)
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

		m_resize_st[k]->x_add_reg = (unsigned short*)_aligned_malloc(4*sizeof(unsigned short), 16);
		m_resize_st[k]->y_add_reg = (unsigned short*)_aligned_malloc(4*sizeof(unsigned short), 16);
		m_resize_st[k]->dirx_cf_reg = (unsigned short*)_aligned_malloc(4*new_w*sizeof(unsigned short), 16);
		m_resize_st[k]->invx_cf_reg = (unsigned short*)_aligned_malloc(4*new_w*sizeof(unsigned short), 16);
		m_resize_st[k]->diry_cf_reg = (unsigned short*)_aligned_malloc(4*new_h*sizeof(unsigned short), 16);
		m_resize_st[k]->invy_cf_reg = (unsigned short*)_aligned_malloc(4*new_h*sizeof(unsigned short), 16);

		/// x dir bilinear coef
		for (i = 0; i < new_w; i++) {
			_mm_empty();
			x = factor * i;
			m_resize_st[k]->x_pos_8bit[i] = (int)(factor * i);
			m_resize_st[k]->x_pos_24bit[i] = (int)(factor * i) * 3;
			m_resize_st[k]->x_pos_32bit[i] = (int)(factor * i) * 4;
			x -= (double)(int)x;
			m_resize_st[k]->dirx_cf[i] = (int)((1 - x) * (1 << 8) + 0.5);
			m_resize_st[k]->invx_cf[i] = (1 << 8) - m_resize_st[k]->dirx_cf[i];
			*(__m64*)(m_resize_st[k]->dirx_cf_reg + 4 * i) = _mm_set1_pi16(m_resize_st[k]->dirx_cf[i]);
			*(__m64*)(m_resize_st[k]->invx_cf_reg + 4 * i) = _mm_set1_pi16(m_resize_st[k]->invx_cf[i]);
			_mm_empty();
		}
		_mm_empty();
		/// y dir bilinear coef
		factor = (double)(h - 1) / (new_h - 1);
		for (j = 1; j < new_h - 1; j++) {
			x = factor * j;
			m_resize_st[k]->y_pos[j] = (int)(x);
			x -= (double)(int)x;
			m_resize_st[k]->diry_cf[j] = (int)((1 - x) * (1 << 8) + 0.5);
			m_resize_st[k]->invy_cf[j] = (1 << 8) - m_resize_st[k]->diry_cf[j];
			*(__m64*)(m_resize_st[k]->diry_cf_reg + 4 * j) = _mm_set1_pi16(m_resize_st[k]->diry_cf[j]);
			*(__m64*)(m_resize_st[k]->invy_cf_reg + 4 * j) = _mm_set1_pi16(m_resize_st[k]->invy_cf[j]);
			_mm_empty();
		}
		*(__m64*)m_resize_st[k]->x_add_reg = _mm_set1_pi16((1 << 8)/2);
		*(__m64*)m_resize_st[k]->y_add_reg = _mm_set1_pi16((1 << 8)/2);
		_mm_empty();
		*/
		/// bicubic coef
		m_resize_st[k]->bicubic_cf_len_x_8bit = genTableAlloc(w, new_w, m_resize_st[k]->bicubic_cf_x_8bit);
		m_resize_st[k]->bicubic_cf_len_x = genTableAlloc_mmx(w, new_w, m_resize_st[k]->bicubic_cf_x);
		m_resize_st[k]->bicubic_cf_len_y = genTableAlloc_mmx(h, new_h, m_resize_st[k]->bicubic_cf_y);

		_mm_empty();

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

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to YUY2 format. MMX version.
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
bool CVSVideoProc_MMX::ConvertI420ToYUY2(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									 BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;
	__m64 my0, my1, mu, mv;

	for(int i = 0; i<height; i++) {
		int W1 = (srcWidth&~7)>>1;
		int j=0;
		for(; j<W1; j+=4) {
			mu = _m_from_int(*(int*)(srcU + j));
			mv = _m_from_int(*(int*)(srcV + j));
			mu = _m_punpcklbw(mu, mv);

			my0 = (*(__m64*)(srcY + j*2 + 0));
			my1 = my0;
			my0 = _m_punpcklbw(my0, mu);
			my1 = _m_punpckhbw(my1, mu);
			*(__m64*)(dst+j*4 + 0) = my0;
			*(__m64*)(dst+j*4 + 8) = my1;
		}
		for(; j<(srcWidth>>1); j++) {
			dst[j*4 + 0] = srcY[j*2 + 0];
			dst[j*4 + 1] = srcU[j   + 0];
			dst[j*4 + 2] = srcY[j*2 + 1];
			dst[j*4 + 3] = srcV[j   + 0];
		}

		srcY+=srcWidth;
		dst+= dstWidth;
		if(i&1)	srcU+= srcWidth/2, srcV+= srcWidth/2;
	}
	_m_empty();
	return true;
}

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to UYVY format. MMX version.
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
bool CVSVideoProc_MMX::ConvertI420ToUYVY(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									 BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;
	__m64 my, mu, mv;

	for(int i = 0; i<height; i++) {
		int W1 = (srcWidth&~7)>>1;
		int j=0;
		for(; j<W1; j+=4) {
			mu = _m_from_int(*(int*)(srcU + j));
			mv = _m_from_int(*(int*)(srcV + j));
			my = (*(__m64*)(srcY + j*2 + 0));
			mu = _m_punpcklbw(mu, mv);
			mv = mu;

			mu = _m_punpcklbw(mu, my);
			mv = _m_punpckhbw(mv, my);
			*(__m64*)(dst+j*4 + 0) = mu;
			*(__m64*)(dst+j*4 + 8) = mv;
		}
		for(; j<(srcWidth>>1); j++)	{
			dst[j*4 + 0] = srcU[j   + 0];
			dst[j*4 + 1] = srcY[j*2 + 0];
			dst[j*4 + 2] = srcV[j   + 0];
			dst[j*4 + 3] = srcY[j*2 + 1];
		}

		srcY+=srcWidth;
		dst+= dstWidth;
		if(i&1)	srcU+= srcWidth/2, srcV+= srcWidth/2;
	}
	_m_empty();
	return true;
}


/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to bitmap-24 format. MMX version.
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
bool CVSVideoProc_MMX::ConvertI420ToBMF24(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									  BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;
	int i ,j;
	__m64 m0, m1, m2, m3, m4, m5, m6, m7;
    const __m64 Mask_mm0	= DW_FILL_QW(0xFF00FF00),
    			Mask_mm1	= DW_FILL_QW(0x00FF00FF),
    			Mask_mm2	= DW_FILL_QW(0x0000FFFF),
    			Mask_016	= DW_FILL_QW(0x10101010),
    			Mask_128	= DW_FILL_QW(0x00800080),
    			mv2r		= W_FILL_QW(V2R>>2),
    			mu2g		= W_FILL_QW(U2G>>2),
    			mv2g		= W_FILL_QW(V2G>>2),
    			my2rgb		= W_FILL_QW(Y2RGB>>2),
    			maddrgb		= W_FILL_QW(ADD_RGB>>2);
//  const __m64 mu2b		= {U2B, U2B, U2B, U2B}; // we use shift <<7;
	const int mu2b = 7;

	srcY+=srcWidth*(height-1);
	srcU+=srcWidth/2*(height/2-1);
	srcV+=srcWidth/2*(height/2-1);

	for (i=0; i<height; i++) {
		int W1 = (srcWidth&~7)>>1;
		for (j=0; j<W1; j+=4) {
			m7 = _mm_setzero_si64();
			m0 = _m_from_int(*(int*)(srcU+j));
			m1 = _m_from_int(*(int*)(srcV+j));
			m0 = _m_punpcklbw(m0, m7);
			m1 = _m_punpcklbw(m1, m7);
			m0 = _m_psubsw(m0, Mask_128);
			m1 = _m_psubsw(m1, Mask_128);
			m2 = m0;
			m3 = m1;

			m0 = _m_pmullw(m0, mu2g);
			m2 = _m_psllwi (m2, 7); //b
			m1 = _m_pmullw(m1, mv2r); //r
			m3 = _m_pmullw(m3, mv2g);
			m0 = _m_paddsw(m0, m3);   //g

			m4 = *(__m64*)(srcY+2*j);
			m4 = _m_psubusb(m4, Mask_016);// unsigned
			m5 = m4;
			m4 = _m_punpcklbw(m4, m7);
			m5 = _m_punpckhbw(m5, m7);

			m4 = _m_pmullw(m4, my2rgb);
			m5 = _m_pmullw(m5, my2rgb);
			m4 = _m_paddsw(m4, maddrgb);
			m5 = _m_paddsw(m5, maddrgb);

			m3 = m0;
			m0 = _m_punpcklwd(m0, m0);
			m3 = _m_punpckhwd(m3, m3);
			m0 = _m_paddsw(m0, m4);
			m3 = _m_paddsw(m3, m5);
			m0 = _m_psrawi(m0, 6);
			m3 = _m_psrawi(m3, 6);
			m0 = _m_packuswb(m0, m3); //g

			m3 = m1;
			m1 = _m_punpcklwd(m1, m1);
			m3 = _m_punpckhwd(m3, m3);
			m1 = _m_paddsw(m1, m4);
			m3 = _m_paddsw(m3, m5);
			m1 = _m_psrawi(m1, 6);
			m3 = _m_psrawi(m3, 6);
			m1 = _m_packuswb(m1, m3); //r

			m3 = m2;
			m2 = _m_punpcklwd(m2, m2);
			m3 = _m_punpckhwd(m3, m3);
			m2 = _m_paddsw(m2, m4);
			m3 = _m_paddsw(m3, m5);
			m2 = _m_psrawi(m2, 6);
			m3 = _m_psrawi(m3, 6);
			m2 = _m_packuswb(m2, m3); //b

			m5 = m0;
			m3 = m2;
			m4 = m1;
			m0 = _m_pand(m0, Mask_mm0);
			m2 = _m_pand(m2, Mask_mm1);
			m2 = _m_por(m2, m0);

			m5 = _m_pand(m5, Mask_mm1);
			m4 = _m_pand(m4, Mask_mm0);
			m5 = _m_por(m5, m4);

			m7 = m2;
			m2 = _m_punpcklbw(m2, m5);
			m7 = _m_punpckhbw(m7, m5);
			m6 = m1;
			m0 = m2;
			m3 = _m_psrlqi(m3, 8);
			m1 = _m_punpcklbw(m1, m3);
			m6 = _m_punpckhbw(m6, m3);
			m6 = _m_pand(m6, Mask_mm2);
			m1 = _m_pand(m1, Mask_mm2);
			m0 = _m_punpcklwd(m0, m1);
			m2 = _m_punpckhwd(m2, m1);
			m5 = m2;
			m5 = _m_psllqi(m5, 48);
			m0 = _m_por(m0, m5);
			m2 = _m_psrlqi(m2, 16);
			m1 = m7;
			m1 = _m_punpcklwd(m1, m6);
			m2 = _m_punpckldq(m2, m1);
			m7 = _m_punpckhwd(m7, m6);
			m7 = _m_psllqi(m7, 16);
			m1 = _m_psrlqi(m1, 32);
			m1 = _m_por(m1, m7);

			*(__m64*)(dst +  0) = m0;
			*(__m64*)(dst +  8) = m2;
			*(__m64*)(dst + 16) = m1;

			dst+=24;
		}
		for(; j<(srcWidth>>1); j++)	{
			int u, v, y;
			int rr, gg, bb, yy;
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j*2+0]-16;
			makeRGB(y, rr, gg, bb, dst[2], dst[1], dst[0], yy);

			y = srcY[j*2+1]-16;
			makeRGB(y, rr, gg, bb, dst[5], dst[4], dst[3], yy);
			dst+= 6;
		}
		dst+= (dstWidth-3*srcWidth);
		srcY-= srcWidth;
		srcU-= srcWidth/2*(i&1);
		srcV-= srcWidth/2*(i&1);
	}
	_mm_empty();
	return true;
}


/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to bitmap-32 format. MMX version.
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
 *  \date    26-11-2002		Created by SMirovK
 ******************************************************************************
 */
bool CVSVideoProc_MMX::ConvertI420ToBMF32(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									  BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;
	int i ,j;
	__m64 m0, m1, m2, m3, m4, m5, m7;
    const __m64 Mask_mm0	= DW_FILL_QW(0xFF00FF00),
    			Mask_mm1	= DW_FILL_QW(0x00FF00FF),
    			Mask_mm2	= DW_FILL_QW(0x0000FFFF),
    			Mask_016	= DW_FILL_QW(0x10101010),
    			Mask_128	= DW_FILL_QW(0x00800080),
    			mv2r		= W_FILL_QW(V2R>>2),
    			mu2g		= W_FILL_QW(U2G>>2),
    			mv2g		= W_FILL_QW(V2G>>2),
    			my2rgb		= W_FILL_QW(Y2RGB>>2),
    			maddrgb		= W_FILL_QW(ADD_RGB>>2);
//  const __m64 mu2b		= {U2B, U2B, U2B, U2B}; // we use shift <<7;
	const int mu2b = 7;

	srcY+=srcWidth*(height-1);
	srcU+=srcWidth/2*(height/2-1);
	srcV+=srcWidth/2*(height/2-1);

	m7 = _mm_setzero_si64();
	for (i=0; i<height; i++) {
		int W1 = (srcWidth&~7)>>1;
		for (j=0; j<W1; j+=4) {
			m0 = _m_from_int(*(int*)(srcU+j));
			m1 = _m_from_int(*(int*)(srcV+j));
			m0 = _m_punpcklbw(m0, m7);
			m1 = _m_punpcklbw(m1, m7);
			m0 = _m_psubsw(m0, Mask_128);
			m1 = _m_psubsw(m1, Mask_128);
			m2 = m0;
			m3 = m1;

			m0 = _m_pmullw(m0, mu2g);
			m2 = _m_psllwi (m2, 7); //b
			m1 = _m_pmullw(m1, mv2r); //r
			m3 = _m_pmullw(m3, mv2g);
			m0 = _m_paddsw(m0, m3);   //g

			m4 = *(__m64*)(srcY+2*j);
			m4 = _m_psubusb(m4, Mask_016);// unsigned
			m5 = m4;
			m4 = _m_punpcklbw(m4, m7);
			m5 = _m_punpckhbw(m5, m7);

			m4 = _m_pmullw(m4, my2rgb);
			m5 = _m_pmullw(m5, my2rgb);
			m4 = _m_paddw(m4, maddrgb);
			m5 = _m_paddw(m5, maddrgb); //usigned

			m3 = m0;
			m0 = _m_punpcklwd(m0, m0);
			m3 = _m_punpckhwd(m3, m3);
			m0 = _m_paddsw(m0, m4);
			m3 = _m_paddsw(m3, m5);
			m0 = _m_psrawi(m0, 6);
			m3 = _m_psrawi(m3, 6);
			m0 = _m_packuswb(m0, m3); //g

			m3 = m1;
			m1 = _m_punpcklwd(m1, m1);
			m3 = _m_punpckhwd(m3, m3);
			m1 = _m_paddsw(m1, m4);
			m3 = _m_paddsw(m3, m5);
			m1 = _m_psrawi(m1, 6);
			m3 = _m_psrawi(m3, 6);
			m1 = _m_packuswb(m1, m3); //r

			m3 = m2;
			m2 = _m_punpcklwd(m2, m2);
			m3 = _m_punpckhwd(m3, m3);
			m2 = _m_paddsw(m2, m4);
			m3 = _m_paddsw(m3, m5);
			m2 = _m_psrawi(m2, 6);
			m3 = _m_psrawi(m3, 6);
			m2 = _m_packuswb(m2, m3); //b

			m3 = m2;
			m2 = _m_punpcklbw(m2, m0); //bgbgbgbg
			m3 = _m_punpckhbw(m3, m0);

			m4 = m1;
			m1 = _m_punpcklbw(m1, m1); //r0r0r0r0
			m4 = _m_punpckhbw(m4, m4);

			m0 = m2;
			m0 = _m_punpcklwd(m0, m1); //rgb0rgb0
			m2 = _m_punpckhwd(m2, m1);

			m1 = m3;
			m1 = _m_punpcklwd(m1, m4);
			m3 = _m_punpckhwd(m3, m4);

			*(__m64*)(dst +  0) = m0;
			*(__m64*)(dst +  8) = m2;
			*(__m64*)(dst + 16) = m1;
			*(__m64*)(dst + 24) = m3;

			dst+=32;
		}
		for(; j<(srcWidth>>1); j++)	{
			int u, v, y;
			int rr, gg, bb, yy;
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j*2+0]-16;
			makeRGB(y, rr, gg, bb, dst[2], dst[1], dst[0], yy);

			y = srcY[j*2+1]-16;
			makeRGB(y, rr, gg, bb, dst[6], dst[5], dst[4], yy);
			dst+= 8;
		}
		dst+= (dstWidth-4*srcWidth);
		srcY-= srcWidth;
		srcU-= srcWidth/2*(i&1);
		srcV-= srcWidth/2*(i&1);
	}
	_m_empty();
	return true;
}

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to bitmap-32 format. MMX version. Vertical flip
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
 *  \date    26-11-2002		Created by SMirovK
 ******************************************************************************
 */
bool CVSVideoProc_MMX::ConvertI420ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV,
												BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;
	int i ,j;
	__m64 m0, m1, m2, m3, m4, m5, m7;
    const __m64 Mask_mm0	= DW_FILL_QW(0xFF00FF00),
    			Mask_mm1	= DW_FILL_QW(0x00FF00FF),
    			Mask_mm2	= DW_FILL_QW(0x0000FFFF),
    			Mask_016	= DW_FILL_QW(0x10101010),
    			Mask_128	= DW_FILL_QW(0x00800080),
    			mv2r		= W_FILL_QW(V2R>>2),
    			mu2g		= W_FILL_QW(U2G>>2),
    			mv2g		= W_FILL_QW(V2G>>2),
    			my2rgb		= W_FILL_QW(Y2RGB>>2),
    			maddrgb		= W_FILL_QW(ADD_RGB>>2);
//  const __m64 mu2b		= {U2B, U2B, U2B, U2B}; // we use shift <<7;
	const int mu2b = 7;

	m7 = _mm_setzero_si64();
	for (i=0; i<height; i++) {
		int W1 = (srcWidth&~7)>>1;
		for (j=0; j<W1; j+=4) {
			m0 = _m_from_int(*(int*)(srcU+j));
			m1 = _m_from_int(*(int*)(srcV+j));
			m0 = _m_punpcklbw(m0, m7);
			m1 = _m_punpcklbw(m1, m7);
			m0 = _m_psubsw(m0, Mask_128);
			m1 = _m_psubsw(m1, Mask_128);
			m2 = m0;
			m3 = m1;

			m0 = _m_pmullw(m0, mu2g);
			m2 = _m_psllwi (m2, 7); //b
			m1 = _m_pmullw(m1, mv2r); //r
			m3 = _m_pmullw(m3, mv2g);
			m0 = _m_paddsw(m0, m3);   //g

			m4 = *(__m64*)(srcY+2*j);
			m4 = _m_psubusb(m4, Mask_016);// unsigned
			m5 = m4;
			m4 = _m_punpcklbw(m4, m7);
			m5 = _m_punpckhbw(m5, m7);

			m4 = _m_pmullw(m4, my2rgb);
			m5 = _m_pmullw(m5, my2rgb);
			m4 = _m_paddw(m4, maddrgb);
			m5 = _m_paddw(m5, maddrgb); //usigned

			m3 = m0;
			m0 = _m_punpcklwd(m0, m0);
			m3 = _m_punpckhwd(m3, m3);
			m0 = _m_paddsw(m0, m4);
			m3 = _m_paddsw(m3, m5);
			m0 = _m_psrawi(m0, 6);
			m3 = _m_psrawi(m3, 6);
			m0 = _m_packuswb(m0, m3); //g

			m3 = m1;
			m1 = _m_punpcklwd(m1, m1);
			m3 = _m_punpckhwd(m3, m3);
			m1 = _m_paddsw(m1, m4);
			m3 = _m_paddsw(m3, m5);
			m1 = _m_psrawi(m1, 6);
			m3 = _m_psrawi(m3, 6);
			m1 = _m_packuswb(m1, m3); //r

			m3 = m2;
			m2 = _m_punpcklwd(m2, m2);
			m3 = _m_punpckhwd(m3, m3);
			m2 = _m_paddsw(m2, m4);
			m3 = _m_paddsw(m3, m5);
			m2 = _m_psrawi(m2, 6);
			m3 = _m_psrawi(m3, 6);
			m2 = _m_packuswb(m2, m3); //b

			m3 = m2;
			m2 = _m_punpcklbw(m2, m0); //bgbgbgbg
			m3 = _m_punpckhbw(m3, m0);

			m4 = m1;
			m1 = _m_punpcklbw(m1, m1); //r0r0r0r0
			m4 = _m_punpckhbw(m4, m4);

			m0 = m2;
			m0 = _m_punpcklwd(m0, m1); //rgb0rgb0
			m2 = _m_punpckhwd(m2, m1);

			m1 = m3;
			m1 = _m_punpcklwd(m1, m4);
			m3 = _m_punpckhwd(m3, m4);

			*(__m64*)(dst +  0) = m0;
			*(__m64*)(dst +  8) = m2;
			*(__m64*)(dst + 16) = m1;
			*(__m64*)(dst + 24) = m3;

			dst+=32;
		}
		for(; j<(srcWidth>>1); j++)	{
			int u, v, y;
			int rr, gg, bb, yy;
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j*2+0]-16;
			makeRGB(y, rr, gg, bb, dst[2], dst[1], dst[0], yy);

			y = srcY[j*2+1]-16;
			makeRGB(y, rr, gg, bb, dst[6], dst[5], dst[4], yy);
			dst+= 8;
		}
		dst+= (dstWidth-4*srcWidth);
		srcY+= srcWidth;
		srcU+= srcWidth/2*(i&1);
		srcV+= srcWidth/2*(i&1);
	}
	_m_empty();
	return true;
}

bool CVSVideoProc_MMX::ConvertYUV444ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV,
												  BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;
	int i ,j;
	__m64 m0, m1, m2, m3, m4, m7;
    const __m64 Mask_mm0	= DW_FILL_QW(0xFF00FF00),
    			Mask_mm1	= DW_FILL_QW(0x00FF00FF),
    			Mask_mm2	= DW_FILL_QW(0x0000FFFF),
    			Mask_016	= DW_FILL_QW(0x10101010),
    			Mask_128	= DW_FILL_QW(0x00800080),
    			mv2r		= W_FILL_QW(V2R>>2),
    			mu2g		= W_FILL_QW(U2G>>2),
    			mv2g		= W_FILL_QW(V2G>>2),
    			my2rgb		= W_FILL_QW(Y2RGB>>2),
    			maddrgb		= W_FILL_QW(ADD_RGB>>2);
//  const __m64 mu2b		= {U2B, U2B, U2B, U2B}; // we use shift <<7;
	const int mu2b = 7;

	m7 = _mm_setzero_si64();
	for (i=0; i<height; i++) {
		int W1 = (srcWidth&~7);
		for (j=0; j<W1; j+=4) {
			m0 = _m_from_int(*(int*)(srcU+j));
			m1 = _m_from_int(*(int*)(srcV+j));
			m0 = _m_punpcklbw(m0, m7);
			m1 = _m_punpcklbw(m1, m7);
			m0 = _m_psubsw(m0, Mask_128);
			m1 = _m_psubsw(m1, Mask_128);
			m2 = m0;
			m3 = m1;

			m0 = _m_pmullw(m0, mu2g);
			m2 = _m_psllwi (m2, 7); //b
			m1 = _m_pmullw(m1, mv2r); //r
			m3 = _m_pmullw(m3, mv2g);
			m0 = _m_paddsw(m0, m3);   //g

			m4 = _m_from_int(*(int*)(srcY+j));
			m4 = _m_psubusb(m4, Mask_016);// unsigned
			m4 = _m_punpcklbw(m4, m7);
			m4 = _m_pmullw(m4, my2rgb);
			m4 = _m_paddw(m4, maddrgb); //usigned

			m0 = _m_paddsw(m0, m4);
			m0 = _m_psrawi(m0, 6);
			m0 = _m_packuswb(m0, m7); //g

			m1 = _m_paddsw(m1, m4);
			m1 = _m_psrawi(m1, 6);
			m1 = _m_packuswb(m1, m7); //r

			m2 = _m_paddsw(m2, m4);
			m2 = _m_psrawi(m2, 6);
			m2 = _m_packuswb(m2, m7); //b

			m2 = _m_punpcklbw(m2, m0); //bgbgbgbg
			m1 = _m_punpcklbw(m1, m7); //r0r0r0r0
			m0 = m2;
			m0 = _m_punpcklwd(m0, m1); //rgb0rgb0
			m2 = _m_punpckhwd(m2, m1);

			*(__m64*)(dst) = m0;
			*(__m64*)(dst+8) = m2;

			dst+=16;
		}
		for(; j<srcWidth; j++)	{
			int u, v, y;
			int rr, gg, bb, yy;
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j]-16;
			makeRGB(y, rr, gg, bb, dst[2], dst[1], dst[0], yy);

			dst+= 4;
		}
		dst+= (dstWidth-4*srcWidth);
		srcY+= srcWidth;
		srcU+= srcWidth;
		srcV+= srcWidth;
	}
	_m_empty();
	return true;
}

bool CVSVideoProc_MMX::ConvertUYVYToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV,
										 int srcWidth, int height, int dstWidth)
{
	if (srcWidth & 0x3 || height & 1) return false;

	const __m64 mask0 = _mm_set1_pi16(0x00FF);
	__m64 m0, m1, m2, m3, m4, m5;

	int i, j, W;
	BYTE *src2, *dstY2;
	W = (srcWidth &~ 15);

	for(i = 0; i < height; i += 2) {
		src2 = src + srcWidth;
		dstY2 = dstY + dstWidth;
		for (j = 0; j < W / 16; j++)	{
			m0 = *(__m64*)(src + j * 16 + 0);
			m1 = *(__m64*)(src + j * 16 + 8);
			// Y
			m4 = m0;
			m0 = _m_psrlqi(m0, 8);
			m5 = m1;
			m1 = _m_psrlqi(m1, 8);
			m0 = _m_pand(m0, mask0);
			m1 = _m_pand(m1, mask0);
			m0 = _m_packuswb(m0, m1);
			// UV
			m4 = _m_pand(m4, mask0);
			m5 = _m_pand(m5, mask0);
			m1 = m4;
			m1 = _m_punpckldq(m1, m5);
			m4 = _m_punpckhdq(m4, m5);
			m1 = _m_packuswb(m1, m1);
			m4 = _m_packuswb(m4, m4);
			m1 = _m_punpcklbw(m1, m4);
			m1 = _mm_shuffle_pi16(m1, _MM_SHUFFLE(3, 1, 2, 0));

			m2 = *(__m64*)(src2 + j * 16 + 0);
			m3 = *(__m64*)(src2 + j * 16 + 8);
			// Y
			m4 = m2;
			m2 = _m_psrlqi(m2, 8);
			m5 = m3;
			m3 = _m_psrlqi(m3, 8);
			m2 = _m_pand(m2, mask0);
			m3 = _m_pand(m3, mask0);
			m2 = _m_packuswb(m2, m3);
			// UV
			m4 = _m_pand(m4, mask0);
			m5 = _m_pand(m5, mask0);
			m3 = m4;
			m3 = _m_punpckldq(m3, m5);
			m4 = _m_punpckhdq(m4, m5);
			m3 = _m_packuswb(m3, m3);
			m4 = _m_packuswb(m4, m4);
			m3 = _m_punpcklbw(m3, m4);
			m3 = _mm_shuffle_pi16(m3, _MM_SHUFFLE(3, 1, 2, 0));

			m1 = _mm_avg_pu8(m1, m3);

			*(__m64*)(dstY  + j * 8) = m0;
			*(__m64*)(dstY2 + j * 8) = m2;
			*(int*)(dstU + j * 4) = _m_to_int(m1);
			m1 = _m_psrlqi(m1, 32);
			*(int*)(dstV + j * 4) = _m_to_int(m1);
		}
		j = (j * 16) / 4;
		for (; j < srcWidth / 4; j++)	{
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
	_m_empty();
	return true;
}

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to 16 bitmap. MMX version.
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
bool CVSVideoProc_MMX::ConvertI420ToBMF16(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									  BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&7) return false;
	if (height<0) {
		height = -height;
		dst = dst + dstWidth*height - dstWidth;
		dstWidth = -dstWidth;
	}

	int i ,j;
	BYTE * pRGB;
	__m64 m0, m1, m2, m3, m4, m5, m6, m7;

    const int Mask_mm0[2]	= {0xFF00FF00, 0xFF00FF00};
    const int Mask_mm1[2]	= {0x00FF00FF, 0x00FF00FF};
    const int Mask_mm2[2]	= {0x0000FFFF, 0x0000FFFF};
    const int Mask_016[2]	= {0x00100010, 0x00100010};
    const int Mask_128[2]	= {0x00800080, 0x00800080};
    const short mv2r[4]		= {V2R>>2, V2R>>2, V2R>>2, V2R>>2};
    const short mu2g[4]		= {U2G>>2, U2G>>2, U2G>>2, U2G>>2};
    const short mv2g[4]		= {V2G>>2, V2G>>2, V2G>>2, V2G>>2};
    const short my2rgb[4]	= {Y2RGB>>2, Y2RGB>>2, Y2RGB>>2, Y2RGB>>2};
    const short maddrgb[4]	= {ADD_RGB>>2, ADD_RGB>>2, ADD_RGB>>2, ADD_RGB>>2};
	const unsigned char rb_mask[8] = {0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8};
	const unsigned char g_mask[8]  = {0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc};
	const int mu2b = 7;

	for (i=0; i<height; i++)
	{
		pRGB = dst + dstWidth*(height-1-i);
		for (j=0; j<srcWidth/2 ;j+=4)
		{
			m7 = _mm_setzero_si64();
			m0 = _mm_cvtsi32_si64(*(int*)(srcU+j));
			m1 = _mm_cvtsi32_si64(*(int*)(srcV+j));
			m0 = _m_punpcklbw(m0, m7);
			m1 = _m_punpcklbw(m1, m7);
			m0 = _m_psubsw(m0, *(__m64*)Mask_128);
			m1 = _m_psubsw(m1, *(__m64*)Mask_128);
			m2 = m0;
			m3 = m1;

			m0 = _m_pmullw(m0, *(__m64*)mu2g);
			m2 = _m_psllwi (m2, 7); //b
			m1 = _m_pmullw(m1, *(__m64*)mv2r); //r
			m3 = _m_pmullw(m3, *(__m64*)mv2g);
			m0 = _m_paddsw(m0, m3);   //g

			m4 = *(__m64*)(srcY+2*j);
			m5 = m4;
			m4 = _m_punpcklbw(m4, m7);
			m5 = _m_punpckhbw(m5, m7);
			m4 = _m_psubsw(m4, *(__m64*)Mask_016);
			m5 = _m_psubsw(m5, *(__m64*)Mask_016);

			m4 = _m_pmullw(m4, *(__m64*)my2rgb);
			m5 = _m_pmullw(m5, *(__m64*)my2rgb);
			m4 = _m_paddsw(m4, *(__m64*)maddrgb);
			m5 = _m_paddsw(m5, *(__m64*)maddrgb);

			m3 = m0;
			m0 = _m_punpcklwd(m0, m0);
			m3 = _m_punpckhwd(m3, m3);
			m0 = _m_paddsw(m0, m4);
			m3 = _m_paddsw(m3, m5);
			m0 = _m_psrawi(m0, 6);
			m3 = _m_psrawi(m3, 6);
			m0 = _m_packuswb(m0, m3); //g
			m0 = _m_pand(m0, *(__m64*)g_mask);
			m3 = m0;
			m0 = _m_punpcklbw(m0, m7);
			m3 = _m_punpckhbw(m3, m7);
			m0 = _m_psllwi(m0, 3);
			m3 = _m_psllwi(m3, 3);

			m6 = m1;
			m1 = _m_punpcklwd(m1, m1);
			m6 = _m_punpckhwd(m6, m6);
			m1 = _m_paddsw(m1, m4);
			m6 = _m_paddsw(m6, m5);
			m1 = _m_psrawi(m1, 6);
			m6 = _m_psrawi(m6, 6);
			m1 = _m_packuswb(m1, m6); //r
			m1 = _m_pand(m1, *(__m64*)rb_mask);
			m6 = m1;
			m1 = _m_punpcklbw(m1, m7);
			m6 = _m_punpckhbw(m6, m7);
			m1 = _m_psllwi(m1, 8);
			m6 = _m_psllwi(m6, 8);
			m0 = _m_por(m0, m1);
			m3 = _m_por(m3, m6);

			m6 = m2;
			m2 = _m_punpcklwd(m2, m2);
			m6 = _m_punpckhwd(m6, m6);
			m2 = _m_paddsw(m2, m4);
			m6 = _m_paddsw(m6, m5);
			m2 = _m_psrawi(m2, 6);
			m6 = _m_psrawi(m6, 6);
			m2 = _m_packuswb(m2, m6); //b
			m2 = _m_pand(m2, *(__m64*)rb_mask);
			m6 = m2;
			m2 = _m_punpcklbw(m2, m7);
			m6 = _m_punpckhbw(m6, m7);
			m2 = _m_psrlwi(m2, 3);
			m6 = _m_psrlwi(m6, 3);
			m0 = _m_por(m0, m2);
			m3 = _m_por(m3, m6);

			*(__m64*)(pRGB +  0) = m0;
			*(__m64*)(pRGB +  8) = m3;

			pRGB+=16;
		}

		srcY+= srcWidth;
		srcU+= (srcWidth/2)*(i&1);
		srcV+= (srcWidth/2)*(i&1);
	}
	_mm_empty();

	return true;
}

bool CVSVideoProc_MMX::ConvertBMF24ToI420(BYTE* Src, BYTE* DstY, BYTE* DstU, BYTE *DstV,
										  int srcW, int h, int dstW)
{
    int i, j, ofs;

	__m64 m0, m1, m2, m3, m4, m5, mmx_null = {0};
	const __m64 ry = _mm_setr_pi16(B2Y, G2Y, R2Y, 0),
				ru = _mm_setr_pi16(B2U, G2U, R2U, 0),
				rv = _mm_setr_pi16(B2V, G2V, R2V, 0);

	unsigned char *pY = DstY, *pU = DstU, *pV = DstV, *pRGB24;

	pY += (h - 1) * dstW;
	pU += (h/2 - 1) * dstW/2;
	pV += (h/2 - 1) * dstW/2;
	pRGB24 = Src;

    for (i = 0; i < (h >> 1); i++)
	{
		ofs = 0;
		for (j = 0; j < (dstW >> 1) - 1; j++)
		{
            m0 = *(__m64*)(pRGB24 + ofs + 0);
			m1 = m0;
			m0 = _m_punpcklbw(m0, mmx_null);
			m1 = _m_psrlqi(m1, 24);
			m1 = _m_punpcklbw(m1, mmx_null);
			m2 = *(__m64*)(pRGB24 + ofs + srcW + 0);
			m3 = m2;
			m2 = _m_punpcklbw(m2, mmx_null);
			m3 = _m_psrlqi(m3, 24);
			m3 = _m_punpcklbw(m3, mmx_null);

			m4 = m0;
			m4 = _m_paddsw(m4, m1);
			m4 = _m_paddsw(m4, m2);
			m4 = _m_paddsw(m4, m3);

			m0 = _m_pmaddwd(m0, ry);
			m1 = _m_pmaddwd(m1, ry);
			m2 = _m_pmaddwd(m2, ry);
			m3 = _m_pmaddwd(m3, ry);

			m5 = m0;
			m5 = _m_psrlqi(m5, 32);
			m0 = _m_paddd(m0, m5);
			m5 = m1;
			m5 = _m_psrlqi(m5, 32);
			m1 = _m_paddd(m1, m5);
			m5 = m2;
			m5 = _m_psrlqi(m5, 32);
			m2 = _m_paddd(m2, m5);
			m5 = m3;
			m5 = _m_psrlqi(m5, 32);
			m3 = _m_paddd(m3, m5);

			pY[j*2-dstW+0	] = CLIP((_mm_cvtsi64_si32(m2) + ADD_Y) >> 8);
			pY[j*2-dstW+1	] = CLIP((_mm_cvtsi64_si32(m3) + ADD_Y) >> 8);
			pY[j*2+0		] = CLIP((_mm_cvtsi64_si32(m0) + ADD_Y) >> 8);
			pY[j*2+1		] = CLIP((_mm_cvtsi64_si32(m1) + ADD_Y) >> 8);

			m0 = m4;
			m0 = _m_pmaddwd(m0, ru);
			m4 = _m_pmaddwd(m4, rv);

			m5 = m0;
			m5 = _m_psrlqi(m5, 32);
			m0 = _m_paddd(m0, m5);
			m5 = m4;
			m5 = _m_psrlqi(m5, 32);
			m4 = _m_paddd(m4, m5);

			pU[j			] = CLIP((_mm_cvtsi64_si32(m0) + ADD_UV*4) >> 10);
			pV[j			] = CLIP((_mm_cvtsi64_si32(m4) + ADD_UV*4) >> 10);

			ofs += 6;
        }

		ofs -= 2;

        m0 = *(__m64*)(pRGB24 + ofs + 0);
		m0 = _m_psrlqi(m0, 16);
		m1 = m0;
		m0 = _m_punpcklbw(m0, mmx_null);
		m1 = _m_psrlqi(m1, 24);
		m1 = _m_punpcklbw(m1, mmx_null);
		m2 = *(__m64*)(pRGB24 + ofs + srcW + 0);
		m2 = _m_psrlqi(m2, 16);
		m3 = m2;
		m2 = _m_punpcklbw(m2, mmx_null);
		m3 = _m_psrlqi(m3, 24);
		m3 = _m_punpcklbw(m3, mmx_null);

		m4 = m0;
		m4 = _m_paddsw(m4, m1);
		m4 = _m_paddsw(m4, m2);
		m4 = _m_paddsw(m4, m3);

		m0 = _m_pmaddwd(m0, ry);
		m1 = _m_pmaddwd(m1, ry);
		m2 = _m_pmaddwd(m2, ry);
		m3 = _m_pmaddwd(m3, ry);

		m5 = m0;
		m5 = _m_psrlqi(m5, 32);
		m0 = _m_paddd(m0, m5);
		m5 = m1;
		m5 = _m_psrlqi(m5, 32);
		m1 = _m_paddd(m1, m5);
		m5 = m2;
		m5 = _m_psrlqi(m5, 32);
		m2 = _m_paddd(m2, m5);
		m5 = m3;
		m5 = _m_psrlqi(m5, 32);
		m3 = _m_paddd(m3, m5);

		pY[j*2-dstW+0	] = CLIP((_mm_cvtsi64_si32(m2) + ADD_Y) >> 8);
		pY[j*2-dstW+1	] = CLIP((_mm_cvtsi64_si32(m3) + ADD_Y) >> 8);
		pY[j*2+0		] = CLIP((_mm_cvtsi64_si32(m0) + ADD_Y) >> 8);
		pY[j*2+1		] = CLIP((_mm_cvtsi64_si32(m1) + ADD_Y) >> 8);

		m0 = m4;
		m0 = _m_pmaddwd(m0, ru);
		m4 = _m_pmaddwd(m4, rv);

		m5 = m0;
		m5 = _m_psrlqi(m5, 32);
		m0 = _m_paddd(m0, m5);
		m5 = m4;
		m5 = _m_psrlqi(m5, 32);
		m4 = _m_paddd(m4, m5);

		pU[j			] = CLIP((_mm_cvtsi64_si32(m0) + ADD_UV*4) >> 10);
		pV[j			] = CLIP((_mm_cvtsi64_si32(m4) + ADD_UV*4) >> 10);

		pRGB24 += 2*srcW;
		pY -= 2*dstW;
		pU -= dstW/2;
		pV -= dstW/2;
    }

	_mm_empty();

	return true;
}

bool CVSVideoProc_MMX::ConvertBMF32ToI420(BYTE* Src, BYTE* DstY, BYTE* DstU, BYTE *DstV,
										  int srcW, int h, int dstW)
{
    int i, j, ofs;

	__m64 m0, m1, m2, m3, m4, m5, mmx_null = {0};
	const __m64 ry = _mm_setr_pi16(B2Y, G2Y, R2Y, 0),
				ru = _mm_setr_pi16(B2U, G2U, R2U, 0),
				rv = _mm_setr_pi16(B2V, G2V, R2V, 0);

	unsigned char *pY = DstY, *pU = DstU, *pV = DstV, *pRGB32;

	pY += (h - 1) * dstW;
	pU += (h/2 - 1) * dstW/2;
	pV += (h/2 - 1) * dstW/2;
	pRGB32 = Src;

    for (i = 0; i < (h >> 1); i++)
	{
		ofs = 0;
		for (j = 0; j < (dstW >> 1); j++)
		{
            m0 = *(__m64*)(pRGB32 + ofs + 0);
			m1 = m0;
			m0 = _m_punpcklbw(m0, mmx_null);
			m1 = _m_punpckhbw(m1, mmx_null);
			m2 = *(__m64*)(pRGB32 + ofs + srcW + 0);
			m3 = m2;
			m2 = _m_punpcklbw(m2, mmx_null);
			m3 = _m_punpckhbw(m3, mmx_null);

			m4 = m0;
			m4 = _m_paddsw(m4, m1);
			m4 = _m_paddsw(m4, m2);
			m4 = _m_paddsw(m4, m3);

			m0 = _m_pmaddwd(m0, ry);
			m1 = _m_pmaddwd(m1, ry);
			m2 = _m_pmaddwd(m2, ry);
			m3 = _m_pmaddwd(m3, ry);

			m5 = m0;
			m5 = _m_psrlqi(m5, 32);
			m0 = _m_paddd(m0, m5);
			m5 = m1;
			m5 = _m_psrlqi(m5, 32);
			m1 = _m_paddd(m1, m5);
			m5 = m2;
			m5 = _m_psrlqi(m5, 32);
			m2 = _m_paddd(m2, m5);
			m5 = m3;
			m5 = _m_psrlqi(m5, 32);
			m3 = _m_paddd(m3, m5);

			pY[j*2-dstW+0	] = CLIP((_mm_cvtsi64_si32(m2) + ADD_Y) >> 8);
			pY[j*2-dstW+1	] = CLIP((_mm_cvtsi64_si32(m3) + ADD_Y) >> 8);
			pY[j*2+0		] = CLIP((_mm_cvtsi64_si32(m0) + ADD_Y) >> 8);
			pY[j*2+1		] = CLIP((_mm_cvtsi64_si32(m1) + ADD_Y) >> 8);

			m0 = m4;
			m0 = _m_pmaddwd(m0, ru);
			m4 = _m_pmaddwd(m4, rv);

			m5 = m0;
			m5 = _m_psrlqi(m5, 32);
			m0 = _m_paddd(m0, m5);
			m5 = m4;
			m5 = _m_psrlqi(m5, 32);
			m4 = _m_paddd(m4, m5);

			pU[j			] = CLIP((_mm_cvtsi64_si32(m0) + ADD_UV*4) >> 10);
			pV[j			] = CLIP((_mm_cvtsi64_si32(m4) + ADD_UV*4) >> 10);

			ofs += 8;
        }

		pRGB32 += 2*srcW;
		pY -= 2*dstW;
		pU -= dstW/2;
		pV -= dstW/2;
    }

	_mm_empty();

	return true;
}

/**
 ******************************************************************************
 * Saturate chroma component of I420 image. MMX version.
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
bool CVSVideoProc_MMX::SaturateI420Chroma(BYTE* src, int srcWidth, int srcHeight, DWORD dwSaturation)
{
	int i, j;

	int sat = dwSaturation*128/100;

	const __m64 m128 = W_FILL_QW(0x80);
	const __m64 m64 = W_FILL_QW(0x40);
	const __m64 msat = W_FILL_QW(sat);
	__m64 m0, m7 = {0};

	for (j = 0; j<srcHeight; j++)
	{
		for (i = 0; i<srcWidth; i+=4)
		{
			m0 = _m_from_int(*(int*)(src+i));
			m0 = _m_punpcklbw(m0, m7);
			m0 = _m_psubsw(m0, m128);
			m0 = _m_pmullw(m0, msat);
			m0 = _m_paddsw(m0, m64);
			m0 = _m_psrawi(m0, 7);
			m0 = _m_paddsw(m0, m128);
			m0 = _m_packuswb(m0, m7);
			*(int*)(src + i) = _m_to_int(m0);
		}
		src+=srcWidth;
	}
	_m_empty();
	return true;
}

/******************************************************************************
 * Interpolate by bicubic 1 row of the image. MMX version.
 *
 * \note
 *
 * \param src				[in]  - pointer to row of source image component;
 * \param dst				[out] - pointer to interpolated row of destination image component;
 * \param w					[in]  - width  of source image;
 * \param h					[in]  - height of source image;
 * \param Luma				[in]  - true if luma component;
 *
 *  \date    26-11-2002		Created
 ******************************************************************************/
void CVSVideoProc_MMX::InterpolateHor1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma)
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
	const __m64 ma1 = W_TO_QW(BIC_A0, BIC_A1, BIC_A2, BIC_A3);
	const __m64 ma0 = W_TO_QW(BIC_A3, BIC_A2, BIC_A1, BIC_A0);
	const __m64 madd = DW_FILL_QW(0x40);
	__m64 m0, m1, m2, m7 = {0};

	for (j = 0; j<h; j++)
	{
		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[0] + BIC_A1*src[0] + BIC_A2*src[1] + BIC_A3*src[2] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst[2] = CLIP((BIC_A3*src[0] + BIC_A2*src[1] + BIC_A1*src[2] + BIC_A0*src[3] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst+=3; src+=2;

		for (i = 0; i<(w/2-2); i++)
		{
			int res;
			dst[0] = src[0];
			m0 = _m_from_int(*(int*)(src-1));
			m1 = _m_from_int(*(int*)(src));
			m0 = _m_punpcklbw(m0, m7);
			m1 = _m_punpcklbw(m1, m7);
			m0 = _m_pmaddwd(m0, ma0);
			m1 = _m_pmaddwd(m1, ma1);
			m2 = m0;
			m0 = _m_punpckldq(m0, m1);
			m2 = _m_punpckhdq(m2, m1);
			m0 = _m_paddd(m0, m2);
			m0 = _m_paddd(m0, madd); // must be unsigned
			m0 = _m_packssdw(m0, m7);// must be less then 2^15
			m0 = _m_psrawi(m0, RADIX_1);
			m0 = _m_packuswb(m0, m7);
			res = _m_to_int(m0);
			*(short*)(dst+1) = (short)res;
			dst+=3; src+=2;
		}

		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A2*src[1] + BIC_A3*src[1] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst[2] = CLIP((BIC_A3*src[ 0] + BIC_A2*src[1] + BIC_A1*src[1] + BIC_A0*src[1] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst+=3; src+=2;
	}
	_m_empty();
}


/**
 ******************************************************************************
 * Bicubic interpolation by factor 1.5 of the image in ver dim. MMX version.
 *
 * \note
 *
 * \param src				[in]  - pointer to row of source image component;
 * \param dst				[out] - pointer to interpolated row of destination image component;
 * \param w					[in]  - width  of source image;
 * \param h					[in]  - height of source image;
 * \param Luma				[in]  - true if luma component;
 *
 *  \date    26-11-2002		Created.
 ******************************************************************************
 */
void CVSVideoProc_MMX::InterpolateVer1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma)
{
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
	const __m64 ma0 = W_FILL_QW(BIC_A0);
	const __m64 ma1 = W_FILL_QW(BIC_A1);
	const __m64 ma2 = W_FILL_QW(BIC_A2);
	const __m64 ma3 = W_FILL_QW(BIC_A3);
	const __m64 madd = DW_FILL_QW(0x40);
	__m64 m0, m1, m2, m3, m4, m5, m6, m7 = {0};
	int i, j;

	for (j = 0; j<w; j+=4)
	{
		*(int*)(dst) = *(int*)(src);
		m0 = _m_from_int(*(int*)(src    ));
		m1 = _m_from_int(*(int*)(src+w  ));
		m2 = _m_from_int(*(int*)(src+w*2));
		m3 = _m_from_int(*(int*)(src+w*3));

		m0 = _m_punpcklbw(m0, m7);
		m4 = m5 = m0;
		m1 = _m_punpcklbw(m1, m7);
		m2 = _m_punpcklbw(m2, m7);
		m6 = m2;
		m3 = _m_punpcklbw(m3, m7);

		m0 = _m_pmullw(m0, ma0);
		m4 = _m_pmullw(m4, ma1);
		m5 = _m_pmullw(m5, ma3);
		m1 = _m_pmullw(m1, ma2);
		m2 = _m_pmullw(m2, ma3);
		m6 = _m_pmullw(m6, ma1);
		m3 = _m_pmullw(m3, ma0);

		m0 = _m_paddsw(m0, m2);
		m0 = _m_paddsw(m0, m4);
		m0 = _m_paddsw(m0, m1);
		m0 = _m_paddsw(m0, madd);

		m5 = _m_paddsw(m5, m6);
		m5 = _m_paddsw(m5, m3);
		m5 = _m_paddsw(m5, m1);
		m5 = _m_paddsw(m5, madd);

		m0 = _m_psrawi(m0, RADIX_1);
		m5 = _m_psrawi(m5, RADIX_1);

		m0 = _m_packuswb(m0, m0);
		m5 = _m_packuswb(m5, m5);

		*(int*)(dst+w  ) = _m_to_int(m0);
		*(int*)(dst+w*2) = _m_to_int(m5);
		dst+=4; src+=4;
	}
	dst+=2*w; src+=w;

	for (i = 0; i<(h/2-2); i++)
	{
		for (j = 0; j<w; j+=4)
		{
			*(int*)(dst) = *(int*)(src);
			m0 = _m_from_int(*(int*)(src-w  ));
			m1 = _m_from_int(*(int*)(src    ));
			m2 = _m_from_int(*(int*)(src+w  ));
			m3 = _m_from_int(*(int*)(src+w*2));
			m4 = _m_from_int(*(int*)(src+w*3));

			m0 = _m_punpcklbw(m0, m7);
			m1 = _m_punpcklbw(m1, m7);
			m5 = m1;
			m2 = _m_punpcklbw(m2, m7);
			m3 = _m_punpcklbw(m3, m7);
			m6 = m3;
			m4 = _m_punpcklbw(m4, m7);

			m0 = _m_pmullw(m0, ma0);
			m1 = _m_pmullw(m1, ma1);
			m5 = _m_pmullw(m5, ma3);
			m2 = _m_pmullw(m2, ma2);
			m3 = _m_pmullw(m3, ma3);
			m6 = _m_pmullw(m6, ma1);
			m4 = _m_pmullw(m4, ma0);

			m0 = _m_paddsw(m0, m1);
			m0 = _m_paddsw(m0, m3);
			m0 = _m_paddsw(m0, m2);
			m0 = _m_paddsw(m0, madd);

			m5 = _m_paddsw(m5, m6);
			m5 = _m_paddsw(m5, m4);
			m5 = _m_paddsw(m5, m2);
			m5 = _m_paddsw(m5, madd);

			m0 = _m_psrawi(m0, RADIX_1);
			m5 = _m_psrawi(m5, RADIX_1);

			m0 = _m_packuswb(m0, m0);
			m5 = _m_packuswb(m5, m5);

			*(int*)(dst+w  ) = _m_to_int(m0);
			*(int*)(dst+w*2) = _m_to_int(m5);
			dst+=4; src+=4;
		}
		dst+=2*w; src+=w;
	}

	for (j = 0; j<w; j+=4)
	{
		*(int*)(dst) = *(int*)(src);
		m0 = _m_from_int(*(int*)(src-w  ));
		m1 = _m_from_int(*(int*)(src    ));
		m2 = _m_from_int(*(int*)(src+w  ));

		m0 = _m_punpcklbw(m0, m7);
		m1 = _m_punpcklbw(m1, m7);
		m3 = m1;
		m2 = _m_punpcklbw(m2, m7);
		m4 = m5 = m6 = m2;

		m0 = _m_pmullw(m0, ma0);
		m1 = _m_pmullw(m1, ma1);
		m3 = _m_pmullw(m3, ma3);
		m2 = _m_pmullw(m2, ma2);
		m4 = _m_pmullw(m4, ma3);
		m5 = _m_pmullw(m5, ma1);
		m6 = _m_pmullw(m6, ma0);

		m0 = _m_paddsw(m0, m1);
		m0 = _m_paddsw(m0, m4);
		m0 = _m_paddsw(m0, m2);
		m0 = _m_paddsw(m0, madd);

		m3 = _m_paddsw(m3, m5);
		m3 = _m_paddsw(m3, m6);
		m3 = _m_paddsw(m3, m2);
		m3 = _m_paddsw(m3, madd);

		m0 = _m_psrawi(m0, RADIX_1);
		m3 = _m_psrawi(m3, RADIX_1);

		m0 = _m_packuswb(m0, m0);
		m3 = _m_packuswb(m3, m3);

		*(int*)(dst+w  ) = _m_to_int(m0);
		*(int*)(dst+w*2) = _m_to_int(m3);
		dst+=4; src+=4;
	}
	_m_empty();
}


/******************************************************************************
 * Interpolate by bicubic on hor dims dy factor 2
 *
 * \note
 *
 * \param src				[in]  - pointer to row of source image component;
 * \param dst				[out] - pointer to interpolated row of destination image component;
 * \param w					[in]  - width  of source image;
 * \param h					[in]  - height of source image;
 * \param Luma				[in]  - true if luma component;
 *
 *  \date    20-02-2004		Created
 ******************************************************************************/
void CVSVideoProc_MMX::InterpolateHor2(BYTE *src, BYTE *dst, int w, int h, bool Luma)
{
	int i, j;
	int BIC_A0, BIC_A1;
	// use /4 koefs!!!
	if (Luma) {
		BIC_A0 = UP2_LA0/4;
		BIC_A1 = UP2_LA1/4;
	}
	else {
		BIC_A0 = UP2_CA0/4;
		BIC_A1 = UP2_CA1/4;
	}
	const __m64 ma0 = W_FILL_QW(BIC_A0);
	const __m64 ma1 = W_FILL_QW(BIC_A1);
	const __m64 madd = W_FILL_QW(0x10);
	__m64 m0, m1, m2, m3, m4;

	for (j = 0; j<h; j++) {
		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[0] + BIC_A1*src[0] + BIC_A1*src[1] + BIC_A0*src[2] + 0x10)>>5);
		//src++;
		dst+=2;

		for (i = 0; i<(w-7); i+=4) {
			m4 = *(__m64*)(src+i);
			m1 = m4;
			m4 = _m_psrlqi(m4, 8);
			m2 = m4;
			m4 = _m_psrlqi(m4, 8);
			m3 = m4;
			m4 = _m_psrlqi(m4, 8);

			m0 = _mm_setzero_si64();
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpcklbw(m2, m0);
			m3 = _m_punpcklbw(m3, m0);
			m4 = _m_punpcklbw(m4, m0);
			m0 = m2;

			m1 = _m_paddsw(m1, m4);
			m2 = _m_paddsw(m2, m3);
			m1 = _m_pmullw(m1, ma0);
			m2 = _m_pmullw(m2, ma1);
			m1 = _m_paddsw(m1, m2);
			m1 = _m_paddsw(m1, madd);
			m1 = _m_psrawi(m1, 5);

			m1 = _m_packuswb(m1, m1);
			m0 = _m_packuswb(m0, m0);
			m0 = _m_punpcklbw(m0, m1);
			*(__m64*)(dst+i*2) = m0;
		}
		dst+=i*2; src+=i+1;

		for (;i<(w-3); i++) {
			dst[0] = src[0];
			dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A1*src[1] + BIC_A0*src[2] + 0x10)>>5);
			dst+=2; src++;
		}

		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A1*src[1] + BIC_A0*src[1] + 0x10)>>5);
		dst+=2; src++;

		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A1*src[0] + BIC_A0*src[0] + 0x10)>>5);
		dst+=2; src++;
	}
	_m_empty();
}


/******************************************************************************
* Bicubic interpolation by factor 2 of the image in ver dim. MMX version.
*
* \note
*
* \param src				[in]  - pointer to row of source image component;
* \param dst				[out] - pointer to interpolated row of destination image component;
* \param w					[in]  - width  of source image;
* \param h					[in]  - height of source image;
* \param Luma				[in]  - true if luma component;
*
*  \date    20-02-2004		Created
******************************************************************************/
void CVSVideoProc_MMX::InterpolateVer2(BYTE *src, BYTE *dst, int w, int h, bool Luma)
{
	int i, j;
	int BIC_A0, BIC_A1;
	// use /4 koefs!!!
	if (Luma) {
		BIC_A0 = UP2_LA0/4;
		BIC_A1 = UP2_LA1/4;
	}
	else {
		BIC_A0 = UP2_CA0/4;
		BIC_A1 = UP2_CA1/4;
	}
	const __m64 ma0 = W_FILL_QW(BIC_A0);
	const __m64 ma1 = W_FILL_QW(BIC_A1);
	const __m64 madd = W_FILL_QW(0x10);
	__m64 m0, m1, m2, m3, m4;

	for (j = 0; j<w; j+=4) {
		*(int*)(dst) = *(int*)(src);
		m2 = _m_from_int(*(int*)(src    ));
		m3 = _m_from_int(*(int*)(src+w  ));
		m4 = _m_from_int(*(int*)(src+2*w));

		m0 = _mm_setzero_si64();
		m2 = _m_punpcklbw(m2, m0);
		m1 = m2;
		m3 = _m_punpcklbw(m3, m0);
		m4 = _m_punpcklbw(m4, m0);

		m1 = _m_paddsw(m1, m4);
		m2 = _m_paddsw(m2, m3);
		m1 = _m_pmullw(m1, ma0);
		m2 = _m_pmullw(m2, ma1);
		m1 = _m_paddsw(m1, m2);
		m1 = _m_paddsw(m1, madd);
		m1 = _m_psrawi(m1, 5);

		m1 = _m_packuswb(m1, m1);
		*(int*)(dst+w  ) = _m_to_int(m1);
		dst+=4; src+=4;
	}
	dst+=w; src+=0;

	for (i = 0; i<(h-3); i++) {
		for (j = 0; j<w; j+=4) {
			*(int*)(dst) = *(int*)(src);
			m1 = _m_from_int(*(int*)(src-w  ));
			m2 = _m_from_int(*(int*)(src    ));
			m3 = _m_from_int(*(int*)(src+w  ));
			m4 = _m_from_int(*(int*)(src+2*w));

			m0 = _mm_setzero_si64();
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpcklbw(m2, m0);
			m3 = _m_punpcklbw(m3, m0);
			m4 = _m_punpcklbw(m4, m0);

			m1 = _m_paddsw(m1, m4);
			m2 = _m_paddsw(m2, m3);
			m1 = _m_pmullw(m1, ma0);
			m2 = _m_pmullw(m2, ma1);
			m1 = _m_paddsw(m1, m2);
			m1 = _m_paddsw(m1, madd);
			m1 = _m_psrawi(m1, 5);

			m1 = _m_packuswb(m1, m1);
			*(int*)(dst+w  ) = _m_to_int(m1);
			dst+=4; src+=4;
		}
		dst+=w; src+=0;
	}

	for (j = 0; j<w; j+=4) {
		*(int*)(dst) = *(int*)(src);
		m1 = _m_from_int(*(int*)(src-w  ));
		m2 = _m_from_int(*(int*)(src    ));
		m3 = _m_from_int(*(int*)(src+w  ));

		m0 = _mm_setzero_si64();
		m1 = _m_punpcklbw(m1, m0);
		m2 = _m_punpcklbw(m2, m0);
		m3 = _m_punpcklbw(m3, m0);
		m4 = m3;

		m1 = _m_paddsw(m1, m4);
		m2 = _m_paddsw(m2, m3);
		m1 = _m_pmullw(m1, ma0);
		m2 = _m_pmullw(m2, ma1);
		m1 = _m_paddsw(m1, m2);
		m1 = _m_paddsw(m1, madd);
		m1 = _m_psrawi(m1, 5);

		m1 = _m_packuswb(m1, m1);
		*(int*)(dst+w  ) = _m_to_int(m1);
		dst+=4; src+=4;
	}
	dst+=w; src+=0;

	for (j = 0; j<w; j+=4) {
		*(int*)(dst) = *(int*)(src);
		m1 = _m_from_int(*(int*)(src-w  ));
		m2 = _m_from_int(*(int*)(src    ));

		m0 = _mm_setzero_si64();
		m1 = _m_punpcklbw(m1, m0);
		m2 = _m_punpcklbw(m2, m0);
		m3 = m4 = m2;

		m1 = _m_paddsw(m1, m4);
		m2 = _m_paddsw(m2, m3);
		m1 = _m_pmullw(m1, ma0);
		m2 = _m_pmullw(m2, ma1);
		m1 = _m_paddsw(m1, m2);
		m1 = _m_paddsw(m1, madd);
		m1 = _m_psrawi(m1, 5);

		m1 = _m_packuswb(m1, m1);
		*(int*)(dst+w  ) = _m_to_int(m1);
		dst+=4; src+=4;
	}
	_m_empty();
}

/******************************************************************************
 * Make Dithering other the channel, depends from the Brightness. MMX version
 * \return none
 *
 * \param Src				[IN, OUT]	- processing image
 * \param W, H				[IN]		- image width and heith
 * \param Bits				[IN]		- Dithering Depth
 *
 *  \date    23-03-2004
 ******************************************************************************/
bool CVSVideoProc_MMX::DitherI420(BYTE* Src, int W, int H, int Bits)
{
	static BYTE Cnt[8]={ 0,  1,  1,  0,  1,  1,  1,  0 };
	static int ThresholdBrightness[3] = {0, 448, 568};
	int i, j;
	BYTE *src;
	static BYTE  Mask[8*6] ={
		0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
		0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe,
		0x01, 0x03, 0x01, 0x03, 0x01, 0x03, 0x01, 0x03,
		0xfe, 0xfc, 0xfe, 0xfc, 0xfe, 0xfc, 0xfe, 0xfc,
		0x03, 0x07, 0x03, 0x07, 0x03, 0x07, 0x03, 0x07,
		0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8
	};

	__m64 m0, mask, nmask, cnt;

	if (Bits>2) Bits = 2;
	if (Bits<0) Bits = 0;

	cnt = *(__m64*)(Cnt);

	for (j = 0; j<H; j++) {
		src = Src+W*j;
		for (i = 0; i<W/8; i++) {
			int	brightness;
			int bits;

			bits = Bits;
			brightness = (src[i*8+0]+src[i*8+1]+src[i*8+2]+src[i*8+3]+src[i*8+4]+src[i*8+5]+src[i*8+6]+src[i*8+7]);
			if (brightness<ThresholdBrightness[bits]) bits--;

			mask = *(__m64*)(Mask+bits*16);
			nmask = *(__m64*)(Mask+bits*16+8);

			m0 = *(__m64*)(src+i*8);
			m0 = _m_paddusb(m0, cnt);
			cnt = _m_pand(m0, mask);
			m0 = _m_pand(m0, nmask);
			*(__m64*)(src+i*8) = m0;
		}
	}
	*(__m64*)(Cnt) = cnt;
	_mm_empty();
	return true;
}

/*******************************************************************************
 * Bilinear free interpolation of 8, 24 and 32 bits per pixel image. MMX version.
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 * \param dstStep			[in]  - destination line length in bytes;
 * \param mode				[in]  - type of image, 0 - 8bit, 2 - 24bit, 2 - 32bit;
 *
 * \return  false if input parametrs are out of range.
 *
 * \date    01-06-2005		Created
 ******************************************************************************/
bool CVSVideoProc_MMX::ResizeBilinear(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep, int mode, int isUVPlane)
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
			bic_resize_x1_mmx(src, m_tbuff, srcW, srcH, dstW, m_resize_st[isUVPlane]->bicubic_cf_x_8bit, m_resize_st[isUVPlane]->bicubic_cf_len_x_8bit);
		else
			b_resize_x1_mmx(src, m_tbuff, srcW, srcH, dstW);
		if (dstH < srcH)
			bic_resize_y1_mmx(m_tbuff, dst, dstW, srcH, dstH, dstStep, m_resize_st[isUVPlane]->bicubic_cf_y, m_resize_st[isUVPlane]->bicubic_cf_len_y);
		else
			b_resize_y1_mmx(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==1) {
		if (dstStep < dstW*3)
			return false;
		if (dstW < srcW)
			bic_resize_x3_mmx(src, m_tbuff, srcW, srcH, dstW, m_resize_st[isUVPlane]->bicubic_cf_x, m_resize_st[isUVPlane]->bicubic_cf_len_x);
		else
			b_resize_x3_mmx(src, m_tbuff, srcW, srcH, dstW);
		if (dstH < srcH)
			bic_resize_y3_mmx(m_tbuff, dst, dstW, srcH, dstH, dstStep, m_resize_st[isUVPlane]->bicubic_cf_y, m_resize_st[isUVPlane]->bicubic_cf_len_y);
		else
			b_resize_y3_mmx(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==2) {
		if (dstStep < dstW*4)
			return false;
		if (dstW < srcW)
			bic_resize_x3_4_mmx(src, m_tbuff, srcW, srcH, dstW, m_resize_st[isUVPlane]->bicubic_cf_x, m_resize_st[isUVPlane]->bicubic_cf_len_x);
		else
			b_resize_x3_4_mmx(src, m_tbuff, srcW, srcH, dstW);
		if (dstH < srcH)
			bic_resize_y3_4_mmx(m_tbuff, dst, dstW, srcH, dstH, dstStep, m_resize_st[isUVPlane]->bicubic_cf_y, m_resize_st[isUVPlane]->bicubic_cf_len_y);
		else
			b_resize_y3_4_mmx(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==3) {
		if (dstStep < dstW*2)
			return false;
		if (dstW<srcW)
			bic_resize_x_565(src, m_tbuff, srcW, srcH, dstW);
		else
			b_resize_x_565_mmx(src, m_tbuff, srcW, srcH, dstW);
		if (dstH<srcH)
			bic_resize_y_565(m_tbuff, dst, dstW, srcH, dstH, dstStep);
		else
			b_resize_y_565_mmx(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==4) {
		if (dstStep < dstW)
			return false;
		bic_resize_8_x1_mmx(src, m_tbuff, srcW, srcH);
		bic_resize_8_y1_mmx(m_tbuff, dst, dstW, srcH, dstStep);
	} else
		return false;
	return true;
}

bool CVSVideoProc_SSE2::InitResize(int w, int h, int new_w, int new_h)
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

		m_resize_st[k]->x_add_reg = (unsigned short*)_aligned_malloc(4*sizeof(unsigned short), 16);
		m_resize_st[k]->y_add_reg = (unsigned short*)_aligned_malloc(8*sizeof(unsigned short), 16);
		m_resize_st[k]->dirx_cf_reg = (unsigned short*)_aligned_malloc(4*new_w*sizeof(unsigned short), 16);
		m_resize_st[k]->invx_cf_reg = (unsigned short*)_aligned_malloc(4*new_w*sizeof(unsigned short), 16);
		m_resize_st[k]->diry_cf_reg = (unsigned short*)_aligned_malloc(8*new_h*sizeof(unsigned short), 16);
		m_resize_st[k]->invy_cf_reg = (unsigned short*)_aligned_malloc(8*new_h*sizeof(unsigned short), 16);

		/// x dir bilinear coef
		for (i = 0; i < new_w; i++) {
			_mm_empty();
			x = factor * i;
			m_resize_st[k]->x_pos_8bit[i] = (int)(factor * i);
			m_resize_st[k]->x_pos_24bit[i] = (int)(factor * i) * 3;
			m_resize_st[k]->x_pos_32bit[i] = (int)(factor * i) * 4;
			x -= (double)(int)x;
			m_resize_st[k]->dirx_cf[i] = (int)((1 - x) * (1 << 8) + 0.5);
			m_resize_st[k]->invx_cf[i] = (1 << 8) - m_resize_st[k]->dirx_cf[i];
			*(__m64*)(m_resize_st[k]->dirx_cf_reg + 4 * i) = _mm_set1_pi16(m_resize_st[k]->dirx_cf[i]);
			*(__m64*)(m_resize_st[k]->invx_cf_reg + 4 * i) = _mm_set1_pi16(m_resize_st[k]->invx_cf[i]);
			_mm_empty();
		}
		_mm_empty();
		/// y dir bilinear coef
		factor = (double)(h - 1) / (new_h - 1);
		for (j = 1; j < new_h - 1; j++) {
			x = factor * j;
			m_resize_st[k]->y_pos[j] = (int)(x);
			x -= (double)(int)x;
			m_resize_st[k]->diry_cf[j] = (int)((1 - x) * (1 << 8) + 0.5);
			m_resize_st[k]->invy_cf[j] = (1 << 8) - m_resize_st[k]->diry_cf[j];
			*(__m128i*)(m_resize_st[k]->diry_cf_reg + 8 * j) = _mm_set1_epi16(m_resize_st[k]->diry_cf[j]);
			*(__m128i*)(m_resize_st[k]->invy_cf_reg + 8 * j) = _mm_set1_epi16(m_resize_st[k]->invy_cf[j]);
			_mm_empty();
		}
		*(__m64*)m_resize_st[k]->x_add_reg = _mm_set1_pi16((1 << 8)/2);
		*(__m128i*)m_resize_st[k]->y_add_reg = _mm_set1_epi16((1 << 8)/2);
		_mm_empty();
		*/
		/// bicubic coef
		m_resize_st[k]->bicubic_cf_len_x_8bit = genTableAlloc(w, new_w, m_resize_st[k]->bicubic_cf_x_8bit);
		m_resize_st[k]->bicubic_cf_len_x = genTableAlloc_mmx(w, new_w, m_resize_st[k]->bicubic_cf_x);
		m_resize_st[k]->bicubic_cf_len_y = genTableAlloc_mmx(h, new_h, m_resize_st[k]->bicubic_cf_y);

		_mm_empty();

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

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to YUY2 format. SSE2 version.
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
bool CVSVideoProc_SSE2::ConvertI420ToYUY2(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									 BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;

	__m128i my0, my1, mu, mv;

	for(int i = 0; i<height; i++) {
		int W1 = (srcWidth / 16 * 16) >> 1;
		int j=0;
		for(; j<W1; j+=8) {
			mu = _mm_loadl_epi64((__m128i *)(srcU+j));
			mv = _mm_loadl_epi64((__m128i *)(srcV+j));
			mu = _mm_unpacklo_epi8(mu, mv);

			my0 = _mm_loadu_si128((__m128i *)(srcY+2*j));
			my1 = my0;
			my0 = _mm_unpacklo_epi8(my0, mu);
			my1 = _mm_unpackhi_epi8(my1, mu);

			_mm_storeu_si128((__m128i*)(dst + j*4 + 0), my0);
			_mm_storeu_si128((__m128i*)(dst + j*4 +16), my1);
		}
		for(; j<(srcWidth>>1); j++) {
			dst[j*4 + 0] = srcY[j*2 + 0];
			dst[j*4 + 1] = srcU[j   + 0];
			dst[j*4 + 2] = srcY[j*2 + 1];
			dst[j*4 + 3] = srcV[j   + 0];
		}

		srcY+=srcWidth;
		dst+= dstWidth;
		if(i&1)	srcU+= srcWidth/2, srcV+= srcWidth/2;
	}
	_m_empty();
	return true;
}

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to UYVY format. SSE2 version.
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
bool CVSVideoProc_SSE2::ConvertI420ToUYVY(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									 BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;

	__m128i my, mu, mv;

	for(int i = 0; i<height; i++) {
		int W1 = (srcWidth / 16 * 16) >> 1;
		int j=0;
		for(; j<W1; j+=8) {
			mu = _mm_loadl_epi64((__m128i *)(srcU+j));
			mv = _mm_loadl_epi64((__m128i *)(srcV+j));
			my = _mm_loadu_si128((__m128i *)(srcY+2*j));
			mu = _mm_unpacklo_epi8(mu, mv);
			mv = mu;

			mu = _mm_unpacklo_epi8(mu, my);
			mv = _mm_unpackhi_epi8(mv, my);
			_mm_storeu_si128((__m128i*)(dst + j*4 + 0), mu);
			_mm_storeu_si128((__m128i*)(dst + j*4 +16), mv);
		}
		for(; j<(srcWidth>>1); j++)	{
			dst[j*4 + 0] = srcU[j   + 0];
			dst[j*4 + 1] = srcY[j*2 + 0];
			dst[j*4 + 2] = srcV[j   + 0];
			dst[j*4 + 3] = srcY[j*2 + 1];
		}

		srcY+=srcWidth;
		dst+= dstWidth;
		if(i&1)	srcU+= srcWidth/2, srcV+= srcWidth/2;
	}
	_m_empty();
	return true;
}

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to bitmap-24 format. SSE2 version.
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
bool CVSVideoProc_SSE2::ConvertI420ToBMF24(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									  BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;

	int i ,j;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	const __m128i Mask0		= _mm_set1_epi32(0xFF00FF00),
				  Mask1		= _mm_set1_epi32(0x00FF00FF),
				  Mask2		= _mm_set1_epi32(0x0000FFFF),
				  Mask_016	= _mm_set1_epi32(0x10101010),
				  Mask_128	= _mm_set1_epi32(0x00800080),
				  mv2r		= _mm_set1_epi16(V2R>>2),
				  mu2g		= _mm_set1_epi16(U2G>>2),
				  mv2g		= _mm_set1_epi16(V2G>>2),
				  my2rgb	= _mm_set1_epi16(Y2RGB>>2),
				  maddrgb	= _mm_set1_epi16(ADD_RGB>>2);

	srcY += srcWidth * (height - 1);
	srcU += srcWidth / 2 * (height / 2 - 1);
	srcV += srcWidth / 2 * (height / 2 - 1);

	for (i = 0; i < height; i++) {
		//int W1 = (srcWidth &~ 7) >> 1;
		//int W1 = srcWidth / 16 * 16;
		int W1 = (srcWidth / 16 * 16) >> 1;
		//W1 = W1 / 16 * 16;
		for (j = 0; j < W1; j += 8) {
		//for (j = 0; j < W1; j += 4) {
			xmm7 = _mm_setzero_si128();
			xmm0 = _mm_loadl_epi64((__m128i *)(srcU+j));
			xmm1 = _mm_loadl_epi64((__m128i *)(srcV+j));
			//xmm0 = _mm_cvtsi32_si128(*(int*)(srcU+j));
			//xmm1 = _mm_cvtsi32_si128(*(int*)(srcV+j));
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm7);
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm7);
			xmm0 = _mm_subs_epi16(xmm0, Mask_128);
			xmm1 = _mm_subs_epi16(xmm1, Mask_128);
						xmm2 = xmm0;
			xmm3 = xmm1;

			xmm0 = _mm_mullo_epi16(xmm0, mu2g);
			xmm2 = _mm_slli_epi16(xmm2, 7); // b
			xmm1 = _mm_mullo_epi16(xmm1, mv2r); // r
			xmm3 = _mm_mullo_epi16(xmm3, mv2g);
			xmm0 = _mm_adds_epi16(xmm0, xmm3); // g

			xmm4 = _mm_loadu_si128((__m128i *)(srcY+2*j));
			//xmm4 = _mm_loadl_epi64((__m128i *)(srcY+2*j));
			xmm4 = _mm_subs_epu8(xmm4, Mask_016); // unsigned
			xmm5 = xmm4;
			xmm4 = _mm_unpacklo_epi8(xmm4, xmm7);
			xmm5 = _mm_unpackhi_epi8(xmm5, xmm7);
			//xmm6 = xmm4;
			//xmm4 = _mm_unpacklo_epi64(xmm4, xmm5);
			//xmm5 = _mm_unpackhi_epi64(xmm6, xmm5);

			xmm4 = _mm_mullo_epi16(xmm4, my2rgb);
			xmm5 = _mm_mullo_epi16(xmm5, my2rgb);
			xmm4 = _mm_adds_epi16(xmm4, maddrgb);
			xmm5 = _mm_adds_epi16(xmm5, maddrgb);

			xmm3 = xmm0;
			xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
			xmm3 = _mm_unpackhi_epi16(xmm3, xmm3);
			xmm0 = _mm_adds_epi16(xmm0, xmm4);
			xmm3 = _mm_adds_epi16(xmm3, xmm5);
			xmm0 = _mm_srai_epi16(xmm0, 6);
			xmm3 = _mm_srai_epi16(xmm3, 6);
			xmm0 = _mm_packus_epi16(xmm0, xmm3); // g

			xmm3 = xmm1;
			xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
			xmm3 = _mm_unpackhi_epi16(xmm3, xmm3);
			xmm1 = _mm_adds_epi16(xmm1, xmm4);
			xmm3 = _mm_adds_epi16(xmm3, xmm5);
			xmm1 = _mm_srai_epi16(xmm1, 6);
			xmm3 = _mm_srai_epi16(xmm3, 6);
			xmm1 = _mm_packus_epi16(xmm1, xmm3); // r

			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
			xmm3 = _mm_unpackhi_epi16(xmm3, xmm3);
			xmm2 = _mm_adds_epi16(xmm2, xmm4);
			xmm3 = _mm_adds_epi16(xmm3, xmm5);
			xmm2 = _mm_srai_epi16(xmm2, 6);
			xmm3 = _mm_srai_epi16(xmm3, 6);
			xmm2 = _mm_packus_epi16(xmm2, xmm3); // b

			xmm5 = xmm0;
			xmm3 = xmm2;
			xmm4 = xmm1;
			xmm0 = _mm_and_si128(xmm0, Mask0);
			xmm2 = _mm_and_si128(xmm2, Mask1);
			xmm2 = _mm_or_si128(xmm2, xmm0);

			xmm5 = _mm_and_si128(xmm5, Mask1);
			xmm4 = _mm_and_si128(xmm4, Mask0);
			xmm5 = _mm_or_si128(xmm5, xmm4);

			xmm7 = xmm2;
			xmm2 = _mm_unpacklo_epi8(xmm2, xmm5);
			xmm7 = _mm_unpackhi_epi8(xmm7, xmm5);
			xmm6 = xmm1;
			xmm0 = xmm2;
			xmm3 = _mm_srli_epi64(xmm3, 8);
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm3);
			xmm6 = _mm_unpackhi_epi8(xmm6, xmm3);
			xmm6 = _mm_and_si128(xmm6, Mask2);
			xmm1 = _mm_and_si128(xmm1, Mask2);
			xmm0 = _mm_unpacklo_epi16(xmm0, xmm1);
			xmm2 = _mm_unpackhi_epi16(xmm2, xmm1);
			xmm1 = xmm0;
			xmm3 = xmm2;
			xmm1 = _mm_srli_si128(xmm1, 8);
			xmm1 = _mm_slli_si128(xmm1, 6);
			xmm0 = _mm_slli_si128(xmm0, 10);
			xmm0 = _mm_srli_si128(xmm0, 10);
			xmm3 = _mm_srli_si128(xmm3, 8);
			xmm3 = _mm_slli_si128(xmm3, 6);
			xmm2 = _mm_slli_si128(xmm2, 10);
			xmm2 = _mm_srli_si128(xmm2, 10);
			xmm0 = _mm_or_si128(xmm0, xmm1); // 1
			xmm2 = _mm_or_si128(xmm2, xmm3); // 2
			xmm3 = xmm7;
			xmm3 = _mm_unpacklo_epi16(xmm3, xmm6);
			xmm7 = _mm_unpackhi_epi16(xmm7, xmm6);
			xmm4 = xmm3;
			xmm5 = xmm7;
			xmm4 = _mm_srli_si128(xmm4, 8);
			xmm4 = _mm_slli_si128(xmm4, 6);
			xmm3 = _mm_slli_si128(xmm3, 10);
			xmm3 = _mm_srli_si128(xmm3, 10);
			xmm5 = _mm_srli_si128(xmm5, 8);
			xmm5 = _mm_slli_si128(xmm5, 6);
			xmm7 = _mm_slli_si128(xmm7, 10);
			xmm7 = _mm_srli_si128(xmm7, 10);
			xmm4 = _mm_or_si128(xmm4, xmm3); // 3
			xmm5 = _mm_or_si128(xmm5, xmm7); // 4
			xmm3 = xmm2;
			xmm3 = _mm_slli_si128(xmm3, 12);
			xmm0 = _mm_or_si128(xmm0, xmm3); // 1
			xmm2 = _mm_srli_si128(xmm2, 4);
			xmm3 = xmm4;
			xmm3 = _mm_slli_si128(xmm3, 8);
			xmm2 = _mm_or_si128(xmm2, xmm3); // 2
			xmm4 = _mm_srli_si128(xmm4, 8);
			xmm5 = _mm_slli_si128(xmm5, 4);
			xmm4 = _mm_or_si128(xmm4, xmm5); // 3

			_mm_storeu_si128((__m128i*)(dst +  0), xmm0);
			_mm_storeu_si128((__m128i*)(dst + 16), xmm2);
			_mm_storeu_si128((__m128i*)(dst + 32), xmm4);

			dst += 48;
		}
		for(; j<(srcWidth>>1); j++)	{
			int u, v, y;
			int rr, gg, bb, yy;
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j*2+0]-16;
			makeRGB(y, rr, gg, bb, dst[2], dst[1], dst[0], yy);

			y = srcY[j*2+1]-16;
			makeRGB(y, rr, gg, bb, dst[5], dst[4], dst[3], yy);
			dst+= 6;
		}
		dst+= (dstWidth-3*srcWidth);
		srcY-= srcWidth;
		srcU-= srcWidth/2*(i&1);
		srcV-= srcWidth/2*(i&1);
	}
	_mm_empty();
	return true;
}

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to bitmap-32 format. SSE2 version.
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
 *  \date    26-11-2002		Created by SMirovK
 ******************************************************************************
 */
bool CVSVideoProc_SSE2::ConvertI420ToBMF32(BYTE* srcY, BYTE* srcU, BYTE* srcV,
									  BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;

	int i ,j;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm7;
	const __m128i Mask0		= _mm_set1_epi32(0xFF00FF00),
				  Mask1		= _mm_set1_epi32(0x00FF00FF),
				  Mask2		= _mm_set1_epi32(0x0000FFFF),
				  Mask_016	= _mm_set1_epi32(0x10101010),
				  Mask_128	= _mm_set1_epi32(0x00800080),
				  mv2r		= _mm_set1_epi16(V2R>>2),
				  mu2g		= _mm_set1_epi16(U2G>>2),
				  mv2g		= _mm_set1_epi16(V2G>>2),
				  my2rgb	= _mm_set1_epi16(Y2RGB>>2),
				  maddrgb	= _mm_set1_epi16(ADD_RGB>>2);

	srcY += srcWidth * (height - 1);
	srcU += srcWidth / 2 * (height / 2 - 1);
	srcV += srcWidth / 2 * (height / 2 - 1);

	for (i = 0; i < height; i++) {
		//int W1 = (srcWidth &~ 7) >> 1;
		//int W1 = srcWidth / 16 * 16;
		int W1 = (srcWidth / 16 * 16) >> 1;
		//W1 = W1 / 16 * 16;
		for (j = 0; j < W1; j += 8) {
		//for (j = 0; j < W1; j += 4) {
			xmm7 = _mm_setzero_si128();
			xmm0 = _mm_loadl_epi64((__m128i *)(srcU+j));
			xmm1 = _mm_loadl_epi64((__m128i *)(srcV+j));
			//xmm0 = _mm_cvtsi32_si128(*(int*)(srcU+j));
			//xmm1 = _mm_cvtsi32_si128(*(int*)(srcV+j));
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm7);
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm7);
			xmm0 = _mm_subs_epi16(xmm0, Mask_128);
			xmm1 = _mm_subs_epi16(xmm1, Mask_128);
						xmm2 = xmm0;
			xmm3 = xmm1;

			xmm0 = _mm_mullo_epi16(xmm0, mu2g);
			xmm2 = _mm_slli_epi16(xmm2, 7); // b
			xmm1 = _mm_mullo_epi16(xmm1, mv2r); // r
			xmm3 = _mm_mullo_epi16(xmm3, mv2g);
			xmm0 = _mm_adds_epi16(xmm0, xmm3); // g

			xmm4 = _mm_loadu_si128((__m128i *)(srcY+2*j));
			//xmm4 = _mm_loadl_epi64((__m128i *)(srcY+2*j));
			xmm4 = _mm_subs_epu8(xmm4, Mask_016); // unsigned
			xmm5 = xmm4;
			xmm4 = _mm_unpacklo_epi8(xmm4, xmm7);
			xmm5 = _mm_unpackhi_epi8(xmm5, xmm7);
			//xmm6 = xmm4;
			//xmm4 = _mm_unpacklo_epi64(xmm4, xmm5);
			//xmm5 = _mm_unpackhi_epi64(xmm6, xmm5);

			xmm4 = _mm_mullo_epi16(xmm4, my2rgb);
			xmm5 = _mm_mullo_epi16(xmm5, my2rgb);
			xmm4 = _mm_adds_epi16(xmm4, maddrgb);
			xmm5 = _mm_adds_epi16(xmm5, maddrgb);

			xmm3 = xmm0;
			xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
			xmm3 = _mm_unpackhi_epi16(xmm3, xmm3);
			xmm0 = _mm_adds_epi16(xmm0, xmm4);
			xmm3 = _mm_adds_epi16(xmm3, xmm5);
			xmm0 = _mm_srai_epi16(xmm0, 6);
			xmm3 = _mm_srai_epi16(xmm3, 6);
			xmm0 = _mm_packus_epi16(xmm0, xmm3); // g

			xmm3 = xmm1;
			xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
			xmm3 = _mm_unpackhi_epi16(xmm3, xmm3);
			xmm1 = _mm_adds_epi16(xmm1, xmm4);
			xmm3 = _mm_adds_epi16(xmm3, xmm5);
			xmm1 = _mm_srai_epi16(xmm1, 6);
			xmm3 = _mm_srai_epi16(xmm3, 6);
			xmm1 = _mm_packus_epi16(xmm1, xmm3); // r

			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
			xmm3 = _mm_unpackhi_epi16(xmm3, xmm3);
			xmm2 = _mm_adds_epi16(xmm2, xmm4);
			xmm3 = _mm_adds_epi16(xmm3, xmm5);
			xmm2 = _mm_srai_epi16(xmm2, 6);
			xmm3 = _mm_srai_epi16(xmm3, 6);
			xmm2 = _mm_packus_epi16(xmm2, xmm3); // b

			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi8(xmm2, xmm0); //bgbgbgbg
			xmm3 = _mm_unpackhi_epi8(xmm3, xmm0);

			xmm4 = xmm1;
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm1); //r0r0r0r0
			xmm4 = _mm_unpackhi_epi8(xmm4, xmm4);

			xmm0 = xmm2;
			xmm0 = _mm_unpacklo_epi16(xmm0, xmm1); //rgb0rgb0
			xmm2 = _mm_unpackhi_epi16(xmm2, xmm1);

			xmm1 = xmm3;
			xmm1 = _mm_unpacklo_epi16(xmm1, xmm4);
			xmm3 = _mm_unpackhi_epi16(xmm3, xmm4);

			_mm_storeu_si128((__m128i*)(dst + 0), xmm0);
			_mm_storeu_si128((__m128i*)(dst +16), xmm2);
			_mm_storeu_si128((__m128i*)(dst +32), xmm1);
			_mm_storeu_si128((__m128i*)(dst +48), xmm3);

			dst+=64;
		}
		for(; j<(srcWidth>>1); j++)	{
			int u, v, y;
			int rr, gg, bb, yy;
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j*2+0]-16;
			makeRGB(y, rr, gg, bb, dst[2], dst[1], dst[0], yy);

			y = srcY[j*2+1]-16;
			makeRGB(y, rr, gg, bb, dst[6], dst[5], dst[4], yy);
			dst+= 8;
		}
		dst+= (dstWidth-4*srcWidth);
		srcY-= srcWidth;
		srcU-= srcWidth/2*(i&1);
		srcV-= srcWidth/2*(i&1);
	}
	_m_empty();
	return true;
}

/**
 ******************************************************************************
 * Convert image from YUV(4:2:0) to bitmap-32 format. SSE2 version. Vertical flip
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
 *  \date    26-11-2002		Created by SMirovK
 ******************************************************************************
 */
bool CVSVideoProc_SSE2::ConvertI420ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV,
												BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;

	int i ,j;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm7;
	const __m128i Mask0		= _mm_set1_epi32(0xFF00FF00),
				  Mask1		= _mm_set1_epi32(0x00FF00FF),
				  Mask2		= _mm_set1_epi32(0x0000FFFF),
				  Mask_016	= _mm_set1_epi32(0x10101010),
				  Mask_128	= _mm_set1_epi32(0x00800080),
				  mv2r		= _mm_set1_epi16(V2R>>2),
				  mu2g		= _mm_set1_epi16(U2G>>2),
				  mv2g		= _mm_set1_epi16(V2G>>2),
				  my2rgb	= _mm_set1_epi16(Y2RGB>>2),
				  maddrgb	= _mm_set1_epi16(ADD_RGB>>2);

	for (i = 0; i < height; i++) {
		//int W1 = (srcWidth &~ 7) >> 1;
		//int W1 = srcWidth / 16 * 16;
		int W1 = (srcWidth / 16 * 16) >> 1;
		//W1 = W1 / 16 * 16;
		for (j = 0; j < W1; j += 8) {
		//for (j = 0; j < W1; j += 4) {
			xmm7 = _mm_setzero_si128();
			xmm0 = _mm_loadl_epi64((__m128i *)(srcU+j));
			xmm1 = _mm_loadl_epi64((__m128i *)(srcV+j));
			//xmm0 = _mm_cvtsi32_si128(*(int*)(srcU+j));
			//xmm1 = _mm_cvtsi32_si128(*(int*)(srcV+j));
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm7);
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm7);
			xmm0 = _mm_subs_epi16(xmm0, Mask_128);
			xmm1 = _mm_subs_epi16(xmm1, Mask_128);
						xmm2 = xmm0;
			xmm3 = xmm1;

			xmm0 = _mm_mullo_epi16(xmm0, mu2g);
			xmm2 = _mm_slli_epi16(xmm2, 7); // b
			xmm1 = _mm_mullo_epi16(xmm1, mv2r); // r
			xmm3 = _mm_mullo_epi16(xmm3, mv2g);
			xmm0 = _mm_adds_epi16(xmm0, xmm3); // g

			xmm4 = _mm_loadu_si128((__m128i *)(srcY+2*j));
			//xmm4 = _mm_loadl_epi64((__m128i *)(srcY+2*j));
			xmm4 = _mm_subs_epu8(xmm4, Mask_016); // unsigned
			xmm5 = xmm4;
			xmm4 = _mm_unpacklo_epi8(xmm4, xmm7);
			xmm5 = _mm_unpackhi_epi8(xmm5, xmm7);
			//xmm6 = xmm4;
			//xmm4 = _mm_unpacklo_epi64(xmm4, xmm5);
			//xmm5 = _mm_unpackhi_epi64(xmm6, xmm5);

			xmm4 = _mm_mullo_epi16(xmm4, my2rgb);
			xmm5 = _mm_mullo_epi16(xmm5, my2rgb);
			xmm4 = _mm_adds_epi16(xmm4, maddrgb);
			xmm5 = _mm_adds_epi16(xmm5, maddrgb);

			xmm3 = xmm0;
			xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
			xmm3 = _mm_unpackhi_epi16(xmm3, xmm3);
			xmm0 = _mm_adds_epi16(xmm0, xmm4);
			xmm3 = _mm_adds_epi16(xmm3, xmm5);
			xmm0 = _mm_srai_epi16(xmm0, 6);
			xmm3 = _mm_srai_epi16(xmm3, 6);
			xmm0 = _mm_packus_epi16(xmm0, xmm3); // g

			xmm3 = xmm1;
			xmm1 = _mm_unpacklo_epi16(xmm1, xmm1);
			xmm3 = _mm_unpackhi_epi16(xmm3, xmm3);
			xmm1 = _mm_adds_epi16(xmm1, xmm4);
			xmm3 = _mm_adds_epi16(xmm3, xmm5);
			xmm1 = _mm_srai_epi16(xmm1, 6);
			xmm3 = _mm_srai_epi16(xmm3, 6);
			xmm1 = _mm_packus_epi16(xmm1, xmm3); // r

			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, xmm2);
			xmm3 = _mm_unpackhi_epi16(xmm3, xmm3);
			xmm2 = _mm_adds_epi16(xmm2, xmm4);
			xmm3 = _mm_adds_epi16(xmm3, xmm5);
			xmm2 = _mm_srai_epi16(xmm2, 6);
			xmm3 = _mm_srai_epi16(xmm3, 6);
			xmm2 = _mm_packus_epi16(xmm2, xmm3); // b

			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi8(xmm2, xmm0); //bgbgbgbg
			xmm3 = _mm_unpackhi_epi8(xmm3, xmm0);

			xmm4 = xmm1;
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm1); //r0r0r0r0
			xmm4 = _mm_unpackhi_epi8(xmm4, xmm4);

			xmm0 = xmm2;
			xmm0 = _mm_unpacklo_epi16(xmm0, xmm1); //rgb0rgb0
			xmm2 = _mm_unpackhi_epi16(xmm2, xmm1);

			xmm1 = xmm3;
			xmm1 = _mm_unpacklo_epi16(xmm1, xmm4);
			xmm3 = _mm_unpackhi_epi16(xmm3, xmm4);

			_mm_storeu_si128((__m128i*)(dst + 0), xmm0);
			_mm_storeu_si128((__m128i*)(dst +16), xmm2);
			_mm_storeu_si128((__m128i*)(dst +32), xmm1);
			_mm_storeu_si128((__m128i*)(dst +48), xmm3);

			dst+=64;
		}
		for(; j<(srcWidth>>1); j++)	{
			int u, v, y;
			int rr, gg, bb, yy;
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j*2+0]-16;
			makeRGB(y, rr, gg, bb, dst[2], dst[1], dst[0], yy);

			y = srcY[j*2+1]-16;
			makeRGB(y, rr, gg, bb, dst[6], dst[5], dst[4], yy);
			dst+= 8;
		}
		dst+= (dstWidth-4*srcWidth);
		srcY+= srcWidth;
		srcU+= srcWidth/2*(i&1);
		srcV+= srcWidth/2*(i&1);
	}
	_m_empty();
	return true;
}

bool CVSVideoProc_SSE2::ConvertYUV444ToBMF32_Vflip(BYTE* srcY, BYTE* srcU, BYTE* srcV,
												   BYTE *dst, int srcWidth, int height, int dstWidth)
{
	if (srcWidth&0x1) return false;

	int i ,j;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm7 = _mm_setzero_si128();
	const __m128i Mask0		= _mm_set1_epi32(0xFF00FF00),
				  Mask1		= _mm_set1_epi32(0x00FF00FF),
				  Mask2		= _mm_set1_epi32(0x0000FFFF),
				  Mask_016	= _mm_set1_epi32(0x10101010),
				  Mask_128	= _mm_set1_epi32(0x00800080),
				  mv2r		= _mm_set1_epi16(V2R>>2),
				  mu2g		= _mm_set1_epi16(U2G>>2),
				  mv2g		= _mm_set1_epi16(V2G>>2),
				  my2rgb	= _mm_set1_epi16(Y2RGB>>2),
				  maddrgb	= _mm_set1_epi16(ADD_RGB>>2);

	for (i = 0; i < height; i++) {
		int W1 = (srcWidth &~ 7);
		for (j = 0; j < W1; j += 8) {
			xmm0 = _mm_loadl_epi64((__m128i *)(srcU+j));
			xmm1 = _mm_loadl_epi64((__m128i *)(srcV+j));
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm7);
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm7);
			xmm0 = _mm_subs_epi16(xmm0, Mask_128);
			xmm1 = _mm_subs_epi16(xmm1, Mask_128);
			xmm2 = xmm0;
			xmm3 = xmm1;

			xmm0 = _mm_mullo_epi16(xmm0, mu2g);
			xmm2 = _mm_slli_epi16(xmm2, 7); // b
			xmm1 = _mm_mullo_epi16(xmm1, mv2r); // r
			xmm3 = _mm_mullo_epi16(xmm3, mv2g);
			xmm0 = _mm_adds_epi16(xmm0, xmm3); // g

			xmm4 = _mm_loadl_epi64((__m128i *)(srcY+j));
			xmm4 = _mm_subs_epu8(xmm4, Mask_016); // unsigned
			xmm4 = _mm_unpacklo_epi8(xmm4, xmm7);
			xmm4 = _mm_mullo_epi16(xmm4, my2rgb);
			xmm4 = _mm_add_epi16(xmm4, maddrgb); // unsigned

			xmm0 = _mm_adds_epi16(xmm0, xmm4);
			xmm0 = _mm_srai_epi16(xmm0, 6);
			xmm0 = _mm_packus_epi16(xmm0, xmm7); // g

			xmm1 = _mm_adds_epi16(xmm1, xmm4);
			xmm1 = _mm_srai_epi16(xmm1, 6);
			xmm1 = _mm_packus_epi16(xmm1, xmm7); // r

			xmm2 = _mm_adds_epi16(xmm2, xmm4);
			xmm2 = _mm_srai_epi16(xmm2, 6);
			xmm2 = _mm_packus_epi16(xmm2, xmm7); // b

			xmm2 = _mm_unpacklo_epi8(xmm2, xmm0); // bgbgbgbgbgbgbgbg
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm7); // r0r0r0r0r0r0r0r0
			xmm0 = xmm2;
			xmm0 = _mm_unpacklo_epi16(xmm0, xmm1); // rgb0rgb0rgb0rgb0
			xmm2 = _mm_unpackhi_epi16(xmm2, xmm1);

			_mm_storeu_si128((__m128i*)(dst + 0), xmm0);
			_mm_storeu_si128((__m128i*)(dst + 16), xmm2);

			dst += 32;
		}
		for (; j < srcWidth; j++) {
			int u, v, y;
			int rr, gg, bb, yy;
			u = srcU[j]-128;
			v = srcV[j]-128;
			makeRRGGBB(u, v, rr, gg, bb);

			y = srcY[j]-16;
			makeRGB(y, rr, gg, bb, dst[2], dst[1], dst[0], yy);

			dst += 4;
		}
		dst += (dstWidth - 4 * srcWidth);
		srcY += srcWidth;
		srcU += srcWidth;
		srcV += srcWidth;
	}
	return true;
}

bool CVSVideoProc_SSE2::ConvertUYVYToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV,
										  int srcWidth, int height, int dstWidth)
{
	if (srcWidth & 0x3 || height & 1) return false;

	const __m128i mask0 = _mm_set1_epi16(0x00FF);
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;

	int i, j, W;
	BYTE *src2, *dstY2;
	W = (srcWidth &~ 31);

	for(i = 0; i < height; i += 2) {
		src2 = src + srcWidth;
		dstY2 = dstY + dstWidth;
		for (j = 0; j < W / 32; j++)	{
			xmm0 = _mm_loadu_si128((__m128i *)(src + j * 32 + 0));
			xmm1 = _mm_loadu_si128((__m128i *)(src + j * 32 + 16));
			// Y
			xmm4 = xmm0;
			xmm0 = _mm_srli_si128(xmm0, 1);
			xmm5 = xmm1;
			xmm1 = _mm_srli_si128(xmm1, 1);
			xmm0 = _mm_and_si128(xmm0, mask0);
			xmm1 = _mm_and_si128(xmm1, mask0);
			xmm0 = _mm_packus_epi16(xmm0, xmm1);
			// UV
			xmm4 = _mm_and_si128(xmm4, mask0);
			xmm5 = _mm_and_si128(xmm5, mask0);
			xmm4 = _mm_shuffle_epi32(xmm4, _MM_SHUFFLE(3, 1, 2, 0));
			xmm5 = _mm_shuffle_epi32(xmm5, _MM_SHUFFLE(3, 1, 2, 0));
			xmm1 = xmm4;
			xmm1 = _mm_unpacklo_epi64(xmm1, xmm5);
			xmm4 = _mm_unpackhi_epi64(xmm4, xmm5);
			xmm1 = _mm_packus_epi16(xmm1, xmm1);
			xmm4 = _mm_packus_epi16(xmm4, xmm4);
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm4);
			xmm1 = _mm_shufflelo_epi16(xmm1, _MM_SHUFFLE(3, 1, 2, 0));
			xmm1 = _mm_shufflehi_epi16(xmm1, _MM_SHUFFLE(3, 1, 2, 0));
			xmm1 = _mm_shuffle_epi32(xmm1, _MM_SHUFFLE(3, 1, 2, 0));

			xmm2 = _mm_loadu_si128((__m128i *)(src2 + j * 32 + 0));
			xmm3 = _mm_loadu_si128((__m128i *)(src2 + j * 32 + 16));
			// Y
			xmm4 = xmm2;
			xmm2 = _mm_srli_si128(xmm2, 1);
			xmm5 = xmm3;
			xmm3 = _mm_srli_si128(xmm3, 1);
			xmm2 = _mm_and_si128(xmm2, mask0);
			xmm3 = _mm_and_si128(xmm3, mask0);
			xmm2 = _mm_packus_epi16(xmm2, xmm3);
			// UV
			xmm4 = _mm_and_si128(xmm4, mask0);
			xmm5 = _mm_and_si128(xmm5, mask0);
			xmm4 = _mm_shuffle_epi32(xmm4, _MM_SHUFFLE(3, 1, 2, 0));
			xmm5 = _mm_shuffle_epi32(xmm5, _MM_SHUFFLE(3, 1, 2, 0));
			xmm3 = xmm4;
			xmm3 = _mm_unpacklo_epi64(xmm3, xmm5);
			xmm4 = _mm_unpackhi_epi64(xmm4, xmm5);
			xmm3 = _mm_packus_epi16(xmm3, xmm3);
			xmm4 = _mm_packus_epi16(xmm4, xmm4);
			xmm3 = _mm_unpacklo_epi8(xmm3, xmm4);
			xmm3 = _mm_shufflelo_epi16(xmm3, _MM_SHUFFLE(3, 1, 2, 0));
			xmm3 = _mm_shufflehi_epi16(xmm3, _MM_SHUFFLE(3, 1, 2, 0));
			xmm3 = _mm_shuffle_epi32(xmm3, _MM_SHUFFLE(3, 1, 2, 0));

			xmm1 = _mm_avg_epu8(xmm1, xmm3);

			_mm_storeu_si128((__m128i*)(dstY  + j * 16), xmm0);
			_mm_storeu_si128((__m128i*)(dstY2 + j * 16), xmm2);
			_mm_storel_epi64((__m128i*)(dstU + j * 8), xmm1);
			xmm1 = _mm_srli_si128(xmm1, 8);
			_mm_storel_epi64((__m128i*)(dstV + j * 8), xmm1);
		}
		j = (j * 32) / 4;
		for (; j < srcWidth / 4; j++)	{
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

bool CVSVideoProc_SSE2::ConvertBMF24ToI420(BYTE* Src, BYTE* DstY, BYTE* DstU, BYTE *DstV,
										  int srcW, int h, int dstW)
{
    int i, j, ofs;
	int dstW4 = dstW / 4 * 4;
	int d = dstW - dstW4;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm_null = _mm_setzero_si128();
	const __m128i ry = _mm_setr_epi16(B2Y, G2Y, R2Y, 0, B2Y, G2Y, R2Y, 0),
				  ru = _mm_setr_epi16(B2U, G2U, R2U, 0, B2U, G2U, R2U, 0),
				  rv = _mm_setr_epi16(B2V, G2V, R2V, 0, B2V, G2V, R2V, 0),
				  mask0 = _mm_setr_epi32(0x00ffffff, 0x0, 0x0, 0x0),
				  mask1 = _mm_setr_epi32(0xff000000, 0x0000ffff, 0x0, 0x0),
				  mask2 = _mm_setr_epi32(0x0, 0xffff0000, 0x000000ff, 0x0),
				  mask3 = _mm_setr_epi32(0x0, 0x0, 0xffffff00, 0x0),
				  mask4 = _mm_setr_epi32(0xffffffff, 0x0, 0xffffffff, 0x0),
				  add_y = _mm_set1_epi32(ADD_Y),
				  add_uv = _mm_set1_epi32(ADD_UV*4);

	unsigned char *pY = DstY, *pU = DstU, *pV = DstV, *pRGB24;

	pY += (h - 1) * dstW;
	pU += (h/2 - 1) * dstW/2;
	pV += (h/2 - 1) * dstW/2;
	pRGB24 = Src;

    for (i = 0; i < (h >> 1); i++)
	{
		ofs = 0;
		for (j = 0; j < (dstW4 >> 1) - 2; j += 2)
		{
			xmm0 = _mm_loadu_si128((__m128i *)(pRGB24 + ofs));
			xmm1 = xmm0;
			xmm2 = xmm0;
			xmm3 = xmm0;
			xmm0 = _mm_and_si128(xmm0, mask0);
			xmm1 = _mm_and_si128(xmm1, mask1);
			xmm2 = _mm_and_si128(xmm2, mask2);
			xmm3 = _mm_and_si128(xmm3, mask3);
			xmm1 = _mm_slli_si128(xmm1, 1);
			xmm2 = _mm_slli_si128(xmm2, 2);
			xmm3 = _mm_slli_si128(xmm3, 3);
			xmm0 = _mm_or_si128(xmm0, xmm1);
			xmm0 = _mm_or_si128(xmm0, xmm2);
			xmm0 = _mm_or_si128(xmm0, xmm3);
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm_null);
			xmm1 = _mm_unpackhi_epi8(xmm1, xmm_null);

			xmm2 = _mm_loadu_si128((__m128i *)(pRGB24 + ofs + srcW));
			xmm3 = xmm2;
			xmm4 = xmm2;
			xmm5 = xmm2;
			xmm2 = _mm_and_si128(xmm2, mask0);
			xmm3 = _mm_and_si128(xmm3, mask1);
			xmm4 = _mm_and_si128(xmm4, mask2);
			xmm5 = _mm_and_si128(xmm5, mask3);
			xmm3 = _mm_slli_si128(xmm3, 1);
			xmm4 = _mm_slli_si128(xmm4, 2);
			xmm5 = _mm_slli_si128(xmm5, 3);
			xmm2 = _mm_or_si128(xmm2, xmm3);
			xmm2 = _mm_or_si128(xmm2, xmm4);
			xmm2 = _mm_or_si128(xmm2, xmm5);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi8(xmm2, xmm_null);
			xmm3 = _mm_unpackhi_epi8(xmm3, xmm_null);

			xmm4 = xmm0;
			xmm6 = xmm1;
			xmm4 = _mm_adds_epi16(xmm4, xmm2);
			xmm6 = _mm_adds_epi16(xmm6, xmm3);
			xmm5 = xmm4;
			xmm5 = _mm_srli_si128(xmm5, 8);
			xmm4 = _mm_adds_epi16(xmm4, xmm5);
			xmm5 = xmm6;
			xmm5 = _mm_srli_si128(xmm5, 8);
			xmm6 = _mm_adds_epi16(xmm6, xmm5);
			xmm4 = _mm_unpacklo_epi64(xmm4, xmm6);

			xmm0 = _mm_madd_epi16(xmm0, ry);
			xmm1 = _mm_madd_epi16(xmm1, ry);
			xmm2 = _mm_madd_epi16(xmm2, ry);
			xmm3 = _mm_madd_epi16(xmm3, ry);

			xmm5 = xmm0;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm0 = _mm_add_epi32(xmm0, xmm5);
			xmm5 = xmm1;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm1 = _mm_add_epi32(xmm1, xmm5);
			xmm5 = xmm2;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm2 = _mm_add_epi32(xmm2, xmm5);
			xmm5 = xmm3;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm3 = _mm_add_epi32(xmm3, xmm5);

			xmm0 = _mm_add_epi32(xmm0, add_y);
			xmm1 = _mm_add_epi32(xmm1, add_y);
			xmm2 = _mm_add_epi32(xmm2, add_y);
			xmm3 = _mm_add_epi32(xmm3, add_y);
			xmm0 = _mm_srai_epi32(xmm0, 8);
			xmm1 = _mm_srai_epi32(xmm1, 8);
			xmm2 = _mm_srai_epi32(xmm2, 8);
			xmm3 = _mm_srai_epi32(xmm3, 8);
			xmm0 = _mm_and_si128(xmm0, mask4);
			xmm1 = _mm_and_si128(xmm1, mask4);
			xmm2 = _mm_and_si128(xmm2, mask4);
			xmm3 = _mm_and_si128(xmm3, mask4);
			xmm0 = _mm_packs_epi32(xmm0, xmm1);
			xmm2 = _mm_packs_epi32(xmm2, xmm3);
			xmm0 = _mm_packs_epi32(xmm0, xmm_null);
			xmm2 = _mm_packs_epi32(xmm2, xmm_null);
			xmm0 = _mm_packus_epi16(xmm0, xmm_null);
			xmm2 = _mm_packus_epi16(xmm2, xmm_null);

			*(int*)(pY + j * 2) = _mm_cvtsi128_si32(xmm0);
			*(int*)(pY + j * 2 - dstW) = _mm_cvtsi128_si32(xmm2);

			xmm0 = xmm4;
			xmm0 = _mm_madd_epi16(xmm0, ru);
			xmm4 = _mm_madd_epi16(xmm4, rv);
			xmm5 = xmm0;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm0 = _mm_add_epi32(xmm0, xmm5);
			xmm5 = xmm4;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm0 = _mm_add_epi32(xmm0, add_uv);
			xmm4 = _mm_add_epi32(xmm4, add_uv);
			xmm0 = _mm_srai_epi32(xmm0, 10);
			xmm4 = _mm_srai_epi32(xmm4, 10);
			xmm0 = _mm_and_si128(xmm0, mask4);
			xmm4 = _mm_and_si128(xmm4, mask4);
			xmm0 = _mm_packs_epi32(xmm0, xmm4);
			xmm0 = _mm_packs_epi32(xmm0, xmm_null);
			xmm0 = _mm_packus_epi16(xmm0, xmm_null);

			*(short*)(pU + j) = (short)(_mm_cvtsi128_si32(xmm0));
			xmm0 = _mm_srli_si128(xmm0, 2);
			*(short*)(pV + j) = (short)(_mm_cvtsi128_si32(xmm0));

			ofs += 12;
        }

		ofs -= 4;

		xmm0 = _mm_loadu_si128((__m128i *)(pRGB24 + ofs));
		xmm0 = _mm_srli_si128(xmm0, 4);
		xmm1 = xmm0;
		xmm2 = xmm0;
		xmm3 = xmm0;
		xmm0 = _mm_and_si128(xmm0, mask0);
		xmm1 = _mm_and_si128(xmm1, mask1);
		xmm2 = _mm_and_si128(xmm2, mask2);
		xmm3 = _mm_and_si128(xmm3, mask3);
		xmm1 = _mm_slli_si128(xmm1, 1);
		xmm2 = _mm_slli_si128(xmm2, 2);
		xmm3 = _mm_slli_si128(xmm3, 3);
		xmm0 = _mm_or_si128(xmm0, xmm1);
		xmm0 = _mm_or_si128(xmm0, xmm2);
		xmm0 = _mm_or_si128(xmm0, xmm3);
		xmm1 = xmm0;
		xmm0 = _mm_unpacklo_epi8(xmm0, xmm_null);
		xmm1 = _mm_unpackhi_epi8(xmm1, xmm_null);

		xmm2 = _mm_loadu_si128((__m128i *)(pRGB24 + ofs + srcW));
		xmm2 = _mm_srli_si128(xmm2, 4);
		xmm3 = xmm2;
		xmm4 = xmm2;
		xmm5 = xmm2;
		xmm2 = _mm_and_si128(xmm2, mask0);
		xmm3 = _mm_and_si128(xmm3, mask1);
		xmm4 = _mm_and_si128(xmm4, mask2);
		xmm5 = _mm_and_si128(xmm5, mask3);
		xmm3 = _mm_slli_si128(xmm3, 1);
		xmm4 = _mm_slli_si128(xmm4, 2);
		xmm5 = _mm_slli_si128(xmm5, 3);
		xmm2 = _mm_or_si128(xmm2, xmm3);
		xmm2 = _mm_or_si128(xmm2, xmm4);
		xmm2 = _mm_or_si128(xmm2, xmm5);
		xmm3 = xmm2;
		xmm2 = _mm_unpacklo_epi8(xmm2, xmm_null);
		xmm3 = _mm_unpackhi_epi8(xmm3, xmm_null);

		xmm4 = xmm0;
		xmm6 = xmm1;
		xmm4 = _mm_adds_epi16(xmm4, xmm2);
		xmm6 = _mm_adds_epi16(xmm6, xmm3);
		xmm5 = xmm4;
		xmm5 = _mm_srli_si128(xmm5, 8);
		xmm4 = _mm_adds_epi16(xmm4, xmm5);
		xmm5 = xmm6;
		xmm5 = _mm_srli_si128(xmm5, 8);
		xmm6 = _mm_adds_epi16(xmm6, xmm5);
		xmm4 = _mm_unpacklo_epi64(xmm4, xmm6);

		xmm0 = _mm_madd_epi16(xmm0, ry);
		xmm1 = _mm_madd_epi16(xmm1, ry);
		xmm2 = _mm_madd_epi16(xmm2, ry);
		xmm3 = _mm_madd_epi16(xmm3, ry);

		xmm5 = xmm0;
		xmm5 = _mm_srli_si128(xmm5, 4);
		xmm0 = _mm_add_epi32(xmm0, xmm5);
		xmm5 = xmm1;
		xmm5 = _mm_srli_si128(xmm5, 4);
		xmm1 = _mm_add_epi32(xmm1, xmm5);
		xmm5 = xmm2;
		xmm5 = _mm_srli_si128(xmm5, 4);
		xmm2 = _mm_add_epi32(xmm2, xmm5);
		xmm5 = xmm3;
		xmm5 = _mm_srli_si128(xmm5, 4);
		xmm3 = _mm_add_epi32(xmm3, xmm5);

		xmm0 = _mm_add_epi32(xmm0, add_y);
		xmm1 = _mm_add_epi32(xmm1, add_y);
		xmm2 = _mm_add_epi32(xmm2, add_y);
		xmm3 = _mm_add_epi32(xmm3, add_y);
		xmm0 = _mm_srai_epi32(xmm0, 8);
		xmm1 = _mm_srai_epi32(xmm1, 8);
		xmm2 = _mm_srai_epi32(xmm2, 8);
		xmm3 = _mm_srai_epi32(xmm3, 8);
		xmm0 = _mm_and_si128(xmm0, mask4);
		xmm1 = _mm_and_si128(xmm1, mask4);
		xmm2 = _mm_and_si128(xmm2, mask4);
		xmm3 = _mm_and_si128(xmm3, mask4);
		xmm0 = _mm_packs_epi32(xmm0, xmm1);
		xmm2 = _mm_packs_epi32(xmm2, xmm3);
		xmm0 = _mm_packs_epi32(xmm0, xmm_null);
		xmm2 = _mm_packs_epi32(xmm2, xmm_null);
		xmm0 = _mm_packus_epi16(xmm0, xmm_null);
		xmm2 = _mm_packus_epi16(xmm2, xmm_null);

		*(int*)(pY + j * 2) = _mm_cvtsi128_si32(xmm0);
		*(int*)(pY + j * 2 - dstW) = _mm_cvtsi128_si32(xmm2);

		xmm0 = xmm4;
		xmm0 = _mm_madd_epi16(xmm0, ru);
		xmm4 = _mm_madd_epi16(xmm4, rv);
		xmm5 = xmm0;
		xmm5 = _mm_srli_si128(xmm5, 4);
		xmm0 = _mm_add_epi32(xmm0, xmm5);
		xmm5 = xmm4;
		xmm5 = _mm_srli_si128(xmm5, 4);
		xmm4 = _mm_add_epi32(xmm4, xmm5);

		xmm0 = _mm_add_epi32(xmm0, add_uv);
		xmm4 = _mm_add_epi32(xmm4, add_uv);
		xmm0 = _mm_srai_epi32(xmm0, 10);
		xmm4 = _mm_srai_epi32(xmm4, 10);
		xmm0 = _mm_and_si128(xmm0, mask4);
		xmm4 = _mm_and_si128(xmm4, mask4);
		xmm0 = _mm_packs_epi32(xmm0, xmm4);
		xmm0 = _mm_packs_epi32(xmm0, xmm_null);
		xmm0 = _mm_packus_epi16(xmm0, xmm_null);

		*(short*)(pU + j) = (short)(_mm_cvtsi128_si32(xmm0));
		xmm0 = _mm_srli_si128(xmm0, 2);
		*(short*)(pV + j) = (short)(_mm_cvtsi128_si32(xmm0));

		if (d > 0) {
			__m64 m0, m1, m2, m3, m4, m5, mmx_null = {0};
			const __m64 ry = _mm_setr_pi16(B2Y, G2Y, R2Y, 0),
						ru = _mm_setr_pi16(B2U, G2U, R2U, 0),
						rv = _mm_setr_pi16(B2V, G2V, R2V, 0);

			ofs += 14;
			j += 2;

			m0 = *(__m64*)(pRGB24 + ofs + 0);
			m0 = _m_psrlqi(m0, 16);
			m1 = m0;
			m0 = _m_punpcklbw(m0, mmx_null);
			m1 = _m_psrlqi(m1, 24);
			m1 = _m_punpcklbw(m1, mmx_null);
			m2 = *(__m64*)(pRGB24 + ofs + srcW + 0);
			m2 = _m_psrlqi(m2, 16);
			m3 = m2;
			m2 = _m_punpcklbw(m2, mmx_null);
			m3 = _m_psrlqi(m3, 24);
			m3 = _m_punpcklbw(m3, mmx_null);

			m4 = m0;
			m4 = _m_paddsw(m4, m1);
			m4 = _m_paddsw(m4, m2);
			m4 = _m_paddsw(m4, m3);

			m0 = _m_pmaddwd(m0, ry);
			m1 = _m_pmaddwd(m1, ry);
			m2 = _m_pmaddwd(m2, ry);
			m3 = _m_pmaddwd(m3, ry);

			m5 = m0;
			m5 = _m_psrlqi(m5, 32);
			m0 = _m_paddd(m0, m5);
			m5 = m1;
			m5 = _m_psrlqi(m5, 32);
			m1 = _m_paddd(m1, m5);
			m5 = m2;
			m5 = _m_psrlqi(m5, 32);
			m2 = _m_paddd(m2, m5);
			m5 = m3;
			m5 = _m_psrlqi(m5, 32);
			m3 = _m_paddd(m3, m5);

			pY[j*2-dstW+0	] = CLIP((_mm_cvtsi64_si32(m2) + ADD_Y) >> 8);
			pY[j*2-dstW+1	] = CLIP((_mm_cvtsi64_si32(m3) + ADD_Y) >> 8);
			pY[j*2+0		] = CLIP((_mm_cvtsi64_si32(m0) + ADD_Y) >> 8);
			pY[j*2+1		] = CLIP((_mm_cvtsi64_si32(m1) + ADD_Y) >> 8);

			m0 = m4;
			m0 = _m_pmaddwd(m0, ru);
			m4 = _m_pmaddwd(m4, rv);

			m5 = m0;
			m5 = _m_psrlqi(m5, 32);
			m0 = _m_paddd(m0, m5);
			m5 = m4;
			m5 = _m_psrlqi(m5, 32);
			m4 = _m_paddd(m4, m5);

			pU[j			] = CLIP((_mm_cvtsi64_si32(m0) + ADD_UV*4) >> 10);
			pV[j			] = CLIP((_mm_cvtsi64_si32(m4) + ADD_UV*4) >> 10);
		}

		pRGB24 += 2*srcW;
		pY -= 2*dstW;
		pU -= dstW/2;
		pV -= dstW/2;
    }

	_mm_empty();

	return true;
}

bool CVSVideoProc_SSE2::ConvertBMF32ToI420(BYTE* Src, BYTE* DstY, BYTE* DstU, BYTE *DstV,
										  int srcW, int h, int dstW)
{
    int i, j, ofs;
	int dstW4 = dstW / 4 * 4;
	int d = dstW - dstW4;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm_null = _mm_setzero_si128();
	const __m128i ry = _mm_setr_epi16(B2Y, G2Y, R2Y, 0, B2Y, G2Y, R2Y, 0),
				  ru = _mm_setr_epi16(B2U, G2U, R2U, 0, B2U, G2U, R2U, 0),
				  rv = _mm_setr_epi16(B2V, G2V, R2V, 0, B2V, G2V, R2V, 0),
				  mask4 = _mm_setr_epi32(0xffffffff, 0x0, 0xffffffff, 0x0),
				  add_y = _mm_set1_epi32(ADD_Y),
				  add_uv = _mm_set1_epi32(ADD_UV*4);

	unsigned char *pY = DstY, *pU = DstU, *pV = DstV, *pRGB32;

	pY += (h - 1) * dstW;
	pU += (h/2 - 1) * dstW/2;
	pV += (h/2 - 1) * dstW/2;
	pRGB32 = Src;

    for (i = 0; i < (h >> 1); i++)
	{
		ofs = 0;
		for (j = 0; j < (dstW4 >> 1); j += 2)
		{
			xmm0 = _mm_loadu_si128((__m128i *)(pRGB32 + ofs));
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm_null);
			xmm1 = _mm_unpackhi_epi8(xmm1, xmm_null);

			xmm2 = _mm_loadu_si128((__m128i *)(pRGB32 + ofs + srcW));
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi8(xmm2, xmm_null);
			xmm3 = _mm_unpackhi_epi8(xmm3, xmm_null);

			xmm4 = xmm0;
			xmm6 = xmm1;
			xmm4 = _mm_adds_epi16(xmm4, xmm2);
			xmm6 = _mm_adds_epi16(xmm6, xmm3);
			xmm5 = xmm4;
			xmm5 = _mm_srli_si128(xmm5, 8);
			xmm4 = _mm_adds_epi16(xmm4, xmm5);
			xmm5 = xmm6;
			xmm5 = _mm_srli_si128(xmm5, 8);
			xmm6 = _mm_adds_epi16(xmm6, xmm5);
			xmm4 = _mm_unpacklo_epi64(xmm4, xmm6);

			xmm0 = _mm_madd_epi16(xmm0, ry);
			xmm1 = _mm_madd_epi16(xmm1, ry);
			xmm2 = _mm_madd_epi16(xmm2, ry);
			xmm3 = _mm_madd_epi16(xmm3, ry);

			xmm5 = xmm0;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm0 = _mm_add_epi32(xmm0, xmm5);
			xmm5 = xmm1;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm1 = _mm_add_epi32(xmm1, xmm5);
			xmm5 = xmm2;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm2 = _mm_add_epi32(xmm2, xmm5);
			xmm5 = xmm3;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm3 = _mm_add_epi32(xmm3, xmm5);

			xmm0 = _mm_add_epi32(xmm0, add_y);
			xmm1 = _mm_add_epi32(xmm1, add_y);
			xmm2 = _mm_add_epi32(xmm2, add_y);
			xmm3 = _mm_add_epi32(xmm3, add_y);
			xmm0 = _mm_srai_epi32(xmm0, 8);
			xmm1 = _mm_srai_epi32(xmm1, 8);
			xmm2 = _mm_srai_epi32(xmm2, 8);
			xmm3 = _mm_srai_epi32(xmm3, 8);
			xmm0 = _mm_and_si128(xmm0, mask4);
			xmm1 = _mm_and_si128(xmm1, mask4);
			xmm2 = _mm_and_si128(xmm2, mask4);
			xmm3 = _mm_and_si128(xmm3, mask4);
			xmm0 = _mm_packs_epi32(xmm0, xmm1);
			xmm2 = _mm_packs_epi32(xmm2, xmm3);
			xmm0 = _mm_packs_epi32(xmm0, xmm_null);
			xmm2 = _mm_packs_epi32(xmm2, xmm_null);
			xmm0 = _mm_packus_epi16(xmm0, xmm_null);
			xmm2 = _mm_packus_epi16(xmm2, xmm_null);

			*(int*)(pY + j * 2) = _mm_cvtsi128_si32(xmm0);
			*(int*)(pY + j * 2 - dstW) = _mm_cvtsi128_si32(xmm2);

			xmm0 = xmm4;
			xmm0 = _mm_madd_epi16(xmm0, ru);
			xmm4 = _mm_madd_epi16(xmm4, rv);
			xmm5 = xmm0;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm0 = _mm_add_epi32(xmm0, xmm5);
			xmm5 = xmm4;
			xmm5 = _mm_srli_si128(xmm5, 4);
			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm0 = _mm_add_epi32(xmm0, add_uv);
			xmm4 = _mm_add_epi32(xmm4, add_uv);
			xmm0 = _mm_srai_epi32(xmm0, 10);
			xmm4 = _mm_srai_epi32(xmm4, 10);
			xmm0 = _mm_and_si128(xmm0, mask4);
			xmm4 = _mm_and_si128(xmm4, mask4);
			xmm0 = _mm_packs_epi32(xmm0, xmm4);
			xmm0 = _mm_packs_epi32(xmm0, xmm_null);
			xmm0 = _mm_packus_epi16(xmm0, xmm_null);

			*(short*)(pU + j) = (short)(_mm_cvtsi128_si32(xmm0));
			xmm0 = _mm_srli_si128(xmm0, 2);
			*(short*)(pV + j) = (short)(_mm_cvtsi128_si32(xmm0));

			ofs += 16;
        }

		if (d >= 2) { /// MMX bounds
			__m64 m0, m1, m2, m3, m4, m5, mmx_null = {0};
			const __m64 ry = _mm_setr_pi16(B2Y, G2Y, R2Y, 0),
						ru = _mm_setr_pi16(B2U, G2U, R2U, 0),
						rv = _mm_setr_pi16(B2V, G2V, R2V, 0);

			m0 = *(__m64*)(pRGB32 + ofs + 0);
			m1 = m0;
			m0 = _m_punpcklbw(m0, mmx_null);
			m1 = _m_punpckhbw(m1, mmx_null);
			m2 = *(__m64*)(pRGB32 + ofs + srcW + 0);
			m3 = m2;
			m2 = _m_punpcklbw(m2, mmx_null);
			m3 = _m_punpckhbw(m3, mmx_null);

			m4 = m0;
			m4 = _m_paddsw(m4, m1);
			m4 = _m_paddsw(m4, m2);
			m4 = _m_paddsw(m4, m3);

			m0 = _m_pmaddwd(m0, ry);
			m1 = _m_pmaddwd(m1, ry);
			m2 = _m_pmaddwd(m2, ry);
			m3 = _m_pmaddwd(m3, ry);

			m5 = m0;
			m5 = _m_psrlqi(m5, 32);
			m0 = _m_paddd(m0, m5);
			m5 = m1;
			m5 = _m_psrlqi(m5, 32);
			m1 = _m_paddd(m1, m5);
			m5 = m2;
			m5 = _m_psrlqi(m5, 32);
			m2 = _m_paddd(m2, m5);
			m5 = m3;
			m5 = _m_psrlqi(m5, 32);
			m3 = _m_paddd(m3, m5);

			pY[j*2-dstW+0	] = CLIP((_mm_cvtsi64_si32(m2) + ADD_Y) >> 8);
			pY[j*2-dstW+1	] = CLIP((_mm_cvtsi64_si32(m3) + ADD_Y) >> 8);
			pY[j*2+0		] = CLIP((_mm_cvtsi64_si32(m0) + ADD_Y) >> 8);
			pY[j*2+1		] = CLIP((_mm_cvtsi64_si32(m1) + ADD_Y) >> 8);

			m0 = m4;
			m0 = _m_pmaddwd(m0, ru);
			m4 = _m_pmaddwd(m4, rv);

			m5 = m0;
			m5 = _m_psrlqi(m5, 32);
			m0 = _m_paddd(m0, m5);
			m5 = m4;
			m5 = _m_psrlqi(m5, 32);
			m4 = _m_paddd(m4, m5);

			pU[j			] = CLIP((_mm_cvtsi64_si32(m0) + ADD_UV*4) >> 10);
			pV[j			] = CLIP((_mm_cvtsi64_si32(m4) + ADD_UV*4) >> 10);
					}

		pRGB32 += 2*srcW;
		pY -= 2*dstW;
		pU -= dstW/2;
		pV -= dstW/2;
    }

	_mm_empty();

	return true;
}

/******************************************************************************
 * Interpolate by bicubic 1 row of the image. SSE2 version.
 *
 * \note
 *
 * \param src				[in]  - pointer to row of source image component;
 * \param dst				[out] - pointer to interpolated row of destination image component;
 * \param w					[in]  - width  of source image;
 * \param h					[in]  - height of source image;
 * \param Luma				[in]  - true if luma component;
 *
 *  \date    26-11-2002		Created
 ******************************************************************************/
void CVSVideoProc_SSE2::InterpolateHor1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma)
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

	const __m128i xma = _mm_set_epi16(BIC_A0, BIC_A1, BIC_A2, BIC_A3, BIC_A3, BIC_A2, BIC_A1, BIC_A0),
				  xadd = _mm_set1_epi32(0x40);
	__m128i xmm0, xmm1, xmm2, xmm3, xmm7 = _mm_setzero_si128();

	int w4 = w / 4 * 4;
	int d = w - w4;

	for (j = 0; j<h; j++)
	{
		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[0] + BIC_A1*src[0] + BIC_A2*src[1] + BIC_A3*src[2] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst[2] = CLIP((BIC_A3*src[0] + BIC_A2*src[1] + BIC_A1*src[2] + BIC_A0*src[3] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst+=3; src+=2;

		for (i = 2; i < (w - 15); i += 4)
		{
			xmm0 = _mm_loadl_epi64((__m128i *)(src - 1));
			xmm2 = xmm0;
			xmm1 = xmm0;
			xmm3 = xmm0;
			xmm2 = _mm_srli_si128(xmm2, 1);
			xmm1 = _mm_srli_si128(xmm1, 2);
			xmm3 = _mm_srli_si128(xmm3, 3);
			xmm0 = _mm_unpacklo_epi32(xmm0, xmm2);
			xmm1 = _mm_unpacklo_epi32(xmm1, xmm3);
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm7);
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm7);
			xmm0 = _mm_madd_epi16(xmm0, xma);
			xmm1 = _mm_madd_epi16(xmm1, xma);
			xmm2 = xmm0;
			xmm3 = xmm1;
			xmm2 = _mm_srli_si128(xmm2, 4);
			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm3 = _mm_srli_si128(xmm3, 4);
			xmm1 = _mm_add_epi32(xmm1, xmm3);
			xmm2 = xmm0;
			xmm3 = xmm1;
			xmm2 = _mm_srli_si128(xmm2, 8);
			xmm3 = _mm_srli_si128(xmm3, 8);
			xmm0 = _mm_unpacklo_epi32(xmm0, xmm2);
			xmm1 = _mm_unpacklo_epi32(xmm1, xmm3);
			xmm0 = _mm_unpacklo_epi64(xmm0, xmm1);
			xmm0 = _mm_add_epi32(xmm0, xadd);
			xmm0 = _mm_srai_epi32(xmm0, RADIX_1);
			xmm0 = _mm_packs_epi32(xmm0, xmm7);
			xmm0 = _mm_packus_epi16(xmm0, xmm7);
			dst[0] = src[0];
			*(short*)(dst+1) = (short)(_mm_cvtsi128_si32(xmm0));
			dst[3] = src[2];
			xmm0 = _mm_srli_si128(xmm0, 2);
			*(short*)(dst+4) = (short)(_mm_cvtsi128_si32(xmm0));
			dst += 6;
			src += 4;
		}

		for (; i < (w - 3); i += 2) {
			const __m64 ma1 = W_TO_QW(BIC_A0, BIC_A1, BIC_A2, BIC_A3);
			const __m64 ma0 = W_TO_QW(BIC_A3, BIC_A2, BIC_A1, BIC_A0);
			const __m64 madd = DW_FILL_QW(0x40);
			__m64 m0, m1, m2, m7 = {0};

			int res;

			dst[0] = src[0];
			m0 = _m_from_int(*(int*)(src-1));
			m1 = _m_from_int(*(int*)(src));
			m0 = _m_punpcklbw(m0, m7);
			m1 = _m_punpcklbw(m1, m7);
			m0 = _m_pmaddwd(m0, ma0);
			m1 = _m_pmaddwd(m1, ma1);
			m2 = m0;
			m0 = _m_punpckldq(m0, m1);
			m2 = _m_punpckhdq(m2, m1);
			m0 = _m_paddd(m0, m2);
			m0 = _m_paddd(m0, madd); // must be unsigned
			m0 = _m_packssdw(m0, m7);// must be less then 2^15
			m0 = _m_psrawi(m0, RADIX_1);
			m0 = _m_packuswb(m0, m7);
			res = _m_to_int(m0);
			*(short*)(dst+1) = (short)res;
			dst+=3; src+=2;
		}

		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A2*src[1] + BIC_A3*src[1] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst[2] = CLIP((BIC_A3*src[ 0] + BIC_A2*src[1] + BIC_A1*src[1] + BIC_A0*src[1] + (1<<(RADIX_1-1)))>>RADIX_1);
		dst+=3; src+=2;
	}
	_m_empty();
}

/**
 ******************************************************************************
 * Bicubic interpolation by factor 1.5 of the image in ver dim. SSE2 version.
 *
 * \note
 *
 * \param src				[in]  - pointer to row of source image component;
 * \param dst				[out] - pointer to interpolated row of destination image component;
 * \param w					[in]  - width  of source image;
 * \param h					[in]  - height of source image;
 * \param Luma				[in]  - true if luma component;
 *
 *  \date    26-11-2002		Created.
 ******************************************************************************
 */
void CVSVideoProc_SSE2::InterpolateVer1_5(BYTE *src, BYTE *dst, int w, int h, bool Luma)
{
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
	const __m64 ma0 = W_FILL_QW(BIC_A0);
	const __m64 ma1 = W_FILL_QW(BIC_A1);
	const __m64 ma2 = W_FILL_QW(BIC_A2);
	const __m64 ma3 = W_FILL_QW(BIC_A3);
	const __m64 madd = DW_FILL_QW(0x40);
	__m64 m0, m1, m2, m3, m4, m5, m6, m7 = {0};
	int i, j;

	int w8 = w / 8 * 8;
	int d = w - w8;

	const __m128i xma0 = _mm_set1_epi16(BIC_A0),
				  xma1 = _mm_set1_epi16(BIC_A1),
				  xma2 = _mm_set1_epi16(BIC_A2),
				  xma3 = _mm_set1_epi16(BIC_A3),
				  xmadd = _mm_set1_epi32(0x40);
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7 = _mm_setzero_si128();

	for (j = 0; j<w; j+=4)
	{
		*(int*)(dst) = *(int*)(src);
		m0 = _m_from_int(*(int*)(src    ));
		m1 = _m_from_int(*(int*)(src+w  ));
		m2 = _m_from_int(*(int*)(src+w*2));
		m3 = _m_from_int(*(int*)(src+w*3));

		m0 = _m_punpcklbw(m0, m7);
		m4 = m5 = m0;
		m1 = _m_punpcklbw(m1, m7);
		m2 = _m_punpcklbw(m2, m7);
		m6 = m2;
		m3 = _m_punpcklbw(m3, m7);

		m0 = _m_pmullw(m0, ma0);
		m4 = _m_pmullw(m4, ma1);
		m5 = _m_pmullw(m5, ma3);
		m1 = _m_pmullw(m1, ma2);
		m2 = _m_pmullw(m2, ma3);
		m6 = _m_pmullw(m6, ma1);
		m3 = _m_pmullw(m3, ma0);

		m0 = _m_paddsw(m0, m2);
		m0 = _m_paddsw(m0, m4);
		m0 = _m_paddsw(m0, m1);
		m0 = _m_paddsw(m0, madd);

		m5 = _m_paddsw(m5, m6);
		m5 = _m_paddsw(m5, m3);
		m5 = _m_paddsw(m5, m1);
		m5 = _m_paddsw(m5, madd);

		m0 = _m_psrawi(m0, RADIX_1);
		m5 = _m_psrawi(m5, RADIX_1);

		m0 = _m_packuswb(m0, m0);
		m5 = _m_packuswb(m5, m5);

		*(int*)(dst+w  ) = _m_to_int(m0);
		*(int*)(dst+w*2) = _m_to_int(m5);
		dst+=4; src+=4;
	}
	dst+=2*w; src+=w;

	for (i = 0; i<(h/2-2); i++)
	{
		for (j = 0; j < w - 15; j += 8)
		{
			*(int*)(dst) = *(int*)(src); *(int*)(dst+4) = *(int*)(src+4);
			xmm0 = _mm_loadl_epi64((__m128i *)(src - w));
			xmm1 = _mm_loadl_epi64((__m128i *)(src + 0));
			xmm2 = _mm_loadl_epi64((__m128i *)(src + w));
			xmm3 = _mm_loadl_epi64((__m128i *)(src + w * 2));
			xmm4 = _mm_loadl_epi64((__m128i *)(src + w * 3));

			xmm0 = _mm_unpacklo_epi8(xmm0, xmm7);
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm7);
			xmm5 = xmm1;
			xmm2 = _mm_unpacklo_epi8(xmm2, xmm7);
			xmm3 = _mm_unpacklo_epi8(xmm3, xmm7);
			xmm6 = xmm3;
			xmm4 = _mm_unpacklo_epi8(xmm4, xmm7);

			xmm0 = _mm_mullo_epi16(xmm0, xma0);
			xmm1 = _mm_mullo_epi16(xmm1, xma1);
			xmm5 = _mm_mullo_epi16(xmm5, xma3);
			xmm2 = _mm_mullo_epi16(xmm2, xma2);
			xmm3 = _mm_mullo_epi16(xmm3, xma3);
			xmm6 = _mm_mullo_epi16(xmm6, xma1);
			xmm4 = _mm_mullo_epi16(xmm4, xma0);

			xmm0 = _mm_adds_epi16(xmm0, xmm1);
			xmm0 = _mm_adds_epi16(xmm0, xmm3);
			xmm0 = _mm_adds_epi16(xmm0, xmm2);
			xmm0 = _mm_adds_epi16(xmm0, xmadd);

			xmm5 = _mm_adds_epi16(xmm5, xmm6);
			xmm5 = _mm_adds_epi16(xmm5, xmm4);
			xmm5 = _mm_adds_epi16(xmm5, xmm2);
			xmm5 = _mm_adds_epi16(xmm5, xmadd);

			xmm0 = _mm_srai_epi16(xmm0, RADIX_1);
			xmm5 = _mm_srai_epi16(xmm5, RADIX_1);

			xmm0 = _mm_packus_epi16(xmm0, xmm0);
			xmm5 = _mm_packus_epi16(xmm5, xmm5);

			_mm_storel_epi64((__m128i*)(dst+w  ), xmm0);
			_mm_storel_epi64((__m128i*)(dst+w*2), xmm5);

			dst+=8; src+=8;
		}

		for (; j < w; j += 4)
		{
			*(int*)(dst) = *(int*)(src);
			m0 = _m_from_int(*(int*)(src-w  ));
			m1 = _m_from_int(*(int*)(src    ));
			m2 = _m_from_int(*(int*)(src+w  ));
			m3 = _m_from_int(*(int*)(src+w*2));
			m4 = _m_from_int(*(int*)(src+w*3));

			m0 = _m_punpcklbw(m0, m7);
			m1 = _m_punpcklbw(m1, m7);
			m5 = m1;
			m2 = _m_punpcklbw(m2, m7);
			m3 = _m_punpcklbw(m3, m7);
			m6 = m3;
			m4 = _m_punpcklbw(m4, m7);

			m0 = _m_pmullw(m0, ma0);
			m1 = _m_pmullw(m1, ma1);
			m5 = _m_pmullw(m5, ma3);
			m2 = _m_pmullw(m2, ma2);
			m3 = _m_pmullw(m3, ma3);
			m6 = _m_pmullw(m6, ma1);
			m4 = _m_pmullw(m4, ma0);

			m0 = _m_paddsw(m0, m1);
			m0 = _m_paddsw(m0, m3);
			m0 = _m_paddsw(m0, m2);
			m0 = _m_paddsw(m0, madd);

			m5 = _m_paddsw(m5, m6);
			m5 = _m_paddsw(m5, m4);
			m5 = _m_paddsw(m5, m2);
			m5 = _m_paddsw(m5, madd);

			m0 = _m_psrawi(m0, RADIX_1);
			m5 = _m_psrawi(m5, RADIX_1);

			m0 = _m_packuswb(m0, m0);
			m5 = _m_packuswb(m5, m5);

			*(int*)(dst+w  ) = _m_to_int(m0);
			*(int*)(dst+w*2) = _m_to_int(m5);
			dst+=4; src+=4;
		}

		dst+=2*w; src+=w;
	}

	for (j = 0; j<w; j+=4)
	{
		*(int*)(dst) = *(int*)(src);
		m0 = _m_from_int(*(int*)(src-w  ));
		m1 = _m_from_int(*(int*)(src    ));
		m2 = _m_from_int(*(int*)(src+w  ));

		m0 = _m_punpcklbw(m0, m7);
		m1 = _m_punpcklbw(m1, m7);
		m3 = m1;
		m2 = _m_punpcklbw(m2, m7);
		m4 = m5 = m6 = m2;

		m0 = _m_pmullw(m0, ma0);
		m1 = _m_pmullw(m1, ma1);
		m3 = _m_pmullw(m3, ma3);
		m2 = _m_pmullw(m2, ma2);
		m4 = _m_pmullw(m4, ma3);
		m5 = _m_pmullw(m5, ma1);
		m6 = _m_pmullw(m6, ma0);

		m0 = _m_paddsw(m0, m1);
		m0 = _m_paddsw(m0, m4);
		m0 = _m_paddsw(m0, m2);
		m0 = _m_paddsw(m0, madd);

		m3 = _m_paddsw(m3, m5);
		m3 = _m_paddsw(m3, m6);
		m3 = _m_paddsw(m3, m2);
		m3 = _m_paddsw(m3, madd);

		m0 = _m_psrawi(m0, RADIX_1);
		m3 = _m_psrawi(m3, RADIX_1);

		m0 = _m_packuswb(m0, m0);
		m3 = _m_packuswb(m3, m3);

		*(int*)(dst+w  ) = _m_to_int(m0);
		*(int*)(dst+w*2) = _m_to_int(m3);
		dst+=4; src+=4;
	}
	_m_empty();
}

/******************************************************************************
 * Interpolate by bicubic on hor dims dy factor 2. SSE2.
 *
 * \note
 *
 * \param src				[in]  - pointer to row of source image component;
 * \param dst				[out] - pointer to interpolated row of destination image component;
 * \param w					[in]  - width  of source image;
 * \param h					[in]  - height of source image;
 * \param Luma				[in]  - true if luma component;
 *
 *  \date    20-02-2004		Created
 ******************************************************************************/
void CVSVideoProc_SSE2::InterpolateHor2(BYTE *src, BYTE *dst, int w, int h, bool Luma)
{
	int i, j;
	int BIC_A0, BIC_A1;
	// use /4 koefs!!!
	if (Luma) {
		BIC_A0 = UP2_LA0/4;
		BIC_A1 = UP2_LA1/4;
	}
	else {
		BIC_A0 = UP2_CA0/4;
		BIC_A1 = UP2_CA1/4;
	}

	const __m128i xma0 = _mm_set1_epi16(BIC_A0),
				  xma1 = _mm_set1_epi16(BIC_A1),
				  xmadd = _mm_set1_epi16(0x10);
	__m128i xmm0 = _mm_setzero_si128(), xmm1, xmm2, xmm3, xmm4, xmm5;

	const __m64 ma0 = W_FILL_QW(BIC_A0);
	const __m64 ma1 = W_FILL_QW(BIC_A1);
	const __m64 madd = W_FILL_QW(0x10);
	__m64 m0, m1, m2, m3, m4;

	for (j = 0; j < h; j++) {
		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[0] + BIC_A1*src[0] + BIC_A1*src[1] + BIC_A0*src[2] + 0x10)>>5);
		dst += 2;

		for (i = 0; i < (w - 15); i += 8) { /// SSE2
			xmm4 = _mm_loadu_si128((__m128i *)(src + i));
			xmm1 = xmm4;
			xmm4 = _mm_srli_si128(xmm4, 1);
			xmm2 = xmm4;
			xmm4 = _mm_srli_si128(xmm4, 1);
			xmm3 = xmm4;
			xmm4 = _mm_srli_si128(xmm4, 1);

			xmm1 = _mm_unpacklo_epi8(xmm1, xmm0);
			xmm2 = _mm_unpacklo_epi8(xmm2, xmm0);
			xmm3 = _mm_unpacklo_epi8(xmm3, xmm0);
			xmm4 = _mm_unpacklo_epi8(xmm4, xmm0);
			xmm5 = xmm2;

			xmm1 = _mm_adds_epi16(xmm1, xmm4);
			xmm2 = _mm_adds_epi16(xmm2, xmm3);
			xmm1 = _mm_mullo_epi16(xmm1, xma0);
			xmm2 = _mm_mullo_epi16(xmm2, xma1);
			xmm1 = _mm_adds_epi16(xmm1, xmm2);
			xmm1 = _mm_adds_epi16(xmm1, xmadd);
			xmm1 = _mm_srai_epi16(xmm1, 5);

			xmm1 = _mm_packus_epi16(xmm1, xmm1);
			xmm5 = _mm_packus_epi16(xmm5, xmm5);
			xmm5 = _mm_unpacklo_epi8(xmm5, xmm1);

			_mm_storeu_si128((__m128i*)(dst+i*2), xmm5);
		}

		for (; i < (w - 7); i += 4) { /// MMX
			m4 = *(__m64*)(src+i);
			m1 = m4;
			m4 = _m_psrlqi(m4, 8);
			m2 = m4;
			m4 = _m_psrlqi(m4, 8);
			m3 = m4;
			m4 = _m_psrlqi(m4, 8);

			m0 = _mm_setzero_si64();
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpcklbw(m2, m0);
			m3 = _m_punpcklbw(m3, m0);
			m4 = _m_punpcklbw(m4, m0);
			m0 = m2;

			m1 = _m_paddsw(m1, m4);
			m2 = _m_paddsw(m2, m3);
			m1 = _m_pmullw(m1, ma0);
			m2 = _m_pmullw(m2, ma1);
			m1 = _m_paddsw(m1, m2);
			m1 = _m_paddsw(m1, madd);
			m1 = _m_psrawi(m1, 5);

			m1 = _m_packuswb(m1, m1);
			m0 = _m_packuswb(m0, m0);
			m0 = _m_punpcklbw(m0, m1);
			*(__m64*)(dst+i*2) = m0;
		}
		dst+=i*2; src+=i+1;

		for (;i<(w-3); i++) {
			dst[0] = src[0];
			dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A1*src[1] + BIC_A0*src[2] + 0x10)>>5);
			dst+=2; src++;
		}

		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A1*src[1] + BIC_A0*src[1] + 0x10)>>5);
		dst+=2; src++;

		dst[0] = src[0];
		dst[1] = CLIP((BIC_A0*src[-1] + BIC_A1*src[0] + BIC_A1*src[0] + BIC_A0*src[0] + 0x10)>>5);
		dst+=2; src++;
	}

	_m_empty();
}

/******************************************************************************
* Bicubic interpolation by factor 2 of the image in ver dim. SSE2 version.
*
* \note
*
* \param src				[in]  - pointer to row of source image component;
* \param dst				[out] - pointer to interpolated row of destination image component;
* \param w					[in]  - width  of source image;
* \param h					[in]  - height of source image;
* \param Luma				[in]  - true if luma component;
*
*  \date    20-02-2004		Created
******************************************************************************/
void CVSVideoProc_SSE2::InterpolateVer2(BYTE *src, BYTE *dst, int w, int h, bool Luma)
{
	int i, j;
	int BIC_A0, BIC_A1;
	// use /4 koefs!!!
	if (Luma) {
		BIC_A0 = UP2_LA0/4;
		BIC_A1 = UP2_LA1/4;
	}
	else {
		BIC_A0 = UP2_CA0/4;
		BIC_A1 = UP2_CA1/4;
	}
	const __m64 ma0 = W_FILL_QW(BIC_A0);
	const __m64 ma1 = W_FILL_QW(BIC_A1);
	const __m64 madd = W_FILL_QW(0x10);
	__m64 m0, m1, m2, m3, m4;

	const __m128i xma0 = _mm_set1_epi16(BIC_A0),
				  xma1 = _mm_set1_epi16(BIC_A1),
				  xmadd = _mm_set1_epi16(0x10);
	__m128i xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8, xmm0 = _mm_setzero_si128();
	int w16 = w / 16 * 16;
	int d = w - w16;

	for (j = 0; j < w16; j += 16) {
		*(int*)(dst + 0) = *(int*)(src + 0); *(int*)(dst +  4) = *(int*)(src +  4);
		*(int*)(dst + 8) = *(int*)(src + 8); *(int*)(dst + 12) = *(int*)(src + 12);

		xmm2 = _mm_loadu_si128((__m128i *)(src + 0));
		xmm3 = _mm_loadu_si128((__m128i *)(src + w));
		xmm4 = _mm_loadu_si128((__m128i *)(src + 2 * w));

		xmm6 = xmm2;
		xmm7 = xmm3;
		xmm8 = xmm4;
		xmm2 = _mm_unpacklo_epi8(xmm2, xmm0);
		xmm1 = xmm2;
		xmm3 = _mm_unpacklo_epi8(xmm3, xmm0);
		xmm4 = _mm_unpacklo_epi8(xmm4, xmm0);
		xmm6 = _mm_unpackhi_epi8(xmm6, xmm0);
		xmm5 = xmm6;
		xmm7 = _mm_unpackhi_epi8(xmm7, xmm0);
		xmm8 = _mm_unpackhi_epi8(xmm8, xmm0);

		xmm1 = _mm_adds_epi16(xmm1, xmm4);
		xmm2 = _mm_adds_epi16(xmm2, xmm3);
		xmm5 = _mm_adds_epi16(xmm5, xmm8);
		xmm6 = _mm_adds_epi16(xmm6, xmm7);
		xmm1 = _mm_mullo_epi16(xmm1, xma0);
		xmm2 = _mm_mullo_epi16(xmm2, xma1);
		xmm5 = _mm_mullo_epi16(xmm5, xma0);
		xmm6 = _mm_mullo_epi16(xmm6, xma1);
		xmm1 = _mm_adds_epi16(xmm1, xmm2);
		xmm5 = _mm_adds_epi16(xmm5, xmm6);
		xmm1 = _mm_adds_epi16(xmm1, xmadd);
		xmm5 = _mm_adds_epi16(xmm5, xmadd);
		xmm1 = _mm_srai_epi16(xmm1, 5);
		xmm5 = _mm_srai_epi16(xmm5, 5);

		xmm1 = _mm_packus_epi16(xmm1, xmm5);
		_mm_storeu_si128((__m128i*)(dst + w), xmm1);
		dst += 16; src += 16;
	}
	for (; j < w; j += 4) {
		*(int*)(dst) = *(int*)(src);
		m2 = _m_from_int(*(int*)(src    ));
		m3 = _m_from_int(*(int*)(src+w  ));
		m4 = _m_from_int(*(int*)(src+2*w));

		m0 = _mm_setzero_si64();
		m2 = _m_punpcklbw(m2, m0);
		m1 = m2;
		m3 = _m_punpcklbw(m3, m0);
		m4 = _m_punpcklbw(m4, m0);

		m1 = _m_paddsw(m1, m4);
		m2 = _m_paddsw(m2, m3);
		m1 = _m_pmullw(m1, ma0);
		m2 = _m_pmullw(m2, ma1);
		m1 = _m_paddsw(m1, m2);
		m1 = _m_paddsw(m1, madd);
		m1 = _m_psrawi(m1, 5);

		m1 = _m_packuswb(m1, m1);
		*(int*)(dst+w  ) = _m_to_int(m1);
		dst+=4; src+=4;
	}
	dst += w; src += 0;

	for (i = 0; i<(h-3); i++) {
		for (j = 0; j < w16; j += 16) {
			*(int*)(dst + 0) = *(int*)(src + 0); *(int*)(dst +  4) = *(int*)(src +  4);
			*(int*)(dst + 8) = *(int*)(src + 8); *(int*)(dst + 12) = *(int*)(src + 12);

			xmm1 = _mm_loadu_si128((__m128i *)(src - w));
			xmm2 = _mm_loadu_si128((__m128i *)(src + 0));
			xmm3 = _mm_loadu_si128((__m128i *)(src + w));
			xmm4 = _mm_loadu_si128((__m128i *)(src + 2 * w));

			xmm5 = xmm1;
			xmm6 = xmm2;
			xmm7 = xmm3;
			xmm8 = xmm4;
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm0);
			xmm2 = _mm_unpacklo_epi8(xmm2, xmm0);
			xmm3 = _mm_unpacklo_epi8(xmm3, xmm0);
			xmm4 = _mm_unpacklo_epi8(xmm4, xmm0);
			xmm5 = _mm_unpackhi_epi8(xmm5, xmm0);
			xmm6 = _mm_unpackhi_epi8(xmm6, xmm0);
			xmm7 = _mm_unpackhi_epi8(xmm7, xmm0);
			xmm8 = _mm_unpackhi_epi8(xmm8, xmm0);

			xmm1 = _mm_adds_epi16(xmm1, xmm4);
			xmm2 = _mm_adds_epi16(xmm2, xmm3);
			xmm5 = _mm_adds_epi16(xmm5, xmm8);
			xmm6 = _mm_adds_epi16(xmm6, xmm7);
			xmm1 = _mm_mullo_epi16(xmm1, xma0);
			xmm2 = _mm_mullo_epi16(xmm2, xma1);
			xmm5 = _mm_mullo_epi16(xmm5, xma0);
			xmm6 = _mm_mullo_epi16(xmm6, xma1);
			xmm1 = _mm_adds_epi16(xmm1, xmm2);
			xmm5 = _mm_adds_epi16(xmm5, xmm6);
			xmm1 = _mm_adds_epi16(xmm1, xmadd);
			xmm5 = _mm_adds_epi16(xmm5, xmadd);
			xmm1 = _mm_srai_epi16(xmm1, 5);
			xmm5 = _mm_srai_epi16(xmm5, 5);

			xmm1 = _mm_packus_epi16(xmm1, xmm5);
			_mm_storeu_si128((__m128i*)(dst + w), xmm1);
			dst += 16; src += 16;
		}

		for (; j < w; j += 4) {
			*(int*)(dst) = *(int*)(src);
			m1 = _m_from_int(*(int*)(src-w  ));
			m2 = _m_from_int(*(int*)(src    ));
			m3 = _m_from_int(*(int*)(src+w  ));
			m4 = _m_from_int(*(int*)(src+2*w));

			m0 = _mm_setzero_si64();
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpcklbw(m2, m0);
			m3 = _m_punpcklbw(m3, m0);
			m4 = _m_punpcklbw(m4, m0);

			m1 = _m_paddsw(m1, m4);
			m2 = _m_paddsw(m2, m3);
			m1 = _m_pmullw(m1, ma0);
			m2 = _m_pmullw(m2, ma1);
			m1 = _m_paddsw(m1, m2);
			m1 = _m_paddsw(m1, madd);
			m1 = _m_psrawi(m1, 5);

			m1 = _m_packuswb(m1, m1);
			*(int*)(dst+w  ) = _m_to_int(m1);
			dst+=4; src+=4;
		}

		dst+=w; src+=0;
	}

	for (j = 0; j < w16; j += 16) {
		*(int*)(dst + 0) = *(int*)(src + 0); *(int*)(dst +  4) = *(int*)(src +  4);
		*(int*)(dst + 8) = *(int*)(src + 8); *(int*)(dst + 12) = *(int*)(src + 12);

		xmm1 = _mm_loadu_si128((__m128i *)(src - w));
		xmm2 = _mm_loadu_si128((__m128i *)(src + 0));
		xmm3 = _mm_loadu_si128((__m128i *)(src + w));

		xmm5 = xmm1;
		xmm6 = xmm2;
		xmm7 = xmm3;
		xmm8 = xmm4;
		xmm1 = _mm_unpacklo_epi8(xmm1, xmm0);
		xmm2 = _mm_unpacklo_epi8(xmm2, xmm0);
		xmm3 = _mm_unpacklo_epi8(xmm3, xmm0);
		xmm4 = xmm3;
		xmm5 = _mm_unpackhi_epi8(xmm5, xmm0);
		xmm6 = _mm_unpackhi_epi8(xmm6, xmm0);
		xmm7 = _mm_unpackhi_epi8(xmm7, xmm0);
		xmm8 = xmm7;

		xmm1 = _mm_adds_epi16(xmm1, xmm4);
		xmm2 = _mm_adds_epi16(xmm2, xmm3);
		xmm5 = _mm_adds_epi16(xmm5, xmm8);
		xmm6 = _mm_adds_epi16(xmm6, xmm7);
		xmm1 = _mm_mullo_epi16(xmm1, xma0);
		xmm2 = _mm_mullo_epi16(xmm2, xma1);
		xmm5 = _mm_mullo_epi16(xmm5, xma0);
		xmm6 = _mm_mullo_epi16(xmm6, xma1);
		xmm1 = _mm_adds_epi16(xmm1, xmm2);
		xmm5 = _mm_adds_epi16(xmm5, xmm6);
		xmm1 = _mm_adds_epi16(xmm1, xmadd);
		xmm5 = _mm_adds_epi16(xmm5, xmadd);
		xmm1 = _mm_srai_epi16(xmm1, 5);
		xmm5 = _mm_srai_epi16(xmm5, 5);

		xmm1 = _mm_packus_epi16(xmm1, xmm5);
		_mm_storeu_si128((__m128i*)(dst + w), xmm1);
		dst += 16; src += 16;
	}
	for (; j < w; j += 4) {
		*(int*)(dst) = *(int*)(src);
		m1 = _m_from_int(*(int*)(src-w  ));
		m2 = _m_from_int(*(int*)(src    ));
		m3 = _m_from_int(*(int*)(src+w  ));

		m0 = _mm_setzero_si64();
		m1 = _m_punpcklbw(m1, m0);
		m2 = _m_punpcklbw(m2, m0);
		m3 = _m_punpcklbw(m3, m0);
		m4 = m3;

		m1 = _m_paddsw(m1, m4);
		m2 = _m_paddsw(m2, m3);
		m1 = _m_pmullw(m1, ma0);
		m2 = _m_pmullw(m2, ma1);
		m1 = _m_paddsw(m1, m2);
		m1 = _m_paddsw(m1, madd);
		m1 = _m_psrawi(m1, 5);

		m1 = _m_packuswb(m1, m1);
		*(int*)(dst+w  ) = _m_to_int(m1);
		dst += 4; src += 4;
	}
	dst += w; src += 0;

	for (j = 0; j < w16; j += 16) {
		*(int*)(dst + 0) = *(int*)(src + 0); *(int*)(dst +  4) = *(int*)(src +  4);
		*(int*)(dst + 8) = *(int*)(src + 8); *(int*)(dst + 12) = *(int*)(src + 12);

		xmm1 = _mm_loadu_si128((__m128i *)(src - w));
		xmm2 = _mm_loadu_si128((__m128i *)(src + 0));

		xmm5 = xmm1;
		xmm6 = xmm2;
		xmm7 = xmm3;
		xmm8 = xmm4;
		xmm1 = _mm_unpacklo_epi8(xmm1, xmm0);
		xmm2 = _mm_unpacklo_epi8(xmm2, xmm0);
		xmm3 = xmm2;
		xmm4 = xmm2;
		xmm5 = _mm_unpackhi_epi8(xmm5, xmm0);
		xmm6 = _mm_unpackhi_epi8(xmm6, xmm0);
		xmm7 = xmm6;
		xmm8 = xmm6;

		xmm1 = _mm_adds_epi16(xmm1, xmm4);
		xmm2 = _mm_adds_epi16(xmm2, xmm3);
		xmm5 = _mm_adds_epi16(xmm5, xmm8);
		xmm6 = _mm_adds_epi16(xmm6, xmm7);
		xmm1 = _mm_mullo_epi16(xmm1, xma0);
		xmm2 = _mm_mullo_epi16(xmm2, xma1);
		xmm5 = _mm_mullo_epi16(xmm5, xma0);
		xmm6 = _mm_mullo_epi16(xmm6, xma1);
		xmm1 = _mm_adds_epi16(xmm1, xmm2);
		xmm5 = _mm_adds_epi16(xmm5, xmm6);
		xmm1 = _mm_adds_epi16(xmm1, xmadd);
		xmm5 = _mm_adds_epi16(xmm5, xmadd);
		xmm1 = _mm_srai_epi16(xmm1, 5);
		xmm5 = _mm_srai_epi16(xmm5, 5);

		xmm1 = _mm_packus_epi16(xmm1, xmm5);
		_mm_storeu_si128((__m128i*)(dst + w), xmm1);
		dst += 16; src += 16;
	}
	for (; j<w; j+=4) {
		*(int*)(dst) = *(int*)(src);
		m1 = _m_from_int(*(int*)(src-w  ));
		m2 = _m_from_int(*(int*)(src    ));

		m0 = _mm_setzero_si64();
		m1 = _m_punpcklbw(m1, m0);
		m2 = _m_punpcklbw(m2, m0);
		m3 = m4 = m2;

		m1 = _m_paddsw(m1, m4);
		m2 = _m_paddsw(m2, m3);
		m1 = _m_pmullw(m1, ma0);
		m2 = _m_pmullw(m2, ma1);
		m1 = _m_paddsw(m1, m2);
		m1 = _m_paddsw(m1, madd);
		m1 = _m_psrawi(m1, 5);

		m1 = _m_packuswb(m1, m1);
		*(int*)(dst+w  ) = _m_to_int(m1);
		dst+=4; src+=4;
	}
	_m_empty();
}

/******************************************************************************
 * Make Dithering other the channel, depends from the Brightness. SSE2 version
 * \return none
 *
 * \param Src				[IN, OUT]	- processing image
 * \param W, H				[IN]		- image width and heith
 * \param Bits				[IN]		- Dithering Depth
 *
 *  \date    23-03-2004
 ******************************************************************************/
bool CVSVideoProc_SSE2::DitherI420(BYTE* Src, int W, int H, int Bits)
{
	__declspec(align(16)) static BYTE Cnt[16] = { 0,  1,  1,  0,  1,  1,  1,  0, 0,  1,  1,  0,  1,  1,  1,  0 };

	static int ThresholdBrightness[3] = {0, 896, 1136};
	int i, j, bits;
	BYTE *src;

	__declspec(align(16)) static BYTE  Mask[16*6] =
	{
		0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
		0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe,
		0x01, 0x03, 0x01, 0x03, 0x01, 0x03, 0x01, 0x03, 0x01, 0x03, 0x01, 0x03, 0x01, 0x03, 0x01, 0x03,
		0xfe, 0xfc, 0xfe, 0xfc, 0xfe, 0xfc, 0xfe, 0xfc, 0xfe, 0xfc, 0xfe, 0xfc, 0xfe, 0xfc, 0xfe, 0xfc,
		0x03, 0x07, 0x03, 0x07, 0x03, 0x07, 0x03, 0x07, 0x03, 0x07, 0x03, 0x07, 0x03, 0x07, 0x03, 0x07,
		0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8, 0xfc, 0xf8
	};

	__m128i xmm0, mask, nmask, cnt;

	if (Bits>2) Bits = 2;
	if (Bits<0) Bits = 0;

	cnt = *(__m128i*)(Cnt);

	for (j = 0; j < H; j++) {
		src = Src + W * j;
		for (i = 0; i < W / 16; i++) {
			int	brightness;

			bits = Bits;
			brightness = (src[i*16+ 0]+src[i*16+ 1]+src[i*16+ 2]+src[i*16+ 3]+src[i*16+ 4]+src[i*16+ 5]+src[i*16+ 6]+src[i*16+ 7]) +
						 (src[i*16+ 8]+src[i*16+ 9]+src[i*16+10]+src[i*16+11]+src[i*16+12]+src[i*16+13]+src[i*16+14]+src[i*16+15]);
			if (brightness<ThresholdBrightness[bits]) bits--;

			mask  = *(__m128i*)(Mask + 32 * bits);
			nmask = *(__m128i*)(Mask + 32 * bits + 16);

			xmm0 = _mm_loadu_si128((__m128i *)(src + i * 16));
			xmm0 = _mm_adds_epu8(xmm0, cnt);
			cnt = _mm_and_si128(xmm0, mask);
			xmm0 = _mm_and_si128(xmm0, nmask);
			_mm_storeu_si128((__m128i*)(src+i*16), xmm0);
		}
	}

	*(__m128i*)(Cnt) = cnt;

	_mm_empty();

	return true;
}

/**
 ******************************************************************************
 * Saturate chroma component of I420 image. SSE2 version.
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
bool CVSVideoProc_SSE2::SaturateI420Chroma(BYTE* src, int srcWidth, int srcHeight, DWORD dwSaturation)
{
	int i, j;

	int sat = dwSaturation*128/100;
	int w16 = srcWidth / 16 * 16;

	const __m128i xm128 = _mm_set1_epi16(0x80),
				  xm64  = _mm_set1_epi16(0x40),
				  xmsat = _mm_set1_epi16(sat);
	__m128i xmm0, xmm1, xmm7 = _mm_setzero_si128();

	const __m64 m128 = W_FILL_QW(0x80);
	const __m64 m64 = W_FILL_QW(0x40);
	const __m64 msat = W_FILL_QW(sat);
	__m64 m0, m7 = {0};

	for (j = 0; j<srcHeight; j++)
	{
		for (i = 0; i < w16; i += 16)
		{
			xmm0 = _mm_loadu_si128((__m128i *)(src + i));
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm7);
			xmm1 = _mm_unpackhi_epi8(xmm1, xmm7);
			xmm0 = _mm_subs_epi16(xmm0, xm128);
			xmm1 = _mm_subs_epi16(xmm1, xm128);
			xmm0 = _mm_mullo_epi16(xmm0, xmsat);
			xmm1 = _mm_mullo_epi16(xmm1, xmsat);
			xmm0 = _mm_adds_epi16(xmm0, xm64);
			xmm1 = _mm_adds_epi16(xmm1, xm64);
			xmm0 = _mm_srai_epi16(xmm0, 7);
			xmm1 = _mm_srai_epi16(xmm1, 7);
			xmm0 = _mm_adds_epi16(xmm0, xm128);
			xmm1 = _mm_adds_epi16(xmm1, xm128);
			xmm0 = _mm_packus_epi16(xmm0, xmm7);
						xmm1 = _mm_packus_epi16(xmm1, xmm7);
						xmm0 = _mm_unpacklo_epi64(xmm0, xmm1);
			_mm_storeu_si128((__m128i*)(src + i), xmm0);
		}
		for (; i<srcWidth; i+=4)
		{
			m0 = _m_from_int(*(int*)(src+i));
			m0 = _m_punpcklbw(m0, m7);
			m0 = _m_psubsw(m0, m128);
			m0 = _m_pmullw(m0, msat);
			m0 = _m_paddsw(m0, m64);
			m0 = _m_psrawi(m0, 7);
			m0 = _m_paddsw(m0, m128);
			m0 = _m_packuswb(m0, m7);
			*(int*)(src + i) = _m_to_int(m0);
		}
		src+=srcWidth;
	}
	_m_empty();
	return true;
}

/**
*****************************************************************************
 * Bilinear free interpolation of 8, 24 and 32 bits per pixel image. SSE2 version.
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 * \param dstStep			[in]  - destination line length in bytes;
 * \param mode				[in]  - type of image, 0 - 8bit, 2 - 24bit, 2 - 32bit;
 *
 * \return  false if input parametrs are out of range.
 *
 * \date    01-06-2005		Created
 ******************************************************************************/
bool CVSVideoProc_SSE2::ResizeBilinear(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH, int dstStep, int mode, int isUVPlane)
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
			bic_resize_x1_mmx(src, m_tbuff, srcW, srcH, dstW, m_resize_st[isUVPlane]->bicubic_cf_x_8bit, m_resize_st[isUVPlane]->bicubic_cf_len_x_8bit);
		else
			b_resize_x1_sse2(src, m_tbuff, srcW, srcH, dstW);
		if (dstH < srcH)
			bic_resize_y1_mmx(m_tbuff, dst, dstW, srcH, dstH, dstStep, m_resize_st[isUVPlane]->bicubic_cf_y, m_resize_st[isUVPlane]->bicubic_cf_len_y);
		else
			b_resize_y1_sse2(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==1) {
		if (dstStep < dstW*3)
			return false;
		if (dstW < srcW)
			bic_resize_x3_mmx(src, m_tbuff, srcW, srcH, dstW, m_resize_st[isUVPlane]->bicubic_cf_x, m_resize_st[isUVPlane]->bicubic_cf_len_x);
		else
			b_resize_x3_sse2(src, m_tbuff, srcW, srcH, dstW);
		if (dstH < srcH)
			bic_resize_y3_mmx(m_tbuff, dst, dstW, srcH, dstH, dstStep, m_resize_st[isUVPlane]->bicubic_cf_y, m_resize_st[isUVPlane]->bicubic_cf_len_y);
		else
			b_resize_y3_sse2(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==2) {
		if (dstStep < dstW*4)
			return false;
		if (dstW < srcW)
			bic_resize_x3_4_mmx(src, m_tbuff, srcW, srcH, dstW, m_resize_st[isUVPlane]->bicubic_cf_x, m_resize_st[isUVPlane]->bicubic_cf_len_x);
		else
			b_resize_x3_4_sse2(src, m_tbuff, srcW, srcH, dstW);
		if (dstH < srcH)
			bic_resize_y3_4_mmx(m_tbuff, dst, dstW, srcH, dstH, dstStep, m_resize_st[isUVPlane]->bicubic_cf_y, m_resize_st[isUVPlane]->bicubic_cf_len_y);
		else
			b_resize_y3_4_sse2(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==3) {
		if (dstStep < dstW*2)
			return false;
		if (dstW<srcW)
			bic_resize_x_565(src, m_tbuff, srcW, srcH, dstW);
		else
			b_resize_x_565_mmx(src, m_tbuff, srcW, srcH, dstW);
		if (dstH<srcH)
			bic_resize_y_565(m_tbuff, dst, dstW, srcH, dstH, dstStep);
		else
			b_resize_y_565_mmx(m_tbuff, dst, dstW, srcH, dstH, dstStep);
	}
	else if (mode==4) {
		if (dstStep < dstW)
			return false;
		bic_resize_8_x1_sse2(src, m_tbuff, srcW, srcH);
		bic_resize_8_y1_sse2(m_tbuff, dst, dstW, srcH, dstStep);
	} else
		return false;
	return true;
}


bool CVSVideoProc_IPP::ConvertYUY2ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW)
{
	return m_videoProcessingIpp.ConvertYUY2ToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool CVSVideoProc_IPP::ConvertUYVYToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW)
{
	return m_videoProcessingIpp.ConvertUYVYToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool CVSVideoProc_IPP::ConvertBMF24ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW)
{
	return m_videoProcessingIpp.ConvertBMF24ToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool CVSVideoProc_IPP::ConvertBMF32ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW)
{
	return m_videoProcessingIpp.ConvertBMF32ToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool CVSVideoProc_IPP::ConvertNV12ToI420(BYTE* src, BYTE* dstY, BYTE* dstU, BYTE *dstV, int srcW, int h, int dstW)
{
	return m_videoProcessingIpp.ConvertNV12ToI420(src, dstY, dstU, dstV, srcW, h, dstW);
}

bool CVSVideoProc_IPP::MirrorI420(BYTE* src, BYTE* dst, int srcW, int srcH, int mode)
{
	return m_videoProcessingIpp.MirrorI420(src, dst, srcW, srcH, mode);
}

bool CVSVideoProc_IPP::ResampleI420(BYTE* src, BYTE *dst, int srcW, int srcH, int dstW, int dstH)
{
	return m_videoProcessingIpp.ResampleI420(src, srcW, srcH, dst, dstW, dstH);
}

bool CVSVideoProc_IPP::ResampleCropI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW, int dstH, int dstStep,
	int srcWR, int srcHR, int srcOffsetW, int srcOffsetH, double factorW, double factorH, int mode)
{
	return m_videoProcessingIpp.ResampleCropI420(pSrc, pDst, srcW, srcH, srcStep, dstW, dstH, dstStep,
		srcWR, srcHR, srcOffsetW, srcOffsetH, factorW, factorH, mode);
}

bool CVSVideoProc_IPP::ResampleInscribedI420(BYTE* pSrc[3], BYTE* pDst[3], int srcW, int srcH, int srcStep, int dstW, int dstH, int dstStep,
	int srcOffsetW, int srcOffsetH, double factorW, double factorH, int mode)
{
	return m_videoProcessingIpp.ResampleInscribedI420(pSrc, pDst, srcW, srcH, srcStep, dstW, dstH, dstStep,
		srcOffsetW, srcOffsetH, factorW, factorH, mode);
}
