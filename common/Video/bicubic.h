/**
 **************************************************************************
 * \file bicubic.h
 * (c) 2002-2005 Visicron Inc.  http://www.visicron.net/
 * \brief bicubic minification functions
 *
 * \b Project Video
 * \author SMirnovK
 * \date 22.06.2005
 *
 * $Revision: 4 $
 *
 * $History: bicubic.h $
 *
 * *****************  Version 4  *****************
 * User: Sanufriev    Date: 22.10.10   Time: 17:43
 * Updated in $/VSNA/Video
 * - optimisation deinterlacing (sse2 bicubic minification)
 *
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 22.10.10   Time: 16:19
 * Updated in $/VSNA/Video
 * - optimization resampling functions
 * - optimization deinterlacing
 * - clean VideoProc class
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
 * *****************  Version 3  *****************
 * User: Sanufriev    Date: 23.09.05   Time: 16:13
 * Updated in $/VS/Video
 * - added 8-bit image processing, C & MMX version
 *
 * *****************  Version 2  *****************
 * User: Sanufriev    Date: 4.07.05    Time: 16:04
 * Updated in $/VS/Video
 * - added 32-bit image processing, C & MMX version
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 22.06.05   Time: 20:13
 * Created in $/VS/Video
 * - new files in Video project
 * - bicubic minification embeedded in videoproc class
 *
 ****************************************************************************/
#ifndef __BICUBIC_H__
#define __BICUBIC_H__

/****************************************************************************
 * Declaration
 ****************************************************************************/
/*******************************************************************************
 * \fn bic_resize_x3
 * \fn bic_resize_y3
 * \fn bic_resize_x3_4
 * \fn bic_resize_y3_4
 *
 * \fn bic_resize_x3_mmx
 * \fn bic_resize_y3_mmx
 * \fn bic_resize_x3_4_mmx
 * \fn bic_resize_y3_4_mmx
 *
 * Bicubic free minification in one direction.
 *
 * \param src				[in]  - pointer to source image;
 * \param dst				[out] - pointer to interpolated destination image;
 * \param x, y				[in]  - width & height of source image;
 * \param newx, newy		[in]  - width & height of destination image;
 * \param step				[in]  - destination line length in bytes;
 *
 * \date    22-06-2005		Created
 ******************************************************************************/

void bic_resize_x1(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len);
void bic_resize_y1(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len);
void bic_resize_x3(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len);
void bic_resize_y3(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len);
void bic_resize_x3_4(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len);
void bic_resize_y3_4(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len);

void bic_resize_x1_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len);
void bic_resize_y1_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len);
void bic_resize_x3_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len);
void bic_resize_y3_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len);
void bic_resize_x3_4_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newx, short *table, int len);
void bic_resize_y3_4_mmx(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step, short *table, int len);

void bic_resize_8_x1(unsigned char* src, unsigned char* dst, int x, int y);
void bic_resize_8_y1(unsigned char* src, unsigned char* dst, int x, int y, int step);
void bic_resize_x_565(unsigned char* src, unsigned char* dst, int x, int y, int newx);
void bic_resize_y_565(unsigned char* src, unsigned char* dst, int x, int y, int newy, int step);

void bic_resize_8_x1_mmx(unsigned char* src, unsigned char* dst, int x, int y);
void bic_resize_8_y1_mmx(unsigned char* src, unsigned char* dst, int x, int y, int step);
void bic_resize_8_x1_sse2(unsigned char* src, unsigned char* dst, int x, int y);
void bic_resize_8_y1_sse2(unsigned char* src, unsigned char* dst, int x, int y, int step);

int genTableAlloc(int x, int newx, short* &table);
int genTableAlloc_mmx(int x, int newx, short* &table);

#endif /*__BICUBIC_H__*/