#pragma once

#include "std-generic/compat/boost/enable_shared_from_this.hpp"

namespace boost {

class enable_shared_from_this_virtual_base : public vs_boost::enable_shared_from_this<enable_shared_from_this_virtual_base>
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
	shared_ptr<T> shared_from_this()
	{
		return shared_ptr<T>(enable_shared_from_this<enable_shared_from_this_virtual_base>::shared_from_this(), static_cast<T*>(this));
	}
	shared_ptr<T const> shared_from_this() const
	{
		return shared_ptr<T const>(enable_shared_from_this<enable_shared_from_this_virtual_base>::shared_from_this(), static_cast<T const*>(this));
	}
};

}