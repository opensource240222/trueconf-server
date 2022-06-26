/**
 *******************************************************************************************************
 * \file Deinterlacing.h
 * \brief Declare for deinterlacing filter
 *
 * (c) 2006 Visicron Corp.  http://www.visicron.net/
 *
 * \b Project: Deinterlacing filter
 * \author Sergey Anufriev
 * \author Smirnov Konstatntin
 * \date 03.04.2006
 *******************************************************************************************************
 */

/**
 *******************************************************************************************************
 * \brief  Productivity measurements (test sequence: 640x480, 1377 frames, [msec]. Pentium 4 HT 3.06 GHz, DDR400 1024 Mb)
 *--------------------------------------------------------------------------------------
 *| type algorithm | Motion detection | Denoising | Area interpolation | All algorithm |
 *--------------------------------------------------------------------------------------
 *| Quality |  C   |	11100		  |  9366	  |        2200	       |   23242	   |
 *|			| MMX  |	 2310		  |	 4633	  |	       1070		   |    8723       |
 *--------------------------------------------------------------------------------------
 *| Speed   |  C   |	11100		  |  3873	  |        2250	       |   17890	   |
 *|			| MMX  |	 2310		  |	 1050	  |	       1093		   |    5180       |
 *--------------------------------------------------------------------------------------
 *******************************************************************************************************
 */

#ifndef  __VS_DEINTERLACING_H__
#define __VS_DEINTERLACING_H__

/// Error mode.
enum VS_DERROR
{
	VS_NOERR = 0,		///< no error
	VS_PARAM = 1,		///< incorrect parameters of initialization
	VS_NOMEM,			///< error of memory initilization
	VS_NOTINIT,			///< error of initilization
	VS_OTHER			///< other errors
};

class VS_VideoProc;

/// Input information class
class VS_Deinterlacing
{
public :
	/// Processor(code) type.
	enum typeProc
	{
		C	 = 0,		///< C implementation
		MMX  = 1,		///< MMX implementation
		SSE2 = 2		///< SSE2 implementation
	};
	/// Processing algorithm.
	enum typeFilter
	{
		QUALITY	= 0,	///< quality mode (better quality)
		BLEND	= 1,	///< blend deinterlacing
	};
	VS_Deinterlacing(typeProc proc = C, typeFilter filter = QUALITY);
	~VS_Deinterlacing();
	VS_DERROR			Init(int width, int height, int pitch_y, int pitch_uv, typeFilter filter);
	VS_DERROR			Process(unsigned char *inframe, unsigned char *outframe);
	void				Release();
private:
	VS_VideoProc		*m_VProcY_up, *m_VProcY_down, *m_VProcUV_up, *m_VProcUV_down;
	typeProc			m_procType;
	typeFilter			m_filterType;
	int					m_w, m_h, m_pitch, m_square, m_w2, m_h2, m_pitch2, m_hminus1, m_hminus3, m_wminus1;
	int					m_threshold_y, m_threshold_uv, m_scenethreshold, m_num_pnt_scene;
	bool				m_isInit;
	unsigned char		*m_moving, *m_nrmoving, *m_moving_u, *m_moving_v;
	unsigned char		*m_moving_d;
	unsigned char		*m_moving_prev, *m_moving_prev_u, *m_moving_prev_v;
	unsigned char		*m_prevFrame;
	///< C implementation
	void				C_MotionDetected(unsigned char *inframe, unsigned char *prevframe, unsigned char *motion, int width, int height, int pitch, int threshold);
	void				C_NoiseReduction_Quality(unsigned char *motion, unsigned char *nrmotion, int width, int height, VS_VideoProc *VprUp, VS_VideoProc *VprDown);
	void				C_NoiseReduction_Fast(unsigned char *motion, unsigned char *nrmotion, int width, int height);
	void				C_InterpolateField(unsigned char *inframe, unsigned char *outframe, unsigned char *motion, unsigned char *motion_prev, int width, int height, int pitch);
	void				C_InterpolateField_SceneChange(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch);
	void				C_BlendInterpolate(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch);
	///< MMX/SSE implementation
	void				MMX_MotionDetected(unsigned char *inframe, unsigned char *prevframe, unsigned char *motion, int width, int height, int pitch, int threshold);
	void				MMX_NoiseReduction_Quality(unsigned char *motion, unsigned char *nrmotion, int width, int height, VS_VideoProc *VprUp, VS_VideoProc *VprDown);
	void				MMX_NoiseReduction_Fast(unsigned char *motion, unsigned char *nrmotion, int width, int height);
	void				MMX_InterpolateField(unsigned char *inframe, unsigned char *outframe, unsigned char *motion, unsigned char *motion_prev, int width, int height, int pitch);
	void				MMX_InterpolateField_SceneChange(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch);
	void				MMX_BlendInterpolate(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch);
	///< SSE2 implementation
	void				SSE2_MotionDetected(unsigned char *inframe, unsigned char *prevframe, unsigned char *motion, int width, int height, int pitch, int threshold);
	void				SSE2_NoiseReduction_Quality(unsigned char *motion, unsigned char *nrmotion, int width, int height, VS_VideoProc *VprUp, VS_VideoProc *VprDown);
	void				SSE2_InterpolateField(unsigned char *inframe, unsigned char *outframe, unsigned char *motion, unsigned char *motion_prev, int width, int height, int pitch);
	void				SSE2_InterpolateField_SceneChange(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch);
	void				SSE2_BlendInterpolate(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch);
	///<
	void				(VS_Deinterlacing::*MotionDetected)(unsigned char *inframe, unsigned char *prevframe, unsigned char *motion, int width, int height, int pitch, int threshold);
	void				(VS_Deinterlacing::*NoiseReduction)(unsigned char *motion, unsigned char *nrmotion, int width, int height, VS_VideoProc *VprUp, VS_VideoProc *VprDown);
	void				(VS_Deinterlacing::*InterpolateField)(unsigned char *inframe, unsigned char *outframe, unsigned char *motion, unsigned char *motion_prev, int width, int height, int pitch);
	void				(VS_Deinterlacing::*InterpolateField_SceneChange)(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch);
	void				(VS_Deinterlacing::*BlendInterpolate)(unsigned char *inframe, unsigned char *outframe, int width, int height, int pitch);
};

#endif /*__VS_DEINTERLACING_H__*/