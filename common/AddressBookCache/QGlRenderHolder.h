#ifndef QGLRENDERHOLDER_H
#define QGLRENDERHOLDER_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QMutex>

#include "QGlRender.h"
#include "QDxRender.h"

class VS_Render;

template <typename T>
class VSQPeerLocker
{
public:
	VSQPeerLocker(QMutex * mutex, T peer)
	{
		m_mutex = mutex;
		m_mutex->lock();
		m_peer = peer;
	}
	virtual ~VSQPeerLocker()
	{
		m_mutex->unlock();
	}

	T&  operator ->() {return m_peer;}
	T peer() {return m_peer;}

private:
	QMutex * m_mutex;
	T m_peer;
};

class CQRenderHolder : public QObject
{
	Q_OBJECT
private:
	CQRenderHolder();
	virtual ~CQRenderHolder();

	static CQRenderHolder *m_instance;
	bool m_useGlRender;
	typedef QMap<QString, QObject*> RenderContainer;
	RenderContainer m_renders;

	mutable QMutex m_mutex;

public:
	static CQRenderHolder *instance();
	bool useGlRender() const;
	void setUseGlRender(bool enable = true);
	const QStringList getPeerList() const;
	template <typename T>
	VSQPeerLocker<T> getPeer(const QString& peerName) const
	{
		T result = 0;
		m_mutex.lock();

		RenderContainer::const_iterator fnd = m_renders.find(peerName);
		if (fnd != m_renders.end())
		{
			result = qobject_cast<T>(*fnd);
		}

		return VSQPeerLocker<T>(&m_mutex, result);
		//return result;
	};
	void insertPeer(const QString& peerName, CQGlRender *peerRender);
	void insertPeer(const QString& peerName, CQDxRender *peerRender);
	void deletePeer(const QString& peerName);
	void connectPeer(const QString& peerName, VS_Render *render);
	void disconnectPeer(const QString& peerName, VS_Render *render);
	void resetPeerList();

signals:
	void childAdded(const QString &peerName);
	void childRemoved(const QString &peerName);
};

#endif
