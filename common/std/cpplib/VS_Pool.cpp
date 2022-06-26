/**
 **************************************************************************
 * \file VS_Pool.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief MT Pool of objects class implementation
 *
 * \b Project Standart Libraries
 * \author StasS
 * \date 09.12.03
 *
 * $Revision: 1 $
 *
 * $History: VS_Pool.cpp $
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
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 20.12.03   Time: 14:21
 * Updated in $/VS/std/cpplib
 * stopped creation after first item failure
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 10.12.03   Time: 20:34
 * Updated in $/VS/std/cpplib
 * pool incapsulated in DBStorage as DB Object pool
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 9.12.03    Time: 20:16
 * Created in $/VS/std/cpplib
 * added storage pool implementation
 ****************************************************************************/


/****************************************************************************
 * Includes
 ****************************************************************************/
#include "VS_Pool.h"

#ifndef dprint3
#include <stdio.h>
#define dprint3					printf
#endif

VS_SharedPool::VS_SharedPool(std::unique_ptr<Factory> factory, size_t size, bool wait)
	: m_factory(std::move(factory)), m_size(size), m_wait(wait)
{
	m_pool.reset(new Item[m_size]);
	for (size_t i = 0; i < m_size; ++i)
	{
		if (!m_factory->New(m_pool[i].m_data))
		{
			m_error++;
			m_pool[i].m_data = nullptr;
			m_size = i;
			break;
		}
	}
}

VS_SharedPool::~VS_SharedPool()
{
	for (size_t i = 0; i < m_size; ++i)
	{
		if (m_pool[i].m_data)
			m_factory->Delete(m_pool[i].m_data);
		m_pool[i].m_data = 0;
	}
}

const VS_SharedPool::Item* VS_SharedPool::Get()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	while (true)
	{
		Item* result = nullptr;
		for (size_t i = 0; i < m_size; ++i)
			if (!m_pool[i].m_busy)
			{
				result = &m_pool[i];
				break;
			}

		if (result)
		{
			result->m_busy = true;
			return result;
		}
		else
		{
			if (!m_wait)
				return nullptr;
			if (m_cv.wait_for(lock, std::chrono::milliseconds(500)) == std::cv_status::timeout)
			{
				dprint3("POOL:wait timeout!\n");
				return nullptr;
			}
		}
	}
}

void VS_SharedPool::Release(const Item* item)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!item->m_busy)
			return;
		const_cast<Item*>(item)->m_busy = false;
	}
	if (m_wait)
		m_cv.notify_one();
}