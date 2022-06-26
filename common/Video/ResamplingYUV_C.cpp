/**
 ************************************************************************************************
 * \file ResamplingYUV_C.cpp
 * \brief YUV420 image scaling up functions
 *
 * (c) 2006 Visicron Corp.  http://www.visicron.com/
 *
 * \b Project: High Quality Resampling
 * \author Smirnov Konstatntin
 * \author Sergey Anufriev
 * \date 24.07.2006
 *
 * $Revision: 1 $
 * $History: ResamplingYUV_C.cpp $
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
#include "Init.h"

void InterpolationUVPlane_C_X(unsigned char *src, unsigned char *dst, int src_width, int src_height, int dst_width);
void InterpolationUVPlane_C_Y(unsigned char *src, unsigned char *dst, int dst_width, int src_height, int dst_height);

/**
 **************************************************************************************************
 * Fast Speed mode - Interpolation of four components on X direction for RGB32 Image
 * \param  [in]      rgb[][]    - source image pixels
 * \param  [in]      a0         - interpolating coefficients for first point
 * \param  [in]      a1         - interpolating coefficients for second point
 * \param  [in]      a2         - interpolating coefficients for third point
 * \param  [in]      a3         - interpolating coefficients for fourth point
 * \param  [out]     result_x   - result of X direction interpolation
 **************************************************************************************************
 */
void FragmentInterpolate_X(unsigned char rgb[4][4], signed short int a0, signed short int a1, signed short int a2,
						   signed short int a3, signed short int* result_x)
{
	result_x[0] = (a0 * rgb[0][0] + a1 * rgb[0][1] + a2 * rgb[0][2] + a3 * rgb[0][3] + deltaPixel) >> shiftPixel;
	result_x[1] = (a0 * rgb[1][0] + a1 * rgb[1][1] + a2 * rgb[1][2] + a3 * rgb[1][3] + deltaPixel) >> shiftPixel;
	result_x[2] = (a0 * rgb[2][0] + a1 * rgb[2][1] + a2 * rgb[2][2] + a3 * rgb[2][3] + deltaPixel) >> shiftPixel;
	result_x[3] = (a0 * rgb[3][0] + a1 * rgb[3][1] + a2 * rgb[3][2] + a3 * rgb[3][3] + deltaPixel) >> shiftPixel;
}

/**
 **************************************************************************************************
 * Fast Speed mode - Interpolation of four components on Y direction for RGB32 Image
 * \param  [in]      result_x[] - result of X direction interpolation
 * \param  [in]      a0         - interpolating coefficients for first point
 * \param  [in]      a1         - interpolating coefficients for second point
 * \param  [in]      a2         - interpolating coefficients for third point
 * \param  [in]      a3         - interpolating coefficients for fourth point
 * \return                        one interpolated component on both, X and Y directions
 **************************************************************************************************
 */
unsigned char FragmentInterpolate_Y(signed short int result_x[4], signed short int a0, signed short int a1,
						   signed short int a2, signed short int a3)
{
	int pixel = (a0 * result_x[0] + a1 * result_x[1] + a2 * result_x[2] + a3 * result_x[3] + deltaPixel) >> shiftPixel;

	if (pixel > 255) pixel = 255;
	if (pixel < 0) pixel = 0;

	return pixel;
}

/**
 **************************************************************************************************
 * Improved Quality mode - Interpolation of eight components on X direction for YUV420 Image
 * \param  [in]      rgb[][]    - source image pixels
 * \param  [in]      a0         - interpolating coefficients for first point
 * \param  [in]      a1         - interpolating coefficients for second point
 * \param  [in]      a2         - interpolating coefficients for third point
 * \param  [in]      a3         - interpolating coefficients for fourth point
 * \param  [in]      a4         - interpolating coefficients for fifth point
 * \param  [in]      a5         - interpolating coefficients for sixth point
 * \param  [in]      a6         - interpolating coefficients for seventh point
 * \param  [in]      a7         - interpolating coefficients for eighth point
 * \param  [out]     result_x   - result of X direction interpolation
 **************************************************************************************************
 */
void FragmentInterpolateIQ_X(unsigned char rgb[8][8], signed short int a0, signed short int a1, signed short int a2,
						   signed short int a3, signed short int a4, signed short int a5, signed short int a6, signed short int a7,
						   signed short int* result_x)
{
	result_x[0] = (a0 * rgb[0][0] + a1 * rgb[0][1] + a2 * rgb[0][2] + a3 * rgb[0][3] +
				   a4 * rgb[0][4] + a5 * rgb[0][5] + a6 * rgb[0][6] + a7 * rgb[0][7]+ deltaPixel) >> shiftPixel;
	result_x[1] = (a0 * rgb[1][0] + a1 * rgb[1][1] + a2 * rgb[1][2] + a3 * rgb[1][3] +
				   a4 * rgb[1][4] + a5 * rgb[1][5] + a6 * rgb[1][6] + a7 * rgb[1][7]+ deltaPixel) >> shiftPixel;
	result_x[2] = (a0 * rgb[2][0] + a1 * rgb[2][1] + a2 * rgb[2][2] + a3 * rgb[2][3] +
				   a4 * rgb[2][4] + a5 * rgb[2][5] + a6 * rgb[2][6] + a7 * rgb[2][7]+ deltaPixel) >> shiftPixel;
	result_x[3] = (a0 * rgb[3][0] + a1 * rgb[3][1] + a2 * rgb[3][2] + a3 * rgb[3][3] +
				   a4 * rgb[3][4] + a5 * rgb[3][5] + a6 * rgb[3][6] + a7 * rgb[3][7]+ deltaPixel) >> shiftPixel;
	result_x[4] = (a0 * rgb[4][0] + a1 * rgb[4][1] + a2 * rgb[4][2] + a3 * rgb[4][3] +
				   a4 * rgb[4][4] + a5 * rgb[4][5] + a6 * rgb[4][6] + a7 * rgb[4][7]+ deltaPixel) >> shiftPixel;
	result_x[5] = (a0 * rgb[5][0] + a1 * rgb[5][1] + a2 * rgb[5][2] + a3 * rgb[5][3] +
				   a4 * rgb[5][4] + a5 * rgb[5][5] + a6 * rgb[5][6] + a7 * rgb[5][7]+ deltaPixel) >> shiftPixel;
	result_x[6] = (a0 * rgb[6][0] + a1 * rgb[6][1] + a2 * rgb[6][2] + a3 * rgb[6][3] +
				   a4 * rgb[6][4] + a5 * rgb[6][5] + a6 * rgb[6][6] + a7 * rgb[6][7]+ deltaPixel) >> shiftPixel;
	result_x[7] = (a0 * rgb[7][0] + a1 * rgb[7][1] + a2 * rgb[7][2] + a3 * rgb[7][3] +
				   a4 * rgb[7][4] + a5 * rgb[7][5] + a6 * rgb[7][6] + a7 * rgb[7][7]+ deltaPixel) >> shiftPixel;
}

/**
 **************************************************************************************************
 * Improved Quality mode - Interpolation of eight components on Y direction for YUV420 Image
 * \param  [in]      result_x[] - result of X direction interpolation
 * \param  [in]      a0         - interpolating coefficients for first point
 * \param  [in]      a1         - interpolating coefficients for second point
 * \param  [in]      a2         - interpolating coefficients for third point
 * \param  [in]      a3         - interpolating coefficients for fourth point
 * \param  [in]      a4         - interpolating coefficients for fifth point
 * \param  [in]      a5         - interpolating coefficients for sixth point
 * \param  [in]      a6         - interpolating coefficients for seventh point
 * \param  [in]      a7         - interpolating coefficients for eighth point
 * \return                        one interpolated component on both, X and Y directions
 **************************************************************************************************
 */
unsigned char FragmentInterpolateIQ_Y(signed short int result_x[8], signed short int a0, signed short int a1, signed short int a2,
						     signed short int a3, signed short int a4, signed short int a5, signed short int a6, signed short int a7)
{
	int pixel = (a0 * result_x[0] + a1 * result_x[1] + a2 * result_x[2] + a3 * result_x[3] +
				 a4 * result_x[4] + a5 * result_x[5] + a6 * result_x[6] + a7 * result_x[7] + deltaPixel) >> shiftPixel;

	if (pixel > 255) pixel = 255;
	if (pixel < 0) pixel = 0;

	return pixel;
}

/**
 **************************************************************************************************
 * YUV image Fast Speed mode interpolation
 * \param  [in]	      in_Img    - source image
 * \param  [out]	  out_Img   - destination image
 * \param  [in]       Str		- input information structure
 **************************************************************************************************
 */
void InterpolationImageYUV420_Fast_C(unsigned char* src, unsigned char* dst, VS_InitStruct* Str)
{
	int src_w = Str->srcW, src_h = Str->srcH, dst_w = Str->dstW, dst_h = Str->dstH;
	int i = 0, l = 0, k = 0, j = 0;
	unsigned char yuv[4][4]; /// array for storing source YUV image multipliers
	signed short int result_x[4]; /// array for storing interpolate YUV image multipliers palette on X direction
	int dl = 0, dy = 0, dy_plus_1 = 0, dy_plus_2 = 0, dy_minus_1 = 0, dx = 0;
	int ddl = 0, sx_ = src_w - 2, sy_ = src_h - 2;
	int x = 0, y = 0;
	signed short int a0, a1, a2, a3;

	int DX = 0, DY = 0, NUMX = 0, NUMY = 0;

	for (y = 0; y <= sy_; y++)
	{
		DY = Str->num_ptn_y[y];

		if (y == 0) {
			dy = 0;
			dy_plus_1 = src_w;
			dy_plus_2 = dy_plus_1 + src_w;
			dy_minus_1 = 0;
		} else if (y == sy_) {
			dy = y * src_w;
			dy_plus_1 = dy + src_w;
			dy_plus_2 = dy_plus_1;
			dy_minus_1 = dy - src_w;
		} else {
			dy = y * src_w;
			dy_plus_1 = dy + src_w;
			dy_plus_2 = dy_plus_1 + src_w;
			dy_minus_1 = dy - src_w;
		}

		NUMX = 0;

		for (x = 0; x <= sx_; x++)
		{
			dx = x - 1;
			/// generating yuv[4][4] array
			if (x == 0) {
				for (l = x; l < 1; l++) {
					yuv[0][l-x] = src[dy_minus_1];
					yuv[1][l-x] = src[dy];
					yuv[2][l-x] = src[dy_plus_1];
					yuv[3][l-x] = src[dy_plus_2];
				}
				for (l = 1 - x; l < 4; l++) {
					dl = dx + l;
					yuv[0][l] = src[dy_minus_1+dl];
					yuv[1][l] = src[dy+dl];
					yuv[2][l] = src[dy_plus_1+dl];
					yuv[3][l] = src[dy_plus_2+dl];
				}
			} else if (x == sx_) {
				for (l = x - 1; l < src_w; l++) {
					dl = l;
					yuv[0][l-x+1] = src[dy_minus_1+dl];
					yuv[1][l-x+1] = src[dy+dl];
					yuv[2][l-x+1] = src[dy_plus_1+dl];
					yuv[3][l-x+1] = src[dy_plus_2+dl];
				}
				for (l = src_w - x + 1; l < 4; l++) {
					dl = src_w - 1;
					yuv[0][l] = src[dy_minus_1+dl];
					yuv[1][l] = src[dy+dl];
					yuv[2][l] = src[dy_plus_1+dl];
					yuv[3][l] = src[dy_plus_2+dl];
				}
			} else {
				for (l = 0; l < 4; l++)
				{
					dl = dx + l;
					yuv[0][l] = src[dy_minus_1+dl];
					yuv[1][l] = src[dy+dl];
					yuv[2][l] = src[dy_plus_1+dl];
					yuv[3][l] = src[dy_plus_2+dl];
				}
			}

			DX = Str->num_ptn_x[x];
			for (k = NUMX; k < DX; k++)
			{
				a0 = Str->coefficient_x[k*4];
				a1 = Str->coefficient_x[k*4+1];
				a2 = Str->coefficient_x[k*4+2];
				a3 = Str->coefficient_x[k*4+3];
				//pixel interpolation on X direction
				FragmentInterpolate_X(yuv, a0, a1, a2, a3, result_x);

				for (j = NUMY; j < DY; j++)
				{
					ddl = j * dst_w + k;
					a0 = Str->coefficient_y[j*4];
					a1 = Str->coefficient_y[j*4+1];
					a2 = Str->coefficient_y[j*4+2];
					a3 = Str->coefficient_y[j*4+3];
					///pixel interpolation on Y direction
					dst[ddl] = FragmentInterpolate_Y(result_x, a0, a1, a2, a3);
				}
			}
			NUMX = DX;
		}
		NUMY = DY;
	}
}

/**
 **************************************************************************************************
 * YUV420 image Improved Quality mode interpolation
 * \param  [in]	      in_Img    - source image
 * \param  [out]	  out_Img   - destination image
 * \param  [in]       Str		- input information structure
 **************************************************************************************************
 */
void InterpolationImageYUV420_IQ_C(unsigned char* src, unsigned char* dst, VS_InitStruct* Str)
{
	int src_w = Str->srcW, src_h = Str->srcH, dst_w = Str->dstW, dst_h = Str->dstH;
	int i = 0, l = 0, k = 0, j = 0;
	unsigned char yuv[8][8];
	signed short int result_x[8];
	unsigned int d_y[8];

	int dl = 0, dx = 0, ind = 0;
	int ddl = 0,
		sx_ = src_w - 2, sy_ = src_h - 2;

	int x = 0, y = 0;
	signed short int a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0, a7 = 0;

	int DX = 0, DY = 0, NUMX = 0, NUMY = 0;

	for (y = 0; y <= sy_; y++)
	{
		DY = Str->num_ptn_y[y];

		if (y < 3) {
			for (l = y; l < 3; l++) {
				d_y[l-y] = 0;
			}
			for (l = 3 - y; l < 8; l++) {
				d_y[l] = (l + y - 3) * src_w;
			}
		} else if (y > sy_ - 3) {
			for (l = y - 3; l < src_h; l++) {
				d_y[l-y+3] = l * src_w;
			}
			for (l = src_h - y + 3; l < 8; l++) {
				d_y[l] = (src_h - 1) * src_w;
			}
		} else {
			d_y[3] = y * src_w;
			d_y[2] = d_y[3] - src_w;
			d_y[1] = d_y[2] - src_w;
			d_y[0] = d_y[1] - src_w;
			d_y[4] = d_y[3] + src_w;
			d_y[5] = d_y[4] + src_w;
			d_y[6] = d_y[5] + src_w;
			d_y[7] = d_y[6] + src_w;
		}

		NUMX = 0;

		for (x = 0; x <= sx_; x++)
		{
			dx = x - 3;
			//generating yuv[8][8] array
			if (x < 3) {
				for (l = x; l < 3; l++) {
					yuv[0][l-x] = src[d_y[0]];
					yuv[1][l-x] = src[d_y[1]];
					yuv[2][l-x] = src[d_y[2]];
					yuv[3][l-x] = src[d_y[3]];
					yuv[4][l-x] = src[d_y[4]];
					yuv[5][l-x] = src[d_y[5]];
					yuv[6][l-x] = src[d_y[6]];
					yuv[7][l-x] = src[d_y[7]];
				}
				for (l = 3 - x; l < 8; l++) {
					dl = dx + l;
					yuv[0][l] = src[d_y[0]+dl];
					yuv[1][l] = src[d_y[1]+dl];
					yuv[2][l] = src[d_y[2]+dl];
					yuv[3][l] = src[d_y[3]+dl];
					yuv[4][l] = src[d_y[4]+dl];
					yuv[5][l] = src[d_y[5]+dl];
					yuv[6][l] = src[d_y[6]+dl];
					yuv[7][l] = src[d_y[7]+dl];
				}
			} else if (x > sx_ - 3) {
				for (l = x - 3; l < src_w; l++) {
					dl = l;
					yuv[0][l-x+3] = src[d_y[0]+dl];
					yuv[1][l-x+3] = src[d_y[1]+dl];
					yuv[2][l-x+3] = src[d_y[2]+dl];
					yuv[3][l-x+3] = src[d_y[3]+dl];
					yuv[4][l-x+3] = src[d_y[4]+dl];
					yuv[5][l-x+3] = src[d_y[5]+dl];
					yuv[6][l-x+3] = src[d_y[6]+dl];
					yuv[7][l-x+3] = src[d_y[7]+dl];
				}
				for (l = src_w - x + 3; l < 8; l++) {
					dl = src_w - 1;
					yuv[0][l] = src[d_y[0]+dl];
					yuv[1][l] = src[d_y[1]+dl];
					yuv[2][l] = src[d_y[2]+dl];
					yuv[3][l] = src[d_y[3]+dl];
					yuv[4][l] = src[d_y[4]+dl];
					yuv[5][l] = src[d_y[5]+dl];
					yuv[6][l] = src[d_y[6]+dl];
					yuv[7][l] = src[d_y[7]+dl];
				}
			} else {
				for (l = 0; l < 8; l++)
				{
					dl = dx + l;
					yuv[0][l] = src[d_y[0]+dl];
					yuv[1][l] = src[d_y[1]+dl];
					yuv[2][l] = src[d_y[2]+dl];
					yuv[3][l] = src[d_y[3]+dl];
					yuv[4][l] = src[d_y[4]+dl];
					yuv[5][l] = src[d_y[5]+dl];
					yuv[6][l] = src[d_y[6]+dl];
					yuv[7][l] = src[d_y[7]+dl];
				}
			}

			DX = Str->num_ptn_x[x];
			for (k = NUMX; k < DX; k++)
			{
				a0 = Str->coefficient_x[k*8];
				a1 = Str->coefficient_x[k*8+1];
				a2 = Str->coefficient_x[k*8+2];
				a3 = Str->coefficient_x[k*8+3];
				a4 = Str->coefficient_x[k*8+4];
				a5 = Str->coefficient_x[k*8+5];
				a6 = Str->coefficient_x[k*8+6];
				a7 = Str->coefficient_x[k*8+7];

				//pixel interpolation on X direction
				FragmentInterpolateIQ_X(yuv, a0, a1, a2, a3, a4, a5, a6, a7, result_x);

				for (j = NUMY; j < DY; j++)
				{
					ddl = j * dst_w + k;
					a0 = Str->coefficient_y[j*8];
					a1 = Str->coefficient_y[j*8+1];
					a2 = Str->coefficient_y[j*8+2];
					a3 = Str->coefficient_y[j*8+3];
					a4 = Str->coefficient_y[j*8+4];
					a5 = Str->coefficient_y[j*8+5];
					a6 = Str->coefficient_y[j*8+6];
					a7 = Str->coefficient_y[j*8+7];
					//pixel interpolation on Y direction
					dst[ddl] = FragmentInterpolateIQ_Y(result_x, a0, a1, a2,
							                                   a3, a4, a5, a6, a7);
				}
			}
			NUMX = DX;
		}
		NUMY = DY;
	}
}

void InterpolationUVPlane_C(unsigned char *src, unsigned char *dst, VS_InitStruct* Str)
{
	int src_width = Str->srcW, src_height = Str->srcH, dst_width = Str->dstW, dst_height = Str->dstH,
		src_width_2 = src_width / 2, src_height_2 = src_height / 2,
		dst_width_2 = dst_width / 2, dst_height_2 = dst_height / 2;
	unsigned char *src_u = src + src_width * src_height,
				  *src_v = src_u + src_width_2 * src_height_2,
				  *dst_u = dst + dst_width * dst_height,
				  *dst_v = dst_u + dst_width_2 * dst_height_2;

	unsigned char* temp = (unsigned char*)VS_malloc(sizeof(unsigned char)*src_height_2*dst_width_2);

	InterpolationUVPlane_C_X(src_u, temp, src_width_2, src_height_2, dst_width_2);
	InterpolationUVPlane_C_Y(temp, dst_u, dst_width_2, src_height_2, dst_height_2);

	InterpolationUVPlane_C_X(src_v, temp, src_width_2, src_height_2, dst_width_2);
	InterpolationUVPlane_C_Y(temp, dst_v, dst_width_2, src_height_2, dst_height_2);

	VS_free(temp);
}

unsigned short interp_lin(double x)
{
	x = x - (double)(int)x;
	return ROUND((1 - x) * RADIX);
}

/// 8 bit, x direction
void InterpolationUVPlane_C_X(unsigned char *src, unsigned char *dst, int src_width, int src_height, int dst_width)
{
	int i = 0, j = 0;
	double factor = (double)(src_width - 1) / (dst_width - 1);

	unsigned short *x_coef = (unsigned short*)VS_malloc(sizeof(unsigned short)*dst_width*2);

	for (i = 0; i < dst_width; i++) {
		x_coef[i*2+0] = (int)(factor * i);
		x_coef[i*2+1] = interp_lin(factor * i);
	}

	for (j = 0; j < src_height; j++) {
		*dst = *src;
		for (i = 1; i < (dst_width - 1); i++) {
			int x_pos = x_coef[i*2+0];
			int dirx_coef = x_coef[i*2+1];
			int invx_coef = RADIX - dirx_coef;
			dst[i] = (src[x_pos] * dirx_coef + src[x_pos+1] * invx_coef + RADIX / 2) >> BASE;
		}
		src += src_width;
		dst += dst_width;
		*(dst - 1) = *(src - 1);
	}
	VS_free(x_coef);
}

/// 8 bit, y direction
void InterpolationUVPlane_C_Y(unsigned char *src, unsigned char *dst, int dst_width, int src_height, int dst_height)
{
	int i = 0, j = 0;
	double factor = (double)(src_height - 1) / (dst_height - 1);

	memcpy(dst, src, dst_width);
	for (j = 1; j < dst_height - 1; j++) {
		int y_pos = (int)(factor * j);
		int y_coef = interp_lin(factor * j);
		int invy_coef = RADIX - y_coef;
		unsigned char * ps = src + dst_width * y_pos;
		unsigned char * pd = dst + dst_width * j;
		for (i = 0; i < dst_width; i += 4) {
			pd[i+0] = (ps[i+0] * y_coef + ps[dst_width+i+0] * invy_coef + RADIX / 2) >> BASE;
			pd[i+1] = (ps[i+1] * y_coef + ps[dst_width+i+1] * invy_coef + RADIX / 2) >> BASE;
			pd[i+2] = (ps[i+2] * y_coef + ps[dst_width+i+2] * invy_coef + RADIX / 2) >> BASE;
			pd[i+3] = (ps[i+3] * y_coef + ps[dst_width+i+3] * invy_coef + RADIX / 2) >> BASE;
		}
		if (i > dst_width)
			for (i -= 4; i < dst_width; i++)
				pd[i+0] = (ps[i+0] * y_coef + ps[dst_width+i+0] * invy_coef + RADIX / 2) >> BASE;
	}
	memcpy(dst + dst_width * j, src + dst_width * (src_height - 1), dst_width);
}

/**
 **************************************************************************************************
 * Finding minimum and maximum intensity value
 * \param  [in]	      grayImg       - grayscale image
 * \param  [out]	  minIntensity  - array of minimum intansity
 * \param  [out]	  maxIntensity  - array of maximum intansity
 * \param  [in]       sx            - number of pixels in source image on X direction
 * \param  [in]       sy            - number of pixels in source image on Y direction
 **************************************************************************************************
 */
void getMAXMINIntensity_C(unsigned char* grayImg, unsigned char* minIntensity, unsigned char* maxIntensity, int sx, int sy)
{
	int s = 0, t = 0, j = 0, k = 0, index = 0, index_ = 0, index__ = 0;
	unsigned char min_intens = 255, max_intens = 0, intens = 0;
	for (j = 0; j < sy - 3; j++)
	{
		index = (j + 2) * (sx + 1);
		for (k = 0; k < sx - 3; k++)
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
	maxIntensity[sy*(sx+1)+sx] = max_intens;
}

/**
 **************************************************************************************************
 * Intensity interpolation
 * \param  [in]		  intens        - array of source value intensity
 * \param  [out]	  interp        - array of inerpolate value intensity
 * \param  [in]       position_y    - array of Y coordinates of source image that are reference points
 *									  to interpolated Y values of destination image pixels
 * \param  [in]       position_x    - array of X coordinates of source image that are reference points
 *									  to interpolated X values of destination image pixels
 * \param  [in]       difference_x  - array of difference on X direction for every points of destination image
 * \param  [in]       difference_y  - array of difference on Y direction for every points of destination image
 * \param  [in]       sx            - number of pixels in source image on X direction
 * \param  [in]       sy            - number of pixels in source image on Y direction
 * \param  [in]       Sx            - number of pixels in destination image on X direction
 * \param  [in]       Sy            - number of pixels in destination image on Y direction
 **************************************************************************************************
 */
void IntensityInterpolate_C(unsigned char* intens, unsigned char* interp, int* position_y, int* position_x,
							double* difference_x, double* difference_y, int sx, int sy, int Sx, int Sy)
{
	int j = 0, k = 0, i = 0;
	int y = 0, x = 0, index = 0, index_ = 0;
	int t = 0;

	short int* temp = (short int*)VS_malloc(sizeof(short int)*Sx*(sy+1));
	short int* delta_t = (short int*)VS_malloc(sizeof(short int)*Sx);
	short int* ind = (short int*)VS_malloc(sizeof(short int)*Sx);

	for (k = 0; k < Sx; k++)
	{
		x = position_x[k];
		i = int(difference_x[k] + 0.5);
		ind[k] = x + i;
		delta_t[k] = int((difference_x[k] + 0.5 - i) * 64 + 0.5);
	}

	for (j = 0; j < sy + 1; j++)
	{
		index = j * (sx + 1);
		index_ = j * Sx;
		for (k = 0; k < Sx; k++)
		{
			i = ind[k];
			t = delta_t[k];
			temp[index_+k] = intens[index+i] + ((t * (intens[index+i+1] -
				                 intens[index+i]) + 32) >> 6);
		}
	}

	VS_free(ind);
	VS_free(delta_t);

	for (j = 0; j < Sy; j++)
	{
		y = position_y[j];
		index_ = j * Sx;
		i = int(difference_y[j] + 0.5);
		index = (y + i + 1) * Sx;
		t = int((difference_y[j] + 0.5 - i) * 64 + 0.5);
		for (k = 0; k < Sx; k++)
		{
			interp[index_+k] = temp[index-Sx+k] + ((t * (temp[index+k] -
					           temp[index-Sx+k]) + 32) >> 6);
		}
	}
	VS_free(temp);
}

/**
 **************************************************************************************************
 * High Quality mode YUV image processing
 * \param  [in,out]		out_Img			- destination image
 * \param  [in]			Str				- pointer on structure
 **************************************************************************************************
 */
void Stage_HighQualityYUV_C(unsigned char* out_Img, VS_InitStruct* Str)
{
	unsigned long j = 0, k = 0;
	unsigned long out_w = Str->dstW, out_h = Str->dstH, index_ = 0, ind = 0;
	unsigned char rgb_min = 0, rgb_max = 0;
	signed short int pixel = 0;
	unsigned char D = 0, threshold = 20;

	for (j = 0; j < out_h; j++) {
		for (k = 0; k < out_w; k++) {
			index_ = j * out_w + k;
			rgb_min = Str->interpMin[index_];
			rgb_max = Str->interpMax[index_];
			D = rgb_max - rgb_min;

			if (D >= threshold) {
				if (out_Img[index_] > 0) {
					ind = (D * 511) + out_Img[index_] - rgb_min + 255;
					out_Img[index_] = Str->Table[ind] + rgb_min;
					if (out_Img[index_] > 255) out_Img[index_] = 255;
				}
			}
		}
	}
}