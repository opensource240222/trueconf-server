#include "Error.h"

namespace transport {

class transport_category_impl : public boost::system::error_category
{
	const char* name() const noexcept override
	{
		return "transport";
	}

	std::string message(int ev) const override
	{
		switch (static_cast<errc>(ev))
		{
		case errc::invalid_message_header:
			return "Invalid message header";
		case errc::invalid_message_head_cksum:
			return "Invalid message header checksum";
		case errc::invalid_message_body_cksum:
			return "Invalid message body checksum";
		case errc::message_encryption_error:
			return "Message encryption error";
		case errc::handshake_error:
			return "Handshake error";
		case errc::secure_handshake_error:
			return "Secure handshake error";
		default:
			return "Unknown transport error";
		}
	}
};

const boost::system::error_category& transport_category()
{
	static transport_category_impl instance;
	return instance;
}

}
