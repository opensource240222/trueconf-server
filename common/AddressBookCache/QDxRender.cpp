#include "QDxRender.h"
#include "QDxRender_moc.h"

CQDxRender::CQDxRender(CVSInterface* pParentInterface, QObject *parent)
	: QObject(parent), CDirectXRender(pParentInterface)
{
}

CQDxRender::~CQDxRender()
{
}
