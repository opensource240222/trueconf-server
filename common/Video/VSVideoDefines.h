/****************************************************************************
 * (c) 2002 Visicron Inc.  http://www.visicron.net/
 *
 * Project: VSVideo processing
 *
 * $Revision: 1 $
 * $History: VSVideoDefines.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Video
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Video
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 17.02.04   Time: 19:47
 * Updated in $/VS/Video
 * scaling tested
 * test util
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 12.02.04   Time: 17:01
 * Updated in $/VS/Video
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 13.04.03   Time: 16:01
 * Updated in $/VS/Video
 * fixed bicubic koefs for chroma component
 *
 *				25.11.02 Created
 *
 ****************************************************************************/
/**
 * \file VSVideoDefines.h
 * \brief Contain defines used in video processing files
 *
 \code
 \\---------------------------------------------------------------------------

  Original coefs to transform are:
	Y  = ( 0.257)*r + (0.504)*g + (0.098*b) + 16;
	Cb = (-0.148)*r - (0.291)*g + (0.439*b) + 128;
	Cr = ( 0.439)*r - (0.368)*g - (0.071*b) + 128;

  Reverse transforms are:
	yy= 1.164*y;
 	r = CLIP(yy+		1.596*v);
	g = CLIP(yy-0.392*u-0.814*v);
	b = CLIP(yy+		2.017*u);

 \endcode
*/

#ifndef VS_VIDEO_DEFINES_H
#define VS_VIDEO_DEFINES_H

// common defines
#define CLIP(x) cipping_table[(x)+512]
#define RADIX 8
#define RADIX_1 7
#define F2I_R(x)  (int)((x)*(1<<RADIX)+0.5)
#define SF2I_R(x) (int)((x)*(1<<RADIX)-0.5)
#define F2I_R_1(x)  (int)((x)*(1<<(RADIX-1))+0.5)
#define SF2I_R_1(x) (int)((x)*(1<<(RADIX-1))-0.5)

/** Begin of defines for color conversions ***********************************/
// reverse transform coefs
#define U2R		( F2I_R(0.000	))
#define V2R		( F2I_R(1.59375	))

#define U2G		(-F2I_R(0.390625))
#define V2G		(-F2I_R(0.8125	))

#define U2B		( F2I_R(2.000	))
#define V2B		( F2I_R(0.000	))

#define Y2RGB   ( F2I_R(1.15625	))
#define ADD_RGB ( F2I_R(0.500	))
/******************************************************************************/
// forward transform coefs
#define R2Y ( F2I_R( .25858146350465099904))
#define G2Y ( F2I_R( .50721748610527695965))
#define B2Y ( F2I_R(0.099065915254936906181))

#define R2U (-F2I_R( .14949240858862635882))
#define G2U (-F2I_R( .29323510915461324230))
#define B2U ( F2I_R( .44272751774323960111))

#define R2V ( F2I_R( .43985266373191986344))
#define G2V (-F2I_R( .36798131344892642171))
#define B2V (-F2I_R(0.071871350282993441739))

#define ADD_Y  (F2I_R(16.5))
#define ADD_UV (F2I_R(128.5))

/******************************************************************************/
// Make y u v components from r g b, general equation
#define countYUV(r, g, b, y, u, v)							\
	y = (R2Y*(r) + G2Y*(g) + B2Y*(b) + ADD_Y )>>RADIX;		\
	u = (R2U*(r) + G2U*(g) + B2U*(b) + ADD_UV)>>RADIX;		\
	v = (R2V*(r) + G2V*(g) + B2V*(b) + ADD_UV)>>RADIX;

// Make y u v components separately from r g b
#define countY(r, g, b, y)	y = (R2Y*(r) + G2Y*(g) + B2Y*(b) + ADD_Y )>>RADIX;
#define countU(r, g, b, u)	u = (R2U*(r) + G2U*(g) + B2U*(b) + ADD_UV)>>RADIX;
#define countV(r, g, b, v)	v = (R2V*(r) + G2V*(g) + B2V*(b) + ADD_UV)>>RADIX;

// Make u v components separately from r*4 g*4 b*4
#define countU4(r, g, b, u)	u = (R2U*(r) + G2U*(g) + B2U*(b) + ADD_UV*4)>>(RADIX+2);
#define countV4(r, g, b, v)	v = (R2V*(r) + G2V*(g) + B2V*(b) + ADD_UV*4)>>(RADIX+2);
/******************************************************************************/
// fill rgb common partial values
#define makeRRGGBB(u, v, rr, gg, bb)		\
	rr = (U2R*u+V2R*v);						\
	gg = (U2G*u+V2G*v);						\
	bb = (U2B*u+V2B*v);

// Make r g b components from y and rgb common partial values
#define makeRGB(y, rr, gg, bb, r, g, b, yy)	\
	yy = y*Y2RGB;						\
	r  = CLIP((yy+rr+ADD_RGB)>>RADIX);		\
	g  = CLIP((yy+gg+ADD_RGB)>>RADIX);		\
	b  = CLIP((yy+bb+ADD_RGB)>>RADIX);
/** End of defines for color conversions  *************************************/

// commom bicubic defines
#define BIC_AA0(BIC_A, BIC_D) SF2I_R_1((0.) + (1.*BIC_A+0.)*BIC_D - (2.*BIC_A+0.)*BIC_D*BIC_D + (1.*BIC_A+0.)*BIC_D*BIC_D*BIC_D)
#define BIC_AA1(BIC_A, BIC_D)  F2I_R_1((1.) + (0.*BIC_A+0.)*BIC_D - (1.*BIC_A+3.)*BIC_D*BIC_D + (1.*BIC_A+2.)*BIC_D*BIC_D*BIC_D)
#define BIC_AA2(BIC_A, BIC_D)  F2I_R_1((0.) - (1.*BIC_A+0.)*BIC_D + (2.*BIC_A+3.)*BIC_D*BIC_D - (1.*BIC_A+2.)*BIC_D*BIC_D*BIC_D)
#define BIC_AA3(BIC_A, BIC_D) SF2I_R_1((0.) + (0.*BIC_A+0.)*BIC_D + (1.*BIC_A+0.)*BIC_D*BIC_D - (1.*BIC_A+0.)*BIC_D*BIC_D*BIC_D)
/************* Begin of defines for 1.5 interpolation; values for 0...x.0 pixel
 * reverse order values for 0.x...0 pixel
 * Valuses normalized to 16384
 ******************************************************************************/
/*
#define BIC_LA0 BIC_AA0((-1.), (2./3))
#define BIC_LA1 BIC_AA1((-1.), (2./3))
#define BIC_LA2 BIC_AA2((-1.), (2./3))
#define BIC_LA3 BIC_AA3((-1.), (2./3))
 */ // sinc(x) for d = 2./3
#define BIC_LA0 (-24)
#define BIC_LA1 ( 61)
#define BIC_LA2 (121)
#define BIC_LA3 (-30)

#define BIC_CA0 BIC_AA0((-0.501), (2./3))
#define BIC_CA1 BIC_AA1((-0.501), (2./3))
#define BIC_CA2 BIC_AA2((-0.501), (2./3))
#define BIC_CA3 BIC_AA3((-0.501), (2./3))

/************* Begin of defines for 2 interpolation; values for 0x0 pixel
 * Valuses normalized to 16384
 ******************************************************************************/
#define UP2_LA0 (-32)
#define UP2_LA1 ( 96)

#define UP2_CA0 BIC_AA0((-0.501), (1./2))	//-8
#define UP2_CA1 BIC_AA1((-0.501), (1./2))	//72

/******************************************************************************/
/** End of defines for resampling  */


#endif /*VS_VIDEO_DEFINES_H*/


