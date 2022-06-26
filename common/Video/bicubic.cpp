/**
 **************************************************************************
 * \file bicubic.h
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief bicubic minification functions implementation
 *
 * \b Project Video
 * \author SMirnovK
 * \date 22.06.2005
 *
 * $Revision: 4 $
 *
 * $History: bicubic.cpp $
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 22.10.10   Time: 17:43
 * Updated in $/VSNA/Video
 * - optimisation deinterlacing (sse2 bicubic minification)
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 22.10.10   Time: 16:19
 * Updated in $/VSNA/Video
 * - optimization resampling functions
 * - optimization deinterlacing
 * - clean VideoProc class
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
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 31.07.06   Time: 20:51
 * Updated in $/VS/Video
 * - added SSE2 videoproc class (turned off now)
 * - align impruvements for 8 bit resampling MMX methods
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 23.09.05   Time: 16:13
 * Updated in $/VS/Video
 * - added 8-bit image processing, C & MMX version
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 4.07.05    Time: 16:04
 * Updated in $/VS/Video
 * - added 32-bit image processing, C & MMX version
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 27.06.05   Time: 18:51
 * Updated in $/VS/Video
 * bicubic downsampling, MMX version
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 22.06.05   Time: 20:13
 * Created in $/VS/Video
 * - new files in Video project
 * - bicubic minification embeedded in videoproc class
 *
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <mmintrin.h>
#include <emmintrin.h>

extern unsigned char cipping_table[];


__declspec(align(16)) static short bicubic_cf_8[32] =
									 { -1, -11, -26, -41, -53, -57, -48, -21,  27,  95, 175, 261, 345, 419, 476, 508,
									  508, 476, 419, 345, 261, 175,  95,  27, -21, -48, -57, -53, -41, -26, -11,  -1 }; // int(cf * 4096 +- 0.5)

__declspec(align(16)) static short bicubic_cf_8_mmx[128] =
									 { -1,  -1,  -1,  -1, -11, -11, -11, -11, -26, -26, -26, -26, -41, -41, -41, -41,
									  -53, -53, -53, -53, -57, -57, -57, -57, -48, -48, -48, -48, -21, -21, -21, -21,
									   27,  27,  27,  27,  95,  95,  95,  95, 175, 175, 175, 175, 261, 261, 261, 261,
									  345, 345, 345, 345, 419, 419, 419, 419, 476, 476, 476, 476, 508, 508, 508, 508,
									  508, 508, 508, 508, 476, 476, 476, 476, 419, 419, 419, 419, 345, 345, 345, 345,
									  261, 261, 261, 261, 175, 175, 175, 175,  95,  95,  95,  95,  27,  27,  27,  27,
									  -21, -21, -21, -21, -48, -48, -48, -48, -57, -57, -57, -57, -53, -53, -53, -53,
									  -41, -41, -41, -41, -26, -26, -26, -26, -11, -11, -11, -11,  -1,  -1,  -1,  -1 };

__declspec(align(16)) short bicubic_cf_8_sse2[256] =
									  { -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -11, -11, -11, -11, -11, -11, -11, -11,
									   -26, -26, -26, -26, -26, -26, -26, -26, -41, -41, -41, -41, -41, -41, -41, -41,
									   -53, -53, -53, -53, -53, -53, -53, -53, -57, -57, -57, -57, -57, -57, -57, -57,
									   -48, -48, -48, -48, -48, -48, -48, -48, -21, -21, -21, -21, -21, -21, -21, -21,
									    27,  27,  27,  27,  27,  27,  27,  27,  95,  95,  95,  95,  95,  95,  95,  95,
									   175, 175, 175, 175, 175, 175, 175, 175, 261, 261, 261, 261, 261, 261, 261, 261,
									   345, 345, 345, 345, 345, 345, 345, 345, 419, 419, 419, 419, 419, 419, 419, 419,
									   476, 476, 476, 476, 476, 476, 476, 476, 508, 508, 508, 508, 508, 508, 508, 508,
									   508, 508, 508, 508, 508, 508, 508, 508, 476, 476, 476, 476, 476, 476, 476, 476,
									   419, 419, 419, 419, 419, 419, 419, 419, 345, 345, 345, 345, 345, 345, 345, 345,
									   261, 261, 261, 261, 261, 261, 261, 261, 175, 175, 175, 175, 175, 175, 175, 175,
									    95,  95,  95,  95,  95,  95,  95,  95,  27,  27,  27,  27,  27,  27,  27,  27,
									   -21, -21, -21, -21, -21, -21, -21, -21, -48, -48, -48, -48, -48, -48, -48, -48,
									   -57, -57, -57, -57, -57, -57, -57, -57, -53, -53, -53, -53, -53, -53, -53, -53,
									   -41, -41, -41, -41, -41, -41, -41, -41, -26, -26, -26, -26, -26, -26, -26, -26,
									   -11, -11, -11, -11, -11, -11, -11, -11,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 };

/****************************************************************************
 * Defines
 ****************************************************************************/
/// number of bits for integer coefficient of filter
#define BASE	(12)
/// scale factor for integer filter coefficient
#define SF		(1<<BASE)
/// rounding error addition in normalisation
#define ADD_C	(1<<(BASE-1))
/// rounding of float
#define ROUND(x) (x)<0.?(int)((x)-0.5):(int)((x)+0.5)
/// clipping to byte boundaries
#define CLIP(x) cipping_table[(x)+512]

/*******************************************************************************
 * Bicubic Interpolation function
 *
 * \param x				[in]  - position of pixel in range [-2..2]
 * \return	calculated value
 *
 * \date    22-06-2005		Created
 ******************************************************************************/
double bicubicInterpFunc(double x)
{
	const double a = -0.75;
	if (x<0.) x = -x;
	if (x<1.)
		return (a+2.)*x*x*x - (a+3.)*x*x + 1.;
	else if (x<2.)
		return a*x*x*x - 5.*a*x*x + 8.*a*x - 4.*a;
	else
		return 0.;
}


/*******************************************************************************
 * Create table with minification filter coefficients
 *
 * \param x				[in]  - number of pixel in source line
 * \param newx			[in]  - number of pixel in destination line
 * \param table			[out] - pointer to allocated table
 * \return	number of coef in minification filter (for one destination pixel)
 *
 * \date    22-06-2005		Created
 ******************************************************************************/
int genTableAlloc(int x, int newx, short* &table)
{
	double k = (double)(x)/(newx);
	double k_1 = 1./k;
	int len = (int)ceil(k*4);
	table = new short[newx*(len+1)];
	double *var = new double[len];
	int i, j;
	for (i = 0; i<newx; i++) {
		short *p = table+i*(len+1);
		double d = i*k - len*0.5 + 0.5*(k-1)+1;
		int numpoint = (int)floor(d);
		double w = d - numpoint;
		double sum = 0.;
		for (j = 0; j<len; j++) {
			double vv = 0.;
			if (!(numpoint+len-j-1 < 0 || numpoint+len-j-1 >= x))
				vv = bicubicInterpFunc((w-0.5 + j-(len-1)*0.5)*k_1);
			var[len-j-1] = vv;
			sum+=vv;
		}
		if (sum!=0.)
			sum = 1./sum;
		for (j = 0; j<len; j++)
			p[j] = ROUND(var[j]*sum*SF);
		p[len] = numpoint;
	}
	delete[] var;
	return len;
}

/*******************************************************************************
 * Create table with minification filter coefficients for MXX version
 *
 * \param x				[in]  - number of pixel in source line
 * \param newx			[in]  - number of pixel in destination line
 * \param table			[out] - pointer to allocated table
 * \return	number of coef in minification filter (for one destination pixel)
 *
 * \date    22-06-2005		Created
 ******************************************************************************/
int genTableAlloc_mmx(int x, int newx, short* &table)
{
	double k = (double)(x)/(newx);
	double k_1 = 1./k;
	int len = (int)ceil(k*4);
	table = new short[4*newx*(len+1)];
	double *var = new double[len];
	int i, j;
	for (i = 0; i<newx; i++) {
		short *p = table+4*i*(len+1);
		double d = i*k - len*0.5 + 0.5*(k-1)+1;
		int numpoint = (int)floor(d);
		double w = d - numpoint;
		double sum = 0.;
		for (j = 0; j<len; j++) {
			double vv = 0.;
			if (!(numpoint+len-j-1 < 0 || numpoint+len-j-1 >= x))
				vv = bicubicInterpFunc((w-0.5 + j-(len-1)*0.5)*k_1);
			var[len-j-1] = vv;
			sum+=vv;
		}
		if (sum!=0.)
			sum = 1./sum;
		for (j = 0; j<len; j++) {
			int tmp = ROUND(var[j]*sum*SF);
			p[4*j+0] = tmp;
			p[4*j+1] = tmp;
			p[4*j+2] = tmp;
			p[4*j+3] = tmp;
		}
		p[4*len+0] = numpoint;
		p[4*len+1] = numpoint;
		p[4*len+2] = numpoint;
		p[4*len+3] = numpoint;
	}
	delete[] var;
	return len;
}

/// 8 bit, x direction
void bic_resize_x1(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len)
{
	int i, j, ynum;
	int bound;
	for (ynum = 0; ynum <y; ynum++) {
		unsigned char *d = dst + newx*ynum;
		for (i = 0; i<newx; i++) {
			short *p = table+i*(len+1);
			int numpoints = p[len];
			unsigned char * s = src+ (ynum*x + numpoints);
			if (numpoints<0) j = -numpoints;
			else j = 0;
			if (numpoints+len >=x)
				bound = x- numpoints;
			else
				bound = len;
			int point = 0;
			for (;j<bound; j++) {
				point+=s[j]*p[j];
			}
			d[i] = CLIP((point+ADD_C)>>BASE);
		}
	}
}

void bic_resize_8_x1(unsigned char* src, unsigned char* dst, int x, int y)
{
	int i, j, ynum;
	int newx = x / 8, sum;
	int delta = x - (newx * 8);
	unsigned char *s, *d;

	for (ynum = 0; ynum < y; ynum++) {
		d = dst + newx * ynum;
		sum = 0;
		s = src;
		for (j = 0; j < 16; j += 8) {
			sum += s[0] * bicubic_cf_8[j+0];
			sum += s[0] * bicubic_cf_8[j+1];
			sum += s[0] * bicubic_cf_8[j+2];
			sum += s[0] * bicubic_cf_8[j+3];
			sum += s[0] * bicubic_cf_8[j+4];
			sum += s[0] * bicubic_cf_8[j+5];
			sum += s[0] * bicubic_cf_8[j+6];
			sum += s[0] * bicubic_cf_8[j+7];
		}
		for (j = 0; j < 16; j += 8) {
			sum += s[j+0] * bicubic_cf_8[j+0+16];
			sum += s[j+1] * bicubic_cf_8[j+1+16];
			sum += s[j+2] * bicubic_cf_8[j+2+16];
			sum += s[j+3] * bicubic_cf_8[j+3+16];
			sum += s[j+4] * bicubic_cf_8[j+4+16];
			sum += s[j+5] * bicubic_cf_8[j+5+16];
			sum += s[j+6] * bicubic_cf_8[j+6+16];
			sum += s[j+7] * bicubic_cf_8[j+7+16];
		}
		d[0] = CLIP((sum+ADD_C)>>BASE);
		sum = 0;
		s = src + 8;
		for (j = 0; j < 8; j += 8) {
			sum += s[0] * bicubic_cf_8[j+0];
			sum += s[0] * bicubic_cf_8[j+1];
			sum += s[0] * bicubic_cf_8[j+2];
			sum += s[0] * bicubic_cf_8[j+3];
			sum += s[0] * bicubic_cf_8[j+4];
			sum += s[0] * bicubic_cf_8[j+5];
			sum += s[0] * bicubic_cf_8[j+6];
			sum += s[0] * bicubic_cf_8[j+7];
		}
		for (j = 0; j < 24; j += 8) {
			sum += s[j+0] * bicubic_cf_8[j+0+8];
			sum += s[j+1] * bicubic_cf_8[j+1+8];
			sum += s[j+2] * bicubic_cf_8[j+2+8];
			sum += s[j+3] * bicubic_cf_8[j+3+8];
			sum += s[j+4] * bicubic_cf_8[j+4+8];
			sum += s[j+5] * bicubic_cf_8[j+5+8];
			sum += s[j+6] * bicubic_cf_8[j+6+8];
			sum += s[j+7] * bicubic_cf_8[j+7+8];
		}
		d[1] = CLIP((sum+ADD_C)>>BASE);
		for (i = 2; i < newx - 2; i++) {
			s = src + (ynum * x + (i - 2) * 8);
			sum = 0;
			for (j = 0; j < 32; j += 8) {
				sum += s[j+0] * bicubic_cf_8[j+0];
				sum += s[j+1] * bicubic_cf_8[j+1];
				sum += s[j+2] * bicubic_cf_8[j+2];
				sum += s[j+3] * bicubic_cf_8[j+3];
				sum += s[j+4] * bicubic_cf_8[j+4];
				sum += s[j+5] * bicubic_cf_8[j+5];
				sum += s[j+6] * bicubic_cf_8[j+6];
				sum += s[j+7] * bicubic_cf_8[j+7];
			}
			d[i] = CLIP((sum+ADD_C)>>BASE);
		}
		sum = 0;
		s = src + ((ynum + 1) * x - 1) - 16;
		for (j = 0; j < 16; j += 8) {
			sum += s[j+0] * bicubic_cf_8[j+0];
			sum += s[j+1] * bicubic_cf_8[j+1];
			sum += s[j+2] * bicubic_cf_8[j+2];
			sum += s[j+3] * bicubic_cf_8[j+3];
			sum += s[j+4] * bicubic_cf_8[j+4];
			sum += s[j+5] * bicubic_cf_8[j+5];
			sum += s[j+6] * bicubic_cf_8[j+6];
			sum += s[j+7] * bicubic_cf_8[j+7];
		}
		for (j = 16; j < 32; j += 8) {
			sum += s[16] * bicubic_cf_8[j+0];
			sum += s[16] * bicubic_cf_8[j+1];
			sum += s[16] * bicubic_cf_8[j+2];
			sum += s[16] * bicubic_cf_8[j+3];
			sum += s[16] * bicubic_cf_8[j+4];
			sum += s[16] * bicubic_cf_8[j+5];
			sum += s[16] * bicubic_cf_8[j+6];
			sum += s[16] * bicubic_cf_8[j+7];
		}
		d[newx - 2] = CLIP((sum+ADD_C)>>BASE);
		sum = 0;
		s = src + ((ynum + 1) * x - 1) - 8;
		for (j = 0; j < 8; j += 8) {
			sum += s[j+0] * bicubic_cf_8[j+0];
			sum += s[j+1] * bicubic_cf_8[j+1];
			sum += s[j+2] * bicubic_cf_8[j+2];
			sum += s[j+3] * bicubic_cf_8[j+3];
			sum += s[j+4] * bicubic_cf_8[j+4];
			sum += s[j+5] * bicubic_cf_8[j+5];
			sum += s[j+6] * bicubic_cf_8[j+6];
			sum += s[j+7] * bicubic_cf_8[j+7];
		}
		for (j = 8; j < 32; j += 8) {
			sum += s[8] * bicubic_cf_8[j+0];
			sum += s[8] * bicubic_cf_8[j+1];
			sum += s[8] * bicubic_cf_8[j+2];
			sum += s[8] * bicubic_cf_8[j+3];
			sum += s[8] * bicubic_cf_8[j+4];
			sum += s[8] * bicubic_cf_8[j+5];
			sum += s[8] * bicubic_cf_8[j+6];
			sum += s[8] * bicubic_cf_8[j+7];
		}
		d[newx - 1] = CLIP((sum+ADD_C)>>BASE);
	}
}

/// 24 bit, x direction
void bic_resize_x3(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len)
{
	int i, j, ynum;
	int bound;
	for (ynum = 0; ynum <y; ynum++) {
		unsigned char *d = dst + newx*3*ynum;
		for (i = 0; i<newx; i++) {
			short *p = table+i*(len+1);
			int numpoints = p[len];
			unsigned char * s = src+ (ynum*x + numpoints)*3;
			if (numpoints<0) j = -numpoints;
			else j = 0;
			if (numpoints+len >=x)
				bound = x- numpoints;
			else
				bound = len;
			int point[3] = {0, 0, 0};
			for (;j<bound; j++) {
				point[0]+=s[j*3+0]*p[j];
				point[1]+=s[j*3+1]*p[j];
				point[2]+=s[j*3+2]*p[j];
			}
			d[i*3+0] = CLIP((point[0]+ADD_C)>>BASE);
			d[i*3+1] = CLIP((point[1]+ADD_C)>>BASE);
			d[i*3+2] = CLIP((point[2]+ADD_C)>>BASE);
		}
	}
}

/// 8 bit, y direction
void bic_resize_y1(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len)
{
	int i, j, k;
	int bound;
	for (i = 0; i<newy; i++) {
		unsigned char *d = dst + step*i;
		short *p = table+i*(len+1);
		int numpoints = p[len];
		if (numpoints+len >=y)
			bound = y - numpoints;
		else
			bound = len;
		for (j = 0; j<x; j++) {
			int point = 0;
			unsigned char * s = src+numpoints*x+j;
			if (numpoints<0) k = -numpoints;
			else k = 0;
			for (;k<bound; k++) {
				point+=s[k*x]*p[k];
			}
			d[j] = CLIP((point+ADD_C)>>BASE);
		}
	}
}

void bic_resize_8_y1(unsigned char* src, unsigned char* dst, int x, int y, int step)
{
	int i, j, k;
	int newy = y / 8, sum;
	int delty = y - (newy * 8);
	unsigned char *s, *d;

	d = dst;
	for (j = 0; j < x; j++) {
		sum = 0;
		s = src + j;
		for (k = 0; k < 16; k += 8) {
			sum += s[0] * bicubic_cf_8[k+0];
			sum += s[1] * bicubic_cf_8[k+1];
			sum += s[2] * bicubic_cf_8[k+2];
			sum += s[3] * bicubic_cf_8[k+3];
			sum += s[4] * bicubic_cf_8[k+4];
			sum += s[5] * bicubic_cf_8[k+5];
			sum += s[6] * bicubic_cf_8[k+6];
			sum += s[7] * bicubic_cf_8[k+7];
		}
		for (k = 0; k < 16; k += 8) {
			sum += s[x*(k+0)] * bicubic_cf_8[k+0+16];
			sum += s[x*(k+1)] * bicubic_cf_8[k+1+16];
			sum += s[x*(k+2)] * bicubic_cf_8[k+2+16];
			sum += s[x*(k+3)] * bicubic_cf_8[k+3+16];
			sum += s[x*(k+4)] * bicubic_cf_8[k+4+16];
			sum += s[x*(k+5)] * bicubic_cf_8[k+5+16];
			sum += s[x*(k+6)] * bicubic_cf_8[k+6+16];
			sum += s[x*(k+7)] * bicubic_cf_8[k+7+16];
		}
		d[j] = CLIP((sum+ADD_C)>>BASE);
	}
	d = dst + step;
	for (j = 0; j < x; j++) {
		sum = 0;
		s = src + j;
		for (k = 0; k < 8; k += 8) {
			sum += s[0] * bicubic_cf_8[k+0];
			sum += s[1] * bicubic_cf_8[k+1];
			sum += s[2] * bicubic_cf_8[k+2];
			sum += s[3] * bicubic_cf_8[k+3];
			sum += s[4] * bicubic_cf_8[k+4];
			sum += s[5] * bicubic_cf_8[k+5];
			sum += s[6] * bicubic_cf_8[k+6];
			sum += s[7] * bicubic_cf_8[k+7];
		}
		for (k = 0; k < 24; k += 8) {
			sum += s[x*(k+0)] * bicubic_cf_8[k+0+8];
			sum += s[x*(k+1)] * bicubic_cf_8[k+1+8];
			sum += s[x*(k+2)] * bicubic_cf_8[k+2+8];
			sum += s[x*(k+3)] * bicubic_cf_8[k+3+8];
			sum += s[x*(k+4)] * bicubic_cf_8[k+4+8];
			sum += s[x*(k+5)] * bicubic_cf_8[k+5+8];
			sum += s[x*(k+6)] * bicubic_cf_8[k+6+8];
			sum += s[x*(k+7)] * bicubic_cf_8[k+7+8];
		}
		d[j] = CLIP((sum+ADD_C)>>BASE);
	}
	for (i = 2; i < newy - 2; i++) {
		d = dst + step * i;
		for (j = 0; j < x; j++) {
			sum = 0;
			s = src + (x * (i - 2) * 8 + j);
			for (k = 0; k < 32; k += 8) {
				sum += s[x*(k+0)] * bicubic_cf_8[k+0];
				sum += s[x*(k+1)] * bicubic_cf_8[k+1];
				sum += s[x*(k+2)] * bicubic_cf_8[k+2];
				sum += s[x*(k+3)] * bicubic_cf_8[k+3];
				sum += s[x*(k+4)] * bicubic_cf_8[k+4];
				sum += s[x*(k+5)] * bicubic_cf_8[k+5];
				sum += s[x*(k+6)] * bicubic_cf_8[k+6];
				sum += s[x*(k+7)] * bicubic_cf_8[k+7];
			}
			d[j] = CLIP((sum+ADD_C)>>BASE);
		}
	}
	d = dst + step * (newy - 2);
	for (j = 0; j < x; j++) {
		sum = 0;
		s = src + (x * ((newy - 1) * 8 - 16) + j);
		for (k = 0; k < 16; k += 8) {
			sum += s[x*j+0] * bicubic_cf_8[k+0];
			sum += s[x*j+1] * bicubic_cf_8[k+1];
			sum += s[x*j+2] * bicubic_cf_8[k+2];
			sum += s[x*j+3] * bicubic_cf_8[k+3];
			sum += s[x*j+4] * bicubic_cf_8[k+4];
			sum += s[x*j+5] * bicubic_cf_8[k+5];
			sum += s[x*j+6] * bicubic_cf_8[k+6];
			sum += s[x*j+7] * bicubic_cf_8[k+7];
		}
		for (k = 16; k < 32; k += 8) {
			sum += s[x*16+0] * bicubic_cf_8[k+0];
			sum += s[x*16+1] * bicubic_cf_8[k+1];
			sum += s[x*16+2] * bicubic_cf_8[k+2];
			sum += s[x*16+3] * bicubic_cf_8[k+3];
			sum += s[x*16+4] * bicubic_cf_8[k+4];
			sum += s[x*16+5] * bicubic_cf_8[k+5];
			sum += s[x*16+6] * bicubic_cf_8[k+6];
			sum += s[x*16+7] * bicubic_cf_8[k+7];
		}
		d[j] = CLIP((sum+ADD_C)>>BASE);
	}
	d = dst + step * (newy - 1);
	for (j = 0; j < x; j++) {
		sum = 0;
		s = src + (x * ((newy - 1) * 8 - 8) + j);
		for (k = 0; k < 8; k += 8) {
			sum += s[x*(8+0)] * bicubic_cf_8[k+0];
			sum += s[x*(8+1)] * bicubic_cf_8[k+1];
			sum += s[x*(8+2)] * bicubic_cf_8[k+2];
			sum += s[x*(8+3)] * bicubic_cf_8[k+3];
			sum += s[x*(8+4)] * bicubic_cf_8[k+4];
			sum += s[x*(8+5)] * bicubic_cf_8[k+5];
			sum += s[x*(8+6)] * bicubic_cf_8[k+6];
			sum += s[x*(8+7)] * bicubic_cf_8[k+7];
		}
		for (k = 8; k < 32; k += 8) {
			sum += s[x*(8+0)] * bicubic_cf_8[k+0];
			sum += s[x*(8+1)] * bicubic_cf_8[k+1];
			sum += s[x*(8+2)] * bicubic_cf_8[k+2];
			sum += s[x*(8+3)] * bicubic_cf_8[k+3];
			sum += s[x*(8+4)] * bicubic_cf_8[k+4];
			sum += s[x*(8+5)] * bicubic_cf_8[k+5];
			sum += s[x*(8+6)] * bicubic_cf_8[k+6];
			sum += s[x*(8+7)] * bicubic_cf_8[k+7];
		}
		d[j] = CLIP((sum+ADD_C)>>BASE);
	}
}

/// 24 bit, y direction
void bic_resize_y3(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len)
{
	return bic_resize_y1(src, dst, x*3, y, newy, step, table, len);
}

/// 16 bit, x direction
void bic_resize_x_565(unsigned char* src, unsigned char* dst, int x, int y, int newx)
{
	int i, j, ynum;
	unsigned short pxl;
	short *table = 0;
	int len = genTableAlloc(x, newx, table);
	int bound;
	unsigned char*  dst_c = new unsigned char [x*4];
	unsigned char*  p_dst_c = dst_c;
	for (ynum = 0; ynum <y; ynum++) {
		for (i = 0; i < x; i++) {
			pxl = *(unsigned short*)(src + 2 * i);
			p_dst_c[4*i+2] = (pxl & 0x001f) << 3;
			p_dst_c[4*i+1] = (pxl & 0x07e0) >> 3;
			p_dst_c[4*i+0] = (pxl & 0xf800) >> 8;
		}
		unsigned char *d = dst + newx*4*ynum;
		for (i = 0; i<newx; i++) {
			short *p = table+i*(len+1);
			int numpoints = p[len];
			unsigned char * s = p_dst_c + numpoints * 4;
			if (numpoints<0) j = -numpoints;
			else j = 0;
			if (numpoints+len >=x)
				bound = x- numpoints;
			else
				bound = len;
			int point[3] = {0, 0, 0};
			for (;j<bound; j++) {
				point[0]+=s[j*4+0]*p[j];
				point[1]+=s[j*4+1]*p[j];
				point[2]+=s[j*4+2]*p[j];
			}
			d[i*4+0] = CLIP((point[0]+ADD_C)>>BASE);
			d[i*4+1] = CLIP((point[1]+ADD_C)>>BASE);
			d[i*4+2] = CLIP((point[2]+ADD_C)>>BASE);
		}
		src += 2 * x;
	}
	delete[] table;
	delete[] dst_c;
}

/// 16 bit, y direction, alfa channel don't processed
void bic_resize_y_565(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step)
{
	int i, j, k;
	unsigned int r,g,b;
	short *table = 0;
	int len = genTableAlloc(y, newy, table);
	int bound;
	for (i = 0; i<newy; i++) {
		unsigned char *d = dst + step*i;
		short *p = table+i*(len+1);
		int numpoints = p[len];
		if (numpoints+len >=y)
			bound = y - numpoints;
		else
			bound = len;
		for (j = 0; j < x; j++) {
			int point[3] = {0,0,0};
			unsigned char * s = src+(numpoints*x+j)*4;
			if (numpoints<0) k = -numpoints;
			else k = 0;
			for (;k<bound; k++) {
				point[0] += s[k*x*4+0] * p[k];
				point[1] += s[k*x*4+1] * p[k];
				point[2] += s[k*x*4+2] * p[k];
			}
			r = CLIP((point[0]+ADD_C)>>BASE);
			g = CLIP((point[1]+ADD_C)>>BASE);
			b = CLIP((point[2]+ADD_C)>>BASE);
			*(unsigned short*)(d + 2 * j) = (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
		}
	}
	delete[] table;
}

/// 32 bit, x direction, alfa channel processed
void bic_resize_x3_4(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len)
{
	int i, j, ynum;
	int bound;
	for (ynum = 0; ynum <y; ynum++) {
		unsigned char *d = dst + newx*4*ynum;
		for (i = 0; i<newx; i++) {
			short *p = table+i*(len+1);
			int numpoints = p[len];
			unsigned char * s = src+ (ynum*x + numpoints)*4;
			if (numpoints<0) j = -numpoints;
			else j = 0;
			if (numpoints+len >=x)
				bound = x- numpoints;
			else
				bound = len;
			int point[4] = {0, 0, 0, 0};
			for (;j<bound; j++) {
				point[0]+=s[j*4+0]*p[j];
				point[1]+=s[j*4+1]*p[j];
				point[2]+=s[j*4+2]*p[j];
				point[3]+=s[j*4+3]*p[j];
			}
			d[i*4+0] = CLIP((point[0]+ADD_C)>>BASE);
			d[i*4+1] = CLIP((point[1]+ADD_C)>>BASE);
			d[i*4+2] = CLIP((point[2]+ADD_C)>>BASE);
			d[i*4+3] = CLIP((point[3]+ADD_C)>>BASE);
		}
	}
}


/// 32 bit, y direction, alfa channel processed
void bic_resize_y3_4(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len)
{
	int i, j, k;
	int bound;
	for (i = 0; i<newy; i++) {
		unsigned char *d = dst + step*i;
		short *p = table+i*(len+1);
		int numpoints = p[len];
		if (numpoints+len >=y)
			bound = y - numpoints;
		else
			bound = len;
		for (j = 0; j < x; j++) {
			int point[4] = {0,0,0,0};
			unsigned char * s = src+(numpoints*x+j)*4;
			if (numpoints<0) k = -numpoints;
			else k = 0;
			for (;k<bound; k++) {
				point[0] += s[k*x*4+0] * p[k];
				point[1] += s[k*x*4+1] * p[k];
				point[2] += s[k*x*4+2] * p[k];
				point[3] += s[k*x*4+3] * p[k];
			}
			d[4*j+0] = CLIP((point[0]+ADD_C)>>BASE);
			d[4*j+1] = CLIP((point[1]+ADD_C)>>BASE);
			d[4*j+2] = CLIP((point[2]+ADD_C)>>BASE);
			d[4*j+3] = CLIP((point[3]+ADD_C)>>BASE);
		}
	}
}

/// 8 bit, x direction, MMX version
void bic_resize_x1_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len)
{
	int i, j, ynum;
	int bound;

	__m64 mm0, mm2, mm3;
	const __m64 nNull = {0};

	int point = 0;

	for (ynum = 0; ynum <y; ynum++) {
		unsigned char *d = dst + newx*ynum;
		for (i = 0; i<newx; i++) {
			short *p = table+i*(len+1);
			int numpoints = p[len];
			unsigned char * s = src+ (ynum*x + numpoints);
			if (numpoints<0) j = -numpoints;
			else j = 0;
			if (numpoints+len >=x)
				bound = x- numpoints;
			else
				bound = len;
			int delta = (bound - j) % 4;
			int m_bound = bound - delta;
			mm3 = nNull;
			for (;j < m_bound; j+=4) {
				mm0 = _m_from_int(*(int *)(s+j));
				mm0 = _m_punpcklbw(mm0, nNull);
				mm2 = *(__m64 *)(p+j);
				mm0 = _m_pmaddwd(mm0, mm2);
				mm3 = _m_paddd(mm3, mm0);
			}
			mm0 = _m_psllqi(mm3, 32);
			mm0 = _m_paddd(mm0, mm3);
			mm0 = _m_psrlqi(mm0, 32);
			point = _m_to_int(mm0);
			for (j = m_bound; j < bound; j++) {
				point += s[j] * p[j];
			}
			d[i] = CLIP((point+ADD_C)>>BASE);
		}
	}
	_m_empty();
}

void bic_resize_8_x1_mmx(unsigned char* src, unsigned char* dst, int x, int y)
{
	int i, j, ynum;
	int newx = x / 8, sum = 0;
	int delta = x - (newx * 8);
	unsigned char *s, *d;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6, mm7;
	const __m64 nNull = {0};

	for (ynum = 0; ynum < y; ynum++) {
		d = dst + newx * ynum;
		sum = 0;
		s = src;
		for (j = 0; j < 16; j += 8) {
			sum += s[0] * bicubic_cf_8[j+0];
			sum += s[0] * bicubic_cf_8[j+1];
			sum += s[0] * bicubic_cf_8[j+2];
			sum += s[0] * bicubic_cf_8[j+3];
			sum += s[0] * bicubic_cf_8[j+4];
			sum += s[0] * bicubic_cf_8[j+5];
			sum += s[0] * bicubic_cf_8[j+6];
			sum += s[0] * bicubic_cf_8[j+7];
		}
		mm0 = *(__m64*)(s + 0);
		mm1 = mm0;
		mm0 = _m_punpcklbw(mm0, nNull);
		mm1 = _m_punpckhbw(mm1, nNull);
		mm2 = *(__m64*)(s + 8);
		mm3 = mm2;
		mm2 = _m_punpcklbw(mm2, nNull);
		mm3 = _m_punpckhbw(mm3, nNull);
		mm4 = *(__m64*)(bicubic_cf_8 + 16);
		mm0 = _m_pmaddwd(mm0, mm4);
		mm5 = *(__m64*)(bicubic_cf_8 + 20);
		mm1 = _m_pmaddwd(mm1, mm5);
		mm6 = *(__m64*)(bicubic_cf_8 + 24);
		mm0 = _m_paddd(mm0, mm1);
		mm2 = _m_pmaddwd(mm2, mm6);
		mm1 = *(__m64*)(bicubic_cf_8 + 28);
		mm3 = _m_pmaddwd(mm3, mm1);
		mm0 = _m_paddd(mm0, mm2);
		mm0 = _m_paddd(mm0, mm3);
		mm1 = mm0;
		mm1 = _m_psllqi(mm1, 32);
		mm0 = _m_paddd(mm0, mm1);
		mm0 = _m_psrlqi(mm0, 32);
		sum += _m_to_int(mm0);
		d[0] = CLIP((sum + ADD_C) >> BASE);

		sum = 0;
		s = src + 8;
		for (j = 0; j < 8; j += 8) {
			sum += s[0] * bicubic_cf_8[j+0];
			sum += s[0] * bicubic_cf_8[j+1];
			sum += s[0] * bicubic_cf_8[j+2];
			sum += s[0] * bicubic_cf_8[j+3];
			sum += s[0] * bicubic_cf_8[j+4];
			sum += s[0] * bicubic_cf_8[j+5];
			sum += s[0] * bicubic_cf_8[j+6];
			sum += s[0] * bicubic_cf_8[j+7];
		}
		mm0 = *(__m64*)(s + 0);
		mm1 = mm0;
		mm0 = _m_punpcklbw(mm0, nNull);
		mm1 = _m_punpckhbw(mm1, nNull);
		mm2 = *(__m64*)(s + 8);
		mm3 = mm2;
		mm2 = _m_punpcklbw(mm2, nNull);
		mm3 = _m_punpckhbw(mm3, nNull);
		mm4 = *(__m64*)(bicubic_cf_8 + 8);
		mm0 = _m_pmaddwd(mm0, mm4);
		mm5 = *(__m64*)(bicubic_cf_8 + 12);
		mm1 = _m_pmaddwd(mm1, mm5);
		mm6 = *(__m64*)(bicubic_cf_8 + 16);
		mm0 = _m_paddd(mm0, mm1);
		mm2 = _m_pmaddwd(mm2, mm6);
		mm1 = *(__m64*)(bicubic_cf_8 + 20);
		mm3 = _m_pmaddwd(mm3, mm1);
		mm0 = _m_paddd(mm0, mm2);
		mm0 = _m_paddd(mm0, mm3);
		mm3 = mm0;
		mm0 = *(__m64*)(s + 16);
		mm1 = mm0;
		mm0 = _m_punpcklbw(mm0, nNull);
		mm1 = _m_punpckhbw(mm1, nNull);
		mm4 = *(__m64*)(bicubic_cf_8 + 24);
		mm0 = _m_pmaddwd(mm0, mm4);
		mm5 = *(__m64*)(bicubic_cf_8 + 28);
		mm1 = _m_pmaddwd(mm1, mm5);
		mm0 = _m_paddd(mm0, mm1);
		mm0 = _m_paddd(mm0, mm3);
		mm1 = mm0;
		mm1 = _m_psllqi(mm1, 32);
		mm0 = _m_paddd(mm0, mm1);
		mm0 = _m_psrlqi(mm0, 32);
		sum += _m_to_int(mm0);
		d[1] = CLIP((sum + ADD_C) >> BASE);

		for (i = 2; i < newx - 2; i++) {
			mm7 = _mm_setzero_si64();
			s = src + (ynum * x + (i - 2) * 8);
			sum = 0;
			for (j = 0; j < 32; j += 16) {
				mm0 = *(__m64*)(s + j + 0);
				mm1 = mm0;
				mm0 = _m_punpcklbw(mm0, nNull);
				mm1 = _m_punpckhbw(mm1, nNull);
				mm2 = *(__m64*)(s + j + 8);
				mm3 = mm2;
				mm2 = _m_punpcklbw(mm2, nNull);
				mm3 = _m_punpckhbw(mm3, nNull);
				mm4 = *(__m64*)(bicubic_cf_8 + j + 0);
				mm0 = _m_pmaddwd(mm0, mm4);
				mm5 = *(__m64*)(bicubic_cf_8 + j + 4);
				mm1 = _m_pmaddwd(mm1, mm5);
				mm6 = *(__m64*)(bicubic_cf_8 + j + 8);
				mm0 = _m_paddd(mm0, mm1);
				mm2 = _m_pmaddwd(mm2, mm6);
				mm1 = *(__m64*)(bicubic_cf_8 + j + 12);
				mm3 = _m_pmaddwd(mm3, mm1);
				mm0 = _m_paddd(mm0, mm2);
				mm0 = _m_paddd(mm0, mm3);
				mm7 = _m_paddd(mm7, mm0);
			}
			mm0 = mm7;
			mm7 = _m_psllqi(mm7, 32);
			mm0 = _m_paddd(mm0, mm7);
			mm0 = _m_psrlqi(mm0, 32);
			sum = _m_to_int(mm0);
			d[i] = CLIP((sum + ADD_C) >> BASE);
		}

		sum = 0;
		s = src + ((ynum + 1) * x - 1) - 16;
		mm0 = *(__m64*)(s + 0);
		mm1 = mm0;
		mm0 = _m_punpcklbw(mm0, nNull);
		mm1 = _m_punpckhbw(mm1, nNull);
		mm2 = *(__m64*)(s + 8);
		mm3 = mm2;
		mm2 = _m_punpcklbw(mm2, nNull);
		mm3 = _m_punpckhbw(mm3, nNull);
		mm4 = *(__m64*)(bicubic_cf_8 + 0);
		mm0 = _m_pmaddwd(mm0, mm4);
		mm5 = *(__m64*)(bicubic_cf_8 + 4);
		mm1 = _m_pmaddwd(mm1, mm5);
		mm6 = *(__m64*)(bicubic_cf_8 + 8);
		mm0 = _m_paddd(mm0, mm1);
		mm2 = _m_pmaddwd(mm2, mm6);
		mm1 = *(__m64*)(bicubic_cf_8 + 12);
		mm3 = _m_pmaddwd(mm3, mm1);
		mm0 = _m_paddd(mm0, mm2);
		mm0 = _m_paddd(mm0, mm3);
		mm1 = mm0;
		mm1 = _m_psllqi(mm1, 32);
		mm0 = _m_paddd(mm0, mm1);
		mm0 = _m_psrlqi(mm0, 32);
		sum += _m_to_int(mm0);
		for (j = 16; j < 32; j += 8) {
			sum += s[16] * bicubic_cf_8[j+0];
			sum += s[16] * bicubic_cf_8[j+1];
			sum += s[16] * bicubic_cf_8[j+2];
			sum += s[16] * bicubic_cf_8[j+3];
			sum += s[16] * bicubic_cf_8[j+4];
			sum += s[16] * bicubic_cf_8[j+5];
			sum += s[16] * bicubic_cf_8[j+6];
			sum += s[16] * bicubic_cf_8[j+7];
		}
		d[newx - 2] = CLIP((sum + ADD_C) >> BASE);

		sum = 0;
		s = src + ((ynum + 1) * x - 1) - 8;
		mm0 = *(__m64*)(s + 0);
		mm1 = mm0;
		mm0 = _m_punpcklbw(mm0, nNull);
		mm1 = _m_punpckhbw(mm1, nNull);
		mm4 = *(__m64*)(bicubic_cf_8 + 0);
		mm0 = _m_pmaddwd(mm0, mm4);
		mm5 = *(__m64*)(bicubic_cf_8 + 4);
		mm1 = _m_pmaddwd(mm1, mm5);
		mm0 = _m_paddd(mm0, mm1);
		mm1 = mm0;
		mm1 = _m_psllqi(mm1, 32);
		mm0 = _m_paddd(mm0, mm1);
		mm0 = _m_psrlqi(mm0, 32);
		sum += _m_to_int(mm0);
		for (j = 8; j < 32; j += 8) {
			sum += s[8] * bicubic_cf_8[j+0];
			sum += s[8] * bicubic_cf_8[j+1];
			sum += s[8] * bicubic_cf_8[j+2];
			sum += s[8] * bicubic_cf_8[j+3];
			sum += s[8] * bicubic_cf_8[j+4];
			sum += s[8] * bicubic_cf_8[j+5];
			sum += s[8] * bicubic_cf_8[j+6];
			sum += s[8] * bicubic_cf_8[j+7];
		}
		d[newx - 1] = CLIP((sum + ADD_C) >> BASE);
	}

	_m_empty();
}

/// 24 bit, x direction, MMX version
void bic_resize_x3_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len)
{
	int i, j, ynum;
	int bound;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5;
	const __m64 nNull = {0}, add_mmx = _mm_set1_pi32(ADD_C);

	for (ynum = 0; ynum <y; ynum++) {
		unsigned char *d = dst + newx*3*ynum;
		for (i = 0; i<newx; i++) {
			short *p = table+4*i*(len+1);
			int numpoints = p[4*len];
			unsigned char * s = src+ (ynum*x + numpoints)*3;
			if (numpoints<0) j = -numpoints;
			else j = 0;
			if (numpoints+len >=x)
				bound = x- numpoints;
			else
				bound = len;
			mm4 = nNull;
			mm5 = nNull;
			for (;j < bound; j++) {
				mm0 = _m_from_int(*(int *)(s+3*j));
				mm0 = _m_punpcklbw(mm0, nNull);
				mm2 = *(__m64 *)(p+4*j);
				mm1 = mm0;
				mm0 = _m_pmullw(mm0, mm2);
				mm1 = _m_pmulhw(mm1, mm2);
				mm3 = mm0;
				mm0 = _m_punpcklwd(mm0,mm1);
				mm1 = _m_punpckhwd(mm3,mm1);
				mm4 = _m_paddd(mm4, mm0);
				mm5 = _m_paddd(mm5, mm1);
			}
			mm4 = _m_paddd(mm4, add_mmx);
			mm5 = _m_paddd(mm5, add_mmx);
			mm4 = _m_psradi(mm4, BASE);
			mm5 = _m_psradi(mm5, BASE);
			mm4 = _m_packssdw(mm4,mm5);
			mm4 = _m_packuswb(mm4,nNull);
			*(int*)(d+i*3) = _m_to_int(mm4);
		}
	}
	_m_empty();
}

/// 8 bit, y direction, MMX version
void bic_resize_y1_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len)
{
	int i, j, k;
	int bound;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5;
	const __m64 nNull = {0}, add_mmx = _mm_set1_pi32(ADD_C);

	for (i = 0; i<newy; i++) {
		unsigned char *d = dst + step*i;
		short *p = table+4*i*(len+1);
		int numpoints = p[4*len];
		if (numpoints+len >=y)
			bound = y - numpoints;
		else
			bound = len;
		int start_k = 0;
		if (numpoints<0) start_k = -numpoints;
		for (j = 0; j < (x&~3); j += 4) {
			int point = 0;
			unsigned char * s = src+numpoints*x+j;
			mm4 = nNull;
			mm5 = nNull;
			for (k = start_k; k < bound; k++) {
				mm0 = _m_from_int(*(int *)(s+k*x));
				mm0 = _m_punpcklbw(mm0, nNull);
				mm2 = *(__m64 *)(p+4*k);
				mm1 = mm0;
				mm0 = _m_pmullw(mm0, mm2);
				mm1 = _m_pmulhw(mm1, mm2);
				mm3 = mm0;
				mm0 = _m_punpcklwd(mm0,mm1);
				mm1 = _m_punpckhwd(mm3,mm1);
				mm4 = _m_paddd(mm4, mm0);
				mm5 = _m_paddd(mm5, mm1);
			}
			mm4 = _m_paddd(mm4, add_mmx);
			mm5 = _m_paddd(mm5, add_mmx);
			mm4 = _m_psradi(mm4, BASE);
			mm5 = _m_psradi(mm5, BASE);
			mm4 = _m_packssdw(mm4,mm5);
			mm4 = _m_packuswb(mm4,nNull);
			*(int*)(d+j) = _m_to_int(mm4);
		}
		for (; j<x; j++) {
			int point = 0;
			unsigned char * s = src+numpoints*x+j;
			for (k = start_k; k < bound; k++) {
				point+=s[k*x]*p[k*4];
			}
			d[j] = CLIP((point+ADD_C)>>BASE);
		}
	}
	_m_empty();
}

void bic_resize_8_y1_mmx(unsigned char* src, unsigned char* dst, int x, int y, int step)
{
	int i, j, k;
	int newy = y / 8, sum;
	int delty = y - (newy * 8);
	unsigned char *s, *d;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5;
	const __m64 nNull = {0}, add_mmx = _mm_set1_pi32(ADD_C);

	d = dst;
	for (j = 0; j < x; j++) {
		sum = 0;
		s = src + j;
		for (k = 0; k < 16; k += 8) {
			sum += s[0] * bicubic_cf_8[k+0];
			sum += s[1] * bicubic_cf_8[k+1];
			sum += s[2] * bicubic_cf_8[k+2];
			sum += s[3] * bicubic_cf_8[k+3];
			sum += s[4] * bicubic_cf_8[k+4];
			sum += s[5] * bicubic_cf_8[k+5];
			sum += s[6] * bicubic_cf_8[k+6];
			sum += s[7] * bicubic_cf_8[k+7];
		}
		for (k = 0; k < 16; k += 8) {
			sum += s[x*(k+0)] * bicubic_cf_8[k+0+16];
			sum += s[x*(k+1)] * bicubic_cf_8[k+1+16];
			sum += s[x*(k+2)] * bicubic_cf_8[k+2+16];
			sum += s[x*(k+3)] * bicubic_cf_8[k+3+16];
			sum += s[x*(k+4)] * bicubic_cf_8[k+4+16];
			sum += s[x*(k+5)] * bicubic_cf_8[k+5+16];
			sum += s[x*(k+6)] * bicubic_cf_8[k+6+16];
			sum += s[x*(k+7)] * bicubic_cf_8[k+7+16];
		}
		d[j] = CLIP((sum+ADD_C)>>BASE);
	}
	d = dst + step;
	for (j = 0; j < x; j++) {
		sum = 0;
		s = src + j;
		for (k = 0; k < 8; k += 8) {
			sum += s[0] * bicubic_cf_8[k+0];
			sum += s[1] * bicubic_cf_8[k+1];
			sum += s[2] * bicubic_cf_8[k+2];
			sum += s[3] * bicubic_cf_8[k+3];
			sum += s[4] * bicubic_cf_8[k+4];
			sum += s[5] * bicubic_cf_8[k+5];
			sum += s[6] * bicubic_cf_8[k+6];
			sum += s[7] * bicubic_cf_8[k+7];
		}
		for (k = 0; k < 24; k += 8) {
			sum += s[x*(k+0)] * bicubic_cf_8[k+0+8];
			sum += s[x*(k+1)] * bicubic_cf_8[k+1+8];
			sum += s[x*(k+2)] * bicubic_cf_8[k+2+8];
			sum += s[x*(k+3)] * bicubic_cf_8[k+3+8];
			sum += s[x*(k+4)] * bicubic_cf_8[k+4+8];
			sum += s[x*(k+5)] * bicubic_cf_8[k+5+8];
			sum += s[x*(k+6)] * bicubic_cf_8[k+6+8];
			sum += s[x*(k+7)] * bicubic_cf_8[k+7+8];
		}
		d[j] = CLIP((sum+ADD_C)>>BASE);
	}
	for (i = 2; i < newy - 2; i++) {
		d = dst + step * i;
		for (j = 0; j < (x&~3); j += 4) {
			sum = 0;
			s = src + (x * (i - 2) * 8 + j);
			mm4 = nNull;
			mm5 = nNull;
			for (k = 0; k < 32; k++) {
				mm0 = _m_from_int(*(int *)(s + k * x));
				mm0 = _m_punpcklbw(mm0, nNull);
				mm2 = *(__m64 *)(bicubic_cf_8_mmx + 4 * k);
				mm1 = mm0;
				mm0 = _m_pmullw(mm0, mm2);
				mm1 = _m_pmulhw(mm1, mm2);
				mm3 = mm0;
				mm0 = _m_punpcklwd(mm0,mm1);
				mm1 = _m_punpckhwd(mm3,mm1);
				mm4 = _m_paddd(mm4, mm0);
				mm5 = _m_paddd(mm5, mm1);
			}
			mm4 = _m_paddd(mm4, add_mmx);
			mm5 = _m_paddd(mm5, add_mmx);
			mm4 = _m_psradi(mm4, BASE);
			mm5 = _m_psradi(mm5, BASE);
			mm4 = _m_packssdw(mm4, mm5);
			mm4 = _m_packuswb(mm4, nNull);
			*(int*)(d + j) = _m_to_int(mm4);
		}
		for (; j < x; j++) {
			sum = 0;
			s = src + (x * (i - 2) * 8 + j);
			for (k = 0; k < 32; k += 8) {
				sum += s[x*(k+0)] * bicubic_cf_8[k+0];
				sum += s[x*(k+1)] * bicubic_cf_8[k+1];
				sum += s[x*(k+2)] * bicubic_cf_8[k+2];
				sum += s[x*(k+3)] * bicubic_cf_8[k+3];
				sum += s[x*(k+4)] * bicubic_cf_8[k+4];
				sum += s[x*(k+5)] * bicubic_cf_8[k+5];
				sum += s[x*(k+6)] * bicubic_cf_8[k+6];
				sum += s[x*(k+7)] * bicubic_cf_8[k+7];
			}
			d[j] = CLIP((sum+ADD_C)>>BASE);
		}
	}
	d = dst + step * (newy - 2);
	for (j = 0; j < x; j++) {
		sum = 0;
		s = src + (x * ((newy - 1) * 8 - 16) + j);
		for (k = 0; k < 16; k += 8) {
			sum += s[x*j+0] * bicubic_cf_8[k+0];
			sum += s[x*j+1] * bicubic_cf_8[k+1];
			sum += s[x*j+2] * bicubic_cf_8[k+2];
			sum += s[x*j+3] * bicubic_cf_8[k+3];
			sum += s[x*j+4] * bicubic_cf_8[k+4];
			sum += s[x*j+5] * bicubic_cf_8[k+5];
			sum += s[x*j+6] * bicubic_cf_8[k+6];
			sum += s[x*j+7] * bicubic_cf_8[k+7];
		}
		for (k = 16; k < 32; k += 8) {
			sum += s[x*16+0] * bicubic_cf_8[k+0];
			sum += s[x*16+1] * bicubic_cf_8[k+1];
			sum += s[x*16+2] * bicubic_cf_8[k+2];
			sum += s[x*16+3] * bicubic_cf_8[k+3];
			sum += s[x*16+4] * bicubic_cf_8[k+4];
			sum += s[x*16+5] * bicubic_cf_8[k+5];
			sum += s[x*16+6] * bicubic_cf_8[k+6];
			sum += s[x*16+7] * bicubic_cf_8[k+7];
		}
		d[j] = CLIP((sum+ADD_C)>>BASE);
	}
	d = dst + step * (newy - 1);
	for (j = 0; j < x; j++) {
		sum = 0;
		s = src + (x * ((newy - 1) * 8 - 8) + j);
		for (k = 0; k < 8; k += 8) {
			sum += s[x*(8+0)] * bicubic_cf_8[k+0];
			sum += s[x*(8+1)] * bicubic_cf_8[k+1];
			sum += s[x*(8+2)] * bicubic_cf_8[k+2];
			sum += s[x*(8+3)] * bicubic_cf_8[k+3];
			sum += s[x*(8+4)] * bicubic_cf_8[k+4];
			sum += s[x*(8+5)] * bicubic_cf_8[k+5];
			sum += s[x*(8+6)] * bicubic_cf_8[k+6];
			sum += s[x*(8+7)] * bicubic_cf_8[k+7];
		}
		for (k = 8; k < 32; k += 8) {
			sum += s[x*(8+0)] * bicubic_cf_8[k+0];
			sum += s[x*(8+1)] * bicubic_cf_8[k+1];
			sum += s[x*(8+2)] * bicubic_cf_8[k+2];
			sum += s[x*(8+3)] * bicubic_cf_8[k+3];
			sum += s[x*(8+4)] * bicubic_cf_8[k+4];
			sum += s[x*(8+5)] * bicubic_cf_8[k+5];
			sum += s[x*(8+6)] * bicubic_cf_8[k+6];
			sum += s[x*(8+7)] * bicubic_cf_8[k+7];
		}
		d[j] = CLIP((sum+ADD_C)>>BASE);
	}
	_m_empty();
}

/// 24 bit, y direction, MMX version
void bic_resize_y3_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len)
{
	return bic_resize_y1_mmx(src, dst, x*3, y, newy, step, table, len);
}

/// 32 bit, x direction, MMX version, alfa channel processed
void bic_resize_x3_4_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len)
{
	int i, j, ynum;
	int bound;

	__m64 mm0, mm1, mm2, mm3, mm4, mm5;
	const __m64 nNull = {0}, add_mmx = _mm_set1_pi32(ADD_C);

	for (ynum = 0; ynum <y; ynum++) {
		unsigned char *d = dst + newx*4*ynum;
		for (i = 0; i<newx; i++) {
			short *p = table+4*i*(len+1);
			int numpoints = p[4*len];
			unsigned char * s = src+ (ynum*x + numpoints)*4;
			if (numpoints<0) j = -numpoints;
			else j = 0;
			if (numpoints+len >=x)
				bound = x- numpoints;
			else
				bound = len;
			mm4 = nNull;
			mm5 = nNull;
			for (;j < bound; j++) {
				mm0 = _m_from_int(*(int *)(s+4*j));
				mm0 = _m_punpcklbw(mm0, nNull);
				mm2 = *(__m64 *)(p+4*j);
				mm1 = mm0;
				mm0 = _m_pmullw(mm0, mm2);
				mm1 = _m_pmulhw(mm1, mm2);
				mm3 = mm0;
				mm0 = _m_punpcklwd(mm0,mm1);
				mm1 = _m_punpckhwd(mm3,mm1);
				mm4 = _m_paddd(mm4, mm0);
				mm5 = _m_paddd(mm5, mm1);
			}
			mm4 = _m_paddd(mm4, add_mmx);
			mm5 = _m_paddd(mm5, add_mmx);
			mm4 = _m_psradi(mm4, BASE);
			mm5 = _m_psradi(mm5, BASE);
			mm4 = _m_packssdw(mm4,mm5);
			mm4 = _m_packuswb(mm4,nNull);
			*(int*)(d+i*4) = _m_to_int(mm4);
		}
	}
	_m_empty();
}

/// 32 bit, y direction, MMX version, alfa channel processed
void bic_resize_y3_4_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len)
{
	return bic_resize_y1_mmx(src, dst, x*4, y, newy, step, table, len);
}

void bic_resize_8_x1_sse2(unsigned char* src, unsigned char* dst, int x, int y)
{
	int i, j, ynum;
	int newx = x / 8, sum = 0;
	int delta = x - (newx * 8);
	unsigned char *s, *d;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5,
			xmm_null =  _mm_setzero_si128();

	for (ynum = 0; ynum < y; ynum++) {
		d = dst + newx * ynum;
		for (i = 2; i < newx - 2; i++) {
			s = src + (ynum * x + (i - 2) * 8);
			sum = 0;
			xmm0 = _mm_loadu_si128((const __m128i*)(s + 0));
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm_null);
			xmm1 = _mm_unpackhi_epi8(xmm1, xmm_null);
			xmm2 = _mm_loadu_si128((const __m128i*)(s + 16));
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi8(xmm2, xmm_null);
			xmm3 = _mm_unpackhi_epi8(xmm3, xmm_null);
			xmm4 = *(__m128i*)(bicubic_cf_8 + 0);
			xmm0 = _mm_madd_epi16(xmm0, xmm4);
			xmm5 = *(__m128i*)(bicubic_cf_8 + 8);
			xmm1 = _mm_madd_epi16(xmm1, xmm5);
			xmm4 = *(__m128i*)(bicubic_cf_8 + 16);
			xmm2 = _mm_madd_epi16(xmm2, xmm4);
			xmm5 = *(__m128i*)(bicubic_cf_8 + 24);
			xmm3 = _mm_madd_epi16(xmm3, xmm5);
			xmm0 = _mm_add_epi32(xmm0, xmm1);
			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm0 = _mm_add_epi32(xmm0, xmm3);
			xmm1 = xmm0;
			xmm1 = _mm_srli_si128(xmm1, 8);
			xmm0 = _mm_add_epi32(xmm0, xmm1);
			xmm1 = xmm0;
			xmm1 = _mm_srli_si128(xmm1, 4);
			xmm0 = _mm_add_epi32(xmm0, xmm1);
			sum = _mm_cvtsi128_si32(xmm0);
			d[i] = CLIP((sum + ADD_C) >> BASE);
		}
	}

	{ /// MMX bounds
		__m64 mm0, mm1, mm2, mm3, mm4, mm5, mm6;
		const __m64 nNull = {0};

		for (ynum = 0; ynum < y; ynum++) {
			d = dst + newx * ynum;
			sum = 0;
			s = src;
			for (j = 0; j < 16; j += 8) {
				sum += s[0] * bicubic_cf_8[j+0];
				sum += s[0] * bicubic_cf_8[j+1];
				sum += s[0] * bicubic_cf_8[j+2];
				sum += s[0] * bicubic_cf_8[j+3];
				sum += s[0] * bicubic_cf_8[j+4];
				sum += s[0] * bicubic_cf_8[j+5];
				sum += s[0] * bicubic_cf_8[j+6];
				sum += s[0] * bicubic_cf_8[j+7];
			}
			mm0 = *(__m64*)(s + 0);
			mm1 = mm0;
			mm0 = _m_punpcklbw(mm0, nNull);
			mm1 = _m_punpckhbw(mm1, nNull);
			mm2 = *(__m64*)(s + 8);
			mm3 = mm2;
			mm2 = _m_punpcklbw(mm2, nNull);
			mm3 = _m_punpckhbw(mm3, nNull);
			mm4 = *(__m64*)(bicubic_cf_8 + 16);
			mm0 = _m_pmaddwd(mm0, mm4);
			mm5 = *(__m64*)(bicubic_cf_8 + 20);
			mm1 = _m_pmaddwd(mm1, mm5);
			mm6 = *(__m64*)(bicubic_cf_8 + 24);
			mm0 = _m_paddd(mm0, mm1);
			mm2 = _m_pmaddwd(mm2, mm6);
			mm1 = *(__m64*)(bicubic_cf_8 + 28);
			mm3 = _m_pmaddwd(mm3, mm1);
			mm0 = _m_paddd(mm0, mm2);
			mm0 = _m_paddd(mm0, mm3);
			mm1 = mm0;
			mm1 = _m_psllqi(mm1, 32);
			mm0 = _m_paddd(mm0, mm1);
			mm0 = _m_psrlqi(mm0, 32);
			sum += _m_to_int(mm0);
			d[0] = CLIP((sum + ADD_C) >> BASE);

			sum = 0;
			s = src + 8;
			for (j = 0; j < 8; j += 8) {
				sum += s[0] * bicubic_cf_8[j+0];
				sum += s[0] * bicubic_cf_8[j+1];
				sum += s[0] * bicubic_cf_8[j+2];
				sum += s[0] * bicubic_cf_8[j+3];
				sum += s[0] * bicubic_cf_8[j+4];
				sum += s[0] * bicubic_cf_8[j+5];
				sum += s[0] * bicubic_cf_8[j+6];
				sum += s[0] * bicubic_cf_8[j+7];
			}
			mm0 = *(__m64*)(s + 0);
			mm1 = mm0;
			mm0 = _m_punpcklbw(mm0, nNull);
			mm1 = _m_punpckhbw(mm1, nNull);
			mm2 = *(__m64*)(s + 8);
			mm3 = mm2;
			mm2 = _m_punpcklbw(mm2, nNull);
			mm3 = _m_punpckhbw(mm3, nNull);
			mm4 = *(__m64*)(bicubic_cf_8 + 8);
			mm0 = _m_pmaddwd(mm0, mm4);
			mm5 = *(__m64*)(bicubic_cf_8 + 12);
			mm1 = _m_pmaddwd(mm1, mm5);
			mm6 = *(__m64*)(bicubic_cf_8 + 16);
			mm0 = _m_paddd(mm0, mm1);
			mm2 = _m_pmaddwd(mm2, mm6);
			mm1 = *(__m64*)(bicubic_cf_8 + 20);
			mm3 = _m_pmaddwd(mm3, mm1);
			mm0 = _m_paddd(mm0, mm2);
			mm0 = _m_paddd(mm0, mm3);
			mm3 = mm0;
			mm0 = *(__m64*)(s + 16);
			mm1 = mm0;
			mm0 = _m_punpcklbw(mm0, nNull);
			mm1 = _m_punpckhbw(mm1, nNull);
			mm4 = *(__m64*)(bicubic_cf_8 + 24);
			mm0 = _m_pmaddwd(mm0, mm4);
			mm5 = *(__m64*)(bicubic_cf_8 + 28);
			mm1 = _m_pmaddwd(mm1, mm5);
			mm0 = _m_paddd(mm0, mm1);
			mm0 = _m_paddd(mm0, mm3);
			mm1 = mm0;
			mm1 = _m_psllqi(mm1, 32);
			mm0 = _m_paddd(mm0, mm1);
			mm0 = _m_psrlqi(mm0, 32);
			sum += _m_to_int(mm0);
			d[1] = CLIP((sum + ADD_C) >> BASE);

			sum = 0;
			s = src + ((ynum + 1) * x - 1) - 16;
			mm0 = *(__m64*)(s + 0);
			mm1 = mm0;
			mm0 = _m_punpcklbw(mm0, nNull);
			mm1 = _m_punpckhbw(mm1, nNull);
			mm2 = *(__m64*)(s + 8);
			mm3 = mm2;
			mm2 = _m_punpcklbw(mm2, nNull);
			mm3 = _m_punpckhbw(mm3, nNull);
			mm4 = *(__m64*)(bicubic_cf_8 + 0);
			mm0 = _m_pmaddwd(mm0, mm4);
			mm5 = *(__m64*)(bicubic_cf_8 + 4);
			mm1 = _m_pmaddwd(mm1, mm5);
			mm6 = *(__m64*)(bicubic_cf_8 + 8);
			mm0 = _m_paddd(mm0, mm1);
			mm2 = _m_pmaddwd(mm2, mm6);
			mm1 = *(__m64*)(bicubic_cf_8 + 12);
			mm3 = _m_pmaddwd(mm3, mm1);
			mm0 = _m_paddd(mm0, mm2);
			mm0 = _m_paddd(mm0, mm3);
			mm1 = mm0;
			mm1 = _m_psllqi(mm1, 32);
			mm0 = _m_paddd(mm0, mm1);
			mm0 = _m_psrlqi(mm0, 32);
			sum += _m_to_int(mm0);
			for (j = 16; j < 32; j += 8) {
				sum += s[16] * bicubic_cf_8[j+0];
				sum += s[16] * bicubic_cf_8[j+1];
				sum += s[16] * bicubic_cf_8[j+2];
				sum += s[16] * bicubic_cf_8[j+3];
				sum += s[16] * bicubic_cf_8[j+4];
				sum += s[16] * bicubic_cf_8[j+5];
				sum += s[16] * bicubic_cf_8[j+6];
				sum += s[16] * bicubic_cf_8[j+7];
			}
			d[newx - 2] = CLIP((sum + ADD_C) >> BASE);

			sum = 0;
			s = src + ((ynum + 1) * x - 1) - 8;
			mm0 = *(__m64*)(s + 0);
			mm1 = mm0;
			mm0 = _m_punpcklbw(mm0, nNull);
			mm1 = _m_punpckhbw(mm1, nNull);
			mm4 = *(__m64*)(bicubic_cf_8 + 0);
			mm0 = _m_pmaddwd(mm0, mm4);
			mm5 = *(__m64*)(bicubic_cf_8 + 4);
			mm1 = _m_pmaddwd(mm1, mm5);
			mm0 = _m_paddd(mm0, mm1);
			mm1 = mm0;
			mm1 = _m_psllqi(mm1, 32);
			mm0 = _m_paddd(mm0, mm1);
			mm0 = _m_psrlqi(mm0, 32);
			sum += _m_to_int(mm0);
			for (j = 8; j < 32; j += 8) {
				sum += s[8] * bicubic_cf_8[j+0];
				sum += s[8] * bicubic_cf_8[j+1];
				sum += s[8] * bicubic_cf_8[j+2];
				sum += s[8] * bicubic_cf_8[j+3];
				sum += s[8] * bicubic_cf_8[j+4];
				sum += s[8] * bicubic_cf_8[j+5];
				sum += s[8] * bicubic_cf_8[j+6];
				sum += s[8] * bicubic_cf_8[j+7];
			}
			d[newx - 1] = CLIP((sum + ADD_C) >> BASE);
		}
	}

	_m_empty();
}

void bic_resize_8_y1_sse2(unsigned char* src, unsigned char* dst, int x, int y, int step)
{
	int i, j, k;
	int newy = y / 8, sum;
	int delty = y - (newy * 8);
	unsigned char *s, *d;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5,
			xmm_null =  _mm_setzero_si128(),
			xmm_add =  _mm_set1_epi32(ADD_C);

	for (i = 2; i < newy - 2; i++) {
		d = dst + step * i;
		for (j = 0; j < (x&~7); j += 8) {
			sum = 0;
			s = src + (x * (i - 2) * 8 + j);
			xmm4 = xmm_null;
			xmm5 = xmm_null;
			for (k = 0; k < 32; k++) {
				xmm0 = _mm_loadl_epi64((const __m128i*)(s + k * x));
				xmm0 = _mm_unpacklo_epi8(xmm0, xmm_null);
				xmm2 = *(__m128i*)(bicubic_cf_8_sse2 + 8 * k);
				xmm1 = xmm0;
				xmm0 = _mm_mullo_epi16(xmm0, xmm2);
				xmm1 = _mm_mulhi_epi16(xmm1, xmm2);
				xmm3 = xmm0;
				xmm0 = _mm_unpacklo_epi16(xmm0, xmm1);
				xmm1 = _mm_unpackhi_epi16(xmm3, xmm1);
				xmm4 = _mm_add_epi32(xmm4, xmm0);
				xmm5 = _mm_add_epi32(xmm5, xmm1);
			}
			xmm4 = _mm_add_epi32(xmm4, xmm_add);
			xmm5 = _mm_add_epi32(xmm5, xmm_add);
			xmm4 = _mm_srai_epi32(xmm4, BASE);
			xmm5 = _mm_srai_epi32(xmm5, BASE);
			xmm4 = _mm_packs_epi32(xmm4, xmm5);
			xmm4 = _mm_packus_epi16(xmm4, xmm_null);
			_mm_storel_epi64((__m128i*)(d + j), xmm4);
		}
		for (; j < x; j++) {
			sum = 0;
			s = src + (x * (i - 2) * 8 + j);
			for (k = 0; k < 32; k += 8) {
				sum += s[x*(k+0)] * bicubic_cf_8[k+0];
				sum += s[x*(k+1)] * bicubic_cf_8[k+1];
				sum += s[x*(k+2)] * bicubic_cf_8[k+2];
				sum += s[x*(k+3)] * bicubic_cf_8[k+3];
				sum += s[x*(k+4)] * bicubic_cf_8[k+4];
				sum += s[x*(k+5)] * bicubic_cf_8[k+5];
				sum += s[x*(k+6)] * bicubic_cf_8[k+6];
				sum += s[x*(k+7)] * bicubic_cf_8[k+7];
			}
			d[j] = CLIP((sum+ADD_C)>>BASE);
		}
	}

	{
		d = dst;
		for (j = 0; j < x; j++) {
			sum = 0;
			s = src + j;
			for (k = 0; k < 16; k += 8) {
				sum += s[0] * bicubic_cf_8[k+0];
				sum += s[1] * bicubic_cf_8[k+1];
				sum += s[2] * bicubic_cf_8[k+2];
				sum += s[3] * bicubic_cf_8[k+3];
				sum += s[4] * bicubic_cf_8[k+4];
				sum += s[5] * bicubic_cf_8[k+5];
				sum += s[6] * bicubic_cf_8[k+6];
				sum += s[7] * bicubic_cf_8[k+7];
			}
			for (k = 0; k < 16; k += 8) {
				sum += s[x*(k+0)] * bicubic_cf_8[k+0+16];
				sum += s[x*(k+1)] * bicubic_cf_8[k+1+16];
				sum += s[x*(k+2)] * bicubic_cf_8[k+2+16];
				sum += s[x*(k+3)] * bicubic_cf_8[k+3+16];
				sum += s[x*(k+4)] * bicubic_cf_8[k+4+16];
				sum += s[x*(k+5)] * bicubic_cf_8[k+5+16];
				sum += s[x*(k+6)] * bicubic_cf_8[k+6+16];
				sum += s[x*(k+7)] * bicubic_cf_8[k+7+16];
			}
			d[j] = CLIP((sum+ADD_C)>>BASE);
		}
		d = dst + step;
		for (j = 0; j < x; j++) {
			sum = 0;
			s = src + j;
			for (k = 0; k < 8; k += 8) {
				sum += s[0] * bicubic_cf_8[k+0];
				sum += s[1] * bicubic_cf_8[k+1];
				sum += s[2] * bicubic_cf_8[k+2];
				sum += s[3] * bicubic_cf_8[k+3];
				sum += s[4] * bicubic_cf_8[k+4];
				sum += s[5] * bicubic_cf_8[k+5];
				sum += s[6] * bicubic_cf_8[k+6];
				sum += s[7] * bicubic_cf_8[k+7];
			}
			for (k = 0; k < 24; k += 8) {
				sum += s[x*(k+0)] * bicubic_cf_8[k+0+8];
				sum += s[x*(k+1)] * bicubic_cf_8[k+1+8];
				sum += s[x*(k+2)] * bicubic_cf_8[k+2+8];
				sum += s[x*(k+3)] * bicubic_cf_8[k+3+8];
				sum += s[x*(k+4)] * bicubic_cf_8[k+4+8];
				sum += s[x*(k+5)] * bicubic_cf_8[k+5+8];
				sum += s[x*(k+6)] * bicubic_cf_8[k+6+8];
				sum += s[x*(k+7)] * bicubic_cf_8[k+7+8];
			}
			d[j] = CLIP((sum+ADD_C)>>BASE);
		}
		d = dst + step * (newy - 2);
		for (j = 0; j < x; j++) {
			sum = 0;
			s = src + (x * ((newy - 1) * 8 - 16) + j);
			for (k = 0; k < 16; k += 8) {
				sum += s[x*j+0] * bicubic_cf_8[k+0];
				sum += s[x*j+1] * bicubic_cf_8[k+1];
				sum += s[x*j+2] * bicubic_cf_8[k+2];
				sum += s[x*j+3] * bicubic_cf_8[k+3];
				sum += s[x*j+4] * bicubic_cf_8[k+4];
				sum += s[x*j+5] * bicubic_cf_8[k+5];
				sum += s[x*j+6] * bicubic_cf_8[k+6];
				sum += s[x*j+7] * bicubic_cf_8[k+7];
			}
			for (k = 16; k < 32; k += 8) {
				sum += s[x*16+0] * bicubic_cf_8[k+0];
				sum += s[x*16+1] * bicubic_cf_8[k+1];
				sum += s[x*16+2] * bicubic_cf_8[k+2];
				sum += s[x*16+3] * bicubic_cf_8[k+3];
				sum += s[x*16+4] * bicubic_cf_8[k+4];
				sum += s[x*16+5] * bicubic_cf_8[k+5];
				sum += s[x*16+6] * bicubic_cf_8[k+6];
				sum += s[x*16+7] * bicubic_cf_8[k+7];
			}
			d[j] = CLIP((sum+ADD_C)>>BASE);
		}
		d = dst + step * (newy - 1);
		for (j = 0; j < x; j++) {
			sum = 0;
			s = src + (x * ((newy - 1) * 8 - 8) + j);
			for (k = 0; k < 8; k += 8) {
				sum += s[x*(8+0)] * bicubic_cf_8[k+0];
				sum += s[x*(8+1)] * bicubic_cf_8[k+1];
				sum += s[x*(8+2)] * bicubic_cf_8[k+2];
				sum += s[x*(8+3)] * bicubic_cf_8[k+3];
				sum += s[x*(8+4)] * bicubic_cf_8[k+4];
				sum += s[x*(8+5)] * bicubic_cf_8[k+5];
				sum += s[x*(8+6)] * bicubic_cf_8[k+6];
				sum += s[x*(8+7)] * bicubic_cf_8[k+7];
			}
			for (k = 8; k < 32; k += 8) {
				sum += s[x*(8+0)] * bicubic_cf_8[k+0];
				sum += s[x*(8+1)] * bicubic_cf_8[k+1];
				sum += s[x*(8+2)] * bicubic_cf_8[k+2];
				sum += s[x*(8+3)] * bicubic_cf_8[k+3];
				sum += s[x*(8+4)] * bicubic_cf_8[k+4];
				sum += s[x*(8+5)] * bicubic_cf_8[k+5];
				sum += s[x*(8+6)] * bicubic_cf_8[k+6];
				sum += s[x*(8+7)] * bicubic_cf_8[k+7];
			}
			d[j] = CLIP((sum+ADD_C)>>BASE);
		}
	}

	_m_empty();
}