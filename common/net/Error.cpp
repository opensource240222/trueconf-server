#include "net/Error.h"

namespace net {

class net_category_impl : public boost::system::error_category
{
	const char* name() const noexcept override
	{
		return "net";
	}

	std::string message(int ev) const override
	{
		switch (static_cast<errc>(ev))
		{
		case errc::packet_truncated:
			return "Incoming packet was truncated to fit the read buffer";
		default:
			return "Unknown net error";
		}
	}
};

const boost::system::error_category& net_category()
{
	static net_category_impl instance;
	return instance;
}

}
