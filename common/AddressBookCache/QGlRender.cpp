#include "QGlRender.h"
#include "QGlRender_moc.h"

#include "../std/VS_ProfileTools.h"
#include "../AddressBookCache/QGlRenderHolder.h"
#include "VS_Render.h"

CQGlRender::CQGlRender(CVSInterface* pParentInterface)
	: QObject(0), CTransDIBRender(pParentInterface), m_size(640, 360)
{

	m_rPic.left = 0;
	m_rPic.top = 0;
	m_rPic.right = 0;
	m_rPic.bottom = 0;

	if (pParentInterface)
	{
		QString s = QString::fromLatin1(pParentInterface->GetInterfaceName());
		s.remove(QChar('>'));
		setObjectName(s);
		//setParent(CQRenderHolder::instance());
		CQRenderHolder::instance()->insertPeer(s, this);
	}
}

CQGlRender::~CQGlRender()
{
	VS_AutoLock lock(this);
	m_glRenders.clear();
	CQRenderHolder::instance()->deletePeer(objectName());
}

void CQGlRender::connectGLRender(VS_Render* render)
{
	VS_AutoLock lock(this);
	if (!m_glRenders.contains(render))
	{
		m_glRenders += render;
	}
}

void CQGlRender::disconnectGLRender(VS_Render* render)
{
	VS_AutoLock lock(this);
	if (render)
		m_glRenders.removeAll(render);
	else
		m_glRenders.clear();
}

int CQGlRender::DrawFrame(HWND hwnd)
{
	//AUTO_PROF
	VS_AutoLock lock(this);

	if (!m_IsValid) return 1;

	int ret = 0;

	if (PrepareFrame(hwnd))
	{
		m_renderFrameSizeMB = m_size.width() * m_size.height()/256;
		//VS_AutoCountTime("CQGlRender::emit UpdateFrame");
		foreach(VS_Render* render, m_glRenders)
		{
			render->UpdateFrame(m_pBufferConvOut, 12, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight, m_biFmtIn.biCompression == FCC_I42S && m_dwStereoRender > 0 );
		}
		//emit UpdateFrame(m_pBufferConvOut, 12, m_biFmtDraw.biWidth, m_biFmtDraw.biHeight, m_biFmtIn.biCompression == FCC_I42S && m_dwStereoRender > 0 );
	}

	return ret;
}

int CQGlRender::SetMode(eVideoRenderMode Mode, HWND hwnd)
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
	//m_pBufferSaturated = new BYTE[m_width*m_height*3/2];
	//m_dwVRUse|=VR_USE_SATURATE;
	m_pBufferSaturated = m_pBufferConvIn;

	m_biFmtDraw.biWidth = m_width;
	m_biFmtDraw.biHeight = m_height;
	m_pBufferScaled = m_pBufferSaturated;
	m_pBufferConvOut = m_pBufferScaled;

	m_DrawPitch = m_biFmtDraw.biWidth*m_displaydepth/8;
	m_RenderMode = Mode;
	m_bNewFrame = 1;
	m_IsValid = true;
	m_hwnd = hwnd;
	return 0;
}

bool CQGlRender::PrepareFrame(HWND hwnd)
{
	if (m_bNewFrame) {
		if (m_dwVRUse&VR_USE_CONVIN)
			if (!ConvertColorSpaceIn()) return false;
		if (m_dwVRUse & VR_USE_FLIP)
			if (!MirrorVertical()) return false;
		if (!Saturate()) return false;
		//if (!Dither()) return false;
		m_bNewFrame = FALSE;
	}
	return true;
}

void CQGlRender::setViewSize(const QSize &size)
{
	m_size = size;
}

void CQGlRender::UpdateFrame(unsigned char *rawData, unsigned long bits, unsigned long width, unsigned long height, int stereo,
					 unsigned long x, unsigned long y, unsigned long rectWidth, unsigned long rectHeight)
{
	VS_AutoLock lock(this);

	foreach(VS_Render* render, m_glRenders)
	{
		render->UpdateFrame(rawData, bits, width, height, stereo, x, y, rectWidth, rectHeight );
	}
}
