/**
 **************************************************************************
 * \file Resampling.cpp
 * \brief Initialize functions, High Quality Stage functions
 *
 * (c) 2006 Visicron Corp.  http://www.visicron.com/
 *
 * \b Project: High Quality Resampling
 * \author Smirnov Konstatntin
 * \author Sergey Anufriev
 * \date 24.07.2006
 *
 * $Revision: 1 $
 * $History: Resampling_YUV.cpp $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Video
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Video
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 28.07.06   Time: 13:41
 * Updated in $/VS/video
 * - bicubic coef for HQ mode = -0.6, for simple mode = -0.75
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 27.07.06   Time: 13:53
 * Updated in $/VCR_Scaling/ResamplingYUV
 * - if (m_prType == VS_PrType_MMX) m_prType = VS_PrType_Common
 *
 * *****************  Version 1  *****************
 * User: Sanufriev    Date: 27.07.06   Time: 12:33
 * Created in $/VCR_Scaling/ResamplingYUV
 * - added Fast Mode & HQ algorithm for YUV420 images
 **************************************************************************************************/

#include "math.h"
#include "malloc.h"
#include "memory.h"
#include "Init.h"

void getNewCoordinates(int, int , int , int , double* , double* , unsigned short int* , unsigned short int* );
void getDifference(int , int , double* , double* , int* );
void createTable(double , signed short int* );

/**
 **************************************************************************************************
 * Allocates memory on a specified alignment boundary
 * \param  [in]      size		- size of the requested memory allocation
 * \return						A pointer to the memory block that was allocated, or NULL if the operation failed
 **************************************************************************************************
 */
void* VS_malloc(int size)
{
    return _aligned_malloc(size,16);
}

/**
 **************************************************************************************************
 * Frees a block of memory that was allocated with VCR_malloc
 * \param  [in]      ptr	- a pointer to the memory block that was returned to the VCR_malloc
 **************************************************************************************************
 */
void VS_free(void* ptr)
{
    _aligned_free(ptr);
}

/**
 **************************************************************************************************
 * Evaluate sign(x)
 * \param  [in]      val	- argument value
 * \return					-1 - if val < 0, 1 - otherwise
 **************************************************************************************************
 */
double sign(double val)
{
	if (val != 0) {
		return fabs(val) / val;
	} else {
		return 1;
	}
}

/**
 **************************************************************************************************
 * Distortion function
 * \param  [in]   D       - contrast value
 * \param  [in]   I_Imin   - minimum intensity
 * \param  [in]   gamma   - amount coefficient for distortion function
 * \return                new contrast value
 **************************************************************************************************
 */
double getValueContrast(double D, double I_Imin, double gamma)
{
	double x = I_Imin / D;
	double k = 1 + gamma * D / 255.0;
	if (x < 0.5)
	{
		return (pow(2*fabs(x),k)) * sign(x) * 0.5 * D;
	}
	else
	{
		return (1 - pow(fabs(2*(1-x)),k) * sign(1-x) * 0.5) * D;
	}
}

/**
 **************************************************************************************************
 * Distortion table create
 * \param  [in]		gamma		- amount coefficient for distortion function
 * \param  [out]	Table		- distortion table
 **************************************************************************************************
 */
void createTable(double gamma, signed short int* Table)
{
	int Imax_Imin = 0, I_Imin = 0, index = 0;
	double pixel = 0.0;
	for (Imax_Imin = 1; Imax_Imin < 256; Imax_Imin++)
	{
		for (I_Imin = 0; I_Imin < 511; I_Imin++)
		{
			Table[Imax_Imin * 511 + I_Imin] = int(getValueContrast(Imax_Imin, I_Imin - 255,
														gamma));
		}
	}
	for (I_Imin = 0; I_Imin < 511; I_Imin++)
	{
		Table[I_Imin] = I_Imin - 255;
	}
}

/**
 **************************************************************************************************
 * Finding coordinates of source image points in new coordinate system
 * \param  [in]      in_w		 - number of pixels in source image on X direction
 * \param  [in]      in_h		 - number of pixels in source image on Y direction
 * \param  [in]      out_w		 - number of pixels in destination image on X direction
 * \param  [in]      out_h		 - number of pixels in destination image on Y direction
 * \param  [out]     coords_x	 - array of destination image coordinates on X direction
 * \param  [out]     coords_y	 - array of destination image coordinates on Y direction
 * \param  [out]     num_ptn_x 	 - number of points of destination image being between the points of source image (on X direction)
 * \param  [out]     num_ptn_y	 - number of points of destination image being between the points of source image (on Y direction)
 **************************************************************************************************
 */
void getNewCoordinates(int in_w, int in_h, int out_w, int out_h, double* coords_x, double* coords_y,
					   unsigned short int* num_ptn_x, unsigned short int* num_ptn_y)
{
	int j = 0, k = 0;
	double N_x = out_w / (double) in_w, N_y = out_h / (double) in_h;

	coords_x[0] = 0.0;
	coords_y[0] = 0.0;
	coords_x[in_w-1] = out_w - 1.0;
	coords_y[in_h-1] = out_h - 1.0;

	double delta_x = 0.0, delta_y = 0.0;

	delta_x = (coords_x[in_w-1] - (in_w - 3) * N_x) * 0.5;
	delta_y = (coords_y[in_h-1] - (in_h - 3) * N_y) * 0.5;

	coords_x[1] = delta_x;
	coords_y[1] = delta_y;
	coords_x[in_w-2] = coords_x[in_w-1] - delta_x;
	coords_y[in_h-2] = coords_y[in_h-1] - delta_y;

	num_ptn_x[0] = (int)coords_x[1] - (int)coords_x[0] + 1;
	num_ptn_y[0] = (int)coords_y[1] - (int)coords_y[0] + 1;

	for (k = 2; k < in_w - 2; k++)
	{
		coords_x[k] = coords_x[k-1] + N_x;
		num_ptn_x[k-1] = num_ptn_x[k-2] + (int)coords_x[k] - (int)coords_x[k-1];
	}

	for (j = 2; j < in_h - 2; j++)
	{
		coords_y[j] = coords_y[j-1] + N_y;
		num_ptn_y[j-1] = num_ptn_y[j-2] + (int)coords_y[j] - (int)coords_y[j-1];
	}

	num_ptn_x[in_w-3] = num_ptn_x[in_w-4] + (int)coords_x[in_w-2] - (int)coords_x[in_w-3];
	num_ptn_y[in_h-3] = num_ptn_y[in_h-4] + (int)coords_y[in_h-2] - (int)coords_y[in_h-3];
	num_ptn_x[in_w-2] = num_ptn_x[in_w-3] + (int)coords_x[in_w-1] - (int)coords_x[in_w-2];
	num_ptn_y[in_h-2] = num_ptn_y[in_h-3] + (int)coords_y[in_h-1] - (int)coords_y[in_h-2];
}

/**
 **************************************************************************************************
 * Finding neighbour points
 * \param  [in,out]  j1       - left neighbour of basic point
 * \param  [in,out]  j2       - right neighbour of basic point
 * \param  [in]      J        - coordinate basic point of destination image
 * \param  [in]      size     - number of pixels in source image on X or Y direction
 * \param  [in]      coords   - array of destination image coordinates on X or Y direction
 * \return                    0 - if neighbour is not, 1 - otherwise
 **************************************************************************************************
 */
int FindNeighbour(int& j1, int& j2, int J, int size, double* coords)
{
	while (coords[j1] < J)
	{
		j1++;
		if (j1 >= size) return 4;
	}

	j2 = j1 - 1;

	return 0;
}

/**
 **************************************************************************************************
 * Finding differences
 * \param  [in]      size         - number of pixels in source image on X or Y direction
 * \param  [in]      inc_size     - number of pixels in destination image on X or Y direction
 * \param  [in]      coords       - array of destination image coordinates on X or Y direction
 * \param  [out]	 difference   - array of difference on X or Y direction for every points of destination image
 * \param  [out]	 position     - array of X or Y coordinates of source image that are reference points to interpolated X or Y values of destination image pixels
 **************************************************************************************************
 */
void getDifference(int size, int inc_size, double* coords, double* difference, int* position)
{
	int j = 0, k = 0;
	int j1 = 1, j2 = 0;

	while (j1 < size)
	{
		difference[j] = (j - coords[j2]) / (coords[j1] - coords[j2]);
		position[j] = j2;
		j++;
        FindNeighbour(j1, j2, j, size, coords);
	}
}

/**
 **************************************************************************************************
 * Evaluate Sinc(x)
 * \param  [in]      value	- argument value
 * \return					  value Sinc(val)
 **************************************************************************************************
 */
double sinc(double value)
{
  if (value != 0.0) {
	value *= VS_PI;
    return sin(value) / value;
  } else {
    return 1.0;
  }
}

/**
 **************************************************************************************************
 * Finding interpolating coefficients for Fast Speed mode and C/MMX variant
 * \param  [in]      size         - number of pixels in source image on X or Y direction
 * \param  [in]      dif          - array of difference on X or Y direction for every points of destination image
 * \param  [out]	 coef         - array of interpolating coefficients on X or Y direction
 * \param  [in]		 coef_a       - polynomial coefficient
 **************************************************************************************************
 */
void getCoefficient_C(int size, double* dif, signed short int* coef, float coef_a)
{
	int j = 0, k = 0;

	double d = 0.0;

	for (j = 0; j < size; j++)
	{
		d = dif[j];
		coef[j*4+1] = ROUND((((coef_a + 2.0) * d - (coef_a + 3.0)) * d * d + 1.0) * prodPixel);
		d = 1 - d;
		coef[j*4+2] = ROUND((((coef_a + 2.0) * d - (coef_a + 3.0)) * d * d + 1.0) * prodPixel);
		d = 1 + d;
		coef[j*4+3] = ROUND((((d - 5.0) * d + 8.0) * d - 4.0) * coef_a * prodPixel);
		d = 3 - d;
		coef[j*4] = ROUND((((d - 5.0) * d + 8.0) * d - 4.0) * coef_a * prodPixel);
	}
}

/**
 **************************************************************************************************
 * Finding interpolating coefficients on Y direction for Fast Speed mode and SSE2 variant
 * \param  [in]      size         - number of pixels in source image on Y direction
 * \param  [in]      dif          - array of difference on Y direction for every points of destination image
 * \param  [out]	 coef         - array of interpolating coefficients on Y direction
 * \param  [in]		 coef_a       - polynomial coefficient
 **************************************************************************************************
 */
void getCoefficient_SSE2_y(int size, double* dif, signed short int* coef, float coef_a)
{
	int j = 0, k = 0;

	double d = 0.0;
	int temp = 0;

	for (j = 0; j < size; j++)
	{
		d = dif[j];
		temp = ROUND((((coef_a + 2.0) * d - (coef_a + 3.0)) * d * d + 1.0) * prodPixel);
		coef[j*32+8] = temp;
		coef[j*32+9] = temp;
		coef[j*32+10] = temp;
		coef[j*32+11] = temp;
		coef[j*32+12] = temp;
		coef[j*32+13] = temp;
		coef[j*32+14] = temp;
		coef[j*32+15] = temp;
		d = 1 - d;
		temp = ROUND((((coef_a + 2.0) * d - (coef_a + 3.0)) * d * d + 1.0) * prodPixel);
		coef[j*32+16] = temp;
		coef[j*32+17] = temp;
		coef[j*32+18] = temp;
		coef[j*32+19] = temp;
		coef[j*32+20] = temp;
		coef[j*32+21] = temp;
		coef[j*32+22] = temp;
		coef[j*32+23] = temp;
		d = 1 + d;
		temp = ROUND((((d - 5.0) * d + 8.0) * d - 4.0) * coef_a * prodPixel);
		coef[j*32+24] = temp;
		coef[j*32+25] = temp;
		coef[j*32+26] = temp;
		coef[j*32+27] = temp;
		coef[j*32+28] = temp;
		coef[j*32+29] = temp;
		coef[j*32+30] = temp;
		coef[j*32+31] = temp;
		d = 3 - d;
		temp = ROUND((((d - 5.0) * d + 8.0) * d - 4.0) * coef_a * prodPixel);
		coef[j*32] = temp;
		coef[j*32+1] = temp;
		coef[j*32+2] = temp;
		coef[j*32+3] = temp;
		coef[j*32+4] = temp;
		coef[j*32+5] = temp;
		coef[j*32+6] = temp;
		coef[j*32+7] = temp;
	}
}

/**
 **************************************************************************************************
 * Finding interpolating coefficients for Improved Quality mode and C/MMX variant
 * \param  [in]      size         - number of pixels in source image on X or Y direction
 * \param  [in]      dif          - array of difference on X or Y direction for every points of destination image
 * \param  [out]	 coef         - array of interpolating coefficients on X or Y direction
 * \param  [in]		 coef_a       - polynomial coefficient
 **************************************************************************************************
 */
void getCoefficientIQ_C(int size, double* dif, signed short int* coef, float coef_a)
{
	int j = 0, k = 0;

	double d = 0.0;

	for (j = 0; j < size; j++)
	{
		d = dif[j];
		coef[j*8+3] = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		d += 1.0;
		coef[j*8+2] = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		d += 1.0;
		coef[j*8+1] = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		d += 1.0;
		coef[j*8] =ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		d = 1.0 -  dif[j];
		coef[j*8+4] = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		d += 1.0;
		coef[j*8+5] = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		d += 1.0;
		coef[j*8+6] = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		d += 1.0;
		coef[j*8+7] = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
	}
}

/**
 **************************************************************************************************
 * Finding interpolating coefficients on X direction for Improved Quality mode and SSE2 variant
 * \param  [in]      size         - number of pixels in source image on X direction
 * \param  [in]      dif          - array of difference on X direction for every points of destination image
 * \param  [out]	 coef         - array of interpolating coefficients on X direction
 * \param  [in]		 coef_a       - polynomial coefficient
 **************************************************************************************************
 */
void getCoefficientIQ_SSE2_x(int size, double* dif, signed short int* coef, float coef_a)
{
	int j = 0, k = 0;

	double d = 0.0;
	signed short int temp = 0;

	for (j = 0; j < size; j++)
	{
		d = dif[j];
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*32+12] = temp;
		coef[j*32+13] = temp;
		coef[j*32+14] = temp;
		coef[j*32+15] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*32+8] = temp;
		coef[j*32+9] = temp;
		coef[j*32+10] = temp;
		coef[j*32+11] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*32+4] = temp;
		coef[j*32+5] = temp;
		coef[j*32+6] = temp;
		coef[j*32+7] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*32] = temp;
		coef[j*32+1] = temp;
		coef[j*32+2] = temp;
		coef[j*32+3] = temp;
		d = 1.0 -  dif[j];
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*32+16] = temp;
		coef[j*32+17] = temp;
		coef[j*32+18] = temp;
		coef[j*32+19] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*32+20] = temp;
		coef[j*32+21] = temp;
		coef[j*32+22] = temp;
		coef[j*32+23] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*32+24] = temp;
		coef[j*32+25] = temp;
		coef[j*32+26] = temp;
		coef[j*32+27] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*32+28] = temp;
		coef[j*32+29] = temp;
		coef[j*32+30] = temp;
		coef[j*32+31] = temp;
	}
}

/**
 **************************************************************************************************
 * Finding interpolating coefficients on Y direction for Improved Quality mode and SSE2 variant
 * \param  [in]      size         - number of pixels in source image on Y direction
 * \param  [in]      dif          - array of difference on Y direction for every points of destination image
 * \param  [out]	 coef         - array of interpolating coefficients on Y direction
 * \param  [in]		 coef_a       - polynomial coefficient
 **************************************************************************************************
 */
void getCoefficientIQ_SSE2_y(int size, double* dif, signed short int* coef, float coef_a)
{
	int j = 0, k = 0;

	double d = 0.0;
	signed short int temp = 0;

	for (j = 0; j < size; j++)
	{
		d = dif[j];
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*64+24] = temp;
		coef[j*64+25] = temp;
		coef[j*64+26] = temp;
		coef[j*64+27] = temp;
		coef[j*64+28] = temp;
		coef[j*64+29] = temp;
		coef[j*64+30] = temp;
		coef[j*64+31] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*64+16] = temp;
		coef[j*64+17] = temp;
		coef[j*64+18] = temp;
		coef[j*64+19] = temp;
		coef[j*64+20] = temp;
		coef[j*64+21] = temp;
		coef[j*64+22] = temp;
		coef[j*64+23] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*64+8] = temp;
		coef[j*64+9] = temp;
		coef[j*64+10] = temp;
		coef[j*64+11] = temp;
		coef[j*64+12] = temp;
		coef[j*64+13] = temp;
		coef[j*64+14] = temp;
		coef[j*64+15] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*64] = temp;
		coef[j*64+1] = temp;
		coef[j*64+2] = temp;
		coef[j*64+3] = temp;
		coef[j*64+4] = temp;
		coef[j*64+5] = temp;
		coef[j*64+6] = temp;
		coef[j*64+7] = temp;
		d = 1.0 -  dif[j];
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*64+32] = temp;
		coef[j*64+33] = temp;
		coef[j*64+34] = temp;
		coef[j*64+35] = temp;
		coef[j*64+36] = temp;
		coef[j*64+37] = temp;
		coef[j*64+38] = temp;
		coef[j*64+39] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*64+40] = temp;
		coef[j*64+41] = temp;
		coef[j*64+42] = temp;
		coef[j*64+43] = temp;
		coef[j*64+44] = temp;
		coef[j*64+45] = temp;
		coef[j*64+46] = temp;
		coef[j*64+47] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*64+48] = temp;
		coef[j*64+49] = temp;
		coef[j*64+50] = temp;
		coef[j*64+51] = temp;
		coef[j*64+52] = temp;
		coef[j*64+53] = temp;
		coef[j*64+54] = temp;
		coef[j*64+55] = temp;
		d += 1.0;
		temp = ROUND((sinc(d) * sinc(d / 4.0)) * prodPixel);
		coef[j*64+56] = temp;
		coef[j*64+57] = temp;
		coef[j*64+58] = temp;
		coef[j*64+59] = temp;
		coef[j*64+60] = temp;
		coef[j*64+61] = temp;
		coef[j*64+62] = temp;
		coef[j*64+63] = temp;
	}
}

VS_HQResampling::VS_HQResampling(VS_ProcType prType, VS_AlgMode algMode)
{
	m_Init = false;
	m_prType = prType;
	if (m_prType == VS_PrType_MMX) m_prType = VS_PrType_Common;
	m_algMode = algMode;
	m_Str = 0;
	m_Str = (VS_InitStruct*)VS_malloc(sizeof(VS_InitStruct));
	//forming destortion table
	if (m_algMode == VS_HQMode) {
		m_Str->Table = (signed short int*)VS_malloc(table_size*sizeof(signed short int));
		m_Str->edgeSharp = (float)0.1 + (float)(15.0 / 20.0);
		createTable(m_Str->edgeSharp, m_Str->Table);
	}
}

VS_HQResampling::~VS_HQResampling()
{
	Release();
	if (m_Str) {
		if (m_algMode == VS_HQMode) {
			if (m_Str->Table) VS_free(m_Str->Table);
		}
		VS_free(m_Str);
		m_Str = 0;
	}
}

VS_ErrorStatus VS_HQResampling::Init(int src_width, int src_height, int dst_width, int dst_height)
{
	if (!m_Init || (m_Str->srcW != src_width || m_Str->srcH != src_height || m_Str->dstW != dst_width || m_Str->dstH != dst_height)) {
		Release();
		m_Str->srcW = src_width;
		m_Str->srcH = src_height;
		m_Str->dstW = dst_width;
		m_Str->dstH = dst_height;
		//preparing data for interpolation
		m_Str->num_ptn_x = (unsigned short int*)VS_malloc(sizeof(unsigned short int)*(src_width-1));
		m_Str->num_ptn_y = (unsigned short int*)VS_malloc(sizeof(unsigned short int)*(src_height-1));
		m_Str->difference_x = (double*)VS_malloc(sizeof(double)*dst_width);
		m_Str->difference_y = (double*)VS_malloc(sizeof(double)*dst_height);
		m_Str->position_x = (int*)VS_malloc(sizeof(int)*dst_width);
		m_Str->position_y = (int*)VS_malloc(sizeof(int)*dst_height);
		//arrays of destination image coordinates
		double *coords_x, *coords_y;
		//finding destination image coordinates
		coords_x = (double*)VS_malloc(sizeof(double)*src_width);
		coords_y = (double*)VS_malloc(sizeof(double)*src_height);
		getNewCoordinates(src_width, src_height, dst_width, dst_height, coords_x, coords_y, m_Str->num_ptn_x, m_Str->num_ptn_y);
		getDifference(src_width, dst_width, coords_x, m_Str->difference_x, m_Str->position_x);
		getDifference(src_height, dst_height, coords_y, m_Str->difference_y, m_Str->position_y);
		double BicubicCoeff = m_algMode==VS_HQMode ? -0.6 : -0.75;
		switch (m_prType) {
			case VS_PrType_MMX:
				HQResampling = &VS_HQResampling::HQResampling_MMX;
				break;
			case VS_PrType_SSE2:
				m_Str->coefficient_x = (signed short int*)VS_malloc(sizeof(signed short int)*dst_width*4);
				m_Str->coefficient_y = (signed short int*)VS_malloc(sizeof(signed short int)*dst_height*32);
				getCoefficient_C(dst_width, m_Str->difference_x, m_Str->coefficient_x, float(BicubicCoeff));
				getCoefficient_SSE2_y(dst_height, m_Str->difference_y, m_Str->coefficient_y, float(BicubicCoeff));
				HQResampling = &VS_HQResampling::HQResampling_SSE2;
				break;
			default:
				m_Str->coefficient_x = (signed short int*)VS_malloc(sizeof(signed short int)*dst_width*4);
				m_Str->coefficient_y = (signed short int*)VS_malloc(sizeof(signed short int)*dst_height*4);
				getCoefficient_C(dst_width, m_Str->difference_x, m_Str->coefficient_x, float(BicubicCoeff));
				getCoefficient_C(dst_height, m_Str->difference_y, m_Str->coefficient_y, float(BicubicCoeff));
				HQResampling = &VS_HQResampling::HQResampling_C;
				break;
		}

		if (m_algMode == VS_HQMode) {
			unsigned long dst_size = dst_width * dst_height;
			m_Str->minIntensity = (unsigned char*)VS_malloc((src_width+1)*(src_height+1));
			m_Str->maxIntensity = (unsigned char*)VS_malloc((src_width+1)*(src_height+1));
			m_Str->interpMax = (unsigned char*)VS_malloc(dst_size);
			m_Str->interpMin = (unsigned char*)VS_malloc(dst_size);
		}

		VS_free(coords_x);
		VS_free(coords_y);

		m_Init = true;
	}
	return VS_ErrSts_NoError;
}

void VS_HQResampling::Release()
{
	if (!m_Init) return;
	VS_free(m_Str->difference_x);
	VS_free(m_Str->difference_y);
	VS_free(m_Str->position_x);
	VS_free(m_Str->position_y);
	VS_free(m_Str->coefficient_x);
	VS_free(m_Str->coefficient_y);
	VS_free(m_Str->num_ptn_x);
	VS_free(m_Str->num_ptn_y);
	if (m_algMode == VS_HQMode) {
		VS_free(m_Str->minIntensity);
		VS_free(m_Str->maxIntensity);
		VS_free(m_Str->interpMax);
		VS_free(m_Str->interpMin);
	}
	m_Init = false;
}

void VS_HQResampling::HQResampling_C(unsigned char* src, unsigned char* dst)
{
	InterpolationImageYUV420_Fast_C(src, dst, m_Str);
	InterpolationUVPlane_C(src, dst, m_Str);

	if (m_algMode == VS_HQMode) {
		getMAXMINIntensity_C(src, m_Str->minIntensity, m_Str->maxIntensity, m_Str->srcW, m_Str->srcH);
		IntensityInterpolate_C(m_Str->minIntensity, m_Str->interpMin, m_Str->position_y, m_Str->position_x,
							m_Str->difference_x, m_Str->difference_y, m_Str->srcW, m_Str->srcH, m_Str->dstW, m_Str->dstH);
		IntensityInterpolate_C(m_Str->maxIntensity, m_Str->interpMax, m_Str->position_y, m_Str->position_x,
							m_Str->difference_x, m_Str->difference_y, m_Str->srcW, m_Str->srcH, m_Str->dstW, m_Str->dstH);
		Stage_HighQualityYUV_C(dst, m_Str);
	}
}

void VS_HQResampling::HQResampling_MMX(unsigned char* src, unsigned char* dst)
{

}

void VS_HQResampling::HQResampling_SSE2(unsigned char* src, unsigned char* dst)
{
	InterpolationImageYUV420_Fast_SSE2(src, dst, m_Str);
	InterpolationUVPlane_MMX(src, dst, m_Str);

	if (m_algMode == VS_HQMode) {
		getMAXMINIntensity_SSE2(src, m_Str->minIntensity, m_Str->maxIntensity, m_Str->srcW, m_Str->srcH);
		IntensityInterpolate_MMX(m_Str->minIntensity, m_Str->interpMin, m_Str->position_y, m_Str->position_x,
							m_Str->difference_x, m_Str->difference_y, m_Str->srcW, m_Str->srcH, m_Str->dstW, m_Str->dstH);
		IntensityInterpolate_MMX(m_Str->maxIntensity, m_Str->interpMax, m_Str->position_y, m_Str->position_x,
							m_Str->difference_x, m_Str->difference_y, m_Str->srcW, m_Str->srcH, m_Str->dstW, m_Str->dstH);
		Stage_HighQualityYUV_SSE2(dst, m_Str);
	}
}

VS_ErrorStatus VS_HQResampling::ResamplingYUV(unsigned char* src, unsigned char* dst)
{
	if (m_Init)
		(this->*HQResampling)(src, dst);
	else
		return VS_ErrSts_NotInit;
	return VS_ErrSts_NoError;
}




