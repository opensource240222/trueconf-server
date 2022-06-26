#include "QGlRenderHolder.h"
#include "QGlRenderHolder_moc.h"

#include <QtWidgets/QtWidgets>

CQRenderHolder *CQRenderHolder::m_instance = 0;

CQRenderHolder::CQRenderHolder()
	: QObject(), m_useGlRender(false), m_mutex(QMutex::Recursive)
{
	//m_peerList.clear();
	moveToThread(QApplication::instance()->thread());
}

CQRenderHolder::~CQRenderHolder()
{
}

CQRenderHolder *CQRenderHolder::instance()
{
	if (!CQRenderHolder::m_instance) CQRenderHolder::m_instance = new CQRenderHolder;
	return m_instance;
}

bool CQRenderHolder::useGlRender() const
{
	return m_useGlRender;
}

void CQRenderHolder::setUseGlRender(bool enable)
{
	m_useGlRender = enable;
}

const QStringList CQRenderHolder::getPeerList() const
{
	QMutexLocker lock(&m_mutex);
	return m_renders.keys();
}

void CQRenderHolder::insertPeer(const QString& peerName, CQGlRender *peerRender)
{
	if (!peerRender || peerName.isEmpty()) return;

	QMutexLocker lock(&m_mutex);
	m_renders.remove(peerName);

	m_renders.insert(peerName, peerRender);

	emit childAdded(peerName);
}

void CQRenderHolder::insertPeer(const QString& peerName, CQDxRender *peerRender)
{
	if (!peerRender || peerName.isEmpty()) return;

	QMutexLocker lock(&m_mutex);
	m_renders.remove(peerName);

	m_renders.insert(peerName, peerRender);

	emit childAdded(peerName);
}

void CQRenderHolder::deletePeer(const QString& peerName)
{
	QMutexLocker lock(&m_mutex);

	m_renders.remove(peerName);
}

void CQRenderHolder::connectPeer(const QString& peerName, VS_Render *render)
{
	QMutexLocker lock(&m_mutex);
	RenderContainer::iterator fnd = m_renders.find(peerName);
	if (fnd != m_renders.end())
	{
		if (CQGlRender * glr = qobject_cast<CQGlRender *>(*fnd))
			glr->connectGLRender(render);
	}
}

void CQRenderHolder::disconnectPeer(const QString& peerName, VS_Render *render)
{
	QMutexLocker lock(&m_mutex);
	RenderContainer::iterator fnd = m_renders.find(peerName);
	if (fnd != m_renders.end())
	{
		if (CQGlRender * glr = qobject_cast<CQGlRender *>(*fnd))
			glr->disconnectGLRender(render);
		else
			(*fnd)->disconnect();
	}
}

void CQRenderHolder::resetPeerList()
{
	QMutexLocker lock(&m_mutex);
	for(RenderContainer::iterator i=m_renders.begin();i!=m_renders.end();++i)
	{
		(*i)->disconnect();
		//(*i)->deleteLater();
	}
	m_renders.clear();
}
