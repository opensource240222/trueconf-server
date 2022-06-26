#ifndef QDXRENDER_H
#define QDXRENDER_H

#include "../VSClient/VSRender.h"

#include <QtCore/QObject>

class CVSInterface;

class CQDxRender :	public QObject, public CDirectXRender
{
	Q_OBJECT
private:

public:
	CQDxRender(CVSInterface* pParentInterface, QObject *parent = 0);
	virtual ~CQDxRender();
};

#endif
