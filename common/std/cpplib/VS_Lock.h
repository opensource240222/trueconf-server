/**
 **************************************************************************
 * \file VS_Lock.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief wraper for Critical Section
 *
 * \b Project Standart Libraries
 * \author SMirnovK
 * \date 09.10.03
 *
 * $Revision: 3 $
 *
 * $History: VS_Lock.h $
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 2.09.11    Time: 20:06
 * Updated in $/VSNA/std/cpplib
 * - cases handled
 *
 * *****************  Version 2  *****************
 * User: Dront78      Date: 1.12.09    Time: 17:32
 * Updated in $/VSNA/std/cpplib
 * - virtual added
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 22.05.06   Time: 10:54
 * Updated in $/VS/std/cpplib
 * - comand queue as list
 * - sent frame queue as map
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 3.02.05    Time: 20:22
 * Updated in $/VS/std/cpplib
 * EcHo cancel repaired
 *
 * *****************  Version 2  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 *
 * *****************  Version 1  *****************
 * User: Smirnov      Date: 9.10.03    Time: 19:44
 * Created in $/VS/std/cpplib
 * new methods in client
 * new files in std...
 *
 ****************************************************************************/
#ifndef VS_LOCK_H
#define VS_LOCK_H

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "fast_mutex.h"
#include <boost/noncopyable.hpp>

/**
 **************************************************************************
 * \brief Use it class as base to add lock/unlock methods that use Windows
 * critical section
 ****************************************************************************/
class VS_Lock: public boost::noncopyable
{
	mutable vs::fast_recursive_mutex m;
public:
	VS_Lock() {}
	virtual ~VS_Lock() {}
	void Lock() const { m.lock(); }
	void UnLock() const { m.unlock(); }
	bool TryLock() const { return m.try_lock(); }
};

/**
 **************************************************************************
 * \brief Declare variable of this class in classes methods derived from VS_Lock
 * to auto lock during whole method procession
 ****************************************************************************/
class VS_AutoLock
{
	const VS_Lock* m_p;
public:
	VS_AutoLock(const VS_Lock* p) {m_p = p; m_p->Lock();}
	~VS_AutoLock() {m_p->UnLock();}
};
#define VS_AutoLock(...) static_assert(false, "Attempt to create temporary object of type VS_AutoLock");

template<class T>
class VS_Locked: public VS_Lock
{
	T t;
public:
	T get() const
	{
		T tmp;
		Lock();
		tmp = t;
		UnLock();
		return tmp;
	}
	void set(T in)
	{
		Lock();
		t = in;
		UnLock();
	}
};


template<class T>
class VS_WithLock : public T, public VS_Lock {};

#endif
