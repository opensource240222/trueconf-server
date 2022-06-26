/**
 **************************************************************************
 * \file VS_Pool.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief MT Pool of objects class
 *
 * \b Project Standart Libraries
 * \author StasS
 * \date 09.12.03
 *
 * $Revision: 1 $
 *
 * $History: VS_Pool.h $
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
 * User: Stass        Date: 1.03.06    Time: 20:05
 * Updated in $/VS/std/cpplib
 * added helper template
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
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
 *
 *
 ****************************************************************************/
#ifndef VS_POOL_H
#define VS_POOL_H


/****************************************************************************
 * Includes
 ****************************************************************************/
#include <memory>
#include <mutex>

#include "std-generic/compat/condition_variable.h"

class VS_Pool
{
public:
	typedef void* Data;

	class Item
	{
	public:
		Data m_data;
		bool m_busy;

		Item(Data data = 0, bool busy = false)
			: m_data(data), m_busy(busy)
		{}
	};

	class Factory
	{
	public:
		virtual bool New(Data&) = 0;
		virtual void Delete(Data data) = 0;
		virtual ~Factory() {}
	};

public:
	int m_error;

	VS_Pool()
		: m_error(0)
	{}
	virtual ~VS_Pool() {}

	virtual const Item* Get() = 0;
	virtual void Release(const Item* item) = 0;
};

class VS_SharedPool : public VS_Pool
{
protected:
	std::unique_ptr<Item[]> m_pool;
	std::unique_ptr<Factory> m_factory;
	size_t m_size;
	const bool m_wait;

	std::mutex m_mutex;
	vs::condition_variable m_cv;

public:
	VS_SharedPool(std::unique_ptr<Factory> factory, size_t size = 16, bool wait = false);
	virtual ~VS_SharedPool();

	const Item* Get() override;
	void Release(const Item* item) override;
	bool Empty() const { return m_size == 0; }
};


class VS_TrivialPool : public VS_Pool
{
public:
	std::unique_ptr<Factory> m_factory;
	Item m_item;

	VS_TrivialPool(std::unique_ptr<Factory> factory)
		: m_factory(std::move(factory))
	{
		m_error = m_factory->New(m_item.m_data) ? 0 : 1;
	}

	virtual ~VS_TrivialPool()
	{
		m_factory->Delete(m_item.m_data);
	}

	const Item* Get() override
	{
		return &m_item;
	}

	void Release(const Item* item) override
	{}
};

template <class DataType>
class VS_PoolItem
{
protected:
	VS_Pool* m_pool;
	const VS_Pool::Item* m_item;

public:
	VS_PoolItem(VS_Pool* pool)
		: m_pool(pool)
	{
		m_item = pool->Get();
	}

	~VS_PoolItem()
	{
		m_pool->Release(m_item);
	}

	DataType* operator->()
	{
		return (DataType*)m_item->m_data;
	}
};


#endif // VS_LICENSE_H
