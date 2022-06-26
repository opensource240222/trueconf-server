#include "VS_ConnectionTLS.h"

#include <winsock2.h>
#include <cstdlib>

#include <openssl/err.h>
#include <boost/make_shared.hpp>
#include <string>

#include "SecureLib/OpenSSLCompat/tc_x509.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_SSLConfigKeys.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_WorkThreadIOCP.h"
#include "net/tls/Utils.h"

#include "VS_ConnectionTypes.h"

#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

namespace
{
const char moduleName[] = "VS_ConnectionTLS: ";
const char handshakeFailed[] = "TLS handshake failed! ";

}

int ssl_do_handshake(SSL *ssl)
{
	ERR_clear_error();
	return ::SSL_do_handshake(ssl);
}

int ssl_read(SSL *ssl, void *buf, int num)
{
	ERR_clear_error();
	return ::SSL_read(ssl, buf, num);
}

int ssl_write(SSL *ssl, const void *buf, int num)
{
	ERR_clear_error();
	return ::SSL_write(ssl, buf, num);
}

// for debugging purposes (sync handshaker is proven to be correct)
static const bool ENABLE_ASYNC_HANDSHAKE = true;
int sslExDataIndex = 0;
std::once_flag sslExDataFlag;

/* !!! Handshaker !!! */

VS_ConnectionTLS::Handshaker::Handshaker(VS_ConnectionTLS *master, bool is_server):
	m_ssl(master->m_ssl), m_ctx(master->m_ctx),
	is_cb_called(false), m_is_srv(is_server),
	m_was_handshake(false), m_in(master->m_bio_in),
	m_out(master->m_bio_out), m_master(master)
{
	m_buf[0] = 0; //cppcheck
}

VS_ConnectionTLS::Handshaker::HandshakeStatus VS_ConnectionTLS::Handshaker::TryHandshake()
{
	if (m_was_handshake)
		return hs_success;

	if (!strcmp("SSLOK ", SSL_state_string(m_ssl)))
	{
		m_was_handshake = true;
		return hs_success;
	}

	int32_t result = ssl_do_handshake(m_ssl);

	if (result == 1 && !strcmp("SSLOK ", SSL_state_string(m_ssl)))
	{
		m_was_handshake = true;
		return hs_success;
	}
	else if (result == -1)
	{
		int32_t error = SSL_get_error(m_ssl, result);
		if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
			return hs_ongoing;
	}

	return hs_failure;
}

int VS_ConnectionTLS::Handshaker::SendAll(BIO *b, unsigned long& mills)
{
	int to_send, sent;

	to_send = sent = 0;

	to_send = BIO_read(b, m_buf, BUFSZ);

	while (sent < to_send)
	{
		int ret;
		ret = m_master->RawSend(m_buf, static_cast<unsigned long>(to_send - sent), mills);

		// error
		if (ret < 0)
			return -1;

		sent += ret;
	}

	return sent;
}

int VS_ConnectionTLS::Handshaker::ReceiveSome(BIO *b, unsigned long& mills)
{
	int received, written;

	received = m_master->RawReceive(m_buf, BUFSZ, mills, true);

	// error
	if (received <= 0)
		return received;

	written = BIO_write(b, m_buf, received);

	//error
	if (written != received)
		return -1;

	return received;
}

template<typename F>
void with_ssl_error_stack(F&& f) {
	BIO* bio = BIO_new(BIO_s_mem());
	ERR_print_errors(bio);
	char* tempBuf = nullptr;
	size_t length = BIO_get_mem_data(bio, &tempBuf);
	string_view errorStack(tempBuf, length);
	std::forward<F>(f)(errorStack);
	BIO_free(bio); // this gets rid of tempBuf as well
}

bool VS_ConnectionTLS::Handshaker::DoHandshake(const void *data, size_t size, unsigned long& mills)
{
	HandshakeStatus status = hs_ongoing;
	int written;

	if (!m_is_srv) // client
	{
//		If we are the ones initiating the connection, we can't possibly have incoming data
		assert(data == nullptr || size == 0);
		SSL_set_connect_state(m_ssl);
	}
	else // server
	{
		SSL_set_accept_state(m_ssl);

		// !!! Client Hello !!!
		// put (part of) Client Hello data we have received into the SSL input buffer, and do handshake.
		if (data != NULL && size > 0)
		{
			if ((written = BIO_write(m_in, data, size)) <= 0)
			{
				dstream2 << moduleName << "error: couldn't feed ClientHello at handshake start!\n";
				return false;
			}
		}
	}

//	Try unless TryHandshake returns true (handshake is finished) or we tried too many times and failed
//	or timeout is reached
	int Try;
	for (Try = 0; hs_ongoing == (status = TryHandshake()) && Try <= MAX_TRIES && mills; Try++)
	{
//		If we have something to send, send it
		if (BIO_ctrl_pending(m_out))
		{
			if (SendAll(m_out, mills) <= 0)
			{
				status = hs_disconnect;
				break;
			}

			continue;
		}

//		If OpenSSL reached the end of already read data, we need to receive more from network
		if (0 != BIO_ctrl(reinterpret_cast<BIO *>(m_in), BIO_CTRL_EOF, 0, NULL))
		{
			if (ReceiveSome(m_in, mills) <= 0)
			{
				status = hs_disconnect;
				break;
			}

			continue;
		}
	}
	if (status == hs_success)
		return true;
	else
	{
		if (mills == 0)
			dstream3 << moduleName << handshakeFailed << "Reason: timeout\n";
		else if (Try >= MAX_TRIES)
			dstream3 << moduleName << handshakeFailed << "Reason: maximum number of attempts exceeded!\n";
		else if (status == hs_disconnect)
			dstream3 << moduleName << handshakeFailed << "Reason: connection closed by remote host\n";
		else if (status == hs_failure)
		{
			with_ssl_error_stack([](string_view errorStack) {
				dstream3 << moduleName << handshakeFailed << "Reason: protocol/SSL error\n";
				dstream4 << moduleName << "OpenSSL error stack: \n" << errorStack << '\n';
			});
		}
		return false;
	}
}

bool VS_ConnectionTLS::Handshaker::Do(const void *data, size_t size, unsigned long& mills)
{
	int bytesRead = 0;
	void *cb_data = NULL; // data to pass to callback
	unsigned long cb_data_len = 0; // size of data for callback

	bool success = DoHandshake(data, size, mills);

	// we have something to send (prabably SSL session ticket, if we are in server mode)
	if ( success && BIO_ctrl_pending(m_out) > 0)
	{
		if (SendAll(m_out, mills) <= 0)
			success = false;
	}

	// we have some incoming data to read from SSL
	if ( success == true && !(0 != BIO_ctrl(reinterpret_cast<BIO*>(m_in), BIO_CTRL_EOF, 0, NULL)))
	{
		bytesRead = ssl_read(m_ssl, m_buf, BUFSZ);

		if (bytesRead > 0)
		{
			cb_data = m_buf;
			cb_data_len = static_cast<unsigned long>(bytesRead);
		}
		else
		{
			bytesRead = 0;
		}
	}

	// invoke callback
	if (!m_master->m_cb.empty())
	{
		m_master->m_cb(success, m_master, cb_data, cb_data_len);
	}

	return success;
}

/* !!! END Handshaker !!! */


/* !!! AsyncHandshaker !!! */

VS_ConnectionTLS::AsyncHandshaker::AsyncHandshaker(VS_ConnectionTLS *master, bool is_server):
	m_thread(master->m_thread), m_ssl(master->m_ssl), m_ctx(master->m_ctx), is_cb_called(false),
	m_is_srv(is_server), m_was_handshake(false), m_in(master->m_bio_in), m_out(master->m_bio_out),
	m_master(master), m_try_number(0)
{
	m_rbuf[0] = m_wbuf[0] = 0; //cppcheck
}

VS_ConnectionTLS::AsyncHandshaker::~AsyncHandshaker()
{
	m_master = NULL;
}

void VS_ConnectionTLS::AsyncHandshaker::Done(void *data, unsigned long len)
{
	boost::shared_ptr<VS_MessageData> msg_data(new VS_MessageData(MSG_DONE, data, len));
	m_thread->Post(boost::static_pointer_cast<VS_MessageHandler>(shared_from_this()), msg_data);
}

void VS_ConnectionTLS::AsyncHandshaker::invokecb(bool status)
{
	void *data = NULL;
	unsigned long len = 0;
	int pending;
	int decoded = 0;

	m_thread->UnregisterTimeout(shared_from_this());
	VS_ConnectionTLS *con = m_master;
	if (is_cb_called)
		return;
	is_cb_called = true;

	if (!m_master)
		return;

	if (status == false)
	{
		m_master->Disconnect();
		m_master->Close();
	}

	// decode pending data if any
	if ((pending = BIO_ctrl_pending(m_in)) > 0)
	{

		// decode loop
		while (!(0 != BIO_ctrl(reinterpret_cast<BIO *>(m_in), BIO_CTRL_EOF, 0, NULL)))
		{
			int chunk_size = ssl_read(m_ssl, m_rbuf + decoded, BUFSZ - decoded);

			if (chunk_size > 0)
			{
				decoded += chunk_size;
			}
			else
				break;
		}

		if (decoded > 0)
		{
			data = m_rbuf;
			len = static_cast<unsigned long>(decoded);
		}
		else
		{
			data = nullptr;
			len = 0;
		}
	}
	else
	{
		data = nullptr;
		len = 0;
	}

	m_master->m_handshake_status = status;
	Done(data, len);
}

VS_ConnectionTLS::AsyncHandshaker::HandshakeResult VS_ConnectionTLS::AsyncHandshaker::TryHandshake()
{
	if (m_was_handshake)
		return hr_success;

	if (!strcmp("SSLOK ", SSL_state_string(m_ssl)))
	{
		m_was_handshake = true;
		return hr_success;
	}

	int32_t result = ssl_do_handshake(m_ssl);

	if (result == 1 && !strcmp("SSLOK ", SSL_state_string(m_ssl)))
	{
		m_was_handshake = true;
		return hr_success;
	}
	else if (result == -1)
	{
		int32_t error = SSL_get_error(m_ssl, result);
		if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
			return hr_ongoing;
	}

	return hr_error;
}

bool VS_ConnectionTLS::AsyncHandshaker::Do(const void *data, unsigned long len, unsigned long& mills)
{
	m_master->SetIOHandler(this);

	m_master->m_io_thread.reset();
	m_master->SetOvReadFields(OV_READ_COMPLETE);
	m_master->SetOvWriteFields(OV_WRITE_COMPLETE);

	// save initial handshake data
	memcpy(m_rbuf, data, (len <= BUFSZ ? len : BUFSZ));
	// send init message to the thread
	boost::shared_ptr<VS_MessageData> pdata(new VS_MessageData(MSG_START, m_rbuf, len));
	m_thread->Post(boost::static_pointer_cast<VS_MessageHandler>(shared_from_this()), pdata);
	m_handshake_timeout_time = std::chrono::steady_clock::now() +
		std::chrono::milliseconds(mills);
	m_thread->RegisterTimeout(shared_from_this());

	self = shared_from_this();
	return true;
}

bool VS_ConnectionTLS::AsyncHandshaker::EnqueueRead(void *data, unsigned long len)
{
	return m_master->RawRead(data, len);
}

bool VS_ConnectionTLS::AsyncHandshaker::EnqueueWrite(void *data, unsigned long len)
{
	return m_master->RawWrite(data, len);
}

void VS_ConnectionTLS::AsyncHandshaker::Handle(const unsigned long transferred, const VS_Overlapped *ov)
{
	int len = 0;

	if (!m_master)
		return;

	switch (ov->field1)
	{
	case OV_READ_COMPLETE:
	{
		len = m_master->RawSetReadResult(transferred, ov, nullptr, true);
		if (len < 0)
		{
			dstream3 << moduleName << handshakeFailed << "Reason: connection closed by remote host!\n";
			invokecb(false);
			return;
		}

		if (BIO_write(m_in, m_rbuf, len) < 0)
		{
			dstream2 << moduleName << handshakeFailed << "Reason: unable to write handshake data to BIO!\n";
			invokecb(false);
			return;
		}
	}
	break;
	case OV_WRITE_COMPLETE:
	{
		int res = m_master->RawSetWriteResult(transferred, ov);

		if (res < 0)
		{
			dstream3 << moduleName << handshakeFailed << "Reason: connection closed by remote host!\n";
			invokecb(false);
			return;
		}
	}
	break;
	default: // illegal operation
	{
		dstream2 << moduleName << handshakeFailed << "Reason: I/O handling error!\n";
		invokecb(false);
		return;
	}
	break;
	}

	HandshakeResult result = TryHandshake(); // handle handshake data
	if (result == hr_error)
	{
		with_ssl_error_stack([](string_view error_stack) {
			dstream3 << moduleName << handshakeFailed << "Reason: protocol/SSL error\n";
			dstream4 << moduleName << "OpenSSL error stack: \n" << error_stack << '\n';
		});
		invokecb(false);
		return;
	}

	// we have some pending data on output
	if (BIO_ctrl_pending(m_out) > 0)
	{
		// send it
		int readen = BIO_read(m_out, m_wbuf, BUFSZ);

		if (readen > 0)
		{
			if (!EnqueueWrite(m_wbuf, readen))
			{
				dstream3 << moduleName << handshakeFailed << "Reason: I/O handling error!\n";
				invokecb(false);
				return;
			}
		}

		return;
	}

	if (result == hr_success)
	{
		invokecb(true);
		return;
	}

	// we have read all data already - receive some
	if (0 != BIO_ctrl(reinterpret_cast<BIO *>(m_in), BIO_CTRL_EOF, 0, NULL)/* || (m_is_srv && m_wrap.BIO_ctrl_pending(m_in) > 0)*/)
	{
		if (!EnqueueRead(m_rbuf, BUFSZ))
		{
			dstream3 << moduleName << handshakeFailed << "Reason: I/O handling error!\n";
			invokecb(false);
		}
		return;
	}

	if (m_try_number >= MAX_TRIES)
	{
		dstream3 << moduleName << handshakeFailed << "Reason: maximum number of attempts exceeded!\n";
		invokecb(false);
	}
	else
		m_try_number++;
}

void VS_ConnectionTLS::AsyncHandshaker::HandleMessage(const boost::shared_ptr<VS_MessageData> &message)
{
	unsigned long type, len;
	void *data = message->GetMessPointer(type, len);

	if (!m_master)
		return;

	switch (type)
	{
	case MSG_START:
	{
		if (!m_is_srv)
		{
			// we are connected already
			SSL_set_connect_state(m_ssl);
			ssl_do_handshake(m_ssl);
		}
		else // server
		{
			// we already have accepted connection
			SSL_set_accept_state(m_ssl);

			// !!! Client Hello !!!
			// put (part of) Client Hello data we have received into the SSL input buffer, and do hanshake.
			if (data != NULL && len > 0)
			{
				int written;
				if ((written = BIO_write(m_in, data, static_cast<int>(len))) <= 0)
				{
					dstream2 <<
						moduleName << handshakeFailed << "Reason: unable to write handshake data to BIO!\n";
					invokecb(false);
					return;
				}
			}
		}

		HandshakeResult result = TryHandshake(); // Handle handshake data;
		if (result == hr_error)
		{
			with_ssl_error_stack([](string_view error_stack) {
				dstream3 << moduleName << handshakeFailed << "Reason: protocol/SSL error\n";
				dstream4 << moduleName << "OpenSSL error stack: \n" << error_stack << '\n';
			});
			invokecb(false);
			return;
		}

		// possible Server Hello
		if (BIO_ctrl_pending(m_out))
		{
			int readen = BIO_read(m_out, m_wbuf, BUFSZ);
			if (readen < 0 )
			{
				dstream2 <<
					moduleName << handshakeFailed << "Reason: unable to read handshake data from BIO!\n";
				invokecb(false);
				return;
			}

			if (!EnqueueWrite(m_wbuf, readen))
			{
				dstream3 << moduleName << handshakeFailed << "Reason: I/O handling error!\n";
				invokecb(false);
			}
			return;
		}

		if (result == hr_success)
		{
			invokecb(true);
			return;
		}

		if (0 != static_cast<int>(BIO_ctrl((BIO *)m_in, BIO_CTRL_EOF, 0, NULL)))
		{
			if (!EnqueueRead(m_rbuf, BUFSZ))
			{
				dstream3 << moduleName << handshakeFailed << "Reason: I/O handling error!\n";
				invokecb(false);
			}
			return;
		}
	}
	break;
	case MSG_DONE:
		if (!m_master->m_cb.empty())
		{
			m_master->m_cb(m_master->m_handshake_status, m_master, data, len);
		}
	    self.reset();
		break;
	}
}

void VS_ConnectionTLS::AsyncHandshaker::HandleError(const unsigned long err, const VS_Overlapped *ov)
{
	switch (ov->field1)
	{
	case OV_READ_COMPLETE:
		m_master->RawSetReadResult(0, ov, nullptr, true);
		break;
	case OV_WRITE_COMPLETE:
		m_master->SetWriteResult(0, ov);
		break;
	}
	invokecb(false);
}

void VS_ConnectionTLS::AsyncHandshaker::Timeout()
{
//	If we (over)reached our timeout, we close the socket so read/write operations will fail and
//	callback(false) will be invoked
	if (std::chrono::steady_clock::now() > m_handshake_timeout_time)
	{
		m_master->Disconnect();
		m_master->Close();
	}
}

/* !!! End AsyncHandshaker !!! */

/* !!! Read Request Utility Class !!! */
VS_ConnectionTLS::ReadRequest::ReadRequest(void *rb, size_t rb_sz)
	: m_readbuf(rb), m_readbuf_size(rb_sz), m_tmpbuf(rb_sz + BUFSZ), m_tmpbuf_offset(0), m_already_read(0), m_freebuf(NULL), m_read_called(false)
{
	if (!rb)
	{
		if (rb_sz == 0)
			m_readbuf = nullptr;
		else
			m_readbuf = malloc(rb_sz);
		m_freebuf = m_readbuf;
	}
}

VS_ConnectionTLS::ReadRequest::~ReadRequest()
{
	if (m_freebuf)
	{
		free(m_freebuf);
		m_freebuf = NULL;
	}
}

bool VS_ConnectionTLS::ReadRequest::IsAllocated()
{
	return (m_freebuf != NULL);
}

void VS_ConnectionTLS::ReadRequest::AccquireReadBuffer(ReadRequest &req)
{
	m_freebuf = req.m_freebuf;

	SetAlreadyRead(req.GetDataSize() + req.GetAlreadyRead());
	req.ResetReadBuffer();
}


void VS_ConnectionTLS::ReadRequest::ResetReadBuffer(void)
{
	m_freebuf = NULL;
	m_readbuf = NULL;
}

void VS_ConnectionTLS::ReadRequest::AppendData(const void *data, const size_t size)
{
	const uint8_t *pbuf = static_cast<const uint8_t *>(data);
	if (size == 0 || data == NULL)
	{
		return;
	}

	std::copy(&pbuf[0], &pbuf[size], std::back_inserter(m_ready_data));
}

void *VS_ConnectionTLS::ReadRequest::GetData(void)
{
	if (m_ready_data.size() > 0)
	{
		return &(m_ready_data[0]);
	}

	return NULL;
}

size_t VS_ConnectionTLS::ReadRequest::GetDataSize(void)
{
	return m_ready_data.size();
}

size_t VS_ConnectionTLS::ReadRequest::GetTotalDataSize(void)
{
	return m_ready_data.size() + m_already_read;
}


size_t VS_ConnectionTLS::ReadRequest::GetReadBufSize(void)
{
	return m_readbuf_size;
}

size_t VS_ConnectionTLS::ReadRequest::GetReadBufSizeLeft(void)
{
	return m_readbuf_size - GetDataSize();
}

void *VS_ConnectionTLS::ReadRequest::GetReadBuf(void)
{
	return m_readbuf;
}

void *VS_ConnectionTLS::ReadRequest::GetReadBufInitial(void)
{
	return (m_freebuf == NULL ? m_readbuf : m_freebuf);
}

void *VS_ConnectionTLS::ReadRequest::GetReadBufLeft(void)
{
	uint8_t *p = static_cast<uint8_t *>(m_readbuf);
	return &p[GetDataSize()];
}

void *VS_ConnectionTLS::ReadRequest::GetTemporaryBuffer(void)
{
	return &(m_tmpbuf[m_tmpbuf_offset]);
}

void *VS_ConnectionTLS::ReadRequest::GetTemporaryBuffer(size_t offset)
{
	if (offset >= m_tmpbuf.size())
		return NULL;
	return &(m_tmpbuf[offset]);
}

size_t VS_ConnectionTLS::ReadRequest::GetTemporaryBufferOffset(void)
{
	return m_tmpbuf_offset;
}

size_t VS_ConnectionTLS::ReadRequest::GetTemporaryBufferSize(void)
{
	return m_tmpbuf.size();
}

void VS_ConnectionTLS::ReadRequest::IncreaseTemporaryBufferOffset(const int count)
{
	if (count < 0 && (static_cast<size_t>(abs(count))) > m_tmpbuf_offset)
		return;

	m_tmpbuf_offset += count;
}

void VS_ConnectionTLS::ReadRequest::SetTemporaryBufferOffset(unsigned int offset)
{
	m_tmpbuf_offset = offset;
}

void VS_ConnectionTLS::ReadRequest::SetAlreadyRead(size_t read)
{
	m_already_read = read;
}

size_t VS_ConnectionTLS::ReadRequest::GetAlreadyRead(void)
{
	return m_already_read;
}

/* !!! End of Read Request Utility Class !!! */

/* SSL callbacks */
static void msg_callback(int write_p, int version, int content_type, const void *buf, size_t len, SSL *ssl, void *arg)
{
	if(content_type == 21) //error
	{
		if(*(static_cast<const char*>(buf)) == 0x02) //fatal
			SSL_shutdown(ssl);
	}
}


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
				), sslExDataIndex
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
	return 0; // failure
}

VS_ConnectionTLS::VS_ConnectionTLS(const boost::shared_ptr<VS_WorkThread> &io_thread,
	const SSL_METHOD* method,
	bool use_IPv6)
	: VS_ConnectionTCP(use_IPv6),
	m_io_thread(io_thread), m_valid(true),
	m_cb(0), m_delegate_write_handler(NULL), m_delegate_read_handler(NULL), m_ssl(NULL),
	m_was_handshake(false), m_handshake_status(false)
{
//	Initialize a special OpenSSL index to be used later
	std::call_once(sslExDataFlag, [this]()
		{
			sslExDataIndex = SSL_CTX_get_ex_new_index(0, nullptr, nullptr, nullptr, nullptr);
		}
	);

	memset(&m_delegate_write_ov, 0, sizeof(VS_Overlapped));
	memset(&m_delegate_read_ov, 0, sizeof(VS_Overlapped));

	m_bio_in = BIO_new(BIO_s_mem());
	BIO_ctrl(m_bio_in, BIO_C_SET_BUF_MEM_EOF_RETURN, EOF, NULL);
	BIO_set_close(m_bio_in, BIO_CLOSE);
	m_bio_out = BIO_new(BIO_s_mem());
	BIO_set_close(m_bio_out, BIO_CLOSE);
	BIO_ctrl(m_bio_out, BIO_C_SET_BUF_MEM_EOF_RETURN, EOF, NULL);

	if(!m_bio_in || !m_bio_out)
		m_valid = false;

	m_ctx = SSL_CTX_new(method);

	if(!m_ctx)
		m_valid = false;
//	REEEEE THIS CODE WILL BREAK! Remove me if it didn't
	SSL_CTX_ctrl(m_ctx, SSL_CTRL_MODE, SSL_OP_ALL | SSL_OP_NO_SSLv3 | SSL_OP_SINGLE_DH_USE, NULL);
	SSL_CTX_set_msg_callback(m_ctx, &msg_callback);

//	Pass m_tlsContext pointer to SSL_CTX so it can be fetched in verify_cb
	SSL_CTX_set_ex_data(m_ctx, sslExDataIndex, &m_tlsContext);

	SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER, verify_cb);
	SSL_CTX_ctrl(m_ctx, SSL_CTRL_MODE, SSL_MODE_ENABLE_PARTIAL_WRITE, NULL);
	SSL_CTX_ctrl(m_ctx, SSL_CTRL_MODE, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER, NULL);
	SSL_CTX_set_options(m_ctx, SSL_OP_NO_COMPRESSION);
	SSL_CTX_set_mode(m_ctx, SSL_MODE_RELEASE_BUFFERS);
}

VS_ConnectionTLS::VS_ConnectionTLS(VS_ConnectionTCP* conn,
	const boost::shared_ptr<VS_WorkThread> &io_thread,
	const SSL_METHOD* method,
	bool use_IPv6)
	: VS_ConnectionTLS(io_thread, method, use_IPv6)
{
	if (conn)
	{
		imp->Close();
		delete imp;

		imp = conn->imp;
		conn->imp = nullptr;
	}
}

VS_ConnectionTLS::~VS_ConnectionTLS()
{
	RemoveCallback();
	if (imp)
	{
		imp->Close();
		delete imp;
		imp = nullptr;
	}

	// shutdown SSL
	if (m_handshake_status)
		SSL_shutdown(m_ssl);
	SSL_free(m_ssl);
	SSL_CTX_free(m_ctx);
}

bool VS_ConnectionTLS::CreateSSL()
{
	m_ssl = SSL_new(m_ctx);

	if(!m_ssl)
		return false;

	SSL_set0_rbio(m_ssl, m_bio_in);
	SSL_set0_wbio(m_ssl, m_bio_out);

	return true;
}

bool VS_ConnectionTLS::AddCaCert()
{
	VS_GET_PEM_CACERT
	return AddCert(PEM_CACERT, strlen(PEM_CACERT) + 1);
}

bool VS_ConnectionTLS::AddCert(const void *cert, const unsigned int size)
{
	assert(m_ctx != nullptr);
	assert(cert != nullptr);
	return ssl::utils::AddCertToCTX(m_ctx, cert, size);
}

bool VS_ConnectionTLS::UseEndCert(const void *cert, const unsigned int size)
{
	assert(m_ctx != nullptr);
	assert(cert != nullptr);
	return ssl::utils::AddCertChainToCTX(m_ctx, cert, size);
}

bool VS_ConnectionTLS::UsePrivateKey(const void *key, const unsigned int size, const char* pass)
{
	assert(key != nullptr);
	assert(pass != nullptr);
	return ssl::utils::AddPrivateKeyToCTX(m_ctx, key, size, pass);
}

bool VS_ConnectionTLS::IsValid() const
{
	return m_valid && VS_ConnectionTCP::IsValid();
}

bool VS_ConnectionTLS::Handshake(const void *in_buf, const unsigned long in_len,
	boost::function<void(bool,VS_ConnectionTLS*,const void*,const unsigned long)> callback,
	unsigned long& mills, bool isServer)
{
	if (m_was_handshake)
	{
		return m_handshake_status;
	}
	m_was_handshake = true;
	m_tlsContext.connectionType = isServer ? VS_TlsContext::ct_am_server : VS_TlsContext::ct_am_client;

	// set connection type as stream
	imp->type = vs_connection_type_stream;

	if(Type() != vs_connection_type_stream ||
		!CreateSSL() ||
		!AddCaCert()
		){
		if(callback)
			callback(false, this, 0, 0);
		return false;
	}

	if (m_io_thread && ENABLE_ASYNC_HANDSHAKE)
	{
		// set io thread if we have any
		if (!m_valid || !SetIOThread(m_io_thread))
		{
			if (callback)
				callback(false, this, nullptr, 0);

			return false;
		}
		m_io_thread.reset();
		if (callback)
			SetCallback(callback);
		VS_ConnectionTLS::AsyncHandshaker::Make(this, isServer)->Do(in_buf, in_len, mills);
		return true;
	}
	else
	{
		if (callback)
			SetCallback(callback);
		boost::shared_ptr<Handshaker>  hs(new Handshaker(this, isServer));
		return m_handshake_status =  hs->Do(in_buf, in_len, mills);
	}
}

bool VS_ConnectionTLS::RRead( const unsigned long n_bytes )
{
	if(SSL_get_shutdown(m_ssl) || m_read_req)
		return false;

	auto req = ReadRequest::Make(n_bytes);

	if (m_extra_readbuffer.empty())
	{
		if (!VS_ConnectionTCP::Read(req->GetTemporaryBuffer(), req->GetTemporaryBufferSize()))
			return false;
		req->SetReadCalled(true);
		m_read_req = req;
	} else
	{
		req->SetReadCalled(false);
		m_read_req = req;
//		Pretend we received something from network so callbacks are called
		imp->readOv.b_last = 1;
		if (ReadOv()->hiocp != nullptr)
			PostQueuedCompletionStatus( ReadOv()->hiocp, n_bytes, 0, (LPOVERLAPPED)ReadOv() );
		if (ReadOv()->over.hEvent != nullptr)
			SetEvent(ReadOv()->over.hEvent);
	}

	return true;
}

bool VS_ConnectionTLS::Read( void *buffer, const unsigned long n_bytes )
{
	if (SSL_get_shutdown(m_ssl) || m_read_req)
		return false;

	auto req = ReadRequest::Make(buffer, n_bytes);

	if (m_extra_readbuffer.empty())
	{
		if (n_bytes == 0)
		{
			if (!VS_ConnectionTCP::Read(nullptr, 0)) // Handle weird StreamsRouter requests
				return false;
		}
		else if (!VS_ConnectionTCP::Read(req->GetTemporaryBuffer(), req->GetTemporaryBufferSize()))
			return false;
		req->SetReadCalled(true);
		m_read_req = req;
	} else
	{
		req->SetReadCalled(false);
		m_read_req = req;
//		Pretend we received something from network so callbacks are called
		imp->readOv.b_last = 1;
		if (ReadOv()->hiocp != nullptr)
			PostQueuedCompletionStatus(ReadOv()->hiocp, n_bytes, 0, const_cast<OVERLAPPED*>(reinterpret_cast<const OVERLAPPED*>(ReadOv())));
		else if (imp->hiocp != nullptr)
			PostQueuedCompletionStatus(imp->hiocp, n_bytes, 0, const_cast<OVERLAPPED*>(reinterpret_cast<const OVERLAPPED*>(ReadOv())));
		if (ReadOv()->over.hEvent != nullptr)
			SetEvent(ReadOv()->over.hEvent);
	}

	return true;
}

bool VS_ConnectionTLS::RawRead(void *buffer, const unsigned long n_bytes)
{
	if (SSL_get_shutdown(m_ssl))
		return false;
	return VS_ConnectionTCP::Read(buffer, n_bytes);
}

bool VS_ConnectionTLS::WriteOutgoing(int written)
{
	// Write request structure
	auto req = boost::make_shared<WriteRequest>();

	// create buffer of apropriate size
	int pending = BIO_ctrl_pending(m_bio_out);
	req->data.resize(static_cast<size_t>(pending));
	req->encoded_size = pending;
	req->real_size = written;

	// get encoded data into IO buffer
	int read, wbsize = 0;
	while ((read = BIO_read(m_bio_out, &(req->data[wbsize]), pending - wbsize)) > 0)
	{
		wbsize += read;

		if (wbsize >= pending)
			break;
	}

	// add raw data size to queue
	m_write_req = req;
	// send it
	return (VS_ConnectionTCP::Write(&(req->data[0]), wbsize));
}

bool VS_ConnectionTLS::RWrite(const VS_Buffer *buffers, const unsigned long n_buffers)
{
	int totalWritten = 0, writtenSum;
	if (SSL_get_shutdown(m_ssl) || m_write_req.get() != NULL)
		return false;

	// encrypt all data in buffers
	for (unsigned long i = 0; i < n_buffers; i++)
	{
		if (buffers[i].length != 0)
		{
			int writeResult;
			writtenSum = 0;
			while (0 < (writeResult = ssl_write(m_ssl,
						reinterpret_cast<const unsigned char*>(buffers[i].buffer) + writtenSum,
						static_cast<int>(buffers[i].length) - writtenSum)))
				writtenSum += writeResult;
			if (writeResult <= 0 && writtenSum == 0)
				return false;
			totalWritten += writtenSum;
		}
	}

	// send it
	return WriteOutgoing(totalWritten);
}

bool VS_ConnectionTLS::Write(const void *buffer, const unsigned long n_bytes)
{
	if(SSL_get_shutdown(m_ssl) || m_write_req.get() != NULL)
		return false;

	if (!n_bytes)
		return false;

	// encrypt data
	int writtenSum(0), writeResult(0);
	while (0 < (writeResult = ssl_write(m_ssl, reinterpret_cast<const unsigned char*>(buffer) + writtenSum,
				n_bytes - writtenSum)))
		writtenSum += writeResult;

	if (writeResult <= 0 && writtenSum == 0)
		return false;

	// send it
	return WriteOutgoing(writtenSum);
}

bool VS_ConnectionTLS::RawWrite(const void *buffer, const unsigned long n_bytes)
{
	return VS_ConnectionTCP::Write(buffer, n_bytes);
}

//	Take data from temporary buffer, decrypt it, put into Data buffer
int VS_ConnectionTLS::DecryptInput(boost::shared_ptr<ReadRequest>& req)
{
//	Fill BIO with received data
	if (0 >= BIO_write(m_bio_in, req->GetTemporaryBuffer(0), req->GetTemporaryBufferOffset()))
		return -1;

	uint8_t tmp[BUFSZ]; //128 KB, should be enough
	int32_t finalSize(0), readResult(0);

	while (0 < (readResult = ssl_read(m_ssl, tmp, BUFSZ)))
	{
		req->AppendData(tmp, readResult);
		finalSize += readResult;
	}

	if (readResult < 0 && finalSize == 0)
	{
		if (SSL_ERROR_WANT_READ == SSL_get_error(m_ssl, readResult))
			return -2; // Request more data, let someone else deal with ReadRequest structure
		with_ssl_error_stack([](string_view errorStack) {
			dstream4 << moduleName << "error while decrypting data! OpenSSL error stack:\n"
				<< errorStack << '\n';
		});
		return -1; // If error != SSL_ERROR_WANT_READ, something way nastier has occured
//					(and we probably need to fix our code)
	}
	return finalSize;
}

int	VS_ConnectionTLS::HandleReadRequest(void **buffer, bool portion)
{
	auto req = m_read_req;
	int32_t decryptedSize(0);
	if (req->WasReadCalled()) // We have something to decrypt
	{
		if (-1 == (decryptedSize = DecryptInput(req)))
		{
			m_read_req = nullptr;
			return -1;
		}
	}
	int transferSize(-1);
	if (m_extra_readbuffer.size() >= req->GetReadBufSize())// Kinda likely, also means that read wasn't called
	{
		transferSize = req->GetReadBufSize();
		memcpy(req->GetReadBuf(), m_extra_readbuffer.data(), transferSize);
		m_extra_readbuffer.erase(m_extra_readbuffer.begin(), m_extra_readbuffer.begin() + transferSize);
		if (decryptedSize > 0) // Just in case
			m_extra_readbuffer.insert(m_extra_readbuffer.end(),
				reinterpret_cast<uint8_t*>(req->GetData()),
				reinterpret_cast<uint8_t*>(req->GetData()) + req->GetDataSize());
	}
	else if (req->GetDataSize() > 0 && m_extra_readbuffer.size() + req->GetDataSize() >= req->GetReadBufSize())
	{
		transferSize = req->GetReadBufSize();
		int toTakeFromDataBuffer = transferSize - m_extra_readbuffer.size();
//		One portion from temp buffer
		memcpy(req->GetReadBuf(), m_extra_readbuffer.data(), m_extra_readbuffer.size());
//		One portion from data buffer
		memcpy(reinterpret_cast<uint8_t*>(req->GetReadBuf()) + m_extra_readbuffer.size(),
			req->GetData(), toTakeFromDataBuffer);
//		Transfer data to temporary buffer before deletion
		m_extra_readbuffer.assign(reinterpret_cast<uint8_t*>(req->GetData()) + toTakeFromDataBuffer,
			reinterpret_cast<uint8_t*>(req->GetData()) + req->GetDataSize());
	}
	else if (portion && (!m_extra_readbuffer.empty() || 0 != req->GetDataSize()))
	{
		transferSize = m_extra_readbuffer.size() + req->GetDataSize();
//		Just flush everything we have
		memcpy(req->GetReadBuf(), m_extra_readbuffer.data(), m_extra_readbuffer.size());
		memcpy(reinterpret_cast<uint8_t*>(req->GetReadBuf()) + m_extra_readbuffer.size(), req->GetData(),
			req->GetDataSize());
	} else
		return -2;

//	If we reached here, this means that request is fulfilled and it's time to clean up and return
	if (buffer)
	{
		if (req->IsAllocated())
		{
			*buffer = req->GetReadBufInitial(); // returns buffer equal to normal read buffer
			req->ResetReadBuffer();
		} else
			*buffer = nullptr;
	}
	m_read_req = nullptr;
	return transferSize;
}

int	VS_ConnectionTLS::SetReadResult(const unsigned long b_trans, const struct VS_Overlapped *ov, void **buffer, const bool portion)
{
	int res = 0;
	VS_SCOPE_EXIT{
		if (res != -2)
			m_read_req = nullptr;
	};

	if (SSL_get_shutdown(m_ssl))
		return -1;

	if (!m_read_req)
		return -1;

	// request
	auto req = m_read_req;

	if (req->WasReadCalled())
	{
		if (req->GetReadBuf() != nullptr)
		{
			res = VS_ConnectionTCP::SetReadResult(b_trans, ov, NULL, true);
			if (res < 0)
				return res;

			req->IncreaseTemporaryBufferOffset(b_trans);
		}
		else // unlikely
		{
			m_read_req = nullptr;
			return VS_ConnectionTCP::SetReadResult(b_trans, ov, NULL, true);
		}
	}

	res = HandleReadRequest(buffer, portion);
	if (res == -2)
	{
		req->SetTemporaryBufferOffset(0);
		if (!VS_ConnectionTCP::Read(req->GetTemporaryBuffer(), req->GetTemporaryBufferSize()))
			return -1;
		req->SetReadCalled(true);
	}
	return res;
}

int VS_ConnectionTLS::GetReadResult(unsigned long &mills, void **buffer, const bool portion)
{
	if (SSL_get_shutdown(m_ssl))
		return -1;

	int res = 0;

	if (!m_read_req)
		return -1;

	// request
	auto req = m_read_req;

	VS_SCOPE_EXIT{
		if (res != -2)
			m_read_req = nullptr;
	};

	do
	{
		if (req->WasReadCalled())
		{
			if (req->GetReadBuf() != nullptr)
			{
				res = VS_ConnectionTCP::GetReadResult(mills, nullptr, true);
				if (res < 0)
					return res;
				req->IncreaseTemporaryBufferOffset(res);
			}
			else // unlikely
			{
				return -1;
			}
		}

		res = HandleReadRequest(buffer, portion);
		if (res == -2)
		{
			req->SetTemporaryBufferOffset(0);
			if (!VS_ConnectionTCP::Read(req->GetTemporaryBuffer(), req->GetTemporaryBufferSize()))
				return -1;
			req->SetReadCalled(true);
		} else
			return res;
	} while (mills > 0);

	return res = -2; // for VS_SCOPE_EXIT
}

int	VS_ConnectionTLS::RawSetReadResult(const unsigned long b_trans, const struct VS_Overlapped *ov, void **buffer, const bool portion)
{
	return VS_ConnectionTCP::SetReadResult(b_trans, ov, buffer, portion);
}

int VS_ConnectionTLS::HandleWriteRequest(const unsigned long b_trans)
{
	// get request pointer
	auto req = m_write_req;

	req->written += static_cast<const int>(b_trans);
	auto left = req->encoded_size - req->written;
	// we have transferred all pending data
	if (req->written == req->encoded_size)
	{
		int really_written = req->real_size;

		m_write_req = NULL;
		return really_written;
	}
	else if (left > 0)
	{
		// write data that left in buffer
		if (!VS_ConnectionTCP::Write(&req->data[req->written], left))
		{
			m_write_req = NULL;
			return -1;
		}

		return -2;
	}

	m_write_req = NULL;
	return -1;
}

int VS_ConnectionTLS::SetWriteResult( const unsigned long b_trans, const struct VS_Overlapped *ov )
{
	int res = VS_ConnectionTCP::SetWriteResult(b_trans, ov);

	if (res < 0)
		return res;

	if (m_write_req.get() == NULL)
	{
		return -1;
	}

	return HandleWriteRequest(res);
}

int VS_ConnectionTLS::RawSetWriteResult(const unsigned long b_trans, const struct VS_Overlapped *ov)
{
	return VS_ConnectionTCP::SetWriteResult(b_trans, ov);
}

int VS_ConnectionTLS::GetWriteResult( unsigned long &mills )
{
	if (SSL_get_shutdown(m_ssl))
		return -1;

	if (m_write_req.get() == NULL)
	{
		return -1;
	}

	// get TCP write result
	int res = VS_ConnectionTCP::GetWriteResult(mills);

	if (res < 0)
		return res;

	return HandleWriteRequest(res);
}

// !!! "Real" send and receive routines (via TCP) !!!
// It is time to bring MOAR ugliness to code
int VS_ConnectionTLS::SendData(const void *buffer, const unsigned long n_bytes, unsigned long *mills, const bool *keep_blocked)
{
	if (mills != nullptr)
	{
		if (keep_blocked != nullptr)
		{
			return VS_ConnectionTCP::Send(buffer, n_bytes, *mills, *keep_blocked);
		}
		else
		{
			return VS_ConnectionTCP::Send(buffer, n_bytes, *mills);
		}
	}

	// simple send
	return VS_ConnectionTCP::Send(buffer, n_bytes);
}

int VS_ConnectionTLS::ReceiveData(void *buffer, const unsigned long n_bytes, unsigned long *mills, bool portion)
{
	if (mills != nullptr)
	{
		return VS_ConnectionTCP::Receive(buffer, n_bytes, *mills, portion);
	}

	// simple receive
	return VS_ConnectionTCP::Receive(buffer, n_bytes);
}

int VS_ConnectionTLS::RawReceive(void *buffer, const unsigned long n_bytes, unsigned long& mills, bool portion)
{
	return ReceiveData(buffer, n_bytes, &mills, portion);
}

int VS_ConnectionTLS::RawSend(const void *buffer, const unsigned long n_bytes, unsigned long& mills)
{
	return SendData(buffer, n_bytes, &mills);
}

// !!! End

int VS_ConnectionTLS::Send(const void *buffer, const unsigned long n_bytes, unsigned long *mills, const bool *keep_blocked)
{
	int readen;
	int to_send, sent, raw_sent;
	int nrecords;
	int i;
	// buffer for encoded record
	const uint8_t *pbuf = reinterpret_cast<const uint8_t *>(buffer);

	if(SSL_get_shutdown(m_ssl))
		return -1;

	// How many chunks should we send?
	nrecords = n_bytes / SSL3_RT_MAX_PLAIN_LENGTH;
	nrecords += (n_bytes % SSL3_RT_MAX_PLAIN_LENGTH > 0 ? 1 : 0);

	for (raw_sent = i = 0; i < nrecords; i++)
	{
		int encoded;
		int res;

		// encode one data record
		if (i == (nrecords - 1))
		{
			if ( (encoded = ssl_write(m_ssl, pbuf + SSL3_RT_MAX_PLAIN_LENGTH * i, n_bytes - SSL3_RT_MAX_PLAIN_LENGTH * i)) < 0)
				return -1;
		}
		else
		{
			if ((encoded = ssl_write(m_ssl, pbuf + SSL3_RT_MAX_PLAIN_LENGTH * i, SSL3_RT_MAX_PLAIN_LENGTH)) < 0)
				return -1;
		}

		// send one data record over network
		{
			to_send = BIO_ctrl_pending(m_bio_out);
			if (to_send == 0)
			{
				continue;
			}

			// buffer for encoded data
			std::vector<uint8_t> sendbuf(to_send);

			readen = BIO_read(m_bio_out, &(sendbuf[0]), to_send);
			if (readen < 0)
			{
				break;
			}

			res = SendData(&(sendbuf[0]), to_send, mills, keep_blocked);

			if (res < 0)
			{
				break;
			}

			raw_sent += res;
		}
	}

	// How many data have we sent?
	if (i == nrecords)
		sent = n_bytes;
	else
		sent = i * SSL3_RT_MAX_PLAIN_LENGTH;


	return sent;
}

int VS_ConnectionTLS::Send(const void *buffer, const unsigned long n_bytes, unsigned long &mills, const bool keep_blocked)
{
	return VS_ConnectionTLS::Send(buffer, n_bytes, &mills, &keep_blocked);
}

int VS_ConnectionTLS::Send(const void *buffer, const unsigned long n_bytes)
{
	return VS_ConnectionTLS::Send(buffer, n_bytes, nullptr, nullptr);
}

int VS_ConnectionTLS::Receive(void* buffer, const unsigned long n_bytes, unsigned long *mills)
{
	if(SSL_get_shutdown(m_ssl))
		return -1;

	if (n_bytes == 0 || buffer == NULL)
		return 0;

	int readen; // how many data decoded
	int received; // how many data received
	int pending = BIO_ctrl_pending(m_bio_in);

	// two cases - something is left in BIO or not

	// unlikely...
	if (pending > 0)
	{
		readen = ssl_read(m_ssl, buffer, n_bytes);

		if (readen < 0)
			return -1;
		else if (readen > 0)
			return readen;
	}

	// buffer to receive data from remote side
	std::vector<uint8_t> receive_buf(TLS_IOBUF_LEN);
	uint8_t *pbuf = &receive_buf[0];
	uint8_t *ubuf = static_cast<uint8_t *>(buffer); // pointer to the buffer specified by user

	readen = received = 0;

	// receive and decode loop
	for (;;)
	{
		if (readen > 0)
			return readen;
		// get data from network
		int res = ReceiveData(pbuf, receive_buf.size(), mills);

		if (res == 0)
			break;
		else if (res > 0)
			received += res;
		else if (res < 0)
		{
			if(readen > 0)
				return readen;

			return -1;
		}

		// feed received data to SSL
		int written = BIO_write(m_bio_in, pbuf, res);

		// error
		if (written < 0)
		{
			if (readen > 0)
				return readen;

			return -1;
		}

		// SSL decoding loop - it can't decode more than 16 KB once
		for (;;)
		{
			// decode data
			int decoded = ssl_read(m_ssl, ubuf + readen, n_bytes - readen);

			// error during decoding
			if (decoded < 0)
			{
				int ecode = SSL_get_error(m_ssl, decoded);
				if (ecode == SSL_ERROR_WANT_READ)
				{
					if (readen > 0)
						return readen;
					// receive more data
					break;
				}
				else
				{
					return readen;
				}
			}
			// how much data we are already have read
			readen += decoded;

			// we filled the user buffer
			if (readen == n_bytes)
				return readen;

			// check if any encoded data left in input BIO
			if (BIO_ctrl_pending(m_bio_in) == 0)
				break;
		}
	}

	return readen;
}

int VS_ConnectionTLS::Receive(void *buffer, const unsigned long n_bytes, unsigned long &mills, bool portion)
{
	return VS_ConnectionTLS::Receive(buffer, n_bytes, &mills);
}

int VS_ConnectionTLS::Receive(void *buffer, const unsigned long n_bytes)
{
	return VS_ConnectionTLS::Receive(buffer, n_bytes, nullptr);
}

bool VS_ConnectionTLS::SetIOThread(const boost::shared_ptr<VS_WorkThread> &io_thread)
{
	if(!m_thread)
	{
		if (!io_thread->SetHandledConnection(this))
			return false;
		m_thread = io_thread;
		return true;
	}
	if(m_thread.get() == io_thread.get())
		return true;
	m_delegate_thread = io_thread;
	m_delegate_read_handler = VS_ConnectionTCP::GetReadOv().io_handler;
	m_delegate_write_handler = VS_ConnectionTCP::GetWriteOv().io_handler;
	this->SetIOHandler(this);
	return true;
}

void VS_ConnectionTLS::Handle(const unsigned long sz, const struct VS_Overlapped *ov)
{
	if (m_thread->IsCurrent())
	{
		m_delegate_thread->Handle(sz, ov);
	}
	else if (m_delegate_thread->IsCurrent())
	{
		if (ov == &(VS_ConnectionTCP::GetWriteOv()))
		{
			m_delegate_write_handler->Handle(sz, ov);
		}
		else if (ov == &(VS_ConnectionTCP::GetReadOv()))
		{
			m_delegate_read_handler->Handle(sz, ov);
		}
	}

}

void VS_ConnectionTLS::HandleError(const unsigned long err, const struct VS_Overlapped *ov)
{
	if (m_thread->IsCurrent())
	{
		m_delegate_thread->HandleError(err, ov);
	}
	else if (m_delegate_thread->IsCurrent())
	{
		if (ov == &(VS_ConnectionTCP::GetWriteOv()))
		{
			m_delegate_write_handler->HandleError(err, ov);
		}
		else if (ov == &(VS_ConnectionTCP::GetReadOv()))
		{
			m_delegate_read_handler->HandleError(err, ov);
		}
	}
}

bool VS_ConnectionTLS::Connect(const char *host, const unsigned short port,
							   unsigned long &mills,
							   bool isFastSocket , const bool qos, _QualityOfService *qos_params)
{
	if (!VS_ConnectionTCP::Connect(host, port, mills, isFastSocket, qos, qos_params))
		return false;

	return Handshake(0, 0, m_cb, mills, false);
}

bool VS_ConnectionTLS::Connect(const unsigned long ip, const unsigned short port,
							   unsigned long &mills,
							   bool isFastSocket , const bool qos, _QualityOfService * qos_params)
{
	if(!VS_ConnectionTCP::Connect( ip, port, mills, isFastSocket , qos , qos_params))
		return false;

	return Handshake(0, 0, m_cb, mills, false);
}

bool VS_ConnectionTLS::Accept( const char *host, const unsigned short port, unsigned long &mills,
							  const bool exclusiveUseAddr,bool isFastSocket, bool qos, _QualityOfService * qos_params )
{
	if (!VS_ConnectionTCP::Accept(host, port, mills, exclusiveUseAddr, isFastSocket, qos, qos_params))
		return false;
	return Handshake(0, 0, m_cb, mills);
}

int VS_ConnectionTLS::Accept( VS_ConnectionTCP *listener, unsigned long &mills,
							 bool isFastSocket, bool qos, _QualityOfService * qos_params )
{
	int accept_res = 0;
	if ((accept_res = VS_ConnectionTCP::Accept(listener, mills, isFastSocket, qos, qos_params)) <= 0)
	{
		if (m_cb)
			m_cb(false, this, NULL, 0);
		return accept_res;
	}

	return Handshake(0, 0, m_cb, mills) == true ? accept_res : -1;
}

// async Accept()
bool VS_ConnectionTLS::Accept( VS_ConnectionTCP *listener,
							  bool isFastSocket, bool qos, _QualityOfService * qos_params )
{
	if(!VS_ConnectionTCP::Accept( listener, isFastSocket , qos, qos_params))
		return false;

	return true;
}

bool VS_ConnectionTLS::IsAccept(void) const
{
	return VS_ConnectionTCP::IsAccept() && m_was_handshake;
}

void VS_ConnectionTLS::RemoveCallback(void)
{
	m_cb.clear();
}

const char *VS_ConnectionTLS::GetStateString() const
{
	return SSL_state_string(m_ssl);
}

const char *VS_ConnectionTLS::GetStateStringLong() const
{
	return SSL_state_string_long(m_ssl);
}

void VS_ConnectionTLS::SetVerifyMode(int mode)
{
	switch (mode)
	{
	case VERIFY_NONE:
		SSL_CTX_set_verify(m_ctx, SSL_VERIFY_NONE, verify_cb);
		break;
	case VERIFY_PEER:
		SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER, verify_cb);
		break;
	}
}

bool VS_ConnectionTLS::DeriveKey(uint8_t *out, size_t out_len, const char *label, size_t label_len, const uint8_t *context, size_t context_len)
{
	if (!m_ssl || SSL_get_shutdown(m_ssl))
		return false;
	return 1 == SSL_export_keying_material(m_ssl, out, out_len, label, label_len, context, context_len, 1);
}

void VS_ConnectionTLS::Close(void)
{
	// remove callback to avoid possible crash on uncompleted handshake
	//RemoveCallback();
	VS_ConnectionTCP::Close();
}
