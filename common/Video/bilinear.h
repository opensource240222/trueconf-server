/**
 **************************************************************************
 * \file bilinear.h
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief bilinear resampling functions declaration
 *
 * \b Project Video
 * \author SMirnovK
 * \date 01.06.2005
 *
 * $Revision: 2 $
 *
 * $History: bilinear.h $
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
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 28.07.06   Time: 16:31
 * Updated in $/VS/Video
 * - added SSE2 variant bilinear resampling
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 1.06.05    Time: 21:03
 * Created in $/VS/Video
 * bilinear scaling integration in Video project
 *
 ****************************************************************************/
#ifndef __BILINEAR_H__
#define __BILINEAR_H__

/****************************************************************************
 * Declaration
 ****************************************************************************/
/*******************************************************************************
 * \fn b_resize_x1
 * \fn b_resize_y1
 * \fn b_resize_x3
 * \fn b_resize_y3
 * \fn b_resize_x3_4
 * \fn b_resize_y3_4
 *
 * \fn b_resize_x1_mmx
 * \fn b_resize_y1_mmx
 * \fn b_resize_x3_mmx
 * \fn b_resize_y3_mmx
 * \fn b_resize_x3_4_mmx
 * \fn b_resize_y3_4_mmx
 *
 * \fn b_resize_x1_sse2
 * \fn b_resize_y1_sse2
 * \fn b_resize_x3_sse2
 * \fn b_resize_y3_sse2
 * \fn b_resize_x3_4_sse2
 * \fn b_resize_y3_4_sse2
 *
 * Bilinear free interpolation in one direction. C and MMX code.
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param x, y				[in]  - width & height of source image;
 * \param newx, newy		[in]  - width & height of destination image;
 * \param dstStep			[in]  - destination line length in bytes;
 *
 * \date    01-06-2005		Created
 ******************************************************************************/
int b_resize_x1(unsigned char *src, unsigned char *dst, int x, int y, int newx);
int b_resize_y1(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep);
int b_resize_x3(unsigned char *src, unsigned char *dst, int x, int y, int newx);
int b_resize_y3(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep);
int b_resize_x3_4(unsigned char *src, unsigned char *dst, int x, int y, int newx);
int b_resize_y3_4(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep);
int b_resize_x_565(unsigned char *src, unsigned char *dst, int x, int y, int newx);
int b_resize_y_565(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep);

int b_resize_x1_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newx);
int b_resize_y1_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep);
int b_resize_x3_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newx);
int b_resize_y3_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep);
int b_resize_x3_4_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newx);
int b_resize_y3_4_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep);
int b_resize_x_565_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newx);
int b_resize_y_565_mmx(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep);

int b_resize_x1_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newx);
int b_resize_y1_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep);
int b_resize_x3_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newx);
int b_resize_y3_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep);
int b_resize_x3_4_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newx);
int b_resize_y3_4_sse2(unsigned char *src, unsigned char *dst, int x, int y, int newy, int dstStep);

#endif /*__BILINEAR_H__*/