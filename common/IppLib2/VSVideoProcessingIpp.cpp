
#include "VSVideoProcessingIpp.h"
#include "IppLib2/libinit.h"

#include <cmath>
#include <cstring>

VSVideoProcessingIpp::VSVideoProcessingIpp()
{
	IppLibInit();

	specSize = 0;
	initBufSize = 0;
	workBufferSize = 0;

	pSpec = nullptr;
	pInitBuf = nullptr;
	pWorkBuf = nullptr;

	lastInterType = ippNearest;
	lastSrcSize = { 0, 0 };
	lastDstSize = { 0, 0 };
}

VSVideoProcessingIpp::~VSVideoProcessingIpp()
{
	ippFree(pSpec);
	ippFree(pInitBuf);
	ippFree(pWorkBuf);
}

bool VSVideoProcessingIpp::ConvertBMF24ToI420(uint8_t* Src, uint8_t* DstY, uint8_t* DstU, uint8_t *DstV,
	int srcW, int h, int dstW)
{
	if (dstW & 0x1 || h & 0x1) return false;
	IppStatus st = ippStsNoErr;
	IppiSize roi = { dstW, h };
	int dst_step[3] = { dstW, dstW >> 1, dstW >> 1 };
	Ipp8u *pDst[3] = { DstY, DstV, DstU };

	st = ippiRGBToYCbCr420_8u_C3P3R(Src, srcW, pDst, dst_step, roi);

	ippiMirror_8u_C1IR(pDst[0], dst_step[0], { dstW, h }, ippAxsHorizontal);
	ippiMirror_8u_C1IR(pDst[1], dst_step[1], { dstW / 2, h / 2 }, ippAxsHorizontal);
	ippiMirror_8u_C1IR(pDst[2], dst_step[2], { dstW / 2, h / 2 }, ippAxsHorizontal);

	return (st == ippStsNoErr);
}

bool VSVideoProcessingIpp::ConvertRGB24ToI420(uint8_t* Src, uint8_t* DstY, uint8_t* DstU, uint8_t *DstV,
	int srcW, int h, int dstW)
{
	if (dstW & 0x1 || h & 0x1) return false;
	IppStatus st = ippStsNoErr;
	IppiSize roi = { dstW, h };
	int dst_step[3] = { dstW, dstW >> 1, dstW >> 1 };
	Ipp8u *pDst[3] = { DstY, DstV, DstU };

	// Artem Boldarev (08.11.2018):
	//
	// This might seem misleading, so it needs some further explanation.
	//
	// This method is added to aid in converting slide images into I420.
	// The  source data is provided by 'stb_image' library which returns
	// image data as an array of RGB(A) values with no padding bytes.
	//
	// For some obscure reason, using 'ippiRGBToYCbCr420*' procedure produces result where
	// 'red' and 'blue' colour components are swapped.
	// It seems that IPP uses somewhat different terminology here which is, probably,
	// influenced by Windows although I am not sure about the latter.
	st = ippiBGRToYCbCr420_8u_C3P3R(Src, srcW, pDst, dst_step, roi);

	ippiMirror_8u_C1IR(pDst[0], dst_step[0], { dstW, h }, ippAxsHorizontal);
	ippiMirror_8u_C1IR(pDst[1], dst_step[1], { dstW / 2, h / 2 }, ippAxsHorizontal);
	ippiMirror_8u_C1IR(pDst[2], dst_step[2], { dstW / 2, h / 2 }, ippAxsHorizontal);

	return (st == ippStsNoErr);
}


/**
 ******************************************************************************
 * Convert image from BMF 32 (Windows RGB 32 bit) to I420. IPP version.
 * \return none
 *
 * \param Src			[in]  - input rgb image
 * \param DstY			[out] - Y component of output image
 * \param DstU			[out] - U component of output image
 * \param DstV			[out] - V component of output image
 * \param srcW			[in]  - width in uint8_ts of input image
 * \param h				[in]  - height in lines of input image
 * \param dstW			[in]  - width in uint8_ts of output image
 *
 *  \date    31-06-2002
 ******************************************************************************
 */
bool VSVideoProcessingIpp::ConvertBMF32ToI420(uint8_t* Src, uint8_t* DstY, uint8_t* DstU, uint8_t *DstV,
	int srcW, int h, int dstW)
{
	if (dstW & 0x1 || h & 0x1) return false;
	IppStatus st = ippStsNoErr;
	IppiSize roi = { dstW, h };
	int dst_step[3] = { dstW, dstW >> 1, dstW >> 1 };
	Ipp8u *pDst[3] = { DstY, DstU, DstV };
	ippiMirror_8u_C4IR(Src, srcW, roi, ippAxsHorizontal);
	st = ippiRGBToYCrCb420_8u_AC4P3R(Src, srcW, pDst, dst_step, roi);
	return (st == ippStsNoErr);
}

/**
 ******************************************************************************
 * Convert image from YUY2 to YUV(4:2:0) format. IPP version.
 * \return true if all OK or false in case of bad srcWidth.
 *
 * \note dstWidth must be multiple of 2
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
bool VSVideoProcessingIpp::ConvertYUY2ToI420(uint8_t* src, uint8_t* dstY, uint8_t* dstU, uint8_t *dstV,
	int srcWidth, int height, int dstWidth)
{
	if (dstWidth & 0x1 || height & 0x1) return false;
	IppStatus st = ippStsNoErr;
	IppiSize roi = { dstWidth, height };
	int dst_step[3] = { dstWidth, dstWidth >> 1, dstWidth >> 1 };
	Ipp8u *pDst[3] = { dstY, dstV, dstU };
	st = ippiYCrCb422ToYCbCr420_8u_C2P3R(src, srcWidth, pDst, dst_step, roi);
	return (st == ippStsNoErr);
}

/**
 ******************************************************************************
 * Convert image from UYVY to YUV(4:2:0) format. IPP version.
 * \return true if all OK or false in case of bad srcWidth.
 *
 * \note dstWidth must be multiple of 2
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
bool VSVideoProcessingIpp::ConvertUYVYToI420(uint8_t* src, uint8_t* dstY, uint8_t* dstU, uint8_t *dstV,
	int srcWidth, int height, int dstWidth)
{
	if (dstWidth & 0x1 || height & 0x1) return false;
	IppStatus st = ippStsNoErr;
	IppiSize roi = { dstWidth, height };
	int dst_step[3] = { dstWidth, dstWidth >> 1, dstWidth >> 1 };
	Ipp8u *pDst[3] = { dstY, dstU, dstV };
	st = ippiCbYCr422ToYCbCr420_8u_C2P3R(src, srcWidth, pDst, dst_step, roi);
	return (st == ippStsNoErr);
}

/**
 ******************************************************************************
 * Convert image from NV12 to YUV(4:2:0) format. IPP version.
 * \return true if all OK or false in case of bad srcWidth.
 *
 * \note dstWidth must be multiple of 2
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
bool VSVideoProcessingIpp::ConvertNV12ToI420(uint8_t* src, uint8_t* dstY, uint8_t* dstU, uint8_t *dstV,
	int srcWidth, int height, int dstWidth)
{
	if (dstWidth & 0x1 || height & 0x1) return false;
	IppStatus st = ippStsNoErr;
	IppiSize roi = { dstWidth, height };
	int dst_step[3] = { dstWidth, dstWidth >> 1, dstWidth >> 1 };
	Ipp8u *pDst[3] = { dstY, dstU, dstV };
	ippiYCbCr420_8u_P2P3R(src, srcWidth, src + srcWidth * height, srcWidth, pDst, dst_step, roi);
	return (st == ippStsNoErr);
}

 bool VSVideoProcessingIpp::ConvertI420ToBMF24(uint8_t * srcY, uint8_t * srcU, uint8_t * srcV, uint8_t * dst, int srcW, int h, int dstW)
 {
	 IppStatus st = ippStsNoErr;
	 const Ipp8u* src[3] = { srcY, srcU, srcV };
	 int srcStep[3] = { srcW, srcW / 2, srcW / 2 };

	 st = ippiYCbCr420ToRGB_8u_P3C3R(src, srcStep, dst, dstW * 3, { srcW, h });

	 return (st == ippStsNoErr);
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
bool VSVideoProcessingIpp::MirrorI420(uint8_t* src, uint8_t* dst, int srcWidth, int srcHeight, int mode)
{
	IppStatus st = ippStsNoErr;
	IppiAxis flip = ippAxsHorizontal;
	if (mode > 0) flip = ippAxsVertical;
	if (mode > 1) flip = ippAxsBoth;

	int Size = srcWidth * srcHeight;
	uint8_t *pY, *pYdst, *pU, *pUdst, *pV, *pVdst;
	pY = src;
	pU = src + Size;
	pV = src + Size * 5 / 4;
	pYdst = dst;
	pUdst = dst + Size;
	pVdst = dst + Size * 5 / 4;

	IppiSize roiSrc = { srcWidth, srcHeight };
	IppiSize roiSrcUV = { srcWidth >> 1, srcHeight >> 1 };
	/// Y
	st = ippiMirror_8u_C1R(pY, srcWidth, pYdst, srcWidth, roiSrc, flip);
	if (st != ippStsNoErr) return false;
	/// U
	st = ippiMirror_8u_C1R(pU, srcWidth >> 1, pUdst, srcWidth >> 1, roiSrcUV, flip);
	if (st != ippStsNoErr) return false;
	/// V
	st = ippiMirror_8u_C1R(pV, srcWidth >> 1, pVdst, srcWidth >> 1, roiSrcUV, flip);

	return (st == ippStsNoErr);
}

bool VSVideoProcessingIpp::ResampleI420(
	uint8_t* src, int srcW, int srcH,
	uint8_t *dst, int dstW, int dstH)
{
	const unsigned char *pY(src), *pU(src + srcW * srcH), *pV(src + srcW * srcH * 5 / 4);
	unsigned char *pY_(dst), *pU_(dst + dstW * dstH), *pV_(dst + dstW * dstH * 5 / 4);

	IppStatus st = ippStsNoErr;
	int mode = dstW < srcW ? IPPI_INTER_LINEAR : IPPI_INTER_CUBIC;

	IppiSize roiSrc = { srcW, srcH };
	IppiSize roiDst = { dstW, dstH };
	IppiRect intSrc = { 0, 0, srcW, srcH };

	double factorW = double(dstW) / double(srcW);
	double factorH = double(dstH) / double(srcH);
	/// Y
	st = IppResizePlane1Channel(pY, roiSrc, srcW, intSrc, pY_, dstW, roiDst, factorW, factorH, mode);
	if (st != ippStsNoErr) return false;
	/// U
	IppiSize roiSrcUV = { srcW >> 1, srcH >> 1 };
	IppiSize roiDstUV = { dstW >> 1, dstH >> 1 };
	IppiRect intSrcUV = { intSrc.x, intSrc.y, intSrc.width / 2, intSrc.height / 2 };
	st = IppResizePlane1Channel(pU, roiSrcUV, srcW / 2, intSrcUV, pU_, dstW / 2, roiDstUV, factorW, factorH, mode);
	if (st != ippStsNoErr) return false;
	/// V
	st = IppResizePlane1Channel(pV, roiSrcUV, srcW / 2, intSrcUV, pV_, dstW / 2, roiDstUV, factorW, factorH, mode);

	return (st == ippStsNoErr);
}

/*******************************************************************************
 * Bicubic resample with crop, IPP implementation
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 *
 *  \date    21-10-2011		Created
 ******************************************************************************/
bool VSVideoProcessingIpp::ResampleCropI420(
	uint8_t* pSrc[3], uint8_t* pDst[3],
	int srcW, int srcH, int srcStep,
	int dstW, int dstH, int dstStep,
	int srcWR, int srcHR,
	int srcOffsetW, int srcOffsetH,
	double factorW, double factorH,
	int mode)
{
	IppStatus st = ippStsNoErr;

	IppiSize roiSrc = { srcW, srcH };
	IppiSize roiDst = { dstW, dstH };
	IppiRect intSrc = { srcOffsetW, srcOffsetH, srcWR, srcHR };
	/// Y
	st = IppResizePlane1Channel(pSrc[0], roiSrc, srcStep, intSrc, pDst[0], dstStep, roiDst, factorW, factorH, mode);
	if (st != ippStsNoErr) return false;
	/// U
	IppiSize roiSrcUV = { srcW >> 1, srcH >> 1 };
	IppiSize roiDstUV = { dstW >> 1, dstH >> 1 };
	IppiRect intSrcUV = { srcOffsetW >> 1, srcOffsetH >> 1, srcWR >> 1, srcHR >> 1 };
	st = IppResizePlane1Channel(pSrc[1], roiSrcUV, srcStep >> 1, intSrcUV, pDst[1], dstStep >> 1, roiDstUV, factorW, factorH, mode);
	if (st != ippStsNoErr) return false;
	/// V
	st = IppResizePlane1Channel(pSrc[2], roiSrcUV, srcStep >> 1, intSrcUV, pDst[2], dstStep >> 1, roiDstUV, factorW, factorH, mode);

	return (st == ippStsNoErr);
}

/*******************************************************************************
 * Bicubic resample without crop, IPP implementation
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param srcW, srcH		[in]  - width & height of source image;
 * \param dstW, dstH		[in]  - width & height of destination image;
 *
 *  \date    21-10-2011		Created
 ******************************************************************************/
bool VSVideoProcessingIpp::ResampleInscribedI420(
	uint8_t* pSrc[3], uint8_t* pDst[3],
	int srcW, int srcH, int srcStep,
	int dstW, int dstH, int dstStep,
	int srcOffsetW, int srcOffsetH,
	double factorW, double factorH,
	int mode)
{
	IppStatus st = ippStsNoErr;
	memset(pDst[0], 0, dstStep * dstH);
	memset(pDst[1], 0x80, (dstStep >> 1) * (dstH >> 1));
	memset(pDst[2], 0x80, (dstStep >> 1) * (dstH >> 1));
	unsigned char *pY = pDst[0] + srcOffsetW + srcOffsetH * dstStep;
	unsigned char *pU = pDst[1] + (srcOffsetW >> 1) + (srcOffsetH >> 1) * (dstStep >> 1);
	unsigned char *pV = pDst[2] + (srcOffsetW >> 1) + (srcOffsetH >> 1) * (dstStep >> 1);
	/// Y
	IppiSize roiSrc = { srcW, srcH };
	IppiSize roiDst = { dstW, dstH };
	IppiRect intSrc = { 0, 0, srcW, srcH };
	st = IppResizePlane1Channel(pSrc[0], roiSrc, srcStep, intSrc, pY, dstStep, roiDst, factorW, factorH, mode);
	if (st != ippStsNoErr) return false;
	/// U
	IppiSize roiSrcUV = { srcW >> 1, srcH >> 1 };
	IppiSize roiDstUV = { dstW >> 1, dstH >> 1 };
	IppiRect intSrcUV = { 0, 0, srcW >> 1, srcH >> 1 };
	st = IppResizePlane1Channel(pSrc[1], roiSrcUV, srcStep >> 1, intSrcUV, pU, dstStep >> 1, roiDstUV, factorW, factorH, mode);
	if (st != ippStsNoErr) return false;
	/// V
	st = IppResizePlane1Channel(pSrc[2], roiSrcUV, srcStep >> 1, intSrcUV, pV, dstStep >> 1, roiDstUV, factorW, factorH, mode);

	return (st == ippStsNoErr);
}

bool VSVideoProcessingIpp::ResampleRGB(const uint8_t * src, int srcW, int srcPitch, int srcH, uint8_t * dst, int dstW, int dstPitch, int dstH)
{
	IppiSize roiSrc = { srcW, srcH };
	IppiSize roiDst = { dstW, dstH };
	IppiRect intSrc = { 0, 0, srcW, srcH };
	float factorW = float(dstW) / srcW;
	float factorH = float(dstH) / srcH;

	IppStatus st = ippStsNoErr;

	st = IppResizePlane3Channel(
		src, roiSrc, srcPitch, intSrc,
		dst, dstPitch, roiDst,
		factorW, factorH,
		(factorW > 1.0f) ? IPPI_INTER_CUBIC : IPPI_INTER_LINEAR);

	return (st == ippStsNoErr);
}

IppStatus VSVideoProcessingIpp::IppResizePlane1Channel(
	const Ipp8u* pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
	Ipp8u* pDst, int dstStep, IppiSize dstRoiSize,
	double xFactor, double yFactor,
	int interpolation)
{
	IppiInterpolationType interType;
	IppStatus status = ippStsNoErr;

	pSrc += srcRoi.x + srcRoi.y * srcStep;
	srcSize.width = srcRoi.width;
	srcSize.height = srcRoi.height;

	dstRoiSize.width = std::round(srcSize.width * xFactor);
	dstRoiSize.height = std::round(srcSize.height * yFactor);

	switch (interpolation)
	{
	case IPPI_INTER_NN:
		interType = IppiInterpolationType::ippNearest;
		break;

	case IPPI_INTER_LINEAR:
		interType = IppiInterpolationType::ippLinear;
		break;

	case IPPI_INTER_CUBIC:
		interType = IppiInterpolationType::ippCubic;
		break;

	case IPPI_INTER_SUPER:
		interType = IppiInterpolationType::ippSuper;
		break;

	case IPPI_INTER_LANCZOS:
		interType = IppiInterpolationType::ippLanczos;
		break;

	default:
		interType = IppiInterpolationType::ippNearest;
		break;
	}

	if (interType != lastInterType ||
		srcSize.height != lastSrcSize.height ||
		srcSize.width != lastSrcSize.width ||
		dstRoiSize.height != lastDstSize.height ||
		dstRoiSize.width != lastDstSize.width)
	{
		int newSpecSize = 0;
		int newInitBufSize = 0;

		status = ippiResizeGetSize_8u(srcSize, dstRoiSize, interType, 0, &newSpecSize, &newInitBufSize);

		if (status != ippStsNoErr)
			return status;

		if (newSpecSize > specSize)
		{
			ippsFree(pSpec);

			pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(newSpecSize);

			if (pSpec == NULL)
			{
				specSize = 0;
				return ippStsNoMemErr;
			}

			specSize = newSpecSize;
		}

		if (newInitBufSize > initBufSize)
		{
			ippsFree(pInitBuf);

			pInitBuf = ippsMalloc_8u(newInitBufSize);

			if (pInitBuf == NULL)
			{
				initBufSize = 0;
				return ippStsNoMemErr;
			}

			initBufSize = newInitBufSize;
		}

		switch (interType)
		{
		case ippNearest:
			status = ippiResizeNearestInit_8u(srcSize, dstRoiSize, pSpec);
			break;

		case ippLinear:
			status = ippiResizeLinearInit_8u(srcSize, dstRoiSize, pSpec);
			break;

		case ippCubic:
			status = ippiResizeCubicInit_8u(srcSize, dstRoiSize, 1.0f / 3.0f, 1.0f / 3.0f, pSpec, pInitBuf);
			break;

		case ippSuper:
			status = ippiResizeSuperInit_8u(srcSize, dstRoiSize, pSpec);
			break;

		case ippLanczos:
			status = ippiResizeLanczosInit_8u(srcSize, dstRoiSize, 3, pSpec, pInitBuf);
			break;
		}

		if (status != ippStsNoErr)
			return status;

		int newWorkBufferSize = 0;

		status = ippiResizeGetBufferSize_8u(pSpec, dstRoiSize, 1, &newWorkBufferSize);

		if (status != ippStsNoErr)
			return status;

		if (newWorkBufferSize > workBufferSize)
		{
			ippsFree(pWorkBuf);

			pWorkBuf = ippsMalloc_8u(newWorkBufferSize);

			if (pWorkBuf == NULL)
			{
				workBufferSize = 0;
				return ippStsNoMemErr;
			}

			workBufferSize = newWorkBufferSize;
		}


		lastInterType = interType;
		lastSrcSize = srcSize;
		lastDstSize = dstRoiSize;
	}

	IppiPoint dstOffset = { 0, 0 };

	switch (interType)
	{
	case ippNearest:
		status = ippiResizeNearest_8u_C1R(pSrc, srcStep, pDst, dstStep, dstOffset, dstRoiSize, pSpec, pWorkBuf);
		break;

	case ippLinear:
		status = ippiResizeLinear_8u_C1R(pSrc, srcStep, pDst, dstStep, dstOffset, dstRoiSize, IppiBorderType::ippBorderRepl, 0, pSpec, pWorkBuf);
		break;

	case ippCubic:
		status = ippiResizeCubic_8u_C1R(pSrc, srcStep, pDst, dstStep, dstOffset, dstRoiSize, IppiBorderType::ippBorderRepl, 0, pSpec, pWorkBuf);
		break;

	case ippSuper:
		status = ippiResizeSuper_8u_C1R(pSrc, srcStep, pDst, dstStep, dstOffset, dstRoiSize, pSpec, pWorkBuf);
		break;

	case ippLanczos:
		status = ippiResizeLanczos_8u_C1R(pSrc, srcStep, pDst, dstStep, dstOffset, dstRoiSize, IppiBorderType::ippBorderRepl, 0, pSpec, pWorkBuf);
		break;
	}

	return status;
}

IppStatus VSVideoProcessingIpp::IppResizePlane3Channel(
	const Ipp8u * pSrc, IppiSize srcSize, int srcStep, IppiRect srcRoi,
	Ipp8u * pDst, int dstStep, IppiSize dstRoiSize,
	double xFactor, double yFactor,
	int interpolation)
{
	IppiInterpolationType interType;
	IppStatus status = ippStsNoErr;

	pSrc += srcRoi.x + srcRoi.y * srcStep;
	srcSize.width = srcRoi.width;
	srcSize.height = srcRoi.height;

	dstRoiSize.width = round(srcSize.width * xFactor);
	dstRoiSize.height = round(srcSize.height * yFactor);

	switch (interpolation)
	{
	case IPPI_INTER_NN:
		interType = IppiInterpolationType::ippNearest;
		break;

	case IPPI_INTER_LINEAR:
		interType = IppiInterpolationType::ippLinear;
		break;

	case IPPI_INTER_CUBIC:
		interType = IppiInterpolationType::ippCubic;
		break;

	case IPPI_INTER_SUPER:
		interType = IppiInterpolationType::ippSuper;
		break;

	case IPPI_INTER_LANCZOS:
		interType = IppiInterpolationType::ippLanczos;
		break;

	default:
		interType = IppiInterpolationType::ippNearest;
		break;
	}

	if (interType != lastInterType ||
		srcSize.height != lastSrcSize.height ||
		srcSize.width != lastSrcSize.width ||
		dstRoiSize.height != lastDstSize.height ||
		dstRoiSize.width != lastDstSize.width)
	{
		int newSpecSize = 0;
		int newInitBufSize = 0;

		status = ippiResizeGetSize_8u(srcSize, dstRoiSize, interType, 0, &newSpecSize, &newInitBufSize);

		if (status != ippStsNoErr)
			return status;

		if (newSpecSize > specSize)
		{
			ippsFree(pSpec);

			pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(newSpecSize);

			if (pSpec == NULL)
			{
				specSize = 0;
				return ippStsNoMemErr;
			}

			specSize = newSpecSize;
		}

		if (newInitBufSize > initBufSize)
		{
			ippsFree(pInitBuf);

			pInitBuf = ippsMalloc_8u(newInitBufSize);

			if (pInitBuf == NULL)
			{
				initBufSize = 0;
				return ippStsNoMemErr;
			}

			initBufSize = newInitBufSize;
		}

		switch (interType)
		{
		case ippNearest:
			status = ippiResizeNearestInit_8u(srcSize, dstRoiSize, pSpec);
			break;

		case ippLinear:
			status = ippiResizeLinearInit_8u(srcSize, dstRoiSize, pSpec);
			break;

		case ippCubic:
			status = ippiResizeCubicInit_8u(srcSize, dstRoiSize, 1.0f / 3.0f, 1.0f / 3.0f, pSpec, pInitBuf);
			break;

		case ippSuper:
			status = ippiResizeSuperInit_8u(srcSize, dstRoiSize, pSpec);
			break;

		case ippLanczos:
			status = ippiResizeLanczosInit_8u(srcSize, dstRoiSize, 3, pSpec, pInitBuf);
			break;
		}

		if (status != ippStsNoErr)
			return status;

		int newWorkBufferSize = 0;

		status = ippiResizeGetBufferSize_8u(pSpec, dstRoiSize, 3, &newWorkBufferSize);

		if (status != ippStsNoErr)
			return status;

		if (newWorkBufferSize > workBufferSize)
		{
			ippsFree(pWorkBuf);

			pWorkBuf = ippsMalloc_8u(newWorkBufferSize);

			if (pWorkBuf == NULL)
			{
				workBufferSize = 0;
				return ippStsNoMemErr;
			}

			workBufferSize = newWorkBufferSize;
		}


		lastInterType = interType;
		lastSrcSize = srcSize;
		lastDstSize = dstRoiSize;
	}

	IppiPoint dstOffset = { 0, 0 };

	switch (interType)
	{
	case ippNearest:
		status = ippiResizeNearest_8u_C3R(pSrc, srcStep, pDst, dstStep, dstOffset, dstRoiSize, pSpec, pWorkBuf);
		break;

	case ippLinear:
		status = ippiResizeLinear_8u_C3R(pSrc, srcStep, pDst, dstStep, dstOffset, dstRoiSize, IppiBorderType::ippBorderRepl, 0, pSpec, pWorkBuf);
		break;

	case ippCubic:
		status = ippiResizeCubic_8u_C3R(pSrc, srcStep, pDst, dstStep, dstOffset, dstRoiSize, IppiBorderType::ippBorderRepl, 0, pSpec, pWorkBuf);
		break;

	case ippSuper:
		status = ippiResizeSuper_8u_C3R(pSrc, srcStep, pDst, dstStep, dstOffset, dstRoiSize, pSpec, pWorkBuf);
		break;

	case ippLanczos:
		status = ippiResizeLanczos_8u_C3R(pSrc, srcStep, pDst, dstStep, dstOffset, dstRoiSize, IppiBorderType::ippBorderRepl, 0, pSpec, pWorkBuf);
		break;
	}

	return status;
}
