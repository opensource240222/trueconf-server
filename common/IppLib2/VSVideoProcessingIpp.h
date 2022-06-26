#pragma once

#include "ipp.h"
#include <cstdint>

class VSVideoProcessingIpp
{
public:
	VSVideoProcessingIpp();
	~VSVideoProcessingIpp();

	bool ConvertYUY2ToI420(uint8_t* src, uint8_t* dstY, uint8_t* dstU, uint8_t *dstV, int srcW, int h, int dstW);
	bool ConvertUYVYToI420(uint8_t* src, uint8_t* dstY, uint8_t* dstU, uint8_t *dstV, int srcW, int h, int dstW);
	bool ConvertBMF24ToI420(uint8_t* src, uint8_t* dstY, uint8_t* dstU, uint8_t *dstV, int srcW, int h, int dstW);
	bool ConvertBMF32ToI420(uint8_t* src, uint8_t* dstY, uint8_t* dstU, uint8_t *dstV, int srcW, int h, int dstW);
	bool ConvertRGB24ToI420(uint8_t* src, uint8_t* dstY, uint8_t* dstU, uint8_t *dstV, int srcW, int h, int dstW);
	bool ConvertNV12ToI420(uint8_t* src, uint8_t* dstY, uint8_t* dstU, uint8_t *dstV, int srcW, int h, int dstW);
	bool ConvertI420ToBMF24(uint8_t* srcY, uint8_t* srcU, uint8_t* srcV, uint8_t *dst, int srcW, int h, int dstW);

	bool MirrorI420(uint8_t* src, uint8_t* dst, int srcW, int srcH, int mode);

	bool ResampleI420(
		uint8_t* src, int srcW, int srcH,
		uint8_t *dst, int dstW, int dstH);

	bool ResampleCropI420(
		uint8_t* pSrc[3], uint8_t* pDst[3],
		int srcW, int srcH, int srcStep,
		int dstW, int dstH, int dstStep,
		int srcWR, int srcHR,
		int srcOffsetW, int srcOffsetH,
		double factorW, double factorH,
		int mode);

	bool ResampleInscribedI420(
		uint8_t* pSrc[3], uint8_t* pDst[3],
		int srcW, int srcH, int srcStep,
		int dstW, int dstH, int dstStep,
		int srcOffsetW, int srcOffsetH,
		double factorW, double factorH,
		int mode);

	bool ResampleRGB(
		const uint8_t* src, int srcW, int srcPitch, int srcH,
		uint8_t* dst, int dstW, int dstPitch, int dstH);

private:
	IppiInterpolationType lastInterType;
	IppiSize lastSrcSize;
	IppiSize lastDstSize;

	int specSize;
	int initBufSize;
	int workBufferSize;

	IppiResizeSpec_32f* pSpec;
	Ipp8u* pInitBuf;
	Ipp8u* pWorkBuf;

	IppStatus IppResizePlane1Channel(
		const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
		Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
		double xFactor, double yFactor,
		int interpolation);

	IppStatus IppResizePlane3Channel(
		const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
		Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
		double xFactor, double yFactor,
		int interpolation);
};
