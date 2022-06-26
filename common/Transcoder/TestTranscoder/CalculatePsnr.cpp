#include "CalculatePsnr.h"

double GetPSNR(int64_t mse, int w, int h)
{
	double psnr = 999.0;
	if (mse > 0) psnr = 10.0 * log10((double)(255.0 * 255.0 * w * h) / (double)mse);
	return psnr;
}

void CalculatePSNR_YUV_C(unsigned char *pImg0, unsigned char *pImg1, int w, int h, tVideoMetrics *vm, tVideoMetrics *vm_max, tVideoMetrics *vm_avg)
{
	memset(vm, 0, sizeof(tVideoMetrics));
	unsigned char *pSrc0 = pImg0, *pSrc1 = pImg1;
	int64_t mseY = 0, mseU = 0, mseV = 0;
	int64_t avgY0 = 0, avgY1 = 0;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			avgY0 += pSrc0[j];
			avgY1 += pSrc1[j];
			mseY += (pSrc0[j] - pSrc1[j]) * (pSrc0[j] - pSrc1[j]);
		}
		pSrc0 += w;
		pSrc1 += w;
	}
	vm->Y_YUV = GetPSNR(mseY, w, h);							vm_avg->Y_YUV += (double)mseY / (double)(w * h);
	vm->D_AVG_Y = (double)(avgY0 - avgY1) / (double)(w * h);	vm_avg->D_AVG_Y += vm_avg->D_AVG_Y;
}

#ifdef _WIN32
#include <emmintrin.h>

void CalculatePSNR_YUV_SSE2(unsigned char *pImg0, unsigned char *pImg1, int w, int h, tVideoMetrics *vm, tVideoMetrics *vm_max, tVideoMetrics *vm_avg)
{
	int i, j, k;
	__int64 mseY = 0, mseU = 0, mseV = 0;
	__int64 avgY0 = 0, avgY1 = 0;
	unsigned char *pSrc0 = pImg0, *pSrc1 = pImg1;
	memset(vm, 0, sizeof(tVideoMetrics));
	int w16 = w & ~15;
	__m128i xmm0, xmm1, xmm2, xmm3, xmm7;

	xmm7 = _mm_setzero_si128();

	for (i = 0; i < w16; i += 16) {
		pSrc0 = pImg0 + i;
		pSrc1 = pImg1 + i;
		xmm3 = _mm_setzero_si128();
		for (j = 0; j < h; j++) {
			for (k = 0; k < 16; k++) {
				avgY0 += pSrc0[k];
				avgY1 += pSrc1[k];
			}
			xmm0 = _mm_loadu_si128((__m128i*)pSrc0);
			xmm1 = _mm_loadu_si128((__m128i*)pSrc1);
			xmm2 = xmm0;
			xmm0 = _mm_subs_epu8(xmm0, xmm1);
			xmm1 = _mm_subs_epu8(xmm1, xmm2);
			xmm0 = _mm_or_si128(xmm0, xmm1);
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm7);
			xmm1 = _mm_unpackhi_epi8(xmm1, xmm7);
			xmm0 = _mm_madd_epi16(xmm0, xmm0);
			xmm1 = _mm_madd_epi16(xmm1, xmm1);
			xmm3 = _mm_add_epi32(xmm3, xmm0);
			xmm3 = _mm_add_epi32(xmm3, xmm1);
			pSrc0 += w;
			pSrc1 += w;
		}
		mseY += _mm_cvtsi128_si32(xmm3);
		xmm3 = _mm_srli_si128(xmm3, 4);
		mseY += _mm_cvtsi128_si32(xmm3);
		xmm3 = _mm_srli_si128(xmm3, 4);
		mseY += _mm_cvtsi128_si32(xmm3);
		xmm3 = _mm_srli_si128(xmm3, 4);
		mseY += _mm_cvtsi128_si32(xmm3);
	}

	if (w16 != w) {
		pSrc0 = pImg0 + i;
		pSrc1 = pImg1 + i;
		for (j = 0; j < h; j++) {
			for (; i < w; i++) {
				avgY0 += pSrc0[j];
				avgY1 += pSrc1[j];
				mseY += (pSrc0[j] - pSrc1[j]) * (pSrc0[j] - pSrc1[j]);
			}
			pSrc0 += w;
			pSrc1 += w;
		}
	}

	vm->Y_YUV = GetPSNR(mseY, w, h);							vm_avg->Y_YUV += (double)mseY / (double)(w * h);
	vm->D_AVG_Y = (double)(avgY0 - avgY1) / (double)(w * h);	vm_avg->D_AVG_Y += vm_avg->D_AVG_Y;
}
#endif // __WIN32

void CalculatePSNR(unsigned char * pImg0, unsigned char * pImg1, int w, int h, tVideoMetrics * vm, tVideoMetrics * vm_max, tVideoMetrics * vm_avg)
{
#ifdef _WIN32
	CalculatePSNR_YUV_SSE2(pImg0, pImg1, w, h, vm, vm_max, vm_avg);
#else
	CalculatePSNR_YUV_C(pImg0, pImg1, w, h, vm, vm_max, vm_avg);
#endif // _WIN32
}
