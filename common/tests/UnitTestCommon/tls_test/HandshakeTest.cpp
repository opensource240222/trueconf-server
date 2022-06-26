#include <gtest/gtest.h>
#include "net/tls/Handshaker.h"
#include "net/tls/Encryptor.h"
#include "net/tls/Utils.h"
#include "SecureLib/OpenSSLCompat/tc_ssl.h"
#include "srv_cert.h"
#include "srv_key.h"
#include "user_cert.h"
#include "user_key.h"
#include "SecureLib/VS_SecureConstants.h"
#include "std-generic/cpplib/VS_Container.h"

// TLS 1.3 first appeared in OPENSSL 1.1.1
constexpr auto tls_1_3_not_available = OPENSSL_VERSION_NUMBER < 0x10101000L;

const static size_t BUF_SIZE = 1024 * 4;

/* SSL callbacks */
static void msg_callback(int /*write_p*/, int /*version*/, int content_type, const void *buf, size_t /*len*/, SSL *ssl, void */*arg*/)
{
	if (content_type == 21) //error
	{
		if (*(static_cast<const char*>(buf)) == 0x02) //fatal
			SSL_shutdown(ssl);
	}
}


static int verify_cb(int preverifyOk, X509_STORE_CTX* /*ctx*/)
{
	return preverifyOk;
}

struct TLSParticipant
{
	BIO *m_in, *m_out;
	SSL_CTX * m_ctx;
	SSL *m_ssl;
	std::unique_ptr<ssl::Handshaker> hshaker;
	std::unique_ptr<ssl::Encryptor> encryptor;
	std::unique_ptr<ssl::Decryptor> decryptor;
	unsigned char m_buf[BUF_SIZE] = {};
	bool m_inited = false;

	explicit TLSParticipant(bool isServer)
		: m_in(BIO_new(BIO_s_mem()))
		, m_out(BIO_new(BIO_s_mem()))
		, m_ctx(SSL_CTX_new(TLS_method()))
		, m_ssl(nullptr)
		, hshaker(nullptr)
	{

		BIO_ctrl(m_in, BIO_C_SET_BUF_MEM_EOF_RETURN, EOF, NULL);
		BIO_ctrl(m_out, BIO_C_SET_BUF_MEM_EOF_RETURN, EOF, NULL);
		SSL_CTX_ctrl(m_ctx, SSL_CTRL_MODE, SSL_OP_ALL | SSL_OP_NO_SSLv3 | SSL_OP_SINGLE_DH_USE, NULL);
		SSL_CTX_ctrl(m_ctx, SSL_CTRL_MODE, SSL_MODE_ENABLE_PARTIAL_WRITE, NULL);
		SSL_CTX_ctrl(m_ctx, SSL_CTRL_MODE, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER, NULL);

		SSL_CTX_set_msg_callback(m_ctx, &msg_callback);
		SSL_CTX_set_verify(m_ctx, SSL_VERIFY_NONE, verify_cb);

		if (isServer) {
			m_inited = ssl::utils::AddCertChainToCTX(m_ctx, server_cert_data, sizeof(server_cert_data))
			&& ssl::utils::AddPrivateKeyToCTX(m_ctx, server_key_data, sizeof(server_key_data), "");
		}
		else {
			m_inited = ssl::utils::AddCertChainToCTX(m_ctx, client_cert_data, sizeof(client_cert_data))
			&&  ssl::utils::AddPrivateKeyToCTX(m_ctx, client_key_data, sizeof(client_key_data), "");
		}

		VS_GET_PEM_CACERT
		m_inited = m_inited && ssl::utils::AddCertToCTX(m_ctx, PEM_CACERT, strlen(PEM_CACERT) + 1);

		m_ssl = SSL_new(m_ctx);
		SSL_set_bio(m_ssl, m_in, m_out);
		hshaker = std::make_unique<ssl::Handshaker>(m_in, m_out, m_ssl, isServer);
		encryptor = std::make_unique<ssl::Encryptor>(m_out, m_ssl);
		decryptor = std::make_unique<ssl::Decryptor>(m_in, m_ssl);
	}

	~TLSParticipant() {
		SSL_shutdown(m_ssl);
		SSL_free(m_ssl);	// it will call  BIO_free(m_in) and BIO_free(m_out)
		SSL_CTX_free(m_ctx);
	}

	bool GetDataToSend(std::vector<uint8_t> &data) {
		data.clear();
		bool res = false;
		for (int toSend; (toSend = BIO_read(m_out, m_buf, BUF_SIZE)) > 0;) {
			res = true;
			copy(m_buf, m_buf + toSend, std::back_inserter(data));
		}
		return res;
	}

};

static void tls_1_2_handshake_test(TLSParticipant& server, TLSParticipant& client)
{
	
		// 1. Client                                               Server
		//	  ClientHello			-------- >
		std::vector<uint8_t> clientSndBuff;
		EXPECT_EQ(client.hshaker->DoHandshake(0, 0), ssl::HandshakeStatus::hs_ongoing);
		client.GetDataToSend(clientSndBuff);
		EXPECT_GT(clientSndBuff.size(), 0u);

		// 2. Client                                               Server
		//                                                    ServerHello
		//                                                    Certificate
		//                                              ServerKeyExchange
		//                                             CertificateRequest
		//                               <--------        ServerHelloDone
		std::vector<uint8_t> serverSndBuff;
		EXPECT_EQ(server.hshaker->DoHandshake(0, 0), ssl::HandshakeStatus::hs_ongoing);
		EXPECT_EQ(server.hshaker->DoHandshake(clientSndBuff.data(), clientSndBuff.size()), ssl::HandshakeStatus::hs_ongoing);
		EXPECT_GT(server.hshaker->PendingBytes(), 0u);
		server.GetDataToSend(serverSndBuff);
		EXPECT_GT(serverSndBuff.size(), 0u);
		EXPECT_EQ(server.hshaker->PendingBytes(), 0u);

		// 3. Client                                               Server
		//    Certificate
		//    ClientKeyExchange
		//    CertificateVerify
		//    [ChangeCipherSpec]
		//    Finished                     -------->
		EXPECT_EQ(client.hshaker->DoHandshake(serverSndBuff.data(), serverSndBuff.size()), ssl::HandshakeStatus::hs_ongoing);
		clientSndBuff.clear();
		EXPECT_GT(client.hshaker->PendingBytes(), 0u);
		client.GetDataToSend(clientSndBuff);
		EXPECT_GT(clientSndBuff.size(), 0u);

		// 4. Client                                               Server
		//                                             [ChangeCipherSpec]
		//                             <--------                 Finished
		EXPECT_EQ(server.hshaker->DoHandshake(clientSndBuff.data(), clientSndBuff.size()), ssl::HandshakeStatus::hs_success);
		serverSndBuff.clear();
		EXPECT_GT(server.hshaker->PendingBytes(), 0u);
		server.GetDataToSend(serverSndBuff);
		EXPECT_GT(serverSndBuff.size(), 0u);
		EXPECT_EQ(client.hshaker->DoHandshake(serverSndBuff.data(), serverSndBuff.size()), ssl::HandshakeStatus::hs_success);
}


static void tls_1_3_handshake_test(TLSParticipant& server, TLSParticipant& client)
{
		// 1.
		// Client                                           Server

		// Key  ^ ClientHello
		// Exch | + key_share*
		//      | + signature_algorithms*
		//      | + psk_key_exchange_modes*
		//      v + pre_shared_key*       -------->
		std::vector<uint8_t> clientSndBuff;
		EXPECT_EQ(client.hshaker->DoHandshake(0, 0), ssl::HandshakeStatus::hs_ongoing);
		client.GetDataToSend(clientSndBuff);
		EXPECT_GT(clientSndBuff.size(), 0u);

		// 2.
		// Client                                           Server
		//                                                   ServerHello  ^ Key
		//                                                  + key_share*  | Exch
		//                                             + pre_shared_key*  v
		//                                         {EncryptedExtensions}  ^  Server
		//                                         {CertificateRequest*}  v  Params
		//                                                {Certificate*}  ^
		//                                          {CertificateVerify*}  | Auth
		//                                <--------           {Finished}  v
		std::vector<uint8_t> serverSndBuff;
		EXPECT_EQ(server.hshaker->DoHandshake(0, 0), ssl::HandshakeStatus::hs_ongoing);
		EXPECT_EQ(server.hshaker->DoHandshake(clientSndBuff.data(), clientSndBuff.size()), ssl::HandshakeStatus::hs_ongoing);
		EXPECT_GT(server.hshaker->PendingBytes(), 0u);
		server.GetDataToSend(serverSndBuff);
		EXPECT_GT(serverSndBuff.size(), 0u);
		EXPECT_EQ(server.hshaker->PendingBytes(), 0u);

		// 3.
		// Client                                           Server
		//      ^ {Certificate*}
		// Auth | {CertificateVerify*}
		//      v {Finished}              -------->
		EXPECT_EQ(client.hshaker->DoHandshake(serverSndBuff.data(), serverSndBuff.size()), ssl::HandshakeStatus::hs_success);
		clientSndBuff.clear();
		EXPECT_GT(client.hshaker->PendingBytes(), 0u);
		client.GetDataToSend(clientSndBuff);
		EXPECT_GT(clientSndBuff.size(), 0u);

		// Session tickets are enabled by default in OpenSSL 1.1.1

		// 4.
		// Client                                           Server
		//                                    <--------      [NewSessionTicket]
		EXPECT_EQ(server.hshaker->DoHandshake(clientSndBuff.data(), clientSndBuff.size()), ssl::HandshakeStatus::hs_success);
		serverSndBuff.clear();
		EXPECT_GT(server.hshaker->PendingBytes(), 0u);
		server.GetDataToSend(serverSndBuff);
		EXPECT_GT(serverSndBuff.size(), 0u);
		EXPECT_EQ(client.hshaker->DoHandshake(serverSndBuff.data(), serverSndBuff.size()), ssl::HandshakeStatus::hs_success);
}

static void tls_handshake_test(TLSParticipant& server, TLSParticipant& client)
{
	(tls_1_3_not_available ? tls_1_2_handshake_test : tls_1_3_handshake_test)(server, client);
}

TEST(Handshaker, DoHandshake) {
	TLSParticipant client(false);
	TLSParticipant server(true);
	ASSERT_TRUE(client.m_inited);
	ASSERT_TRUE(server.m_inited);

	tls_handshake_test(server, client);

	EXPECT_EQ(client.hshaker->PendingBytes(), 0u);
	EXPECT_EQ(server.hshaker->PendingBytes(), 0u);
}

TEST(TLS, Encryption) {
	TLSParticipant client(false);
	TLSParticipant server(true);

	tls_handshake_test(server, client);

	// encrypt/decrypt part
	const std::vector<uint8_t> testData = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
	std::vector<uint8_t> encrypted;
	std::vector<uint8_t> decrypted;

	EXPECT_TRUE(server.encryptor->Encrypt(testData.data(), testData.size(), encrypted));
	EXPECT_TRUE(client.decryptor->Decrypt(encrypted.data(), encrypted.size(), decrypted));
	EXPECT_EQ(testData, decrypted);

	decrypted.clear();
	encrypted.clear();
	EXPECT_TRUE(client.encryptor->Encrypt(testData.data(), testData.size(), encrypted));
	EXPECT_GT(server.decryptor->Decrypt(encrypted.data(), encrypted.size(), decrypted),0);
	EXPECT_EQ(testData, decrypted);
	decrypted.clear();

	// partial decrypt
	EXPECT_TRUE(client.encryptor->Encrypt(testData.data(), testData.size(), encrypted));
	for (size_t i = 0; i < encrypted.size() - 1; ++i){
		EXPECT_EQ(server.decryptor->Decrypt(encrypted.data() + i, 1, decrypted), -2);	// feed by one byte
		EXPECT_TRUE(decrypted.empty());
	}

	EXPECT_GT(server.decryptor->Decrypt(encrypted.data() + encrypted.size() - 1, 1, decrypted), 0);	// decrypt last byte
	EXPECT_EQ(testData, decrypted);


	// squashed decrypt
	decrypted.clear();
	encrypted.clear();
	std::vector<uint8_t> encryptedPart;
	EXPECT_TRUE(server.encryptor->Encrypt(testData.data(), testData.size(), encrypted));
	EXPECT_TRUE(server.encryptor->Encrypt(testData.data(), testData.size(), encryptedPart));
	encrypted.insert(encrypted.end(), encryptedPart.begin(), encryptedPart.end());
	EXPECT_EQ(client.decryptor->Decrypt(encrypted.data(), 1, decrypted), -2);
	EXPECT_TRUE(client.decryptor->Decrypt(encrypted.data() + 1, encrypted.size() - 1, decrypted));
	EXPECT_EQ(testData, decrypted);
	decrypted.clear();

	int pend = client.decryptor->Pending();
	EXPECT_TRUE(client.decryptor->Decrypt(nullptr, 0, decrypted));
	EXPECT_EQ(testData, decrypted);
}