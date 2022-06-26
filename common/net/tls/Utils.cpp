#include "Utils.h"
#include "std-generic/cpplib/scope_exit.h"
#include <openssl/err.h>

bool ssl::utils::AddCertToCTX(SSL_CTX * ctx, const void * cert, const unsigned int size)
{
	BIO* temp_bio = BIO_new_mem_buf((void*)cert, size);
	X509_STORE *store = SSL_CTX_get_cert_store(ctx);
	bool done = false;
	if (temp_bio)
	{
		X509* crt = PEM_read_bio_X509(temp_bio, 0, 0, 0);
		if (crt)
		{
			done = (1 == X509_STORE_add_cert(store, crt));
		}
		X509_free(crt);
		BIO_free_all(temp_bio);
	}
	return done;
}

bool ssl::utils::AddCertChainToCTX(SSL_CTX * ctx, const void * cert, const unsigned int size)
{
	BIO *tmp_bio = NULL;
	STACK_OF(X509_INFO) *chain_stack = NULL;
	size_t certs_count = 0;
	X509_INFO *ci = NULL;

	VS_SCOPE_EXIT{
		if (chain_stack) {
			while ((ci = static_cast<X509_INFO *>(sk_pop(reinterpret_cast<_STACK *>(chain_stack)))) != NULL)
			{
				X509_INFO_free(ci);
			}
			sk_X509_INFO_free(chain_stack);
		}
		if(tmp_bio)
			BIO_free_all(tmp_bio);
	};

	tmp_bio = BIO_new_mem_buf(cert, size);
	if (tmp_bio == NULL)
		return false;

	// read info into BIO
	chain_stack = PEM_X509_INFO_read_bio(tmp_bio, NULL, NULL, NULL);
	if (chain_stack == NULL)
		return false;

	certs_count = sk_num(reinterpret_cast<_STACK*>(chain_stack)); // sk_X509_INFO_num(chain_stack)
	// add all certificates
	for (size_t i = certs_count; i > 0; i--)
	{
		// get certificate from chain
		ci = static_cast<X509_INFO *>(sk_value(reinterpret_cast<_STACK*>(chain_stack), i - 1)); // sk_X509_INFO_value()
		if (ci == NULL)
			return false;

		// add certificate to the chain
		if (SSL_CTX_ctrl(ctx, SSL_CTRL_EXTRA_CHAIN_CERT, 0, X509_dup(ci->x509)) != 1) //SSL_CTX_add_extra_chain_cert()
			return false;

		// use first ceritifcate in chain by default
		if (i == 1)
		{
			if (SSL_CTX_use_certificate(ctx, ci->x509) != 1)
				return false;
		}
	}
	return true;
}

bool ssl::utils::AddPrivateKeyToCTX(SSL_CTX * ctx, const void * key, const unsigned int size, const char * pass)
{
	BIO* temp_bio = BIO_new_mem_buf(key, size);
	bool done = false;
	if (temp_bio)
	{

		RSA* rsa = PEM_read_bio_RSAPrivateKey(temp_bio, 0, 0, reinterpret_cast<void*>(const_cast<char*>(pass)));
		if (rsa){
			done = (1 == SSL_CTX_use_RSAPrivateKey(ctx, rsa));
		}
		RSA_free(rsa);
		BIO_free_all(temp_bio);
	}

	// check if private key is valid
	done = SSL_CTX_check_private_key(ctx) == 1;

	return done;
}

std::string ssl::utils::GetOpenSSLErrorStack()
{
	BIO* bio = BIO_new(BIO_s_mem());
	VS_SCOPE_EXIT{ BIO_free(bio); };	// this gets rid of tempBuf as well

	ERR_print_errors(bio);
	char* tempBuf = nullptr;
	size_t length = BIO_get_mem_data(bio, &tempBuf);
	if (length == 0)
		return {};
	return std::string(tempBuf, tempBuf + length);
}
