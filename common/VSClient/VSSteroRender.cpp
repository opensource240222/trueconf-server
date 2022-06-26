/**
 **************************************************************************
 * \file VSStereoRender.h
 * \brief Video render classes for stereo data.
 *
 * \b Project Client
 * \author I.A.Melechko
 * \date 17.02.2011
 *
 * $Revision: 12 $
 *
 * $History: VSSteroRender.cpp $
 *
 * *****************  Version 12  *****************
 * User: Melechko     Date: 11.07.11   Time: 13:09
 * Updated in $/VSNA/VSClient
 * Add anaglyph render
 *
 * *****************  Version 11  *****************
 * User: Builder      Date: 8.07.11    Time: 13:01
 * Updated in $/VSNA/VSClient
 * [ktrushnikov]
 * - link of glew32s.lib in AppDependedImp (not in VSStereoRender)
 * - different path in SingleGateway
 * - ^VideoBot: Debug Win32: defines fixed
 *
 * *****************  Version 10  *****************
 * User: Sanufriev    Date: 6.07.11    Time: 16:35
 * Updated in $/VSNA/VSClient
 * - were added auto stereo mode detect
 *
 * *****************  Version 9  *****************
 * User: Melechko     Date: 24.06.11   Time: 19:32
 * Updated in $/VSNA/VSClient
 * Add OpenGL interlaced render
 *
 * *****************  Version 8  *****************
 * User: Melechko     Date: 15.06.11   Time: 17:42
 * Updated in $/VSNA/VSClient
 * fix interlaced render
 *
 * *****************  Version 7  *****************
 * User: Melechko     Date: 23.05.11   Time: 16:39
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 6  *****************
 * User: Melechko     Date: 18.05.11   Time: 13:16
 * Updated in $/VSNA/VSClient
 * Add opengl render
 *
 * *****************  Version 5  *****************
 * User: Melechko     Date: 2.03.11    Time: 19:00
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 4  *****************
 * User: Melechko     Date: 2.03.11    Time: 17:37
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 3  *****************
 * User: Melechko     Date: 2.03.11    Time: 17:32
 * Updated in $/VSNA/VSClient
 *
 * *****************  Version 2  *****************
 * User: Melechko     Date: 21.02.11   Time: 20:18
 * Updated in $/VSNA/VSClient
 * fix anaglyph color
 *
 * *****************  Version 1  *****************
 * User: Melechko     Date: 17.02.11   Time: 14:19
 * Created in $/VSNA/VSClient
 * Add stereo renders
 **/
#include "VSStereoRender.h"
#include "VS_Dmodule.h"
#include "../Video/VSVideoDefines.h"

CDIBRenderStereoAnaglyph::CDIBRenderStereoAnaglyph(CVSInterface* pParentInterface)
:CDIBRender(pParentInterface)
{
};
extern BYTE cipping_table[];
bool CDIBRenderStereoAnaglyph::PrepareFrame(HWND m_hwnd){
     if (m_bNewFrame) {
		if (m_dwVRUse&VR_USE_CONVIN)
			if (!ConvertColorSpaceIn()) return false;
		if (!Saturate()) return false;
#ifdef _VIRTUAL_OUT
		VirtualOut();
#endif
		if (m_dwVRUse&VR_USE_SCALE)
			if (!ResampleImage()) return false;
		if (!Dither()) return false;

		if (m_dwVRUse&VR_USE_CONVOUT)
			if (!ConvertColorSpaceOut(m_pBufferConvOut, m_DrawPitch)) return false;
        //memset(m_pBufferConvOut,128,640*200*4);
		if(m_biFmtDraw.biCompression== BI_RGB)
			if (m_biFmtDraw.biBitCount==32){
				BYTE *pTmpR=m_pBufferConvOut;
				BYTE *pTmpL=m_pBufferConvOut+(m_biFmtDraw.biHeight*m_biFmtDraw.biWidth/2)*4;
				for(int i=0;i<(m_biFmtDraw.biHeight*m_biFmtDraw.biWidth/2);i++){
					int B,G,R;
					R=pTmpL[2]*27224+pTmpL[1]*30867+pTmpL[0]*10938-pTmpR[2]*714-pTmpR[1]*2386-pTmpR[0]*393;
					G=-pTmpL[2]*3002-pTmpL[1]*3173-pTmpL[0]*1684+pTmpR[2]*24614+pTmpR[1]*48055+pTmpR[0]*726;
					B=-pTmpL[2]*3584-pTmpL[1]*4029+pTmpL[0]*840-pTmpR[2]*4265-pTmpR[1]*8433+pTmpR[0]*85007;
					pTmpR[0]=CLIP(B>>16);
					pTmpR[1]=CLIP(G>>16);
					pTmpR[2]=CLIP(R>>16);
					pTmpR+=4;
					pTmpL+=4;
/*
27224	30867	10938		-714	-2386	-393
-3002	-3173	-1684		24614	48055	726
-3584	-4029	840		    -4265	-8433	85007
*/
				}
			}
		m_bNewFrame = FALSE;
	}
	return true;
};
int CDIBRenderStereoAnaglyph::DrawFrame(HWND hwnd)
{
	if (!m_IsValid) return 1;
	Lock();

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
				if (m_rPic.right != m_biFmtDraw.biWidth || m_rPic.bottom != m_biFmtDraw.biHeight/2) {
					if (!EqualRect(&m_rPic, &m_rPic2) || !m_pBufferDIBDraw) {
						if (m_pBufferDIBDraw)
							delete [] m_pBufferDIBDraw;
						m_pBufferDIBDraw = new BYTE [m_rPic.right*m_rPic.bottom*m_displaydepth/8];
						memset(m_pBufferDIBDraw, 0, m_rPic.right*m_rPic.bottom*m_displaydepth/8);
						m_rPic2 = m_rPic;
					}

					float x = (float)m_biFmtDraw.biWidth/m_rPic.right;
					float y = (float)(m_biFmtDraw.biHeight/2)/m_rPic.bottom; // src/dst
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
					x_offs/=2;
					y_offs/=2;

					if (bih.biBitCount==24) {
						BYTE *p = m_pBufferDIBDraw + y_offs*bih.biWidth*3 + x_offs*3;
						m_pVSVideoProc->ResampleRGB24(m_pBufferConvOut, p, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight/2, m_rPic.right, m_rPic.bottom, bih.biWidth*3);
					}
					else {
						BYTE *p = m_pBufferDIBDraw + y_offs*bih.biWidth*4 + x_offs*4;
						m_pVSVideoProc->ResampleRGB32(m_pBufferConvOut, p, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight/2, m_rPic.right, m_rPic.bottom, bih.biWidth*4);
					}
					if ( StretchDIBits(hdc, 0, 0, bih.biWidth, bih.biHeight,
						0, 0, bih.biWidth, bih.biHeight, m_pBufferDIBDraw, (BITMAPINFO*)&bih, 0, SRCCOPY) == GDI_ERROR)
						ret = 1;
				}
				else {
					if ( StretchDIBits(hdc, 0, 0, m_rPic.right, m_rPic.bottom, 0, 0, m_biFmtDraw.biWidth,
						m_biFmtDraw.biHeight/2, m_pBufferConvOut, (BITMAPINFO*)&m_biFmtDraw, 0, SRCCOPY) == GDI_ERROR)
						ret = 1;
				}
			}
			ReleaseDC(hwnd, hdc);
		}
	}
	UnLock();
	return ret;
}
CDIBRenderStereoInterlaced::CDIBRenderStereoInterlaced(CVSInterface* pParentInterface)
:CDIBRender(pParentInterface)
{
	m_pBufferDIBString=0;
};
CDIBRenderStereoInterlaced::~CDIBRenderStereoInterlaced()
{
	if (m_pBufferDIBString)
       delete [] m_pBufferDIBString;

};

int CDIBRenderStereoInterlaced::DrawFrame(HWND hwnd)
{
	if (!m_IsValid) return 1;
	Lock();

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

					if (!EqualRect(&m_rPic, &m_rPic2) || !m_pBufferDIBDraw) {
						if (m_pBufferDIBDraw)
							delete [] m_pBufferDIBDraw;
						if (m_pBufferDIBString)
                            delete [] m_pBufferDIBString;
						m_pBufferDIBDraw = new BYTE [m_rPic.right*m_rPic.bottom*m_displaydepth/8];
                        m_pBufferDIBString = new BYTE [m_rPic.right*m_rPic.bottom*m_displaydepth/8];
						memset(m_pBufferDIBDraw, 0, m_rPic.right*m_rPic.bottom*m_displaydepth/8);
						m_rPic2 = m_rPic;
					}

					float x = (float)m_biFmtDraw.biWidth/m_rPic.right;
					float y = (float)(m_biFmtDraw.biHeight/2)/m_rPic.bottom; // src/dst
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
					int iPitch=bih.biWidth*m_displaydepth/8;
					BYTE *pTmpBuff=m_pBufferDIBString;
                    BYTE *pSrc1;
					BYTE *pSrc2;
					POINT pt;
					pt.x=0;
					pt.y=0;
					ClientToScreen(hwnd,&pt);
					if((pt.y&1)==1){
						 pSrc1=m_pBufferDIBDraw;
					     pSrc2=m_pBufferDIBDraw +iPitch*bih.biHeight/2;
					}
					else{
						pSrc1=m_pBufferDIBDraw +iPitch*bih.biHeight/2;
						pSrc2=m_pBufferDIBDraw;
					}
					for(int i=0;i<bih.biHeight/2;i++){
						memcpy(pTmpBuff,pSrc1,iPitch);
						pTmpBuff+=iPitch;
						memcpy(pTmpBuff,pSrc2,iPitch);
						pTmpBuff+=iPitch;
						pSrc1+=iPitch;
						pSrc2+=iPitch;
					}
					if ( StretchDIBits(hdc, 0, 0, bih.biWidth, bih.biHeight,0, 0, bih.biWidth, bih.biHeight, m_pBufferDIBString, (BITMAPINFO*)&bih, 0, SRCCOPY) == GDI_ERROR)
						ret = 1;
				}

			ReleaseDC(hwnd, hdc);
		}
		}
	UnLock();
	return ret;
}

// Stereo Blitdefines
#define NVSTEREO_IMAGE_SIGNATURE 0x4433564e //NV3D


typedef struct _Nv_Stereo_Image_Header
{
unsigned int dwSignature;
unsigned int dwWidth;
unsigned int dwHeight;
unsigned int dwBPP;
unsigned int dwFlags;
} NVSTEREOIMAGEHEADER, *LPNVSTEREOIMAGEHEADER;


// ORedflags in the dwFlagsfielsof the _Nv_Stereo_Image_Headerstructure above
#define SIH_SWAP_EYES 0x00000001
#define SIH_SCALE_TO_FIT 0x00000002

bool CDirect3DRenderStereo::m_bClassRegistered=false;
WNDCLASSEX CDirect3DRenderStereo::m_wc;

CDirect3DRenderStereo::CDirect3DRenderStereo(CVSInterface* pParentInterface):CDIBRender(pParentInterface){
  m_ServiceHWND=0;
  m_bFullScreen=FALSE;
  if(!m_bClassRegistered){
	  m_bClassRegistered=true;

    // clear out the window class for use
    ZeroMemory(&m_wc, sizeof(WNDCLASSEX));

    // fill in the struct with the needed information
    m_wc.cbSize = sizeof(WNDCLASSEX);
    m_wc.style = CS_HREDRAW | CS_VREDRAW;
    m_wc.lpfnWndProc = WindowProc2;//WindowProc;
    m_wc.hInstance = GetModuleHandle(0);
    m_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    // wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    m_wc.lpszClassName = "VSnVidiaRender";

    // register the window class
    RegisterClassEx(&m_wc);
  }
};
CDirect3DRenderStereo::~CDirect3DRenderStereo(){
	if(m_bFullScreen){
		DestroyWindow(m_ServiceHWND);
	}
	DeleteSurfaces();
    m_pD3DDev	= NULL;
	m_pD3D = NULL;
};
int	CDirect3DRenderStereo::SetMode(eVideoRenderMode Mode, HWND hwnd)
{
	VS_AutoLock lock(this);
//	if((hwnd==m_hwnd))

	/*if (Mode== VRM_DEFAULT)
		Mode = DetermineMode(hwnd);
	if (m_RenderMode!= VRM_UNDEF)
		if (m_RenderMode==Mode)
			return 0;*/

	// begin reinitialisation
	m_IsValid = false;

	m_displaydepth = 32;

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
	// always use saturate
	m_pBufferSaturated = new BYTE[m_width*m_height*3/2];
	m_dwVRUse|=VR_USE_SATURATE;

	m_biFmtDraw.biWidth = m_width;
	m_biFmtDraw.biHeight = m_height;
	m_pBufferScaled = m_pBufferSaturated;

	/*if (m_ResizeMode==RM_DEFAULT) {
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
	}*/

	m_biFmtDraw.biSizeImage = m_displaydepth*m_biFmtDraw.biWidth*m_biFmtDraw.biHeight/8;
	m_pBufferConvOut = new BYTE [m_biFmtDraw.biSizeImage];
	m_dwVRUse|=VR_USE_CONVOUT;	// always
	m_hwnd = hwnd;
    if (!ResetDevice(hwnd, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight)) {
			return VRDDERR_CREATESURF;
	}
	m_DrawPitch = m_biFmtDraw.biWidth*m_displaydepth/8;
	m_RenderMode = Mode;
	m_bNewFrame = 1;
	m_IsValid = true;

	/*VS_AutoLock lock(this);
	if (!IsInited())
			return 1;

	if (!SetRectangle(hwnd))
		return 0;

	CRect rect;
	SetRect(&rect, 0, 0, m_rPic.right - m_rPic.left, m_rPic.bottom - m_rPic.top);

	if (!m_IsValid) {
		m_displaydepth = 32;
		m_biFmtDraw.biCompression = BI_RGB;
		m_biFmtDraw.biBitCount	  = m_displaydepth;
		CleanBuffers();

		switch(m_biFmtIn.biCompression)		// input format
		{
		case FCC_I420:
		case FCC_IYUV:
			m_pBufferConvIn = m_pBufferIn;
			break;
		case FCC_YV12:
		case FCC_YUY2:
		case FCC_UYVY:
		case BI_RGB:
			m_pBufferConvIn = new BYTE[m_width*m_height*3/2];
			m_dwVRUse|=VR_USE_CONVIN;
			break;
		}
		// always use saturate
		m_pBufferSaturated = new BYTE[m_width*m_height*3/2];
		m_dwVRUse|=VR_USE_SATURATE;

		m_biFmtDraw.biWidth = m_width;
		m_biFmtDraw.biHeight = m_height;
		m_pBufferScaled = m_pBufferSaturated;
		m_DrawPitch = m_biFmtDraw.biWidth * m_biFmtDraw.biBitCount / 8;
		SetRect(&m_rPic2, 0, 0, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight);// sourse image dimensions
		m_biFmtDraw.biSizeImage = m_biFmtDraw.biBitCount*m_biFmtDraw.biWidth*m_biFmtDraw.biHeight/8;
		m_pBufferConvOut = new BYTE [m_biFmtDraw.biSizeImage];
		m_pBufferConvOutTmp = new BYTE [m_biFmtDraw.biSizeImage];
		m_dwVRUse|=VR_USE_CONVOUT;	// always

		CopyRect(&m_rDstVid, &rect);
		CopyRect(&m_rDstSci, &rect);
		CopyRect(&rect, &m_rcb1);
		int dx = (rect.Width() == m_rDstVid.right) ? 0 : rect.Width();
		int dy = (rect.Height() == m_rDstVid.bottom) ? 0 : rect.Height();
		OffsetRect(&m_rDstSci, dx, dy);
		CopyRect(&m_rViewAreaPrev, &m_rViewArea);
		if (!ResetDevice(hwnd, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight)) {
			return VRDDERR_CREATESURF;
		}
	} else {
		if (!EqualRect(&m_rViewAreaPrev, &m_rViewArea) || m_hwnd != hwnd) {
			HRESULT hr = D3D_OK;
			CopyRect(&m_rDstVid, &rect);
			CopyRect(&m_rDstSci, &rect);
			CopyRect(&rect, &m_rcb1);
			int dx = (rect.Width() == m_rDstVid.right) ? 0 : rect.Width();
			int dy = (rect.Height() == m_rDstVid.bottom) ? 0 : rect.Height();
			OffsetRect(&m_rDstSci, dx, dy);
			GetClientRect(hwnd, &rect);
			m_pp.BackBufferWidth = rect.Width();
			m_pp.BackBufferHeight = rect.Height();
			m_pp.hDeviceWindow = hwnd;
			//D3DDISPLAYMODE d3ddmDesktop;
			//m_pD3D->GetAdapterDisplayMode(GetAdapter(m_pD3D,hwnd), &d3ddmDesktop);
			if (!ResetDevice(hwnd, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight)) {
			  return VRDDERR_CREATESURF;
		    }
			hr |= AllocSurfaces(m_biFmtDraw.biWidth, m_biFmtDraw.biHeight);
			if (FAILED(hr)) {
				DTRACE(VSTM_VRND, "failed reinit d3d");
				m_IsValid = false;
				return 1;
			}
			CopyRect(&m_rViewAreaPrev, &m_rViewArea);
		}
	}

	m_RenderMode = Mode;
	m_bNewFrame = 1;
	m_IsValid = true;
	m_hwnd = hwnd;
*/
	return 0;
}
UINT CDirect3DRenderStereo::GetAdapter(IDirect3D9* pD3D, HWND hwnd)
{
	if (hwnd == NULL || pD3D == NULL) {
		return D3DADAPTER_DEFAULT;
	}

	HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	if (hMonitor == NULL) return D3DADAPTER_DEFAULT;

	UINT adp = 0, num_adp = 0;
	for(adp = 0, num_adp = pD3D->GetAdapterCount(); adp < num_adp; ++adp) {
		HMONITOR hAdpMon = pD3D->GetAdapterMonitor(adp);
		if (hAdpMon == hMonitor) {
			return adp;
		}
	}

	return D3DADAPTER_DEFAULT;
}

bool CDirect3DRenderStereo::ResetDevice(HWND hwnd, int width, int height)
{
	HRESULT hr;

	RECT rPic;
	GetClientRect(hwnd, &rPic);
	/*if((rPic.right-rPic.left==m_d3ddm.Width)&&(rPic.bottom-rPic.top==m_d3ddm.Height)){
		m_bFullScreen=TRUE;
	}
	else*/{
		m_bFullScreen=FALSE;
	}
	if(FAILED(hr = CreateDevice(hwnd)) || FAILED(hr = AllocSurfaces(width, height))) {
		return false;
	}
	return true;
}
HRESULT CDirect3DRenderStereo::CreateDirect3D()
{

	m_pD3D = NULL;


	m_pD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
	if (!m_pD3D) {
		m_pD3D.Attach(Direct3DCreate9(D3D9b_SDK_VERSION));
	}
	if (!m_pD3D) {
		DTRACE(VSTM_VRND, "don't create Direct3D9");
		return D3DERR_NOTAVAILABLE;
	}


	return D3D_OK;
}

HRESULT CDirect3DRenderStereo::CreateDevice(HWND hwnd)
{
	HRESULT hr;
	DWORD dwVertexProcessing;
	DeleteSurfaces();

    m_pD3DDev = NULL;
	if(m_ServiceHWND){
		DestroyWindow(m_ServiceHWND);
		m_ServiceHWND=0;

	}

	if ((hr = CreateDirect3D()) != D3D_OK) {
		return hr;
	}

    UINT uAdapter = GetAdapter(m_pD3D, hwnd);

	ZeroMemory(&m_d3ddm, sizeof(m_d3ddm));
	if (FAILED(m_pD3D->GetAdapterDisplayMode(uAdapter, &m_d3ddm))) {
		DTRACE(VSTM_VRND, "failed GetAdapterDisplayMode");
		return E_UNEXPECTED;
	}

	ZeroMemory(&m_caps, sizeof(m_caps));
	m_pD3D->GetDeviceCaps(uAdapter, D3DDEVTYPE_HAL, &m_caps);
    if ((m_caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
        dwVertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
    } else {
        dwVertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }


    ZeroMemory(&m_pp, sizeof(m_pp));
	if(m_bFullScreen){

	  m_pp.Windowed = FALSE;

	  m_ServiceHWND = CreateWindowExA(NULL,
                          "VSnVidiaRender",    // name of the window class
                          "",   // title of the window
                         /* WS_EX_TOPMOST | WS_POPUP*/0,    // fullscreen values
                          0,    // x-position of the window
                          0,    // y-position of the window
                          0,//0,//m_d3ddm.Width,    // width of the window
                          00,//0,//m_d3ddm.Height,    // height of the window
                          NULL,    // we have no parent window, NULL
                          NULL,    // we aren't using menus, NULL
                          GetModuleHandle(0),    // application handle
                          NULL);    // used with multiple windows, NULL


    // display the window on the screen
    ShowWindow(m_ServiceHWND, SW_SHOW);
	m_pp.hDeviceWindow = m_ServiceHWND;
	m_pp.BackBufferFormat = D3DFMT_A8R8G8B8;  // set the back buffer format to 32 bit
	m_pp.BackBufferWidth = m_d3ddm.Width;
    m_pp.BackBufferHeight = m_d3ddm.Height;
    m_pp.PresentationInterval = D3DPRESENT_RATE_DEFAULT;//D3DPRESENT_INTERVAL_ONE;
    m_pp.BackBufferCount = 1;
	m_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;
	hwnd=m_ServiceHWND;
	SetWindowLongA(m_ServiceHWND,GWL_USERDATA,(LONG)this);
	}
	else{

		m_pp.Windowed = TRUE;
		m_pp.hDeviceWindow = hwnd;
		RECT WinRect;
		GetClientRect(hwnd,&WinRect);
		//m_BuffWidth=WinRect.right-WinRect.left;
		//m_BuffHeight=WinRect.bottom-WinRect.top;
		m_pp.BackBufferWidth = WinRect.right-WinRect.left;
		m_pp.BackBufferHeight = WinRect.bottom-WinRect.top;
		m_pp.BackBufferFormat = D3DFMT_A8R8G8B8;  // set the back buffer format to 32 bit
	//	m_pp.BackBufferWidth = m_width;
//		m_pp.BackBufferHeight = m_height/2;
		m_pp.PresentationInterval = D3DPRESENT_RATE_DEFAULT;//D3DPRESENT_INTERVAL_ONE;
		m_pp.BackBufferCount = 1;
		m_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;
	}

	m_pp.Flags = D3DPRESENTFLAG_VIDEO;
	m_pp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	//m_BackbufferType = D3DFMT_X8R8G8B8/*d3ddm.Format*/;
	//m_DisplayType = m_d3ddm.Format;
 /*  if(m_pD3DDev)
	   m_pD3DDev->Reset(&m_pp);
   else*/
	hr = m_pD3D->CreateDevice(uAdapter, D3DDEVTYPE_HAL, hwnd,
								  dwVertexProcessing| D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE, //D3DCREATE_MANAGED
								  &m_pp, &m_pD3DDev);




	if (FAILED(hr)) {
		DTRACE(VSTM_VRND, "failed create Direct3D Device");
		return hr;
	}



    ZeroMemory(&m_caps, sizeof(m_caps));
	hr = m_pD3DDev->GetDeviceCaps(&m_caps);

	double p=m_width*1.0/(m_height/2.0);
	double step=std::min<double>(m_pp.BackBufferWidth*1.0/p, m_pp.BackBufferHeight);
	m_BuffWidth = step*p;
	m_BuffHeight =step;

	return S_OK;
}
LRESULT CALLBACK CDirect3DRenderStereo::WindowProc2(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
    CDirect3DRenderStereo *pVideoRenderBase=(CDirect3DRenderStereo*)GetWindowLongPtr(hwnd, GWL_USERDATA);
	//DWORD currTime;
	if (pVideoRenderBase!=NULL)
	switch(uMsg)
	{
	  case WM_KEYDOWN:
      case WM_KEYUP:
	  case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_LBUTTONDBLCLK:
		  SendMessage(pVideoRenderBase->m_hwnd,uMsg,wParam,lParam);

	}
	return	DefWindowProc( hwnd,uMsg,wParam,lParam);
}
HRESULT CDirect3DRenderStereo::AllocSurfaces(int width, int height, D3DFORMAT Format)
{
	HRESULT hr;
	int i = 0, j = 0;

	DeleteSurfaces();

//	if(m_bFullScreen){
		if (FAILED(hr = m_pD3DDev->CreateOffscreenPlainSurface(width*2,height/2, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pBackBuffer, NULL)))
		{
			return hr;
		}
		if (FAILED(hr = m_pD3DDev->CreateRenderTarget(m_BuffWidth*2,m_BuffHeight+1, D3DFMT_A8R8G8B8,D3DMULTISAMPLE_NONE,0,TRUE, &m_pBackBufferScaled, NULL)))
		{
			return hr;
		}
		// Lock the stereo image
        D3DLOCKED_RECT lr;
        m_pBackBufferScaled->LockRect(&lr,NULL,0);
        // write stereo signature in the last raw of the stereo image
        LPNVSTEREOIMAGEHEADER pSIH =
        (LPNVSTEREOIMAGEHEADER)(((unsigned char *) lr.pBits) + (lr.Pitch * (m_BuffHeight)));

        // Update the signature header values
        pSIH->dwSignature = NVSTEREO_IMAGE_SIGNATURE;
        pSIH->dwBPP = 32;
		pSIH->dwFlags=0;
        //pSIH->dwFlags = SIH_SWAP_EYES; // Src image has left on left and right on right, thats why this flag is not needed.
        pSIH->dwWidth = m_BuffWidth;//*2;
        pSIH->dwHeight = m_BuffHeight;
		if(m_pBufferConvOut){
			free(m_pBufferConvOut);
			m_pBufferConvOut=NULL;
		}

		m_pBufferConvOut=(BYTE*)malloc(width*height*4);

        // Unlock surface
        m_pBackBufferScaled->UnlockRect();
/*	}
	else{



	}*/
	return hr;
}
void CDirect3DRenderStereo::DeleteSurfaces()
{
	m_pBackBuffer = NULL;
	m_pBackBufferScaled = NULL;
	if(m_pBufferConvOut){
			free(m_pBufferConvOut);
			m_pBufferConvOut=NULL;
		}

}
int CDirect3DRenderStereo::DrawFrame(HWND hwnd)
{
	VS_AutoLock lock(this);

	if (!IsInited())
		return 1;

	if (!m_IsValid) { // try to repair if render cannot draw
		DWORD currTime = timeGetTime();
		if (currTime - m_repairTime >= VR_REPAIR_TIME) {
			m_repairTime = currTime;
			SetMode(VRM_DEFAULT, hwnd);
			return 1;
		}
		if (!m_IsValid)
			return 1;
	}

	if (hwnd!=m_hwnd || m_bForceBicubicChange!=m_bForceBicubic) {
		m_RenderMode = VRM_UNDEF;
		m_bForceBicubicChange = m_bForceBicubic;
		if (SetMode(VRM_DEFAULT, hwnd))
			return 1;
	}

	HRESULT hRet;

	if (!IsWindowVisible(m_hwnd)) return 0;
	if (PrepareFrame(hwnd))	{
		m_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);
		if (FAILED(hRet = m_pD3DDev->BeginScene())) return 1;

		CComPtr<IDirect3DSurface9> pBackBuffer;
		hRet |= m_pD3DDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);


		D3DLOCKED_RECT lrDst;
		m_pBackBuffer->LockRect(&lrDst,NULL,0);

		BYTE *Dst1=(BYTE*)lrDst.pBits;
		BYTE *Src1=(BYTE*)m_pBufferConvOut;
		BYTE *Dst2=Dst1+m_width*4;
		BYTE *Src2=m_pBufferConvOut+m_width*4*m_height/2;
		for(int i=0;i<m_height/2;i++){
			memcpy(Dst1,Src1,m_width*4);
			memcpy(Dst2,Src2,m_width*4);
			Dst1+=lrDst.Pitch;
			Src1+=m_width*4;
			Dst2+=lrDst.Pitch;
			Src2+=m_width*4;
		}
		//m_pVideoSurface->UnlockRect();
		m_pBackBuffer->UnlockRect();
		RECT destRect;
		destRect.top=0;
		destRect.left=0;
		destRect.bottom=m_BuffHeight;
		destRect.right=m_BuffWidth*2;
		RECT srcRect;
		srcRect.top=0;
		srcRect.left=0;
		srcRect.bottom=m_height/2;
		srcRect.right=m_width*2;
		hRet=m_pD3DDev->StretchRect(m_pBackBuffer,&srcRect,m_pBackBufferScaled,&destRect,D3DTEXF_LINEAR);
		//if(hRet==D3DERR_INVALIDCALL)
        destRect.top=0;
		destRect.left=0;
		destRect.bottom=m_BuffHeight;
		destRect.right=m_BuffWidth;
		m_pD3DDev->StretchRect(m_pBackBufferScaled,NULL,pBackBuffer,NULL,D3DTEXF_NONE);


		hRet |= m_pD3DDev->EndScene();

		hRet |= m_pD3DDev->Present(NULL, NULL, NULL, NULL);
	}

	return hRet != D3D_OK;
}
CDirect3DRenderStereo2::CDirect3DRenderStereo2(CVSInterface* pParentInterface):CDirect3DRenderStereo(pParentInterface){
};
CDirect3DRenderStereo2::~CDirect3DRenderStereo2(){
};
#ifdef _USE_OPENGL

#pragma         comment (lib,"opengl32.lib")
COpenGLRender::COpenGLRender(CVSInterface* pParentInterface)
:CDIBRender(pParentInterface)
{

///////////////////////////////////
	m_TypeRender = VR_OPENGL;
    m_texture=0;
	m_drawList=0;
///////////////////////////////////
	m_intBuffer=NULL;
	m_hEvDraw = CreateEvent(NULL, FALSE, FALSE, NULL);
	ActivateThread(this);

};
COpenGLRender::~COpenGLRender(){

	//DisableOpenGL(m_hwnd,m_hDCGL,m_hRC);
	DesactivateThread();
	CloseHandle(m_hEvDraw);
};
/*bool COpenGLRender::PrepareFrame(HWND m_hwnd){
};*/
int	COpenGLRender::DrawFrame(HWND hwnd){
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

	int ret = 0;

	if (PrepareFrame(hwnd))	{
		  SetEvent(m_hEvDraw);

		}
	UnLock();
	return ret;
};


void COpenGLRender::EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    // get the device context (DC)
    *hDC = GetDC( hWnd );

    // set the pixel format for the DC
    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                  PFD_DOUBLEBUFFER| PFD_STEREO ;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat( *hDC, &pfd );
    SetPixelFormat( *hDC, iFormat, &pfd );

    // create and enable the render context (RC)
    *hRC = wglCreateContext( *hDC );
    wglMakeCurrent( *hDC, *hRC );
  //  setVSync();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glShadeModel(GL_FLAT);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	m_drawList = glGenLists(1);
	glNewList(m_drawList, GL_COMPILE);

	glDrawBuffer (GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // non-black bkground, less ghosting
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glDrawBuffer(GL_BACK_LEFT);

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(1.0f, 0.5f); glVertex2f( 1.0f, -1.0f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(0.0f, 0.5f); glVertex2f(-1.0f, -1.0f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);	// Top Left Of The Texture and Quad
	glEnd();
	//glClear(GL_DEPTH_BUFFER_BIT);
	//glFlush();

	// Draw right eye view to the Right Back Buffer
	glDrawBuffer(GL_BACK_RIGHT);


	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);	// Bottom Right Of The Texture and Quad
	glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);	// Bottom Left Of The Texture and Quad
	glTexCoord2f(1.0f, 0.5f); glVertex2f( 1.0f,  1.0f);	// Top Right Of The Texture and Quad
	glTexCoord2f(0.0f, 0.5f); glVertex2f(-1.0f,  1.0f);	// Top Left Of The Texture and Quad
	glEnd();
	glFlush();

	glEndList();
	if (m_height / 2 <= 512)
		th = 512 * 2;
	else if (m_height / 2 <= 1024)
		th = 1024 * 2;
	else if (m_height / 2 <= 2048)
		th = 2048 * 2;
	else if (m_height / 2 <= 4096)
		th = 4096 * 2;
	else
		th = 8192 * 2;


	if (m_width <= 512)
		tw = 512;
	else if (m_width <= 1024)
		tw = 1024;
	else if (m_width <= 2048)
		tw = 2048;
	else if (m_width <= 4096)
		tw = 4096;
	else
		tw = 8192;

	m_intBuffer=malloc(tw * th*4);


};
void COpenGLRender::DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &m_texture);
	glDeleteLists(m_drawList, 1);
	free(m_intBuffer);

    wglMakeCurrent( NULL, NULL );
    wglDeleteContext( hRC );
    ReleaseDC( hWnd, hDC );
}
void COpenGLRender::PrepareData(HWND hwnd){
	for(int i=0;i<m_height/2;i++)
		memcpy((unsigned char *)m_intBuffer + i * tw * 4, m_pBufferConvOut + (i) * m_width * 4, m_width * 4);
	for(int i=0;i<m_height/2;i++)
		memcpy((unsigned char *)m_intBuffer + (i+th/2) * tw * 4, m_pBufferConvOut + ( i+m_height/2) * m_width * 4, m_width * 4);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA/*m_internalFormat*/, tw, th, 0, GL_BGRA_EXT , GL_UNSIGNED_BYTE, m_intBuffer);


};

DWORD COpenGLRender::Loop(LPVOID hEvDie){
	HANDLE handles[2];
	handles[0] = hEvDie;
	handles[1] = m_hEvDraw;
	HWND hwnd=0;
	RECT r={0,0,0,0};
	HDC m_hDCGL=0;
	HGLRC m_hRC=0;
	while(true) {
		DWORD dwret = WaitForMultipleObjects(2, handles, FALSE, 500);
		if		(dwret == WAIT_OBJECT_0+0)
			break;
		else if (dwret == WAIT_OBJECT_0+1){
			VS_AutoLock lock(this);
			if(hwnd!=m_hwnd){
			 if(m_hDCGL){
                DisableOpenGL(hwnd,m_hDCGL,m_hRC);
				m_hDCGL=0;
				m_hRC=0;
				}
              hwnd=m_hwnd;
			  if(hwnd)
			    EnableOpenGL(hwnd,&m_hDCGL,&m_hRC);

			}
			if(hwnd){
				RECT r_;
				GetClientRect(hwnd, &r_);
				if(!EqualRect(&r_, &r)){
				//	DisableOpenGL(hwnd,m_hDCGL,m_hRC);
                //    EnableOpenGL(hwnd,&m_hDCGL,&m_hRC);

					CopyRect(&r,&r_);
					double kx,ky;
					if((double(r.right-r.left)/double(m_width))<(double(r.bottom-r.top)*2.0/double(m_height))){
						ky=(double(r.right-r.left)/double(m_width))/(double(r.bottom-r.top)*2.0/double(m_height));
						kx=1.0;
					}
					else{
						ky=1.0;
						kx=(double(r.bottom-r.top)*2.0/double(m_height))/(double(r.right-r.left)/double(m_width));
					}
					glViewport((r.right-r.left)*(1.0-kx)/2.0, (r.bottom-r.top)*(1.0-ky)/2.0, kx*(r.right-r.left)*tw/m_width, ky*(r.bottom-r.top)*th/m_height);
				}
				PrepareData(hwnd);
				glCallList(m_drawList);
				SwapBuffers( m_hDCGL );
			}


		}
			//while (m_capture->Capture(bufffer, len)>0);
	}
	if(m_hDCGL){
		DisableOpenGL(hwnd,m_hDCGL,m_hRC);
	}

	return 1;
};
const char COpenGLRenderInterlaced::strP[]="uniform sampler2D Texture0;uniform sampler2D Texture1;uniform int delta;varying vec2 Texcoord;void main(void){ if(((int(gl_FragCoord.y)+delta) & 1)==1)gl_FragColor =texture2D(Texture0, Texcoord);else gl_FragColor =texture2D(Texture1, Texcoord);}";
const char COpenGLRenderInterlaced::strV[]="varying vec2 Texcoord;void main(void){gl_Position = ftransform();Texcoord = vec2(gl_MultiTexCoord0);}";
COpenGLRenderInterlaced::COpenGLRenderInterlaced(CVSInterface* pParentInterface)
:COpenGLRender(pParentInterface){
	_variant_t vr(0);
	m_iSwap = 0;
	ReadParam("StereoSwap", &vr.GetVARIANT());
	if (int(vr)!=0)
		m_iSwap = 1;
	m_intBuffer2=0;
	m_texture2=0;
	program=0;
	shaderP=0;
	shaderV=0;
	m_TypeRender = VR_OPENGL_INTERLACE;
	shad_strP=(char*)strP;//"uniform sampler2D Texture0;uniform sampler2D Texture1;uniform int delta;varying vec2 Texcoord;void main(void){ if(((int(gl_FragCoord.y)+delta) & 1)==1)gl_FragColor =texture2D(Texture0, Texcoord);else gl_FragColor =texture2D(Texture1, Texcoord);}";
			//int lens_shad_str[1];
			//lens_shad_str[0]=strlen(shad_str[0]);
	shad_strV=(char*)strV;//"varying vec2 Texcoord;void main(void){gl_Position = ftransform();Texcoord = vec2(gl_MultiTexCoord0);}";
};

const char COpenGLRenderAnaglyph::strP[]="uniform sampler2D Texture0;uniform sampler2D Texture1;uniform int delta;varying vec2 Texcoord;void main(void){float rL,gL,bL,rR,gR,bR;rL=texture2D(Texture1, Texcoord).r;gL=texture2D(Texture1, Texcoord).g;  bL=texture2D(Texture1, Texcoord).b;rR=texture2D(Texture0, Texcoord).r;gR=texture2D(Texture0, Texcoord).g;bR=texture2D(Texture0, Texcoord).b;gl_FragColor= vec4(0.4154*rL+0.4710*gL+0.1669*bL-0.0109*rR-0.0364*gR-0.0060*bR,-0.0458*rL-0.0484*gL-0.0257*bL+0.3756*rR+0.7333*gR+0.0111*bR,-0.0547*rL-0.0615*gL+0.0128*bL-0.0651*rR-0.1287*gR+1.2971*bR,1.0 );}";
const char COpenGLRenderAnaglyph::strV[]="varying vec2 Texcoord;void main(void){gl_Position = ftransform();Texcoord = vec2(gl_MultiTexCoord0);}";
COpenGLRenderAnaglyph::COpenGLRenderAnaglyph(CVSInterface* pParentInterface)
:COpenGLRenderInterlaced(pParentInterface)
{
	shad_strP=(char*)strP;
	shad_strV=(char*)strV;//
};
const char COpenGLRenderActive::strP[] = "";// "uniform sampler2D Texture0;uniform sampler2D Texture1;uniform int delta;varying vec2 Texcoord;void main(void){ if(/*((int(gl_FragCoord.y)+delta) & 1)==1*/gl_PointCoord​.y>0.5)gl_FragColor =texture2D(Texture0, vec2(Texcoord.x,Texcoord.y*2.0));else gl_FragColor =vec4(0.0,0.0,1.0,1.0);/*texture2D(Texture1, vec2(Texcoord.x,Texcoord.y*2.0-1.0));*/}";
const char COpenGLRenderActive::strV[] = "";// "varying vec2 Texcoord;void main(void){gl_Position = ftransform();Texcoord = vec2(gl_MultiTexCoord0);}";
COpenGLRenderActive::COpenGLRenderActive(CVSInterface* pParentInterface)
	:COpenGLRenderInterlaced(pParentInterface)
{
	shad_strP = (char*)strP;
	shad_strV = (char*)strV;//
};
void COpenGLRenderActive::EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int iFormat;

	// get the device context (DC)
	*hDC = GetDC(hWnd);

	// set the pixel format for the DC
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	iFormat = ChoosePixelFormat(*hDC, &pfd);
	SetPixelFormat(*hDC, iFormat, &pfd);

	// create and enable the render context (RC)
	*hRC = wglCreateContext(*hDC);
	wglMakeCurrent(*hDC, *hRC);
	if (glewInit() == GLEW_OK){
		if (GLEW_VERSION_2_0){
			/*

			*/
			// создаем шейдер
			/*shaderP = glCreateShader(GL_FRAGMENT_SHADER);
			shaderV = glCreateShader(GL_VERTEX_SHADER);
			// устанавливаем источник исходного кода шейдера
			int num_shad_str = 1;
			//char shad_str[1][MAX_PATH];

			glShaderSource(shaderP, 1, (const GLchar **)&shad_strP, NULL);
			glShaderSource(shaderV, 1, (const GLchar **)&shad_strV, NULL);
			// компилируем шейдер
			glCompileShader(shaderP);
			glCompileShader(shaderV);
			// создаём программный модуль шейдера
			program = glCreateProgram();
			// присоединяем шейдер к модулю
			glAttachShader(program, shaderP);
			glAttachShader(program, shaderV);
			// прилинковываем модуль
			glLinkProgram(program);

			// используем полученную шейдерную программку
			glUseProgram(program);
			GLuint loc = glGetUniformLocation(program, "Texture0");
			glUniform1i(loc, 0);

			//для второй

			loc = glGetUniformLocation(program, "Texture1");
			glUniform1i(loc, 1);*/
		}
		else{
		}
	}

	//  setVSync();
	if (m_height / 2 <= 512)
		th = 512 * 2;
	else if (m_height / 2 <= 1024)
		th = 1024 * 2;
	else if (m_height / 2 <= 2048)
		th = 2048 * 2;
	else if (m_height / 2 <= 4096)
		th = 4096 * 2;
	else
		th = 8192 * 2;


	if (m_width <= 512)
		tw = 512;
	else if (m_width <= 1024)
		tw = 1024;
	else if (m_width <= 2048)
		tw = 2048;
	else if (m_width <= 4096)
		tw = 4096;
	else
		tw = 8192;
	m_intBuffer = malloc(tw * th / 2 * 4);
	m_intBuffer2 = malloc(tw * th / 2 * 4);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glShadeModel(GL_FLAT);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &m_texture2);
	glBindTexture(GL_TEXTURE_2D, m_texture2);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	m_drawList = glGenLists(1);
	glNewList(m_drawList, GL_COMPILE);

	glDrawBuffer(GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // non-black bkground, less ghosting
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_STRIP);

	glTexCoord2f( 1.0f, 0.0f);
	glVertex2f(1.0f, -1.0f);	// Bottom Right Of The Texture and Quad

	glTexCoord2f( 0.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);	// Bottom Left Of The Texture and Quad

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 0.0f);	// Top Right Of The Texture and Quad

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, 0.0f);	// Top Left Of The Texture and Quad

	glEnd();
	glActiveTexture( GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture2);
	glEnable(GL_TEXTURE_2D);
	//glDrawBuffer(GL_BACK_LEFT);
	glBegin(GL_TRIANGLE_STRIP);
	float k = (1.0f*m_height) / (1.0f*th);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, -(1.0f-k));

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-1.0f, -(1.0f - k));	// Bottom Left Of The Texture and Quad

	glTexCoord2f( 1.0f, 1.0f);
	glVertex2f(1.0f, k);	// Top Right Of The Texture and Quad

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, k);	// Top Left Of The Texture and Quad

	glEnd();



	//////////////




	glFlush();

	glEndList();



};

void COpenGLRenderActive::DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC){
	if (GLEW_VERSION_2_0)
	{

		/*glDetachShader(program, shaderV);
		glDetachShader(program, shaderP);
		glDeleteShader(shaderV);
		glDeleteShader(shaderP);
		glDeleteProgram(program);*/
	}
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &m_texture);
	glDeleteTextures(1, &m_texture2);
	glDeleteLists(m_drawList, 1);
	free(m_intBuffer);
	free(m_intBuffer2);

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);
};
/*const char COpenGLRenderInterlaced::strP[]="
uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform int delta;
varying vec2 Texcoord;
void main(void){
if(((int(gl_FragCoord.y)+delta) & 1)==1)
  gl_FragColor =texture2D(Texture0, Texcoord);
else
  gl_FragColor =texture2D(Texture1, Texcoord);
 }";
const char COpenGLRenderInterlaced::strV[]="
varying vec2 Texcoord;
void main(void){
gl_Position = ftransform();
Texcoord = vec2(gl_MultiTexCoord0);
}
";*/
void COpenGLRenderInterlaced::EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    // get the device context (DC)
    *hDC = GetDC( hWnd );

    // set the pixel format for the DC
    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                  PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat( *hDC, &pfd );
    SetPixelFormat( *hDC, iFormat, &pfd );

    // create and enable the render context (RC)
    *hRC = wglCreateContext( *hDC );
	wglMakeCurrent( *hDC, *hRC );
	if(glewInit()==GLEW_OK){
		if(GLEW_VERSION_2_0){
/*

*/
			// создаем шейдер
			shaderP = glCreateShader(GL_FRAGMENT_SHADER );
			shaderV = glCreateShader(GL_VERTEX_SHADER );
			// устанавливаем источник исходного кода шейдера
			int num_shad_str=1;
			//char shad_str[1][MAX_PATH];

			glShaderSource(shaderP, 1,(const GLchar **) &shad_strP, NULL );
			glShaderSource( shaderV, 1,(const GLchar **) &shad_strV, NULL );
			// компилируем шейдер
			glCompileShader( shaderP );
			glCompileShader( shaderV );
			// создаём программный модуль шейдера
			program = glCreateProgram();
			// присоединяем шейдер к модулю
			glAttachShader( program, shaderP );
			glAttachShader( program, shaderV );
			// прилинковываем модуль
			glLinkProgram( program );

			// используем полученную шейдерную программку
			glUseProgram( program );
			GLuint loc = glGetUniformLocation(program,"Texture0");
            glUniform1i(loc, 0);

//для второй

             loc = glGetUniformLocation(program,"Texture1");
             glUniform1i(loc, 1);
		}
		else{
		}
	}

  //  setVSync();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glShadeModel(GL_FLAT);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &m_texture2);
	glBindTexture(GL_TEXTURE_2D, m_texture2);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	m_drawList = glGenLists(1);
	glNewList(m_drawList, GL_COMPILE);

	glDrawBuffer (GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // non-black bkground, less ghosting
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//glDrawBuffer(GL_BACK_LEFT);

	glBegin(GL_TRIANGLE_STRIP);
	glMultiTexCoord2f(GL_TEXTURE0_ARB,1.0f, 0.0f);
	glMultiTexCoord2f(GL_TEXTURE1_ARB,1.0f, 0.0f);
	glVertex2f( 1.0f, -1.0f);	// Bottom Right Of The Texture and Quad

    glMultiTexCoord2f(GL_TEXTURE0_ARB,0.0f, 0.0f);
	glMultiTexCoord2f(GL_TEXTURE1_ARB,0.0f, 0.0f);

	glVertex2f(-1.0f, -1.0f);	// Bottom Left Of The Texture and Quad

	glMultiTexCoord2f(GL_TEXTURE0_ARB,1.0f, 1.0f);
	glMultiTexCoord2f(GL_TEXTURE1_ARB,1.0f, 1.0f);

	glVertex2f( 1.0f,  1.0f);	// Top Right Of The Texture and Quad
	glMultiTexCoord2f(GL_TEXTURE0_ARB,0.0f, 1.0f);
	glMultiTexCoord2f(GL_TEXTURE1_ARB,0.0f, 1.0f);

	glVertex2f(-1.0f,  1.0f);	// Top Left Of The Texture and Quad
	glEnd();


	glFlush();

	glEndList();
	if(m_height/2<=512)
	  th=512*2;
    else if(m_height/2<=1024)
	  th=1024*2;
	else if (m_height/2<=2048)
	  th=2048*2;
	else if (m_height / 2 <= 4096)
		th = 4096 * 2;
	else
		th = 8192 * 2;


	if(m_width<=512)
	  tw=512;
    else if(m_width<=1024)
	  tw=1024;
	else if (m_width<=2048)
	  tw=2048;
	else if (m_width <= 4096)
		tw = 4096;
	else
		tw = 8192;

	m_intBuffer=malloc(tw * th/2*4);
	m_intBuffer2=malloc(tw * th/2*4);


};
void COpenGLRenderInterlaced::PrepareData(HWND hwnd){

	for(int i=0;i<m_height/2;i++)
		memcpy((unsigned char *)m_intBuffer + i * tw * 4, m_pBufferConvOut + (i) * m_width * 4, m_width * 4);
		//memset((unsigned char *)m_intBuffer + i * tw * 4, 0x80, m_width * 4);
	for(int i=0;i<m_height/2;i++)
		memcpy((unsigned char *)m_intBuffer2 + (i) * tw * 4, m_pBufferConvOut + ( i+m_height/2) * m_width * 4, m_width * 4);
		//memset((unsigned char *)m_intBuffer2 + (i)* tw * 4, 0xFF, m_width * 4);
	glActiveTexture ( GL_TEXTURE0 );
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA/*m_internalFormat*/, tw, th/2, 0, GL_BGRA_EXT , GL_UNSIGNED_BYTE, m_intBuffer);
	glActiveTexture ( GL_TEXTURE1 );
	glBindTexture(GL_TEXTURE_2D, m_texture2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA/*m_internalFormat*/, tw, th/2, 0, GL_BGRA_EXT , GL_UNSIGNED_BYTE, m_intBuffer2);
	POINT pt;
	pt.x=0;
	pt.y=0;
	ClientToScreen(hwnd,&pt);
	GLuint loc = glGetUniformLocation(program,"delta");
	glUniform1i(loc, pt.y + m_iSwap);

};
void COpenGLRenderInterlaced::DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC){
if (GLEW_VERSION_2_0)
{

    glDetachShader( program, shaderV );
	glDetachShader( program, shaderP );
    glDeleteShader( shaderV );
	glDeleteShader( shaderP );
    glDeleteProgram( program );
}
    glActiveTexture ( GL_TEXTURE0 );
    glDisable(GL_TEXTURE_2D);
	glActiveTexture ( GL_TEXTURE1 );
    glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &m_texture);
	glDeleteTextures(1, &m_texture2);
	glDeleteLists(m_drawList, 1);
	free(m_intBuffer);
	free(m_intBuffer2);

    wglMakeCurrent( NULL, NULL );
    wglDeleteContext( hRC );
    ReleaseDC( hWnd, hDC );
};


const char COpenGLRenderActive2::strP[] = "";// "uniform sampler2D Texture0;uniform sampler2D Texture1;uniform int delta;varying vec2 Texcoord;void main(void){ if(/*((int(gl_FragCoord.y)+delta) & 1)==1*/gl_PointCoord​.y>0.5)gl_FragColor =texture2D(Texture0, vec2(Texcoord.x,Texcoord.y*2.0));else gl_FragColor =vec4(0.0,0.0,1.0,1.0);/*texture2D(Texture1, vec2(Texcoord.x,Texcoord.y*2.0-1.0));*/}";
const char COpenGLRenderActive2::strV[] = "";// "varying vec2 Texcoord;void main(void){gl_Position = ftransform();Texcoord = vec2(gl_MultiTexCoord0);}";
COpenGLRenderActive2::COpenGLRenderActive2(CVSInterface* pParentInterface)
	:COpenGLRenderInterlaced(pParentInterface)
{
	shad_strP = (char*)strP;
	shad_strV = (char*)strV;//
};
void COpenGLRenderActive2::EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
{
	PIXELFORMATDESCRIPTOR pfd;
	int iFormat;

	// get the device context (DC)
	*hDC = GetDC(hWnd);

	// set the pixel format for the DC
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	iFormat = ChoosePixelFormat(*hDC, &pfd);
	SetPixelFormat(*hDC, iFormat, &pfd);

	// create and enable the render context (RC)
	*hRC = wglCreateContext(*hDC);
	wglMakeCurrent(*hDC, *hRC);
	if (glewInit() == GLEW_OK){
		if (GLEW_VERSION_2_0){
			/*

			*/
			// создаем шейдер
			/*shaderP = glCreateShader(GL_FRAGMENT_SHADER);
			shaderV = glCreateShader(GL_VERTEX_SHADER);
			// устанавливаем источник исходного кода шейдера
			int num_shad_str = 1;
			//char shad_str[1][MAX_PATH];

			glShaderSource(shaderP, 1, (const GLchar **)&shad_strP, NULL);
			glShaderSource(shaderV, 1, (const GLchar **)&shad_strV, NULL);
			// компилируем шейдер
			glCompileShader(shaderP);
			glCompileShader(shaderV);
			// создаём программный модуль шейдера
			program = glCreateProgram();
			// присоединяем шейдер к модулю
			glAttachShader(program, shaderP);
			glAttachShader(program, shaderV);
			// прилинковываем модуль
			glLinkProgram(program);

			// используем полученную шейдерную программку
			glUseProgram(program);
			GLuint loc = glGetUniformLocation(program, "Texture0");
			glUniform1i(loc, 0);

			//для второй

			loc = glGetUniformLocation(program, "Texture1");
			glUniform1i(loc, 1);*/
		}
		else{
		}
	}

	//  setVSync();

	if (m_height / 2 <= 512)
		th = 512 * 2;
	else if (m_height / 2 <= 1024)
		th = 1024 * 2;
	else if (m_height / 2 <= 2048)
		th = 2048 * 2;
	else if (m_height / 2 <= 4096)
		th = 4096 * 2;
	else
		th = 8192 * 2;


	if (m_width <= 512)
		tw = 512;
	else if (m_width <= 1024)
		tw = 1024;
	else if (m_width <= 2048)
		tw = 2048;
	else if (m_width <= 4096)
		tw = 4096;
	else
		tw = 8192;

	m_intBuffer = malloc(tw * th / 2 * 4);
	m_intBuffer2 = malloc(tw * th / 2 * 4);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glShadeModel(GL_FLAT);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &m_texture2);
	glBindTexture(GL_TEXTURE_2D, m_texture2);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	m_drawList = glGenLists(1);
	glNewList(m_drawList, GL_COMPILE);

	glDrawBuffer(GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  // non-black bkground, less ghosting
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glEnable(GL_TEXTURE_2D);
	//glDrawBuffer(GL_BACK_LEFT);
	glBegin(GL_TRIANGLE_STRIP);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(0.0f, -1.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);	// Bottom Left Of The Texture and Quad

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(0.0f, 1.0f);	// Top Right Of The Texture and Quad

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(-1.0f, 1.0f);	// Top Left Of The Texture and Quad

	glEnd();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glEnable(GL_TEXTURE_2D);
	float k = (1.0f*m_width) / (1.0f*tw);

	glBegin(GL_TRIANGLE_STRIP);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(k, -1.0f);	// Bottom Right Of The Texture and Quad

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(k-1.0f, -1.0f);	// Bottom Left Of The Texture and Quad

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(k, 1.0f);	// Top Right Of The Texture and Quad

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(k-1.0f, 1.0f);	// Top Left Of The Texture and Quad

	glEnd();




	//////////////




	glFlush();

	glEndList();



};

void COpenGLRenderActive2::DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC){
	if (GLEW_VERSION_2_0)
	{

		/*glDetachShader(program, shaderV);
		glDetachShader(program, shaderP);
		glDeleteShader(shaderV);
		glDeleteShader(shaderP);
		glDeleteProgram(program);*/
	}
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &m_texture);
	glDeleteTextures(1, &m_texture2);
	glDeleteLists(m_drawList, 1);
	free(m_intBuffer);
	free(m_intBuffer2);

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);
};

#endif