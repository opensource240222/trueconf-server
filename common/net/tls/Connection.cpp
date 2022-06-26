#include "Connection.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/clib/strcasecmp.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/MakeShared.h"
#include "SecureLib/VS_SecureConstants.h"
#include "std-generic/compat/memory.h"
#include "acs_v2/Handler.h"

#include "std/debuglog/VS_Debug.h"

#include "SecureLib/OpenSSLCompat/tc_x509.h"
#include <openssl/err.h>

#include <mutex>

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

const size_t detail::SocketImpl::c_read_size = 0xffff;

tls::socket::socket(boost::asio::io_service& service)
	: m_impl(vs::MakeShared<detail::SocketImpl>(service))
{
	assert(m_impl != nullptr);
}

tls::socket::socket(boost::asio::io_service & service, bool initSecData)
	: m_impl(vs::MakeShared<detail::SocketImpl>(service, initSecData))
{
	assert(m_impl != nullptr);
}

tls::socket& tls::socket::operator=(tls::socket&& other) noexcept
{
	if (this == &other)
		return *this;
	tls::socket temp(std::move(*this));
	m_impl = std::move(other.m_impl);
	other.m_impl = nullptr;
	return *this;
}

tls::socket::socket(tls::socket&& other) noexcept
	:m_impl(std::move(other.m_impl))
{
	other.m_impl = nullptr;
}

boost::asio::io_service& tls::socket::get_io_service()
{
	return m_impl->GetIOS();
}

boost::asio::ip::tcp::socket::native_handle_type tls::socket::native_handle()
{
	return m_impl->NativeHandle();
}

void tls::socket::close(boost::system::error_code& ec)
{
	m_impl->CloseConnection(ec);
}

bool tls::socket::is_open() const
{
	return m_impl->IsOpen();
}

tls::endpoint tls::socket::local_endpoint(boost::system::error_code& ec) const
{
	return m_impl->LocalEp(ec);
}

tls::endpoint tls::socket::remote_endpoint(boost::system::error_code& ec) const
{
	return m_impl->RemoteEp(ec);
}

bool tls::socket::reset_secure_data(const void * privKey, unsigned int privKeySize, const char * privKeyPass, const void * endCert, unsigned int endCertSize, const void * caCert, unsigned int caCertSize)
{
	return m_impl->ResetSecureData(privKey, privKeySize, privKeyPass, endCert, endCertSize, caCert, caCertSize);
}

bool tls::socket::derive_key(std::vector<uint8_t>& key, size_t wantedLen, const char * label, size_t label_len, const uint8_t * context, size_t context_len)
{
	return m_impl->DeriveKey(key, wantedLen, label, label_len, context, context_len);
}

static int s_additionalDataIndex = 0;
static std::once_flag s_sslExDataFlag;

static int verify_cb(int preverifyOk, X509_STORE_CTX* ctx)
{
	//	We have to do this because OpenSSL doesn't provide a proper way to pass/capture TlsContext or 'this'
	//	to a callback (since it only accepts C function pointers), so we have to use OpenSSL extra data fields
	//	(which, as you can see below, are quite disgusting)
	VS_TlsContext* tlsContext = // here we go
		static_cast<VS_TlsContext*>(
			SSL_CTX_get_ex_data(
				static_cast<SSL_CTX*>(
					SSL_get_SSL_CTX(
	/*_Inside thi_*/	static_cast<SSL*>(
	/*_s pyramid _*/		X509_STORE_CTX_get_ex_data(
	/*_lies a who_*/			ctx, SSL_get_ex_data_X509_STORE_CTX_idx()
	/*_le day of _*/		)
	/*_my work..._*/	)
					)
				), s_additionalDataIndex
			)
		);
	X509* certPointer = X509_STORE_CTX_get0_cert(ctx);
	if (certPointer != nullptr)
		tlsContext->cert.SetCert(X509_dup(certPointer));
	//	If we are ok, we are ok
	if (preverifyOk)
	{
		tlsContext->certCheckStatus = VS_TlsContext::ccs_success;
		return preverifyOk;
	}
	//	In case someone tries to restore session with certificate that became invalid
	//	(if ccs_success was set way earlier)
	if (tlsContext->certCheckStatus == VS_TlsContext::ccs_success)
		return preverifyOk;
	tlsContext->certCheckStatus = VS_TlsContext::ccs_failure;
	//	If cert check failed, maybe it's just a regular client trying to connect to us?
	//	(if it's not, we will terminate later in things like TransportRouter::SetConnection)
	if (tlsContext->connectionType == VS_TlsContext::ct_am_server)
		return 1; // success in OpenSSLian
//	We are a client, so it's a server who has an invalid certificate
	dstream4 << "tls::socket: Certificate verification failed! " << ssl::utils::GetOpenSSLErrorStack();
	return 0; // failure
}

/* SSL callbacks */
static void msg_callback(int write_p, int /*version*/, int content_type, const void *buf, size_t /*len*/, SSL *ssl, void */*arg*/)
{
	if (content_type == 21) //error
	{
		if (*(static_cast<const char*>(buf)) == 0x02) //fatal
			SSL_shutdown(ssl);
	}
}

void detail::SocketImpl::Destroy()
{
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		m_socket.close();
		if (m_handshakeStatus == ssl::HandshakeStatus::hs_success)
			SSL_shutdown(m_ssl.get());

		// if SSL_set_bio wasn't called pRbio and pWbio will be nullptrs
		// this means m_bio_in and m_bio_out are not connected to m_ssl
		// and we must free it manualy
		auto pRbio = SSL_get_rbio(m_ssl.get());
		if (!pRbio)
			BIO_free(m_bio_in);
		auto pWbio = SSL_get_wbio(m_ssl.get());
		if (!pWbio)
			BIO_free(m_bio_out);

		m_ssl.reset();	// it will call  BIO_free(m_bio_in) and BIO_free(m_bio_out) when they are connected to m_ssl
		m_ctx.reset();
	});
	done.wait();
}

void detail::SocketImpl::MakeHandshake(const void* data, size_t size)
{
	assert(m_strand.running_in_this_thread());
	assert(m_hshaker != nullptr);
	assert(m_socket.is_open());
	assert(m_handshakeStatus != ssl::HandshakeStatus::hs_success);

	m_handshakeStatus = m_hshaker->DoHandshake(data, size);
	if (m_handshakeStatus == ssl::HandshakeStatus::hs_failure) {
		boost::system::error_code ec;
		dstream4 << "tls::socket " << m_socket.local_endpoint(ec) << " <-> " << m_socket.remote_endpoint(ec) << " handshake failure. Error stack:" << ssl::utils::GetOpenSSLErrorStack();;
		m_socket.get_io_service().post([this, self = shared_from_this()]() {m_onHandshakeDone(boost::asio::error::operation_aborted); });
		m_socket.close();
		return;
	}

	// if after hanshake success we have nothing to send - it's 'finished' msg was received
	if (!m_hshaker->IsServer() && m_hshaker->PendingBytes() == 0u)
		serversFinishedMsgReceived = true;

	// If we have something to send, send it
	if (m_hshaker->PendingBytes() != 0u) {
		acs::Handler::stream_buffer wBuff;
		m_hshaker->GetDataToSend(wBuff);

		m_wQueue.emplace(std::move(wBuff));
		auto &msg = m_wQueue.back();
		boost::asio::async_write(m_socket, boost::asio::buffer(msg.data(), msg.size()), m_strand.wrap(
			[this, self = shared_from_this()](const boost::system::error_code& ec, size_t bytes_transferred) {
			HandleHandshakeWrite(ec, bytes_transferred);
		}));
	}

	if (m_handshakeStatus == ssl::HandshakeStatus::hs_success) {
		if (serversFinishedMsgReceived || m_hshaker->IsServer())
			m_socket.get_io_service().post([this, self = shared_from_this()]() {m_onHandshakeDone(boost::system::error_code()); });
		else
			m_handshakeStatus = ssl::HandshakeStatus::hs_ongoing;
	}
}

void detail::SocketImpl::HandleHandshakeWrite(boost::system::error_code ec, size_t /*transfered*/)
{
	assert(m_strand.running_in_this_thread());

	if (ec) {
		m_handshakeStatus = ssl::HandshakeStatus::hs_failure;
		boost::system::error_code err;
		dstream2 << "tls::socket " << m_socket.local_endpoint(err) << " <-> " << m_socket.remote_endpoint(err) << " : write failed: " << ec.message();
		m_socket.close();
	}

	assert(!m_wQueue.empty());
	m_wQueue.pop();
}

void detail::SocketImpl::StartHandshakeRead()
{
	assert(m_strand.running_in_this_thread());

	m_rQueue.emplace();
	auto &readBuff = m_rQueue.back();
	readBuff.resize(c_read_size);

	m_socket.async_read_some(boost::asio::buffer(readBuff.data(), readBuff.size()), m_strand.wrap(
		[this, self = this->shared_from_this()](const boost::system::error_code& ec, size_t bytesTransferred) mutable
	{
		boost::system::error_code err;
		if (ec) {
			dstream2 << "tls::socket " << m_socket.local_endpoint(err) << " <-> " << m_socket.remote_endpoint(err) << " : read failed: " << ec.message();
			m_socket.close();
			return;
		}
		if (!m_socket.is_open()) {
			dstream4 << "tls::socket " << m_socket.local_endpoint(err) << " <-> " << m_socket.remote_endpoint(err) << ": was closed before dispatch was finished\n";
			return;
		}

		acs::Handler::stream_buffer recvData(std::move(m_rQueue.front()));
		assert(!m_rQueue.empty());
		m_rQueue.pop();
		if (bytesTransferred == 0) {
			StartHandshakeRead();
			return;
		}

		recvData.resize(bytesTransferred);
		HandleHandshakeRead(std::move(recvData));
	}));
}

void detail::SocketImpl::HandleHandshakeRead(acs::Handler::stream_buffer && buffer)
{
	assert(m_strand.running_in_this_thread());
	MakeHandshake(buffer.data(), buffer.size());

	if(m_handshakeStatus == ssl::HandshakeStatus::hs_ongoing)
		StartHandshakeRead();
}

bool InitWithSecureData(SSL_CTX *ctx)
{
	assert(ctx != nullptr);
	if (!ctx)
		return false;

	std::unique_ptr<uint8_t, free_deleter> buf;
	VS_RegistryKey key(false, CONFIGURATION_KEY, true, true);

	int size = key.GetValue(buf, VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY);
	if (size <= 0)
		return false;

	VS_Container chainCont;
	chainCont.Deserialize(buf.get(), size);
	chainCont.Reset();
	while (chainCont.Next())
	{
		if (chainCont.GetName() && (strcasecmp(chainCont.GetName(), CERTIFICATE_CHAIN_PARAM) == 0))
		{
			size_t sz = 0;
			const void *cert = chainCont.GetBinValueRef(sz);

			if (!cert || sz == 0)
				return false;
			if (!ssl::utils::AddCertChainToCTX(ctx, cert, sz))
				return false;
		}
	}

	// !!! Load certificate !!!
	size = key.GetValue(buf, VS_REG_BINARY_VT, SRV_CERT_KEY);
	if (size <= 0)
		return false;
	if (!ssl::utils::AddCertChainToCTX(ctx, buf.get(), static_cast<unsigned int>(size)))
		return false;

	// !!! Load private key !!!
	size = key.GetValue(buf, VS_REG_BINARY_VT, SRV_PRIVATE_KEY);
	if (size <= 0)
		return false;
	if (!ssl::utils::AddPrivateKeyToCTX(ctx, buf.get(), size, ""))
		return false;

	VS_GET_PEM_CACERT
	if (!ssl::utils::AddCertToCTX(ctx, PEM_CACERT, strlen(PEM_CACERT) + 1))
		return false;

	return true;
}

detail::SocketImpl::SocketImpl(boost::asio::io_service &ios, bool initSecData)
	: m_strand(ios)
	, m_socket(ios)
	, m_ctx(nullptr)
	, m_ssl(nullptr)
	, m_bio_in(nullptr)
	, m_bio_out(nullptr)
	, m_secureDataLoaded(false)
	, m_handshakeStatus(ssl::HandshakeStatus::hs_failure)
	, serversFinishedMsgReceived(false)
	, m_hshaker(nullptr)
	, m_encryptor(nullptr)
	, m_decryptor(nullptr)
{
	//	Initialize a special OpenSSL index to be used later
	std::call_once(s_sslExDataFlag, [this]() {
		s_additionalDataIndex = SSL_CTX_get_ex_new_index(0, nullptr, nullptr, nullptr, nullptr);
	});

	m_ctx = std::unique_ptr<SSL_CTX, ssl::utils::SSL_CTX_deleter>(SSL_CTX_new(TLS_method()));
	if (!m_ctx) {
		dstream3 << "detail::SocketImpl error=" << ERR_get_error() << ", in SSL_CTX_new\n" << ssl::utils::GetOpenSSLErrorStack();
		return;
	}

	m_bio_in = BIO_new(BIO_s_mem());
	BIO_ctrl(m_bio_in, BIO_C_SET_BUF_MEM_EOF_RETURN, EOF, NULL);
	m_bio_out = BIO_new(BIO_s_mem());
	BIO_ctrl(m_bio_out, BIO_C_SET_BUF_MEM_EOF_RETURN, EOF, NULL);
	if (!m_bio_in || !m_bio_out) {
		dstream3 << "detail::SocketImpl error=" << ERR_get_error() << ", in BIO_new\n" << ssl::utils::GetOpenSSLErrorStack();
		return;
	}

	// REEEEE THIS CODE WILL BREAK!Remove me if it didn't
	SSL_CTX_ctrl(m_ctx.get(), SSL_CTRL_MODE, SSL_OP_ALL | SSL_OP_NO_SSLv3 | SSL_OP_SINGLE_DH_USE, NULL);
	SSL_CTX_set_msg_callback(m_ctx.get(), &msg_callback);

	//	Pass m_tlsContext pointer to SSL_CTX so it can be fetched in verify_cb
	SSL_CTX_set_ex_data(m_ctx.get(), s_additionalDataIndex, &m_tlsContext);

	SSL_CTX_set_verify(m_ctx.get(), SSL_VERIFY_PEER, verify_cb);
	SSL_CTX_ctrl(m_ctx.get(), SSL_CTRL_MODE, SSL_MODE_ENABLE_PARTIAL_WRITE, NULL);
	SSL_CTX_ctrl(m_ctx.get(), SSL_CTRL_MODE, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER, NULL);

	m_secureDataLoaded = true;
	if(initSecData)
		m_secureDataLoaded = InitWithSecureData(m_ctx.get());

	m_ssl = std::unique_ptr<SSL, ssl::utils::SSL_deleter>(SSL_new(m_ctx.get()));
	if (!m_ssl) {
		dstream3 << "detail::SocketImpl error=" << ERR_get_error() << ", in SSL_new\n" << ssl::utils::GetOpenSSLErrorStack();
		return;
	}
	SSL_set_bio(m_ssl.get(), m_bio_in, m_bio_out);
	m_encryptor = std::make_unique<ssl::Encryptor>(m_bio_out, m_ssl.get());
	m_decryptor = std::make_unique<ssl::Decryptor>(m_bio_in, m_ssl.get());
}

detail::SocketImpl::~SocketImpl() {
	Destroy();
}

bool detail::SocketImpl::IsValid() const
{
	return m_secureDataLoaded;
}

bool detail::SocketImpl::DeriveKey(std::vector<uint8_t>& key, size_t wantedLen, const char * label, size_t label_len, const uint8_t * context, size_t context_len)
{
	bool res(false);
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		if (!m_ssl || SSL_get_shutdown(m_ssl.get())) {
			assert(!res);
			return;
		}
		key.resize(wantedLen);
		res = (1 == SSL_export_keying_material(m_ssl.get(), key.data(), wantedLen, label, label_len, context, context_len, 1));
	});
	done.wait();
	return res;
}

bool detail::SocketImpl::ResetPrivateKey(const void * key, unsigned int size, const char *pass)
{
	assert(m_strand.running_in_this_thread());
	assert(key != nullptr);
	assert(pass != nullptr);
	return ssl::utils::AddPrivateKeyToCTX(m_ctx.get(), key, size, pass);
}

bool detail::SocketImpl::ResetEndCert(const void * cert, unsigned int size)
{
	assert(m_strand.running_in_this_thread());
	assert(m_ctx != nullptr);
	assert(cert != nullptr);
	return ssl::utils::AddCertChainToCTX(m_ctx.get(), cert, size);
}

bool detail::SocketImpl::AddCert(const void * cert, unsigned int size)
{
	assert(m_strand.running_in_this_thread());
	assert(m_ctx != nullptr);
	assert(cert != nullptr);
	return ssl::utils::AddCertToCTX(m_ctx.get(), cert, size);
}

bool detail::SocketImpl::ResetSecureData(const void * privKey, unsigned int privKeySize, const char * privKeyPass,
	const void * endCert, unsigned int endCertSize,
	const void * caCert, unsigned int caCertSize)
{
	bool res(false);
	vs::event done(true);
	m_strand.dispatch([&]() {
		VS_SCOPE_EXIT{ done.set(); };
		res = m_secureDataLoaded =
			ResetEndCert(endCert, endCertSize) && ResetPrivateKey(privKey, privKeySize, privKeyPass) && AddCert(caCert, caCertSize);
	});
	done.wait();
	return res;
}



#undef DEBUG_CURRENT_MODULE