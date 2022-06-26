#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "std-generic/compat/memory.h"
#include <fstream>
#include <vector>
#include <thread>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#endif


#include "UnitTestConstants.h"

#include "std-generic/cpplib/VS_Container.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/event.h"
#include "std-generic/cpplib/scope_exit.h"
#include "SecureLib/VS_SecureConstants.h"

#include "SecureLib/VS_CryptoInit.h"
#include "SecureLib/VS_StreamCrypter.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_CertificateIssue.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_Sign.h"
#include "SecureLib/VS_SecureHandshake.h"
#include "SecureLib/VS_SymmetricCrypt.h"

#define BUFSIZE 1024 * 8


void encrypt_file(VS_StreamCrypter& stream_crypter, const std::string& file_in, const std::string& file_out)
{
	unsigned char source_buf[BUFSIZE];
	unsigned char encrypted_buf[2 * BUFSIZE];

	std::ifstream source(file_in, std::ifstream::binary);
	std::ofstream encrypted(file_out, std::ifstream::binary);

	ASSERT_TRUE(source.is_open());
	ASSERT_TRUE(encrypted.is_open());
	for (uint32_t enc_bytes=sizeof(encrypted_buf);!source.eof();enc_bytes=sizeof(encrypted_buf))
	{
		source.read(reinterpret_cast<char*>(source_buf), sizeof(source_buf));
		ASSERT_TRUE(stream_crypter.Encrypt(source_buf, static_cast<uint32_t>(source.gcount()), encrypted_buf, &enc_bytes));
		encrypted.write(reinterpret_cast<char*>(encrypted_buf), enc_bytes);
	}
}

void decrypt_file(VS_StreamCrypter& stream_crypter, const std::string& file_in, const std::string& file_out)
{
	unsigned char decrypted_buf[2*BUFSIZE];
	unsigned char encrypted_buf[BUFSIZE+16];

	std::ifstream encrypted(file_in, std::ifstream::binary);
	std::ofstream decrypted(file_out, std::ifstream::binary);

	ASSERT_TRUE(encrypted.is_open());
	ASSERT_TRUE(decrypted.is_open());

	for (uint32_t dec_bytes = sizeof(decrypted_buf); !encrypted.eof(); dec_bytes = sizeof(decrypted_buf))
	{
		encrypted.read(reinterpret_cast<char*>(encrypted_buf), sizeof(encrypted_buf));
		ASSERT_TRUE(stream_crypter.Decrypt(encrypted_buf, static_cast<uint32_t>(encrypted.gcount()), decrypted_buf, &dec_bytes));
		decrypted.write(reinterpret_cast<char*>(decrypted_buf), dec_bytes);
	}
}

void compare_files(const std::string& file_left, const std::string& file_right)
{
	std::ifstream left(file_left, std::ifstream::binary);
	std::ifstream right(file_right, std::ifstream::binary);

	ASSERT_TRUE(left.is_open());
	ASSERT_TRUE(right.is_open());

	char buf_left[8192];
	char buf_right[8192];

	for (; !left.eof() && !right.eof();)
	{
		left.read(buf_left, sizeof(buf_left));
		right.read(buf_right, sizeof(buf_right));

		ASSERT_EQ(left.gcount(), right.gcount());
		ASSERT_EQ(memcmp(buf_left, buf_right, static_cast<uint32_t>(left.gcount())), 0);
	}
	ASSERT_TRUE(left.eof());
	ASSERT_TRUE(right.eof());
}
class SecureLibFixture: public ::testing::Test
{
public:
};

TEST_F(SecureLibFixture, DISABLED_AES128) {

	VS_StreamCrypter	stream_crypter;
	unsigned char key[] = "abcdefghijklmno"; //16bit key
	ASSERT_TRUE(stream_crypter.Init(key, sizeof(key)));
	ASSERT_TRUE(stream_crypter.IsValid());

	std::string self_name;// = VS_CryptocomLibHandler::MakePathToLib(nullptr);

	encrypt_file(stream_crypter, self_name, self_name+".encrypted");
	decrypt_file(stream_crypter, self_name + ".encrypted", self_name + ".decrypted");
	compare_files(self_name, self_name + ".decrypted");
}

TEST_F(SecureLibFixture, DISABLED_AES256) {

	VS_StreamCrypter	stream_crypter;
	unsigned char key[] = "abcdefghijklmnosdfsdfsdffffffff"; //32bit key
	ASSERT_TRUE(stream_crypter.Init(key, sizeof(key)));
	ASSERT_TRUE(stream_crypter.IsValid());

	std::string self_name;// = VS_CryptocomLibHandler::MakePathToLib(nullptr);

	encrypt_file(stream_crypter, self_name, self_name + ".encrypted");
	decrypt_file(stream_crypter, self_name + ".encrypted", self_name + ".decrypted");
	compare_files(self_name, self_name + ".decrypted");
}

TEST_F(SecureLibFixture, CertificateRequest) {

	std::unique_ptr<VS_PKey> key ( new VS_PKey );
	key->GenerateKeys(2048, alg_pk_RSA);
	uint32_t private_key_length;
	ASSERT_TRUE(key->WritePrivateKey((char*)constants::new_cert_private_key_filename, private_key_length, store_PEM_FILE, alg_sym_AES128, mode_CBC, "password", strlen("password")));
	std::unique_ptr<VS_CertificateRequest> cert_req(new VS_CertificateRequest);
	ASSERT_TRUE(cert_req->SetPKeys(key.get(), key.get()));
	ASSERT_TRUE(cert_req->SetEntry("countryName", "AU"));
	ASSERT_TRUE(cert_req->SetEntry("stateOrProvinceName", "Some State"));
	ASSERT_TRUE(cert_req->SetEntry("organizationName", "Some Company"));
	ASSERT_TRUE(cert_req->SetEntry("organizationalUnitName", "Some Unit"));
	ASSERT_TRUE(cert_req->SetEntry("emailAddress", "johnsmith@company.au"));
	ASSERT_TRUE(cert_req->SetEntry("commonName", "Common Name"));
	ASSERT_TRUE(cert_req->SetEntry("nsCertType", "server"));
	ASSERT_TRUE(cert_req->SetEntry("surname", "Smith"));
	ASSERT_TRUE(cert_req->SetExtension("subjectAltName", "DNS:splat.zork.org"));
	ASSERT_TRUE(cert_req->SignRequest());
	uint32_t buffsize = 0;
	ASSERT_FALSE(cert_req->SaveTo(nullptr, buffsize, store_PEM_BUF));
	std::vector<char> buff(buffsize);
	ASSERT_TRUE(cert_req->SaveTo(buff.data(), buffsize, store_PEM_BUF));
	ASSERT_TRUE(cert_req->SaveTo((char*)"request.pem", buffsize, store_PEM_FILE));

}

TEST_F(SecureLibFixture, CertificateIssue) {

	auto private_key = vs::make_unique<VS_PKey>();
	ASSERT_TRUE(private_key->SetPrivateKey(constants::server_private_key, store_PEM_BUF));
	auto cert_auth = vs::make_unique<VS_CertAuthority>();
	ASSERT_TRUE(cert_auth->SetCACertificate(constants::server_cert, sizeof(constants::server_cert), store_PEM_BUF));
	ASSERT_TRUE(cert_auth->SetCAPrivateKey(private_key.get()));
	ASSERT_TRUE(cert_auth->SetCertRequest("request.pem", sizeof("request.pem"), store_PEM_FILE));
	ASSERT_TRUE(cert_auth->VerifyRequestSign());
	ASSERT_TRUE(cert_auth->SetExpirationTime(60 * 60 * 24 * 365));
	ASSERT_TRUE(cert_auth->SetExtension("subjectAltName", "DNS:splat.zork.org"));
	ASSERT_TRUE(cert_auth->SetSerialNumber(1));
	uint32_t filename_size = sizeof(constants::new_cert_filename);
	ASSERT_TRUE(cert_auth->IssueCertificate((char*)constants::new_cert_filename, filename_size, store_PEM_FILE));
}



TEST_F(SecureLibFixture, CertificateCheck) {
	VS_GET_PEM_CACERT
	VS_CertificateCheck cert_check;
	ASSERT_TRUE(cert_check.SetCert(constants::server_cert, sizeof(constants::server_cert), store_PEM_BUF));
	ASSERT_TRUE(cert_check.SetCertToChain(constants::server_cert_chain, sizeof(constants::server_cert_chain), store_PEM_BUF));
	ASSERT_TRUE(cert_check.SetCertToChain(PEM_CACERT, strlen(PEM_CACERT) + 1, store_PEM_BUF));
	int err;
	ASSERT_TRUE(cert_check.VerifyCert(&err));
	ASSERT_EQ(err, 0);
}

std::vector<unsigned char> GetExecutableInBuffer()//DELET THIS
{
	std::vector<unsigned char> buf;
/*
	std::ifstream self(VS_CryptocomLibHandler::MakePathToLib(nullptr), std::ifstream::binary);
	for (; !self.eof(); buf.push_back(self.get()) ) ;
*/
	return buf;
}

TEST_F(SecureLibFixture, DISABLED_SignCheck) {
	VS_Certificate cert;
	ASSERT_TRUE(cert.SetCert(constants::new_cert_filename, sizeof(constants::new_cert_filename), store_PEM_FILE));
	VS_PKey public_key;
	ASSERT_TRUE(cert.GetCertPublicKey(&public_key));
	uint32_t buffer_size = 0;
	ASSERT_FALSE(public_key.GetPublicKey(store_PEM_BUF, 0, &buffer_size));
	std::vector<char> public_key_content(buffer_size);
	ASSERT_TRUE(public_key.GetPublicKey(store_PEM_BUF, public_key_content.data(), &buffer_size));
	VS_SignArg sign_arg = { alg_pk_RSA, alg_hsh_SHA1 };
	VS_Sign sign, verify_sign;
	ASSERT_TRUE(sign.Init(sign_arg));
	ASSERT_TRUE(sign.SetPrivateKey(constants::new_cert_private_key_filename, store_PEM_FILE, "password"));
	buffer_size = 0;
	auto buffer = GetExecutableInBuffer();
	ASSERT_FALSE(sign.SignData(buffer.data(), buffer.size(), 0, &buffer_size));
	std::vector<unsigned char> signature(buffer_size);
	ASSERT_TRUE(sign.SignData(buffer.data(), buffer.size(), signature.data(), &buffer_size));
	ASSERT_TRUE(verify_sign.Init(sign_arg));
	ASSERT_TRUE(verify_sign.SetPublicKey(public_key_content.data(), public_key_content.size(), store_PEM_BUF));
	ASSERT_TRUE(verify_sign.VerifySign(buffer.data(), buffer.size(), signature.data(), signature.size()));
}

#define SERVER_PORT 31000
static const char test_string[] = "Test string";

class Server
{
public:
	Server()
	{
		initialize();
	}
	~Server() {
		#ifdef _WIN32
			::closesocket(m_sock);
			::closesocket(m_server_sock);
		#else
			close(m_sock);
			close(m_server_sock);
		#endif

	}
	void initialize()
	{
		ASSERT_NE(m_server_sock = ::socket(AF_INET, SOCK_STREAM, 0), -1);
		struct sockaddr_in addr_in;
		addr_in.sin_family = AF_INET;
		addr_in.sin_port = htons(SERVER_PORT);
		addr_in.sin_addr.s_addr = INADDR_ANY;
		ASSERT_EQ(bind(m_server_sock, (sockaddr*)&addr_in, sizeof(addr_in)), 0);
		ASSERT_EQ(listen(m_server_sock, 8), 0);
		s_started_listen.set();
		struct sockaddr addr;
		socklen_t addrlen = sizeof(addr);
		ASSERT_NE(m_sock = accept(m_server_sock, &addr, &addrlen), -1);
	}

	int send(const char* buf, size_t len)
	{
		return ::send(m_sock, buf, len, 0);
	}
	int recv(char* buf, size_t len)
	{
		return ::recv(m_sock, buf, len, 0);
	}
	static vs::event s_started_listen;
	int m_server_sock;
	int m_sock;
};

vs::event Server::s_started_listen(false,false);

class Client
{
	public:
		Client()
		{
			initialize();
		}

		~Client() {
		#ifdef _WIN32
			::closesocket(m_sock);
		#else
			close(m_sock);
		#endif
		}
		void initialize()
		{
			Server::s_started_listen.wait_for(std::chrono::milliseconds(10000));
			ASSERT_NE(m_sock = ::socket(AF_INET, SOCK_STREAM, 0), -1);
			struct sockaddr_in addr_in;
			addr_in.sin_family = AF_INET;
			addr_in.sin_port = htons(SERVER_PORT);
			ASSERT_EQ(inet_pton(AF_INET, "127.0.0.1", &addr_in.sin_addr), 1);
			ASSERT_EQ(connect(m_sock, (sockaddr*)&addr_in, sizeof(addr_in)), 0);
		}
		int send(const char* buf, size_t len)
		{
			return ::send(m_sock, buf, len, 0);
		}
		int recv(char* buf, size_t len)
		{
			return ::recv(m_sock, buf, len, 0);
		}
		int m_sock;

};

void handshake_server(int version)
{
	vs::SetThreadName("T:HSServer");

	VS_SecureHandshake handshake(version, handshake_type_Server);
	VS_SymmetricCrypt	*read_crypt, *write_crypt;
	VS_SecureHandshakeState state;
	void *buf;
	uint32_t buf_size;
	Server server;

	//setup private key
	auto private_key = vs::make_unique<VS_PKey>();
	ASSERT_TRUE(private_key->SetPrivateKey(constants::server_private_key, store_PEM_BUF));
	ASSERT_TRUE(handshake.SetPrivateKey(private_key.get()));

	state = handshake.Next();
	ASSERT_EQ(state, secure_st_SendCert);

	//send certificate
	VS_Container cnt;
	ASSERT_TRUE(cnt.AddValue(CERTIFICATE_PARAM, constants::server_cert, sizeof(constants::server_cert)));
	ASSERT_TRUE(cnt.AddValue(CERTIFICATE_CHAIN_PARAM, constants::server_cert_chain, sizeof(constants::server_cert_chain)));
	{
		size_t buf_size2 = 0;
		ASSERT_TRUE(cnt.SerializeAlloc(buf, buf_size2));
		VS_SCOPE_EXIT { free(buf); };
		buf_size = buf_size2;
		ASSERT_EQ(server.send((char*)&buf_size, sizeof(buf_size)), (int)sizeof(buf_size));
		ASSERT_EQ(server.send((char*)buf, buf_size), (int)buf_size);
	}
	state = handshake.Next();

	//handshake loop
	for (buf = 0, buf_size = 0;
		 state != secure_st_Error && state != secure_st_Finish;
		 state = handshake.Next())
	{
		ASSERT_TRUE(handshake.PreparePacket(&buf, (uint32_t*)&buf_size));
		VS_SCOPE_EXIT { handshake.FreePacket((void**)&buf); };
		if (state == secure_st_SendPacket || state == secure_st_SendCert)
		{
			ASSERT_EQ(server.send((char*)buf, buf_size), (int)buf_size);
		}
		else
		{
			ASSERT_EQ(server.recv((char*)buf, buf_size), (int)buf_size);
			ASSERT_TRUE(handshake.ProcessPacket(buf, buf_size));
		}
	}

	ASSERT_EQ(state, secure_st_Finish);

	ASSERT_NE(write_crypt = handshake.GetWriteSymmetricCrypt(), nullptr);
	ASSERT_NE(read_crypt = handshake.GetReadSymmetricCrypt(), nullptr);

	//receive and decrypt test string
	uint32_t decrypted_size = 0;
	int recv_size = 0;
	unsigned char recv_buf[512];
	ASSERT_GT((recv_size = server.recv((char*)recv_buf, sizeof(recv_buf))), 0);
	ASSERT_FALSE(read_crypt->Decrypt(recv_buf, recv_size, 0, &decrypted_size));
	std::vector<unsigned char> decrypted_buf(decrypted_size);
	ASSERT_TRUE(read_crypt->Decrypt(recv_buf, recv_size, decrypted_buf.data(), &decrypted_size));
	std::string decrypted((char*)decrypted_buf.data());
	ASSERT_EQ(decrypted, test_string);

	VS_SecureHandshake::ReleaseSymmetricCrypt(&write_crypt);
	VS_SecureHandshake::ReleaseSymmetricCrypt(&read_crypt);
}

void handshake_client(int version)
{
	vs::SetThreadName("T:HSClient");

	VS_SecureHandshake handshake(version, handshake_type_Client);
	VS_SymmetricCrypt	*read_crypt, *write_crypt;
	VS_SecureHandshakeState state;
	void *buf;
	uint32_t buf_size;

	Client client;

	//setup private key
	auto private_key = vs::make_unique<VS_PKey>();
	ASSERT_TRUE(private_key->SetPrivateKey(constants::server_private_key, store_PEM_BUF));
	ASSERT_TRUE(handshake.SetPrivateKey(private_key.get()));

	state = handshake.Next();
	ASSERT_EQ(state, (handshake.GetVersion() == 1 ? secure_st_GetPacket : secure_st_SendCert));
	if (handshake.GetVersion() == 2)
	{
		//send certificate
		VS_Container cnt;
		ASSERT_TRUE(cnt.AddValue(CERTIFICATE_PARAM, constants::server_cert, sizeof(constants::server_cert)));
		ASSERT_TRUE(cnt.AddValue(CERTIFICATE_CHAIN_PARAM, constants::server_cert_chain, sizeof(constants::server_cert_chain)));
		size_t buf_size2 = 0;
		ASSERT_TRUE(cnt.SerializeAlloc(buf, buf_size2));
		VS_SCOPE_EXIT { free(buf); };
		buf_size = buf_size2;
		ASSERT_EQ(client.send((char*)&buf_size, sizeof(buf_size)), (int)sizeof(buf_size));
		ASSERT_EQ(client.send((char*)buf, buf_size), (int)buf_size);
		state = handshake.Next();
	}

	//handshake loop
	for (buf = 0, buf_size = 0;
		state != secure_st_Error && state != secure_st_Finish;
		state = handshake.Next())
	{
		ASSERT_TRUE(handshake.PreparePacket(&buf, (uint32_t*)&buf_size));
		VS_SCOPE_EXIT { handshake.FreePacket((void**)&buf); };
		if (state == secure_st_SendPacket || state == secure_st_SendCert)
		{
			ASSERT_EQ(client.send((char*)buf, buf_size), (int)buf_size);
		}
		else
		{
			ASSERT_EQ(client.recv((char*)buf, buf_size), (int)buf_size);
			ASSERT_TRUE(handshake.ProcessPacket(buf, buf_size));
		}
	}

	ASSERT_EQ(state, secure_st_Finish);
	ASSERT_NE(write_crypt = handshake.GetWriteSymmetricCrypt(), nullptr);
	ASSERT_NE(read_crypt = handshake.GetReadSymmetricCrypt(), nullptr);

	//send encrypted test string
	uint32_t encrypted_size = 0;
	ASSERT_FALSE(write_crypt->Encrypt((unsigned char*)test_string, sizeof(test_string), nullptr, &encrypted_size));
	std::vector<unsigned char> encrypted_buf(encrypted_size);
	ASSERT_TRUE(write_crypt->Encrypt((unsigned char*)test_string, sizeof(test_string), encrypted_buf.data(), &encrypted_size));
	ASSERT_EQ(client.send((char*)encrypted_buf.data(), encrypted_buf.size()), static_cast<int>(encrypted_buf.size()));
	VS_SecureHandshake::ReleaseSymmetricCrypt(&write_crypt);
	VS_SecureHandshake::ReleaseSymmetricCrypt(&read_crypt);
}

TEST_F(SecureLibFixture, SecureHandshakeVer1)
{
#ifdef _WIN32
	WSADATA wsaData = { 0 };
	ASSERT_EQ(WSAStartup(MAKEWORD(2, 2), &wsaData), 0);
#endif
	std::thread server_thread(handshake_server, 1);
	std::thread client_thread(handshake_client, 1);
	server_thread.join();
	client_thread.join();
}


TEST_F(SecureLibFixture, SecureHandshakeVer2)
{
	std::thread server_thread(handshake_server, 2);
	std::thread client_thread(handshake_client, 2);
	server_thread.join();
	client_thread.join();
}
