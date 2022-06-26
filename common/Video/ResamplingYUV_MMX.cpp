/**
 ************************************************************************************************
 * \file ResamplingYUV_MMX.cpp
 * \brief YUV420 image scaling up functions [MMX version]
 *
 * (c) 2006 Visicron Corp.  http://www.visicron.com/
 *
 * \b Project: High Quality Resampling
 * \author Smirnov Konstatntin
 * \author Sergey Anufriev
 * \date 25.07.2006
 *
 * $Revision: 1 $
 * $History: ResamplingYUV_MMX.cpp $
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
 *
 **************************************************************************************************/

#include "math.h"
#include "emmintrin.h"
#include "Init.h"

void InterpolationUVPlane_MMX_X(unsigned char *src, unsigned char *dst, int src_width, int src_height, int dst_width);
void InterpolationUVPlane_MMX_Y(unsigned char *src, unsigned char *dst, int dst_width, int src_height, int dst_height);
void InterpolationUVPlane_C_X(unsigned char *src, unsigned char *dst, int src_width, int src_height, int dst_width);

void InterpolationUVPlane_MMX(unsigned char *src, unsigned char *dst, VS_InitStruct* Str)
{
	int src_width = Str->srcW, src_height = Str->srcH, dst_width = Str->dstW, dst_height = Str->dstH,
		src_width_2 = src_width / 2, src_height_2 = src_height / 2,
		dst_width_2 = dst_width / 2, dst_height_2 = dst_height / 2;
	unsigned char *src_u = src + src_width * src_height,
				  *src_v = src_u + src_width_2 * src_height_2,
				  *dst_u = dst + dst_width * dst_height,
				  *dst_v = dst_u + dst_width_2 * dst_height_2;

	unsigned char* temp = (unsigned char*)VS_malloc(sizeof(unsigned char)*src_height_2*dst_width_2);

	InterpolationUVPlane_MMX_X(src_u, temp, src_width_2, src_height_2, dst_width_2);
	InterpolationUVPlane_MMX_Y(temp, dst_u, dst_width_2, src_height_2, dst_height_2);

	InterpolationUVPlane_MMX_X(src_v, temp, src_width_2, src_height_2, dst_width_2);
	InterpolationUVPlane_MMX_Y(temp, dst_v, dst_width_2, src_height_2, dst_height_2);

	VS_free(temp);
}

/// 8 bit, x direction
void InterpolationUVPlane_MMX_X(unsigned char *src, unsigned char *dst, int src_width, int src_height, int dst_width)
{
	InterpolationUVPlane_C_X(src, dst, src_width, src_height, dst_width);
}

/// 8 bit, y direction, MMX version
void InterpolationUVPlane_MMX_Y(unsigned char *src, unsigned char *dst, int dst_width, int src_height, int dst_height)
{
	int i,j;
	double factor = (double)(src_height - 1) / (dst_height - 1);
	bool proc_end = (dst_width & 7) != 0;

	unsigned short *y_coef = (unsigned short*)VS_malloc(sizeof(unsigned short)*dst_height*2);

	for (j = 0; j < dst_height; j++) {
		y_coef[j*2+0] = (int)(factor * j);
		y_coef[j*2+1] = interp_lin(factor * j);
	}

	memcpy(dst, src, dst_width);
	const __m64 y_add = _mm_set1_pi16(RADIX / 2);
	for (j = 1; j < dst_height - 1; j++) {
		int y_pos = y_coef[j*2+0];
		short diry_coef = y_coef[j*2+1];
		short invy_coef = RADIX - diry_coef;
		unsigned char * ps = src + dst_width * y_pos;
		unsigned char * pd = dst + dst_width * j;
		const __m64 diry = _mm_set1_pi16(diry_coef);
		const __m64 invy = _mm_set1_pi16(invy_coef);
		__m64 m0 = {0}, m1, m2, m3, m4;
		i = 0;
		for (; i < dst_width; i += 8) {
			m1 = *(__m64 *)(ps + i);
			m2 = *(__m64 *)(ps + dst_width + i);
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
		if (proc_end) {
			for (i -= 8; i < dst_width; i++)
				pd[i+0] = (ps[i+0] * diry_coef + ps[dst_width + i + 0] * invy_coef + RADIX / 2) >> BASE;
		}
	}
	_m_empty();
	memcpy(dst + dst_width * j, src + dst_width * (src_height - 1), dst_width);
	VS_free(y_coef);
}

/**
 **************************************************************************************************
 * Intensity interpolation [MMX version]
 * \param  [in]		  intens         - array of source value intensity
 * \param  [out]	  interp         - array of inerpolate value intensity
 * \param  [in]       position_y     - array of Y coordinates of source image that are reference points
 *									   to interpolated Y values of destination image pixels
 * \param  [in]       position_x     - array of X coordinates of source image that are reference points
 *									   to interpolated X values of destination image pixels
 * \param  [in]       difference_x   - array of difference on X direction for every points of destination image
 * \param  [in]       difference_y   - array of difference on Y direction for every points of destination image
 * \param  [in]       sx             - number of pixels in source image on X direction
 * \param  [in]       sy             - number of pixels in source image on Y direction
 * \param  [in]       Sx             - number of pixels in destination image on X direction
 * \param  [in]       Sy             - number of pixels in destination image on Y direction
 **************************************************************************************************
 */
void IntensityInterpolate_MMX(unsigned char* intens, unsigned char* interp, int* position_y, int* position_x,
							  double* difference_x, double* difference_y, int sx, int sy, int Sx, int Sy)
{
	int j = 0, k = 0, i = 0;
	int y = 0, x = 0, index = 0, index_ = 0;
	int t = 0;

	short int* temp = (short int*)VS_malloc(sizeof(short int)*Sx*(sy+1));
	short int* delta_t_x = (short int*)VS_malloc(sizeof(short int)*Sx);
	short int* ind_x = (short int*)VS_malloc(sizeof(short int)*Sx);
	short int* delta_t_y = (short int*)VS_malloc(sizeof(short int)*Sy);
	int* ind_y = (int*)VS_malloc(sizeof(int)*Sy);

	for (k = 0; k < Sx; k++)
	{
		x = position_x[k];
		i = int(difference_x[k] + 0.5);
		ind_x[k] = x + i;
		delta_t_x[k] = int((difference_x[k] + 0.5 - i) * 64 + 0.5);
	}

	for (j = 0; j < sy + 1; j++)
	{
		index = j * (sx + 1);
		index_ = j * Sx;
		for (k = 0; k < Sx; k++)
		{
			i = ind_x[k];
			t = delta_t_x[k];
			temp[index_+k] = intens[index+i] + ((t * (intens[index+i+1] -
				                 intens[index+i]) + 32) >> 6);
		}
	}

	VS_free(ind_x);
	VS_free(delta_t_x);

	for (k = 0; k < Sy; k++)
	{
		y = position_y[k];
		i = int(difference_y[k] + 0.5);
		ind_y[k] = (y + i) * Sx;
		delta_t_y[k] = int((difference_y[k] + 0.5 - i) * 64 + 0.5);
	}

	int nLoop = (Sx / 4) * 4;
	int delta = Sx - nLoop;

	__m64 mm0, mm1, mm4, mm5;
	const __m64 delta_32 = _mm_set_pi16(32, 32, 32, 32), nNull = _m_from_int(0);

	for (j = 0; j < Sy; j++)
	{
		y = position_y[j];
		index = ind_y[j];
		t = delta_t_y[j];
		mm4 = _mm_set1_pi16(t);
		short int *ptr = temp + index;
		unsigned char *ptr_out = interp + j * Sx;
		for (k = 0; k < nLoop; k += 4)
		{
			mm0 = *(__m64*)(ptr);
			mm1 = *(__m64*)(ptr+Sx);

			mm5 = _mm_subs_pi16(mm1, mm0);
			mm5 = _mm_mullo_pi16(mm5, mm4);

			mm5 = _mm_adds_pi16(mm5, delta_32);
			mm5 = _mm_srai_pi16(mm5, 6);
			mm5 = _mm_adds_pi16(mm5, mm0);
			mm5 = _mm_packs_pu16(mm5, nNull);
			*(int*)(ptr_out+k) = _m_to_int(mm5);
			ptr += 4;
		}

		if (delta)
		{
			short int *ptr = temp + index;
			for (k = nLoop; k < nLoop + delta; k++)
			{
				ptr_out[k] = ptr[k] + ((t * (ptr[Sx+k] -
					            ptr[k]) + 32) >> 6);
			}
		}
	}
	_mm_empty ();

	VS_free(temp);
	VS_free(ind_y);
	VS_free(delta_t_y);
}

/**
 **************************************************************************************************
 * High Quality mode YUV image processing [MMX variant]
 * \param  [in,out]		out_Img			- destination image
 * \param  [in]			Str				- pointer on structure
 **************************************************************************************************
 */
void Stage_HighQualityYUV_MMX(unsigned char* out_Img, VS_InitStruct* Str)
{
	Stage_HighQualityYUV_C(out_Img, Str);
}