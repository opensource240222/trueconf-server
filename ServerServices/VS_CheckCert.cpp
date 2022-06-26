#include "ProtectionLib/HardwareKey.h"
#include "std-generic/cpplib/VS_Container.h"
#include "std/debuglog/VS_Debug.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_SimpleStr.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/VS_IntConv.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "ProtectionLib/Protection.h"

extern std::string g_tr_endpoint_name;

#define DEBUG_CURRENT_MODULE VS_DM_ALL_

bool VS_CheckCert()
{
	////{
	////	FILE *f = fopen("ca.crt","r");
	////	unsigned char *buf(0);
	////	fseek( f, 0, SEEK_END );
	////	long b_size=ftell(f);
	////	fseek(f,0,SEEK_SET);
	////	buf = new unsigned char [b_size];
	////	fread(buf,b_size,1,f);
	////	fclose(f);
	////	f = 0;
	////

	////	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	////	rkey.SetValue(buf,b_size,VS_REG_BINARY_VT,"Certificate Reg");
	////	delete [] buf;
	////	buf = 0;
	////	b_size = 0;

	////	f= fopen("key.pem","r");
	////	fseek( f, 0, SEEK_END );
	////	b_size=ftell(f);
	////	fseek(f,0,SEEK_SET);
	////	buf = new unsigned char [b_size];
	////	fread(buf,b_size,1,f);
	////	fclose(f);

	////	rkey.SetValue(buf,b_size,VS_REG_BINARY_VT,"PrivateKey Reg");
	////	delete [] buf;
	////
	////}

	//////const char _PEM_CACERT[] =	"-----BEGIN CERTIFICATE-----\n\
	//////MIIDfzCCAmegAwIBAgIJAJXdEfe6hB0+MA0GCSqGSIb3DQEBBQUAMHYxHjAcBgNV\n\
//////BAMUFXJlZy50cnVlY29uZi5jb20jcmVnczEPMA0GA1UECBMGTW9zY293MQswCQYD\n\
//////VQQGEwJSVTEjMCEGCSqGSIb3DQEJARYUdnBfaW5mb0B0cnVlY29uZi5jb20xETAP\n\
//////BgNVBAoTCFRydWVDb25mMB4XDTExMDUwNDE1MDg1OFoXDTM4MTIyODE1MDg1OFow\n\
//////djEeMBwGA1UEAxQVcmVnLnRydWVjb25mLmNvbSNyZWdzMQ8wDQYDVQQIEwZNb3Nj\n\
//////b3cxCzAJBgNVBAYTAlJVMSMwIQYJKoZIhvcNAQkBFhR2cF9pbmZvQHRydWVjb25m\n\
//////LmNvbTERMA8GA1UEChMIVHJ1ZUNvbmYwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAw\n\
//////ggEKAoIBAQDePFzA139hByklCwptDHjip4f+iIRCDwVHevlc6RzHDY1VeQ1tbwSR\n\
//////Jm1WwvW/yxQZgJLFjioS3Cur1nvvnJAmO8FcOnNZXV0zjxR+kurDa3dWvXaw6HuB\n\
//////3Pe0t1uANl6ELpc32aFhjTO5wSdSOESALbM1mCUS4XDPiV1iIL+2eD1Qk4B3F4t3\n\
//////KAD5HI35ha6kQSwpcECPDqgPgc1Io5GUz76+5XBhp/uRZETDyN0BdXmKzozQJxTq\n\
//////n3GWe8B8CtZ+JAgAkvjN0q6yj2CS3mCkEjTs6jj4/fCRg2UytzsaEWsnjpwLwJ+L\n\
//////yrOHGsuxEwAtAZVVa1opIlp9FqvIkCJ9AgMBAAGjEDAOMAwGA1UdEwQFMAMBAf8w\n\
//////DQYJKoZIhvcNAQEFBQADggEBAF4D2gbGmNK8B+zj+hTsjX0C5tIlgw+4LNd3zoWq\n\
//////QY1dc3nz8INFqD3a2zcg99nRF57Q9c4nbWv726oCKUE5E9x+QEnazrgwc0cdBQQT\n\
///////ANU8829gpmuejtPEMgoTgr1R5Q3mupgW0TPZJv0NpU/fB2fXBACNiNI0Ja91ZUW\n\
//////EM+nQrALwHnH5+Uya/wD7yJharABIDe4kzXsk8/paygUl0p2IuqAFzCf2ZZoUvK1\n\
//////25yOOqpdV9gmPIEidd7AwOIKrcJgtJRJvyea39M1/nfE5ggDRKoQv3VZMlhoh6fd\n\
//////GGHRzXuBtZ4JTZ0F5HlBQhiLsnDnUQBkG0gzGX4dOTHNTlI=\n\
//////-----END CERTIFICATE-----\n\
//////";
//////
//////
//////	VS_Container cnt;
//////	cnt.AddValue("",_PEM_CACERT);
//////	char *tmp(0);
//////	unsigned long sz(0);
//////	cnt.SerializeAlloc((void*&)tmp,sz);
//////	int j=0;
//////	for(unsigned long i =0;i<sz;i++)
//////	{
//////		if(j==16)
//////		{
//////			printf("\n");
//////			j  = 0;
//////		}
//////		printf("0x%2.2x, ",(byte)tmp[i]);
//////		j++;
//////	}
	VS_GET_PEM_CACERT



	VS_PKeyCrypt		crypt_encr, crypt_decr;

	VS_CertificateCheck	certCheck;
	VS_PKey				private_key, public_key;
	VS_Certificate		cert;
	VS_RegistryKey		rKey(false, LICENSE_KEY_NAME, false, true);

	rKey.SetString("", "Error");
	rKey.SetString("", "Error Certificate");

	auto report_error = [&rKey](const char* text)
	{
		dprint0("CHECK CERT: %s\n", text);
		rKey.SetString(text, "Error");
	};
	auto report_cert_error = [&rKey](const char* text)
	{
		dprint0("CHECK CERT: %s\n", text);
		rKey.SetString(text, "Error Certificate");
	};

	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::unique_ptr<char, free_deleter> privateKey;
	rkey.GetValue(privateKey, VS_REG_BINARY_VT, SRV_PRIVATE_KEY);
	if (!privateKey)
	{
		report_error("Private key is absent!");
		return false;
	}
	if (!private_key.SetPrivateKey(privateKey.get(), store_PEM_BUF))
	{
		report_error("Private key not valid!");
		return false;
	}

	std::unique_ptr<char, free_deleter> certBuf;
	int certBufSz = rkey.GetValue(certBuf, VS_REG_BINARY_VT, SRV_CERT_KEY);

	if (!certBuf || certBufSz <= 0)
	{
		report_error("Certificate is absent!");
		return false;
	}
#ifdef _DEBUG
	FILE *f = fopen("cert.crt", "w+");
	if (f)
	{
		fwrite(certBuf.get(), strlen(certBuf.get()), 1, f);
		fclose(f);
	}

#endif
	if (!cert.SetCert(certBuf.get(), certBufSz, store_PEM_BUF) ||
		!certCheck.SetCert(certBuf.get(), certBufSz, store_PEM_BUF) || !cert.GetCertPublicKey(&public_key))
	{
		report_error("Certificate data not valid!");
		return false;
	}
	if (!certCheck.SetCertToChain(PEM_CACERT, strlen(PEM_CACERT) + 1, store_PEM_BUF))
	{
		report_error("Add Trusted Cert failed (Ca Cert)!");
		return false;
	}

	std::unique_ptr<void, free_deleter> certChain;
	int chainLen = rkey.GetValue(certChain, VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY);
	VS_Container	cert_chain;
	if (certChain)
		cert_chain.Deserialize(certChain.get(), chainLen);
	cert_chain.Reset();
	if (cert_chain.IsValid())
	{
		while (cert_chain.Next())
		{
			if (!!cert_chain.GetName() && (0 == strcasecmp(cert_chain.GetName(), CERTIFICATE_CHAIN_PARAM)))
			{
				size_t sz(0);
				const char *cert_in_chain = (const char*)cert_chain.GetBinValueRef(sz);
				if (sz && cert_in_chain)
					certCheck.SetCertToChain(cert_in_chain, sz, store_PEM_BUF);
			}
		}
	}

	std::string err_descr;
	if (!certCheck.VerifyCert(nullptr, &err_descr))
	{
		err_descr.insert(0, "Certificate not verified! ");
		report_error(err_descr.c_str());
		return false;
	}
	/**
	Check ProductType and hw_key
	*/
	std::string buf;
	if (cert.GetExtension(HWKEY_HASH_CERT_EXTENSIONS, buf))
	{
		/**
		прочитать с реестра, склеить
		*/
		VS_RegistryKey key(false, CONFIGURATION_KEY);
		char buf_key[33] = { 0 };
		if (key.GetValue(buf_key, sizeof(buf_key), VS_REG_STRING_VT, "Key") <= 0)
		{
			report_error("KEY is absent!");
			return false;
		}
		char hw_key[protection::HWKeyLength + 1];
		const auto hw_key_result = protection::ReadHWKey(hw_key);
		if (hw_key_result != 0)
		{
			dstream1 << "HWKey error: " << hw_key_result;
			report_error("Can't read HW key!");
			return false;
		}
		if (!protection::CheckSaltedHWKey(hw_key, buf_key, buf.c_str())){
			report_error("HW key is failed!");
			return false;
		}
	}
	if (cert.GetExtension(SERVER_VERSION_EXTENSIONS, buf))
	{
		if (atou_s(VCS_SERVER_VERSION) != atou_s(buf.c_str()))
		{
			report_cert_error("Server Version is incorrect!");
			return false;
		}
	}
	else
	{
		report_cert_error("Server Version is incorrect!");
		return false;

	}
	if (cert.GetSubjectEntry("commonName", buf))
	{
		if (strcasecmp(buf.c_str(), g_tr_endpoint_name.c_str()) != 0)
		{
			dstream0 << "From cert:" << buf << ", from var:" << g_tr_endpoint_name;
			report_cert_error("Server Name is incorrect!");
			return false;
		}
	}
	else
	{
		report_cert_error("Server Name is incorrect!");
		return false;
	}
	if (!crypt_decr.SetPrivateKey(&private_key) || !crypt_encr.SetPublicKey(&public_key))
	{
		report_error("Set Private and Public keys failed!");
		return false;
	}
	const char *src_data = "Test successful!";
	const uint32_t ln_src_data = (unsigned long)strlen(src_data) + 1;
	const char *res_encr_buf(0), *res_decr_buf(0), *sym_key(0);
	uint32_t ln_encr(0), ln_decr(0), ln_sym_key(0);
	unsigned char iv[16] = { 0 };

	crypt_encr.Encrypt((const unsigned char*)src_data, ln_src_data, alg_sym_AES256,
		(unsigned char*)res_encr_buf, &ln_encr, iv, (unsigned char*)sym_key, &ln_sym_key);
	sym_key = new char[ln_sym_key];
	crypt_encr.Encrypt((const unsigned char*)src_data, ln_src_data, alg_sym_AES256,
		(unsigned char*)res_encr_buf, &ln_encr, iv, (unsigned char*)sym_key, &ln_sym_key);
	res_encr_buf = new char[ln_encr];

	if (!crypt_encr.Encrypt((const unsigned char*)src_data, ln_src_data, alg_sym_AES256,
		(unsigned char*)res_encr_buf, &ln_encr, iv, (unsigned char*)sym_key, &ln_sym_key))
	{
		delete[] sym_key;
		delete[] res_encr_buf;
		report_error("Test encryption failed!");
		return false;
	}
	crypt_decr.Decrypt((const unsigned char*)res_encr_buf, ln_encr, alg_sym_AES256, iv,
		(const unsigned char*)sym_key, ln_sym_key, (unsigned char*)res_decr_buf, &ln_decr);
	res_decr_buf = new char[ln_decr];
	if (!crypt_decr.Decrypt((const unsigned char*)res_encr_buf, ln_encr, alg_sym_AES256, iv,
		(const unsigned char*)sym_key, ln_sym_key, (unsigned char*)res_decr_buf, &ln_decr))
	{
		delete[] sym_key;
		delete[] res_encr_buf;
		delete[]res_decr_buf;

		report_error("Test decryption failed!");
		return false;
	}
	if (strcmp(src_data, res_decr_buf))
	{
		delete[] sym_key;
		delete[] res_encr_buf;
		delete[]res_decr_buf;
		report_error("Decryption's result failed!");
		return false;
	}
	delete[] sym_key;
	delete[] res_encr_buf;
	delete[]res_decr_buf;
	return true;
}