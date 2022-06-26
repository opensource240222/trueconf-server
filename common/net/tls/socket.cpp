#include "socket.h"

#include <boost/asio/buffer.hpp>

#include "SecureLib/VS_SecureConstants.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std-generic/compat/memory.h"
#include "std-generic/clib/strcasecmp.h"

namespace net
{

tls::socket::socket(boost::asio::io_service& service):
	m_ctx(boost::asio::ssl::context::tlsv12),
	m_sslStream(vs::make_unique<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(service, *m_ctx))
{
	VS_GET_PEM_CACERT
	m_ctx->set_options(
		boost::asio::ssl::context_base::no_sslv3 |
		boost::asio::ssl::context_base::single_dh_use);
//	Didn't find this in boost::asio::ssl::context so gonna do it manually
	SSL_CTX_ctrl(m_ctx->native_handle(), SSL_CTRL_MODE, SSL_MODE_ENABLE_PARTIAL_WRITE, NULL);
	SSL_CTX_ctrl(m_ctx->native_handle(), SSL_CTRL_MODE, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER, NULL);
	m_ctx->add_certificate_authority(boost::asio::const_buffer(PEM_CACERT, strlen(PEM_CACERT) + 1));
	m_ctx->set_verify_mode(boost::asio::ssl::verify_peer);
	AttemptLoadCerts();
}

tls::socket& tls::socket::operator=(tls::socket&& other)
{
	m_sslStream = std::move(other.m_sslStream);
	m_ctx = std::move(other.m_ctx);
	return *this;
}

tls::socket::socket(tls::socket&& other)
{
	m_sslStream = std::move(other.m_sslStream);
	m_ctx = std::move(other.m_ctx);
}

boost::asio::io_service& tls::socket::get_io_service()
{
	return m_sslStream->get_io_service();
}

boost::asio::ip::tcp::socket::native_handle_type tls::socket::native_handle()
{
	return m_sslStream->lowest_layer().native_handle();
}

void tls::socket::close(boost::system::error_code& ec)
{
	m_sslStream->lowest_layer().close(ec);
}

bool tls::socket::is_open() const
{
	return m_sslStream->lowest_layer().is_open();
}

net::tls::endpoint tls::socket::local_endpoint(boost::system::error_code& ec) const
{
	return m_sslStream->lowest_layer().local_endpoint(ec);
}

net::tls::endpoint tls::socket::remote_endpoint(boost::system::error_code& ec) const
{
	return m_sslStream->lowest_layer().remote_endpoint(ec);
}

bool tls::socket::AttemptLoadCerts()
{
	// open registry key
	int size;
	std::unique_ptr<uint8_t, free_deleter> buf;
	VS_RegistryKey key(false, CONFIGURATION_KEY, true, true);

	// !!! Load certificate chain !!!
	size = key.GetValue(buf, VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY);

	// can't read from registry
	if (size == 0)
		return false;

	{
		VS_Container chain_cont;

		chain_cont.Deserialize(buf.get(), size);
		chain_cont.Reset();
		while (chain_cont.Next())
		{
			if (chain_cont.GetName() && (strcasecmp(chain_cont.GetName(), CERTIFICATE_CHAIN_PARAM) == 0))
			{
				size_t sz = 0;
				const void *cert = chain_cont.GetBinValueRef(sz);

				if (!cert || sz == 0)
					return false;
				m_ctx->use_certificate(boost::asio::const_buffer(cert, sz),
					boost::asio::ssl::context::file_format::pem);
			}
		}
	}

	// !!! Load certificate !!!
	size = key.GetValue(buf, VS_REG_BINARY_VT, SRV_CERT_KEY);

	// can't read from registry
	if (size == 0)
		return false;

	m_ctx->use_certificate(boost::asio::const_buffer(buf.get(), size),
		boost::asio::ssl::context::file_format::pem);

	// !!! Load private key !!!
	size = key.GetValue(buf, VS_REG_BINARY_VT, SRV_PRIVATE_KEY);

	// can't read from registry
	if (size <= 0)
		return false;

	m_ctx->use_private_key(boost::asio::const_buffer(buf.get(), size),
		boost::asio::ssl::context::file_format::pem);

	return true;
}

}// namespace net