/**
 **************************************************************************
 * \file VSRender.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Video render class. Draw image on external HWND
 *
 * \b Project Client
 * \author SMirnovK
 * \date 07.10.2002
 *
 * $Revision: 25 $
 *
 * $History: VSRender.h $
 *
 * *****************  Version 25  *****************
 * User: Sanufriev    Date: 12.05.12   Time: 11:57
 * Updated in $/VSNA/VSClient
 * - were added mirror self view video
 *
 * *****************  Version 24  *****************
 * User: Sanufriev    Date: 6.04.12    Time: 11:36
 * Updated in $/VSNA/VSClient
 * - case for video render from registry
 *
 * *****************  Version 23  *****************
 * User: Sanufriev    Date: 19.03.12   Time: 10:19
 * Updated in $/VSNA/VSClient
 * - clean d3d10
 *
 * *****************  Version 22  *****************
 * User: Sanufriev    Date: 6.03.12    Time: 19:40
 * Updated in $/VSNA/VSClient
 * - fix d3d10 (install on WinXP)
 *
 * *****************  Version 21  *****************
 * User: Sanufriev    Date: 5.03.12    Time: 16:08
 * Updated in $/VSNA/VSClient
 * - were added Direct3D10 support
 *
 * *****************  Version 20  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 19  *****************
 * User: Sanufriev    Date: 27.05.11   Time: 18:59
 * Updated in $/VSNA/VSClient
 * - direct draw & remote desctop (bug 8982)
 *
 * *****************  Version 18  *****************
 * User: Smirnov      Date: 5.05.11    Time: 12:51
 * Updated in $/VSNA/VSClient
 * - bugfix#8297
 *
 * *****************  Version 17  *****************
 * User: Sanufriev    Date: 1.04.11    Time: 16:24
 * Updated in $/VSNA/VSClient
 * - set display name
 *
 * *****************  Version 16  *****************
 * User: Sanufriev    Date: 1.04.11    Time: 13:00
 * Updated in $/VSNA/VSClient
 * - were added draft version DisplayName for Direct3D (not work yet)
 *
 * *****************  Version 15  *****************
 * User: Sanufriev    Date: 25.03.11   Time: 12:15
 * Updated in $/VSNA/VSClient
 * - display name for DirectX render
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 24.03.11   Time: 18:43
 * Updated in $/VSNA/VSClient
 * - videoName
 *
 * *****************  Version 13  *****************
 * User: Melechko     Date: 21.03.11   Time: 15:49
 * Updated in $/VSNA/VSClient
 * add clear dirty region in direct3d render
 *
 * *****************  Version 12  *****************
 * User: Sanufriev    Date: 2.03.11    Time: 17:41
 * Updated in $/VSNA/VSClient
 * - fix render d3d
 *
 * *****************  Version 11  *****************
 * User: Sanufriev    Date: 17.01.11   Time: 18:06
 * Updated in $/VSNA/VSClient
 * - were adde type error in d3d init
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 17.01.11   Time: 13:25
 * Updated in $/VSNA/VSClient
 * - ench #8350 d3d caps
 *
 * *****************  Version 9  *****************
 * User: Sanufriev    Date: 20.12.10   Time: 12:56
 * Updated in $/VSNA/VSClient
 * - remove D3DXLoadSurfaceFromMemory from d3d renderer
 *
 * *****************  Version 8  *****************
 * User: Sanufriev    Date: 16.12.10   Time: 19:40
 * Updated in $/VSNA/VSClient
 * - performance & clean direct3d render
 *
 * *****************  Version 7  *****************
 * User: Sanufriev    Date: 9.11.10    Time: 18:43
 * Updated in $/VSNA/VSClient
 * - shaders from file
 *
 * *****************  Version 6  *****************
 * User: Sanufriev    Date: 13.09.10   Time: 18:11
 * Updated in $/VSNA/VSClient
 * - fix D3D render
 *
 * *****************  Version 5  *****************
 * User: Sanufriev    Date: 4.06.10    Time: 17:35
 * Updated in $/VSNA/VSClient
 * - Direct3D Render implementation
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 5.03.09    Time: 20:08
 * Updated in $/VSNA/VSClient
 * - multimonitor support
 *
 * *****************  Version 3  *****************
 * User: Melechko     Date: 6.06.08    Time: 20:16
 * Updated in $/VSNA/VSClient
 * Add transparent render
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 18.01.08   Time: 17:02
 * Updated in $/VSNA/VSClient
 * - bigfixes from old architecture.
 * - ambiguous code marked as "old code"
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 23.10.07   Time: 19:13
 * Updated in $/VS2005/VSClient
 * - bugfix #3329
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 10.08.07   Time: 20:23
 * Updated in $/VS2005/VSClient
 * - removed video driver info retrieval every video render
 * reinitialisation (speed-up)
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/VSClient
 *
 * *****************  Version 69  *****************
 * User: Smirnov      Date: 28.07.06   Time: 18:19
 * Updated in $/VS/VSClient
 * - added HighQuality Resampling
 *
 * *****************  Version 68  *****************
 * User: Smirnov      Date: 29.05.06   Time: 18:03
 * Updated in $/VS/VSClient
 * - zero members in DX Render constructor (bug whith PC not having DX )
 *
 * *****************  Version 67  *****************
 * User: Smirnov      Date: 16.02.06   Time: 12:15
 * Updated in $/VS/VSClient
 * - new TheadBase class
 * - receiver now can be inited while in conference
 *
 * *****************  Version 66  *****************
 * User: Smirnov      Date: 28.11.05   Time: 17:09
 * Updated in $/VS/VSClient
 * - added Keep Aspect Ratio flag
 *
 * *****************  Version 65  *****************
 * User: Smirnov      Date: 7.09.05    Time: 17:57
 * Updated in $/VS/VSClient
 * - added proportional scaling (only square pixel)
 *
 * *****************  Version 64  *****************
 * User: Smirnov      Date: 17.12.04   Time: 20:22
 * Updated in $/VS/VSClient
 * header to files
 *
 * *****************  Version 63  *****************
 * User: Smirnov      Date: 4.11.04    Time: 18:56
 * Updated in $/VS/VSClient
 * added release method in video render
 *
 * *****************  Version 62  *****************
 * User: Smirnov      Date: 28.05.04   Time: 18:57
 * Updated in $/VS/VSClient
 * repair render if display mode changed
 *
 * *****************  Version 61  *****************
 * User: Smirnov      Date: 23.03.04   Time: 15:54
 * Updated in $/VS/VSClient
 * added dithering
 *
 * *****************  Version 60  *****************
 * User: Smirnov      Date: 18.03.04   Time: 16:46
 * Updated in $/VS/VSClient
 * render min refresh time
 *
 * *****************  Version 59  *****************
 * User: Smirnov      Date: 17.03.04   Time: 18:47
 * Updated in $/VS/VSClient
 * added dib bilinear
 *
 * *****************  Version 58  *****************
 * User: Smirnov      Date: 16.03.04   Time: 14:20
 * Updated in $/VS/vsclient
 * new capture frame func
 *
 * *****************  Version 57  *****************
 * User: Melechko     Date: 5.03.04    Time: 16:02
 * Updated in $/VS/VSClient
 *
 * *****************  Version 56  *****************
 * User: Smirnov      Date: 4.03.04    Time: 18:03
 * Updated in $/VS/VSClient
 * On size reset mode
 *
 * *****************  Version 55  *****************
 * User: Smirnov      Date: 24.02.04   Time: 15:07
 * Updated in $/VS/VSClient
 * Set auto Mode for Up 2 scaling
 *
 * *****************  Version 54  *****************
 * User: Smirnov      Date: 13.02.04   Time: 14:51
 * Updated in $/VS/vsclient
 * rewroten videoProc
 *
 * *****************  Version 53  *****************
 * User: Smirnov      Date: 11.02.04   Time: 18:52
 * Updated in $/VS/VSClient
 * invert colors for cameras only
 *
 * *****************  Version 52  *****************
 * User: Melechko     Date: 17.12.03   Time: 14:52
 * Updated in $/VS/VSClient
 * Add new virtual out
 *
 * *****************  Version 51  *****************
 * User: Smirnov      Date: 17.12.03   Time: 14:50
 * Updated in $/VS/VSClient
 * added invert color support
 *
 * *****************  Version 50  *****************
 * User: Smirnov      Date: 16.12.03   Time: 18:47
 * Updated in $/VS/VSClient
 * corrected DIB sinc drow
 *
 * *****************  Version 49  *****************
 * User: Smirnov      Date: 28.11.03   Time: 18:02
 * Updated in $/VS/VSClient
 * capture frame function
 *
 * *****************  Version 48  *****************
 * User: Smirnov      Date: 12.09.03   Time: 14:07
 * Updated in $/VS/VSClient
 * DIrectX version forsed to 6.0
 *
 * *****************  Version 47  *****************
 * User: Smirnov      Date: 12.09.03   Time: 11:32
 * Updated in $/VS/VSClient
 * Video render bug 100% load fixed
 *
 * *****************  Version 46  *****************
 * User: Smirnov      Date: 3.09.03    Time: 15:20
 * Updated in $/VS/VSClient
 * bounds checker
 *
 * *****************  Version 45  *****************
 * User: Melechko     Date: 25.08.03   Time: 16:47
 * Updated in $/VS/VSClient
 *
 * *****************  Version 44  *****************
 * User: Melechko     Date: 11.08.03   Time: 16:27
 * Updated in $/VS/VSClient
 * Disable virtual out
 *
 * *****************  Version 43  *****************
 * User: Melechko     Date: 11.08.03   Time: 16:26
 * Updated in $/VS/VSClient
 * add virtual out
 *
 * *****************  Version 42  *****************
 * User: Melechko     Date: 22.07.03   Time: 15:51
 * Updated in $/VS/VSClient
 * New screen clear method
 *
 * *****************  Version 41  *****************
 * User: Smirnov      Date: 15.04.03   Time: 14:45
 * Updated in $/VS/VSClient
 * low DX resources fatal error fixed
 *
 * *****************  Version 40  *****************
 * User: Smirnov      Date: 13.04.03   Time: 17:33
 * Updated in $/VS/VSClient
 * test dx error code
 *
 * *****************  Version 39  *****************
 * User: Smirnov      Date: 11.04.03   Time: 19:40
 * Updated in $/VS/VSClient
 * rewrited all rendering,
 * added support YUY2, UYVY
 *
 * Created 05.10.02
 *
 ****************************************************************************/
#ifndef _VS_RENDER_H
#define _VS_RENDER_H

/****************************************************************************
 * Defines
 ****************************************************************************/
#define VRDDERR_OK					      0
#define VRDDERR_INITDD		VRDDERR_OK +  1
#define VRDDERR_NOTSUP		VRDDERR_OK +  2
#define VRDDERR_MODEUNKNOWN	VRDDERR_OK +  3
#define VRDDERR_FCCUNKNOWN	VRDDERR_OK +  4
#define VRDDERR_CREATESURF	VRDDERR_OK +  5

#define VR_REFRESH_TIME		50
#define VR_REPAIR_TIME		1000

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VSClientBase.h"
#include "VSCapture.h"
#include "../Video/VSVideoProc.h"
#include "../Video/VS_Resize.h"
#include "../std/cpplib/VS_Lock.h"
// D3D
#include <atltypes.h>
#include <atlstr.h>
#include <d3d9.h>
#include <d3dx9shader.h>
#include <D3dx9tex.h>
#include <math.h>
// D3D10
#include <d3d10.h>
//

typedef LRESULT (*RENDERPROC)( HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam,LPVOID lParam1);

/**
****************************************************************************
* \brief Video Render Modes
******************************************************************************/
enum eVideoRenderMode
{
	VRM_UNDEF = -1,		///< undefined
	VRM_DEFAULT = 0,	///< auto determine mode
	VRM_1X2,			///< image will be downsampled  by factor of 2
	VRM_1X1,			///< no scaling
	VRM_1d5X1,			///< image will be interpolated by factor of 1.5
	VRM_2X1,			///< image will be interpolated by factor of 2
	VRM_FS				///< prepare to display on over display
};

enum eVideoRenderType
{
	VR_NONE			= -1,
	VR_DIB			= 0,
	VR_DIRECT_DRAW,
	VR_DIRECT_3D9,
	VR_DIRECT_3D10,
	VR_OPENGL,
	VR_OPENGL_INTERLACE
};

/****************************************************************************
 * Defines
 ****************************************************************************/
#define VR_USE_CONVIN	0x00000001
#define VR_USE_SCALE	0x00000002
#define VR_USE_CONVOUT	0x00000004
#define VR_USE_SATURATE	0x00000008
#define VR_USE_FLIP		0x00000010
#define DX_BCKGR_COLOR	0x00000000
#define DX_TEXT_COLOR	0x004040ff

/****************************************************************************
 * Globals
 ****************************************************************************/
typedef HRESULT (WINAPI *LPDIRECT3DCREATE9EX)(UINT, void **);
typedef HRESULT (WINAPI *LPD3DXLOADSURFASEFROMMEMORY)(LPDIRECT3DSURFACE9	pDestSurface,
													  CONST PALETTEENTRY*	pDestPalette,
													  CONST RECT*			pDestRect,
													  LPCVOID				pSrcMemory,
													  D3DFORMAT				SrcFormat,
													  UINT					SrcPitch,
													  CONST PALETTEENTRY*	pSrcPalette,
													  CONST RECT*			pSrcRect,
													  DWORD					Filter,
													  D3DCOLOR				ColorKey);
typedef HRESULT (WINAPI * LPD3DXCOMPILESHADERFROMFILE)(LPCSTR pSrcFile,
													   const D3DXMACRO *pDefines,
													   LPD3DXINCLUDE pInclude,
													   LPCSTR pFunctionName,
													   LPCSTR pProfile,
													   DWORD Flags,
													   LPD3DXBUFFER *ppShader,
													   LPD3DXBUFFER *ppErrorMsgs,
													   LPD3DXCONSTANTTABLE *ppConstantTable);
typedef HRESULT (WINAPI * LPD3DXCOMPILESHADER)(LPCSTR pSrcData,
											   UINT srcDataLen,
											   const D3DXMACRO *pDefines,
											   LPD3DXINCLUDE pInclude,
											   LPCSTR pFunctionName,
											   LPCSTR pProfile,
											   DWORD Flags,
											   LPD3DXBUFFER *ppShader,
											   LPD3DXBUFFER *ppErrorMsgs,
											   LPD3DXCONSTANTTABLE *ppConstantTable);
typedef HRESULT (WINAPI * LPCREATEDXGIFACTORY)(REFIID riid, void **ppFactory);
typedef HRESULT (WINAPI * LPD3D10COMPILEEFFECTFROMMEMORY)(void *pData,
														  SIZE_T DataLength,
														  LPCSTR pSrcFileName,
														  CONST D3D10_SHADER_MACRO *pDefines,
														  ID3D10Include *pInclude,
														  UINT HLSLFlags,
														  UINT FXFlags,
														  ID3D10Blob **ppCompiledEffect,
														  ID3D10Blob **ppErrors);
typedef HRESULT (WINAPI * LPD3D10CREATEEFFECTFROMMEMORY)(void *pData,
														 SIZE_T DataLength,
														 UINT FXFlags,
														 ID3D10Device *pDevice,
														 ID3D10EffectPool *pEffectPool,
														 ID3D10Effect **ppEffect);
typedef HRESULT (WINAPI * LPCREATEDEVICEANDSWAPCHAIN)(IDXGIAdapter *pAdapter,
													  D3D10_DRIVER_TYPE DriverType,
													  HMODULE Software,
													  UINT Flags,
													  UINT SDKVersion,
													  DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
													  IDXGISwapChain **ppSwapChain,
													  ID3D10Device **ppDevice);


extern HINSTANCE						g_hD3D9;
extern HINSTANCE						g_hD3DX9;
extern LPDIRECT3DCREATE9EX				g_pDirect3DCreate9Ex;
#ifndef D3D_SHADERS_FROM_FILE
extern LPD3DXCOMPILESHADER				g_pD3DXCompileShader;
#else
extern LPD3DXCOMPILESHADERFROMFILE		g_pD3DXCompileShader;
#endif
extern HINSTANCE						g_hD3D10;
extern HINSTANCE						g_hDXGI;
extern LPCREATEDXGIFACTORY				g_CreateDXGIFactory;
extern LPCREATEDEVICEANDSWAPCHAIN		g_D3D10CreateDeviceAndSwapChain;
extern LPD3D10COMPILEEFFECTFROMMEMORY	g_D3D10CompileEffectFromMemory;
extern LPD3D10CREATEEFFECTFROMMEMORY	g_D3D10CreateEffectFromMemory;

/**
 **************************************************************************
 * \brief base video rander class
 ****************************************************************************/
class CVideoRenderBase:  public CColorSpace, public VS_Lock, public CVSInterface
{
protected:
	enum Resize_Mode
	{
		RM_DEFAULT = 0,
		RM_BILINEAR = 1,
		RM_BICUBIC = 2,
		RM_LANCZOS = 3,
		RM_HQBICUBIC = 4
	};
	struct VS_DisplayName
	{
		int width;
		int height;
		std::wstring name;
		RECT rName;
		bool update;
	};
private:
	DWORD				m_renderTime;
protected:
	BITMAPINFOHEADER	m_biFmtIn;			///< input image format
	BITMAPINFOHEADER	m_biFmtDraw;		///< format to draw
	int					m_height;			///< doubled m_biFmtIn
	int					m_width;			///< doubled m_biFmtIn
	RECT				m_rPic;				///< additional parametr
	RECT				m_rPic2;			///< additional parametr

	DWORD				m_dwVRUse;			///< scale-conv-satur flag
	BYTE*				m_pBufferIn;		///< pointer to input image
	BYTE*				m_pBufferConvIn;	///< pointer to converted from input image
	BYTE*				m_pBufferMirrorIn;	///< pointer to mirror input image
	BYTE*				m_pBufferSaturated;	///< pointer to saturated from converted image
	BYTE*				m_pBufferScaled;	///< pointer to scaled from saturated image
	BYTE*				m_pBufferConvOut;	///< pointer to converted from scaled image
	BYTE*				m_pBufferConvOutTmp;///< pointer to converted from scaled image
	VS_VideoProc*		m_pVSVideoProc;		///< class of video procesiing tools

	static BOOL			m_bForceBicubic;
	BOOL				m_bForceBicubicChange;
	bool				m_bFlipFrame;
	bool				m_bSelfView;
	bool				m_bDrawLogo;

	long				m_bKeepAspectRatio;	///< keep aspect ratio flag
	long				m_ResizeMode;		///< Resize Mode
	int					m_renderFrameSizeMB;

	static DWORD		m_dwStereoRender;
	static DWORD		m_dwTypeRender;

	bool					ConvertColorSpaceIn();
	bool					MirrorVertical();
	bool					Saturate();
	bool					ResampleImage();
	bool					Dither();
	bool					ConvertColorSpaceOut(BYTE* dst, DWORD dstWidth);
	void					CleanBuffers();
	void					DrawBorders();

	eVideoRenderMode		DetermineMode(HWND hwnd);
	HWND					m_hwnd;
	VS_HQResampling*		m_HQRes;
	VS_DisplayName			m_DisplayName;
	VS_DisplayName			m_DisplayLogo;
	eVideoRenderType		m_TypeRender;
	std::vector<uint8_t>	m_BorderTemplateY;
	std::vector<uint8_t>	m_BorderTemplateUV;
	bool					m_DrawBorders = false;
	uint8_t					m_BorderAlpha = 0;
public:
	CVideoRenderBase(CVSInterface* pParentInterface);
	virtual	~CVideoRenderBase();
    int	iInitRender(HWND hwnd, unsigned char *pBuffer, CColorMode *pCm, bool bSelfView);
	void Release();
	static LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual int	SetMode(eVideoRenderMode Mode, HWND hwnd)=0;
	static void	ForceBicubic(DWORD force){m_bForceBicubic = !!force;}
	virtual bool PrepareFrame(HWND m_hwnd)=0;
	virtual int DrawFrame(HWND hwnd)=0;
	virtual void CheckMonitor(HWND hwnd) {}
	void SetDisplayName(wchar_t *name);
	virtual bool IsNeedResetOnSize() {return false;}
	virtual int GetFrameSizeMB() { return m_renderFrameSizeMB; }
	int CaptureFrame(unsigned char* frame, int x, int y);
	void SetFlipFrame(bool flip);
	bool IsInited();
	static void Open();
	static void Close();
	static CVideoRenderBase* RetrieveVideoRender(HWND hwnd, CVSInterface* pParentInterface, unsigned int iStereo = 0);
	void EnableBorders(bool enable);
	void SetBordersAlpha(uint8_t alpha);

	int					m_displaydepth;		///< bits perpixel on display
	RECT				m_display;			///< display info
	int					m_error_code;		///< Erorr code
	bool				m_IsValid;			///< Valid flags

	eVideoRenderMode	m_RenderMode;		///< store last video render mode
	BOOL				m_bCanFS;			///< Can Draw in FS Mode
	static BOOL			m_bUseDX;
	static BOOL			m_bUseD3D;
	BOOL				m_bNewFrame;		///< show new frame is present
	static DWORD		m_dwSaturation;		///< show new frame is present
private:
  int ProcessFunction(VS_INTERFACE_OPERATION VS_OPERATION, const char* pSection, VARIANT* pVar) override;
#ifdef _VIRTUAL_OUT
private:
	bool m_bVirtualOut;
	HANDLE m_hMMFile;
	unsigned char *m_pMMMem;
	bool (*CheckState)(void*);
	void *m_Param;
public:
	void SetVirtualOut(bool bVirtualOut,void*pProc,void *pParam);
protected:
	void VirtualOut();						///< out image to virtual video driver
#endif
};

/**
****************************************************************************
* \brief DIB Render Class. Use DIB Windows API functions to draw
******************************************************************************/
class CDIBRender:public CVideoRenderBase{
protected:
	DWORD				m_DrawPitch;		///< pitch for drawn image
	BYTE*				m_pBufferDIBDraw;	///< pointer to converted from scaled image
public:
							CDIBRender(CVSInterface* pParentInterface);
							~CDIBRender();
	int						SetMode(eVideoRenderMode Mode, HWND hwnd);
	bool					PrepareFrame(HWND m_hwnd);
	int						DrawFrame(HWND hwnd);
};
typedef int   (__stdcall  tTransparentStretch)(HWND hwnd,HDC hdc, int xDest, int yDest, int DestWidth,  int DestHeight, int xSrc, int ySrc, int SrcWidth, int SrcHeight,
										   CONST VOID * lpBits,CONST BITMAPINFO * lpbmi,  UINT iUsage,  DWORD rop);

class CTransDIBRender: public CDIBRender{
public:
	CTransDIBRender(CVSInterface* pParentInterface);
	int						DrawFrame(HWND hwnd);
	static tTransparentStretch     *m_StretchFunc;
};

/// forse to DirectDraw 6.0
#ifdef DIRECTDRAW_VERSION
#define DIRECTDRAW_VERSION_OLD DIRECTDRAW_VERSION
#undef DIRECTDRAW_VERSION
#endif

#define DIRECTDRAW_VERSION 0x0600
#include <ddraw.h>


/**
****************************************************************************
* \brief Additional caps used in CDirectXRender
******************************************************************************/
struct DIRECTDROWCAPS
{
	BOOL	bAnyHardSupp;
	BOOL	bOverlayFcc;
	BOOL	bBltFcc;
	BOOL	bOverlayScaleUp;
	BOOL	bBltScaleUp;
	BOOL	bOverlayScaleDown;
	BOOL	bBltScaleDown;
	BOOL	bFccUYVY;
	BOOL	bFccYUY2;
	BOOL	bFccRGB32;
	BOOL	bFccRGB24;
	BOOL	bHaveBelin;
	DWORD	dwMem;
	DWORD	dwMemFree;
};

struct VSMonitorInfo
{
	GUID		m_guid;
	HMONITOR	m_monitor;
};

/**
****************************************************************************
* \brief DirectDraw Render class
******************************************************************************/
class CDirectXRender: public CVideoRenderBase
{
private:
	DWORD					m_repairTime;		///< repair period if not valid
	// DirectX interfaces and structures
	LPDIRECTDRAW4			m_pDD;				///< main interfase
	LPDIRECTDRAWSURFACE4	m_pPrimarySurface;	///< primary surface
	LPDIRECTDRAWSURFACE4	m_pOffSurface;		///< working off-screen or overlay surface
	LPDIRECTDRAWSURFACE4	m_pOffSurfaceMain;	///< working main off-screen or overlay surface
	LPDIRECTDRAWSURFACE4	m_pTextSurface;		///< working test surface
	LPDIRECTDRAWCLIPPER		m_pClipper;			///< clipper interface

	DDSURFACEDESC2			m_ddSurfaceDesc;	///< surfase format description
	DDCAPS					m_ddCaps;			///< DirectDraw capabilities structure

	RECT					m_rcb1;
	RECT					m_rcb2;
	RECT					m_rPicMain;			///< additional parametr
	RECT					m_rPicMain2;		///< additional parametr

	HMONITOR				m_monitor;
	MONITORINFOEX			m_minfo;

	int InitDirectDraw();
	void RemoveDirectDraw();
	BOOL CreateSurfaces(int x, int y, HWND hwnd);
	bool RestoreOffcreenMainSurface();
	bool ResetOffcreenMainSurface();
	bool SetRectangle(HWND hwnd);
	int TestDDCaps();
	void CheckMonitor(HWND hwnd);
	int CreateSurfaceName(int w, int h);

public:
	VSMonitorInfo			m_VideoDrv[20];
	DWORD					m_VideoDrvIndex;
    DIRECTDROWCAPS	m_DirectDrawCaps;
	CDirectXRender(CVSInterface* pParentInterface);
	~CDirectXRender();
	int	SetMode(eVideoRenderMode Mode, HWND hwnd);
	bool PrepareFrame(HWND hwnd);
	int DrawFrame(HWND hwnd);
	bool FillDeviceInfo(char* driver, char* desc);
	bool IsNeedResetOnSize();
};

#ifdef DIRECTDRAW_VERSION_OLD
#undef DIRECTDRAW_VERSION
#define DIRECTDRAW_VERSION DIRECTDRAW_VERSION_OLD
#endif

/**
****************************************************************************
* \brief D3D Render class
******************************************************************************/

struct VS_D3DCAPS
{
	UINT uWidth;
	UINT uHeight;
	UINT uDepth;
	UINT uShaderVerMajor;
	UINT uShaderVerMinor;
	UINT uTypeShaderResampler;
	UINT uVertexProcessing;
	UINT uMemAvailable;
	UINT uGPUPriotity;
	char Error[8192];
};

class VS_Direct3DRender: public CVideoRenderBase
{
protected:

	struct vector3D
	{
		float x, y, z;
	};

	struct CustomVertex3D
	{
		float x, y, z, rhw;
		float u, v;
	};

	char	m_cError[8192];
	CSize	m_ScreenSize;
	UINT	m_RefreshRate;

	CComPtr<IDirect3D9Ex>			m_pD3DEx;
	CComPtr<IDirect3D9>				m_pD3D;
	CComPtr<IDirect3DDevice9Ex>		m_pD3DDevEx;
	CComPtr<IDirect3DDevice9>		m_pD3DDev;
	CComPtr<IDirect3DTexture9>		m_pVideoTexture;
	CComPtr<IDirect3DTexture9>		m_pNameTexture;
	CComPtr<IDirect3DSurface9>		m_pVideoSurface;
	CComPtr<IDirect3DTexture9>		m_pTextureI420[3];
	CComPtr<IDirect3DTexture9>		m_pTextureYUV444[3];
	CComPtr<IDirect3DSurface9>		m_pSurfaceYUV444[3];

	UINT							m_uAdapter;
	D3DFORMAT						m_SurfaceType;
	D3DFORMAT						m_BackbufferType;
	D3DFORMAT						m_DisplayType;
	D3DTEXTUREFILTERTYPE			m_filter;
	D3DPRESENT_PARAMETERS			m_pp;

	///< D3D caps
	D3DCAPS9						m_caps;
	D3DDISPLAYMODE					m_d3ddm;

	DWORD							m_DrawPitch;		///< pitch for drawn image
	DWORD							m_repairTime;		///< repair period if not valid
	RECT							m_rcb1;
	RECT							m_rcb2;
	RECT							m_rDstVid;
	RECT							m_rDstSci;
	RECT							m_rViewArea;
	RECT							m_rViewAreaPrev;

	CComPtr<IDirect3DPixelShader9>	m_pPSResizer[2];
	CComPtr<IDirect3DPixelShader9>	m_pPSYUV2RGB;
	CComPtr<IDirect3DTexture9>		m_pScreenSizeTemporaryTexture;

	HRESULT CreateDirect3D();
	HRESULT CreateDevice(HWND hwnd);
	HRESULT AllocSurfaces(int width, int height, D3DFORMAT Format = D3DFMT_X8R8G8B8);
	void DeleteSurfaces();
	bool ResetDevice(HWND hwnd, int width, int height);
	bool SetRectangle(HWND hwnd);
	UINT GetAdapter(IDirect3D9 *pD3D, HWND hwnd);

	void SetTransform(CRect r, vector3D vct[4]);
	void SetVertex(CustomVertex3D *vertex, vector3D vct, float u, float v);
	void SetVertex(CustomVertex3D *vertex, float x, float y, float z, float u, float v);
	void AdjustVertex(CustomVertex3D *vertex, float dx, float dy);

	HRESULT InitResizers();
	HRESULT TextureBlt(CustomVertex3D v[4], D3DTEXTUREFILTERTYPE filter = D3DTEXF_LINEAR, int iTexturesCoord = 1);
	HRESULT TextureAlphaBlt(CustomVertex3D v[4], D3DTEXTUREFILTERTYPE filter = D3DTEXF_LINEAR);
	HRESULT TextureResize(CComPtr<IDirect3DTexture9> pTexture, vector3D vct[4], D3DTEXTUREFILTERTYPE filter, const CRect &SrcRect);
	HRESULT ConvertI420toYUV444(CComPtr<IDirect3DTexture9> pSrcTexture[3], CComPtr<IDirect3DSurface9> pDstSurface[3], const CRect &SrcRect);
	HRESULT ConvertYUV444toRGBA(CComPtr<IDirect3DTexture9> pSrcTexture[3], CComPtr<IDirect3DSurface9> pDstSurface, const CRect &SrcRect, float saturation);
	HRESULT ResamplingRGBA(CComPtr<IDirect3DTexture9> pTexture, vector3D dst[4], const CRect &SrcRect);
	HRESULT DrawDisplayName(CComPtr<IDirect3DTexture9> pTextureText, const CRect &SrcRect);
	void ClearNonPaintedArea();

	int CreateTextureName(int w, int h);

public:

	VS_Direct3DRender(CVSInterface* pParentInterface, HWND hwnd);
	~VS_Direct3DRender();
	bool TestD3DCaps(HWND hwnd);
	int	SetMode(eVideoRenderMode Mode, HWND hwnd);
	bool PrepareFrame(HWND hwnd);
	int DrawFrame(HWND hwnd);
	bool FillDeviceInfo(char* driver, char* desc);
	void GetDirect3DCaps(VS_D3DCAPS *caps);
};

class VS_Direct3D10Render: public CVideoRenderBase
{
private:

	enum eEffectVarName
	{
		SATURATE = 0,
		TEXEL05 = 1,
		TEXEL,
		TEXELU,
		TEXELV,
		IMAGESIZE,
		NUM_EFFECTVAR_NAME,
	};

	enum eStagesName
	{
		I420 = 0,
		YUV444_P0 = 1,
		YUV444_P1,
		YUV444_P2,
		RGBA,
		RESAMPLE_W,
		MAIN,
		DISPLAY_NAME,
		DISPLAY_LOGO,
		NUM_STAGESNAME_NAME
	};

protected:

	IDXGIFactory						*m_pDXGIFactory;
	IDXGIAdapter						*m_pDXGIAdapter;
	IDXGIDevice							*m_pDXGIDevice;
	ID3D10Device						*m_pDeviceD3D10;
	IDXGISwapChain						*m_pSwapChain;
	ID3D10RenderTargetView				*m_pRenderTarget[NUM_STAGESNAME_NAME];
	ID3D10Texture2D						*m_pTexture[NUM_STAGESNAME_NAME];
	ID3D10BlendState					*m_pBlendState;
	/// shader vars
	ID3D10ShaderResourceView			*m_pShaderResView[NUM_STAGESNAME_NAME];
	ID3D10EffectShaderResourceVariable	*m_pShaderResource[NUM_STAGESNAME_NAME];
	ID3D10EffectScalarVariable			*m_pEffectVar[NUM_EFFECTVAR_NAME];
	ID3D10EffectVectorVariable			*m_pEffectVarVect[NUM_EFFECTVAR_NAME];
	/// rasterizer
	ID3D10RasterizerState				*m_pRS;
	/// input layout and vertex buffer
	ID3D10Buffer						*m_pVertexBuffer;
	ID3D10InputLayout					*m_pVertexLayout;
	/// effects and techniques
	ID3D10Effect						*m_pEffect;
	ID3D10EffectTechnique				*m_pTechnique;

	char							m_cError[8192];

	unsigned int					m_numVertices;
	DWORD							m_DrawPitch;		///< pitch for drawn image
	DWORD							m_repairTime;		///< repair period if not valid
	RECT							m_rcb1;
	RECT							m_rcb2;
	RECT							m_rDstVid;
	RECT							m_rDstSci;
	RECT							m_rViewArea;
	RECT							m_rViewAreaPrev;

	bool SetRectangle(HWND hwnd);
	HRESULT CreateDevice(HWND hwnd, int width, int height);
	HRESULT CreateDevice3D(HWND hwnd, int width, int height);
	HRESULT InitInputAssemplyStage();
	HRESULT InitRasterizerStage();
	HRESULT InitSceneVariables();
	HRESULT AllocTextures(int width, int height, DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM);
	void DeleteTextures();
	void ReleaseDevice();
	bool ResetDevice(HWND hwnd, int back_buffer_width, int back_buffer_height, int width, int height);
	HRESULT ResetSwapChain(HWND hwnd, int back_buffer_width, int back_buffer_height, int width, int height);

	void	SetViewports(int TopLeftX, int TopLeftY, int width, int height);
	HRESULT SetVertexBuffer(float tex_u0, float tex_u1, float tex_v0, float tex_v1);

	HRESULT ConvertI420toYUV444();
	HRESULT ConvertYUV444toRGBA(int saturation);
	HRESULT ResamplingRGBA();
	HRESULT ResamplingRGBA_WeightTex();
	HRESULT CopyRGBA();
	HRESULT DrawDisplayName();
	HRESULT DrawDisplayLogo();

	HRESULT FillWeightTexture(int w, int h);
	int CreateTextureName(int w, int h);
	int CreateTextureLogo(int w, int h);

public:

	VS_Direct3D10Render(CVSInterface* pParentInterface, HWND hwnd);
	~VS_Direct3D10Render();
	bool TestD3DCaps(HWND hwnd);
	int	SetMode(eVideoRenderMode Mode, HWND hwnd);
	bool PrepareFrame(HWND hwnd);
	int DrawFrame(HWND hwnd);
	bool FillDeviceInfo(char* driver, char* desc);
	void GetDirect3DCaps(VS_D3DCAPS *caps);
};

#endif /*_VSRENDER_H*/
