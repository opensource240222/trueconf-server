/**
 **************************************************************************
 * \file VSRender.cpp
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Video render classes Implementation
 *
 * \b Project Client
 * \author SMirnovK
 * \author Melechko Ivan
 * \date 07.10.2002
 *
 * $Revision: 44 $
 *
 * $History: VSRender.cpp $
 *
 * *****************  Version 44  *****************
 * User: Sanufriev    Date: 13.06.12   Time: 18:27
 * Updated in $/VSNA/VSClient
 * - fix direct draw renderer : client rect
 *
 * *****************  Version 43  *****************
 * User: Sanufriev    Date: 30.05.12   Time: 17:15
 * Updated in $/VSNA/VSClient
 * - fix for dx10 on windows xp
 *
 * *****************  Version 42  *****************
 * User: Smirnov      Date: 25.05.12   Time: 17:53
 * Updated in $/VSNA/VSClient
 * -camera mirror is switched on by default
 *
 * *****************  Version 41  *****************
 * User: Sanufriev    Date: 12.05.12   Time: 19:24
 * Updated in $/VSNA/VSClient
 * - fix CaptureFrame() for mirror selfview
 *
 * *****************  Version 40  *****************
 * User: Sanufriev    Date: 12.05.12   Time: 11:57
 * Updated in $/VSNA/VSClient
 * - were added mirror self view video
 *
 * *****************  Version 39  *****************
 * User: Sanufriev    Date: 6.04.12    Time: 11:36
 * Updated in $/VSNA/VSClient
 * - case for video render from registry
 *
 * *****************  Version 38  *****************
 * User: Samoilov     Date: 29.03.12   Time: 15:42
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 37  *****************
 * User: Sanufriev    Date: 6.03.12    Time: 19:40
 * Updated in $/VSNA/VSClient
 * - fix d3d10 (install on WinXP)
 *
 * *****************  Version 36  *****************
 * User: Sanufriev    Date: 5.03.12    Time: 16:08
 * Updated in $/VSNA/VSClient
 * - were added Direct3D10 support
 *
 * *****************  Version 35  *****************
 * User: Sanufriev    Date: 25.08.11   Time: 12:42
 * Updated in $/VSNA/VSClient
 * - fix video render for avi player
 *
 * *****************  Version 34  *****************
 * User: Sanufriev    Date: 18.07.11   Time: 19:02
 * Updated in $/VSNA/VSClient
 * - fix video render
 *
 * *****************  Version 33  *****************
 * User: Melechko     Date: 11.07.11   Time: 13:09
 * Updated in $/VSNA/VSClient
 * Add anaglyph render
 *
 * *****************  Version 32  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 31  *****************
 * User: Sanufriev    Date: 27.05.11   Time: 18:59
 * Updated in $/VSNA/VSClient
 * - direct draw & remote desctop (bug 8982)
 *
 * *****************  Version 30  *****************
 * User: Smirnov      Date: 5.05.11    Time: 12:51
 * Updated in $/VSNA/VSClient
 * - bugfix#8297
 *
 * *****************  Version 29  *****************
 * User: Sanufriev    Date: 29.04.11   Time: 14:48
 * Updated in $/VSNA/VSClient
 * - delete hFont
 *
 * *****************  Version 28  *****************
 * User: Sanufriev    Date: 28.04.11   Time: 11:34
 * Updated in $/VSNA/VSClient
 * - fix direct draw: display name after ctrl+alt+del
 * - fix av direct draw
 *
 * *****************  Version 27  *****************
 * User: Sanufriev    Date: 25.04.11   Time: 11:22
 * Updated in $/VSNA/VSClient
 * - fix direct draw for desctop sharing
 *
 * *****************  Version 26  *****************
 * User: Sanufriev    Date: 22.04.11   Time: 11:07
 * Updated in $/VSNA/VSClient
 * - direct draw: check surface lost
 *
 * *****************  Version 25  *****************
 * User: Sanufriev    Date: 21.04.11   Time: 11:11
 * Updated in $/VSNA/VSClient
 * - increase text height
 *
 * *****************  Version 24  *****************
 * User: Smirnov      Date: 12.04.11   Time: 17:09
 * Updated in $/VSNA/VSClient
 * - av in video fixed
 *
 * *****************  Version 23  *****************
 * User: Sanufriev    Date: 25.03.11   Time: 16:06
 * Updated in $/VSNA/VSClient
 * - fix DirectX Render for non hardware case
 *
 * *****************  Version 22  *****************
 * User: Sanufriev    Date: 25.03.11   Time: 12:15
 * Updated in $/VSNA/VSClient
 * - display name for DirectX render
 *
 * *****************  Version 21  *****************
 * User: Smirnov      Date: 24.03.11   Time: 18:43
 * Updated in $/VSNA/VSClient
 * - videoName
 *
 * *****************  Version 20  *****************
 * User: Smirnov      Date: 1.03.11    Time: 20:11
 * Updated in $/VSNA/VSClient
 * - debug message
 *
 * *****************  Version 19  *****************
 * User: Melechko     Date: 17.02.11   Time: 14:19
 * Updated in $/VSNA/VSClient
 * Add stereo renders
 *
 * *****************  Version 18  *****************
 * User: Sanufriev    Date: 17.01.11   Time: 13:57
 * Updated in $/VSNA/VSClient
 * - fix video memory print
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 13.01.11   Time: 18:57
 * Updated in $/VSNA/VSClient
 * - log renders
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 20.12.10   Time: 12:56
 * Updated in $/VSNA/VSClient
 * - remove D3DXLoadSurfaceFromMemory from d3d renderer
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 16.12.10   Time: 19:40
 * Updated in $/VSNA/VSClient
 * - performance & clean direct3d render
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 19.11.10   Time: 20:07
 * Updated in $/VSNA/VSClient
 * - AV in group conf with DS?
 *
 * *****************  Version 13  *****************
 * User: Sanufriev    Date: 9.11.10    Time: 18:43
 * Updated in $/VSNA/VSClient
 * - shaders from file
 *
 * *****************  Version 12  *****************
 * User: Smirnov      Date: 6.10.10    Time: 15:40
 * Updated in $/VSNA/VSClient
 * - blured image fix for 480x352
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 31.08.10   Time: 17:54
 * Updated in $/VSNA/VSClient
 * - D3D optimization
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 4.06.10    Time: 17:35
 * Updated in $/VSNA/VSClient
 * - Direct3D Render implementation
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 20.07.09   Time: 14:44
 * Updated in $/VSNA/VSClient
 * - debuging bug
 * - fix aec for Vista
 * - change directx version detect
 *
 * *****************  Version 8  *****************
 * User: Smirnov      Date: 5.03.09    Time: 20:08
 * Updated in $/VSNA/VSClient
 * - multimonitor support
 *
 * *****************  Version 7  *****************
 * User: Dront78      Date: 5.03.09    Time: 17:19
 * Updated in $/VSNA/VSClient
 * - added NO_TEARING and ASYNC render flags
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 13.08.08   Time: 14:38
 * Updated in $/VSNA/VSClient
 * - dib render av fix
 *
 * *****************  Version 5  *****************
 * User: Melechko     Date: 6.06.08    Time: 20:16
 * Updated in $/VSNA/VSClient
 * Add transparent render
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 26.05.08   Time: 17:46
 * Updated in $/VSNA/VSClient
 * - black offset removed in SV window
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 14.02.08   Time: 13:30
 * Updated in $/VSNA/VSClient
 * - endpoInt properties
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 23.10.07   Time: 19:13
 * Updated in $/VS2005/VSClient
 * - bugfix #3329
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 10.08.07   Time: 20:23
 * Updated in $/VS2005/VSClient
 * - removed video driver info retrieval every video render
 * reinitialisation (speed-up)
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 2.05.07    Time: 18:15
 * Updated in $/VS2005/VSClient
 * - video driver name obtained (for ep properties)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 103  *****************
 * User: Smirnov      Date: 15.11.06   Time: 18:05
 * Updated in $/VS/VSClient
 * - debug info
 *
 * *****************  Version 102  *****************
 * User: Smirnov      Date: 28.07.06   Time: 18:19
 * Updated in $/VS/VSClient
 * - added HighQuality Resampling
 *
 * *****************  Version 101  *****************
 * User: Smirnov      Date: 29.05.06   Time: 18:03
 * Updated in $/VS/VSClient
 * - zero members in DX Render constructor (bug whith PC not having DX )
 *
 * *****************  Version 100  *****************
 * User: Smirnov      Date: 20.04.06   Time: 18:41
 * Updated in $/VS/VSClient
 * - bugfix#1223
 *
 * *****************  Version 99  *****************
 * User: Smirnov      Date: 16.02.06   Time: 12:15
 * Updated in $/VS/VSClient
 * - new TheadBase class
 * - receiver now can be inited while in conference
 *
 * *****************  Version 98  *****************
 * User: Smirnov      Date: 28.11.05   Time: 17:09
 * Updated in $/VS/VSClient
 * - added Keep Aspect Ratio flag
 *
 * *****************  Version 97  *****************
 * User: Smirnov      Date: 19.10.05   Time: 13:31
 * Updated in $/VS/VSClient
 * - renew DirectX if Display Changed
 *
 * *****************  Version 96  *****************
 * User: Smirnov      Date: 18.10.05   Time: 20:01
 * Updated in $/VS/VSClient
 * bug whith non-standart format in scaling functions (mod(4)!=0!)
 *
 * *****************  Version 95  *****************
 * User: Smirnov      Date: 13.10.05   Time: 14:42
 * Updated in $/VS/VSClient
 * - bug fix # 976 (Emulate DirectX mode)
 *
 * *****************  Version 94  *****************
 * User: Smirnov      Date: 28.09.05   Time: 18:55
 * Updated in $/VS/VSClient
 * - another bug whith proportional scaling resolved
 *
 * *****************  Version 93  *****************
 * User: Smirnov      Date: 26.09.05   Time: 18:12
 * Updated in $/VS/VSClient
 * - Capture frame bounded to 4 pixel on horisontal dimension
 *
 * *****************  Version 92  *****************
 * User: Smirnov      Date: 8.09.05    Time: 15:43
 * Updated in $/VS/VSClient
 * - on 24 bit display DIB render bug fix
 *
 * *****************  Version 91  *****************
 * User: Smirnov      Date: 7.09.05    Time: 17:57
 * Updated in $/VS/VSClient
 * - added proportional scaling (only square pixel)
 *
 * *****************  Version 90  *****************
 * User: Smirnov      Date: 21.06.05   Time: 12:54
 * Updated in $/VS/VSClient
 * - bug fix #747
 *
 * *****************  Version 89  *****************
 * User: Smirnov      Date: 1.06.05    Time: 21:03
 * Updated in $/VS/VSClient
 * bilinear scaling integration in Video project
 *
 * *****************  Version 88  *****************
 * User: Smirnov      Date: 20.01.05   Time: 19:54
 * Updated in $/VS/VSClient
 * Render checking for FS removed
 *
 ****************************************************************************/

/****************************************************************************
 * Includes
 ****************************************************************************/
#include <stdio.h>
#include <algorithm>
#include <windows.h>
#include "VSRender.h"
#include "VSStereoRender.h"
#include "VS_Dmodule.h"
#include "VS_ApplicationInfo.h"
#include "../std/cpplib/VS_RegistryKey.h"
#include "../std/VS_ProfileTools.h"
/// vzo7
//#include "../AddressBookCache/VZOchat7.h"

#ifdef VZOCHAT7
//#include "../AddressBookCache/QGlRenderHolder.h"
#include "../AddressBookCache/QGlRender.h"
//#include "../AddressBookCache/QDxRender.h"
#endif

/// static flag to on precise scaling
BOOL CVideoRenderBase::m_bForceBicubic = 0;
/// stauration in percents
DWORD CVideoRenderBase::m_dwSaturation = 100;
/// flag to DD ude
BOOL CVideoRenderBase::m_bUseDX = false;
/// flag to D3D use
BOOL CVideoRenderBase::m_bUseD3D = false;
/// Stereo Render type
DWORD CVideoRenderBase::m_dwStereoRender = 0;
/// Render type
DWORD CVideoRenderBase::m_dwTypeRender = 0;

HINSTANCE						g_hD3D9 = NULL;
HINSTANCE						g_hD3DX9 = NULL;
LPDIRECT3DCREATE9EX				g_pDirect3DCreate9Ex = NULL;
#ifndef D3D_SHADERS_FROM_FILE
LPD3DXCOMPILESHADER				g_pD3DXCompileShader = NULL;
#else
LPD3DXCOMPILESHADERFROMFILE		g_pD3DXCompileShader = NULL;
#endif
HINSTANCE						g_hD3D10 = NULL;
HINSTANCE						g_hDXGI = NULL;
LPCREATEDXGIFACTORY				g_CreateDXGIFactory = NULL;
LPCREATEDEVICEANDSWAPCHAIN		g_D3D10CreateDeviceAndSwapChain = NULL;
LPD3D10COMPILEEFFECTFROMMEMORY	g_D3D10CompileEffectFromMemory = NULL;
LPD3D10CREATEEFFECTFROMMEMORY	g_D3D10CreateEffectFromMemory = NULL;

/**
****************************************************************************
 * \date    27-11-2002
 ******************************************************************************/
CVideoRenderBase::CVideoRenderBase(CVSInterface* pParentInterface):
CVSInterface( "Render",pParentInterface)
{
	m_renderTime = 0;
	m_bNewFrame = FALSE;
	m_pVSVideoProc = new VS_VideoProc;
	m_pBufferIn = 0;
	m_pBufferConvIn = 0;
	m_pBufferMirrorIn = 0;
	m_pBufferSaturated = 0;
	m_pBufferScaled = 0;
	m_pBufferConvOut = 0;
	m_pBufferConvOutTmp = 0;
	m_dwVRUse = 0;
	m_IsValid = false;
	m_height = 0;
	m_width = 0;
	m_RenderMode = VRM_UNDEF;
	m_hwnd = 0;
	m_renderFrameSizeMB = 0;

	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	m_bKeepAspectRatio = 1;
	key.GetValue(&m_bKeepAspectRatio, 4, VS_REG_INTEGER_VT, "KeepAspectRatio");

	VS_ProcType ptype;
	if		(m_pVSVideoProc->Cpu()&VS_CPU_SSE2)
		ptype = VS_PrType_SSE2;
	else if (m_pVSVideoProc->Cpu()&VS_CPU_MMX)
		ptype = VS_PrType_MMX;
	else
		ptype = VS_PrType_Common;
	m_ResizeMode = RM_DEFAULT;
	key.GetValue(&m_ResizeMode, 4, VS_REG_INTEGER_VT, "ResizeMode");
	if (m_ResizeMode < RM_DEFAULT || m_ResizeMode > RM_HQBICUBIC)
		m_ResizeMode = RM_DEFAULT;
	if (m_ResizeMode==RM_BICUBIC || m_ResizeMode==RM_LANCZOS)
		m_HQRes = new VS_HQResampling(ptype, VS_FastMode);
	else if (m_ResizeMode==RM_HQBICUBIC)
		m_HQRes = new VS_HQResampling(ptype, VS_HQMode);
	else
		m_HQRes = 0;

	m_DisplayName.width = 0;
	m_DisplayName.height = 0;
	memset(&m_DisplayName.rName, 0, sizeof(m_DisplayName.rName));
	m_DisplayName.update = true;

	m_DisplayLogo.width = 0;
	m_DisplayLogo.height = 0;
	memset(&m_DisplayLogo.rName, 0, sizeof(m_DisplayLogo.rName));
	m_DisplayLogo.update = false;

	m_bDrawLogo = false;

	m_TypeRender = VR_NONE;
	m_dwStereoRender = 0;
	key.GetValue(&m_dwStereoRender, 4, VS_REG_INTEGER_VT, "RcvStereo");

	int flip = 1;
	key.GetValue(&flip, 4, VS_REG_INTEGER_VT, "FlipFrame");
	m_bFlipFrame = (flip > 0);
	m_bSelfView = true;

#ifdef _VIRTUAL_OUT
  m_bVirtualOut=false;
#endif
}

/**
****************************************************************************
 * \date    27-11-2002
 ******************************************************************************/
CVideoRenderBase::~CVideoRenderBase()
{
#ifdef _VIRTUAL_OUT
	if(m_bVirtualOut)
		SetVirtualOut(false,NULL,NULL);
#endif
	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
	int flip = (int)m_bFlipFrame;
	key.SetValue(&flip, 4, VS_REG_INTEGER_VT, "FlipFrame");
	CleanBuffers();
	delete m_pVSVideoProc; m_pVSVideoProc = 0;
	if (m_HQRes) delete m_HQRes; m_HQRes = 0;
}

void CVideoRenderBase::Open()
{
	m_dwTypeRender = VR_DIRECT_DRAW;
	OSVERSIONINFOA osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	if (GetVersionEx(&osvi)) {
		if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && osvi.dwMajorVersion >= 6) {
			m_dwTypeRender = VR_DIRECT_3D10;
		}
	}
	VS_RegistryKey key(true, REG_CurrentConfiguratuon);
	key.GetValue(&m_dwTypeRender, 4, VS_REG_INTEGER_VT, "TypeVR");
	if (m_dwTypeRender == VR_DIRECT_3D10) g_hD3D10 = LoadLibrary("d3d10.dll");
	if (g_hD3D10) {
		DTRACE(VSTM_VRND, "load d3d10.dll");
		(FARPROC &)g_D3D10CreateDeviceAndSwapChain = GetProcAddress(g_hD3D10, "D3D10CreateDeviceAndSwapChain");
		(FARPROC &)g_D3D10CompileEffectFromMemory = GetProcAddress(g_hD3D10, "D3D10CompileEffectFromMemory");
		(FARPROC &)g_D3D10CreateEffectFromMemory = GetProcAddress(g_hD3D10, "D3D10CreateEffectFromMemory");
		g_hDXGI = LoadLibrary("dxgi.dll");
		if (g_hDXGI) {
			DTRACE(VSTM_VRND, "load dxgi.dll");
			(FARPROC &)g_CreateDXGIFactory = GetProcAddress(g_hDXGI, "CreateDXGIFactory");
		}
	}
	if (!g_hD3D10 || !g_hDXGI) {
		DTRACE(VSTM_VRND, "no Direct3D10");
		if (m_dwTypeRender == VR_DIRECT_3D10) m_dwTypeRender = VR_DIRECT_3D9;
		if (m_dwTypeRender == VR_DIRECT_3D9) g_hD3D9 = LoadLibrary("d3d9.dll");
		if (g_hD3D9) {
			DTRACE(VSTM_VRND, "load d3d9.dll");
			(FARPROC &)g_pDirect3DCreate9Ex = GetProcAddress(g_hD3D9, "Direct3DCreate9Ex");
			int nDXSdkRelease = 0;
			CString strD3DX9Version;
			// Try to load latest DX9 available
			for (int i = 42; i > 23; i--) {
				if (i != 33) {	// Prevent using DXSDK April 2007 (crash sometimes during shader compilation)
					strD3DX9Version.Format(_T("d3dx9_%d.dll"), i);
					g_hD3DX9 = LoadLibrary (strD3DX9Version);
					if (g_hD3DX9) {
						nDXSdkRelease = i;
						break;
					}
				}
			}
			if (g_hD3DX9) {
				DTRACE(VSTM_VRND, "load %s", strD3DX9Version.GetBuffer());
#ifndef D3D_SHADERS_FROM_FILE
				(FARPROC&)g_pD3DXCompileShader = GetProcAddress(g_hD3DX9, "D3DXCompileShader");
#else
				(FARPROC&)g_pD3DXCompileShader = GetProcAddress(g_hD3DX9, "D3DXCompileShaderFromFileA");
#endif
			}
			if (g_pD3DXCompileShader == NULL || g_pDirect3DCreate9Ex == NULL) {
				DTRACE(VSTM_VRND, "no Direct3D9");
				Close();
			}
		}
	}

}

void CVideoRenderBase::Close()
{
	if (g_hD3D10) FreeLibrary(g_hD3D10); g_hD3D10 = NULL;
	if (g_hDXGI) FreeLibrary(g_hDXGI); g_hDXGI = NULL;
	if (g_hD3D9) FreeLibrary(g_hD3D9); g_hD3D9 = NULL;
	if (g_hD3DX9) FreeLibrary(g_hD3DX9); g_hD3DX9 = NULL;
	m_bUseD3D = false;
	m_bUseDX = false;
}

CVideoRenderBase* CVideoRenderBase::RetrieveVideoRender(HWND hwnd, CVSInterface* pParentInterface, unsigned int iStereo)
{
	CVideoRenderBase *pRender = NULL;

#ifdef VZOCHAT7

	pRender = new CQGlRender(pParentInterface);

#else

	if (iStereo > 0 && m_dwStereoRender > 0) {
		switch(m_dwStereoRender){
			case 1:
		      pRender = new COpenGLRenderAnaglyph(pParentInterface);
			  break;
			case 2:
		      pRender = new COpenGLRenderInterlaced(pParentInterface);
			  break;
			case 3:
		      pRender = new COpenGLRender(pParentInterface);
			  break;
			case 4:
				pRender = new COpenGLRenderActive(pParentInterface);
				break;
			case 5:
				pRender = new COpenGLRenderActive2(pParentInterface);
				break;
		}
	} else {
		if (CTransDIBRender::m_StretchFunc) {
			pRender = new CTransDIBRender(pParentInterface);
		} else {
			if (g_hD3D10 && g_hDXGI) pRender = new VS_Direct3D10Render(pParentInterface, hwnd);
			if (!m_bUseD3D || m_dwTypeRender != VR_DIRECT_3D10) {
				delete pRender; pRender = NULL;
				if (g_pD3DXCompileShader && g_pDirect3DCreate9Ex) pRender = new VS_Direct3DRender(pParentInterface, hwnd);
				if (!m_bUseD3D) {
					delete pRender; pRender = NULL;
					pRender = new CDirectXRender(pParentInterface);
					if (!m_bUseDX) {
						delete pRender; pRender = NULL;
						pRender = new CDIBRender(pParentInterface);
					}
				}
			}
		}
	}

#endif

	return pRender;
}

void CVideoRenderBase::EnableBorders(bool enable)
{
	m_DrawBorders = enable;
}

void CVideoRenderBase::SetBordersAlpha(uint8_t alpha)
{
	m_BorderAlpha = alpha;
}

void CVideoRenderBase::DrawBorders()
{
	const uint8_t bgY = 140;
	const uint8_t bgU = 186;
	const uint8_t bgV = 75;

	uint8_t* planeY = m_pBufferIn;
	uint8_t* planeU = planeY + m_width * m_height;
	uint8_t* planeV = planeU + m_width * m_height / 4;

	// up top and bottom

	for (int y = 0; y < m_BorderTemplateY.size(); y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			uint8_t alpha = m_BorderTemplateY[y] * m_BorderAlpha / 255;

			int idx = y * m_width + x;
			planeY[idx] = bgY * alpha / 255 + planeY[idx] * (255 - alpha) / 255;

			idx = ((m_height - 1) - y) * m_width + x;
			planeY[idx] = bgY * alpha / 255 + planeY[idx] * (255 - alpha) / 255;
		}
	}


	for (int y = 0; y < m_BorderTemplateUV.size(); y++)
	{
		for (int x = 0; x < m_width / 2; x++)
		{
			uint8_t alpha = m_BorderTemplateUV[y] * m_BorderAlpha / 255;

			int idx = y * (m_width / 2) + x;
			planeU[idx] = bgU * alpha / 255 + planeU[idx] * (255 - alpha) / 255;
			planeV[idx] = bgV * alpha / 255 + planeV[idx] * (255 - alpha) / 255;

			idx = (m_height / 2 - 1 - y) * m_width / 2 + x;
			planeU[idx] = bgU * alpha / 255 + planeU[idx] * (255 - alpha) / 255;
			planeV[idx] = bgV * alpha / 255 + planeV[idx] * (255 - alpha) / 255;
		}
	}

	// left and right

	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_BorderTemplateY.size(); x++)
		{
			uint8_t alpha = m_BorderTemplateY[x] * m_BorderAlpha / 255;

			int idx = y * m_width + x;
			planeY[idx] = bgY * alpha / 255 + planeY[idx] * (255 - alpha) / 255;

			idx = y * m_width + (m_width - 1 - x);
			planeY[idx] = bgY * alpha / 255 + planeY[idx] * (255 - alpha) / 255;
		}
	}

	for (int y = 0; y < m_height / 2; y++)
	{
		for (int x = 0; x < m_BorderTemplateUV.size(); x++)
		{
			uint8_t alpha = m_BorderTemplateUV[x] * m_BorderAlpha / 255;

			int idx = y * (m_width / 2) + x;
			planeU[idx] = bgU * alpha / 255 + planeU[idx] * (255 - alpha) / 255;
			planeV[idx] = bgV * alpha / 255 + planeV[idx] * (255 - alpha) / 255;

			idx = y * (m_width / 2) + (m_width / 2 - 1 - x);
			planeU[idx] = bgU * alpha / 255 + planeU[idx] * (255 - alpha) / 255;
			planeV[idx] = bgV * alpha / 255 + planeV[idx] * (255 - alpha) / 255;
		}
	}
}

/**
****************************************************************************
 * \brief Initializate video render.
 *
 * \return 0 (if everything is OK)
 *
 * \param hwnd				- handler to window to be drawn;
 * \param pBuffer			- pointer to get image from;
 * \param pCm				- pointer to image color mode descriptor;
 *
 * \date    18-11-2002
 ******************************************************************************/
int CVideoRenderBase::iInitRender(HWND hwnd,unsigned char *pBuffer, CColorMode *pCm, bool bSelfView)
{
	VS_AutoLock lock(this);
	Release();
	pCm->ColorModeToBitmapInfoHeader(&m_biFmtIn);
	if (m_biFmtIn.biCompression == FCC_I42S && m_dwStereoRender == 0) {
		CColorMode cm;
		cm.SetColorMode(NULL, cm.I420, m_biFmtIn.biHeight / 2, m_biFmtIn.biWidth);
		cm.ColorModeToBitmapInfoHeader(&m_biFmtDraw);
		m_height = m_biFmtDraw.biHeight;
		m_width = m_biFmtDraw.biWidth;
	} else {
		pCm->ColorModeToBitmapInfoHeader(&m_biFmtDraw);
		m_height = m_biFmtIn.biHeight;
		m_width = m_biFmtIn.biWidth;
	}
	if (m_height*m_width==0) return 1;
	m_pBufferIn = pBuffer;
    memset(m_pBufferIn, 0 , m_biFmtIn.biHeight * m_biFmtIn.biWidth);
    memset(m_pBufferIn + m_biFmtIn.biHeight * m_biFmtIn.biWidth, 0x80, m_biFmtIn.biHeight  *m_biFmtIn.biWidth / 2);
	m_bSelfView = bSelfView;

	int borderSize = std::min(m_width, m_height) * 5 / 100;

	m_BorderTemplateY.clear();
	m_BorderTemplateUV.clear();

	for (int i = 0; i < borderSize; i++)
		m_BorderTemplateY.push_back(i * (-255) / borderSize + 255);

	for (int i = 0; i < borderSize / 2; i++)
		m_BorderTemplateUV.push_back(i * (-255) / (borderSize / 2) + 255);

	return SetMode(VRM_DEFAULT, hwnd);
}

/**
****************************************************************************
 * \brief Check properly initialization.
 *
 * \return true if initialized ok
 * \date    13-02-2006
 ******************************************************************************/
bool CVideoRenderBase::IsInited() {
	return m_height && m_width;
}

/**
****************************************************************************
 * \brief Release video render.
 *
 * \date    04-11-2004
 ******************************************************************************/
void CVideoRenderBase::Release()
{
	VS_AutoLock lock(this);
	m_IsValid = false;
	CleanBuffers();
	m_bNewFrame = FALSE;
	m_height = 0;
	m_width = 0;
	m_RenderMode = VRM_UNDEF;
	m_hwnd = 0;
}

void HelpTraceMess(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef _DEBUG
	const char* ops[5] = { "SIZE_RESTORED", "SIZE_MINIMIZED", "SIZE_MAXIMIZED", "SIZE_MAXSHOW", "SIZE_MAXHIDE" };
	switch (uMsg)
	{
	case WM_DISPLAYCHANGE:
		DTRACE(VSTM_VRND, "h==%p, %s, d=%d, w=%d, h=%d", hwnd, "WM_DISPLAYCHANGE", wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_USER + 10:
		DTRACE(VSTM_VRND, "h==%p, %s", hwnd, "WM_USER + 10");
		break;
	case WM_SIZE:
		DTRACE(VSTM_VRND, "h==%p, %s, ops = %s, w=%d, h=%d", hwnd, "WM_SIZE", ops[wParam], LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOVE:
		DTRACE(VSTM_VRND, "h==%p, %s, x=%d, y=%d", hwnd, "WM_MOVE", LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_NCPAINT:
		DTRACE(VSTM_VRND, "h==%p, %s, wp=%p", hwnd, "WM_NCPAINT", wParam);
		break;
	}
#else
	switch (uMsg)
	{
	case WM_DISPLAYCHANGE:
	case WM_USER + 10:
	case WM_SIZE:
	case WM_MOVE:
	case WM_NCPAINT:
		DTRACE(VSTM_VRND, "h==%p, mess=%p, wp=%p, lp=%p", hwnd, uMsg, wParam, lParam);
		break;
	}
#endif // DEBUG
}

/**
*****************************************************************************
 * \brief video Render window function that processes messages sent to a window
 * \return default value for catched message
 *
 * \param hwnd				- [in] Handle to the window.
 * \param uMsg				- [in] Specifies the message.
 * \param wParam			- [in] Specifies additional message information.
 *		  The contents of this parameter depend on the value of the uMsg parameter.
 * \param lParam			- [in] Specifies additional message information.
 *		  The contents of this parameter depend on the value of the uMsg parameter.
 *
 *  \date    05-11-2002
 ******************************************************************************/
LRESULT CALLBACK CVideoRenderBase::WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	CVideoRenderBase **ppVideoRenderBase = (CVideoRenderBase**)GetWindowLongPtr(hwnd, GWL_USERDATA);
	HelpTraceMess(hwnd, uMsg, wParam, lParam);
	DWORD currTime;
	if (ppVideoRenderBase!=NULL)
	switch(uMsg)
	{
	case WM_DISPLAYCHANGE:
		(*ppVideoRenderBase)->m_RenderMode = VRM_UNDEF;
		(*ppVideoRenderBase)->SetMode(VRM_DEFAULT, hwnd);
	case WM_USER+10:
		return (*ppVideoRenderBase)->DrawFrame(hwnd);
	case WM_SIZE:
		if (HIWORD(lParam)<=0 || LOWORD(lParam)<=0)
			return 0;
		if ((*ppVideoRenderBase)->m_ResizeMode!=RM_DEFAULT || (*ppVideoRenderBase)->IsNeedResetOnSize())
			(*ppVideoRenderBase)->m_RenderMode = VRM_UNDEF;
		(*ppVideoRenderBase)->SetMode(VRM_DEFAULT, hwnd);
		(*ppVideoRenderBase)->CheckMonitor(hwnd);
		(*ppVideoRenderBase)->m_renderTime = timeGetTime();
		(*ppVideoRenderBase)->m_bNewFrame = TRUE;
		return (*ppVideoRenderBase)->DrawFrame(hwnd);
	case WM_MOVE:
		(*ppVideoRenderBase)->CheckMonitor(hwnd);
	case WM_NCPAINT:
		currTime = timeGetTime();
		if (currTime - (*ppVideoRenderBase)->m_renderTime >= VR_REFRESH_TIME) {
			(*ppVideoRenderBase)->m_renderTime = currTime;
			return (*ppVideoRenderBase)->DrawFrame(hwnd);
		}
	}
	return	0;
}

/**
****************************************************************************
 * \brief Set display name
 *
 * \param name					- display name to set;
 *
 * \date    15-03-2011
 ******************************************************************************/
void CVideoRenderBase::SetDisplayName(wchar_t *name)
{
	VS_AutoLock lock(this);

	if (!name || !*name) {
		m_DisplayName.name.erase();
	} else {
		m_DisplayName.name = name;
	}
	m_DisplayName.update = true;
}

/**
****************************************************************************
 * \brief Color spase convertor
 * \return TRUE if everything is OK
 *
 * \param dst					- destination image;
 * \param pitch					- destination images pitch;
 *
 *  \date    05-11-2002
 ******************************************************************************/
bool CVideoRenderBase::ConvertColorSpaceOut(BYTE* dst, DWORD dstW)
{
	int srcH = m_biFmtDraw.biHeight;
	int srcW = m_biFmtDraw.biWidth;
	int Size = srcW*srcH;
	bool ret = false;

	BYTE *pY, *pU, *pV;
	pY = m_pBufferScaled;
	pU = m_pBufferScaled+Size;
	pV = m_pBufferScaled+Size*5/4;

	switch(m_biFmtDraw.biCompression)
	{
	case FCC_YUY2:
		ret = m_pVSVideoProc->ConvertI420ToYUY2(pY, pU, pV, dst, srcW, srcH, dstW);
		break;
	case FCC_UYVY:
		ret = m_pVSVideoProc->ConvertI420ToUYVY(pY, pU, pV, dst, srcW, srcH, dstW);
		break;
	case BI_RGB:
		if (m_biFmtDraw.biBitCount==24)
			ret = m_pVSVideoProc->ConvertI420ToBMF24(pY, pU, pV, dst, srcW, srcH, dstW);
		else {
			if (m_TypeRender == VR_DIRECT_3D9 || m_TypeRender == VR_DIRECT_3D10) {
				ret = m_pVSVideoProc->ConvertI420ToBMF32_Vflip(pY, pU, pV, dst, srcW, srcH, dstW);
			} else {
				ret = m_pVSVideoProc->ConvertI420ToBMF32(pY, pU, pV, dst, srcW, srcH, dstW);
			}
		}
		break;
	}
	return ret;
}

/**
****************************************************************************
 * \brief Image Dimension convertor
 * \return TRUE if everything is OK
 *
 *  \date    03-12-2002
 ******************************************************************************/
bool CVideoRenderBase::ResampleImage()
{
	if (m_ResizeMode==RM_BILINEAR) {
		return m_pVSVideoProc->ResampleI420(m_pBufferSaturated, m_pBufferScaled,
											m_width, m_height,
											m_biFmtDraw.biWidth, m_biFmtDraw.biHeight);
	}
	else if (m_ResizeMode==RM_BICUBIC || m_ResizeMode==RM_HQBICUBIC ) {
		return m_HQRes->ResamplingYUV(m_pBufferSaturated, m_pBufferScaled)== VS_ErrSts_NoError;
	}
	else {
		switch(m_RenderMode)
		{
		case VRM_1d5X1:
			return m_pVSVideoProc->ResampleUp_1d5(m_pBufferSaturated, m_pBufferScaled,
													m_width, m_height);
		case VRM_2X1:
			return m_pVSVideoProc->ResampleUp_2(m_pBufferSaturated, m_pBufferScaled,
													m_width, m_height);
		case VRM_1X2:
			return m_pVSVideoProc->ResampleI420(m_pBufferSaturated, m_pBufferScaled,
													m_width, m_height,
													m_biFmtDraw.biWidth, m_biFmtDraw.biHeight);
		}
	}
	return false;
}

/**
****************************************************************************
 * \brief Dither the scaled image (only Y)
 * \return TRUE
 *
 *  \date    23-03-2004
 ******************************************************************************/
bool CVideoRenderBase::Dither()
{
	return m_pVSVideoProc->DitherI420(m_pBufferScaled, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight, 1);
}

#ifdef _VIRTUAL_OUT
/**
****************************************************************************
 * \brief out image to virtual video driver
 * \return none
 *
 *  \date    07-08-2003
 ******************************************************************************/
void CVideoRenderBase::VirtualOut(){
	if(m_bVirtualOut)
		if(CheckState)
			if(CheckState(m_Param))
				memcpy(m_pMMMem, m_pBufferConvIn, m_width*m_height*3/2);
}

/**
****************************************************************************
 * \brief set virtual video driver flag
 * \return none
 *
 *  \date    07-08-2003
 ******************************************************************************/
void CVideoRenderBase::SetVirtualOut(bool bVirtualOut,void*pProc,void *pParam){
	if(bVirtualOut&&(!m_bVirtualOut)){
		m_Param=pParam;
		CheckState=(bool(__cdecl*)(void*))pProc;
		m_hMMFile=CreateFileMapping (INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE|SEC_COMMIT, 0 , m_width*m_height*3/2, "VisicronVirtualVideoDriver");
		if(m_hMMFile==INVALID_HANDLE_VALUE)
			bVirtualOut=false;
		else{
			m_pMMMem=(unsigned char*)MapViewOfFile(m_hMMFile,FILE_MAP_ALL_ACCESS,0,0,0);
			if(m_pMMMem==NULL){
				CloseHandle(m_hMMFile);
				bVirtualOut=false;
			}
		}
		m_bVirtualOut=bVirtualOut;
	}
	else
		if(!bVirtualOut&&(m_bVirtualOut)){
			m_bVirtualOut=false;
			UnmapViewOfFile(m_pMMMem);
			CloseHandle(m_hMMFile);
		}
}
#endif

/**
****************************************************************************
 * \brief Image saturation
 * \return TRUE if everything is OK
 *
 *  \date    07-02-2002
 ******************************************************************************/
bool CVideoRenderBase::Saturate()
{
	memcpy(m_pBufferSaturated, m_pBufferMirrorIn, m_width*m_height*3/2);
	return m_pVSVideoProc->SaturateI420(m_pBufferSaturated, m_width, m_height, m_dwSaturation);
}

/**
****************************************************************************
 * \brief Color spase convertor
 * \return TRUE if everything is OK
 *
 * \param dst					- destination image;
 * \param pitch					- destination images pitch;
 *
 *  \date    05-11-2002
 ******************************************************************************/
bool CVideoRenderBase::ConvertColorSpaceIn()
{
	bool ret = false;
	int Size = m_height*m_width;
	BYTE *pY, *pU, *pV;
	pY = m_pBufferConvIn;
	pU = m_pBufferConvIn+Size;
	pV = m_pBufferConvIn+Size*5/4;

	switch(m_biFmtIn.biCompression)		// input format
	{
	case FCC_I420:
	case FCC_IYUV:
		ret = true;
		break;
	case FCC_YV12:
		ret = m_pVSVideoProc->ConvertYV12ToI420(m_pBufferIn, pY, pU, pV, m_width, m_height, m_width);
		break;
	case FCC_YUY2:
		ret = m_pVSVideoProc->ConvertYUY2ToI420(m_pBufferIn, pY, pU, pV, m_width*2, m_height, m_width);
		break;
	case FCC_UYVY:
		ret = m_pVSVideoProc->ConvertUYVYToI420(m_pBufferIn, pY, pU, pV, m_width*2, m_height, m_width);
		break;
	case BI_RGB:
		if (m_biFmtIn.biBitCount==24)
			ret = m_pVSVideoProc->ConvertBMF24ToI420(m_pBufferIn, pY, pU, pV, m_width*3, m_height, m_width);
		else
			ret = m_pVSVideoProc->ConvertBMF32ToI420(m_pBufferIn, pY, pU, pV, m_width*4, m_height, m_width);
		break;
	case FCC_I42S:
		ret = m_pVSVideoProc->ConvertI42SToI420(m_pBufferIn, pY, pU, pV, m_width, m_height, m_width);
    break;
	}
	return ret;
}

/**
****************************************************************************
 * \brief Mirrors an image about a vertical axis
 * \return TRUE if everything is OK
 *
 * \param dst					- destination image;
 * \param pitch					- destination images pitch;
 *
 *  \date    05-11-2002
 ******************************************************************************/
bool CVideoRenderBase::MirrorVertical()
{
	bool ret = m_pVSVideoProc->MirrorVertical(m_pBufferConvIn, m_pBufferMirrorIn, m_width, m_height);
	return ret;
}

void CVideoRenderBase::CleanBuffers()
{
	if (m_bUseD3D)
		delete[] m_pBufferConvOutTmp;
	m_pBufferConvOutTmp = 0;
	if (m_dwVRUse&VR_USE_CONVOUT)
		delete[] m_pBufferConvOut;
	m_pBufferConvOut = 0;
	if (m_dwVRUse&VR_USE_SCALE)
		delete[] m_pBufferScaled;
	m_pBufferScaled = 0;
	if (m_dwVRUse&VR_USE_SATURATE)
		delete[] m_pBufferSaturated;
	m_pBufferSaturated = 0;
	if (m_dwVRUse&VR_USE_FLIP)
		delete[] m_pBufferMirrorIn;
	m_pBufferMirrorIn = 0;
	if (m_dwVRUse&VR_USE_CONVIN)
		delete[] m_pBufferConvIn;
	m_pBufferConvIn = 0;
	m_dwVRUse = 0;
}

int CVideoRenderBase::CaptureFrame(unsigned char* frame, int x, int y)
{
	VS_AutoLock lock(this);
	if (!frame || x<=0 || y<=0)
		return -2;
	if (!m_IsValid) {
		memset(frame, 0, x*y*4);
		return -1;
	}
	if ((x|y)&0x7)
		return -2; // %8

	unsigned char *pScaled = 0;

	int Size;
	BYTE *pY, *pU, *pV;
	bool ret = true;

	if (x != m_width || y != m_height) {
		pScaled = new unsigned char[x*y*3/2*2];

		float xk = (float)m_width / x;
		float yk = (float)m_height / y;
		int new_x = x;
		int new_y = y;
		if (m_bKeepAspectRatio && xk > 0.f && yk > 0.f) {
			if (xk>yk)
				new_y = (int)(y*yk/xk + 0.5)&~7;
			else if (yk>xk)
				new_x = (int)(x*xk/yk + 0.5)&~7;
		}

		ret = m_pVSVideoProc->ResampleI420(m_pBufferMirrorIn, pScaled + x*y * 3 / 2, m_width, m_height, new_x, new_y);
		if (ret)
			ret = m_pVSVideoProc->ClipI420(pScaled+x*y*3/2, pScaled, new_x, new_y, x, y, 5);
		if (!ret) {
			if (pScaled) delete[] pScaled;
			return -3;
		}
	}
	else {
		pScaled = m_pBufferMirrorIn;
	}

	Size = x*y; pY = pScaled; pU = pScaled+Size; pV = pScaled+Size*5/4;
	ret = m_pVSVideoProc->ConvertI420ToBMF32(pY, pU, pV, frame, x, y, x*4);
	if (pScaled && pScaled!=m_pBufferMirrorIn) delete[] pScaled;
	return ret? 0:-4;
}
/***************************************/
int CVideoRenderBase::ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar)
{
	_variant_t* var=(_variant_t*)pVar;
	if (strcmp(pSection, "GetCapability")==0) {
		if (VS_OPERATION==GET_PARAM) {
			*var=m_bCanFS;
			return VS_INTERFACE_OK;
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (strcmp(pSection, "GetImage")==0) {
		if (VS_OPERATION==GET_PARAM) {
			if (var->vt==(VT_ARRAY|VT_VARIANT)) {
				SAFEARRAY *psa=var->parray;
				VARIANT vr;
				long l=0;
				unsigned char *pBuff;
				SafeArrayGetElement(psa,&l,&vr);
				pBuff=(unsigned char *)(vr.ulVal);
				l++;
				SafeArrayGetElement(psa,&l,&vr);
				if(vr.boolVal)
					CaptureFrame(pBuff, 160, 120);
				else
					CaptureFrame(pBuff, 320, 240);
				return VS_INTERFACE_OK;
			}
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (strcmp(pSection, "GetImage2")==0) {
		if (VS_OPERATION==GET_PARAM) {
			bool res = false;
			VARIANT *vars = 0;
			int num = ExtractVars(vars, var);
			if (num >= 3) {
				unsigned char *pBuff = (unsigned char *)vars[0].ulVal;
				unsigned x = vars[1].ulVal;
				unsigned y = vars[2].ulVal;
				res = CaptureFrame(pBuff, x, y)==0;
			}
			if (num > 0) delete[] vars;
			return res ? VS_INTERFACE_OK : VS_INTERFACE_INTERNAL_ERROR;
		}
		else if (VS_OPERATION==RUN_COMMAND) {
			*var = (unsigned long)((m_biFmtIn.biWidth<<16) | m_biFmtIn.biHeight);
			return VS_INTERFACE_OK;
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (strcmp(pSection, "VideoName")==0) {
		if (VS_OPERATION==SET_PARAM) {
			SetDisplayName((_bstr_t)var);
			return VS_INTERFACE_OK;
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	else if (strcmp(pSection, "FlipFrame")==0) {
		if (VS_OPERATION == SET_PARAM) {
			bool bFlip = !!(*var);
			SetFlipFrame(bFlip);
			return VS_INTERFACE_OK;
		}
		else if (VS_OPERATION == GET_PARAM) {
			*var = (int)m_bFlipFrame;
			return VS_INTERFACE_OK;
		}
		return VS_INTERFACE_INTERNAL_ERROR;
	}
	return VS_INTERFACE_NO_FUNCTION;
}
/**
****************************************************************************
 * \brief Determine Render Mode according window client area
 * \return Render Mode
 * \param hwnd					- window handler to render in;
 *
 *  \date    04-04-2004
 ******************************************************************************/
eVideoRenderMode CVideoRenderBase::DetermineMode(HWND hwnd)
{
	RECT rdim;
	eVideoRenderMode Mode;
	GetClientRect(hwnd, &rdim);
	double x = (double)(rdim.right-rdim.left)/m_width;
	double y = (double)(rdim.bottom-rdim.top)/m_height;
	double k = x;
	if (y<x) k = y;
	if		(k < 1.0)
		Mode = VRM_1X2;
	else if (k < 1.5)
		Mode = VRM_1X1;
	else if (k < 2.0)
		Mode = VRM_1d5X1;
	else
		Mode = VRM_2X1;
	return Mode;
}
/**
****************************************************************************
 * \brief Set Mirror Frame
 * \return
 * \param hwnd					- flip frame;
 *
 *  \date    04-04-2004
 ******************************************************************************/
void CVideoRenderBase::SetFlipFrame(bool flip)
{
	VS_AutoLock lock(this);

	if (flip) {
		if (!(m_dwVRUse & VR_USE_FLIP) && m_bSelfView) {
			m_pBufferMirrorIn = new BYTE[m_width*m_height*3/2];
			m_dwVRUse |= VR_USE_FLIP;
		}
	} else {
		if (m_dwVRUse & VR_USE_FLIP) {
			delete [] m_pBufferMirrorIn; m_pBufferMirrorIn = 0;
			m_dwVRUse &= ~VR_USE_FLIP;
			m_pBufferMirrorIn = m_pBufferConvIn;
		}
	}

	VS_RegistryKey key(true, REG_CurrentConfiguratuon, false, true);
	int isFlipFrame = flip ? 1 : 0;
	key.SetValue(&isFlipFrame, 4, VS_REG_INTEGER_VT, "FlipFrame");

	m_bFlipFrame = flip;
}


/******************************************************************************
 * CDIBRender
 ******************************************************************************/

/**
****************************************************************************
 * \date    18-11-2002
 ******************************************************************************/
CDIBRender::CDIBRender(CVSInterface* pParentInterface):
CVideoRenderBase(pParentInterface)
{
	m_displaydepth = 0;
	m_bUseDX = 0;
	m_bCanFS = 1;
	m_pBufferDIBDraw = 0;
	m_TypeRender = VR_DIB;
}

/**
*****************************************************************************
 * \date    18-11-2002
 *******************************************************************************/
CDIBRender::~CDIBRender()
{
	if (m_pBufferDIBDraw)
		delete[] m_pBufferDIBDraw;
	m_pBufferDIBDraw = 0;
}


/**
*****************************************************************************
 * \brief Set DIB Render Mode according to eVideoRenderMode type
 * \return 0 if everything is OK and error number else
 *
 * \param Mode					- new Render Mode ;
 * \param hwnd					- window handler to render in;
 *
 * \date    03-12-2002
 ******************************************************************************/
int CDIBRender::SetMode(eVideoRenderMode Mode, HWND hwnd)
{
	VS_AutoLock lock(this);
	if (Mode== VRM_DEFAULT)
		Mode = DetermineMode(hwnd);
	if (m_RenderMode!= VRM_UNDEF)
		if (m_RenderMode==Mode)
			return 0;

	// begin reinitialisation
	m_IsValid = false;

	HDC hdc = GetDC(NULL);
	m_displaydepth = GetDeviceCaps(hdc, BITSPIXEL);
	if (m_displaydepth!=24) m_displaydepth=32;	/// 24 and 32 bit are supported
	ReleaseDC(NULL, hdc);

	m_biFmtDraw.biCompression = BI_RGB;			/// forse parametrers, output always RGB in DIB
	m_biFmtDraw.biBitCount = m_displaydepth;	/// try convert to compatible bit depth

	CleanBuffers();

	switch(m_biFmtIn.biCompression)		// input format
	{
	case FCC_I420:
	case FCC_IYUV:
		m_pBufferConvIn = m_pBufferIn;
		break;
	case FCC_I42S:
		m_pBufferConvIn = m_pBufferIn;
		if (m_dwStereoRender > 0) break;
	case FCC_YV12:
	case FCC_YUY2:
	case FCC_UYVY:
	case BI_RGB:
		m_pBufferConvIn = new BYTE[m_width*m_height*3/2];
		m_dwVRUse|=VR_USE_CONVIN;
		break;
	}

	m_pBufferMirrorIn = m_pBufferConvIn;
	if (m_bSelfView && m_bFlipFrame) {
		m_pBufferMirrorIn = new BYTE[m_width*m_height*3/2];
		m_dwVRUse |= VR_USE_FLIP;
	}

	// always use saturate
	m_pBufferSaturated = new BYTE[m_width*m_height*3/2];
	m_dwVRUse|=VR_USE_SATURATE;

	m_biFmtDraw.biWidth = m_width;
	m_biFmtDraw.biHeight = m_height;
	m_pBufferScaled = m_pBufferSaturated;

	if (m_ResizeMode==RM_DEFAULT) {
		switch(Mode)
		{
		case VRM_1d5X1:
			if (m_bForceBicubic) {
				m_biFmtDraw.biWidth = m_width*3/2;
				m_biFmtDraw.biHeight = m_height*3/2;
				m_pBufferScaled = new BYTE [m_biFmtDraw.biWidth*m_biFmtDraw.biHeight*3/2];
				m_dwVRUse|=VR_USE_SCALE;
			}
			break;
		case VRM_2X1:
			if (m_bForceBicubic) {
				m_biFmtDraw.biWidth = m_width*2;
				m_biFmtDraw.biHeight = m_height*2;
				m_pBufferScaled = new BYTE [m_biFmtDraw.biWidth*m_biFmtDraw.biHeight*3/2];
				m_dwVRUse|=VR_USE_SCALE;
			}
			break;
		}
	}
	else if (m_bForceBicubic) {
		GetClientRect(hwnd, &m_rPic);
		m_biFmtDraw.biWidth = (m_rPic.right - m_rPic.left+3)&~3;
		m_biFmtDraw.biHeight = (m_rPic.bottom - m_rPic.top+3)&~3;
		m_pBufferScaled = new BYTE [m_biFmtDraw.biWidth*m_biFmtDraw.biHeight*3/2];
		m_dwVRUse|=VR_USE_SCALE;
		if (m_ResizeMode!=RM_BILINEAR)
			m_HQRes->Init(m_width, m_height, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight);
	}

	m_biFmtDraw.biSizeImage = m_displaydepth*m_biFmtDraw.biWidth*m_biFmtDraw.biHeight/8;
	m_pBufferConvOut = new BYTE [m_biFmtDraw.biSizeImage];
	m_dwVRUse|=VR_USE_CONVOUT;	// always

	m_DrawPitch = m_biFmtDraw.biWidth*m_displaydepth/8;
	m_RenderMode = Mode;
	m_bNewFrame = 1;
	m_IsValid = true;
	m_hwnd = hwnd;
	return 0;
}

/**
****************************************************************************
 * \brief Prepare frame for DrawFrame in case of new resieved
 * \return TRUE (if everything is OK)
 *
 * \param hwnd					- window handler to draw in;
 *
 * \date    18-11-2002
 ******************************************************************************/
bool CDIBRender::PrepareFrame(HWND hwnd)
{
	if (m_bNewFrame) {
		if (m_dwVRUse&VR_USE_CONVIN)
			if (!ConvertColorSpaceIn()) return false;
		if (m_dwVRUse & VR_USE_FLIP)
			if (!MirrorVertical()) return false;
		if (!Saturate()) return false;
#ifdef _VIRTUAL_OUT
		VirtualOut();
#endif
		if (m_dwVRUse&VR_USE_SCALE)
			if (!ResampleImage()) return false;
		if (!Dither()) return false;

		if (m_dwVRUse&VR_USE_CONVOUT)
			if (!ConvertColorSpaceOut(m_pBufferConvOut, m_DrawPitch)) return false;
		m_bNewFrame = FALSE;
	}
	return true;
}


/**
****************************************************************************
 * \brief Render itself. Draw frame on client area of window
 * \return 0 (if everything is OK)
 *
 * \param hwnd					- window handler to draw in;
 *
 * \date    18-11-2002
 ******************************************************************************/
int CDIBRender::DrawFrame(HWND hwnd)
{
	if (!m_IsValid) return 1;
	Lock();

	if (m_DrawBorders)
		DrawBorders();

	if (hwnd!=m_hwnd || m_bForceBicubicChange!=m_bForceBicubic) {
		m_RenderMode = VRM_UNDEF;
		m_bForceBicubicChange = m_bForceBicubic;
		if (SetMode(VRM_DEFAULT, hwnd))
			return 1;
	}

	HDC hdc;
	int ret = 0;

	if (PrepareFrame(hwnd))	{
		hdc=GetWindowDC(hwnd);
		if(hdc!=NULL) {
			GetClientRect(hwnd, &m_rPic);
			m_rPic.right=(m_rPic.right+3)&~3;
			if (m_rPic.right > 0 && m_rPic.bottom > 0) {
				if (m_rPic.right != m_biFmtDraw.biWidth || m_rPic.bottom != m_biFmtDraw.biHeight) {
					if (!EqualRect(&m_rPic, &m_rPic2) || !m_pBufferDIBDraw) {
						if (m_pBufferDIBDraw)
							delete [] m_pBufferDIBDraw;
						m_pBufferDIBDraw = new BYTE [m_rPic.right*m_rPic.bottom*m_displaydepth/8];
						memset(m_pBufferDIBDraw, 0, m_rPic.right*m_rPic.bottom*m_displaydepth/8);
						m_rPic2 = m_rPic;
					}

					float x = (float)m_biFmtDraw.biWidth/m_rPic.right;
					float y = (float)m_biFmtDraw.biHeight/m_rPic.bottom; // src/dst
					int x_offs = 0;
					int y_offs = 0;
					if (m_bKeepAspectRatio && x>0 && y>0) {
						if (x>y)
							y_offs = (int)((m_rPic.bottom - m_rPic.bottom*y/x))&~1;
						else if (y>x)

							x_offs = (int)((m_rPic.right - m_rPic.right*x/y))&~1;
					}

					BITMAPINFOHEADER bih;
					memset(&bih, 0, sizeof(BITMAPINFOHEADER));
					bih.biSize = sizeof(BITMAPINFOHEADER);
					bih.biWidth = m_rPic.right;
					bih.biHeight = m_rPic.bottom;
					bih.biBitCount = m_displaydepth;
					bih.biCompression = BI_RGB;
					bih.biPlanes = 1;
					bih.biSizeImage = bih.biWidth*bih.biHeight*bih.biBitCount/8;

					m_rPic.right-=x_offs;
					m_rPic.bottom-=y_offs;

					int frameSizeMB = (m_rPic.right - m_rPic.left) * (m_rPic.bottom - m_rPic.top) / 256;
					m_renderFrameSizeMB = frameSizeMB;

					x_offs/=2;
					y_offs/=2;

					if (bih.biBitCount==24) {
						BYTE *p = m_pBufferDIBDraw + y_offs*bih.biWidth*3 + x_offs*3;
						m_pVSVideoProc->ResampleRGB24(m_pBufferConvOut, p, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight, m_rPic.right, m_rPic.bottom, bih.biWidth*3);
					}
					else {
						BYTE *p = m_pBufferDIBDraw + y_offs*bih.biWidth*4 + x_offs*4;
						m_pVSVideoProc->ResampleRGB32(m_pBufferConvOut, p, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight, m_rPic.right, m_rPic.bottom, bih.biWidth*4);
					}
					if ( StretchDIBits(hdc, 0, 0, bih.biWidth, bih.biHeight,
						0, 0, bih.biWidth, bih.biHeight, m_pBufferDIBDraw, (BITMAPINFO*)&bih, 0, SRCCOPY) == GDI_ERROR)
						ret = 1;
				}
				else {
					if ( StretchDIBits(hdc, 0, 0, m_rPic.right, m_rPic.bottom, 0, 0, m_biFmtDraw.biWidth,
						m_biFmtDraw.biHeight, m_pBufferConvOut, (BITMAPINFO*)&m_biFmtDraw, 0, SRCCOPY) == GDI_ERROR)
						ret = 1;
				}
			}
			ReleaseDC(hwnd, hdc);
		}
	}
	UnLock();
	return ret;
}

tTransparentStretch * CTransDIBRender::m_StretchFunc=NULL;

CTransDIBRender::CTransDIBRender(CVSInterface* pParentInterface)
:CDIBRender(pParentInterface)
{
}
int CTransDIBRender::DrawFrame(HWND hwnd)
{
	if (!m_IsValid) return 1;
	Lock();

	if (m_DrawBorders)
		DrawBorders();

	if (hwnd!=m_hwnd || m_bForceBicubicChange!=m_bForceBicubic) {
		m_RenderMode = VRM_UNDEF;
		m_bForceBicubicChange = m_bForceBicubic;
		if (SetMode(VRM_DEFAULT, hwnd))
			return 1;
	}

	HDC hdc;
	int ret = 0;

	if (PrepareFrame(hwnd))	{
		hdc=GetWindowDC(hwnd);
		if(hdc!=NULL) {
			GetClientRect(hwnd, &m_rPic);
			m_rPic.right=(m_rPic.right+3)&~3;
			if (m_rPic.right > 0 && m_rPic.bottom > 0) {
				if (m_rPic.right != m_biFmtDraw.biWidth || m_rPic.bottom != m_biFmtDraw.biHeight) {
					if (!EqualRect(&m_rPic, &m_rPic2) || !m_pBufferDIBDraw) {
						if (m_pBufferDIBDraw)
							delete [] m_pBufferDIBDraw;
						m_pBufferDIBDraw = new BYTE [m_rPic.right*m_rPic.bottom*m_displaydepth/8];
						memset(m_pBufferDIBDraw, 0, m_rPic.right*m_rPic.bottom*m_displaydepth/8);
						m_rPic2 = m_rPic;
					}

					float x = (float)m_biFmtDraw.biWidth/m_rPic.right;
					float y = (float)m_biFmtDraw.biHeight/m_rPic.bottom; // src/dst
					int x_offs = 0;
					int y_offs = 0;
					if (m_bKeepAspectRatio && x>0 && y>0) {
						if (x>y)
							y_offs = (int)((m_rPic.bottom - m_rPic.bottom*y/x))&~1;
						else if (y>x)

							x_offs = (int)((m_rPic.right - m_rPic.right*x/y))&~1;
					}

					BITMAPINFOHEADER bih;
					memset(&bih, 0, sizeof(BITMAPINFOHEADER));
					bih.biSize = sizeof(BITMAPINFOHEADER);
					bih.biWidth = m_rPic.right;
					bih.biHeight = m_rPic.bottom;
					bih.biBitCount = m_displaydepth;
					bih.biCompression = BI_RGB;
					bih.biPlanes = 1;
					bih.biSizeImage = bih.biWidth*bih.biHeight*bih.biBitCount/8;

					m_rPic.right-=x_offs;
					m_rPic.bottom-=y_offs;

					int frameSizeMB = (m_rPic.right - m_rPic.left) * (m_rPic.bottom - m_rPic.top) / 256;
					m_renderFrameSizeMB = frameSizeMB;

					x_offs/=2;
					y_offs/=2;

					if (bih.biBitCount==24) {
						BYTE *p = m_pBufferDIBDraw + y_offs*bih.biWidth*3 + x_offs*3;
						m_pVSVideoProc->ResampleRGB24(m_pBufferConvOut, p, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight, m_rPic.right, m_rPic.bottom, bih.biWidth*3);
					}
					else {
						BYTE *p = m_pBufferDIBDraw + y_offs*bih.biWidth*4 + x_offs*4;
						m_pVSVideoProc->ResampleRGB32(m_pBufferConvOut, p, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight, m_rPic.right, m_rPic.bottom, bih.biWidth*4);
					}
					if(m_StretchFunc){
						if ( (*m_StretchFunc)(hwnd,hdc, 0, 0, bih.biWidth, bih.biHeight,
							0, 0, bih.biWidth, bih.biHeight, m_pBufferDIBDraw, (BITMAPINFO*)&bih, 0, SRCCOPY) == GDI_ERROR)
							ret = 1;
					}
					else
						ret = 1;
				}
				else {
					if(m_StretchFunc){
						if ( (*m_StretchFunc)(hwnd,hdc, 0, 0, m_rPic.right, m_rPic.bottom, 0, 0, m_biFmtDraw.biWidth,
							m_biFmtDraw.biHeight, m_pBufferConvOut, (BITMAPINFO*)&m_biFmtDraw, 0, SRCCOPY) == GDI_ERROR)
							ret = 1;
					}
					else
						ret = 1;
				}
			}
			ReleaseDC(hwnd, hdc);
		}
	}
	UnLock();
	return ret;
}


/******************************************************************************
 * CDirectXRender
 ******************************************************************************/
/**
****************************************************************************
 * \date    18-11-2002
 ******************************************************************************/
//LPDDENUMCALLBACKEX
BOOL WINAPI DDEnumCallbackEx(GUID FAR *lpGUID, LPSTR lpDriverDescription,
							 LPSTR lpDriverName, LPVOID lpContext, HMONITOR  hm)
{
	CDirectXRender* render = (CDirectXRender*)lpContext;
	OLECHAR str[256] = {0};
	StringFromGUID2(*lpGUID, str, 256);
	DTRACE(255, "Video GUID = %S | %s(%s) |%x", str, lpDriverDescription, lpDriverName, hm);
	if (lpGUID) {
		render->m_VideoDrv[render->m_VideoDrvIndex].m_guid = *lpGUID;
		render->m_VideoDrv[render->m_VideoDrvIndex].m_monitor = hm;
		render->m_VideoDrvIndex++;
	}
	return TRUE;
}



CDirectXRender::CDirectXRender(CVSInterface *pParentInterface):
CVideoRenderBase(pParentInterface)
{
	m_repairTime = 0;
	m_displaydepth = 0;

	m_pDD = NULL;
	m_pPrimarySurface= NULL;
	m_pOffSurface= NULL;
	m_pOffSurfaceMain = NULL;
	m_pTextSurface = NULL;
	m_pClipper= NULL;
	ZeroMemory(&m_ddSurfaceDesc, sizeof(m_ddSurfaceDesc));
	ZeroMemory(&m_ddCaps, sizeof(m_ddCaps));
	ZeroMemory(&m_DirectDrawCaps, sizeof(m_DirectDrawCaps));

	m_VideoDrvIndex = 0;
	memset(&m_VideoDrv, 0, sizeof(m_VideoDrv));
	memset(&m_minfo, 0, sizeof(m_minfo));
	DirectDrawEnumerateEx(DDEnumCallbackEx, this, DDENUM_ATTACHEDSECONDARYDEVICES);
	m_monitor = 0;

	m_bUseDX = !(m_error_code = TestDDCaps()); // init and test caps
	m_bCanFS = 1;
	m_TypeRender = VR_DIRECT_DRAW;
}

/**
****************************************************************************
 * \date    18-11-2002
 *****************************************************************************/
CDirectXRender::~CDirectXRender()
{
	RemoveDirectDraw();
}

/**
****************************************************************************
 * \brief Init and reinit main DirectDraw interfase. Set display properties.
 *
 * \return 0 if OK, else error code
 * \date    18-11-2002
 ******************************************************************************/
int CDirectXRender::InitDirectDraw()
{
	RemoveDirectDraw();
	LPDIRECTDRAW pdd=0;

	if (m_monitor) {
		for (DWORD i = 0; i<m_VideoDrvIndex; i++) {
			if (m_monitor == m_VideoDrv[i].m_monitor)
				if (DirectDrawCreate(&m_VideoDrv[i].m_guid, &pdd, NULL)!=DD_OK) return -1;
		}
	}
	else {
		if (DirectDrawCreate(NULL, &pdd, NULL)!=DD_OK) return -1;
	}
	if (!pdd)
		return -1;
	if (pdd->QueryInterface(IID_IDirectDraw4, (void**)&m_pDD) !=DD_OK) return -1;
	pdd->Release();

	ZeroMemory(&m_ddSurfaceDesc, sizeof(m_ddSurfaceDesc));
	m_ddSurfaceDesc.dwSize=sizeof(m_ddSurfaceDesc);

	if (m_pDD->GetDisplayMode(&m_ddSurfaceDesc)!=DD_OK) return -2;

	m_display.left = 0;
	m_display.top = 0;
	m_display.right = m_ddSurfaceDesc.dwWidth;
	m_display.bottom = m_ddSurfaceDesc.dwHeight;
	m_displaydepth = m_ddSurfaceDesc.ddpfPixelFormat.dwRGBBitCount;
	m_DirectDrawCaps.bFccRGB24 = m_displaydepth==24;
	m_DirectDrawCaps.bFccRGB32 = m_displaydepth==32;
	if (m_displaydepth==8) return -3;
	return 0;
}

/**
****************************************************************************
 * \brief Test DirectDraw capabilities
 * \return 0 (if everything is OK), -1 if DirectDraw filed,
 *			1 in case of pure support
 *
 * \date    18-11-2002
 ******************************************************************************/
int CDirectXRender::TestDDCaps()
{
	HRESULT hRet = 0;
	int result = 0;

	if (result = InitDirectDraw())
		return result;

	ZeroMemory(&m_ddCaps, sizeof(DDCAPS));
	ZeroMemory(&m_DirectDrawCaps, sizeof(m_DirectDrawCaps));
	m_ddCaps.dwSize = sizeof(DDCAPS);
	hRet = m_pDD->GetCaps(&m_ddCaps, NULL); // nedd not HEL Caps

	m_DirectDrawCaps.bAnyHardSupp = !(m_ddCaps.dwCaps & DDCAPS_NOHARDWARE) && (m_ddCaps.dwCaps & DDCAPS_BLT) && (m_ddCaps.dwVidMemFree > 1500000);

	if (m_DirectDrawCaps.bAnyHardSupp) {
		if (m_ddCaps.dwCaps & DDCAPS_BLTSTRETCH) {
			m_DirectDrawCaps.bBltScaleDown = ((m_ddCaps.dwFXCaps & DDFXCAPS_BLTSHRINKX)&&(m_ddCaps.dwFXCaps & DDFXCAPS_BLTSHRINKY));
			m_DirectDrawCaps.bBltScaleUp = ((m_ddCaps.dwFXCaps & DDFXCAPS_BLTSTRETCHX)&&(m_ddCaps.dwFXCaps & DDFXCAPS_BLTSTRETCHY));
			m_DirectDrawCaps.bBltFcc = !!(m_ddCaps.dwCaps & DDCAPS_BLTFOURCC);
		}
		if (m_ddCaps.dwCaps & DDCAPS_OVERLAY) {
			m_DirectDrawCaps.bOverlayScaleDown = ((m_ddCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKX)&&(m_ddCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKY));
			m_DirectDrawCaps.bOverlayScaleUp = ((m_ddCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHX)&&(m_ddCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHY));
			m_DirectDrawCaps.bOverlayFcc = !!(m_ddCaps.dwCaps & DDCAPS_OVERLAYFOURCC);
		}
		if (m_DirectDrawCaps.bBltFcc || m_DirectDrawCaps.bOverlayFcc) {
			DWORD num = 100;
			DWORD codes[100];
			ZeroMemory(codes, sizeof(codes));
			hRet = m_pDD->GetFourCCCodes(&num, codes);
			for (DWORD i = 0; i<num; i++) {
				if (codes[i]==FCC_YUY2) m_DirectDrawCaps.bFccYUY2 = TRUE;
				if (codes[i]==FCC_UYVY) m_DirectDrawCaps.bFccUYVY = TRUE;
			}
		}
	}
	else {
		hRet = m_pDD->GetCaps(NULL, &m_ddCaps); // HEL Caps
		if ((m_ddCaps.dwCaps & DDCAPS_NOHARDWARE) || !(m_ddCaps.dwCaps & DDCAPS_BLT))
			return -5;
	}
	m_DirectDrawCaps.dwMem = m_ddCaps.dwVidMemTotal;
	m_DirectDrawCaps.dwMemFree = m_ddCaps.dwVidMemFree;

	//< testing beliniar support
	LPDIRECTDRAWSURFACE4 sfs1 = NULL, sfs2 = NULL, sfs0 = NULL;
	RECT rc1, rc2;
	int depth = m_displaydepth/8;
	BYTE *test = new BYTE[64*depth*64];
	for (int j = 0; j<16; j++)
		for (int i = 0; i<16; i++)
			for (int k = 0; k<depth; k++)
				test[i*depth+k+j*16*depth] = i*16;

	hRet = m_pDD->SetCooperativeLevel(NULL, DDSCL_NORMAL);
	if (hRet!=DD_OK) goto exit;

	ZeroMemory(&m_ddSurfaceDesc,sizeof(m_ddSurfaceDesc));
	m_ddSurfaceDesc.dwSize = sizeof(m_ddSurfaceDesc);
	m_ddSurfaceDesc.dwFlags = DDSD_CAPS;
	m_ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	hRet = m_pDD->CreateSurface(&m_ddSurfaceDesc, &sfs0, NULL);
	if (hRet!=DD_OK) goto exit;

	ZeroMemory(&m_ddSurfaceDesc,sizeof(m_ddSurfaceDesc));
	m_ddSurfaceDesc.dwSize = sizeof(m_ddSurfaceDesc);
	m_ddSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	m_ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	m_ddSurfaceDesc.dwHeight = 16;
	m_ddSurfaceDesc.dwWidth = 16;
	hRet = m_pDD->CreateSurface(&m_ddSurfaceDesc, &sfs1, NULL);
	if (hRet!=DD_OK) goto exit;

	ZeroMemory(&m_ddSurfaceDesc,sizeof(m_ddSurfaceDesc));
	m_ddSurfaceDesc.dwSize = sizeof(m_ddSurfaceDesc);
	m_ddSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	m_ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	m_ddSurfaceDesc.dwHeight = 64;
	m_ddSurfaceDesc.dwWidth = 64;
	hRet = m_pDD->CreateSurface(&m_ddSurfaceDesc, &sfs2, NULL);
	if (hRet!=DD_OK) goto exit;

	SetRect(&rc1, 0, 0, 16, 16);
	SetRect(&rc2, 0, 0, 64, 64);

	ZeroMemory(&m_ddSurfaceDesc,sizeof(m_ddSurfaceDesc));
	m_ddSurfaceDesc.dwSize = sizeof(m_ddSurfaceDesc);
	if (sfs1->Lock(NULL, &m_ddSurfaceDesc, DDLOCK_WAIT|DDLOCK_WRITEONLY, NULL)==DD_OK) {
		BYTE *d = (BYTE *)(m_ddSurfaceDesc.lpSurface), *s = test;
		for (int i = 0; i<16; i++) {
			memcpy(d, s, m_ddSurfaceDesc.lPitch);
			d+=m_ddSurfaceDesc.lPitch; s+=16*depth;
		}
		sfs1->Unlock(NULL);
	}
	else goto exit;

	hRet = sfs2->Blt(&rc2, sfs1, &rc1, DDBLT_WAIT, NULL);

	ZeroMemory(&m_ddSurfaceDesc,sizeof(m_ddSurfaceDesc));
	m_ddSurfaceDesc.dwSize = sizeof(m_ddSurfaceDesc);
	if (sfs2->Lock(NULL, &m_ddSurfaceDesc, DDLOCK_WAIT|DDLOCK_READONLY, NULL)==DD_OK) {
		BYTE *s = (BYTE *)(m_ddSurfaceDesc.lpSurface), *d = test;
		for (int i = 0; i<16; i++) {
			memcpy(d, s, m_ddSurfaceDesc.lPitch);
			s+=m_ddSurfaceDesc.lPitch; d+=64*depth;
		}
		sfs2->Unlock(NULL);
	}
	else goto exit;

	if (!(test[depth*0]==test[depth*1]==test[depth*2]==test[depth*3]) )
		m_DirectDrawCaps.bHaveBelin = TRUE;

exit:
	if (sfs2) sfs2->Release();
	if (sfs1) sfs1->Release();
	if (sfs0) sfs0->Release();
	delete[] test;

	ZeroMemory(&m_ddSurfaceDesc,sizeof(m_ddSurfaceDesc));
	m_ddSurfaceDesc.dwSize = sizeof(m_ddSurfaceDesc);

	return 0;
}

/**
****************************************************************************
 * \brief Fill info about video driver
 *
 * \param driver				- name of driver dll;
 * \param desc					- driver desc;
 *
 * \date    10-08-2007
 ******************************************************************************/
bool CDirectXRender::FillDeviceInfo(char* driver, char* desc)
{
	DDDEVICEIDENTIFIER DDI;
	if (m_pDD && m_pDD->GetDeviceIdentifier(&DDI, 0)==DD_OK) {
		strncpy(driver, DDI.szDriver, MAX_DDDEVICEID_STRING-1);
		strncpy(desc, DDI.szDescription, MAX_DDDEVICEID_STRING-1);
		return true;
	}
	return false;
}

/**
****************************************************************************
 * \brief Create surfasies according input dimensions and set clipper for hwnd
 * \return TRUE (if everything is OK)
 *
 * \param x						- widht of created surfase;
 * \param y						- heigth of created surfase;
 * \param hwnd					- window handler to attach clipper;
 *
 * \date    18-11-2002
 ******************************************************************************/
BOOL CDirectXRender::CreateSurfaces(int x, int y, HWND hwnd)
{
	HRESULT hRet = DD_OK;
	DWORD fcc = 0, dwFlags = 0;

	hRet|= m_pDD->SetCooperativeLevel(hwnd, DDSCL_NORMAL);

	ZeroMemory(&m_ddSurfaceDesc,sizeof(m_ddSurfaceDesc));
	m_ddSurfaceDesc.dwSize = sizeof(m_ddSurfaceDesc);
	m_ddSurfaceDesc.dwFlags = DDSD_CAPS;
	m_ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	hRet|= m_pDD->CreateSurface(&m_ddSurfaceDesc, &m_pPrimarySurface, NULL);
	if (hRet!=DD_OK) goto error_exit;
	hRet|= m_pDD->CreateClipper(NULL, &m_pClipper, NULL);
	if (hRet!=DD_OK) goto error_exit;
	hRet|= m_pClipper->SetHWnd(NULL, hwnd);
	hRet|= m_pPrimarySurface->SetClipper(m_pClipper);

	ZeroMemory(&m_ddSurfaceDesc,sizeof(m_ddSurfaceDesc));
	m_ddSurfaceDesc.dwSize = sizeof(m_ddSurfaceDesc);
	m_ddSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	m_ddSurfaceDesc.dwHeight = 128;
	m_ddSurfaceDesc.dwWidth = 128;
	m_ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	if (m_DirectDrawCaps.bAnyHardSupp) m_ddSurfaceDesc.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	hRet |= m_pDD->CreateSurface(&m_ddSurfaceDesc, &m_pOffSurfaceMain, NULL);

	switch (m_biFmtDraw.biCompression)
	{
	case BI_RGB:
		dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		break;
	case FCC_YUY2:
		dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH |DDSD_PIXELFORMAT;
		fcc = FCC_YUY2;
		break;
	case FCC_UYVY:
		dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH |DDSD_PIXELFORMAT;
		fcc = FCC_UYVY;
		break;
	default:
		return FALSE;
	}

	ZeroMemory(&m_ddSurfaceDesc,sizeof(m_ddSurfaceDesc));
	m_ddSurfaceDesc.dwSize = sizeof(m_ddSurfaceDesc);
	m_ddSurfaceDesc.dwFlags = dwFlags;
	m_ddSurfaceDesc.dwHeight = y;
	m_ddSurfaceDesc.dwWidth = x;
	m_ddSurfaceDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	m_ddSurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
	m_ddSurfaceDesc.ddpfPixelFormat.dwFourCC = fcc;

	m_ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	if (m_DirectDrawCaps.bAnyHardSupp) m_ddSurfaceDesc.ddsCaps.dwCaps|= DDSCAPS_VIDEOMEMORY;
	hRet|= m_pDD->CreateSurface(&m_ddSurfaceDesc, &m_pOffSurface, NULL);

error_exit:
	if (hRet!=DD_OK) {
		if (m_pClipper)
			m_pClipper->Release();
		m_pClipper = NULL;
		if (m_pPrimarySurface)
			m_pPrimarySurface->Release();
		m_pPrimarySurface = NULL;
		if (m_pOffSurface)
			m_pOffSurface->Release();
		m_pOffSurface = NULL;
		return FALSE;
	}
	else
		return TRUE;
}

/**
****************************************************************************
 * \brief Restore main offscreen surface
 * \return TRUE (if everything is OK)
 *
 * \date    18-11-2002
 ******************************************************************************/
bool CDirectXRender::RestoreOffcreenMainSurface()
{
	HRESULT hRet = DD_OK;
	if (m_pOffSurfaceMain == NULL) {
		DDSURFACEDESC2 desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.dwSize = sizeof(DDSURFACEDESC2);
		desc.dwHeight = 128;
		desc.dwWidth = 128;
		desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		if (m_DirectDrawCaps.bAnyHardSupp) desc.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		hRet = m_pDD->CreateSurface(&desc, &m_pOffSurfaceMain, NULL);
	}
	return (hRet == DD_OK);
}

/**
****************************************************************************
 * \brief Reset main offscreen surface
 * \return TRUE (if everything is OK)
 *
 * \date    18-11-2002
 ******************************************************************************/
bool CDirectXRender::ResetOffcreenMainSurface()
{
	HRESULT hRet = DD_OK;
	DDSURFACEDESC2 desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.dwSize = sizeof(DDSURFACEDESC2);
	hRet = m_pOffSurfaceMain->GetSurfaceDesc(&desc);
	if (hRet == DD_OK) {
		int ow = desc.dwWidth, oh = desc.dwHeight;
		int nw = m_rPicMain2.right - m_rPicMain2.left;
		int nh = m_rPicMain2.bottom - m_rPicMain2.top;
		int thrw = nw - ow;
		int thrh = nh - oh;
		if ((thrw > 0 || thrh > 0) && (ow != m_display.right || oh != m_display.bottom)) {
			m_pOffSurfaceMain->Release();
			m_pOffSurfaceMain = NULL;
			ZeroMemory(&desc,sizeof(desc));
			desc.dwSize = sizeof(m_ddSurfaceDesc);
			desc.dwHeight = (nh * 115 / 100 + 15)&~15;
			desc.dwWidth = (nw * 115 / 100 + 15)&~15;
			if ((long)desc.dwHeight > m_display.bottom) desc.dwHeight = m_display.bottom;
			if ((long)desc.dwWidth > m_display.right) desc.dwWidth = m_display.right;
			desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
			desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			if (m_DirectDrawCaps.bAnyHardSupp) desc.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
			hRet = m_pDD->CreateSurface(&desc, &m_pOffSurfaceMain, NULL);
		}
	}
	return (hRet == DD_OK);
}

/**
****************************************************************************
 * \brief Remove DirecDrow interfasies
 *
 * \date    18-11-2002
 ******************************************************************************/
void CDirectXRender::RemoveDirectDraw()
{
	if (m_pDD!=NULL) {
		if (m_pClipper)
			m_pClipper->Release();
			m_pClipper = NULL;
		if (m_pPrimarySurface)
			m_pPrimarySurface->Release();
			m_pPrimarySurface = NULL;
		if (m_pOffSurface)
			m_pOffSurface->Release();
			m_pOffSurface = NULL;
		if (m_pOffSurfaceMain)
			m_pOffSurfaceMain->Release();
			m_pOffSurfaceMain = NULL;
		if (m_pTextSurface)
			m_pTextSurface->Release();
			m_pTextSurface = NULL;
		m_pDD->Release();
		m_pDD = NULL;
	}
}

bool CDirectXRender::IsNeedResetOnSize()
{
	return (!m_DirectDrawCaps.bAnyHardSupp && m_RenderMode==VRM_1X2);
}


/**
****************************************************************************
 * \brief Create display name on surface
 *
 * \param width					- window width;
 * \param height				- window height;
 *
 * \date    15-03-2011
 ******************************************************************************/
int CDirectXRender::CreateSurfaceName(int w, int h)
{
	if (m_DisplayName.name.empty() || w <= 0 || h <= 0) {
		SetRectEmpty(&m_DisplayName.rName);
		return -1;
	}

	int ow, oh;
	ow = m_DisplayName.width;
	oh = m_DisplayName.height;
	m_DisplayName.width = w;
	m_DisplayName.height = h / 16;
	if (m_DisplayName.height < 13) m_DisplayName.height = 13;
	int htxt = m_DisplayName.height;
	m_DisplayName.height = (m_DisplayName.height + 1)&~1;

	HRESULT hr;
	HDC hDC = NULL;
	SIZE sizeText;
	HFONT hFont;
	DDSURFACEDESC2 desc;

	if (ow != m_DisplayName.width || oh != m_DisplayName.height || m_pTextSurface == NULL || m_DisplayName.update) {
		SetRectEmpty(&m_DisplayName.rName);
		if (!m_DisplayName.name.empty()) {
			hFont = CreateFontW(htxt, 0, 0, 0, FW_LIGHT, 0, 0, 0,
								DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,
								FF_SWISS|VARIABLE_PITCH, L"Tahoma\0");
			if (hFont) {
				hDC = GetDC(NULL);
				SelectObject(hDC, hFont);
				GetTextExtentPoint32W(hDC, m_DisplayName.name.data(), m_DisplayName.name.length(), &sizeText);
				ReleaseDC(NULL, hDC);
			} else {
				return -1;
			}

			if (m_pTextSurface) m_pTextSurface->Release(); m_pTextSurface = NULL;

			ZeroMemory(&desc,sizeof(DDSURFACEDESC2));
			desc.dwSize = sizeof(DDSURFACEDESC2);
			desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
			desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			if (m_DirectDrawCaps.bAnyHardSupp) desc.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
			desc.dwWidth = m_DisplayName.width;
			desc.dwHeight = m_DisplayName.height;

			hr = m_pDD->CreateSurface(&desc, &m_pTextSurface, NULL);

			if (hr == DD_OK) {
				DDCOLORKEY SrcColorkey;
				SrcColorkey.dwColorSpaceLowValue = DX_BCKGR_COLOR;
				SrcColorkey.dwColorSpaceHighValue = DX_BCKGR_COLOR;
				hr = m_pTextSurface->SetColorKey(DDCKEY_SRCBLT, &SrcColorkey);

				if (hr == DD_OK) {
					hr = m_pTextSurface->Restore();
					if (hr == DD_OK) {
						hr = m_pTextSurface->GetDC(&hDC);
						if (hr == DD_OK) {
							SetRect(&m_DisplayName.rName, 0, 0, m_DisplayName.width, m_DisplayName.height);
							SetBkColor(hDC, DX_BCKGR_COLOR);
							HBRUSH hbr = CreateSolidBrush(DX_BCKGR_COLOR);
							FillRect(hDC, &m_DisplayName.rName, hbr);
							DeleteObject(hbr);
							SetTextColor(hDC, DX_TEXT_COLOR);
							if (hFont) SelectObject(hDC, hFont);
							int ret = DrawTextW(hDC, m_DisplayName.name.data(), m_DisplayName.name.length(), &m_DisplayName.rName, DT_CENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
							m_pTextSurface->ReleaseDC(hDC);
						}
					}
				}
			}
			DeleteObject(hFont);
			if (hr != DD_OK) {
				if (m_pTextSurface) m_pTextSurface->Release(); m_pTextSurface = NULL;
				return -2;
			}
		}
		m_DisplayName.update = false;
	}

	return 0;
}

/**
****************************************************************************
 * \brief Prepare frame for DrawFrame in case of new one resieved or loosing surfacies
 * \return TRUE (if everything is OK)
 *
 * \param hwnd					- window handler to draw in;
 *
 * \date    18-11-2002
 ******************************************************************************/
bool CDirectXRender::PrepareFrame(HWND hwnd)
{
	HRESULT hRet = DD_OK;
	bool sucs = true;

	if (!RestoreOffcreenMainSurface()) return false;

	if (m_pPrimarySurface->IsLost())
		hRet |= m_pPrimarySurface->Restore();
	if (m_pOffSurfaceMain->IsLost())
		hRet |= m_pOffSurfaceMain->Restore();
	if (m_pOffSurface->IsLost())
		hRet |= m_pOffSurface->Restore();
	if (m_pTextSurface && m_pTextSurface->IsLost()) {
		hRet |= m_pTextSurface->Restore();
		if (hRet == DD_OK) {
			m_DisplayName.width = 0;
			m_DisplayName.height = 0;
			SetRectEmpty(&m_DisplayName.rName);
		}
	}
	if (hRet != DD_OK)
		return false;

	if (m_bNewFrame) {
		if (m_dwVRUse&VR_USE_CONVIN)
			if (!ConvertColorSpaceIn()) return false;
		if (m_dwVRUse & VR_USE_FLIP)
			if (!MirrorVertical()) return false;
		if (!Saturate()) return false;
#ifdef _VIRTUAL_OUT
    VirtualOut();
#endif
		if (m_dwVRUse&VR_USE_SCALE)
			if (!ResampleImage()) return false;
		if (!Dither()) return false;

		if (m_biFmtDraw.biCompression!=BI_RGB) {
			if (m_pOffSurface->Lock(NULL, &m_ddSurfaceDesc, DDLOCK_WAIT|DDLOCK_WRITEONLY, NULL)==DD_OK)	{
				sucs = ConvertColorSpaceOut((BYTE*)m_ddSurfaceDesc.lpSurface, m_ddSurfaceDesc.lPitch);
				m_pOffSurface->Unlock(NULL);
			}
			else return false;
		}
		else {
			HDC hdc;
			if (m_dwVRUse&VR_USE_CONVOUT)
				sucs = ConvertColorSpaceOut(m_pBufferConvOut, m_biFmtDraw.biBitCount*m_biFmtDraw.biWidth/8);
			if ((m_pOffSurface->GetDC(&hdc)) == DD_OK) {
				StretchDIBits(hdc, 0, 0, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight, 0, 0, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight, // not software scaling
								m_pBufferConvOut, (BITMAPINFO*)&m_biFmtDraw, 0, SRCCOPY);
				m_pOffSurface->ReleaseDC(hdc);
			}
			else return false;
		}
		sucs = ResetOffcreenMainSurface();
		if (sucs) {
			CreateSurfaceName(m_rPicMain2.right - m_rPicMain2.left, m_rPicMain2.bottom - m_rPicMain2.top);
			m_bNewFrame = FALSE;
		}
	}
	return sucs;
}

/**
****************************************************************************
 * \brief Render itself. Draw frame on client area of window
 * \return 0 (if everything is OK)
 *
 * \param hwnd					- window handler to draw in;
 *
 * \date    18-11-2002
 ******************************************************************************/
int CDirectXRender::DrawFrame(HWND hwnd)
{
	VS_AutoLock lock(this);

	if (!IsInited())
		return 1;

	if (m_DrawBorders)
		DrawBorders();

	if (!m_IsValid) { // try to repair if render cannot draw
		DWORD currTime = timeGetTime();
		if (currTime - m_repairTime >= VR_REPAIR_TIME) {
			m_repairTime = currTime;
			SetMode(VRM_DEFAULT, hwnd);
		}
		if (!m_IsValid)
			return 1;
	}
	if (hwnd!=m_hwnd || m_bForceBicubicChange!=m_bForceBicubic) {
		CheckMonitor(hwnd);
		m_RenderMode = VRM_UNDEF;
		m_bForceBicubicChange = m_bForceBicubic;
		if (SetMode(VRM_DEFAULT, hwnd))
			return 1;
	}

	HRESULT hRet;
	if (!SetRectangle(hwnd))
		return 1;
	if (!PrepareFrame(hwnd))
		return 1;

	DDBLTFX DDBltFx;
	memset(&DDBltFx, 0, sizeof(DDBLTFX));
	DDBltFx.dwSize = sizeof(DDBLTFX);
	DDBltFx.dwFillColor = 0;
	DDBltFx.dwDDFX = DDBLTFX_NOTEARING;
	if (!IsRectEmpty(&m_rcb1)) hRet = m_pOffSurfaceMain->Blt(&m_rcb1, 0, 0, DDBLT_COLORFILL | DDBLT_DDFX | DDBLT_ASYNC, &DDBltFx);
	hRet = m_pOffSurfaceMain->Blt(&m_rPicMain, m_pOffSurface, &m_rPic2, DDBLT_DDFX | DDBLT_ASYNC, &DDBltFx);
	if (!IsRectEmpty(&m_rcb2)) hRet = m_pOffSurfaceMain->Blt(&m_rcb2, 0, 0, DDBLT_COLORFILL | DDBLT_DDFX | DDBLT_ASYNC, &DDBltFx);
	if (!IsRectEmpty(&m_DisplayName.rName)) {
		hRet = m_pOffSurfaceMain->Blt(&m_DisplayName.rName, m_pTextSurface, &m_DisplayName.rName, DDBLT_DDFX | DDBLT_ASYNC | DDBLT_KEYSRC | DDBLT_WAIT, &DDBltFx);
	}
	hRet = m_pPrimarySurface->Blt(&m_rPic, m_pOffSurfaceMain, &m_rPicMain2, DDBLT_DDFX | DDBLT_ASYNC, &DDBltFx);

	return hRet!=DD_OK;
}

void CDirectXRender::CheckMonitor(HWND hwnd)
{
	if (m_VideoDrvIndex <= 1)
		return;
	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	if (monitor!=m_monitor) {
		m_monitor = monitor;
		m_minfo.cbSize = sizeof(MONITORINFOEX);
		if (GetMonitorInfo(m_monitor, &m_minfo)) {
			DTRACE(255, "SIZE: %d X %d, at %x", m_minfo.rcWork.right, m_minfo.rcWork.bottom, m_monitor);
		}
		m_IsValid = false;
		m_RenderMode = VRM_UNDEF;
	}
}

/**
****************************************************************************
 * \brief Set DirectDraw specific variables
 *
 * \param hwnd					- window handler to render in;
 *
 *  \date    03-12-2002
 ******************************************************************************/
bool CDirectXRender::SetRectangle(HWND hwnd)
{
	POINT point ={0, 0};
	GetClientRect(hwnd, &m_rPic);
	m_rPic.right = ((m_rPic.right - m_rPic.left) + 3)&~3;	// divisible by 4
	m_rPic.bottom = ((m_rPic.bottom - m_rPic.top) + 1)&~1;	// divisible by 2
	m_rPic.left = 0;
	m_rPic.top = 0;
	if (m_rPic.right <= 0 || m_rPic.bottom <=0)
		return false;

	if (m_display.right > 0 && m_display.bottom > 0) {
		if (m_rPic.right > m_display.right) m_rPic.right = m_display.right;
		if (m_rPic.bottom > m_display.bottom) m_rPic.bottom = m_display.bottom;
	}

	ClientToScreen(hwnd, &point);
	point.x-=m_minfo.rcMonitor.left;
	point.y-=m_minfo.rcMonitor.top;
	m_rcb1 = m_rPic;
	m_rcb2 = m_rPic;

	float x = (float)m_biFmtDraw.biWidth/m_rPic.right;
	float y = (float)m_biFmtDraw.biHeight/m_rPic.bottom; // src/dst
	int x_offs = 0;
	int y_offs = 0;
	if (m_bKeepAspectRatio && x>0.f && y>0.f) {
		if (x>y)
			y_offs = (int)((m_rPic.bottom - (int)(m_rPic.bottom*y/x))/2); // blurred image fix for 480x352
		else if (y>=x)
			x_offs = (int)((m_rPic.right - (int)(m_rPic.right*x/y))/2);
	}

	if (y_offs>0) {
		m_rcb1.bottom = y_offs;
		m_rcb2.top = m_rcb2.bottom-y_offs;
	}
	else if (x_offs>0) {
		m_rcb1.right = x_offs;
		m_rcb2.left = m_rcb2.right-x_offs;
	}
	else {
		SetRectEmpty(&m_rcb1);
		SetRectEmpty(&m_rcb2);
	}
	m_rPicMain = m_rPic;
	m_rPicMain2 = m_rPic;
	InflateRect(&m_rPicMain, -x_offs, -y_offs);
	OffsetRect(&m_rPic, point.x, point.y);

	return true;
}

/**
****************************************************************************
 * \brief Set DirectDraw Render Mode according to eVideoRenderMode type
 * \return 0 if everything is OK and error number else
 *
 * \param Mode					- new Render Mode ;
 * \param hwnd					- window handler to render in;
 *
 *  \date    03-12-2002
 ******************************************************************************/
int CDirectXRender::SetMode(eVideoRenderMode Mode, HWND hwnd)
{
	VS_AutoLock lock(this);

	if (!IsInited())
			return 1;

	RECT rPic = m_rPic,
	     rPicMain = m_rPicMain,
		 rPicMain2 = m_rPicMain2,
		 rcb1 = m_rcb1,
		 rcb2 = m_rcb2;

	if (!SetRectangle(hwnd))
		return 0;
	int frameSizeMB = (m_rPicMain.right - m_rPicMain.left) * (m_rPicMain.bottom - m_rPicMain.top) / 256;
	m_renderFrameSizeMB = frameSizeMB;

	if (Mode== VRM_DEFAULT)
		Mode = DetermineMode(hwnd);
	if (m_RenderMode!= VRM_UNDEF)
		if (m_RenderMode==Mode) {
			m_rPic = rPic,
			m_rPicMain = rPicMain,
			m_rPicMain2 = rPicMain2,
			m_rcb1 = rcb1,
			m_rcb2 = rcb2;
			return 0;
		}

	// begin reinitialisation
	m_IsValid = false;

	int retCaps = TestDDCaps();
	if (retCaps)
		return retCaps;

	m_biFmtDraw.biCompression = BI_RGB;
	m_biFmtDraw.biBitCount = (m_displaydepth==24)?24:32;

	if (m_DirectDrawCaps.bBltFcc || (Mode==VRM_FS && !m_DirectDrawCaps.bBltScaleUp
		&& m_DirectDrawCaps.bOverlayScaleUp && m_DirectDrawCaps.bOverlayFcc))
	{
		if (m_DirectDrawCaps.bFccYUY2) {
			m_biFmtDraw.biCompression = FCC_YUY2;
			m_biFmtDraw.biBitCount = 16;
		}
		else if (m_DirectDrawCaps.bFccUYVY) {
			m_biFmtDraw.biCompression = FCC_UYVY;
			m_biFmtDraw.biBitCount = 16;
		}
	}

	CleanBuffers();

	switch(m_biFmtIn.biCompression)		// input format
	{
	case FCC_I420:
	case FCC_IYUV:
		m_pBufferConvIn = m_pBufferIn;
		break;
	case FCC_I42S:
		m_pBufferConvIn = m_pBufferIn;
		if (m_dwStereoRender > 0) break;
	case FCC_YV12:
	case FCC_YUY2:
	case FCC_UYVY:
	case BI_RGB:
		m_pBufferConvIn = new BYTE[m_width*m_height*3/2];
		m_dwVRUse|=VR_USE_CONVIN;
		break;
	}

	m_pBufferMirrorIn = m_pBufferConvIn;
	if (m_bSelfView && m_bFlipFrame) {
		m_pBufferMirrorIn = new BYTE[m_width*m_height*3/2];
		m_dwVRUse |= VR_USE_FLIP;
	}

	// always use saturate
	m_pBufferSaturated = new BYTE[m_width*m_height*3/2];
	m_dwVRUse|=VR_USE_SATURATE;

	m_biFmtDraw.biWidth = m_width;
	m_biFmtDraw.biHeight = m_height;
	m_pBufferScaled = m_pBufferSaturated;
	if (m_ResizeMode==RM_DEFAULT) {
		switch(Mode)
		{
		case VRM_1d5X1:
			if (!m_DirectDrawCaps.bHaveBelin || m_bForceBicubic) {
				m_biFmtDraw.biWidth = m_width*3/2;
				m_biFmtDraw.biHeight = m_height*3/2;
				m_pBufferScaled = new BYTE [m_biFmtDraw.biWidth*m_biFmtDraw.biHeight*3/2];
				m_dwVRUse|=VR_USE_SCALE;
			}
			break;
		case VRM_2X1:
			if (!m_DirectDrawCaps.bHaveBelin || m_bForceBicubic) {
				m_biFmtDraw.biWidth = m_width*2;
				m_biFmtDraw.biHeight = m_height*2;
				m_pBufferScaled = new BYTE [m_biFmtDraw.biWidth*m_biFmtDraw.biHeight*3/2];
				m_dwVRUse|=VR_USE_SCALE;
			}
			break;
		case VRM_1X2:
			if (!m_DirectDrawCaps.bBltScaleDown) {
				m_biFmtDraw.biWidth = (m_rPicMain.right - m_rPicMain.left+3)&~3;
				m_biFmtDraw.biHeight = (m_rPicMain.bottom - m_rPicMain.top+1)&~1;
				m_pBufferScaled = new BYTE [m_biFmtDraw.biWidth*m_biFmtDraw.biHeight*3/2];
				m_dwVRUse|=VR_USE_SCALE;

			}
			break;
		}
	}
	else if (m_bForceBicubic) {
		m_biFmtDraw.biWidth = (m_rPicMain.right - m_rPicMain.left+3)&~3;
		m_biFmtDraw.biHeight = (m_rPicMain.bottom - m_rPicMain.top+1)&~1;
		m_pBufferScaled = new BYTE [m_biFmtDraw.biWidth*m_biFmtDraw.biHeight*3/2];
		m_dwVRUse|=VR_USE_SCALE;
		if (m_ResizeMode!=RM_BILINEAR)
			m_HQRes->Init(m_width, m_height, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight);
	}
	SetRect(&m_rPic2, 0, 0, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight);// sourse image dimensions

	if (!CreateSurfaces(m_biFmtDraw.biWidth, m_biFmtDraw.biHeight, hwnd)) {
		if (m_biFmtDraw.biCompression!= BI_RGB) {
			m_biFmtDraw.biCompression = BI_RGB;
			m_biFmtDraw.biBitCount = (m_displaydepth==24)?24:32;
			if (!CreateSurfaces(m_biFmtDraw.biWidth, m_biFmtDraw.biHeight, hwnd))
				return VRDDERR_CREATESURF;
		}
		else
			return VRDDERR_CREATESURF;
	}

	m_biFmtDraw.biSizeImage = m_biFmtDraw.biBitCount*m_biFmtDraw.biWidth*m_biFmtDraw.biHeight/8;
	m_pBufferConvOut = new BYTE [m_biFmtDraw.biSizeImage];
	m_dwVRUse|=VR_USE_CONVOUT;	// always

	m_RenderMode = Mode;
	m_bNewFrame = TRUE;
	m_IsValid = true;
	m_hwnd = hwnd;
	return 0;
}

HRESULT VSGetDXVersion(DWORD *DXVersionMajor, DWORD *DXVersionMinor);
/**
****************************************************************************
 * \brief Get DirectDraw information and form sring
 *
 * \param prop			- sring to fill
 *
 * \date    13-02-2008
 ******************************************************************************/
void VSGetDirectDrawInfo(VS_SimpleStr &prop)
{
	char str[1024] = {0};

	DWORD DXMajor, DXMinor;
	if (S_OK == VSGetDXVersion(&DXMajor, &DXMinor)) {
		sprintf(str, "Version:      %ld.%ld\n", DXMajor, DXMinor);
	} else {
		sprintf(str, "Version:      unknown\n");
	}
	prop+=str;

	CDirectXRender *vrender =new CDirectXRender(NULL);
	char	szDriver[MAX_DDDEVICEID_STRING] = {0};
	char	szDesc[MAX_DDDEVICEID_STRING] = {0};
	if (vrender->FillDeviceInfo(szDriver, szDesc)) {
		if (*szDriver) {
			sprintf(str, "Driver:       %s ", szDriver);
			if (*szDesc)
				strcat(str, szDesc);
			prop+=str;
		}
	}

	sprintf(str, "\nResolution:   %ldx%ld, %d bit \n", vrender->m_display.right, vrender->m_display.bottom, vrender->m_displaydepth);
	prop+=str;

	sprintf(str, "Video Memory: total - %lu MB, free - %lu MB\n", vrender->m_DirectDrawCaps.dwMem / 1024 / 1024, vrender->m_DirectDrawCaps.dwMemFree / 1024 / 1024);
	strcat(str, "Capabilities: | Bu | Bd | Ou | Od |BFcc|OFcc|YUY2|UYVY| HB |\n");
	prop+=str;
	sprintf(str,"              | %d  | %d  | %d  | %d  | %d  | %d  | %d  | %d  | %d  |",
		vrender->m_DirectDrawCaps.bBltScaleUp,
		vrender->m_DirectDrawCaps.bBltScaleDown,
		vrender->m_DirectDrawCaps.bOverlayScaleUp,
		vrender->m_DirectDrawCaps.bOverlayScaleDown,
		vrender->m_DirectDrawCaps.bBltFcc,
		vrender->m_DirectDrawCaps.bOverlayFcc,
		vrender->m_DirectDrawCaps.bFccYUY2,
		vrender->m_DirectDrawCaps.bFccUYVY,
		vrender->m_DirectDrawCaps.bHaveBelin);

	if (!vrender->m_bUseDX) {
		strcat(str, "\nDX NOT USED BECAUSE: ");
		switch(vrender->m_error_code)
		{
		case -1: strcat(str, "  Eror DirectDraw create"); break;
		case -2: strcat(str, "  Eror Get display mode"); break;
		case -3: strcat(str, "  display depth = 8"); break;
		case -4: strcat(str, "  No videocard hardware support"); break;
		case -5: strcat(str, "  No blt support"); break;
		case -6: strcat(str, "  Free video memory is low"); break;
		default: strcat(str, "  Unknown reason"); break;
		}
	}
	else
		strcat(str, "\nDX INIT OK");
	prop+=str;
	delete vrender;
}
