/**
 *******************************************************************************************************
 * \file Deinterlacing.cpp
 * \brief Deinterlacing filter implementation
 *
 * (c) 2006 Visicron Corp.  http://www.visicron.net/
 *
 * \b Project: Deinterlacing filter
 * \author Sergey Anufriev
 * \author Smirnov Konstatntin
 * \date 03.04.2006
 *******************************************************************************************************
 */

#include "Deinterlacing.h"
#include "VSVideoProc.h"
#include <math.h>
#include <memory.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#pragma warning(disable:4309)

typedef unsigned long vs32u;
typedef unsigned char vs8u;

/// Time measurements
#ifdef TEST_TIME
#include <windows.h>
#include <stdio.h>
	LARGE_INTEGER		frequency, time_motion, time_bicubic, time_noiser, time_memory;
#endif

/// Get maximum value
#ifndef vs_max
#define vs_max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

/// Get minimum value
#ifndef vs_min
#define vs_min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#undef THRESHOLD_Y
#define THRESHOLD_Y (20)		///< threshold of motion for luma

#undef SCENECHANGE
#define SCENECHANGE (45)		///< scene change threshold

#undef ALGORITM_FILTER
#define ALGORITM_FILTER (0)		///< algorithm of filtering

#define ALLOC_ALIGN(t, s) (t*)_aligned_malloc(s, 16)
#define FREE_ALIGN(p) \
	if (p) { \
		_aligned_free(p); \
	} \
	p = 0; \

/**
 *******************************************************************************************************
 * Constructor.
 *******************************************************************************************************
 */
VS_Deinterlacing::VS_Deinterlacing(typeProc proc, typeFilter filter)
{
	m_prevFrame = 0;
	m_moving = 0;
	m_moving_prev = 0;
	m_moving_u = 0;
	m_moving_prev_u = 0;
	m_moving_v = 0;
	m_moving_prev_v = 0;
	m_nrmoving = 0;
	m_moving_d = 0;
	m_VProcY_up = 0;
	m_VProcY_down = 0;
	m_VProcUV_up = 0;
	m_VProcUV_down = 0;
	m_isInit = false;

	m_threshold_y = THRESHOLD_Y;
	m_threshold_uv = m_threshold_y / 3;
	m_scenethreshold = SCENECHANGE;
	m_procType = proc;
	m_filterType = filter;
}

/**
 *******************************************************************************************************
 * Destructor.
 *******************************************************************************************************
 */
VS_Deinterlacing::~VS_Deinterlacing()
{
	Release();
}

/**
 *******************************************************************************************************
 * Initialisation function. Allocate memory for Initialisation Structure, validate input numeric values.
 * \param  [in]      width		 - number of pixels in source image on X direction
 * \param  [in]      height		 - number of pixels in source image on Y direction
 * \param  [in]      pitch_y	 - luma plane offset
 * \param  [in]      pitch_uv	 - chroma plane offset
 * \param  [in]      proc		 - processor type
 * \param  [in]      filter		 - algorithm mode
 * \return	VS_NOERR if no error occurred
 *******************************************************************************************************
 */
VS_DERROR VS_Deinterlacing::Init(int width, int height, int pitch_y, int pitch_uv, typeFilter filter)
{
	if (width%16 != 0 || height < 2 || pitch_y < width || pitch_uv < (width >> 1) || width > 65536)
		return VS_PARAM;

	Release();

	m_w = width;
	m_h = height;
	m_pitch = pitch_y;
	m_square = m_w * m_h;
	m_w2 = m_w >> 1;
	m_h2 = m_h >> 1;
	m_pitch2 = pitch_uv;
	m_hminus1 = m_h - 1;
	m_hminus3 = m_h - 3;
	m_wminus1 = m_w - 1;
	m_filterType = filter;

	if (m_filterType == QUALITY) {

		switch (m_procType)
		{
		case C:
			  {
				  MotionDetected = &VS_Deinterlacing::C_MotionDetected;
				  NoiseReduction = &VS_Deinterlacing::C_NoiseReduction_Quality;
				  InterpolateField = &VS_Deinterlacing::C_InterpolateField;
				  InterpolateField_SceneChange = &VS_Deinterlacing::C_InterpolateField_SceneChange;
				  break;
			  }
		case MMX:
			  {
				  MotionDetected = &VS_Deinterlacing::MMX_MotionDetected;
				  NoiseReduction = &VS_Deinterlacing::MMX_NoiseReduction_Quality;
				  InterpolateField = &VS_Deinterlacing::MMX_InterpolateField;
				  InterpolateField_SceneChange = &VS_Deinterlacing::MMX_InterpolateField_SceneChange;

#if (ALGORITM_FILTER == 1)
				  InterpolateField = &VS_Deinterlacing::C_InterpolateField;
				  InterpolateField_SceneChange = &VS_Deinterlacing::C_InterpolateField_SceneChange;
#endif

				  break;
			  }
		case SSE2:
			{
				  MotionDetected = &VS_Deinterlacing::SSE2_MotionDetected;
				  NoiseReduction = &VS_Deinterlacing::SSE2_NoiseReduction_Quality;
				  InterpolateField = &VS_Deinterlacing::SSE2_InterpolateField;
				  InterpolateField_SceneChange = &VS_Deinterlacing::SSE2_InterpolateField_SceneChange;

#if (ALGORITM_FILTER == 1)
				  InterpolateField = &VS_Deinterlacing::C_InterpolateField;
				  InterpolateField_SceneChange = &VS_Deinterlacing::C_InterpolateField_SceneChange;
#endif

				  break;
			}
		}

		m_prevFrame = ALLOC_ALIGN(unsigned char, (m_pitch + m_pitch2) * m_h);
		memset(m_prevFrame, 0, (m_pitch + m_pitch2) * m_h);
		m_moving = ALLOC_ALIGN(unsigned char, m_square);
		memset(m_moving, 0, m_square);
		m_moving_prev = ALLOC_ALIGN(unsigned char, m_square);
		memset(m_moving_prev, 0, m_square);
		m_moving_u = ALLOC_ALIGN(unsigned char, m_square / 4);
		memset(m_moving_u, 0, m_square / 4);
		m_moving_prev_u = ALLOC_ALIGN(unsigned char, m_square / 4);
		memset(m_moving_prev_u, 0, m_square / 4);
		m_moving_v = ALLOC_ALIGN(unsigned char, m_square / 4);
		memset(m_moving_v, 0, m_square / 4);
		m_moving_prev_v =ALLOC_ALIGN(unsigned char, m_square / 4);
		memset(m_moving_prev_v, 0, m_square / 4);
		m_nrmoving = ALLOC_ALIGN(unsigned char, m_square);
		memset(m_nrmoving, 1, m_square);
		m_moving_d = ALLOC_ALIGN(unsigned char, m_square);
		memset(m_moving_d, 0, m_square);

		m_VProcY_down = new VS_VideoProc((m_procType == C));
		m_VProcY_up = new VS_VideoProc((m_procType == C));
		m_VProcUV_down = new VS_VideoProc((m_procType == C));
		m_VProcUV_up = new VS_VideoProc((m_procType == C));

	} else if (m_filterType == BLEND) {
		switch (m_procType)
		{
		case C:
			BlendInterpolate =  &VS_Deinterlacing::C_BlendInterpolate;
			break;
		case MMX:
			BlendInterpolate =  &VS_Deinterlacing::MMX_BlendInterpolate;
			break;
		case SSE2:
			BlendInterpolate =  &VS_Deinterlacing::SSE2_BlendInterpolate;
			break;
		}
	}

	m_isInit = true;

#ifdef TEST_TIME
	QueryPerformanceFrequency(&frequency);
	time_motion.QuadPart = 0;
	time_bicubic.QuadPart = 0;
	time_noiser.QuadPart = 0;
	time_memory.QuadPart = 0;
#endif

	return VS_NOERR;
}

/**
 **************************************************************************************************
 * Motion detecting, motion map filling. C implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [in]      prevframe	 - pointer to previous image
 * \param  [out]     motion		 - pointer to motion map
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 * \param  [in]      threshold 	 - threshold of motion for luma/chroma
 **************************************************************************************************
 */
void VS_Deinterlacing::C_MotionDetected(unsigned char *inframe, unsigned char *prevframe, unsigned char *motion, int width, int height, int pitch, int threshold)
{
	unsigned char	*moving;
	vs8u			*prev, *src;
	int				x = 0, y = 0;
	unsigned short	delta_frame = 0, delta_field1 = 0, delta_field2 = 0, delta_thr_frame = 0;

	src = inframe + pitch;
	prev = prevframe + pitch;
	moving = motion + width;
	for (y = 1; y < height - 1; y++) {
		for (x = 0; x < width; x++) {
			moving[x] = 0;
			delta_frame = abs(src[x] - prev[x]);
			delta_thr_frame = vs_max(0, threshold - (delta_frame >> 1));
			if (delta_frame > delta_thr_frame) {
				moving[x] = 1;
				m_num_pnt_scene++;
			}
		}
		src += pitch;
		prev += pitch;
		moving += width;
	}
}

/**
 **************************************************************************************************
 * Noise reduction on motion map. Quality mode. C implementation.
 * \param  [in,out]  motion		 - pointer to motion map
 * \param  [in,out]  nrmotion	 - pointer to temporary array
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 **************************************************************************************************
 */
void VS_Deinterlacing::C_NoiseReduction_Quality(unsigned char *motion, unsigned char *nrmotion, int width, int height, VS_VideoProc *VprUp, VS_VideoProc *VprDown)
{
	int x = 0, y = 0, u = 0, sum = 0;
	vs8u *moving = motion, *nrmoving = nrmotion + 2 * width;

	for (y = 2; y < height - 2; y++) {
		for (x = 2; x < width - 2; x++) {
			sum = 0;
			for (u = 0; u < 5; u++) {
				sum += moving[x-2];
				sum += moving[x-1];
				sum += moving[x];
				sum += moving[x+1];
				sum += moving[x+2];
				moving += width;
			}
			moving -= 5 * width;
			nrmoving[x] = 0;
			if (sum > 5) {
				nrmoving[x] = 200;
			}
		}
		moving += width;
		nrmoving += width;
	}

	moving = motion;
	nrmoving = nrmotion;
	VprDown->ResampleDown_8d1(nrmoving, m_moving_d, width, height);
	VprUp->ResampleRGB8(m_moving_d, moving, width / 8, height / 8, width, height, width);
}

/**
 **************************************************************************************************
 * Noise reduction on motion map. Speed mode. C implementation.
 * \param  [in,out]  motion		 - pointer to motion map
 * \param  [in,out]  nrmotion	 - pointer to temporary array
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 **************************************************************************************************
 */
void VS_Deinterlacing::C_NoiseReduction_Fast(unsigned char *motion, unsigned char *nrmotion, int width, int height)
{
	int x = 0, y = 0, u = 0, v = 0, sum = 0;
	int w = (width / 6) * 6;
	vs8u *moving = motion, *nrmoving = nrmotion + width;

	for (y = 1; y < height - 1; y++) {
		for (x = 1; x < w; x++) {
			sum = 0;
			sum += moving[x-1];
			sum += moving[x];
			sum += moving[x+1];
			sum += moving[x+width-1];
			sum += moving[x+width];
			sum += moving[x+width+1];
			sum += moving[x+2*width-1];
			sum += moving[x+2*width];
			sum += moving[x+2*width+1];
			nrmoving[x] = 0;
			if (sum > 3) {
				nrmoving[x] = 1;
			}
		}
		moving += width;
		nrmoving += width;
	}

	memcpy(motion, nrmotion, width * height);
}

/**
 **************************************************************************************************
 * Interpolate moving areas (except when scene change). C implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [out]     outframe	 - pointer to destination image
 * \param  [in]      motion		 - pointer to motion map current image
 * \param  [in]      motion_prev - pointer to motion map previous image
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 **************************************************************************************************
 */
void VS_Deinterlacing::C_InterpolateField(unsigned char *inframe, unsigned char *outframe, unsigned char *motion, unsigned char *motion_prev, int width, int height, int pitch)
{

#if (ALGORITM_FILTER == 0)

	vs8u			*src, *dst, *srcminus, *srcplus, *srcminusminus, *srcplusplus;
	vs8u			*moving, *movingminus, *movingplus, *moving_prev;
	int				x, y;
	short			luma;

	src = inframe;
	dst = outframe;
	memcpy(dst, src, pitch);
	src = inframe + pitch;
	srcminus = inframe;
	srcplus = inframe + 2 * pitch;
	srcminusminus = inframe - 2 * pitch;
	srcplusplus = inframe + 4 * pitch;
	dst = outframe + pitch;
	moving = motion + width;
	movingminus = moving - width;
	movingplus = moving + width;
	moving_prev = motion_prev + width;
	for (y = 1; y < height - 1; y++) {
		if (~y&1) {
			for (x = 0; x < width; x++)	{
				if (!(movingminus[x] || moving[x] || movingplus[x] || moving_prev[x])) {
					dst[x] = src[x];
				} else {
					if ((y > 2) && (y < (height - 3))) {
						luma = (5 * (srcminus[x] + srcplus[x]) - (srcminusminus[x] + srcplusplus[x]) + 4) >> 3;
						if (luma > 255) luma = 255;
						if (luma < 0) luma = 0;
						dst[x] = (vs8u)luma;
					} else {
						dst[x] = (srcminus[x] + srcplus[x] + 1) >> 1;
					}
				}
			}
		}
		else {
			memcpy(dst, src, pitch);
		}
		src += pitch;
		srcminus += pitch;
		srcplus += pitch;
		srcminusminus += pitch;
		srcplusplus += pitch;
		dst += pitch;
		moving += width;
		movingminus += width;
		movingplus += width;
		moving_prev += width;
	}
	memcpy(dst, src, pitch);

#else
#if (ALGORITM_FILTER == 1)

	vs8u			*src, *dst, *srcminus, *srcplus;
	vs8u			*moving, *movingminus, *movingplus, *moving_prev;
	int				x, y;

	int stop = 0, min = 0, minf = 0, maxf = 0, val = 0, u = 0, s1 = 0, s2 = 0, temp1 = 0, temp2 = 0;
	src = inframe;
	dst = outframe;
	memcpy(dst, src, m_pitch);
	src = inframe + m_pitch;
	srcminus = inframe;
	srcplus = inframe + 2 * m_pitch;
	dst = outframe + m_pitch;
	moving = m_moving + m_w;
	movingminus = moving - m_w;
	movingplus = moving + m_w;
	moving_prev = m_moving_prev + m_w;
	for (y = 1; y < m_hminus1; y++) {
		if (~y&1) {
			dst[0] = (srcminus[0] + srcplus[0]) >> 1;
			dst[1] = (srcminus[1] + srcplus[1]) >> 1;
			dst[2] = (srcminus[2] + srcplus[2]) >> 1;
			dst[3] = (srcminus[3] + srcplus[3]) >> 1;
			dst[m_wminus1-3] = (srcminus[m_wminus1-3] + srcplus[m_wminus1-3]) >> 1;
			dst[m_wminus1-2] = (srcminus[m_wminus1-2] + srcplus[m_wminus1-2]) >> 1;
			dst[m_wminus1-1] = (srcminus[m_wminus1-1] + srcplus[m_wminus1-1]) >> 1;
			dst[m_wminus1] = (srcminus[m_wminus1] + srcplus[m_wminus1-1]) >> 1;
			for (x = 4; x < m_wminus1 - 3; x++) {
				if (!(movingminus[x] | moving[x] | movingplus[x] | moving_prev[x])) {
					dst[x] = src[x];
				} else {
					minf = vs_min(srcminus[x], srcplus[x]) - 2;
					maxf = vs_max(srcminus[x], srcplus[x]) + 2;
					val = (srcminus[x] + srcplus[x] + 1) >> 1;
					for (min = 450, u = 0; u <= 8; ++u) {
						s1 = srcminus[x+(u>>1)] + srcminus[x+((u+1)>>1)];
						s2 = srcplus[x-(u>>1)] + srcplus[x-((u+1)>>1)];
						temp1 = abs(s1 - s2) + abs(srcminus[x-1]-srcplus[x-1-u]) +
							(abs(srcminus[x]-srcplus[x-u])<<1) + abs(srcminus[x+1]-srcplus[x+1-u]) +
							abs(srcplus[x-1]-srcminus[x-1+u]) + (abs(srcplus[x]-srcminus[x+u])<<1) +
							abs(srcplus[x+1]-srcminus[x+1+u]);
						temp2 = (s1 + s2 + 2) >> 2;
						if (temp1 < min && temp2 >= minf && temp2 <= maxf) {
							min = temp1;
							val = temp2;
						}
						s1 = srcminus[x-(u>>1)] + srcminus[x-((u+1)>>1)];
						s2 = srcplus[x+(u>>1)] + srcplus[x+((u+1)>>1)];
						temp1 = abs(s1 - s2) + abs(srcminus[x-1]-srcplus[x-1+u]) +
							(abs(srcminus[x]-srcplus[x+u])<<1) + abs(srcminus[x+1]-srcplus[x+1+u])+
							abs(srcplus[x-1]-srcminus[x-1-u]) + (abs(srcplus[x]-srcminus[x-u])<<1) +
							abs(srcplus[x+1]-srcminus[x+1-u]);
						temp2 = (s1 + s2 + 2) >> 2;
						if (temp1 < min && temp2 >= minf && temp2 <= maxf) {
							min = temp1;
							val = temp2;
						}
					}
					dst[x] = val;
				}
			}
		}
		else {
			memcpy(dst, src, m_pitch);
		}
		src += m_pitch;
		srcminus += m_pitch;
		srcplus += m_pitch;
		dst += m_pitch;
		moving += m_w;
		movingminus += m_w;
		movingplus += m_w;
		moving_prev += m_w;
	}
	memcpy(dst, src, m_pitch);

#endif
#endif

}

/**
 **************************************************************************************************
 * Scene change interpolate image. C implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [out]     outframe	 - pointer to destination image
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 **************************************************************************************************
 */
void VS_Deinterlacing::C_InterpolateField_SceneChange(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch)
{

#if (ALGORITM_FILTER == 0)

	vs8u			*src, *dst, *srcminus, *srcplus, *srcminusminus, *srcplusplus;
	int				x, y;
	short			luma;

	src = inframe;
	dst = outframe;
	memcpy(dst, src, pitch);
	src = inframe + pitch;
	srcminus = inframe;
	srcplus = inframe + 2 * pitch;
	srcminusminus = inframe - 2 * pitch;
	srcplusplus = inframe + 4 * pitch;
	dst = outframe + pitch;
	for (y = 1; y < height - 1; y++) {
		if (~y&1) {
			for (x = 0; x < width; x++) {
				if ((y > 2) && (y < (height - 3))) {
					luma = (5 * (srcminus[x] + srcplus[x]) - (srcminusminus[x] + srcplusplus[x]) + 4) >> 3;
					if (luma > 255) luma = 255;
					if (luma < 0) luma = 0;
					dst[x] = (vs8u)luma;
				} else {
					dst[x] = (srcminus[x] + srcplus[x] + 1) >> 1;
				}
			}
		}
		else {
			memcpy(dst, src, pitch);
		}
		src += pitch;
		srcminus += pitch;
		srcplus += pitch;
		srcminusminus += pitch;
		srcplusplus += pitch;
		dst += pitch;
	}
	memcpy(dst, src, pitch);

#else
#if (ALGORITM_FILTER == 1)

	vs8u			*src, *dst, *srcminus, *srcplus;
	int				x, y;

	int stop = 0, min = 0, minf = 0, maxf = 0, val = 0, u = 0, s1 = 0, s2 = 0, temp1 = 0, temp2 = 0;
	src = inframe;
	dst = outframe;
	memcpy(dst, src, m_pitch);
	src = inframe + m_pitch;
	srcminus = inframe;
	srcplus = inframe + 2 * m_pitch;
	dst = outframe + m_pitch;
	for (y = 1; y < m_hminus1; y++) {
		if (~y&1) {
			dst[0] = (srcminus[0] + srcplus[0]) >> 1;
			dst[1] = (srcminus[1] + srcplus[1]) >> 1;
			dst[2] = (srcminus[2] + srcplus[2]) >> 1;
			dst[3] = (srcminus[3] + srcplus[3]) >> 1;
			dst[m_wminus1-3] = (srcminus[m_wminus1-3] + srcplus[m_wminus1-3]) >> 1;
			dst[m_wminus1-2] = (srcminus[m_wminus1-2] + srcplus[m_wminus1-2]) >> 1;
			dst[m_wminus1-1] = (srcminus[m_wminus1-1] + srcplus[m_wminus1-1]) >> 1;
			dst[m_wminus1] = (srcminus[m_wminus1] + srcplus[m_wminus1-1]) >> 1;
			for (x = 4; x < m_wminus1 - 3; x++) {
				minf = vs_min(srcminus[x], srcplus[x]) - 2;
				maxf = vs_max(srcminus[x], srcplus[x]) + 2;
				val = (srcminus[x] + srcplus[x] + 1) >> 1;
				for (min = 450, u = 0; u <= 8; ++u) {
					s1 = srcminus[x+(u>>1)] + srcminus[x+((u+1)>>1)];
					s2 = srcplus[x-(u>>1)] + srcplus[x-((u+1)>>1)];
					temp1 = abs(s1 - s2) + abs(srcminus[x-1]-srcplus[x-1-u]) +
						(abs(srcminus[x]-srcplus[x-u])<<1) + abs(srcminus[x+1]-srcplus[x+1-u]) +
						abs(srcplus[x-1]-srcminus[x-1+u]) + (abs(srcplus[x]-srcminus[x+u])<<1) +
						abs(srcplus[x+1]-srcminus[x+1+u]);
					temp2 = (s1 + s2 + 2) >> 2;
					if (temp1 < min && temp2 >= minf && temp2 <= maxf) {
						min = temp1;
						val = temp2;
					}
					s1 = srcminus[x-(u>>1)] + srcminus[x-((u+1)>>1)];
					s2 = srcplus[x+(u>>1)] + srcplus[x+((u+1)>>1)];
					temp1 = abs(s1 - s2) + abs(srcminus[x-1]-srcplus[x-1+u]) +
						(abs(srcminus[x]-srcplus[x+u])<<1) + abs(srcminus[x+1]-srcplus[x+1+u])+
						abs(srcplus[x-1]-srcminus[x-1-u]) + (abs(srcplus[x]-srcminus[x-u])<<1) +
						abs(srcplus[x+1]-srcminus[x+1-u]);
					temp2 = (s1 + s2 + 2) >> 2;
					if (temp1 < min && temp2 >= minf && temp2 <= maxf) {
						min = temp1;
						val = temp2;
					}
				}
				dst[x] = val;
			}
		} else {
			memcpy(dst, src, m_pitch);
		}
		src += m_pitch;
		srcminus += m_pitch;
		srcplus += m_pitch;
		dst += m_pitch;
	}
	memcpy(dst, src, m_pitch);

#endif
#endif

}

/**
 **************************************************************************************************
 * Blend interpolate image. C implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [out]     outframe	 - pointer to destination image
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 **************************************************************************************************
 */
void VS_Deinterlacing::C_BlendInterpolate(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch)
{
	vs8u			*src, *dst, *srcminus;
	int				x, y;

	src = inframe;
	dst = outframe;
	memcpy(dst, src, pitch);
	src = inframe + pitch;
	srcminus = inframe;
	dst = outframe + pitch;
	for (y = 1; y < height; y++) {
		for (x = 0; x < width; x++) {
			dst[x] = (src[x] + srcminus[x] + 1) >> 1;
		}
		src += pitch;
		srcminus += pitch;
		dst += pitch;
	}
}

/**
 **************************************************************************************************
 * Motion detecting, motion map filling. MMX implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [in]      prevframe	 - pointer to previous image
 * \param  [out]     motion		 - pointer to motion map
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 * \param  [in]      threshold 	 - threshold of motion for luma/chroma
 **************************************************************************************************
 */
void VS_Deinterlacing::MMX_MotionDetected(unsigned char *inframe, unsigned char *prevframe, unsigned char *motion, int width, int height, int pitch, int threshold)
{
	unsigned char	*moving;
	vs8u			*prev, *src;
	int				x = 0, y = 0;

	__m64 m0, m1, m2, m4, m5, m6, mmx_acc;
	const __m64 mmx_thr  = _mm_set1_pi16 (threshold),
				mmx_null = _mm_setzero_si64 (),
				mmx_mask = _mm_set1_pi16 (1);

	src = inframe + pitch;
	prev = prevframe + pitch;
	moving = motion + width;
	vs8u *src_up, *src_low;

	for (y = 1; y < height - 1; y++) {
		src_up = src + pitch;
		src_low = src - pitch;
		mmx_acc = _mm_setzero_si64 ();
		for (x = 0; x < width; x += 8) {
			m0 = *(__m64*)(src+x);
			m1 = *(__m64*)(prev+x);
			/// abs(src[x] - prev[x]) = delta_frame
			m2 = _m_psubusb(m0, m1);
			m1 = _m_psubusb(m1, m0);
			m1 = _m_paddusb(m1, m2);
			m5 = _m_punpckhbw(m1, mmx_null);
			m6 = _m_punpcklbw(m1, mmx_null);
			m0 = m6;
			m4 = m5;
			/// m_threshold_y - (delta_frame >> 1)
			m5 = _m_psrawi(m5, 1);
			m6 = _m_psrawi(m6, 1);
			m5 = _m_psubusw(mmx_thr, m5);
			m6 = _m_psubusw(mmx_thr, m6);
			/// m0 ? m1
			m0 = _m_pcmpgtw(m0, m6);
			m4 = _m_pcmpgtw(m4, m5);
			m0 = _m_pand(m0, mmx_mask);
			m4 = _m_pand(m4, mmx_mask);
			mmx_acc = _m_paddusb(mmx_acc, m0); // scene change
			mmx_acc = _m_paddusb(mmx_acc, m4); // scene change
			*(__m64*)(moving+x) = _m_packuswb(m0, m4);
		}
		m0 = _m_psrlqi(mmx_acc, 32);
		mmx_acc = _m_paddusw(mmx_acc, m0);
		m0 = _m_psrlqi(mmx_acc, 16);
		mmx_acc = _m_paddusw(mmx_acc, m0);
		m_num_pnt_scene += (unsigned short)_m_to_int(mmx_acc);
		src += pitch;
		prev += pitch;
		moving += width;
	}
	_mm_empty();
}

/**
 **************************************************************************************************
 * Noise reduction on motion map. Quality mode. MMX implementation.
 * \param  [in,out]  motion		 - pointer to motion map
 * \param  [in,out]  nrmotion	 - pointer to temporary array
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 **************************************************************************************************
 */
void VS_Deinterlacing::MMX_NoiseReduction_Quality(unsigned char *motion, unsigned char *nrmotion, int width, int height, VS_VideoProc *VprUp, VS_VideoProc *VprDown)
{
	int x = 0, y = 0, u = 0, v = 0;
	vs8u *moving = motion, *nrmoving = nrmotion + 2 * width;

	__m64 m0, m1, m2, m3, m4, m5;
	const __m64 mmx_noise_threshold = _mm_set1_pi8 (5),
				mmx_noise_mask = _mm_set1_pi8 (200);

	for (y = 2; y < height - 2; y++) {
		for (x = 2; x < width - 2; x += 8) {
			m0 = *(__m64*)(moving+x-2);
			moving += width;
			m1 = *(__m64*)(moving+x-2);
			moving += width;
			m2 = *(__m64*)(moving+x-2);
			moving += width;
			m3 = *(__m64*)(moving+x-2);
			moving += width;
			m4 = *(__m64*)(moving+x-2);
			m0 = _m_paddusb(m0, m1);
			m0 = _m_paddusb(m0, m2);
			m0 = _m_paddusb(m0, m3);
			m0 = _m_paddusb(m0, m4);
			m1 = m0;
			m1 = _m_psrlqi(m1, 8);
			m0 = _m_paddusb(m0, m1);
			m1 = _m_psrlqi(m1, 8);
			m0 = _m_paddusb(m0, m1);
			m1 = _m_psrlqi(m1, 8);
			m0 = _m_paddusb(m0, m1);
			m1 = _m_psrlqi(m1, 8);
			m0 = _m_paddusb(m0, m1);
			m5 = m0;
			moving -= 4 * width;

			m0 = *(__m64*)(moving+x+2);
			moving += width;
			m1 = *(__m64*)(moving+x+2);
			moving += width;
			m2 = *(__m64*)(moving+x+2);
			moving += width;
			m3 = *(__m64*)(moving+x+2);
			moving += width;
			m4 = *(__m64*)(moving+x+2);
			m0 = _m_paddusb(m0, m1);
			m0 = _m_paddusb(m0, m2);
			m0 = _m_paddusb(m0, m3);
			m0 = _m_paddusb(m0, m4);
			m1 = m0;
			m1 = _m_psrlqi(m1, 8);
			m0 = _m_paddusb(m0, m1);
			m1 = _m_psrlqi(m1, 8);
			m0 = _m_paddusb(m0, m1);
			m1 = _m_psrlqi(m1, 8);
			m0 = _m_paddusb(m0, m1);
			m1 = _m_psrlqi(m1, 8);
			m0 = _m_paddusb(m0, m1);
			moving -= 4 * width;

			m0 = _m_punpckldq(m5, m0);
			m0 = _m_pcmpgtb(m0, mmx_noise_threshold);
			m0 = _m_pand(m0, mmx_noise_mask);
			*(__m64*)(nrmoving+x) = m0;
		}
		moving += width;
		nrmoving += width;
	}

	_mm_empty();

	moving = motion;
	nrmoving = nrmotion;
	VprDown->ResampleDown_8d1(nrmoving, m_moving_d, width, height);
	VprUp->ResampleRGB8(m_moving_d, moving, width / 8, height / 8, width, height, width);

	_mm_empty();
}

/**
 **************************************************************************************************
 * Noise reduction on motion map. Speed mode. MMX implementation.
 * \param  [in,out]  motion		 - pointer to motion map
 * \param  [in,out]  nrmotion	 - pointer to temporary array
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 **************************************************************************************************
 */
void VS_Deinterlacing::MMX_NoiseReduction_Fast(unsigned char *motion, unsigned char *nrmotion, int width, int height)
{
	int x = 0, y = 0, u = 0, v = 0;
	int w = (width / 6) * 6;
	vs8u *moving = motion, *nrmoving = nrmotion + width;

	__m64 m0, m1, m2;
	const __m64 mmx_noise_threshold1 = _mm_set1_pi8 (3),
				mmx_noise_mask = _mm_set1_pi8 (1);

	for (y = 1; y < height - 1; y++) {
		for (x = 1; x < w; x += 6) {
			m0 = *(__m64*)(moving+x-1);
			m1 = *(__m64*)(moving+x+width-1);
			m2 = *(__m64*)(moving+x+2*width-1);
			m0 = _m_paddusb(m0, m1);
			m0 = _m_paddusb(m0, m2);
			m2 = m0;

			m2 = _m_psrlqi(m2, 8);
			m0 = _m_paddusb(m0, m2);
			m2 = _m_psrlqi(m2, 8);
			m0 = _m_paddusb(m0, m2);

			m0 = _m_pcmpgtb(m0, mmx_noise_threshold1);
			m0 = _m_pand(m0, mmx_noise_mask);
			*(__m64*)(nrmoving+x) = m0;
		}
		moving += width;
		nrmoving += width;
	}
	memcpy(motion, nrmotion, width * height);

	_mm_empty();
}

/**
 **************************************************************************************************
 * Interpolate moving areas (except when scene change). MMX implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [out]     outframe	 - pointer to destination image
 * \param  [in]      motion		 - pointer to motion map current image
 * \param  [in]      motion_prev - pointer to motion map previous image
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 **************************************************************************************************
 */
void VS_Deinterlacing::MMX_InterpolateField(unsigned char *inframe, unsigned char *outframe, unsigned char *motion, unsigned char *motion_prev, int width, int height, int pitch)
{
	vs8u			*src, *dst, *srcminus, *srcplus, *srcminusminus, *srcplusplus;
	vs8u			*moving, *movingminus, *movingplus, *moving_prev;
	int				x, y;

	__m64 m0, m1, m2, m3, m4, m5, m6, m7;
	const __m64 mmx_add_shift4 = _mm_set1_pi16 (4),
				mmx_add_shift1 = _mm_set1_pi16 (1),
				mmx_null = _mm_setzero_si64 (),
				mmx_mask = _mm_set1_pi8 (0xff);

	src = inframe;
	dst = outframe;
	memcpy(dst, src, pitch);
	src = inframe + pitch;
	srcminus = inframe;
	srcplus = inframe + 2 * pitch;
	srcminusminus = inframe - 2 * pitch;
	srcplusplus = inframe + 4 * pitch;
	dst = outframe + pitch;
	moving = motion + width;
	movingminus = moving - width;
	movingplus = moving + width;
	moving_prev = motion_prev + width;
	for (y = 1; y < height - 1; y++) {
		if (~y&1) {
			for (x = 0; x < width; x += 8) {
				m0 = *(__m64*)(srcminus + x);
				m2 = *(__m64*)(srcplus + x);
				m1 = _m_punpckhbw(m0, mmx_null);
				m0 = _m_punpcklbw(m0, mmx_null);
				m3 = _m_punpckhbw(m2, mmx_null);
				m2 = _m_punpcklbw(m2, mmx_null);
				if ((y > 2) && (y < (height - 3))) {
                    m4 = *(__m64*)(srcminusminus+x);
					m5 = _m_punpckhbw(m4, mmx_null);
					m4 = _m_punpcklbw(m4, mmx_null);
					m6 = *(__m64*)(srcplusplus+x);
					m7 = _m_punpckhbw(m6, mmx_null);
					m6 = _m_punpcklbw(m6, mmx_null);
					m0 = _m_paddusw(m0, m2);
					m1 = _m_paddusw(m1, m3);
					m4 = _m_paddusw(m4, m6);
					m5 = _m_paddusw(m5, m7);
					m2 = _m_psllwi(m0, 2);
					m3 = _m_psllwi(m1, 2);
					m0 = _m_paddusw(m0, m2);
					m1 = _m_paddusw(m1, m3);
					m0 = _m_psubsw(m0, m4);
					m1 = _m_psubsw(m1, m5);
					m0 = _m_paddw(m0, mmx_add_shift4);
					m1 = _m_paddw(m1, mmx_add_shift4);
					m0 = _m_psrawi(m0, 3);
					m1 = _m_psrawi(m1, 3);
					m0 = _m_packuswb(m0, m1);
				} else {
					m0 = _m_paddusw(m0, m2);
					m1 = _m_paddusw(m1, m3);
					m0 = _m_paddusw(m0, mmx_add_shift1);
					m1 = _m_paddusw(m1, mmx_add_shift1);
					m0 = _m_psrawi(m0, 1);
					m1 = _m_psrawi(m1, 1);
					m0 = _m_packuswb(m0, m1);
				}
				m2 = *(__m64*)(src + x);
				m3 = *(__m64*)(moving + x);
				m4 = *(__m64*)(movingminus + x);
				m5 = *(__m64*)(movingplus + x);
				m6 = *(__m64*)(moving_prev + x);
				m3 = _m_por(m3, m4);
				m3 = _m_por(m3, m5);
				m3 = _m_por(m3, m6);
				m3 = _m_pcmpeqb(m3, mmx_null);
				m3 = _m_pxor(m3, mmx_mask );
				m0 = _m_pand(m0, m3);
				m2 = _m_pandn(m3, m2);
				m0 = _m_por(m0, m2);
				*(__m64*)(dst+x) = m0;
			}
		} else {
			memcpy(dst, src, pitch);
		}
		src += pitch;
		srcminus += pitch;
		srcplus += pitch;
		srcminusminus += pitch;
		srcplusplus += pitch;
		dst += pitch;
		moving += width;
		movingminus += width;
		movingplus += width;
		moving_prev += width;
	}
	memcpy(dst, src, pitch);

	_mm_empty();
}

/**
 **************************************************************************************************
 * Scene change interpolate image. MMX implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [out]     outframe	 - pointer to destination image
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 **************************************************************************************************
 */
void VS_Deinterlacing::MMX_InterpolateField_SceneChange(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch)
{
	vs8u			*src, *dst, *srcminus, *srcplus, *srcminusminus, *srcplusplus;
	int				x, y;

	__m64 m0, m1, m2, m3, m4, m5, m6, m7;
	const __m64 mmx_add_shift4 = _mm_set1_pi16 (4),
				mmx_add_shift1 = _mm_set1_pi16 (1),
				mmx_null = _mm_setzero_si64 ();

	src = inframe;
	dst = outframe;
	memcpy(dst, src, pitch);
	src = inframe + pitch;
	srcminus = inframe;
	srcplus = inframe + 2 * pitch;
	srcminusminus = inframe - 2 * pitch;
	srcplusplus = inframe + 4 * pitch;
	dst = outframe + pitch;

	for (y = 1; y < height - 1; y++) {
		if (~y&1) {
			for (x = 0; x < width; x += 8) {
				m0 = *(__m64*)(srcminus + x);
				m2 = *(__m64*)(srcplus + x);
				m1 = _m_punpckhbw(m0, mmx_null);
				m0 = _m_punpcklbw(m0, mmx_null);
				m3 = _m_punpckhbw(m2, mmx_null);
				m2 = _m_punpcklbw(m2, mmx_null);
				if ((y > 2) && (y < (height - 3))) {
                    m4 = *(__m64*)(srcminusminus+x);
					m5 = _m_punpckhbw(m4, mmx_null);
					m4 = _m_punpcklbw(m4, mmx_null);
					m6 = *(__m64*)(srcplusplus+x);
					m7 = _m_punpckhbw(m6, mmx_null);
					m6 = _m_punpcklbw(m6, mmx_null);
					m0 = _m_paddusw(m0, m2);
					m1 = _m_paddusw(m1, m3);
					m4 = _m_paddusw(m4, m6);
					m5 = _m_paddusw(m5, m7);
					m2 = _m_psllwi(m0, 2);
					m3 = _m_psllwi(m1, 2);
					m0 = _m_paddusw(m0, m2);
					m1 = _m_paddusw(m1, m3);
					m0 = _m_psubsw(m0, m4);
					m1 = _m_psubsw(m1, m5);
					m0 = _m_paddw(m0, mmx_add_shift4);
					m1 = _m_paddw(m1, mmx_add_shift4);
					m0 = _m_psrawi(m0, 3);
					m1 = _m_psrawi(m1, 3);
					m0 = _m_packuswb(m0, m1);
				} else {
					m0 = _m_paddusw(m0, m2);
					m1 = _m_paddusw(m1, m3);
					m0 = _m_paddusw(m0, mmx_add_shift1);
					m1 = _m_paddusw(m1, mmx_add_shift1);
					m0 = _m_psrawi(m0, 1);
					m1 = _m_psrawi(m1, 1);
					m0 = _m_packuswb(m0, m1);
				}
				*(__m64*)(dst+x) = m0;
			}
		} else {
			memcpy(dst, src, pitch);
		}
		src += pitch;
		srcminus += pitch;
		srcplus += pitch;
		srcminusminus += pitch;
		srcplusplus += pitch;
		dst += pitch;
	}
	memcpy(dst, src, pitch);

	_mm_empty();
}

/**
 **************************************************************************************************
 * Blend interpolate image. MMX/SSE implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [out]     outframe	 - pointer to destination image
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 **************************************************************************************************
 */
void VS_Deinterlacing::MMX_BlendInterpolate(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch)
{
	vs8u			*src, *dst, *srcminus;
	int				x, y;

	__m64 m0, m1;

	src = inframe;
	dst = outframe;
	memcpy(dst, src, pitch);
	src = inframe + pitch;
	srcminus = inframe;
	dst = outframe + pitch;
	for (y = 1; y < height; y++) {
		for (x = 0; x < width; x += 8) {
			m0 = *(__m64*)(srcminus + x);
			m1 = *(__m64*)(src + x);
			m0 = _m_pavgb(m0, m1);
			*(__m64*)(dst + x) = m0;
		}
		src += pitch;
		srcminus += pitch;
		dst += pitch;
	}

	_mm_empty();
}

/**
 **************************************************************************************************
 * Motion detecting, motion map filling. SSE2 implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [in]      prevframe	 - pointer to previous image
 * \param  [out]     motion		 - pointer to motion map
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 * \param  [in]      threshold 	 - threshold of motion for luma/chroma
 **************************************************************************************************
 */
void VS_Deinterlacing::SSE2_MotionDetected(unsigned char *inframe, unsigned char *prevframe, unsigned char *motion, int width, int height, int pitch, int threshold)
{
	unsigned char	*moving;
	vs8u			*prev, *src;
	int				x = 0, y = 0;
	int				W = width / 16 * 16;

	__m128i xmm0, xmm1, xmm2, xmm_acc,
			xmm_thr = _mm_set1_epi8(threshold),
			xmm_mask_7f = _mm_set1_epi8(0x7f),
			xmm_mask_01 = _mm_set1_epi8(0x1),
			xmm_mask_ff = _mm_set1_epi8(0xff),
			xmm_null = _mm_setzero_si128();

	src = inframe + pitch;
	prev = prevframe + pitch;
	moving = motion + width;

	for (y = 1; y < height - 1; y++) {
		xmm_acc = _mm_setzero_si128();
		for (x = 0; x < W; x += 16) {
			xmm0 = _mm_loadu_si128((const __m128i*)(src  + x));
			xmm1 = _mm_loadu_si128((const __m128i*)(prev + x));
			/// abs(src[x] - prev[x]) = delta_frame
			xmm2 = xmm0;
			xmm0 = _mm_subs_epu8(xmm0, xmm1);
			xmm1 = _mm_subs_epu8(xmm1, xmm2);
			xmm0 = _mm_or_si128(xmm0, xmm1);
			/// m_threshold_y - (delta_frame >> 1)
			xmm1 = xmm0;
			xmm1 = _mm_srai_epi16(xmm1, 1);
			xmm1 = _mm_and_si128(xmm1, xmm_mask_7f);
			xmm1 = _mm_subs_epu8(xmm_thr, xmm1);
			/// xmm0 ? xmm1
			xmm0 = _mm_subs_epu8(xmm0, xmm1);
			xmm0 = _mm_cmpeq_epi8(xmm0, xmm_null);
			xmm0 = _mm_xor_si128(xmm0, xmm_mask_ff);
			xmm0 = _mm_and_si128(xmm0, xmm_mask_01);
			_mm_storeu_si128((__m128i*)(moving + x), xmm0);
			xmm1 = xmm0;
			xmm0 = _mm_unpacklo_epi8(xmm0, xmm_null);
			xmm1 = _mm_unpackhi_epi8(xmm1, xmm_null);
			xmm_acc = _mm_adds_epu16(xmm_acc, xmm0);
			xmm_acc = _mm_adds_epu16(xmm_acc, xmm1);
		}
		xmm0 = xmm_acc;
		xmm0 = _mm_srli_si128(xmm0, 8);
		xmm_acc = _mm_adds_epu16(xmm_acc, xmm0);
		xmm0 = xmm_acc;
		xmm0 = _mm_srli_si128(xmm0, 4);
		xmm_acc = _mm_adds_epu16(xmm_acc, xmm0);
		xmm0 = xmm_acc;
		xmm0 = _mm_srli_si128(xmm0, 2);
		xmm_acc = _mm_adds_epu16(xmm_acc, xmm0);
		m_num_pnt_scene += (unsigned short)_mm_cvtsi128_si32(xmm_acc);
		src += pitch;
		prev += pitch;
		moving += width;
	}

	if (width - W >= 8) { /// MMX
		__m64 m0, m1, m2, m4, m5, m6, mmx_acc;
		const __m64 mmx_thr  = _mm_set1_pi16 (threshold),
					mmx_null = _mm_setzero_si64 (),
					mmx_mask = _mm_set1_pi16 (1);

		src = inframe + pitch;
		prev = prevframe + pitch;
		moving = motion + width;

		for (y = 1; y < height - 1; y++) {
			mmx_acc = _mm_setzero_si64 ();
			for (x = W; x < width; x += 8) {
				m0 = *(__m64*)(src+x);
				m1 = *(__m64*)(prev+x);
				/// abs(src[x] - prev[x]) = delta_frame
				m2 = _m_psubusb(m0, m1);
				m1 = _m_psubusb(m1, m0);
				m1 = _m_paddusb(m1, m2);
				m5 = _m_punpckhbw(m1, mmx_null);
				m6 = _m_punpcklbw(m1, mmx_null);
				m0 = m6;
				m4 = m5;
				/// m_threshold_y - (delta_frame >> 1)
				m5 = _m_psrawi(m5, 1);
				m6 = _m_psrawi(m6, 1);
				m5 = _m_psubusw(mmx_thr, m5);
				m6 = _m_psubusw(mmx_thr, m6);
				/// m0 ? m1
				m0 = _m_pcmpgtw(m0, m6);
				m4 = _m_pcmpgtw(m4, m5);
				m0 = _m_pand(m0, mmx_mask);
				m4 = _m_pand(m4, mmx_mask);
				mmx_acc = _m_paddusb(mmx_acc, m0); // scene change
				mmx_acc = _m_paddusb(mmx_acc, m4); // scene change
				*(__m64*)(moving+x) = _m_packuswb(m0, m4);
			}
			m0 = _m_psrlqi(mmx_acc, 32);
			mmx_acc = _m_paddusw(mmx_acc, m0);
			m0 = _m_psrlqi(mmx_acc, 16);
			mmx_acc = _m_paddusw(mmx_acc, m0);
			m_num_pnt_scene += (unsigned short)_m_to_int(mmx_acc);
			src += pitch;
			prev += pitch;
			moving += width;
		}
	}

	_mm_empty();
}

/**
 **************************************************************************************************
 * Noise reduction on motion map. Quality mode. SSE2 implementation.
 * \param  [in,out]  motion		 - pointer to motion map
 * \param  [in,out]  nrmotion	 - pointer to temporary array
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 **************************************************************************************************
 */
void VS_Deinterlacing::SSE2_NoiseReduction_Quality(unsigned char *motion, unsigned char *nrmotion, int width, int height, VS_VideoProc *VprUp, VS_VideoProc *VprDown)
{
	int x = 0, y = 0, u = 0, v = 0;
	vs8u *moving = motion, *nrmoving = nrmotion + 2 * width;
	int	W = width / 16 * 16;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4;
	const __m128i xmm_noise_threshold = _mm_set1_epi8(5),
				  xmm_noise_mask = _mm_set1_epi8(200);

	for (y = 2; y < height - 2; y++) {
		for (x = 2; x < W; x += 12) {
			xmm0 = _mm_loadu_si128((const __m128i*)(moving + x - 2));
			moving += width;
			xmm1 = _mm_loadu_si128((const __m128i*)(moving + x - 2));
			moving += width;
			xmm2 = _mm_loadu_si128((const __m128i*)(moving + x - 2));
			moving += width;
			xmm3 = _mm_loadu_si128((const __m128i*)(moving + x - 2));
			moving += width;
			xmm4 = _mm_loadu_si128((const __m128i*)(moving + x - 2));
			moving -= 4 * width;
			xmm0 = _mm_adds_epu8(xmm0, xmm1);
			xmm0 = _mm_adds_epu8(xmm0, xmm2);
			xmm0 = _mm_adds_epu8(xmm0, xmm3);
			xmm0 = _mm_adds_epu8(xmm0, xmm4);
			xmm1 = xmm0;
			xmm1 = _mm_srli_si128(xmm1, 1);
			xmm0 = _mm_adds_epu8(xmm0, xmm1);
			xmm1 = _mm_srli_si128(xmm1, 1);
			xmm0 = _mm_adds_epu8(xmm0, xmm1);
			xmm1 = _mm_srli_si128(xmm1, 1);
			xmm0 = _mm_adds_epu8(xmm0, xmm1);
			xmm1 = _mm_srli_si128(xmm1, 1);
			xmm0 = _mm_adds_epu8(xmm0, xmm1);
			xmm0 = _mm_cmpgt_epi8(xmm0, xmm_noise_threshold);
			xmm0 = _mm_and_si128(xmm0, xmm_noise_mask);
			*(int*)(nrmoving+x+0) = _mm_cvtsi128_si32(xmm0);
			xmm0 = _mm_srli_si128(xmm0, 4);
			*(int*)(nrmoving+x+4) = _mm_cvtsi128_si32(xmm0);
			xmm0 = _mm_srli_si128(xmm0, 4);
			*(int*)(nrmoving+x+8) = _mm_cvtsi128_si32(xmm0);
		}
		moving += width;
		nrmoving += width;
	}


	if (x < width) { /// MMX
		int st = x;

		__m64 m0, m1, m2, m3, m4, m5;
		const __m64 mmx_noise_threshold = _mm_set1_pi8 (5),
					mmx_noise_mask = _mm_set1_pi8 (200);

		moving = motion;
		nrmoving = nrmotion + 2 * width;

		for (y = 2; y < height - 2; y++) {
			for (x = st; x < width - 2; x += 8) {
				m0 = *(__m64*)(moving+x-2);
				moving += width;
				m1 = *(__m64*)(moving+x-2);
				moving += width;
				m2 = *(__m64*)(moving+x-2);
				moving += width;
				m3 = *(__m64*)(moving+x-2);
				moving += width;
				m4 = *(__m64*)(moving+x-2);
				m0 = _m_paddusb(m0, m1);
				m0 = _m_paddusb(m0, m2);
				m0 = _m_paddusb(m0, m3);
				m0 = _m_paddusb(m0, m4);
				m1 = m0;
				m1 = _m_psrlqi(m1, 8);
				m0 = _m_paddusb(m0, m1);
				m1 = _m_psrlqi(m1, 8);
				m0 = _m_paddusb(m0, m1);
				m1 = _m_psrlqi(m1, 8);
				m0 = _m_paddusb(m0, m1);
				m1 = _m_psrlqi(m1, 8);
				m0 = _m_paddusb(m0, m1);
				m5 = m0;
				moving -= 4 * width;

				m0 = *(__m64*)(moving+x+2);
				moving += width;
				m1 = *(__m64*)(moving+x+2);
				moving += width;
				m2 = *(__m64*)(moving+x+2);
				moving += width;
				m3 = *(__m64*)(moving+x+2);
				moving += width;
				m4 = *(__m64*)(moving+x+2);
				m0 = _m_paddusb(m0, m1);
				m0 = _m_paddusb(m0, m2);
				m0 = _m_paddusb(m0, m3);
				m0 = _m_paddusb(m0, m4);
				m1 = m0;
				m1 = _m_psrlqi(m1, 8);
				m0 = _m_paddusb(m0, m1);
				m1 = _m_psrlqi(m1, 8);
				m0 = _m_paddusb(m0, m1);
				m1 = _m_psrlqi(m1, 8);
				m0 = _m_paddusb(m0, m1);
				m1 = _m_psrlqi(m1, 8);
				m0 = _m_paddusb(m0, m1);
				moving -= 4 * width;

				m0 = _m_punpckldq(m5, m0);
				m0 = _m_pcmpgtb(m0, mmx_noise_threshold);
				m0 = _m_pand(m0, mmx_noise_mask);
				*(__m64*)(nrmoving+x) = m0;
			}
			moving += width;
			nrmoving += width;
		}

		_mm_empty();
	}

	moving = motion;
	nrmoving = nrmotion;
	VprDown->ResampleDown_8d1(nrmoving, m_moving_d, width, height);
	VprUp->ResampleRGB8(m_moving_d, moving, width / 8, height / 8, width, height, width);

	_mm_empty();
}

/**
 **************************************************************************************************
 * Interpolate moving areas (except when scene change). MMX implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [out]     outframe	 - pointer to destination image
 * \param  [in]      motion		 - pointer to motion map current image
 * \param  [in]      motion_prev - pointer to motion map previous image
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 **************************************************************************************************
 */
void VS_Deinterlacing::SSE2_InterpolateField(unsigned char *inframe, unsigned char *outframe, unsigned char *motion, unsigned char *motion_prev, int width, int height, int pitch)
{
	vs8u			*src, *dst, *srcminus, *srcplus, *srcminusminus, *srcplusplus;
	vs8u			*moving, *movingminus, *movingplus, *moving_prev;
	int				x, y;
	int				W = width / 16 * 16;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	const __m128i xmm_add_shift4 = _mm_set1_epi16 (4),
				  xmm_add_shift1 = _mm_set1_epi16 (1),
				  xmm_null = _mm_setzero_si128 (),
				  xmm_mask = _mm_set1_epi8 (0xff);

	src = inframe;
	dst = outframe;
	memcpy(dst, src, pitch);
	src = inframe + pitch;
	srcminus = inframe;
	srcplus = inframe + 2 * pitch;
	srcminusminus = inframe - 2 * pitch;
	srcplusplus = inframe + 4 * pitch;
	dst = outframe + pitch;
	moving = motion + width;
	movingminus = moving - width;
	movingplus = moving + width;
	moving_prev = motion_prev + width;
	for (y = 1; y < height - 1; y++) {
		if (~y&1) {
			for (x = 0; x < W; x += 16) {
				xmm0 = _mm_loadu_si128((const __m128i*)(srcminus + x));
				xmm2 = _mm_loadu_si128((const __m128i*)(srcplus + x));
				if ((y > 2) && (y < (height - 3))) {
					xmm1 = _mm_unpackhi_epi8(xmm0, xmm_null);
					xmm0 = _mm_unpacklo_epi8(xmm0, xmm_null);
					xmm3 = _mm_unpackhi_epi8(xmm2, xmm_null);
					xmm2 = _mm_unpacklo_epi8(xmm2, xmm_null);
					xmm4 = _mm_loadu_si128((const __m128i*)(srcminusminus + x));
					xmm5 = _mm_unpackhi_epi8(xmm4, xmm_null);
					xmm4 = _mm_unpacklo_epi8(xmm4, xmm_null);
					xmm6 = _mm_loadu_si128((const __m128i*)(srcplusplus + x));
					xmm7 = _mm_unpackhi_epi8(xmm6, xmm_null);
					xmm6 = _mm_unpacklo_epi8(xmm6, xmm_null);
					xmm0 = _mm_adds_epu16(xmm0, xmm2);
					xmm1 = _mm_adds_epu16(xmm1, xmm3);
					xmm4 = _mm_adds_epu16(xmm4, xmm6);
					xmm5 = _mm_adds_epu16(xmm5, xmm7);
					xmm2 = _mm_slli_epi16(xmm0, 2);
					xmm3 = _mm_slli_epi16(xmm1, 2);
					xmm0 = _mm_adds_epu16(xmm0, xmm2);
					xmm1 = _mm_adds_epu16(xmm1, xmm3);
					xmm0 = _mm_subs_epi16(xmm0, xmm4);
					xmm1 = _mm_subs_epi16(xmm1, xmm5);
					xmm0 = _mm_add_epi16(xmm0, xmm_add_shift4);
					xmm1 = _mm_add_epi16(xmm1, xmm_add_shift4);
					xmm0 = _mm_srai_epi16(xmm0, 3);
					xmm1 = _mm_srai_epi16(xmm1, 3);
					xmm0 = _mm_packus_epi16(xmm0, xmm1);
				} else {
					xmm0 = _mm_avg_epu8(xmm0, xmm2);
				}
				xmm2 = _mm_loadu_si128((const __m128i*)(src + x));
				xmm3 = _mm_loadu_si128((const __m128i*)(moving + x));
				xmm4 = _mm_loadu_si128((const __m128i*)(movingminus + x));
				xmm5 = _mm_loadu_si128((const __m128i*)(movingplus + x));
				xmm6 = _mm_loadu_si128((const __m128i*)(moving_prev + x));
				xmm3 = _mm_or_si128(xmm3, xmm4);
				xmm3 = _mm_or_si128(xmm3, xmm5);
				xmm3 = _mm_or_si128(xmm3, xmm6);
				xmm3 = _mm_cmpeq_epi8(xmm3, xmm_null);
				xmm3 = _mm_xor_si128(xmm3, xmm_mask);
				xmm0 = _mm_and_si128(xmm0, xmm3);
				xmm2 = _mm_andnot_si128(xmm3, xmm2);
				xmm0 = _mm_or_si128(xmm0, xmm2);
				_mm_storeu_si128((__m128i*)(dst + x), xmm0);
			}
		} else {
			memcpy(dst, src, pitch);
		}
		src += pitch;
		srcminus += pitch;
		srcplus += pitch;
		srcminusminus += pitch;
		srcplusplus += pitch;
		dst += pitch;
		moving += width;
		movingminus += width;
		movingplus += width;
		moving_prev += width;
	}
	memcpy(dst, src, pitch);

	if (width - W >= 8) { /// MMX

		__m64 m0, m1, m2, m3, m4, m5, m6, m7;
		const __m64 mmx_add_shift4 = _mm_set1_pi16 (4),
					mmx_add_shift1 = _mm_set1_pi16 (1),
					mmx_null = _mm_setzero_si64 (),
					mmx_mask = _mm_set1_pi8 (0xff);

		src = inframe + pitch;
		srcminus = inframe;
		srcplus = inframe + 2 * pitch;
		srcminusminus = inframe - 2 * pitch;
		srcplusplus = inframe + 4 * pitch;
		dst = outframe + pitch;
		moving = motion + width;
		movingminus = moving - width;
		movingplus = moving + width;
		moving_prev = motion_prev + width;
		for (y = 1; y < height - 1; y++) {
			if (~y&1) {
				for (x = W; x < width; x += 8) {
					m0 = *(__m64*)(srcminus + x);
					m2 = *(__m64*)(srcplus + x);
					m1 = _m_punpckhbw(m0, mmx_null);
					m0 = _m_punpcklbw(m0, mmx_null);
					m3 = _m_punpckhbw(m2, mmx_null);
					m2 = _m_punpcklbw(m2, mmx_null);
					if ((y > 2) && (y < (height - 3))) {
						m4 = *(__m64*)(srcminusminus+x);
						m5 = _m_punpckhbw(m4, mmx_null);
						m4 = _m_punpcklbw(m4, mmx_null);
						m6 = *(__m64*)(srcplusplus+x);
						m7 = _m_punpckhbw(m6, mmx_null);
						m6 = _m_punpcklbw(m6, mmx_null);
						m0 = _m_paddusw(m0, m2);
						m1 = _m_paddusw(m1, m3);
						m4 = _m_paddusw(m4, m6);
						m5 = _m_paddusw(m5, m7);
						m2 = _m_psllwi(m0, 2);
						m3 = _m_psllwi(m1, 2);
						m0 = _m_paddusw(m0, m2);
						m1 = _m_paddusw(m1, m3);
						m0 = _m_psubsw(m0, m4);
						m1 = _m_psubsw(m1, m5);
						m0 = _m_paddw(m0, mmx_add_shift4);
						m1 = _m_paddw(m1, mmx_add_shift4);
						m0 = _m_psrawi(m0, 3);
						m1 = _m_psrawi(m1, 3);
						m0 = _m_packuswb(m0, m1);
					} else {
						m0 = _m_paddusw(m0, m2);
						m1 = _m_paddusw(m1, m3);
						m0 = _m_paddusw(m0, mmx_add_shift1);
						m1 = _m_paddusw(m1, mmx_add_shift1);
						m0 = _m_psrawi(m0, 1);
						m1 = _m_psrawi(m1, 1);
						m0 = _m_packuswb(m0, m1);
					}
					m2 = *(__m64*)(src + x);
					m3 = *(__m64*)(moving + x);
					m4 = *(__m64*)(movingminus + x);
					m5 = *(__m64*)(movingplus + x);
					m6 = *(__m64*)(moving_prev + x);
					m3 = _m_por(m3, m4);
					m3 = _m_por(m3, m5);
					m3 = _m_por(m3, m6);
					m3 = _m_pcmpeqb(m3, mmx_null);
					m3 = _m_pxor(m3, mmx_mask );
					m0 = _m_pand(m0, m3);
					m2 = _m_pandn(m3, m2);
					m0 = _m_por(m0, m2);
					*(__m64*)(dst+x) = m0;
				}
			}
			src += pitch;
			srcminus += pitch;
			srcplus += pitch;
			srcminusminus += pitch;
			srcplusplus += pitch;
			dst += pitch;
			moving += width;
			movingminus += width;
			movingplus += width;
			moving_prev += width;
		}
	}

	_mm_empty();
}

/**
 **************************************************************************************************
 * Scene change interpolate image. SSE2 implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [out]     outframe	 - pointer to destination image
 * \param  [in]      motion		 - pointer to motion map current image
 * \param  [in]      motion_prev - pointer to motion map previous image
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 **************************************************************************************************
 */
void VS_Deinterlacing::SSE2_InterpolateField_SceneChange(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch)
{
	vs8u			*src, *dst, *srcminus, *srcplus, *srcminusminus, *srcplusplus;
	int				x, y;
	int				W = width / 16 * 16;

	__m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
	const __m128i xmm_add_shift4 = _mm_set1_epi16 (4),
				  xmm_add_shift1 = _mm_set1_epi16 (1),
				  xmm_null = _mm_setzero_si128 (),
				  xmm_mask = _mm_set1_epi8 (0xff);

	src = inframe;
	dst = outframe;
	memcpy(dst, src, pitch);
	src = inframe + pitch;
	srcminus = inframe;
	srcplus = inframe + 2 * pitch;
	srcminusminus = inframe - 2 * pitch;
	srcplusplus = inframe + 4 * pitch;
	dst = outframe + pitch;
	for (y = 1; y < height - 1; y++) {
		if (~y&1) {
			for (x = 0; x < W; x += 16) {
				xmm0 = _mm_loadu_si128((const __m128i*)(srcminus + x));
				xmm2 = _mm_loadu_si128((const __m128i*)(srcplus + x));
				if ((y > 2) && (y < (height - 3))) {
					xmm1 = _mm_unpackhi_epi8(xmm0, xmm_null);
					xmm0 = _mm_unpacklo_epi8(xmm0, xmm_null);
					xmm3 = _mm_unpackhi_epi8(xmm2, xmm_null);
					xmm2 = _mm_unpacklo_epi8(xmm2, xmm_null);
					xmm4 = _mm_loadu_si128((const __m128i*)(srcminusminus + x));
					xmm5 = _mm_unpackhi_epi8(xmm4, xmm_null);
					xmm4 = _mm_unpacklo_epi8(xmm4, xmm_null);
					xmm6 = _mm_loadu_si128((const __m128i*)(srcplusplus + x));
					xmm7 = _mm_unpackhi_epi8(xmm6, xmm_null);
					xmm6 = _mm_unpacklo_epi8(xmm6, xmm_null);
					xmm0 = _mm_adds_epu16(xmm0, xmm2);
					xmm1 = _mm_adds_epu16(xmm1, xmm3);
					xmm4 = _mm_adds_epu16(xmm4, xmm6);
					xmm5 = _mm_adds_epu16(xmm5, xmm7);
					xmm2 = _mm_slli_epi16(xmm0, 2);
					xmm3 = _mm_slli_epi16(xmm1, 2);
					xmm0 = _mm_adds_epu16(xmm0, xmm2);
					xmm1 = _mm_adds_epu16(xmm1, xmm3);
					xmm0 = _mm_subs_epi16(xmm0, xmm4);
					xmm1 = _mm_subs_epi16(xmm1, xmm5);
					xmm0 = _mm_add_epi16(xmm0, xmm_add_shift4);
					xmm1 = _mm_add_epi16(xmm1, xmm_add_shift4);
					xmm0 = _mm_srai_epi16(xmm0, 3);
					xmm1 = _mm_srai_epi16(xmm1, 3);
					xmm0 = _mm_packus_epi16(xmm0, xmm1);
				} else {
					xmm0 = _mm_avg_epu8(xmm0, xmm2);
				}
				_mm_storeu_si128((__m128i*)(dst + x), xmm0);
			}
		} else {
			memcpy(dst, src, pitch);
		}
		src += pitch;
		srcminus += pitch;
		srcplus += pitch;
		srcminusminus += pitch;
		srcplusplus += pitch;
		dst += pitch;
	}
	memcpy(dst, src, pitch);

	if (width - W >= 8) { /// MMX

		__m64 m0, m1, m2, m3, m4, m5, m6, m7;
		const __m64 mmx_add_shift4 = _mm_set1_pi16 (4),
					mmx_add_shift1 = _mm_set1_pi16 (1),
					mmx_null = _mm_setzero_si64 (),
					mmx_mask = _mm_set1_pi8 (0xff);

		src = inframe + pitch;
		srcminus = inframe;
		srcplus = inframe + 2 * pitch;
		srcminusminus = inframe - 2 * pitch;
		srcplusplus = inframe + 4 * pitch;
		dst = outframe + pitch;
		for (y = 1; y < height - 1; y++) {
			if (~y&1) {
				for (x = W; x < width; x += 8) {
					m0 = *(__m64*)(srcminus + x);
					m2 = *(__m64*)(srcplus + x);
					m1 = _m_punpckhbw(m0, mmx_null);
					m0 = _m_punpcklbw(m0, mmx_null);
					m3 = _m_punpckhbw(m2, mmx_null);
					m2 = _m_punpcklbw(m2, mmx_null);
					if ((y > 2) && (y < (height - 3))) {
						m4 = *(__m64*)(srcminusminus+x);
						m5 = _m_punpckhbw(m4, mmx_null);
						m4 = _m_punpcklbw(m4, mmx_null);
						m6 = *(__m64*)(srcplusplus+x);
						m7 = _m_punpckhbw(m6, mmx_null);
						m6 = _m_punpcklbw(m6, mmx_null);
						m0 = _m_paddusw(m0, m2);
						m1 = _m_paddusw(m1, m3);
						m4 = _m_paddusw(m4, m6);
						m5 = _m_paddusw(m5, m7);
						m2 = _m_psllwi(m0, 2);
						m3 = _m_psllwi(m1, 2);
						m0 = _m_paddusw(m0, m2);
						m1 = _m_paddusw(m1, m3);
						m0 = _m_psubsw(m0, m4);
						m1 = _m_psubsw(m1, m5);
						m0 = _m_paddw(m0, mmx_add_shift4);
						m1 = _m_paddw(m1, mmx_add_shift4);
						m0 = _m_psrawi(m0, 3);
						m1 = _m_psrawi(m1, 3);
						m0 = _m_packuswb(m0, m1);
					} else {
						m0 = _m_paddusw(m0, m2);
						m1 = _m_paddusw(m1, m3);
						m0 = _m_paddusw(m0, mmx_add_shift1);
						m1 = _m_paddusw(m1, mmx_add_shift1);
						m0 = _m_psrawi(m0, 1);
						m1 = _m_psrawi(m1, 1);
						m0 = _m_packuswb(m0, m1);
					}
					*(__m64*)(dst+x) = m0;
				}
			}
			src += pitch;
			srcminus += pitch;
			srcplus += pitch;
			srcminusminus += pitch;
			srcplusplus += pitch;
			dst += pitch;
		}
	}

	_mm_empty();
}

/**
 **************************************************************************************************
 * Blend interpolate image. SSE2 implementation.
 * \param  [in]      inframe	 - pointer to source image
 * \param  [out]     outframe	 - pointer to destination image
 * \param  [in]      width		 - number of pixels in source plane (luma/chroma) on X direction
 * \param  [in]      height		 - number of pixels in source plane (luma/chroma) on Y direction
 * \param  [in]      pitch		 - luma/chroma plane offset
 **************************************************************************************************
 */
void VS_Deinterlacing::SSE2_BlendInterpolate(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch)
{
	vs8u			*src, *dst, *srcminus;
	int				x, y;
	int				width16;

	__m128i xmm0, xmm1;
	__m64 m0, m1;

	width16 = width / 16 * 16;
	src = inframe;
	dst = outframe;
	memcpy(dst, src, pitch);
	src = inframe + pitch;
	srcminus = inframe;
	dst = outframe + pitch;
	for (y = 1; y < height; y++) {
		for (x = 0; x < width16; x += 16) {
			xmm0 = _mm_loadu_si128((const __m128i*)(srcminus + x));
			xmm1 = _mm_loadu_si128((const __m128i*)(src + x));
			xmm0 = _mm_avg_epu8(xmm0, xmm1);
			_mm_storeu_si128((__m128i*)(dst + x), xmm0);
		}
		for (; x < width; x += 8) {
			m0 = *(__m64*)(srcminus + x);
			m1 = *(__m64*)(src + x);
			m0 = _m_pavgb(m0, m1);
			*(__m64*)(dst + x) = m0;
		}
		src += pitch;
		srcminus += pitch;
		dst += pitch;
	}

	_mm_empty();
}

/**
 **************************************************************************************************
 * Main deinterlacing function.
 * \param  [in]			inframe			- source image
 * \param  [out]		outframe		- destination image
 * \return	VS_NOERR if no error occurred
 **************************************************************************************************
 */
VS_DERROR VS_Deinterlacing::Process(unsigned char *inframe, unsigned char *outframe)
{
	if (!m_isInit) return VS_NOTINIT;

#ifdef TEST_TIME
	LARGE_INTEGER start_t, end_t;

	start_t.QuadPart = 0;
	end_t.QuadPart = 0;
	QueryPerformanceCounter(&start_t);
#endif

	if (m_filterType == QUALITY) {

		m_num_pnt_scene = 0;
		memcpy(m_moving_prev, m_moving, m_square);
		memcpy(m_moving_prev_u, m_moving_u, m_square>>2);
		memcpy(m_moving_prev_v, m_moving_v, m_square>>2);

#ifdef TEST_TIME
		QueryPerformanceCounter(&end_t);
		time_memory.QuadPart += (end_t.QuadPart - start_t.QuadPart);

		start_t.QuadPart = 0;
		end_t.QuadPart = 0;
		QueryPerformanceCounter(&start_t);
#endif

		(this->*MotionDetected)(inframe, m_prevFrame, m_moving, m_w, m_h, m_pitch, m_threshold_y); // Y

#ifdef TEST_TIME
		QueryPerformanceCounter(&end_t);
		time_motion.QuadPart += (end_t.QuadPart - start_t.QuadPart);
#endif

#ifdef TEST_TIME
		start_t.QuadPart = 0;
		end_t.QuadPart = 0;
		QueryPerformanceCounter(&start_t);
#endif

		int num = (m_num_pnt_scene * 100) / m_square;
		if (num < m_scenethreshold) {  // not scene change
			(this->*NoiseReduction)(m_moving, m_nrmoving, m_w, m_h, m_VProcY_up, m_VProcY_down);

#ifdef TEST_TIME
			QueryPerformanceCounter(&end_t);
			time_noiser.QuadPart += (end_t.QuadPart - start_t.QuadPart);
#endif

#ifdef TEST_TIME
			start_t.QuadPart = 0;
			end_t.QuadPart = 0;
			QueryPerformanceCounter(&start_t);
#endif

			m_num_pnt_scene = 0;
			(this->*MotionDetected)(inframe + m_square, m_prevFrame + m_square, m_moving_u, m_w2, m_h2, m_pitch2, m_threshold_uv); // U
			m_num_pnt_scene = 0;
			(this->*MotionDetected)(inframe + 5 * m_square / 4, m_prevFrame + 5 * m_square / 4, m_moving_v, m_w2, m_h2, m_pitch2, m_threshold_uv); // V

#ifdef TEST_TIME
			QueryPerformanceCounter(&end_t);
			time_motion.QuadPart += (end_t.QuadPart - start_t.QuadPart);
#endif

			memcpy(m_prevFrame, inframe, (m_pitch + m_pitch2) * m_h);

#ifdef TEST_TIME
			start_t.QuadPart = 0;
			end_t.QuadPart = 0;
			QueryPerformanceCounter(&start_t);
#endif

			(this->*NoiseReduction)(m_moving_u, m_moving_prev_u, m_w2, m_h2, m_VProcUV_up, m_VProcUV_down); // U
			(this->*NoiseReduction)(m_moving_v, m_moving_prev_v, m_w2, m_h2, m_VProcUV_up, m_VProcUV_down); // V

#ifdef MOTION_MAP
		}
#endif

#ifdef TEST_TIME
			QueryPerformanceCounter(&end_t);
			time_noiser.QuadPart += (end_t.QuadPart - start_t.QuadPart);
#endif

#ifndef MOTION_MAP

#ifdef TEST_TIME
			start_t.QuadPart = 0;
			end_t.QuadPart = 0;
			QueryPerformanceCounter(&start_t);
#endif

			(this->*InterpolateField)(inframe, outframe, m_moving, m_moving_prev, m_w, m_h, m_pitch); // Y
			(this->*InterpolateField)(inframe + m_square, outframe + m_square, m_moving_u, m_moving_prev_u, m_w2, m_h2, m_pitch2); // U
			(this->*InterpolateField)(inframe + 5 * m_square / 4, outframe + 5 * m_square / 4, m_moving_v, m_moving_prev_v, m_w2, m_h2, m_pitch2); // V

#ifdef TEST_TIME
			QueryPerformanceCounter(&end_t);
			time_bicubic.QuadPart += (end_t.QuadPart - start_t.QuadPart);
#endif

		} else { // scene change

#ifdef TEST_TIME
			start_t.QuadPart = 0;
			end_t.QuadPart = 0;
			QueryPerformanceCounter(&start_t);
#endif

			memset(m_moving, 0, m_square);
			memset(m_moving_u, 0, m_square>>2);
			memset(m_moving_v, 0, m_square>>2);
			memset(m_nrmoving, 0, m_square);
			memcpy(m_prevFrame, inframe, (m_pitch + m_pitch2) * m_h);

#ifdef TEST_TIME
			QueryPerformanceCounter(&end_t);
			time_memory.QuadPart += (end_t.QuadPart - start_t.QuadPart);

			start_t.QuadPart = 0;
			end_t.QuadPart = 0;
			QueryPerformanceCounter(&start_t);
#endif

			(this->*InterpolateField_SceneChange)(inframe, outframe, m_w, m_h, m_pitch); // Y
			(this->*InterpolateField_SceneChange)(inframe + m_square, outframe + m_square, m_w2, m_h2, m_pitch2); // U
			(this->*InterpolateField_SceneChange)(inframe + 5 * m_square / 4, outframe + 5 * m_square / 4, m_w2, m_h2, m_pitch2); // V

#ifdef TEST_TIME
			QueryPerformanceCounter(&end_t);
			time_bicubic.QuadPart += (end_t.QuadPart - start_t.QuadPart);
#endif

		}

#else

		memcpy(m_prevFrame, inframe, (m_pitch + m_pitch2) * m_h);
		int x = 0, y = 0;
		for (y = 0; y < m_h; y++) {
			for (x = 0; x < m_w; x++) {
				//if (m_moving[y*m_w+x]) m_moving[y*m_w+x] = 255;
				 //m_moving[y*m_w+x] = 0;
			}
		}
		for (y = 0; y < m_h/2; y++) {
			for (x = 0; x < m_w/2; x++) {
				//if (m_moving_u[y*m_w/2+x]) m_moving_u[y*m_w/2+x] = 255;
				//if (m_moving_u[y*m_w/2+x]) m_moving_u[y*m_w/2+x] = 255;
				if (m_moving_u[y*m_w/2+x] == 0) m_moving_u[y*m_w/2+x] = 0x80;
				//if (m_moving_v[y*m_w/2+x]) m_moving_v[y*m_w/2+x] = 255;
				if (m_moving_v[y*m_w/2+x] == 0) m_moving_v[y*m_w/2+x] = 0x80;
			}
		}
		memcpy(outframe, m_moving, m_square);
		memcpy(outframe + m_square, m_moving_u, m_square/4);
		memcpy(outframe + m_square + m_square/4, m_moving_v, m_square/4);

#endif

	} else if (m_filterType == BLEND) {

		(this->*BlendInterpolate)(inframe, outframe, m_w, m_h, m_pitch); // Y
		(this->*BlendInterpolate)(inframe + m_square, outframe + m_square, m_w2, m_h2, m_pitch2); // U
		(this->*BlendInterpolate)(inframe + 5 * m_square / 4, outframe + 5 * m_square / 4, m_w2, m_h2, m_pitch2); // V

#ifdef TEST_TIME
		QueryPerformanceCounter(&end_t);
		time_bicubic.QuadPart += (end_t.QuadPart - start_t.QuadPart);
#endif

	}

	return VS_NOERR;
}

/**
 **************************************************************************************************
 * Deallocate internal resorses.
 **************************************************************************************************
 */
void VS_Deinterlacing::Release()
{
#ifdef TEST_TIME
	if (m_isInit) {
		double time = time_motion.QuadPart / double(frequency.QuadPart);
		printf("\nAverage time detecting motion: t = %f ms\n", time * 1000);
		time = time_noiser.QuadPart / double(frequency.QuadPart);
		printf("Average time noise reduction: t = %f ms\n", time * 1000);
		time = time_bicubic.QuadPart / double(frequency.QuadPart);
		printf("Average time interpolation: t = %f ms\n", time * 1000);
		time = (time_bicubic.QuadPart + time_noiser.QuadPart + time_motion.QuadPart + time_memory.QuadPart) / double(frequency.QuadPart);
		printf("Average time deinterlacing: t = %f ms\n", time * 1000);
	}
#endif
	m_isInit = false;
	m_w = 0;
	m_h = 0;
	m_pitch = 0;
	m_square = 0;
	m_w2 = 0;
	m_h2 = 0;
	m_pitch2 = 0;
	m_hminus1 = 0;
	m_hminus3 = 0;
	m_wminus1 = 0;
	m_num_pnt_scene = 0;
	FREE_ALIGN(m_prevFrame);
	FREE_ALIGN(m_moving);
	FREE_ALIGN(m_moving_prev);
	FREE_ALIGN(m_moving_u);
	FREE_ALIGN(m_moving_prev_u);
	FREE_ALIGN(m_moving_v);
	FREE_ALIGN(m_moving_prev_v);
	FREE_ALIGN(m_nrmoving);
	FREE_ALIGN(m_moving_d);
	if (m_VProcY_up) delete m_VProcY_up; m_VProcY_up = 0;
	if (m_VProcY_down) delete m_VProcY_down; m_VProcY_down = 0;
	if (m_VProcUV_up) delete m_VProcUV_up; m_VProcUV_up = 0;
	if (m_VProcUV_down) delete m_VProcUV_down; m_VProcUV_down = 0;
}

#pragma warning(default:4309)
