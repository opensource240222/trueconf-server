#include "Encryptor.h"
#include "Utils.h"

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_OTHER

static const size_t BUFSZ = 128 * 1024; // 128 Kb

ssl::Encryptor::Encryptor(BIO * out, SSL * ssl)
	: m_out(out)
	, m_ssl(ssl)
{
	assert(m_out != nullptr);
	assert(m_ssl != nullptr);
}

bool ssl::Encryptor::Encrypt(const void * in, size_t size, acs::Handler::stream_buffer & out)
{
	if (in == nullptr || size == 0 ||  SSL_get_shutdown(m_ssl))
		return false;

	int writeResult(0);
	int totalWritten(0);
	while (0 < (writeResult = SSL_write(m_ssl,
			reinterpret_cast<const unsigned char*>(in) + totalWritten,
			static_cast<int>(size) - totalWritten)))
		totalWritten += writeResult;
	if (totalWritten <= 0)
		return false;

	// create buffer of apropriate size
	int pending = BIO_ctrl_pending(m_out);
	if (pending <= 0)
		return false;
	out.resize(pending);

	// get encrypted data into out buffer
	int read, wbsize = 0;
	while ((read = BIO_read(m_out, out.data() + wbsize, pending - wbsize)) > 0)
	{
		wbsize += read;
		if (wbsize >= pending)
			break;
	}

	return true;
}

ssl::Decryptor::Decryptor(BIO * in, SSL * ssl)
	: m_in(in)
	, m_ssl(ssl)
{
	assert(m_in != nullptr);
	assert(m_ssl != nullptr);
}

int ssl::Decryptor::Decrypt(const void * in, size_t size, acs::Handler::stream_buffer & out)
{
	if (in != nullptr && size > 0)
	{
		//	Fill BIO with received data
		int written = BIO_write(m_in, in, size);
		assert(written == static_cast<int>(size));
		if (written <= 0)
			return -1;
	}

	int readResult = SSL_read(m_ssl, nullptr, 0);
	int pending = SSL_pending(m_ssl);
	out.resize(pending);

	readResult = SSL_read(m_ssl, out.data(), pending);
	if (readResult <= 0) {
		out.clear();
		if (SSL_ERROR_WANT_READ == SSL_get_error(m_ssl, readResult))
			return -2; // Request more data
		dstream4 << "ssl::Decryptor failed to decrypt data! " << utils::GetOpenSSLErrorStack();
		return -1;
	}

	return readResult;
}

int ssl::Decryptor::Pending() const
{
	assert(m_ssl != nullptr);
	int readResult = SSL_read(m_ssl, nullptr, 0);
	return  SSL_pending(m_ssl);

}
