/**
 **************************************************************************
 * \file Init.h
 * \brief Common functions and initialisation structure declaration
 *
 * (c) 2006 Visicron Corp.  http://www.visicron.com/
 *
 * \b Project: High Quality Resampling
 * \author Smirnov Konstatntin
 * \author Sergey Anufriev
 * \date 24.07.2006
 *
 * $Revision: 1 $
 * $History: Init.h $
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
 ***************************************************************************/

#ifndef  __VS_INIT_H__
#define __VS_INIT_H__

#include <memory.h>
#include "VS_Resize.h"

#define VS_PI 3.14159265358979323846		///< PI value

#define table_size			(130816)		///< number of elements in distortion table

#define MIN(a,b) (a<b) ? (a) : (b)			///< find minimum
#define MAX(a,b) (a>b) ? (a) : (b)			///< find maximum

#define shiftPixel	(13)					///< normalization shift for interpolation algorithm
#define prodPixel	(1 <<(shiftPixel  ))	///< multiplier coefficient for interpolation algorithm
#define deltaPixel	(1 <<(shiftPixel-1))	///< rounding error addition for interpolation algorithm

/// integer calculations precision
#define BASE		(8)
/// scale parameter for pointed precision
#define RADIX		(1<<BASE)
/// Round float value to nearest integer
#define ROUND(x) (x)<0 ? (int)((x)-0.5) : ( (x)>0 ? (int)((x)+0.5) : 0)

/**
 **************************************************************************************************
 *  Initialisation (information) Structure
 *************************************************************************************************/
struct VS_InitStruct
{
	unsigned long		srcW;			///<  number of pixels in source image on X direction
	unsigned long		srcH;			///<  number of pixels in source image on Y direction
	unsigned long		dstW;			///<  number of pixels in destination image on X direction
	unsigned long		dstH;			///<  number of pixels in destination image on Y direction

	float				edgeSharp;		///<  High Quality mode amount coeficient

	unsigned short int*	num_ptn_x;		///<  number of points of destination image being between the points of source image (on X direction)
	unsigned short int*	num_ptn_y;		///<  number of points of destination image being between the points of source image (on Y direction)

	signed short int *	coefficient_x;	///<  array of interpolating coefficients on X direction
	signed short int *	coefficient_y;	///<  array of interpolating coefficients on Y direction

	int*				position_x;	///<  array of X coordinates of source image that are reference points to interpolated X values of destination image pixels
	int*				position_y;	///<  array of Y coordinates of source image that are reference points to interpolated Y values of destination image pixels

	double*				difference_x;	///<  array of difference on X direction for every points of destination image
	double*				difference_y;	///<  array of difference on Y direction for every points of destination image

	unsigned char*		interpMax;		///<  array of maximum interpolate intansity
	unsigned char*		interpMin;		///<  array of minimum interpolate intansity

	unsigned char *		minIntensity;	///< array of minimum intansity
	unsigned char *		maxIntensity;	///< array of maximum intansity

	signed short int*	Table;			///< destortion table
};

void* VS_malloc(int size);
void  VS_free(void* ptr);

void InterpolationImageYUV420_Fast_C(unsigned char* , unsigned char* , VS_InitStruct* );
void InterpolationImageYUV420_Fast_MMX(unsigned char* , unsigned char* , VS_InitStruct* );
void InterpolationImageYUV420_Fast_SSE2(unsigned char* , unsigned char* , VS_InitStruct* );
void InterpolationImageYUV420_IQ_C(unsigned char* , unsigned char* , VS_InitStruct* );
void InterpolationImageYUV420_IQ_MMX(unsigned char* , unsigned char* , VS_InitStruct* );
void InterpolationImageYUV420_IQ_SSE2(unsigned char* , unsigned char* , VS_InitStruct* );

void getCoefficient_C(int , double* , signed short int* , float );
void getCoefficient_SSE2_y(int , double* , signed short int* , float );
void getCoefficientIQ_C(int , double* , signed short int* , float );
void getCoefficientIQ_SSE2_x(int , double* , signed short int* , float );
void getCoefficientIQ_SSE2_y(int , double* , signed short int* , float );

void InterpolationUVPlane_C(unsigned char* , unsigned char* , VS_InitStruct* );
void InterpolationUVPlane_MMX(unsigned char* , unsigned char* , VS_InitStruct* );

unsigned short interp_lin(double);

void getMAXMINIntensity_C(unsigned char* , unsigned char* , unsigned char* , int , int );
void getMAXMINIntensity_SSE2(unsigned char* , unsigned char* , unsigned char* , int , int );
void IntensityInterpolate_C(unsigned char* , unsigned char* , int* , int* , double* , double* , int , int , int , int );
void IntensityInterpolate_MMX(unsigned char* , unsigned char* , int* , int* , double* , double* , int , int , int , int );
void Stage_HighQualityYUV_C(unsigned char* , VS_InitStruct* );
void Stage_HighQualityYUV_MMX(unsigned char* , VS_InitStruct* );
void Stage_HighQualityYUV_SSE2(unsigned char* , VS_InitStruct* );

#endif /*__VS_INIT_H__*/