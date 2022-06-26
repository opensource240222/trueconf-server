#pragma once

#include <boost/enable_shared_from_this.hpp>

#if BOOST_VERSION >= 105800

namespace vs_boost {
using boost::enable_shared_from_this;
}

#else

namespace vs_boost {

template <class T>
class enable_shared_from_this : public ::boost::enable_shared_from_this<T>
{
public:
	boost::weak_ptr<T>       weak_from_this()       { return this->shared_from_this(); }
	boost::weak_ptr<const T> weak_from_this() const { return this->shared_from_this(); }
};

}

#endif
