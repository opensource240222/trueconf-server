#pragma once

#include "std-generic/compat/memory.h"

//namespace std {

class enable_shared_from_this_virtual_base : public vs::enable_shared_from_this<enable_shared_from_this_virtual_base>
{
protected:
	enable_shared_from_this_virtual_base()
	{
	}
};

template<class T>
class enable_shared_from_this_virtual : virtual public enable_shared_from_this_virtual_base
{
protected:
	enable_shared_from_this_virtual()
	{
	}

public:
	std::shared_ptr<T> shared_from_this()
	{
		return std::shared_ptr<T>(enable_shared_from_this<enable_shared_from_this_virtual_base>::shared_from_this(), static_cast<T*>(this));
	}
	std::shared_ptr<T const> shared_from_this() const
	{
		return std::shared_ptr<T const>(enable_shared_from_this<enable_shared_from_this_virtual_base>::shared_from_this(), static_cast<T const*>(this));
	}
};

//}