#include "Error.h"

namespace acs {

class acs_category_impl : public boost::system::error_category
{
	const char* name() const noexcept override
	{
		return "acs";
	}

	std::string message(int ev) const override
	{
		switch (static_cast<errc>(ev))
		{
		case errc::handler_initialization_failed:
			return "Handler initialization failed";
		case errc::handler_not_found:
			return "Handler not found";
		case errc::listener_already_exists:
			return "Listener already exists";
		case errc::listener_not_found:
			return "Listener not found";
		default:
			return "Unknown ACS error";
		}
	}
};

const boost::system::error_category& acs_category()
{
	static acs_category_impl instance;
	return instance;
}

}
