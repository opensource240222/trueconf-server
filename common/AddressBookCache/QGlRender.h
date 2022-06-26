#ifndef QGLRENDER_H
#define QGLRENDER_H

#include "../VSClient/VSRender.h"

#include <QtCore/QObject>
#include <QtCore/QSize>

class CVSInterface;
class VS_Render;

class CQGlRender :	public QObject, public CTransDIBRender
{
	Q_OBJECT
private:
	QSize m_size;
	QList<VS_Render*> m_glRenders;
public:
	CQGlRender(CVSInterface* pParentInterface);
	virtual ~CQGlRender();
	int DrawFrame(HWND hwnd);
	int SetMode(eVideoRenderMode Mode, HWND hwnd);
	bool PrepareFrame(HWND hwnd);
	void setViewSize(const QSize &size);
	void connectGLRender(VS_Render* render);
	void disconnectGLRender(VS_Render* render = 0);

	void UpdateFrame(unsigned char *rawData, unsigned long bits, unsigned long width, unsigned long height, int stereo = 0,
					 unsigned long x =0, unsigned long y = 0, unsigned long rectWidth = 0, unsigned long rectHeight = 0);
};


#endif