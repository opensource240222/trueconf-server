/**
 **************************************************************************
 * \file VSStereoRender.h
 * \brief Video render classes for stereo data.
 *
 * \b Project Client
 * \author I.A.Melechko
 * \date 17.02.2011
 *
 * $Revision: 8 $
 *
 * $History: VSStereoRender.h $
 *
 * *****************  Version 8  *****************
 * User: Melechko     Date: 11.07.11   Time: 13:09
 * Updated in $/VSNA/VSClient
 * Add anaglyph render
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 6  *****************
 * User: Melechko     Date: 24.06.11   Time: 19:32
 * Updated in $/VSNA/VSClient
 * Add OpenGL interlaced render
 *
 * *****************  Version 5  *****************
 * User: Melechko     Date: 23.05.11   Time: 16:39
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 4  *****************
 * User: Melechko     Date: 18.05.11   Time: 13:16
 * Updated in $/VSNA/VSClient
 * Add opengl render
 *
 * *****************  Version 3  *****************
 * User: Melechko     Date: 2.03.11    Time: 19:00
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 2  *****************
 * User: Melechko     Date: 2.03.11    Time: 17:32
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 1  *****************
 * User: Melechko     Date: 17.02.11   Time: 14:19
 * Created in $/VSNA/VSClient
 * Add stereo renders
 **/
#include "VSRender.h"
class CDIBRenderStereoAnaglyph :public CDIBRender{
public:
                            CDIBRenderStereoAnaglyph(CVSInterface* pParentInterface);
    bool					PrepareFrame(HWND m_hwnd);
	int						DrawFrame(HWND hwnd);
};
class CDIBRenderStereoInterlaced :public CDIBRender{
protected:
    BYTE*				m_pBufferDIBString;
public:
                            CDIBRenderStereoInterlaced(CVSInterface* pParentInterface);
							~CDIBRenderStereoInterlaced();
	int						DrawFrame(HWND hwnd);
};
class CDirect3DRenderStereo: public CDIBRender{
public:
  CDirect3DRenderStereo(CVSInterface* pParentInterface);
  ~CDirect3DRenderStereo();
  int	SetMode(eVideoRenderMode Mode, HWND hwnd);
  static LRESULT CALLBACK WindowProc2(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam);
  int DrawFrame(HWND hwnd);
protected:
	CComPtr<IDirect3D9>				m_pD3D;

	CComPtr<IDirect3DDevice9>		m_pD3DDev;
	D3DPRESENT_PARAMETERS			m_pp;
	D3DDISPLAYMODE					m_d3ddm;
	D3DCAPS9						m_caps;
	DWORD							m_repairTime;
	bool ResetDevice(HWND hwnd, int width, int height);
	UINT GetAdapter(IDirect3D9* pD3D, HWND hwnd);
	HRESULT CreateDirect3D();
	HRESULT CreateDevice(HWND hwnd);
	HRESULT AllocSurfaces(int width, int height, D3DFORMAT Format = D3DFMT_X8R8G8B8);
	void DeleteSurfaces();
	CComPtr<IDirect3DSurface9>		m_pBackBuffer;
	CComPtr<IDirect3DSurface9>		m_pBackBufferScaled;
	HWND m_ServiceHWND;
	BOOL m_bFullScreen;
	static WNDCLASSEX m_wc;
	static bool m_bClassRegistered;
	int m_BuffWidth;
	int m_BuffHeight;
private:
	CComPtr<IDirect3DSurface9> gFrameBuffer;
	CComPtr<IDirect3DSurface9> gPackedStereo;
	CComPtr<IDirect3DTexture9> gBlitConfig;
};
class  CDirect3DRenderStereo2: public CDirect3DRenderStereo{
public:
  CDirect3DRenderStereo2(CVSInterface* pParentInterface);
  ~CDirect3DRenderStereo2();
};
#define _USE_OPENGL
#ifdef _USE_OPENGL
#ifndef __OPENGL_STEREO
#define __OPENGL_STEREO
//#include <gl/gl.h>
#define GLEW_STATIC
#include "glew/GL/glew.h"
#include "glew/GL/wglew.h"
#endif

class COpenGLRender :public CDIBRender, CVSThread{
public:
                            COpenGLRender(CVSInterface* pParentInterface);
							~COpenGLRender();
    //bool					PrepareFrame(HWND m_hwnd);
	int						DrawFrame(HWND hwnd);
	//int	SetMode(eVideoRenderMode Mode, HWND hwnd);
	DWORD Loop(LPVOID lpParameter);
protected:
	virtual void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC);
	virtual void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
	virtual void PrepareData(HWND hwnd);
	GLuint m_texture;
	GLuint m_drawList;
	int th;
	int tw;
	void *m_intBuffer;
	HANDLE m_hEvDraw;

};
class COpenGLRenderInterlaced: public COpenGLRender{
public:
	COpenGLRenderInterlaced(CVSInterface* pParentInterface);
protected:
	char *shad_strP;
	char *shad_strV;
	GLuint m_texture2;
	GLuint program;
	GLuint shaderP;
	GLuint shaderV;
	void *m_intBuffer2;
	void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC);
	void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
	void PrepareData(HWND hwnd);
	int m_iSwap;
private:
	static const char strP[];
	static const char strV[];
};
class COpenGLRenderAnaglyph:public COpenGLRenderInterlaced{
public:
	COpenGLRenderAnaglyph(CVSInterface* pParentInterface);
private:
	static const char strP[];
	static const char strV[];
};
class COpenGLRenderActive :public COpenGLRenderInterlaced{
public:
	COpenGLRenderActive(CVSInterface* pParentInterface);
protected:
	void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC);
	void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
private:
	static const char strP[];
	static const char strV[];
};
class COpenGLRenderActive2 :public COpenGLRenderInterlaced{
public:
	COpenGLRenderActive2(CVSInterface* pParentInterface);
protected:
	void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC);
	void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC);
private:
	static const char strP[];
	static const char strV[];
};
#endif