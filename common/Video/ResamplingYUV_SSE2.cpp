/**
 ************************************************************************************************
 * \file ResamplingYUV_SSE2.cpp
 * \brief YUV420 image scaling up functions [SSE2 version]
 *
 * (c) 2006 Visicron Corp.  http://www.visicron.com/
 *
 * \b Project: High Quality Resampling
 * \author Smirnov Konstatntin
 * \author Sergey Anufriev
 * \date 24.07.2006
 *
 * $Revision: 1 $
 * $History: ResamplingYUV_SSE2.cpp $
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

/**
 **************************************************************************************************
 * YUV420 image Fast Speed mode interpolation [SSE2 version]
 * \param  [in]	      in_Img         - source image
 * \param  [out]	  out_Img        - destination image
 * \param  [in]       Str			 - input information structure
 **************************************************************************************************
 */
void InterpolationImageYUV420_Fast_SSE2(unsigned char* src, unsigned char* dst, VS_InitStruct* Str)
{
	unsigned long src_w = Str->srcW, src_h = Str->srcH, dst_w = Str->dstW, dst_h = Str->dstH;
	unsigned long i = 0, l = 0, k = 0, j = 0;
	int dy = 0, dx = 0, index = 0, index_ = 0;
	int ddy = 0, ddx = 0;
	unsigned long x = 0, y = 0, sx_ = src_w - 2, sy_ = src_h - 2;

	unsigned long DX = 0, DY = 0, NUMX = 0, NUMY = 0;

	__m64 mm0, mm1, mm2, mm3, delta_4096_mmx = _mm_set_pi32(0x1000, 0x1000), nNull_mmx = _mm_setzero_si64 ();
	__m128i nNull_xmm = _mm_setzero_si128 (), delta_4096 =  _mm_set_epi32(0x1000,0x1000,0x1000,0x1000);
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, coef, coef1, coef2, coef3, temp;

	signed short int* temp_Img = (signed short int*)VS_malloc(sizeof(signed short int)*src_h*dst_w);

	//interpolation on X direction
	for (y = 0; y < src_h ; y++)
	{
		DX = Str->num_ptn_x[0];
		NUMX = 0;
		dy = y * src_w;

		unsigned char * ptr = src + dy;
		signed short int * ptr_out = temp_Img + y * dst_w;

		mm0 = _m_from_int(*(int *)(ptr+0));
		mm0 = _m_punpcklbw(mm0, nNull_mmx);
		mm1 = mm0;
		mm1 = _m_psrlqi(mm1, 16);
		mm0 = _m_punpcklwd(mm0, mm0);
		mm0 = _m_punpckldq(mm0, mm1);

		for (k = NUMX; k < DX; k++)
		{
			mm1 = *(__m64 *)(Str->coefficient_x+4*k);
			mm1 = _m_pmaddwd(mm1, mm0);
			mm2 = mm1;
			mm2 = _m_psrlqi(mm2, 32);
			mm1 = _m_paddd(mm1, mm2);
			mm1 = _m_paddd(mm1, delta_4096_mmx);
			mm1 = _m_psradi(mm1, 13);
			mm1 = _m_punpckldq(mm1, nNull_mmx);
			mm1 = _m_packssdw(mm1, nNull_mmx);
			ptr_out[k] = _m_to_int(mm1);
		}

		NUMX = DX;

		for (x = 1; x < sx_; x++)
		{
			DX = Str->num_ptn_x[x];
			ddx = x - 1;

			dy = y * src_w + ddx;
			unsigned char *ptr = src + dy;

			mm0 = _m_from_int(*(int *)(ptr+0));
			mm0 = _m_punpcklbw(mm0, nNull_mmx);
			for (k = NUMX; k < DX; k++)
			{
				mm1 = *(__m64 *)(Str->coefficient_x+4*k);
				mm1 = _m_pmaddwd(mm1, mm0);
				mm2 = mm1;
				mm2 = _m_psrlqi(mm2, 32);
				mm1 = _m_paddd(mm1, mm2);
				mm1 = _m_paddd(mm1, delta_4096_mmx);
				mm1 = _m_psradi(mm1, 13);
				mm1 = _m_punpckldq(mm1, nNull_mmx);
				mm1 = _m_packssdw(mm1, nNull_mmx);
				ptr_out[k] = _m_to_int(mm1);
			}
			NUMX = DX;
		}

		DX = Str->num_ptn_x[sx_];

		mm0 = _m_psrlqi(mm0, 16);
		mm1 = _m_punpckhwd(mm0, nNull_mmx);
		mm1 = _m_punpcklwd(mm1, mm1);
		mm0 = _m_punpckldq(mm0, mm1);

		for (k = NUMX; k < DX; k++)
		{
			mm1 = *(__m64 *)(Str->coefficient_x+4*k);
			mm1 = _m_pmaddwd(mm1, mm0);
			mm2 = mm1;
			mm2 = _m_psrlqi(mm2, 32);
			mm1 = _m_paddd(mm1, mm2);
			mm1 = _m_paddd(mm1, delta_4096_mmx);
			mm1 = _m_psradi(mm1, 13);
			mm1 = _m_punpckldq(mm1, nNull_mmx);
			mm1 = _m_packssdw(mm1, nNull_mmx);
			ptr_out[k] = _m_to_int(mm1);
		}
	}

	//interpolation on Y direction
	NUMY = 0;
	DY = Str->num_ptn_y[0];
	dy = 0;
	unsigned long nLoop = (dst_w / 8) * 8;
	int delta = dst_w - nLoop;

	for (k = NUMY; k < DY; k++)
	{
        index = k * dst_w;
		unsigned char * ptr_out = dst + index;
		signed short * ptr = temp_Img;

		coef = *(__m128i *)(Str->coefficient_y+k*32);
		coef1 = *(__m128i *)(Str->coefficient_y+k*32+8);
		coef2 = *(__m128i *)(Str->coefficient_y+k*32+16);
		coef3 = *(__m128i *)(Str->coefficient_y+k*32+24);

		for (x = 0; x < nLoop; x += 8)
		{
			xmm0 = _mm_loadu_si128((__m128i *)ptr);
			xmm1 = _mm_loadu_si128((__m128i *)ptr);
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+dst_w));
			xmm3 = _mm_loadu_si128((__m128i *)(ptr+2*dst_w));

			temp = xmm0;
			xmm0 = _mm_mullo_epi16 (xmm0, coef);
			temp = _mm_mulhi_epi16 (temp, coef);
			xmm4 = xmm0;
			xmm0 = _mm_unpacklo_epi16(xmm0, temp);//first point
			xmm4 = _mm_unpackhi_epi16(xmm4, temp);//second point

			temp = xmm1;
			xmm1 = _mm_mullo_epi16 (xmm1, coef1);
			temp = _mm_mulhi_epi16 (temp, coef1);
			xmm5 = xmm1;
			xmm1 = _mm_unpacklo_epi16(xmm1, temp);//first point
			xmm5 = _mm_unpackhi_epi16(xmm5, temp);//second point
			xmm0 = _mm_add_epi32(xmm0, xmm1);
			xmm4 = _mm_add_epi32(xmm4, xmm5);

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef2);
			temp = _mm_mulhi_epi16 (temp, coef2);
			xmm5 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm5 = _mm_unpackhi_epi16(xmm5, temp);//second point
			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm4 = _mm_add_epi32(xmm4, xmm5);

			temp = xmm3;
			xmm3 = _mm_mullo_epi16 (xmm3, coef3);
			temp = _mm_mulhi_epi16 (temp, coef3);
			xmm5 = xmm3;
			xmm3 = _mm_unpacklo_epi16(xmm3, temp);//first point
			xmm5 = _mm_unpackhi_epi16(xmm5, temp);//second point
			xmm0 = _mm_add_epi32(xmm0, xmm3);
			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm0 = _mm_add_epi32 (xmm0, delta_4096);
			xmm4 = _mm_add_epi32 (xmm4, delta_4096);
			xmm0 = _mm_srai_epi32(xmm0, 13);
			xmm4 = _mm_srai_epi32(xmm4, 13);
			xmm0 = _mm_packs_epi32(xmm0, nNull_xmm);
			xmm4 = _mm_packs_epi32(xmm4, nNull_xmm);
			xmm0 = _mm_packus_epi16(xmm0, nNull_xmm);
			xmm4 = _mm_packus_epi16(xmm4, nNull_xmm);

			*(int*)(ptr_out+x) = _mm_cvtsi128_si32(xmm0);
			*(int*)(ptr_out+x+4) = _mm_cvtsi128_si32(xmm4);

			ptr += 8;
		}
		if (delta)
		{
			int delta_loop = 0;
			delta -= 4;
			if (delta >= 0) {
				delta_loop = 4;
				mm0 = *(__m64 *)(ptr);
				mm1 = *(__m64 *)(ptr);
				mm2 = *(__m64 *)(ptr+dst_w);
				mm3 = *(__m64 *)(ptr+2*dst_w);

				xmm0 = _mm_setr_epi64 (mm0, mm1);
				xmm1 = _mm_setr_epi64 (mm2, mm3);

				coef = _mm_unpacklo_epi64(coef, coef1);
				coef1 = _mm_unpacklo_epi64(coef2, coef3);

				xmm2 = _mm_mullo_epi16 (xmm0, coef);
				xmm3 = _mm_mulhi_epi16 (xmm0, coef);

				temp = xmm2;
				xmm2 = _mm_unpacklo_epi16(xmm2, xmm3);
				temp = _mm_unpackhi_epi16(temp, xmm3);
				xmm2 = _mm_add_epi32(xmm2, temp);

				xmm3 = _mm_mullo_epi16 (xmm1, coef1);
				xmm4 = _mm_mulhi_epi16 (xmm1, coef1);

				temp = xmm3;
				xmm3 = _mm_unpacklo_epi16(xmm3, xmm4);
				temp = _mm_unpackhi_epi16(temp, xmm4);
				xmm3 = _mm_add_epi32(xmm3, temp);

				xmm2 = _mm_add_epi32(xmm2, xmm3);
				xmm2 = _mm_add_epi32 (xmm2, delta_4096);
				xmm2 = _mm_srai_epi32(xmm2, 13);
				xmm2 = _mm_packs_epi32(xmm2, nNull_xmm);
				xmm2 = _mm_packus_epi16(xmm2, nNull_xmm);
				*(int*)(ptr_out+x) = _mm_cvtsi128_si32(xmm2);
				ptr += 4;
			}
			if (delta) {
				signed short a0 = 0, a1 = 0, a2 = 0, a3= 0;
				signed short c0 = Str->coefficient_y[k*32+ 0],
							 c1 = Str->coefficient_y[k*32+ 8],
							 c2 = Str->coefficient_y[k*32+16],
							 c3 = Str->coefficient_y[k*32+24];
				int pixel = 0;
				for (x = nLoop + delta_loop; x < dst_w; x++) {
					pixel = 0;
					a0 = ptr[0];
					a1 = ptr[0];
					a2 = ptr[dst_w];
					a3 = ptr[2*dst_w];
					pixel += c0 * a0;
					pixel += c1 * a1;
					pixel += c2 * a2;
					pixel += c3 * a3;
					if (pixel > 255) pixel = 255;
					if (pixel < 0) pixel = 0;
					ptr_out[x] = pixel;
					ptr++;
				}
			}
			delta += 4;
		}
	}

	NUMY = DY;

	for (y = 1; y < sy_; y++)
	{
		DY = Str->num_ptn_y[y];
		dy = (y - 1) * dst_w;

		for (k = NUMY; k < DY; k++)
		{
            index = k * dst_w;
			unsigned char * ptr_out = dst + index;
			signed short * ptr = temp_Img + dy;

			coef = *(__m128i *)(Str->coefficient_y+k*32);
			coef1 = *(__m128i *)(Str->coefficient_y+k*32+8);
			coef2 = *(__m128i *)(Str->coefficient_y+k*32+16);
			coef3 = *(__m128i *)(Str->coefficient_y+k*32+24);

			for (x = 0; x < nLoop; x += 8)
			{
				xmm0 = _mm_loadu_si128((__m128i *)(ptr));
				xmm1 = _mm_loadu_si128((__m128i *)(ptr+dst_w));
				xmm2 = _mm_loadu_si128((__m128i *)(ptr+2*dst_w));
				xmm3 = _mm_loadu_si128((__m128i *)(ptr+3*dst_w));

				temp = xmm0;
				xmm0 = _mm_mullo_epi16 (xmm0, coef);
				temp = _mm_mulhi_epi16 (temp, coef);
				xmm4 = xmm0;
				xmm0 = _mm_unpacklo_epi16(xmm0, temp);//first point
				xmm4 = _mm_unpackhi_epi16(xmm4, temp);//second point

				temp = xmm1;
				xmm1 = _mm_mullo_epi16 (xmm1, coef1);
				temp = _mm_mulhi_epi16 (temp, coef1);
				xmm5 = xmm1;
				xmm1 = _mm_unpacklo_epi16(xmm1, temp);//first point
				xmm5 = _mm_unpackhi_epi16(xmm5, temp);//second point
				xmm0 = _mm_add_epi32(xmm0, xmm1);
				xmm4 = _mm_add_epi32(xmm4, xmm5);

				temp = xmm2;
				xmm2 = _mm_mullo_epi16 (xmm2, coef2);
				temp = _mm_mulhi_epi16 (temp, coef2);
				xmm5 = xmm2;
				xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
				xmm5 = _mm_unpackhi_epi16(xmm5, temp);//second point
				xmm0 = _mm_add_epi32(xmm0, xmm2);
				xmm4 = _mm_add_epi32(xmm4, xmm5);

				temp = xmm3;
				xmm3 = _mm_mullo_epi16 (xmm3, coef3);
				temp = _mm_mulhi_epi16 (temp, coef3);
				xmm5 = xmm3;
				xmm3 = _mm_unpacklo_epi16(xmm3, temp);//first point
				xmm5 = _mm_unpackhi_epi16(xmm5, temp);//second point
				xmm0 = _mm_add_epi32(xmm0, xmm3);
				xmm4 = _mm_add_epi32(xmm4, xmm5);

				xmm0 = _mm_add_epi32 (xmm0, delta_4096);
				xmm4 = _mm_add_epi32 (xmm4, delta_4096);
				xmm0 = _mm_srai_epi32(xmm0, 13);
				xmm4 = _mm_srai_epi32(xmm4, 13);
				xmm0 = _mm_packs_epi32(xmm0, nNull_xmm);
				xmm4 = _mm_packs_epi32(xmm4, nNull_xmm);
				xmm0 = _mm_packus_epi16(xmm0, nNull_xmm);
				xmm4 = _mm_packus_epi16(xmm4, nNull_xmm);

				*(int*)(ptr_out+x) = _mm_cvtsi128_si32(xmm0);
				*(int*)(ptr_out+x+4) = _mm_cvtsi128_si32(xmm4);

				ptr += 8;
			}
			if (delta)
			{
				int delta_loop = 0;
				delta  -= 4;
				if (delta >= 0) {
					delta_loop = 4;
					mm0 = *(__m64 *)(ptr);
					mm1 = *(__m64 *)(ptr+dst_w);
					mm2 = *(__m64 *)(ptr+2*dst_w);
					mm3 = *(__m64 *)(ptr+3*dst_w);

					xmm0 = _mm_setr_epi64 (mm0, mm1);
					xmm1 = _mm_setr_epi64 (mm2, mm3);

					coef = _mm_unpacklo_epi64(coef, coef1);
					coef1 = _mm_unpacklo_epi64(coef2, coef3);

					xmm2 = _mm_mullo_epi16 (xmm0, coef);
					xmm3 = _mm_mulhi_epi16 (xmm0, coef);

					temp = xmm2;
					xmm2 = _mm_unpacklo_epi16(xmm2, xmm3);
					temp = _mm_unpackhi_epi16(temp, xmm3);
					xmm2 = _mm_add_epi32(xmm2, temp);

					xmm3 = _mm_mullo_epi16 (xmm1, coef1);
					xmm4 = _mm_mulhi_epi16 (xmm1, coef1);

					temp = xmm3;
					xmm3 = _mm_unpacklo_epi16(xmm3, xmm4);
					temp = _mm_unpackhi_epi16(temp, xmm4);
					xmm3 = _mm_add_epi32(xmm3, temp);

					xmm2 = _mm_add_epi32(xmm2, xmm3);
					xmm2 = _mm_add_epi32 (xmm2, delta_4096);
					xmm2 = _mm_srai_epi32(xmm2, 13);
					xmm2 = _mm_packs_epi32(xmm2, nNull_xmm);
					xmm2 = _mm_packus_epi16(xmm2, nNull_xmm);
					*(int*)(ptr_out+x) = _mm_cvtsi128_si32(xmm2);
					ptr += 4;
				}
				if (delta) {
					signed short a0 = 0, a1 = 0, a2 = 0, a3= 0;
					signed short c0 = Str->coefficient_y[k*32+ 0],
								 c1 = Str->coefficient_y[k*32+ 8],
								 c2 = Str->coefficient_y[k*32+16],
								 c3 = Str->coefficient_y[k*32+24];
					int pixel = 0;
					for (x = nLoop + delta_loop; x < dst_w; x++) {
						pixel = 0;
						a0 = ptr[0];
						a1 = ptr[dst_w];
						a2 = ptr[2*dst_w];
						a3 = ptr[3*dst_w];
						pixel += c0 * a0;
						pixel += c1 * a1;
						pixel += c2 * a2;
						pixel += c3 * a3;
						if (pixel > 255) pixel = 255;
						if (pixel < 0) pixel = 0;
						ptr_out[x] = pixel;
						ptr++;
					}
				}
				delta += 4;
			}
		}
		NUMY = DY;
	}

	DY = Str->num_ptn_y[sy_];
	dy = (sy_ - 1) * dst_w;

	for (k = NUMY; k < DY; k++)
	{
        index = k * dst_w;
		unsigned char * ptr_out = dst + index;
		signed short int * ptr = temp_Img + dy;

			coef = *(__m128i *)(Str->coefficient_y+k*32);
			coef1 = *(__m128i *)(Str->coefficient_y+k*32+8);
			coef2 = *(__m128i *)(Str->coefficient_y+k*32+16);
			coef3 = *(__m128i *)(Str->coefficient_y+k*32+24);

			for (x = 0; x < nLoop; x += 8)
			{
				xmm0 = _mm_loadu_si128((__m128i *)(ptr));
				xmm1 = _mm_loadu_si128((__m128i *)(ptr+dst_w));
				xmm2 = _mm_loadu_si128((__m128i *)(ptr+2*dst_w));
				xmm3 = _mm_loadu_si128((__m128i *)(ptr+2*dst_w));

				temp = xmm0;
				xmm0 = _mm_mullo_epi16 (xmm0, coef);
				temp = _mm_mulhi_epi16 (temp, coef);
				xmm4 = xmm0;
				xmm0 = _mm_unpacklo_epi16(xmm0, temp);//first point
				xmm4 = _mm_unpackhi_epi16(xmm4, temp);//second point

				temp = xmm1;
				xmm1 = _mm_mullo_epi16 (xmm1, coef1);
				temp = _mm_mulhi_epi16 (temp, coef1);
				xmm5 = xmm1;
				xmm1 = _mm_unpacklo_epi16(xmm1, temp);//first point
				xmm5 = _mm_unpackhi_epi16(xmm5, temp);//second point
				xmm0 = _mm_add_epi32(xmm0, xmm1);
				xmm4 = _mm_add_epi32(xmm4, xmm5);

				temp = xmm2;
				xmm2 = _mm_mullo_epi16 (xmm2, coef2);
				temp = _mm_mulhi_epi16 (temp, coef2);
				xmm5 = xmm2;
				xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
				xmm5 = _mm_unpackhi_epi16(xmm5, temp);//second point
				xmm0 = _mm_add_epi32(xmm0, xmm2);
				xmm4 = _mm_add_epi32(xmm4, xmm5);

				temp = xmm3;
				xmm3 = _mm_mullo_epi16 (xmm3, coef3);
				temp = _mm_mulhi_epi16 (temp, coef3);
				xmm5 = xmm3;
				xmm3 = _mm_unpacklo_epi16(xmm3, temp);//first point
				xmm5 = _mm_unpackhi_epi16(xmm5, temp);//second point
				xmm0 = _mm_add_epi32(xmm0, xmm3);
				xmm4 = _mm_add_epi32(xmm4, xmm5);

				xmm0 = _mm_add_epi32 (xmm0, delta_4096);
				xmm4 = _mm_add_epi32 (xmm4, delta_4096);
				xmm0 = _mm_srai_epi32(xmm0, 13);
				xmm4 = _mm_srai_epi32(xmm4, 13);
				xmm0 = _mm_packs_epi32(xmm0, nNull_xmm);
				xmm4 = _mm_packs_epi32(xmm4, nNull_xmm);
				xmm0 = _mm_packus_epi16(xmm0, nNull_xmm);
				xmm4 = _mm_packus_epi16(xmm4, nNull_xmm);

				*(int*)(ptr_out+x) = _mm_cvtsi128_si32(xmm0);
				*(int*)(ptr_out+x+4) = _mm_cvtsi128_si32(xmm4);

				ptr += 8;
			}
			if (delta)
			{
				int delta_loop = 0;
				delta  -= 4;
				if (delta >= 0) {
					delta_loop = 4;
					mm0 = *(__m64 *)(ptr);
					mm1 = *(__m64 *)(ptr+dst_w);
					mm2 = *(__m64 *)(ptr+2*dst_w);
					mm3 = *(__m64 *)(ptr+2*dst_w);

					xmm0 = _mm_setr_epi64 (mm0, mm1);
					xmm1 = _mm_setr_epi64 (mm2, mm3);

					coef = _mm_unpacklo_epi64(coef, coef1);
					coef1 = _mm_unpacklo_epi64(coef2, coef3);

					xmm2 = _mm_mullo_epi16 (xmm0, coef);
					xmm3 = _mm_mulhi_epi16 (xmm0, coef);

					temp = xmm2;
					xmm2 = _mm_unpacklo_epi16(xmm2, xmm3);
					temp = _mm_unpackhi_epi16(temp, xmm3);
					xmm2 = _mm_add_epi32(xmm2, temp);

					xmm3 = _mm_mullo_epi16 (xmm1, coef1);
					xmm4 = _mm_mulhi_epi16 (xmm1, coef1);

					temp = xmm3;
					xmm3 = _mm_unpacklo_epi16(xmm3, xmm4);
					temp = _mm_unpackhi_epi16(temp, xmm4);
					xmm3 = _mm_add_epi32(xmm3, temp);

					xmm2 = _mm_add_epi32(xmm2, xmm3);
					xmm2 = _mm_add_epi32 (xmm2, delta_4096);
					xmm2 = _mm_srai_epi32(xmm2, 13);
					xmm2 = _mm_packs_epi32(xmm2, nNull_xmm);
					xmm2 = _mm_packus_epi16(xmm2, nNull_xmm);
					*(int*)(ptr_out+x) = _mm_cvtsi128_si32(xmm2);
					ptr += 4;
				}
				if (delta) {
					signed short a0 = 0, a1 = 0, a2 = 0, a3= 0;
					signed short c0 = Str->coefficient_y[k*32+ 0],
								 c1 = Str->coefficient_y[k*32+ 8],
								 c2 = Str->coefficient_y[k*32+16],
								 c3 = Str->coefficient_y[k*32+24];
					int pixel = 0;
					for (x = nLoop + delta_loop; x < dst_w; x++) {
						pixel = 0;
						a0 = ptr[0];
						a1 = ptr[dst_w];
						a2 = ptr[2*dst_w];
						a3 = ptr[2*dst_w];
						pixel += c0 * a0;
						pixel += c1 * a1;
						pixel += c2 * a2;
						pixel += c3 * a3;
						if (pixel > 255) pixel = 255;
						if (pixel < 0) pixel = 0;
						ptr_out[x] = pixel;
						ptr++;
					}
				}
				delta += 4;
			}
	}

	_mm_empty ();
	VS_free(temp_Img);
}

/**
 **************************************************************************************************
 * 32-bit image Improved Quality mode interpolation [SSE2 version]
 * \param  [in]	      in_Img         - source image
 * \param  [out]	  out_Img        - destination image
 * \param  [in]       Str			 - input information structure
 **************************************************************************************************
 */
void InterpolationImageYUV420_IQ_SSE2(unsigned char* src, unsigned char* dst, VS_InitStruct* Str)
{
	unsigned long src_w = Str->srcW, src_h = Str->srcH, dst_w = Str->dstW, dst_h = Str->dstH;
	unsigned long i = 0, l = 0, k = 0, j = 0;
	//вспомогательные индексы
	int dy = 0, dx = 0, index = 0, index_ = 0;
	int ddy = 0, ddx = 0;
	unsigned long x = 0, y = 0, sx_ = src_w - 2, sy_ = src_h - 2;

	unsigned long DX = 0, DY = 0, NUMX = 0, NUMY = 0;

	__m128i nNull_xmm = _mm_setzero_si128 (), delta_4096 =  _mm_set_epi32(0x1000,0x1000,0x1000,0x1000);
	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5,
		    coef, coef1, coef2, coef3, coef4, coef5, coef6, coef7,
			temp;

	signed short int* temp_Img = (signed short int*)VS_malloc(sizeof(signed short int)*src_h*dst_w);

	//interpolation on X direction
	for (y = 0; y < src_h ; y++)
	{
 		DX = Str->num_ptn_x[0];
		NUMX = 0;
		dy = y * src_w;

		unsigned char * ptr = src + dy;
		signed short int * ptr_out = temp_Img + y * dst_w;

		xmm0 = _mm_loadu_si128((__m128i *)(ptr+0));
		xmm2 = _mm_loadu_si128((__m128i *)(ptr+1));
		xmm3 = xmm2;

		xmm0 = _mm_unpacklo_epi8(xmm0, nNull_xmm);
		xmm1 = xmm0;
		xmm0 = _mm_unpacklo_epi16(xmm0, xmm0);
		xmm0 = _mm_unpacklo_epi32(xmm0, xmm0);
		xmm2 = _mm_unpacklo_epi8(xmm2, nNull_xmm);
		xmm0 = _mm_unpacklo_epi64(xmm0, xmm2);

		for (k = NUMX; k < DX; k++)
		{
			coef =  *(__m128i *)(Str->coefficient_x+8*k);

			xmm4 = _mm_mullo_epi16(xmm0, coef);
			xmm5 = _mm_mulhi_epi16(xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

			temp = _mm_srli_si128(xmm4, 8);
			xmm4 = _mm_add_epi32(xmm4, temp);
			temp = _mm_srli_si128(xmm4, 4);
			xmm4 = _mm_add_epi32(xmm4, temp);
            xmm4 = _mm_add_epi32 (xmm4, delta_4096);
			xmm4 = _mm_srai_epi32(xmm4, 13);
			xmm4 = _mm_unpacklo_epi32(xmm4, nNull_xmm);
			xmm4 = _mm_packs_epi32(xmm4, nNull_xmm);
			xmm4 = _mm_unpacklo_epi32(xmm4, nNull_xmm);

			_mm_storel_epi64((__m128i *)(ptr_out+k), xmm4);
		}

		NUMX = DX;
		DX = Str->num_ptn_x[1];

/*		xmm2 = _mm_unpacklo_epi16(xmm1, xmm2);
		xmm2 = _mm_unpacklo_epi32(xmm1, xmm2);
		xmm2 = _mm_loadu_si128((__m128i *)(ptr+2));
		xmm3 = xmm2;
		xmm2 = _mm_unpacklo_epi8(xmm2, nNull_xmm);
//		xmm3 = _mm_unpackhi_epi8(xmm3, nNull_xmm);
		xmm0 = _mm_packus_epi16(xmm1, xmm2);

		for (k = NUMX; k < DX; k++)
		{
			coef =  *(__m128i *)(Str->coefficient_x+8*k);

			xmm4 = _mm_mullo_epi16(xmm0, coef);
			xmm5 = _mm_mulhi_epi16(xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

            xmm4 = _mm_add_epi32 (xmm4, delta_4096);
			xmm4 = _mm_srai_epi32(xmm4, 13);
			xmm4 = _mm_packs_epi32(xmm4, nNull_xmm);

			_mm_storel_epi64((__m128i *)(ptr_out+k), xmm4);
		}

		NUMX = DX;
		DX = Str->num_ptn_x[2];

		xmm1 = _mm_srli_si128(xmm1, 8);
		xmm1 = _mm_unpacklo_epi64(xmm1, xmm2);
		xmm2 = _mm_loadu_si128((__m128i *)(ptr+12));
		xmm3 = xmm2;
		xmm2 = _mm_unpacklo_epi8(xmm2, nNull_xmm);
		xmm3 = _mm_unpackhi_epi8(xmm3, nNull_xmm);

		for (k = NUMX; k < DX; k++)
		{
			coef =  *(__m128i *)(Str->coefficient_x+32*k);
			coef1 = *(__m128i *)(Str->coefficient_x+32*k+8);
			coef2 = *(__m128i *)(Str->coefficient_x+32*k+16);
			coef3 = *(__m128i *)(Str->coefficient_x+32*k+24);

			//1-2 points
			xmm4 = _mm_mullo_epi16(xmm0, coef);
			xmm5 = _mm_mulhi_epi16(xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

			//3-4 points
			xmm5 = _mm_mullo_epi16 (xmm1, coef1);
			xmm6 = _mm_mulhi_epi16 (xmm1, coef1);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			//5-6 points
			xmm5 = _mm_mullo_epi16 (xmm2, coef2);
			xmm6 = _mm_mulhi_epi16 (xmm2, coef2);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			//7-8 points
			xmm5 = _mm_mullo_epi16 (xmm3, coef3);
			xmm6 = _mm_mulhi_epi16 (xmm3, coef3);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

            xmm4 = _mm_add_epi32 (xmm4, delta_4096);
			xmm4 = _mm_srai_epi32(xmm4, 13);
			xmm4 = _mm_packs_epi32(xmm4, nNull_xmm);

			_mm_storel_epi64((__m128i *)(ptr_out+4*k), xmm4);
		}*/

		NUMX = Str->num_ptn_x[2];//DX;

		for (x = 3; x <= sx_ - 3; x++)
		{
			DX = Str->num_ptn_x[x];
			ddx = x - 3;

			dy = y * src_w + ddx;
			unsigned char *ptr = src + dy;

//			xmm0 = _mm_srli_si128(xmm0, 8);
//			xmm0 = _mm_unpacklo_epi64(xmm0, xmm1);
//			xmm1 = _mm_srli_si128(xmm1, 8);
//			xmm1 = _mm_unpacklo_epi64(xmm1, xmm2);
			xmm0 = _mm_loadu_si128((__m128i *)(ptr+0));
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi8(xmm0, nNull_xmm);

//			xmm2 = _mm_loadu_si128((__m128i *)(ptr+16));
//			xmm3 = xmm2;
//			xmm2 = _mm_unpacklo_epi8(xmm2, nNull_xmm);
//			xmm3 = _mm_unpackhi_epi8(xmm3, nNull_xmm);

			for (k = NUMX; k < DX; k++)
			{
				coef =  *(__m128i *)(Str->coefficient_x+8*k);

				xmm4 = _mm_mullo_epi16(xmm0, coef);
				xmm5 = _mm_mulhi_epi16(xmm0, coef);

				temp = xmm4;
				xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
				temp = _mm_unpackhi_epi16(temp, xmm5);
				xmm4 = _mm_add_epi32(xmm4, temp);

				temp = _mm_srli_si128(xmm4, 8);
				xmm4 = _mm_add_epi32(xmm4, temp);
				temp = _mm_srli_si128(xmm4, 4);
				xmm4 = _mm_add_epi32(xmm4, temp);
                xmm4 = _mm_add_epi32 (xmm4, delta_4096);
				xmm4 = _mm_srai_epi32(xmm4, 13);
				xmm4 = _mm_unpacklo_epi32(xmm4, nNull_xmm);
				xmm4 = _mm_packs_epi32(xmm4, nNull_xmm);
				xmm4 = _mm_unpacklo_epi32(xmm4, nNull_xmm);

				_mm_storel_epi64((__m128i *)(ptr_out+k), xmm4);
			}
			NUMX = DX;
		}

/*		ddx = (sx_ - 5) << 2;
		dy = y * d_in_w + ddx;
		ptr = in_Img + dy;

		DX = Str->num_ptn_x[sx_-2];

		xmm0 = _mm_loadu_si128((__m128i *)(ptr));
		xmm2 = _mm_loadu_si128((__m128i *)(ptr+12));

		xmm1 = xmm0;
		xmm0 = _mm_unpacklo_epi8(xmm0, nNull_xmm);
		xmm1 = _mm_unpackhi_epi8(xmm1, nNull_xmm);
		xmm2 = _mm_srli_si128(xmm2, 4);
		xmm3 = xmm2;
		xmm2 = _mm_unpacklo_epi8(xmm2, nNull_xmm);
		xmm3 = _mm_unpackhi_epi32(xmm3, xmm3);
		xmm3 = _mm_unpacklo_epi8(xmm3, nNull_xmm);

		for (k = NUMX; k < DX; k++)
		{
			coef =  *(__m128i *)(Str->coefficient_x+32*k);
			coef1 = *(__m128i *)(Str->coefficient_x+32*k+8);
			coef2 = *(__m128i *)(Str->coefficient_x+32*k+16);
			coef3 = *(__m128i *)(Str->coefficient_x+32*k+24);

			//1-2 points
			xmm4 = _mm_mullo_epi16(xmm0, coef);
			xmm5 = _mm_mulhi_epi16(xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

			//3-4 points
			xmm5 = _mm_mullo_epi16 (xmm1, coef1);
			xmm6 = _mm_mulhi_epi16 (xmm1, coef1);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			//5-6 points
			xmm5 = _mm_mullo_epi16 (xmm2, coef2);
			xmm6 = _mm_mulhi_epi16 (xmm2, coef2);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			//7-8 points
			xmm5 = _mm_mullo_epi16 (xmm3, coef3);
			xmm6 = _mm_mulhi_epi16 (xmm3, coef3);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

            xmm4 = _mm_add_epi32 (xmm4, delta_4096);
			xmm4 = _mm_srai_epi32(xmm4, 13);
			xmm4 = _mm_packs_epi32(xmm4, nNull_xmm);

			_mm_storel_epi64((__m128i *)(ptr_out+4*k), xmm4);
		}

		NUMX = DX;
		DX = Str->num_ptn_x[sx_-1];

		xmm0 = _mm_loadu_si128((__m128i *)(ptr+4));

		xmm1 = xmm0;
		xmm0 = _mm_unpacklo_epi8(xmm0, nNull_xmm);
		xmm1 = _mm_unpackhi_epi8(xmm1, nNull_xmm);
		xmm2 = _mm_srli_si128(xmm2, 8);
		xmm2 = _mm_unpacklo_epi64(xmm2, xmm3);

		for (k = NUMX; k < DX; k++)
		{
			coef =  *(__m128i *)(Str->coefficient_x+32*k);
			coef1 = *(__m128i *)(Str->coefficient_x+32*k+8);
			coef2 = *(__m128i *)(Str->coefficient_x+32*k+16);
			coef3 = *(__m128i *)(Str->coefficient_x+32*k+24);

			//1-2 points
			xmm4 = _mm_mullo_epi16(xmm0, coef);
			xmm5 = _mm_mulhi_epi16(xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

			//3-4 points
			xmm5 = _mm_mullo_epi16 (xmm1, coef1);
			xmm6 = _mm_mulhi_epi16 (xmm1, coef1);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			//5-6 points
			xmm5 = _mm_mullo_epi16 (xmm2, coef2);
			xmm6 = _mm_mulhi_epi16 (xmm2, coef2);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			//7-8 points
			xmm5 = _mm_mullo_epi16 (xmm3, coef3);
			xmm6 = _mm_mulhi_epi16 (xmm3, coef3);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

            xmm4 = _mm_add_epi32 (xmm4, delta_4096);
			xmm4 = _mm_srai_epi32(xmm4, 13);
			xmm4 = _mm_packs_epi32(xmm4, nNull_xmm);

			_mm_storel_epi64((__m128i *)(ptr_out+4*k), xmm4);
		}

		NUMX = DX;
		DX = Str->num_ptn_x[sx_];

		xmm0 = _mm_loadu_si128((__m128i *)(ptr+8));

		xmm1 = xmm0;
		xmm0 = _mm_unpacklo_epi8(xmm0, nNull_xmm);
		xmm1 = _mm_unpackhi_epi8(xmm1, nNull_xmm);
		xmm2 = xmm3;

		for (k = NUMX; k < DX; k++)
		{
			coef =  *(__m128i *)(Str->coefficient_x+32*k);
			coef1 = *(__m128i *)(Str->coefficient_x+32*k+8);
			coef2 = *(__m128i *)(Str->coefficient_x+32*k+16);
			coef3 = *(__m128i *)(Str->coefficient_x+32*k+24);

			//1-2 points
			xmm4 = _mm_mullo_epi16(xmm0, coef);
			xmm5 = _mm_mulhi_epi16(xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

			//3-4 points
			xmm5 = _mm_mullo_epi16 (xmm1, coef1);
			xmm6 = _mm_mulhi_epi16 (xmm1, coef1);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			//5-6 points
			xmm5 = _mm_mullo_epi16 (xmm2, coef2);
			xmm6 = _mm_mulhi_epi16 (xmm2, coef2);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			//7-8 points
			xmm5 = _mm_mullo_epi16 (xmm3, coef3);
			xmm6 = _mm_mulhi_epi16 (xmm3, coef3);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

            xmm4 = _mm_add_epi32 (xmm4, delta_4096);
			xmm4 = _mm_srai_epi32(xmm4, 13);
			xmm4 = _mm_packs_epi32(xmm4, nNull_xmm);

			_mm_storel_epi64((__m128i *)(ptr_out+4*k), xmm4);
		}*/
	}

	//interpolation on Y direction
	DY = Str->num_ptn_y[0];
	dy = 0;
	unsigned long nLoop = (dst_w / 8) * 8;
	int delta = dst_w - nLoop;

/*	for (k = NUMY; k < DY; k++)
	{
        index = k * d_out_w;
		unsigned char * ptr_out = out_Img + index;
		signed short * ptr = temp_Img + dy;

		coef = *(__m128i *)(Str->coefficient_y+k*64);
		coef1 = *(__m128i *)(Str->coefficient_y+k*64+8);
		coef2 = *(__m128i *)(Str->coefficient_y+k*64+16);
		coef3 = *(__m128i *)(Str->coefficient_y+k*64+24);
		coef4 = *(__m128i *)(Str->coefficient_y+k*64+32);
		coef5 = *(__m128i *)(Str->coefficient_y+k*64+40);
		coef6 = *(__m128i *)(Str->coefficient_y+k*64+48);
		coef7 = *(__m128i *)(Str->coefficient_y+k*64+56);

		for (x = 0; x < nLoop; x += 2)
		{
			//1 row
			xmm0 = _mm_loadu_si128((__m128i *)(ptr));
			xmm5 = xmm0;

			temp = xmm0;
			xmm0 = _mm_mullo_epi16 (xmm0, coef);
			temp = _mm_mulhi_epi16 (temp, coef);
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi16(xmm0, temp);//first point
			xmm1 = _mm_unpackhi_epi16(xmm1, temp);//second point

			//2 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef1);
			temp = _mm_mulhi_epi16 (temp, coef1);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//3 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef2);
			temp = _mm_mulhi_epi16 (temp, coef2);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//4 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef3);
			temp = _mm_mulhi_epi16 (temp, coef3);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//5 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef4);
			temp = _mm_mulhi_epi16 (temp, coef4);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//6 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+2*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef5);
			temp = _mm_mulhi_epi16 (temp, coef5);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//7 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+3*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef6);
			temp = _mm_mulhi_epi16 (temp, coef6);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//8 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+4*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef7);
			temp = _mm_mulhi_epi16 (temp, coef7);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			xmm0 = _mm_add_epi32 (xmm0, delta_4096);
			xmm1 = _mm_add_epi32 (xmm1, delta_4096);
			xmm0 = _mm_srai_epi32(xmm0, 13);
			xmm1 = _mm_srai_epi32(xmm1, 13);
			xmm0 = _mm_packs_epi32(xmm0, nNull_xmm);
			xmm1 = _mm_packs_epi32(xmm1, nNull_xmm);
			xmm0 = _mm_packus_epi16(xmm0, nNull_xmm);
			xmm1 = _mm_packus_epi16(xmm1, nNull_xmm);

			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm0);
			*(int*)(ptr_out+4*x+4) = _mm_cvtsi128_si32(xmm1);

			ptr += 8;
		}
		if (delta)
		{
			mm0 = *(__m64 *)(ptr);
			mm1 = mm0;
			mm2 = mm0;
			mm3 = mm0;
			mm4 = *(__m64 *)(ptr+d_out_w);
			mm5 = *(__m64 *)(ptr+2*d_out_w);
			mm6 = *(__m64 *)(ptr+3*d_out_w);
			mm7 = *(__m64 *)(ptr+4*d_out_w);

			xmm0 = _mm_setr_epi64 (mm0, mm1);
			xmm1 = _mm_setr_epi64 (mm2, mm3);
			xmm2 = _mm_setr_epi64 (mm4, mm5);
			xmm3 = _mm_setr_epi64 (mm6, mm7);

			coef = _mm_unpacklo_epi64(coef, coef1);
			coef1 = _mm_unpacklo_epi64(coef2, coef3);
			coef2 = _mm_unpacklo_epi64(coef4, coef5);
			coef3 = _mm_unpacklo_epi64(coef6, coef7);

			xmm4 = _mm_mullo_epi16 (xmm0, coef);
			xmm5 = _mm_mulhi_epi16 (xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

			xmm5 = _mm_mullo_epi16 (xmm1, coef1);
			xmm6 = _mm_mulhi_epi16 (xmm1, coef1);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm2, coef2);
			xmm6 = _mm_mulhi_epi16 (xmm2, coef2);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm3, coef3);
			xmm6 = _mm_mulhi_epi16 (xmm3, coef3);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);
			xmm2 = xmm4;

			xmm2 = _mm_add_epi32 (xmm2, delta_4096);
			xmm2 = _mm_srai_epi32(xmm2, 13);
			xmm2 = _mm_packs_epi32(xmm2, nNull_xmm);
			xmm2 = _mm_packus_epi16(xmm2, nNull_xmm);
			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm2);
		}
	}

	NUMY = DY;
	DY = Str->num_ptn_y[1];

	for (k = NUMY; k < DY; k++)
	{
        index = k * d_out_w;
		unsigned char * ptr_out = out_Img + index;
		signed short * ptr = temp_Img + dy;

		coef = *(__m128i *)(Str->coefficient_y+k*64);
		coef1 = *(__m128i *)(Str->coefficient_y+k*64+8);
		coef2 = *(__m128i *)(Str->coefficient_y+k*64+16);
		coef3 = *(__m128i *)(Str->coefficient_y+k*64+24);
		coef4 = *(__m128i *)(Str->coefficient_y+k*64+32);
		coef5 = *(__m128i *)(Str->coefficient_y+k*64+40);
		coef6 = *(__m128i *)(Str->coefficient_y+k*64+48);
		coef7 = *(__m128i *)(Str->coefficient_y+k*64+56);

		for (x = 0; x < nLoop; x += 2)
		{
			//1 row
			xmm0 = _mm_loadu_si128((__m128i *)(ptr));
			xmm5 = xmm0;

			temp = xmm0;
			xmm0 = _mm_mullo_epi16 (xmm0, coef);
			temp = _mm_mulhi_epi16 (temp, coef);
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi16(xmm0, temp);//first point
			xmm1 = _mm_unpackhi_epi16(xmm1, temp);//second point

			//2 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef1);
			temp = _mm_mulhi_epi16 (temp, coef1);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//3 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef2);
			temp = _mm_mulhi_epi16 (temp, coef2);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//4 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef3);
			temp = _mm_mulhi_epi16 (temp, coef3);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//5 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+2*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef4);
			temp = _mm_mulhi_epi16 (temp, coef4);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//6 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+3*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef5);
			temp = _mm_mulhi_epi16 (temp, coef5);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//7 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+4*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef6);
			temp = _mm_mulhi_epi16 (temp, coef6);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//8 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+5*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef7);
			temp = _mm_mulhi_epi16 (temp, coef7);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			xmm0 = _mm_add_epi32 (xmm0, delta_4096);
			xmm1 = _mm_add_epi32 (xmm1, delta_4096);
			xmm0 = _mm_srai_epi32(xmm0, 13);
			xmm1 = _mm_srai_epi32(xmm1, 13);
			xmm0 = _mm_packs_epi32(xmm0, nNull_xmm);
			xmm1 = _mm_packs_epi32(xmm1, nNull_xmm);
			xmm0 = _mm_packus_epi16(xmm0, nNull_xmm);
			xmm1 = _mm_packus_epi16(xmm1, nNull_xmm);

			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm0);
			*(int*)(ptr_out+4*x+4) = _mm_cvtsi128_si32(xmm1);

			ptr += 8;
		}
		if (delta)
		{
			mm0 = *(__m64 *)(ptr);
			mm1 = mm0;
			mm2 = mm0;
			mm3 = *(__m64 *)(ptr+d_out_w);
			mm4 = *(__m64 *)(ptr+2*d_out_w);
			mm5 = *(__m64 *)(ptr+3*d_out_w);
			mm6 = *(__m64 *)(ptr+4*d_out_w);
			mm7 = *(__m64 *)(ptr+5*d_out_w);

			xmm0 = _mm_setr_epi64 (mm0, mm1);
			xmm1 = _mm_setr_epi64 (mm2, mm3);
			xmm2 = _mm_setr_epi64 (mm4, mm5);
			xmm3 = _mm_setr_epi64 (mm6, mm7);

			coef = _mm_unpacklo_epi64(coef, coef1);
			coef1 = _mm_unpacklo_epi64(coef2, coef3);
			coef2 = _mm_unpacklo_epi64(coef4, coef5);
			coef3 = _mm_unpacklo_epi64(coef6, coef7);

			xmm4 = _mm_mullo_epi16 (xmm0, coef);
			xmm5 = _mm_mulhi_epi16 (xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

			xmm5 = _mm_mullo_epi16 (xmm1, coef1);
			xmm6 = _mm_mulhi_epi16 (xmm1, coef1);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm2, coef2);
			xmm6 = _mm_mulhi_epi16 (xmm2, coef2);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm3, coef3);
			xmm6 = _mm_mulhi_epi16 (xmm3, coef3);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);
			xmm2 = xmm4;

			xmm2 = _mm_add_epi32 (xmm2, delta_4096);
			xmm2 = _mm_srai_epi32(xmm2, 13);
			xmm2 = _mm_packs_epi32(xmm2, nNull_xmm);
			xmm2 = _mm_packus_epi16(xmm2, nNull_xmm);
			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm2);
		}
	}

	NUMY = DY;
	DY = Str->num_ptn_y[2];

	for (k = NUMY; k < DY; k++)
	{
        index = k * d_out_w;
		unsigned char * ptr_out = out_Img + index;
		signed short * ptr = temp_Img + dy;

		coef = *(__m128i *)(Str->coefficient_y+k*64);
		coef1 = *(__m128i *)(Str->coefficient_y+k*64+8);
		coef2 = *(__m128i *)(Str->coefficient_y+k*64+16);
		coef3 = *(__m128i *)(Str->coefficient_y+k*64+24);
		coef4 = *(__m128i *)(Str->coefficient_y+k*64+32);
		coef5 = *(__m128i *)(Str->coefficient_y+k*64+40);
		coef6 = *(__m128i *)(Str->coefficient_y+k*64+48);
		coef7 = *(__m128i *)(Str->coefficient_y+k*64+56);

		for (x = 0; x < nLoop; x += 2)
		{
			//1 row
			xmm0 = _mm_loadu_si128((__m128i *)(ptr));
			xmm5 = xmm0;

			temp = xmm0;
			xmm0 = _mm_mullo_epi16 (xmm0, coef);
			temp = _mm_mulhi_epi16 (temp, coef);
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi16(xmm0, temp);//first point
			xmm1 = _mm_unpackhi_epi16(xmm1, temp);//second point

			//2 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef1);
			temp = _mm_mulhi_epi16 (temp, coef1);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//3 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef2);
			temp = _mm_mulhi_epi16 (temp, coef2);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//4 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+2*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef3);
			temp = _mm_mulhi_epi16 (temp, coef3);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//5 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+3*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef4);
			temp = _mm_mulhi_epi16 (temp, coef4);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//6 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+4*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef5);
			temp = _mm_mulhi_epi16 (temp, coef5);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//7 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+5*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef6);
			temp = _mm_mulhi_epi16 (temp, coef6);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//8 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+6*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef7);
			temp = _mm_mulhi_epi16 (temp, coef7);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			xmm0 = _mm_add_epi32 (xmm0, delta_4096);
			xmm1 = _mm_add_epi32 (xmm1, delta_4096);
			xmm0 = _mm_srai_epi32(xmm0, 13);
			xmm1 = _mm_srai_epi32(xmm1, 13);
			xmm0 = _mm_packs_epi32(xmm0, nNull_xmm);
			xmm1 = _mm_packs_epi32(xmm1, nNull_xmm);
			xmm0 = _mm_packus_epi16(xmm0, nNull_xmm);
			xmm1 = _mm_packus_epi16(xmm1, nNull_xmm);

			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm0);
			*(int*)(ptr_out+4*x+4) = _mm_cvtsi128_si32(xmm1);

			ptr += 8;
		}
		if (delta)
		{
			mm0 = *(__m64 *)(ptr);
			mm1 = mm0;
			mm2 = *(__m64 *)(ptr+d_out_w);
			mm3 = *(__m64 *)(ptr+2*d_out_w);
			mm4 = *(__m64 *)(ptr+3*d_out_w);
			mm5 = *(__m64 *)(ptr+4*d_out_w);
			mm6 = *(__m64 *)(ptr+5*d_out_w);
			mm7 = *(__m64 *)(ptr+6*d_out_w);

			xmm0 = _mm_setr_epi64 (mm0, mm1);
			xmm1 = _mm_setr_epi64 (mm2, mm3);
			xmm2 = _mm_setr_epi64 (mm4, mm5);
			xmm3 = _mm_setr_epi64 (mm6, mm7);

			coef = _mm_unpacklo_epi64(coef, coef1);
			coef1 = _mm_unpacklo_epi64(coef2, coef3);
			coef2 = _mm_unpacklo_epi64(coef4, coef5);
			coef3 = _mm_unpacklo_epi64(coef6, coef7);

			xmm4 = _mm_mullo_epi16 (xmm0, coef);
			xmm5 = _mm_mulhi_epi16 (xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

			xmm5 = _mm_mullo_epi16 (xmm1, coef1);
			xmm6 = _mm_mulhi_epi16 (xmm1, coef1);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm2, coef2);
			xmm6 = _mm_mulhi_epi16 (xmm2, coef2);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm3, coef3);
			xmm6 = _mm_mulhi_epi16 (xmm3, coef3);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);
			xmm2 = xmm4;

			xmm2 = _mm_add_epi32 (xmm2, delta_4096);
			xmm2 = _mm_srai_epi32(xmm2, 13);
			xmm2 = _mm_packs_epi32(xmm2, nNull_xmm);
			xmm2 = _mm_packus_epi16(xmm2, nNull_xmm);
			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm2);
		}
	}*/

	NUMY = Str->num_ptn_y[2];//DY;

	for (y = 3; y <= sy_ - 3; y++)
	{
		DY = Str->num_ptn_y[y];
		dy = (y - 3) * dst_w;

		for (k = NUMY; k < DY; k++)
		{
            index = k * dst_w;
			unsigned char * ptr_out = dst + index;
			signed short * ptr = temp_Img + dy;

			coef = *(__m128i *)(Str->coefficient_y+k*64);
			coef1 = *(__m128i *)(Str->coefficient_y+k*64+8);
			coef2 = *(__m128i *)(Str->coefficient_y+k*64+16);
			coef3 = *(__m128i *)(Str->coefficient_y+k*64+24);
			coef4 = *(__m128i *)(Str->coefficient_y+k*64+32);
			coef5 = *(__m128i *)(Str->coefficient_y+k*64+40);
			coef6 = *(__m128i *)(Str->coefficient_y+k*64+48);
			coef7 = *(__m128i *)(Str->coefficient_y+k*64+56);

			for (x = 0; x < (nLoop + 8); x += 8)
			{
				//1 row
				xmm0 = _mm_loadu_si128((__m128i *)(ptr));

				temp = xmm0;
				xmm0 = _mm_mullo_epi16 (xmm0, coef);
				temp = _mm_mulhi_epi16 (temp, coef);
				xmm1 = xmm0;
				xmm0 = _mm_unpacklo_epi16(xmm0, temp);//first point
				xmm1 = _mm_unpackhi_epi16(xmm1, temp);//second point

				//2 row
				xmm2 = _mm_loadu_si128((__m128i *)(ptr+dst_w));

				temp = xmm2;
				xmm2 = _mm_mullo_epi16 (xmm2, coef1);
				temp = _mm_mulhi_epi16 (temp, coef1);
				xmm3 = xmm2;
				xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
				xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

				xmm0 = _mm_add_epi32(xmm0, xmm2);
				xmm1 = _mm_add_epi32(xmm1, xmm3);

				//3 row
				xmm2 = _mm_loadu_si128((__m128i *)(ptr+2*dst_w));

				temp = xmm2;
				xmm2 = _mm_mullo_epi16 (xmm2, coef2);
				temp = _mm_mulhi_epi16 (temp, coef2);
				xmm3 = xmm2;
				xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
				xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

				xmm0 = _mm_add_epi32(xmm0, xmm2);
				xmm1 = _mm_add_epi32(xmm1, xmm3);

				//4 row
				xmm2 = _mm_loadu_si128((__m128i *)(ptr+3*dst_w));

				temp = xmm2;
				xmm2 = _mm_mullo_epi16 (xmm2, coef3);
				temp = _mm_mulhi_epi16 (temp, coef3);
				xmm3 = xmm2;
				xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
				xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

				xmm0 = _mm_add_epi32(xmm0, xmm2);
				xmm1 = _mm_add_epi32(xmm1, xmm3);

				//5 row
				xmm2 = _mm_loadu_si128((__m128i *)(ptr+4*dst_w));

				temp = xmm2;
				xmm2 = _mm_mullo_epi16 (xmm2, coef4);
				temp = _mm_mulhi_epi16 (temp, coef4);
				xmm3 = xmm2;
				xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
				xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

				xmm0 = _mm_add_epi32(xmm0, xmm2);
				xmm1 = _mm_add_epi32(xmm1, xmm3);

				//6 row
				xmm2 = _mm_loadu_si128((__m128i *)(ptr+5*dst_w));

				temp = xmm2;
				xmm2 = _mm_mullo_epi16 (xmm2, coef5);
				temp = _mm_mulhi_epi16 (temp, coef5);
				xmm3 = xmm2;
				xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
				xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

				xmm0 = _mm_add_epi32(xmm0, xmm2);
				xmm1 = _mm_add_epi32(xmm1, xmm3);

				//7 row
				xmm2 = _mm_loadu_si128((__m128i *)(ptr+6*dst_w));

				temp = xmm2;
				xmm2 = _mm_mullo_epi16 (xmm2, coef6);
				temp = _mm_mulhi_epi16 (temp, coef6);
				xmm3 = xmm2;
				xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
				xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

				xmm0 = _mm_add_epi32(xmm0, xmm2);
				xmm1 = _mm_add_epi32(xmm1, xmm3);

				//8 row
				xmm2 = _mm_loadu_si128((__m128i *)(ptr+7*dst_w));

				temp = xmm2;
				xmm2 = _mm_mullo_epi16 (xmm2, coef7);
				temp = _mm_mulhi_epi16 (temp, coef7);
				xmm3 = xmm2;
				xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
				xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

				xmm0 = _mm_add_epi32(xmm0, xmm2);
				xmm1 = _mm_add_epi32(xmm1, xmm3);

				xmm0 = _mm_add_epi32 (xmm0, delta_4096);
				xmm1 = _mm_add_epi32 (xmm1, delta_4096);
				xmm0 = _mm_srai_epi32(xmm0, 13);
				xmm1 = _mm_srai_epi32(xmm1, 13);
				xmm0 = _mm_packs_epi32(xmm0, nNull_xmm);
				xmm1 = _mm_packs_epi32(xmm1, nNull_xmm);
				xmm0 = _mm_packus_epi16(xmm0, nNull_xmm);
				xmm1 = _mm_packus_epi16(xmm1, nNull_xmm);

				*(int*)(ptr_out+x) = _mm_cvtsi128_si32(xmm0);
				*(int*)(ptr_out+x+4) = _mm_cvtsi128_si32(xmm1);

				ptr += 8;
			}
/*			if (delta)
			{
				mm0 = *(__m64 *)(ptr);
				mm1 = *(__m64 *)(ptr+dst_w);
				mm2 = *(__m64 *)(ptr+2*dst_w);
				mm3 = *(__m64 *)(ptr+3*dst_w);
				mm4 = *(__m64 *)(ptr+4*dst_w);
				mm5 = *(__m64 *)(ptr+5*dst_w);
				mm6 = *(__m64 *)(ptr+6*dst_w);
				mm7 = *(__m64 *)(ptr+7*dst_w);

				xmm0 = _mm_setr_epi64 (mm0, mm1);
				xmm1 = _mm_setr_epi64 (mm2, mm3);
				xmm2 = _mm_setr_epi64 (mm4, mm5);
				xmm3 = _mm_setr_epi64 (mm6, mm7);

				coef = _mm_unpacklo_epi64(coef, coef1);
				coef1 = _mm_unpacklo_epi64(coef2, coef3);
				coef2 = _mm_unpacklo_epi64(coef4, coef5);
				coef3 = _mm_unpacklo_epi64(coef6, coef7);

				xmm4 = _mm_mullo_epi16 (xmm0, coef);
				xmm5 = _mm_mulhi_epi16 (xmm0, coef);

				temp = xmm4;
				xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
				temp = _mm_unpackhi_epi16(temp, xmm5);
				xmm4 = _mm_add_epi32(xmm4, temp);

				xmm5 = _mm_mullo_epi16 (xmm1, coef1);
				xmm6 = _mm_mulhi_epi16 (xmm1, coef1);

				temp = xmm5;
				xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
				temp = _mm_unpackhi_epi16(temp, xmm6);
				xmm5 = _mm_add_epi32(xmm5, temp);

				xmm4 = _mm_add_epi32(xmm4, xmm5);

				xmm5 = _mm_mullo_epi16 (xmm2, coef2);
				xmm6 = _mm_mulhi_epi16 (xmm2, coef2);

				temp = xmm5;
				xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
				temp = _mm_unpackhi_epi16(temp, xmm6);
				xmm5 = _mm_add_epi32(xmm5, temp);

				xmm4 = _mm_add_epi32(xmm4, xmm5);

				xmm5 = _mm_mullo_epi16 (xmm3, coef3);
				xmm6 = _mm_mulhi_epi16 (xmm3, coef3);

				temp = xmm5;
				xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
				temp = _mm_unpackhi_epi16(temp, xmm6);
				xmm5 = _mm_add_epi32(xmm5, temp);

				xmm4 = _mm_add_epi32(xmm4, xmm5);
				xmm2 = xmm4;

				xmm2 = _mm_add_epi32 (xmm2, delta_4096);
				xmm2 = _mm_srai_epi32(xmm2, 13);
				xmm2 = _mm_packs_epi32(xmm2, nNull_xmm);
				xmm2 = _mm_packus_epi16(xmm2, nNull_xmm);
				*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm2);
			}*/
		}
		NUMY = DY;
	}

/*	dy = (sy_ - 5) * d_out_w;
	DY = Str->num_ptn_y[sy_-2];

	for (k = NUMY; k < DY; k++)
	{
        index = k * d_out_w;
		unsigned char * ptr_out = out_Img + index;
		signed short * ptr = temp_Img + dy;

		coef = *(__m128i *)(Str->coefficient_y+k*64);
		coef1 = *(__m128i *)(Str->coefficient_y+k*64+8);
		coef2 = *(__m128i *)(Str->coefficient_y+k*64+16);
		coef3 = *(__m128i *)(Str->coefficient_y+k*64+24);
		coef4 = *(__m128i *)(Str->coefficient_y+k*64+32);
		coef5 = *(__m128i *)(Str->coefficient_y+k*64+40);
		coef6 = *(__m128i *)(Str->coefficient_y+k*64+48);
		coef7 = *(__m128i *)(Str->coefficient_y+k*64+56);

		for (x = 0; x < nLoop; x += 2)
		{
			//1 row
			xmm0 = _mm_loadu_si128((__m128i *)(ptr));

			temp = xmm0;
			xmm0 = _mm_mullo_epi16 (xmm0, coef);
			temp = _mm_mulhi_epi16 (temp, coef);
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi16(xmm0, temp);//first point
			xmm1 = _mm_unpackhi_epi16(xmm1, temp);//second point

			//2 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef1);
			temp = _mm_mulhi_epi16 (temp, coef1);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//3 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+2*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef2);
			temp = _mm_mulhi_epi16 (temp, coef2);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//4 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+3*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef3);
			temp = _mm_mulhi_epi16 (temp, coef3);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//5 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+4*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef4);
			temp = _mm_mulhi_epi16 (temp, coef4);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//6 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+5*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef5);
			temp = _mm_mulhi_epi16 (temp, coef5);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//7 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+6*d_out_w));
			xmm5 = xmm2;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef6);
			temp = _mm_mulhi_epi16 (temp, coef6);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//8 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef7);
			temp = _mm_mulhi_epi16 (temp, coef7);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			xmm0 = _mm_add_epi32 (xmm0, delta_4096);
			xmm1 = _mm_add_epi32 (xmm1, delta_4096);
			xmm0 = _mm_srai_epi32(xmm0, 13);
			xmm1 = _mm_srai_epi32(xmm1, 13);
			xmm0 = _mm_packs_epi32(xmm0, nNull_xmm);
			xmm1 = _mm_packs_epi32(xmm1, nNull_xmm);
			xmm0 = _mm_packus_epi16(xmm0, nNull_xmm);
			xmm1 = _mm_packus_epi16(xmm1, nNull_xmm);

			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm0);
			*(int*)(ptr_out+4*x+4) = _mm_cvtsi128_si32(xmm1);

			ptr += 8;
		}
		if (delta)
		{
			mm0 = *(__m64 *)(ptr);
			mm1 = *(__m64 *)(ptr+d_out_w);
			mm2 = *(__m64 *)(ptr+2*d_out_w);
			mm3 = *(__m64 *)(ptr+3*d_out_w);
			mm4 = *(__m64 *)(ptr+4*d_out_w);
			mm5 = *(__m64 *)(ptr+5*d_out_w);
			mm6 = *(__m64 *)(ptr+6*d_out_w);
			mm7 = mm6;

			xmm0 = _mm_setr_epi64 (mm0, mm1);
			xmm1 = _mm_setr_epi64 (mm2, mm3);
			xmm2 = _mm_setr_epi64 (mm4, mm5);
			xmm3 = _mm_setr_epi64 (mm6, mm7);

			coef = _mm_unpacklo_epi64(coef, coef1);
			coef1 = _mm_unpacklo_epi64(coef2, coef3);
			coef2 = _mm_unpacklo_epi64(coef4, coef5);
			coef3 = _mm_unpacklo_epi64(coef6, coef7);

			xmm4 = _mm_mullo_epi16 (xmm0, coef);
			xmm5 = _mm_mulhi_epi16 (xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

			xmm5 = _mm_mullo_epi16 (xmm1, coef1);
			xmm6 = _mm_mulhi_epi16 (xmm1, coef1);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm2, coef2);
			xmm6 = _mm_mulhi_epi16 (xmm2, coef2);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm3, coef3);
			xmm6 = _mm_mulhi_epi16 (xmm3, coef3);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);
			xmm2 = xmm4;

			xmm2 = _mm_add_epi32 (xmm2, delta_4096);
			xmm2 = _mm_srai_epi32(xmm2, 13);
			xmm2 = _mm_packs_epi32(xmm2, nNull_xmm);
			xmm2 = _mm_packus_epi16(xmm2, nNull_xmm);
			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm2);
		}
	}

	NUMY = DY;
	DY = DY = Str->num_ptn_y[sy_-1];

	for (k = NUMY; k < DY; k++)
	{
        index = k * d_out_w;
		unsigned char * ptr_out = out_Img + index;
		signed short * ptr = temp_Img + dy;

		coef = *(__m128i *)(Str->coefficient_y+k*64);
		coef1 = *(__m128i *)(Str->coefficient_y+k*64+8);
		coef2 = *(__m128i *)(Str->coefficient_y+k*64+16);
		coef3 = *(__m128i *)(Str->coefficient_y+k*64+24);
		coef4 = *(__m128i *)(Str->coefficient_y+k*64+32);
		coef5 = *(__m128i *)(Str->coefficient_y+k*64+40);
		coef6 = *(__m128i *)(Str->coefficient_y+k*64+48);
		coef7 = *(__m128i *)(Str->coefficient_y+k*64+56);

		for (x = 0; x < nLoop; x += 2)
		{
			//1 row
			xmm0 = _mm_loadu_si128((__m128i *)(ptr+d_out_w));

			temp = xmm0;
			xmm0 = _mm_mullo_epi16 (xmm0, coef);
			temp = _mm_mulhi_epi16 (temp, coef);
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi16(xmm0, temp);//first point
			xmm1 = _mm_unpackhi_epi16(xmm1, temp);//second point

			//2 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+2*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef1);
			temp = _mm_mulhi_epi16 (temp, coef1);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//3 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+3*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef2);
			temp = _mm_mulhi_epi16 (temp, coef2);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//4 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+4*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef3);
			temp = _mm_mulhi_epi16 (temp, coef3);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//5 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+5*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef4);
			temp = _mm_mulhi_epi16 (temp, coef4);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//6 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+6*d_out_w));
			xmm5 = xmm2;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef5);
			temp = _mm_mulhi_epi16 (temp, coef5);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//7 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef6);
			temp = _mm_mulhi_epi16 (temp, coef6);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//8 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef7);
			temp = _mm_mulhi_epi16 (temp, coef7);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			xmm0 = _mm_add_epi32 (xmm0, delta_4096);
			xmm1 = _mm_add_epi32 (xmm1, delta_4096);
			xmm0 = _mm_srai_epi32(xmm0, 13);
			xmm1 = _mm_srai_epi32(xmm1, 13);
			xmm0 = _mm_packs_epi32(xmm0, nNull_xmm);
			xmm1 = _mm_packs_epi32(xmm1, nNull_xmm);
			xmm0 = _mm_packus_epi16(xmm0, nNull_xmm);
			xmm1 = _mm_packus_epi16(xmm1, nNull_xmm);

			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm0);
			*(int*)(ptr_out+4*x+4) = _mm_cvtsi128_si32(xmm1);

			ptr += 8;
		}
		if (delta)
		{
			mm0 = *(__m64 *)(ptr+d_out_w);
			mm1 = *(__m64 *)(ptr+2*d_out_w);
			mm2 = *(__m64 *)(ptr+3*d_out_w);
			mm3 = *(__m64 *)(ptr+4*d_out_w);
			mm4 = *(__m64 *)(ptr+5*d_out_w);
			mm5 = *(__m64 *)(ptr+6*d_out_w);
			mm6 = mm5;
			mm7 = mm5;

			xmm0 = _mm_setr_epi64 (mm0, mm1);
			xmm1 = _mm_setr_epi64 (mm2, mm3);
			xmm2 = _mm_setr_epi64 (mm4, mm5);
			xmm3 = _mm_setr_epi64 (mm6, mm7);

			coef = _mm_unpacklo_epi64(coef, coef1);
			coef1 = _mm_unpacklo_epi64(coef2, coef3);
			coef2 = _mm_unpacklo_epi64(coef4, coef5);
			coef3 = _mm_unpacklo_epi64(coef6, coef7);

			xmm4 = _mm_mullo_epi16 (xmm0, coef);
			xmm5 = _mm_mulhi_epi16 (xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

			xmm5 = _mm_mullo_epi16 (xmm1, coef1);
			xmm6 = _mm_mulhi_epi16 (xmm1, coef1);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm2, coef2);
			xmm6 = _mm_mulhi_epi16 (xmm2, coef2);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm3, coef3);
			xmm6 = _mm_mulhi_epi16 (xmm3, coef3);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);
			xmm2 = xmm4;

			xmm2 = _mm_add_epi32 (xmm2, delta_4096);
			xmm2 = _mm_srai_epi32(xmm2, 13);
			xmm2 = _mm_packs_epi32(xmm2, nNull_xmm);
			xmm2 = _mm_packus_epi16(xmm2, nNull_xmm);
			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm2);
		}
	}

	NUMY = DY;
	DY = Str->num_ptn_y[sy_];

	for (k = NUMY; k < DY; k++)
	{
        index = k * d_out_w;
		unsigned char * ptr_out = out_Img + index;
		signed short * ptr = temp_Img + dy;

		coef = *(__m128i *)(Str->coefficient_y+k*64);
		coef1 = *(__m128i *)(Str->coefficient_y+k*64+8);
		coef2 = *(__m128i *)(Str->coefficient_y+k*64+16);
		coef3 = *(__m128i *)(Str->coefficient_y+k*64+24);
		coef4 = *(__m128i *)(Str->coefficient_y+k*64+32);
		coef5 = *(__m128i *)(Str->coefficient_y+k*64+40);
		coef6 = *(__m128i *)(Str->coefficient_y+k*64+48);
		coef7 = *(__m128i *)(Str->coefficient_y+k*64+56);

		for (x = 0; x < nLoop; x += 2)
		{
			//1 row
			xmm0 = _mm_loadu_si128((__m128i *)(ptr+2*d_out_w));

			temp = xmm0;
			xmm0 = _mm_mullo_epi16 (xmm0, coef);
			temp = _mm_mulhi_epi16 (temp, coef);
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi16(xmm0, temp);//first point
			xmm1 = _mm_unpackhi_epi16(xmm1, temp);//second point

			//2 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+3*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef1);
			temp = _mm_mulhi_epi16 (temp, coef1);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//3 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+4*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef2);
			temp = _mm_mulhi_epi16 (temp, coef2);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//4 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+5*d_out_w));

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef3);
			temp = _mm_mulhi_epi16 (temp, coef3);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//5 row
			xmm2 = _mm_loadu_si128((__m128i *)(ptr+6*d_out_w));
			xmm5 = xmm2;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef4);
			temp = _mm_mulhi_epi16 (temp, coef4);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//6 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef5);
			temp = _mm_mulhi_epi16 (temp, coef5);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//7 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef6);
			temp = _mm_mulhi_epi16 (temp, coef6);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			//8 row
			xmm2 = xmm5;

			temp = xmm2;
			xmm2 = _mm_mullo_epi16 (xmm2, coef7);
			temp = _mm_mulhi_epi16 (temp, coef7);
			xmm3 = xmm2;
			xmm2 = _mm_unpacklo_epi16(xmm2, temp);//first point
			xmm3 = _mm_unpackhi_epi16(xmm3, temp);//second point

			xmm0 = _mm_add_epi32(xmm0, xmm2);
			xmm1 = _mm_add_epi32(xmm1, xmm3);

			xmm0 = _mm_add_epi32 (xmm0, delta_4096);
			xmm1 = _mm_add_epi32 (xmm1, delta_4096);
			xmm0 = _mm_srai_epi32(xmm0, 13);
			xmm1 = _mm_srai_epi32(xmm1, 13);
			xmm0 = _mm_packs_epi32(xmm0, nNull_xmm);
			xmm1 = _mm_packs_epi32(xmm1, nNull_xmm);
			xmm0 = _mm_packus_epi16(xmm0, nNull_xmm);
			xmm1 = _mm_packus_epi16(xmm1, nNull_xmm);

			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm0);
			*(int*)(ptr_out+4*x+4) = _mm_cvtsi128_si32(xmm1);

			ptr += 8;
		}
		if (delta)
		{
			mm0 = *(__m64 *)(ptr+2*d_out_w);
			mm1 = *(__m64 *)(ptr+3*d_out_w);
			mm2 = *(__m64 *)(ptr+4*d_out_w);
			mm3 = *(__m64 *)(ptr+5*d_out_w);
			mm4 = *(__m64 *)(ptr+6*d_out_w);
			mm5 = mm4;
			mm6 = mm4;
			mm7 = mm4;

			xmm0 = _mm_setr_epi64 (mm0, mm1);
			xmm1 = _mm_setr_epi64 (mm2, mm3);
			xmm2 = _mm_setr_epi64 (mm4, mm5);
			xmm3 = _mm_setr_epi64 (mm6, mm7);

			coef = _mm_unpacklo_epi64(coef, coef1);
			coef1 = _mm_unpacklo_epi64(coef2, coef3);
			coef2 = _mm_unpacklo_epi64(coef4, coef5);
			coef3 = _mm_unpacklo_epi64(coef6, coef7);

			xmm4 = _mm_mullo_epi16 (xmm0, coef);
			xmm5 = _mm_mulhi_epi16 (xmm0, coef);

			temp = xmm4;
			xmm4 = _mm_unpacklo_epi16(xmm4, xmm5);
			temp = _mm_unpackhi_epi16(temp, xmm5);
			xmm4 = _mm_add_epi32(xmm4, temp);

			xmm5 = _mm_mullo_epi16 (xmm1, coef1);
			xmm6 = _mm_mulhi_epi16 (xmm1, coef1);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm2, coef2);
			xmm6 = _mm_mulhi_epi16 (xmm2, coef2);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);

			xmm5 = _mm_mullo_epi16 (xmm3, coef3);
			xmm6 = _mm_mulhi_epi16 (xmm3, coef3);

			temp = xmm5;
			xmm5 = _mm_unpacklo_epi16(xmm5, xmm6);
			temp = _mm_unpackhi_epi16(temp, xmm6);
			xmm5 = _mm_add_epi32(xmm5, temp);

			xmm4 = _mm_add_epi32(xmm4, xmm5);
			xmm2 = xmm4;

			xmm2 = _mm_add_epi32 (xmm2, delta_4096);
			xmm2 = _mm_srai_epi32(xmm2, 13);
			xmm2 = _mm_packs_epi32(xmm2, nNull_xmm);
			xmm2 = _mm_packus_epi16(xmm2, nNull_xmm);
			*(int*)(ptr_out+4*x) = _mm_cvtsi128_si32(xmm2);
		}
	}*/
	_mm_empty ();
	VS_free(temp_Img);
}

/**
 **************************************************************************************************
 * Finding minimum and maximum intensity value [SSE2 variant]
 * \param  [in]	      grayImg       - grayscale image
 * \param  [out]	  minIntensity  - array of minimum intansity
 * \param  [out]	  maxIntensity  - array of maximum intansity
 * \param  [in]       sx            - number of pixels in source image on X direction
 * \param  [in]       sy            - number of pixels in source image on Y direction
 **************************************************************************************************
 */
void getMAXMINIntensity_SSE2(unsigned char* grayImg, unsigned char* minIntensity, unsigned char* maxIntensity, int sx, int sy)
{
	int s = 0, t = 0, j = 0, k = 0, index = 0, index_ = 0, index__ = 0;
	unsigned char min_intens = 255, max_intens = 0, intens = 0;

	int nLoop = ((sx - 3) / 15) * 15;
	int delta = sx - nLoop;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6;

	for (j = 0; j < sy - 3; j++) {
		unsigned char *ptr_gray = grayImg + j * sx;
		unsigned char *ptr_min = minIntensity + (j + 2) * (sx + 1),
					  *ptr_max = maxIntensity + (j + 2) * (sx + 1);
		for (k = 0; k < nLoop; k += 15) {
			xmm0 = _mm_loadu_si128((__m128i *)(ptr_gray+sx));
			xmm1 = _mm_loadu_si128((__m128i *)(ptr_gray+2*sx));

			xmm5 = xmm0;
			xmm5 = _mm_min_epu8(xmm5, xmm1);
			xmm6 = xmm0;
			xmm6 = _mm_max_epu8(xmm6, xmm1);

			xmm0 = _mm_loadu_si128((__m128i *)(ptr_gray+sx+3));
			xmm1 = _mm_loadu_si128((__m128i *)(ptr_gray+2*sx+3));

			xmm5 = _mm_min_epu8(xmm5, xmm0);
			xmm5 = _mm_min_epu8(xmm5, xmm1);
			xmm6 = _mm_max_epu8(xmm6, xmm0);
			xmm6 = _mm_max_epu8(xmm6, xmm1);

			xmm0 = _mm_loadu_si128((__m128i *)(ptr_gray+1));
			xmm1 = _mm_loadu_si128((__m128i *)(ptr_gray+sx+1));
			xmm2 = _mm_loadu_si128((__m128i *)(ptr_gray+2*sx+1));
			xmm3 = _mm_loadu_si128((__m128i *)(ptr_gray+3*sx+1));

			xmm4 = xmm0;
			xmm4 = _mm_min_epu8(xmm4, xmm1);
			xmm4 = _mm_min_epu8(xmm4, xmm2);
			xmm4 = _mm_min_epu8(xmm4, xmm3);
			xmm0 = _mm_max_epu8(xmm0, xmm1);
			xmm0 = _mm_max_epu8(xmm0, xmm2);
			xmm0 = _mm_max_epu8(xmm0, xmm3);
			xmm1 = xmm6;
			xmm2 = xmm4;
			xmm3 = xmm5;

			// xmm0 & xmm1 - max; xmm2 & xmm3 - min

			xmm4 = xmm0;
			xmm4 = _mm_srli_si128(xmm4, 1);
			xmm5 = xmm2;
			xmm5 = _mm_srli_si128(xmm5, 1);

			xmm0 = _mm_max_epu8(xmm0, xmm4);
			xmm0 = _mm_max_epu8(xmm0, xmm1);
			xmm2 = _mm_min_epu8(xmm2, xmm5);
			xmm2 = _mm_min_epu8(xmm2, xmm3);

			_mm_storeu_si128((__m128i *)(ptr_max+2), xmm0);
			_mm_storeu_si128((__m128i *)(ptr_min+2), xmm2);

			ptr_gray += 15;
			ptr_min += 15;
			ptr_max += 15;
		}
	}
	_mm_empty();
	if (delta) {
		for (j = 0; j < sy - 3; j++)
		{
			index = (j + 2) * (sx + 1);
			for (k = nLoop; k < sx - 3; k++)
			{
				min_intens = 255;
				max_intens = 0;
				index_ = j * sx + k;

				for (s = 1; s < 3; s++)
				{
					intens = grayImg[index_+s*sx];
					if (intens <= min_intens) min_intens = intens;
					if (intens >= max_intens) max_intens = intens;
				}

				for (s = 0; s < 4; s++)
				{
					for (t = 1; t < 3; t++)
					{
						intens = grayImg[index_+s*sx+t];
						if (intens <= min_intens) min_intens = intens;
						if (intens >= max_intens) max_intens = intens;
					}
				}

				for (s = 1; s < 3; s++)
				{
					intens = grayImg[index_+s*sx+3];
					if (intens <= min_intens) min_intens = intens;
					if (intens >= max_intens) max_intens = intens;
				}

				minIntensity[index+k+2] = min_intens;
				maxIntensity[index+k+2] = max_intens;
			}
		}
	}

	for (k = 0; k < sx - 3; k++)
	{
		min_intens = 255;
		max_intens = 0;
		index_ = k;

		for (s = 0; s < 2; s++)
		{
			intens = grayImg[index_+s*sx];
			if (intens <= min_intens) min_intens = intens;
			if (intens >= max_intens) max_intens = intens;
		}

		for (s = 0; s < 3; s++)
		{
			for (t = 1; t < 3; t++)
			{
				intens = grayImg[index_+s*sx+t];
				if (intens <= min_intens) min_intens = intens;
				if (intens >= max_intens) max_intens = intens;
			}
		}

		for (s = 0; s < 2; s++)
		{
			intens = grayImg[index_+s*sx+3];
			if (intens <= min_intens) min_intens = intens;
			if (intens >= max_intens) max_intens = intens;
		}

		minIntensity[sx+1+k+2] = min_intens;
		maxIntensity[sx+1+k+2] = max_intens;
		minIntensity[k+2] = min_intens;
		maxIntensity[k+2] = max_intens;
	}

	index = (sy - 1) * (sx + 1);
	for (k = 0; k < sx - 3; k++)
	{
		min_intens = 255;
		max_intens = 0;
		index_ = (sy - 3) * sx + k;

		for (s = 1; s < 3; s++)
		{
			intens = grayImg[index_+s*sx];
			if (intens <= min_intens) min_intens = intens;
			if (intens >= max_intens) max_intens = intens;
		}

		for (s = 0; s < 3; s++)
		{
			for (t = 1; t < 3; t++)
			{
				intens = grayImg[index_+s*sx+t];
				if (intens <= min_intens) min_intens = intens;
				if (intens >= max_intens) max_intens = intens;
			}
		}

		for (s = 1; s < 3; s++)
		{
			intens = grayImg[index_+s*sx+3];
			if (intens <= min_intens) min_intens = intens;
			if (intens >= max_intens) max_intens = intens;
		}

		minIntensity[index+k+2] = min_intens;
		maxIntensity[index+k+2] = max_intens;
		minIntensity[sx+1+index+k+2] = min_intens;
		maxIntensity[sx+1+index+k+2] = max_intens;
	}

	for (j = 0; j < sy - 3; j++)
	{
		index = (j + 2) * (sx + 1);

		min_intens = 255;
		max_intens = 0;
		index_ = j * sx;

		for (s = 0; s < 4; s++)
		{
			for (t = 1; t < 3; t++)
			{
				intens = grayImg[index_+s*sx+t];
				if (intens <= min_intens) min_intens = intens;
				if (intens >= max_intens) max_intens = intens;
			}
		}

		for (s = 1; s < 3; s++)
		{
			intens = grayImg[index_+s*sx+3];
			if (intens <= min_intens) min_intens = intens;
			if (intens >= max_intens) max_intens = intens;
		}

		minIntensity[index] = min_intens;
		maxIntensity[index] = max_intens;
		minIntensity[index+1] = min_intens;
		maxIntensity[index+1] = max_intens;
	}

	for (j = 0; j < sy - 3; j++)
	{
		index = (j + 2) * (sx + 1);

		min_intens = 255;
		max_intens = 0;
		index_ = (j + 1) * sx - 3;

		for (s = 1; s < 3; s++)
		{
			intens = grayImg[index_+s*sx];
			if (intens <= min_intens) min_intens = intens;
			if (intens >= max_intens) max_intens = intens;
		}

		for (s = 0; s < 4; s++)
		{
			for (t = 1; t < 3; t++)
			{
				intens = grayImg[index_+s*sx+t];
				if (intens <= min_intens) min_intens = intens;
				if (intens >= max_intens) max_intens = intens;
			}
		}

		minIntensity[index+sx-1] = min_intens;
		maxIntensity[index+sx-1] = max_intens;
		minIntensity[index+sx] = min_intens;
		maxIntensity[index+sx] = max_intens;
	}

	min_intens = 255;
	max_intens = 0;
	for (s = 0; s < 2; s++)
	{
		for (t = 0; t < 3; t++)
		{
			intens = grayImg[s*sx+t];
			if (intens <= min_intens) min_intens = intens;
			if (intens >= max_intens) max_intens = intens;
		}
	}
	intens = grayImg[2*sx];
	if (intens <= min_intens) min_intens = intens;
	if (intens >= max_intens) max_intens = intens;
	intens = grayImg[2*sx+1];
	if (intens <= min_intens) min_intens = intens;
	if (intens >= max_intens) max_intens = intens;
	minIntensity[sx+2] = min_intens;
	maxIntensity[sx+2] = max_intens;
	minIntensity[sx+1] = min_intens;
	maxIntensity[sx+1] = max_intens;
	minIntensity[0] = min_intens;
	maxIntensity[0] = max_intens;
	minIntensity[1] = min_intens;
	maxIntensity[1] = max_intens;

	min_intens = 255;
	max_intens = 0;
	for (s = 0; s < 2; s++)
	{
		for (t = 0; t < 3; t++)
		{
			intens = grayImg[(s+1)*sx+t-3];
			if (intens <= min_intens) min_intens = intens;
			if (intens >= max_intens) max_intens = intens;
		}
	}
	intens = grayImg[3*sx-2];
	if (intens <= min_intens) min_intens = intens;
	if (intens >= max_intens) max_intens = intens;
	intens = grayImg[3*sx];
	if (intens <= min_intens) min_intens = intens;
	if (intens >= max_intens) max_intens = intens;
	minIntensity[2*sx+1] = min_intens;
	maxIntensity[2*sx+1] = max_intens;
	minIntensity[2*sx] = min_intens;
	maxIntensity[2*sx] = max_intens;
	minIntensity[sx-1] = min_intens;
	maxIntensity[sx-1] = max_intens;
	minIntensity[sx] = min_intens;
	maxIntensity[sx] = max_intens;

	min_intens = 255;
	max_intens = 0;
	for (s = 1; s < 3; s++)
	{
		for (t = 0; t < 3; t++)
		{
			intens = grayImg[(s+sy-3)*sx+t];
			if (intens <= min_intens) min_intens = intens;
			if (intens >= max_intens) max_intens = intens;
		}
	}
	intens = grayImg[(sy-3)*sx];
	if (intens <= min_intens) min_intens = intens;
	if (intens >= max_intens) max_intens = intens;
	intens = grayImg[(sy-3)*sx+1];
	if (intens <= min_intens) min_intens = intens;
	if (intens >= max_intens) max_intens = intens;
	minIntensity[(sy-1)*(sx+1)] = min_intens;
	maxIntensity[(sy-1)*(sx+1)] = max_intens;
	minIntensity[(sy-1)*(sx+1)+1] = min_intens;
	maxIntensity[(sy-1)*(sx+1)+1] = max_intens;
	minIntensity[sy*(sx+1)] = min_intens;
	maxIntensity[sy*(sx+1)] = max_intens;
	minIntensity[sy*(sx+1)+1] = min_intens;
	maxIntensity[sy*(sx+1)+1] = max_intens;

	min_intens = 255;
	max_intens = 0;
	for (s = 1; s < 3; s++)
	{
		for (t = 0; t < 3; t++)
		{
			intens = grayImg[(s+sy-2)*sx+t-3];
			if (intens <= min_intens) min_intens = intens;
			if (intens >= max_intens) max_intens = intens;
		}
	}
	intens = grayImg[(sy-2)*sx-2];
	if (intens <= min_intens) min_intens = intens;
	if (intens >= max_intens) max_intens = intens;
	intens = grayImg[(sy-2)*sx-1];
	if (intens <= min_intens) min_intens = intens;
	if (intens >= max_intens) max_intens = intens;
	minIntensity[(sy-1)*(sx+1)+sx-1] = min_intens;
	maxIntensity[(sy-1)*(sx+1)+sx-1] = max_intens;
	minIntensity[(sy-1)*(sx+1)+sx] = min_intens;
	maxIntensity[(sy-1)*(sx+1)+sx] = max_intens;
	minIntensity[sy*(sx+1)+sx-1] = min_intens;
	maxIntensity[sy*(sx+1)+sx-1] = max_intens;
	minIntensity[sy*(sx+1)+sx] = min_intens;
}

/**
 **************************************************************************************************
 * High Quality mode YUV image processing [SSE2 variant]
 * \param  [in,out]		out_Img			- destination image
 * \param  [in]			Str				- pointer on structure
 **************************************************************************************************
 */
void Stage_HighQualityYUV_SSE2(unsigned char* out_Img, VS_InitStruct* Str)
{
	Stage_HighQualityYUV_C(out_Img, Str);
}