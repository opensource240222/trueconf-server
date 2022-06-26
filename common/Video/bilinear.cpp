/**
 **************************************************************************
 * \file bilinear.cpp
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief bilinear resampling functions implementation. Include MMX code.
 *
 * \b Project Video
 * \author SMirnovK
 * \date 01.06.2005
 *
 * $Revision: 2 $
 *
 * $History: bilinear.cpp $
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
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 31.07.06   Time: 20:51
 * Updated in $/VS/video
 * - added SSE2 videoproc class (turned off now)
 * - align impruvements for 8 bit resampling MMX methods
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 28.07.06   Time: 16:31
 * Updated in $/VS/Video
 * - added SSE2 variant bilinear resampling
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 27.07.06   Time: 13:44
 * Updated in $/VS/Video
 * - added HQ Resize
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 1.06.05    Time: 21:03
 * Created in $/VS/Video
 * bilinear scaling integration in Video project
 *
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include <string.h>
#include <math.h>
#include <mmintrin.h>
#include <emmintrin.h>

#pragma warning(disable:4309)

/****************************************************************************
 * Defines
 ****************************************************************************/

/// integer calculations precision
#define BASE	(8)
/// scale parameter for pointed precision
#define RADIX	(1<<BASE)
/// rounding to nearest integer for positive x
#define ROUND(x) (int)((x)+0.5)

/****************************************************************************
 * Functions
 ****************************************************************************/

/*******************************************************************************
 * \fn interp_cubic
 * \fn interp_exp
 * \fn interp_lin
 * Calculate Bilinear interpolation coeficients
 *
 * \param src				[in]  - coordinate of interpolated pixel;
 *
 * \return coeficient for left neighbour pixel
 *
 * \date    01-06-2005		Created
 ******************************************************************************/
unsigned short interp_cubic(double x)
{
	x = x - (double)(int)x;
	x-=0.5;
	return ROUND((-4.*x*x*x +0.5)*RADIX);
}

unsigned short interp_exp(double x)
{
	x = x - (double)(int)x;
	return ROUND((exp(-2.5*x*x)-x*exp(-2.5))*RADIX);
}

unsigned short interp_lin_(double x)
{
	x = x - (double)(int)x;
	return ROUND((1-x)*RADIX);
}

/// pointer to function that Calculate Bilinear interpolation coeficients
unsigned short (*interp_funk)(double) = interp_lin_;

/// 8 bit, y direction
int b_resize_y1(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep)
{
	int i,j;
	double factor = (double)(y-1)/(newy-1);

	memcpy(dst, src, x);
	for (j = 1; j<newy-1; j++) {
		int y_pos = (int)(factor*j);
		int y_coef = interp_funk(factor*j);
		int invy_coef = RADIX - y_coef;
		unsigned char * ps = src+x*y_pos;
		unsigned char * pd = dst+dstStep*j;
		for (i = 0; i<x; i+=4) {
			pd[i+0] = (ps[i+0]*y_coef+ps[x+i+0]*invy_coef+RADIX/2)>>BASE;
			pd[i+1] = (ps[i+1]*y_coef+ps[x+i+1]*invy_coef+RADIX/2)>>BASE;
			pd[i+2] = (ps[i+2]*y_coef+ps[x+i+2]*invy_coef+RADIX/2)>>BASE;
			pd[i+3] = (ps[i+3]*y_coef+ps[x+i+3]*invy_coef+RADIX/2)>>BASE;
		}
		if (i>x)
			for (i-=4; i<x; i++)
				pd[i+0] = (ps[i+0]*y_coef+ps[x+i+0]*invy_coef+RADIX/2)>>BASE;
	}
	memcpy(dst+dstStep*j, src+x*(y-1), x);
	return 0;
}


/// 8 bit, x direction
int b_resize_x1(unsigned char *src, unsigned char *dst, int x, int y, int newx)
{
	int i,j;
	double factor = (double)(x-1)/(newx-1);

	unsigned short *x_coef = new unsigned short [newx*2];

	for (i = 0; i<newx; i++) {
		x_coef[i*2+0] = (int)(factor*i);
		x_coef[i*2+1] = interp_funk(factor*i);
	}

	for (j = 0; j<y; j++) {
		*dst = *src;
		for (i = 1; i<(newx-1); i++) {
			int x_pos = x_coef[i*2+0];
			int dirx_coef = x_coef[i*2+1];
			int invx_coef = RADIX - dirx_coef;
			dst[i] = (src[x_pos]*dirx_coef + src[x_pos+1]*invx_coef + RADIX/2)>>BASE;
		}
		src+=x;
		dst+=newx;
		*(dst-1) = *(src-1);
	}
	delete[] x_coef;
	return 0;
}


/// 24 bit, y direction
int b_resize_y3(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep)
{
	return b_resize_y1(src, dst, x*3, y, newy, dstStep);
}


/// 24 bit, x direction
int b_resize_x3(unsigned char *src, unsigned char *dst, int x, int y, int newx)
{
	int i,j;
	double factor = (double)(x-1)/(newx-1);

	unsigned short *x_coef = new unsigned short[newx*2];

	for (i = 0; i<newx; i++) {
		x_coef[i*2+0] = (int)(factor*i)*3; // 3 components
		x_coef[i*2+1] = interp_funk(factor*i);
	}

	for (j = 0; j<y; j++) {
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		for (i = 1; i<(newx-1); i++) {
			int x_pos = x_coef[i*2+0];
			int dirx_coef = x_coef[i*2+1];
			int invx_coef = RADIX - dirx_coef;
			dst[i*3+0] = (src[x_pos+0]*dirx_coef + src[x_pos+3]*invx_coef + RADIX/2)>>BASE;
			dst[i*3+1] = (src[x_pos+1]*dirx_coef + src[x_pos+4]*invx_coef + RADIX/2)>>BASE;
			dst[i*3+2] = (src[x_pos+2]*dirx_coef + src[x_pos+5]*invx_coef + RADIX/2)>>BASE;
		}
		src+=x*3;
		dst+=newx*3;
		dst[-3] = src[-3];
		dst[-2] = src[-2];
		dst[-1] = src[-1];
	}
	delete[] x_coef;
	return 0;
}

/// 16 bit, y direction
int b_resize_y_565(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep)
{
	int i,j,x_out;
	unsigned int r,g,b;
	double factor = (double)(y-1)/(newy-1);
	unsigned char * ps = src;
	unsigned char * pd = dst;

	x_out = x * 2;
	x *= 4;

	for (i = 0; i < x_out; i += 2) {
		*(unsigned short*)(pd) = (((ps[0] >> 3) << 11) | ((ps[1] >> 2) << 5) | (ps[2] >> 3));
		pd += 2;
		ps += 4;
	}
	for (j = 1; j<newy-1; j++) {
		int y_pos = (int)(factor*j);
		int y_coef = interp_funk(factor*j);
		int invy_coef = RADIX - y_coef;
		ps = src+x*y_pos;
		pd = dst+dstStep*j;
		for (i = 0; i<x; i+=4) {
			r = (ps[i+0]*y_coef+ps[x+i+0]*invy_coef+RADIX/2)>>BASE;
			g = (ps[i+1]*y_coef+ps[x+i+1]*invy_coef+RADIX/2)>>BASE;
			b = (ps[i+2]*y_coef+ps[x+i+2]*invy_coef+RADIX/2)>>BASE;
			*(unsigned short*)(pd) = (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
			pd += 2;
		}
	}
	ps = src + x * (y - 1);
	pd = dst + dstStep * j;
	for (i = 0; i < x_out; i += 2) {
		*(unsigned short*)(pd) = (((ps[0] >> 3) << 11) | ((ps[1] >> 2) << 5) | (ps[2] >> 3));
		pd += 2;
		ps += 4;
	}
	return 0;
}

/// 16 bit, x direction
int b_resize_x_565(unsigned char *src, unsigned char *dst, int x, int y, int newx)
{
	int i,j;
	unsigned int pxl;
	double factor = (double)(x-1)/(newx-1);

	unsigned short *x_coef = new unsigned short[newx*2];
	unsigned char*  dst_c = new unsigned char [x*4];
	unsigned char*  p_dst_c = dst_c;

	for (i = 0; i<newx; i++) {
		x_coef[i*2+0] = (int)(factor*i)*4; // 3 components
		x_coef[i*2+1] = interp_funk(factor*i);
	}

	for (j = 0; j<y; j++) {
		for (i = 0; i < x; i++) {
			pxl = *(unsigned short*)(src + 2 * i);
			p_dst_c[4*i+2] = (pxl & 0x001f) << 3;
			p_dst_c[4*i+1] = (pxl & 0x07e0) >> 3;
			p_dst_c[4*i+0] = (pxl & 0xf800) >> 8;
		}
		dst[0] = p_dst_c[0];
		dst[1] = p_dst_c[1];
		dst[2] = p_dst_c[2];
		for (i = 1; i< (newx - 1); i++) {
			int x_pos = x_coef[i*2+0];
			int dirx_coef = x_coef[i*2+1];
			int invx_coef = RADIX - dirx_coef;
			dst[i*4+0] = (p_dst_c[x_pos+0]*dirx_coef + p_dst_c[x_pos+4]*invx_coef + RADIX/2)>>BASE;
			dst[i*4+1] = (p_dst_c[x_pos+1]*dirx_coef + p_dst_c[x_pos+5]*invx_coef + RADIX/2)>>BASE;
			dst[i*4+2] = (p_dst_c[x_pos+2]*dirx_coef + p_dst_c[x_pos+6]*invx_coef + RADIX/2)>>BASE;
		}
		src+=x*2;
		dst+=newx*4;
		dst[-4] = p_dst_c[4*x-4];
		dst[-3] = p_dst_c[4*x-3];
		dst[-2] = p_dst_c[4*x-2];
	}
	delete[] x_coef;
	delete[] dst_c;
	return 0;
}

/// 32 bit, y direction, alfa channel processed
int b_resize_y3_4(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep)
{
	int i,j;
	double factor = (double)(y-1)/(newy-1);
	x*=4;

	memcpy(dst, src, x);
	for (j = 1; j<newy-1; j++) {
		int y_pos = (int)(factor*j);
		int y_coef = interp_funk(factor*j);
		int invy_coef = RADIX - y_coef;
		unsigned char * ps = src+x*y_pos;
		unsigned char * pd = dst+dstStep*j;
		for (i = 0; i<x; i+=4) {
			pd[i+0] = (ps[i+0]*y_coef+ps[x+i+0]*invy_coef+RADIX/2)>>BASE;
			pd[i+1] = (ps[i+1]*y_coef+ps[x+i+1]*invy_coef+RADIX/2)>>BASE;
			pd[i+2] = (ps[i+2]*y_coef+ps[x+i+2]*invy_coef+RADIX/2)>>BASE;
			pd[i+3] = (ps[i+3]*y_coef+ps[x+i+3]*invy_coef+RADIX/2)>>BASE;
		}
	}
	memcpy(dst+dstStep*j, src+x*(y-1), x);
	return 0;
}


/// 32 bit, x direction, alfa channel processed
int b_resize_x3_4(unsigned char *src, unsigned char *dst, int x, int y, int newx)
{
	int i,j;
	double factor = (double)(x-1)/(newx-1);

	unsigned short *x_coef = new unsigned short[newx*2];

	for (i = 0; i<newx; i++) {
		x_coef[i*2+0] = (int)(factor*i)*4; // 4 components
		x_coef[i*2+1] = interp_funk(factor*i);
	}

	for (j = 0; j<y; j++) {
		*(int*)dst = *(int*)src;
		for (i = 1; i<(newx-1); i++) {
			int x_pos = x_coef[i*2+0];
			int dirx_coef = x_coef[i*2+1];
			int invx_coef = RADIX - dirx_coef;
			dst[i*4+0] = (src[x_pos+0]*dirx_coef + src[x_pos+4]*invx_coef + RADIX/2)>>BASE;
			dst[i*4+1] = (src[x_pos+1]*dirx_coef + src[x_pos+5]*invx_coef + RADIX/2)>>BASE;
			dst[i*4+2] = (src[x_pos+2]*dirx_coef + src[x_pos+6]*invx_coef + RADIX/2)>>BASE;
			dst[i*4+3] = (src[x_pos+3]*dirx_coef + src[x_pos+7]*invx_coef + RADIX/2)>>BASE;
		}
		src+=x*4;
		dst+=newx*4;
		*(int*)(dst-4) = *(int*)(src-4);
	}
	delete[] x_coef;
	return 0;
}


/****************************************************************************
 * MMX
 ****************************************************************************/

/// 8 bit, y direction, MMX version
int b_resize_y1_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep)
{
	int i,j;
	double factor = (double)(y-1)/(newy-1);
	unsigned short *y_coef = new unsigned short[newy*2];
	for (j = 0; j<newy; j++) {
		y_coef[j*2+0] = (int)(factor*j);
		y_coef[j*2+1] = interp_funk(factor*j);
	}

	memcpy(dst, src, x);
	const __m64 y_add = _mm_set1_pi16(RADIX/2);
	for (j = 1; j<newy-1; j++) {
		int y_pos = y_coef[j*2+0];
		short diry_coef = y_coef[j*2+1];
		short invy_coef = RADIX - diry_coef;
		unsigned char * ps = src+x*y_pos;
		unsigned char * pd = dst+dstStep*j;
		const __m64 diry = _mm_set1_pi16(diry_coef);
		const __m64 invy = _mm_set1_pi16(invy_coef);
		__m64 m0 = {0}, m1, m2, m3, m4;
		i = 0;
		int X1 = x&~7;
		for (; i<X1; i+=8) {
			m1 = *(__m64 *)(ps + i);
			m2 = *(__m64 *)(ps + x + i);
			m3 = m1;
			m4 = m2;
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpcklbw(m2, m0);
			m3 = _m_punpckhbw(m3, m0);
			m4 = _m_punpckhbw(m4, m0);
			m1 = _m_pmullw(m1, diry);
			m3 = _m_pmullw(m3, diry);
			m2 = _m_pmullw(m2, invy);
			m4 = _m_pmullw(m4, invy);
			m1 = _m_paddw(m1, m2);
			m3 = _m_paddw(m3, m4);
			m1 = _m_paddw(m1, y_add);
			m3 = _m_paddw(m3, y_add);
			m1 = _m_psrlwi(m1, BASE);
			m3 = _m_psrlwi(m3, BASE);
			m1 = _m_packuswb(m1, m3);
			*(__m64 *)(pd + i) = m1;
		}
		for (; i<x; i++)
			pd[i+0] = (ps[i+0]*diry_coef+ps[x+i+0]*invy_coef+RADIX/2)>>BASE;
	}
	_m_empty();
	memcpy(dst+dstStep*j, src+x*(y-1), x);
	delete[] y_coef;
	return 0;
}


/// 8 bit, x direction, MMX version
int b_resize_x1_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newx)
{
	return b_resize_x1(src, dst, x, y, newx);
}


/// 24 bit, y direction, MMX version
int b_resize_y3_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep)
{
	return b_resize_y1_mmx(src, dst, x*3, y, newy, dstStep);
}


/// 24 bit, x direction, MMX version
int b_resize_x3_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newx)
{
	return b_resize_x3(src, dst, x, y, newx);
}

/// 16 bit, y direction, MMX version
int b_resize_y_565_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep)
{
	int i,j,x_out;
	unsigned int r,g,b;
	double factor = (double)(y-1)/(newy-1);
	unsigned short *y_coef = new unsigned short[newy*2];
	unsigned char * ps = src;
	unsigned char * pd = dst;

	for (j = 0; j<newy; j++) {
		y_coef[j*2+0] = (int)(factor*j);
		y_coef[j*2+1] = interp_funk(factor*j);
	}

	const __m64 and_r = _mm_set1_pi16(0x00f8);
	const __m64 and_g = _mm_set1_pi16(0x00fc);
	const __m64 and_b = _mm_set1_pi16(0x00f8);

	x_out = x * 2;
	x *= 4;

	for (i = 0; i < x_out; i += 2) {
		*(unsigned short*)(pd) = (((ps[0] >> 3) << 11) | ((ps[1] >> 2) << 5) | (ps[2] >> 3));
		pd += 2;
		ps += 4;
	}
	const __m64 y_add = _mm_set1_pi16(RADIX/2);
	for (j = 1; j<newy-1; j++) {
		int y_pos = y_coef[j*2+0];
		short diry_coef = y_coef[j*2+1];
		short invy_coef = RADIX - diry_coef;
		unsigned char * ps = src+x*y_pos;
		unsigned char * pd = dst+dstStep*j;
		const __m64 diry = _mm_set1_pi16(diry_coef);
		const __m64 invy = _mm_set1_pi16(invy_coef);
		__m64 m0, m1, m2, m3, m4, m5, m6, m7;
		i = 0;
		int X1 = x/16*16;
		for (; i<X1; i+=16) {
			m0 = _mm_setzero_si64();

			m1 = *(__m64 *)(ps + i);
			m2 = *(__m64 *)(ps + 8 + i);
			m3 = m1;
			m4 = m2;
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpcklbw(m2, m0);
			m3 = _m_punpckhbw(m3, m0);
			m4 = _m_punpckhbw(m4, m0);
			m5 = m1;
			m1 = _m_punpcklwd(m1, m3);
			m3 = _m_punpckhwd(m5, m3);
			m5 = m2;
			m2 = _m_punpcklwd(m2, m4);
			m4 = _m_punpckhwd(m5, m4);
			m5 = m1;
			m1 = _m_punpckldq(m1, m2);
			m2 = _m_punpckhdq(m5, m2);
			m3 = _m_punpckldq(m3, m4);

			m4 = *(__m64 *)(ps + x + i);
			m5 = *(__m64 *)(ps + x + 8 + i);
			m6 = m4;
			m7 = m5;
			m4 = _m_punpcklbw(m4, m0);
			m5 = _m_punpcklbw(m5, m0);
			m6 = _m_punpckhbw(m6, m0);
			m7 = _m_punpckhbw(m7, m0);
			m0 = m4;
			m4 = _m_punpcklwd(m4, m6);
			m6 = _m_punpckhwd(m0, m6);
			m0 = m5;
			m5 = _m_punpcklwd(m5, m7);
			m7 = _m_punpckhwd(m0, m7);
			m0 = m4;
			m4 = _m_punpckldq(m4, m5);
			m5 = _m_punpckhdq(m0, m5);
			m6 = _m_punpckldq(m6, m7);

			m1 = _m_pmullw(m1, diry);
			m2 = _m_pmullw(m2, diry);
			m3 = _m_pmullw(m3, diry);
			m4 = _m_pmullw(m4, invy);
			m5 = _m_pmullw(m5, invy);
			m6 = _m_pmullw(m6, invy);
			m1 = _m_paddw(m1, m4);
			m2 = _m_paddw(m2, m5);
			m3 = _m_paddw(m3, m6);
			m1 = _m_paddw(m1, y_add);
			m2 = _m_paddw(m2, y_add);
			m3 = _m_paddw(m3, y_add);
			m1 = _m_psrlwi(m1, BASE);
			m2 = _m_psrlwi(m2, BASE);
			m3 = _m_psrlwi(m3, BASE);

			m1 = _mm_and_si64(m1, and_r);
			m1 = _mm_slli_si64(m1, 8);
			m2 = _mm_and_si64(m2, and_g);
			m2 = _mm_slli_si64(m2, 3);
			m3 = _mm_and_si64(m3, and_b);
			m3 = _mm_srli_si64(m3, 3);
			m1 = _mm_or_si64(m1, m2);
			m1 = _mm_or_si64(m1, m3);

			*(__m64*)(pd) = m1;

			pd += 8;
		}
		for (; i<x; i+=4) {
			r = (ps[i+0]*diry_coef+ps[x+i+0]*invy_coef+RADIX/2)>>BASE;
			g = (ps[i+1]*diry_coef+ps[x+i+1]*invy_coef+RADIX/2)>>BASE;
			b = (ps[i+2]*diry_coef+ps[x+i+2]*invy_coef+RADIX/2)>>BASE;
			*(unsigned short*)(pd) = (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
			pd += 2;
		}
	}
	_m_empty();
	ps = src + x * (y - 1);
	pd = dst + dstStep * j;
	for (i = 0; i < x_out; i += 2) {
		*(unsigned short*)(pd) = (((ps[0] >> 3) << 11) | ((ps[1] >> 2) << 5) | (ps[2] >> 3));
		pd += 2;
		ps += 4;
	}

	delete[] y_coef;

	return 0;
}

/// 16 bit, x direction, MMX version
int b_resize_x_565_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newx)
{
	int i,j;
	unsigned int pxl;
	double factor = (double)(x-1)/(newx-1);
	unsigned short *x_coef = new unsigned short[newx*2];
	unsigned char*  dst_c = new unsigned char [x*4*4];
	unsigned char*  p_dst_c = dst_c;

	for (i = 0; i<newx; i++) {
		x_coef[i*2+0] = (int)(factor*i)*4; // 4 components
		x_coef[i*2+1] = interp_funk(factor*i);
	}

	const __m64 y_add = _mm_set1_pi16(RADIX/2);
	const __m64 and_r = _mm_set1_pi16(0xf800);
	const __m64 and_g = _mm_set1_pi16(0x07e0);
	const __m64 and_b = _mm_set1_pi16(0x001f);
	__m64 m0 = {0}, m1, m2, m3;
	int Y1 = y&~3, X1 = x / 8 * 8;
	for (j = 0; j<Y1; j+=4) {
		for (i = 0; i < X1; i+=4) {
			m1 = *(__m64*)(src + 2 * i);
			m2 = m1;
			m3 = m1;
			m1 = _mm_and_si64(m1, and_r);
			m2 = _mm_and_si64(m2, and_g);
			m3 = _mm_and_si64(m3, and_b);
			m1 = _m_psrlwi(m1, 8);
			m2 = _m_psllwi(m2, 5);
			m3 = _m_psllwi(m3, 3);
			m1 = _mm_or_si64(m1, m2);
			m2 = m1;
			m1 = _m_punpcklwd(m1, m3);
			m2 = _m_punpckhwd(m2, m3);
			*(__m64 *)(p_dst_c + 4 * i) = m1;
			*(__m64 *)(p_dst_c + 4 * i + 8) = m2;

			m1 = *(__m64*)(src + 2 * i + 2 * x);
			m2 = m1;
			m3 = m1;
			m1 = _mm_and_si64(m1, and_r);
			m2 = _mm_and_si64(m2, and_g);
			m3 = _mm_and_si64(m3, and_b);
			m1 = _m_psrlwi(m1, 8);
			m2 = _m_psllwi(m2, 5);
			m3 = _m_psllwi(m3, 3);
			m1 = _mm_or_si64(m1, m2);
			m2 = m1;
			m1 = _m_punpcklwd(m1, m3);
			m2 = _m_punpckhwd(m2, m3);
			*(__m64 *)(p_dst_c + 4 * i + 4 * x) = m1;
			*(__m64 *)(p_dst_c + 4 * i + 8 + 4 * x) = m2;

			m1 = *(__m64*)(src + 2 * i + 4 * x);
			m2 = m1;
			m3 = m1;
			m1 = _mm_and_si64(m1, and_r);
			m2 = _mm_and_si64(m2, and_g);
			m3 = _mm_and_si64(m3, and_b);
			m1 = _m_psrlwi(m1, 8);
			m2 = _m_psllwi(m2, 5);
			m3 = _m_psllwi(m3, 3);
			m1 = _mm_or_si64(m1, m2);
			m2 = m1;
			m1 = _m_punpcklwd(m1, m3);
			m2 = _m_punpckhwd(m2, m3);
			*(__m64 *)(p_dst_c + 4 * i + 8 * x) = m1;
			*(__m64 *)(p_dst_c + 4 * i + 8 + 8 * x) = m2;

			m1 = *(__m64*)(src + 2 * i + 6 * x);
			m2 = m1;
			m3 = m1;
			m1 = _mm_and_si64(m1, and_r);
			m2 = _mm_and_si64(m2, and_g);
			m3 = _mm_and_si64(m3, and_b);
			m1 = _m_psrlwi(m1, 8);
			m2 = _m_psllwi(m2, 5);
			m3 = _m_psllwi(m3, 3);
			m1 = _mm_or_si64(m1, m2);
			m2 = m1;
			m1 = _m_punpcklwd(m1, m3);
			m2 = _m_punpckhwd(m2, m3);
			*(__m64 *)(p_dst_c + 4 * i + 12 * x) = m1;
			*(__m64 *)(p_dst_c + 4 * i + 8 + 12 * x) = m2;
		}
		for (; i<x; i++) {
			pxl = *(unsigned short*)(src + 2 * i);
			p_dst_c[4*i+2] = (pxl & 0x001f) << 3;
			p_dst_c[4*i+1] = (pxl & 0x07e0) >> 3;
			p_dst_c[4*i+0] = (pxl & 0xf800) >> 8;
			p_dst_c[4*i+3] = 0;
			pxl = *(unsigned short*)(src + 2 * i + 2 * x);
			p_dst_c[4*i+2+4*x] = (pxl & 0x001f) << 3;
			p_dst_c[4*i+1+4*x] = (pxl & 0x07e0) >> 3;
			p_dst_c[4*i+0+4*x] = (pxl & 0xf800) >> 8;
			p_dst_c[4*i+3+4*x] = 0;
			pxl = *(unsigned short*)(src + 2 * i + 4 * x);
			p_dst_c[4*i+2+8*x] = (pxl & 0x001f) << 3;
			p_dst_c[4*i+1+8*x] = (pxl & 0x07e0) >> 3;
			p_dst_c[4*i+0+8*x] = (pxl & 0xf800) >> 8;
			p_dst_c[4*i+3+8*x] = 0;
			pxl = *(unsigned short*)(src + 2 * i + 6 * x);
			p_dst_c[4*i+2+12*x] = (pxl & 0x001f) << 3;
			p_dst_c[4*i+1+12*x] = (pxl & 0x07e0) >> 3;
			p_dst_c[4*i+0+12*x] = (pxl & 0xf800) >> 8;
			p_dst_c[4*i+3+12*x] = 0;
		}
		for (i = 0; i<(newx-1); i++) {
			int x_pos = x_coef[i*2+0];
			int dirx_coef = x_coef[i*2+1];
			int invx_coef = RADIX - dirx_coef;
			const __m64 diry = _mm_set1_pi16(dirx_coef);
			const __m64 invy = _mm_set1_pi16(invx_coef);
			m1 = *(__m64 *)(p_dst_c + x_pos);
			m2 = m1;
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpckhbw(m2, m0);
			m1 = _m_pmullw(m1, diry);
			m2 = _m_pmullw(m2, invy);
			m1 = _m_paddw(m1, m2);
			m1 = _m_paddw(m1, y_add);
			m1 = _m_psrlwi(m1, BASE);
			m1 = _m_packuswb(m1, m0);
			*(int *)(dst + i*4) = _m_to_int(m1);

			m1 = *(__m64 *)(p_dst_c + x_pos + x*4);
			m2 = m1;
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpckhbw(m2, m0);
			m1 = _m_pmullw(m1, diry);
			m2 = _m_pmullw(m2, invy);
			m1 = _m_paddw(m1, m2);
			m1 = _m_paddw(m1, y_add);
			m1 = _m_psrlwi(m1, BASE);
			m1 = _m_packuswb(m1, m0);
			*(int *)(dst + i*4 + newx*4) = _m_to_int(m1);

			m1 = *(__m64 *)(p_dst_c + x_pos + x*8);
			m2 = m1;
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpckhbw(m2, m0);
			m1 = _m_pmullw(m1, diry);
			m2 = _m_pmullw(m2, invy);
			m1 = _m_paddw(m1, m2);
			m1 = _m_paddw(m1, y_add);
			m1 = _m_psrlwi(m1, BASE);
			m1 = _m_packuswb(m1, m0);
			*(int *)(dst + i*4 + newx*8) = _m_to_int(m1);

			m1 = *(__m64 *)(p_dst_c + x_pos + x*12);
			m2 = m1;
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpckhbw(m2, m0);
			m1 = _m_pmullw(m1, diry);
			m2 = _m_pmullw(m2, invy);
			m1 = _m_paddw(m1, m2);
			m1 = _m_paddw(m1, y_add);
			m1 = _m_psrlwi(m1, BASE);
			m1 = _m_packuswb(m1, m0);
			*(int *)(dst + i*4 + newx*12) = _m_to_int(m1);
		}
		src+=x*8;
		dst+=newx*4;
		*(int*)(dst-4) = *(int*)(p_dst_c+4*x-4);
		dst+=newx*4;
		*(int*)(dst-4) = *(int*)(p_dst_c+8*x-4);
		dst+=newx*4;
		*(int*)(dst-4) = *(int*)(p_dst_c+12*x-4);
		dst+=newx*4;
		*(int*)(dst-4) = *(int*)(p_dst_c+16*x-4);
	}
	_m_empty();

	for (; j<y; j++) {
		for (i = 0; i < x; i++) {
			pxl = *(unsigned short*)(src + 2 * i);
			p_dst_c[4*i+2] = (pxl & 0x001f) << 3;
			p_dst_c[4*i+1] = (pxl & 0x07e0) >> 3;
			p_dst_c[4*i+0] = (pxl & 0xf800) >> 8;
			p_dst_c[4*i+3] = 0;
		}
		*(int*)dst = *(int*)p_dst_c;
		for (i = 1; i<(newx-1); i++) {
			int x_pos = x_coef[i*2+0];
			int dirx_coef = x_coef[i*2+1];
			int invx_coef = RADIX - dirx_coef;
			dst[i*4+0] = (p_dst_c[x_pos+0]*dirx_coef + p_dst_c[x_pos+4]*invx_coef + RADIX/2)>>BASE;
			dst[i*4+1] = (p_dst_c[x_pos+1]*dirx_coef + p_dst_c[x_pos+5]*invx_coef + RADIX/2)>>BASE;
			dst[i*4+2] = (p_dst_c[x_pos+2]*dirx_coef + p_dst_c[x_pos+6]*invx_coef + RADIX/2)>>BASE;
		}
		src+=x*2;
		dst+=newx*4;
		*(int*)(dst-4) = *(int*)(p_dst_c+4*x-4);
	}
	delete[] x_coef;
	delete[] dst_c;
	return 0;
}

/// 32 bit, y direction, MMX version
int b_resize_y3_4_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep)
{
	return b_resize_y1_mmx(src, dst, x*4, y, newy, dstStep);
}


/// 32 bit, x direction, alfa channel don't processed, MMX version
int b_resize_x3_4_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newx)
{
	int i,j;
	double factor = (double)(x-1)/(newx-1);
	unsigned short *x_coef = new unsigned short[newx*2];
	for (i = 0; i<newx; i++) {
		x_coef[i*2+0] = (int)(factor*i)*4; // 4 components
		x_coef[i*2+1] = interp_funk(factor*i);
	}

	const __m64 y_add = _mm_set1_pi16(RADIX/2);
	__m64 m0 = {0}, m1, m2;
	int Y1 = y&~3;
	for (j = 0; j<Y1; j+=4) {
		for (i = 0; i<(newx-1); i++) {
			int x_pos = x_coef[i*2+0];
			int dirx_coef = x_coef[i*2+1];
			int invx_coef = RADIX - dirx_coef;
			const __m64 diry = _mm_set1_pi16(dirx_coef);
			const __m64 invy = _mm_set1_pi16(invx_coef);
			m1 = *(__m64 *)(src + x_pos);
			m2 = m1;
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpckhbw(m2, m0);
			m1 = _m_pmullw(m1, diry);
			m2 = _m_pmullw(m2, invy);
			m1 = _m_paddw(m1, m2);
			m1 = _m_paddw(m1, y_add);
			m1 = _m_psrlwi(m1, BASE);
			m1 = _m_packuswb(m1, m0);
			*(int *)(dst + i*4) = _m_to_int(m1);

			m1 = *(__m64 *)(src + x_pos + x*4);
			m2 = m1;
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpckhbw(m2, m0);
			m1 = _m_pmullw(m1, diry);
			m2 = _m_pmullw(m2, invy);
			m1 = _m_paddw(m1, m2);
			m1 = _m_paddw(m1, y_add);
			m1 = _m_psrlwi(m1, BASE);
			m1 = _m_packuswb(m1, m0);
			*(int *)(dst + i*4 + newx*4) = _m_to_int(m1);

			m1 = *(__m64 *)(src + x_pos + x*8);
			m2 = m1;
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpckhbw(m2, m0);
			m1 = _m_pmullw(m1, diry);
			m2 = _m_pmullw(m2, invy);
			m1 = _m_paddw(m1, m2);
			m1 = _m_paddw(m1, y_add);
			m1 = _m_psrlwi(m1, BASE);
			m1 = _m_packuswb(m1, m0);
			*(int *)(dst + i*4 + newx*8) = _m_to_int(m1);

			m1 = *(__m64 *)(src + x_pos + x*12);
			m2 = m1;
			m1 = _m_punpcklbw(m1, m0);
			m2 = _m_punpckhbw(m2, m0);
			m1 = _m_pmullw(m1, diry);
			m2 = _m_pmullw(m2, invy);
			m1 = _m_paddw(m1, m2);
			m1 = _m_paddw(m1, y_add);
			m1 = _m_psrlwi(m1, BASE);
			m1 = _m_packuswb(m1, m0);
			*(int *)(dst + i*4 + newx*12) = _m_to_int(m1);
		}
		src+=x*4;
		dst+=newx*4;
		*(int*)(dst-4) = *(int*)(src-4);
		src+=x*4;
		dst+=newx*4;
		*(int*)(dst-4) = *(int*)(src-4);
		src+=x*4;
		dst+=newx*4;
		*(int*)(dst-4) = *(int*)(src-4);
		src+=x*4;
		dst+=newx*4;
		*(int*)(dst-4) = *(int*)(src-4);
	}
	_m_empty();

	for (; j<y; j++) {
		*(int*)dst = *(int*)src;
		for (i = 1; i<(newx-1); i++) {
			int x_pos = x_coef[i*2+0];
			int dirx_coef = x_coef[i*2+1];
			int invx_coef = RADIX - dirx_coef;
			dst[i*4+0] = (src[x_pos+0]*dirx_coef + src[x_pos+4]*invx_coef + RADIX/2)>>BASE;
			dst[i*4+1] = (src[x_pos+1]*dirx_coef + src[x_pos+5]*invx_coef + RADIX/2)>>BASE;
			dst[i*4+2] = (src[x_pos+2]*dirx_coef + src[x_pos+6]*invx_coef + RADIX/2)>>BASE;
		}
		src+=x*4;
		dst+=newx*4;
		*(int*)(dst-4) = *(int*)(src-4);
	}
	delete[] x_coef;
	return 0;
}

/****************************************************************************
 * SSE2
 ****************************************************************************/

/// 8 bit, y direction, SSE2 version
int b_resize_y1_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep)
{
	int i,j;
	double factor = (double)(y - 1)/(newy - 1);
	unsigned short *y_coef = new unsigned short[newy*2];
	for (j = 0; j < newy; j++) {
		y_coef[j*2+0] = (int)(factor*j);
		y_coef[j*2+1] = interp_funk(factor*j);
	}

	memcpy(dst, src, x);
	const __m128i y_add = _mm_set1_epi16(RADIX/2);
	for (j = 1; j < newy - 1; j++) {
		int y_pos = y_coef[j*2+0];
		short diry_coef = y_coef[j*2+1];
		short invy_coef = RADIX - diry_coef;
		unsigned char * ps = src + x * y_pos;
		unsigned char * pd = dst + dstStep * j;
		const __m128i diry = _mm_set1_epi16(diry_coef);
		const __m128i invy = _mm_set1_epi16(invy_coef);
		__m128i xmm0 = _mm_setzero_si128 (), xmm1, xmm2, xmm3, xmm4;
		i = 0;
		int X1 = x&~15;
		for (; i < X1; i += 16) {
			xmm1 = _mm_loadu_si128((__m128i *)(ps + i));
			xmm2 = _mm_loadu_si128((__m128i *)(ps + x + i));
			xmm3 = xmm1;
			xmm4 = xmm2;
			xmm1 = _mm_unpacklo_epi8(xmm1, xmm0);
			xmm2 = _mm_unpacklo_epi8(xmm2, xmm0);
			xmm3 = _mm_unpackhi_epi8(xmm3, xmm0);
			xmm4 = _mm_unpackhi_epi8(xmm4, xmm0);
			xmm1 = _mm_mullo_epi16(xmm1, diry);
			xmm3 = _mm_mullo_epi16(xmm3, diry);
			xmm2 = _mm_mullo_epi16(xmm2, invy);
			xmm4 = _mm_mullo_epi16(xmm4, invy);
			xmm1 = _mm_add_epi16(xmm1, xmm2);
			xmm3 = _mm_add_epi16(xmm3, xmm4);
			xmm1 = _mm_add_epi16(xmm1, y_add);
			xmm3 = _mm_add_epi16(xmm3, y_add);
			xmm1 = _mm_srli_epi16(xmm1, BASE);
			xmm3 = _mm_srli_epi16(xmm3, BASE);
			xmm1 = _mm_packus_epi16(xmm1, xmm3);
			_mm_storeu_si128((__m128i *)(pd + i), xmm1);
		}
		for (; i < x; i++)
			pd[i+0] = (ps[i+0] * diry_coef + ps[x+i+0] * invy_coef + RADIX / 2) >> BASE;
	}
	_m_empty();
	memcpy(dst+dstStep*j, src+x*(y-1), x);
	delete[] y_coef;
	return 0;
}


/// 8 bit, x direction, SSE2 version
int b_resize_x1_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newx)
{
	return b_resize_x1(src, dst, x, y, newx);
}


/// 24 bit, y direction, SSE2 version
int b_resize_y3_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep)
{
	return b_resize_y1_sse2(src, dst, x*3, y, newy, dstStep);
}


/// 24 bit, x direction, SSE2 version
int b_resize_x3_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newx)
{
	return b_resize_x3(src, dst, x, y, newx);
}

/// 32 bit, y direction, SSE2 version
int b_resize_y3_4_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep)
{
	return b_resize_y1_sse2(src, dst, x*4, y, newy, dstStep);
}


/// 32 bit, x direction, alfa channel don't processed, SSE2 version
int b_resize_x3_4_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newx)
{
	return b_resize_x3_4_mmx(src, dst, x, y, newx);
}

#pragma warning(default:4309)