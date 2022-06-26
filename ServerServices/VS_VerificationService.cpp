#include "VS_VerificationService.h"

#include "acs/Lib/VS_AcsLib.h"
#include "acs/VS_AcsDefinitions.h"
#include "ProtectionLib/HardwareKey.h"
#include "ProtectionLib/Protection.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_CertificateIssue.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_UtilsLib.h"
#include "ServerServices/VS_CheckCert.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/cpplib/VS_IntConv.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_Utils.h"
#include "std/debuglog/VS_Debug.h"
#include "std/RegistrationStatus.h"
#include "std/VS_RegServer.h"
#include "tools/Server/VS_Server.h"

#include "Common.h"

#include <openssl/x509_vfy.h>

#include <fstream>

#include <cstdlib>
#include <ctime>
#include <cstring>

extern std::string g_tr_endpoint_name;

#define DEBUG_CURRENT_MODULE VS_DM_VERIFYSERVICE

VS_VerificationService::VS_VerificationService(vs::RegistrationParams&& rp)
	: m_rp(std::move(rp))
	, m_recvMess(0)
	, m_watchdog(0)
	, m_isCertVerifyed(false)
	, m_noCertTick(0)
	, m_lastCertCheck(0)
	, m_check_cert_period(5000)
{
	m_TimeInterval = std::chrono::seconds(5);
}
VS_VerificationService::~VS_VerificationService()
{
}

bool VS_VerificationService::Init(const char* /*our_endpoint*/, const char* /*our_service*/, const bool /*permittedAll*/)
{
	char hw_key[protection::HWKeyLength + 1];
	const auto hw_key_result = protection::ReadHWKey(hw_key);
	if (hw_key_result != 0)
	{
		dstream1 << "HWKey error: " << hw_key_result;
		return false;
	}
	m_Key = hw_key;

	char buff[256] = {0};
	VS_RegistryKey cfg(false, CONFIGURATION_KEY, false, true);
	if (cfg.GetValue(buff, 256, VS_REG_STRING_VT, SERVER_MANAGER_TAG) > 0 && *buff) {
		m_SM = buff;
		// m_SM always is constant, no collisions
	}


	if(1==m_rp.mode)
	{
		RegisterServer();
		m_isCertVerifyed = false;
	}
	else if(2==m_rp.mode)
	{
		RegFromFile();
		m_watchdog->Shutdown();
	}
	else if(!VS_CheckCert())
	{
		dprint0("Certificate is not valid!\n");
		m_watchdog->Shutdown();
	}
	else
		m_isCertVerifyed = true;
	return true;
}

bool VS_VerificationService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
    if (recvMess == 0)  return true;
    switch (recvMess->Type())
    {
    case transport::MessageType::Invalid:
        break;
    case transport::MessageType::Request:
        break;
    case transport::MessageType::Reply:
		m_recvMess = recvMess.get();
		{
			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize())) {
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0) {
					// Process methods
					if(strcasecmp(method,REGISTERSERVER_METHOD) == 0)
					{
						RegisterServer_Method(cnt);
					}
					else if(strcasecmp(method,REGISTERSERVEROFFLINE_METHOD) == 0)
					{
					}
					else if(strcasecmp(method,CERTIFICATEUPDATE_METHOD) == 0)
					{
						CertificateUpdate_Method(cnt);
					}
				}
			}
		}
        break;
    case transport::MessageType::Notify:
		if(1 == m_rp.mode)
		{
			if (strcmp(m_recvMess->DstService(), REGISTRATION_SRV)==0)
			{
				RegStatus res = RegStatus::brokerIsNotAvailable;
				VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
				rkey.SetValue(&res,sizeof(res),VS_REG_INTEGER_VT,"RegResult");
				m_watchdog->Shutdown();
			}
		}
        break;
    }
    m_recvMess = nullptr;
    return true;

}

bool VS_VerificationService::ApplyNewCert(VS_Container &cnt, const bool offline_reg, int &err, std::string &err_str)
{
	const char *cert = cnt.GetStrValueRef(CERTIFICATE_PARAM);
	if(!cert)
	{
		err = -1;
		err_str ="certificate is absent!\n";
		return false;
	}
VS_GET_PEM_CACERT

	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	VS_Container cert_chain_cnt;
	VS_CertificateCheck	certCheck;
	certCheck.SetCert(cert,strlen(cert)+1,store_PEM_BUF);
	certCheck.SetCertToChain(PEM_CACERT,strlen(PEM_CACERT)+1, store_PEM_BUF);
	cnt.Reset();
	while(cnt.Next())
	{
		if(!!cnt.GetName() && (0 == strcasecmp(cnt.GetName(),CERTIFICATE_CHAIN_PARAM)))
		{
			const char *cert_in_chain = cnt.GetStrValueRef();
			if(cert_in_chain)
			{
				certCheck.SetCertToChain(cert_in_chain,strlen(cert_in_chain) + 1, store_PEM_BUF);
				cert_chain_cnt.AddValue(CERTIFICATE_CHAIN_PARAM,(void*)cert_in_chain,strlen(cert_in_chain));
			}
		}
	}
	if(cnt.GetStrValueRef(PARENT_CERT_PARAM))
		cert_chain_cnt.AddValue(PARENT_CERT_PARAM,cnt.GetStrValueRef(PARENT_CERT_PARAM));
#ifdef _DEBUG

	FILE *fp = fopen("certificate.pem","wb");
	if(fp)
	{
		fwrite(cert,strlen(cert)+1,1,fp);
		fclose(fp);
	}
#endif
	if(certCheck.VerifyCert(&err, &err_str))
	{
		VS_Certificate	srv_cert;
		srv_cert.SetCert(cert,strlen(cert) + 1,store_PEM_BUF);
		if(!rkey.SetValue(cert, static_cast<unsigned long>(strlen(cert)) + 1, VS_REG_BINARY_VT, SRV_CERT_KEY))
		{
			err_str = "Registry access error!\n";
			err = -2;
			return false;
		}
		if(!offline_reg)
		{
			if(!rkey.SetValue(m_PrivateKey.m_str, m_PrivateKey.Length() + 1, VS_REG_BINARY_VT, SRV_PRIVATE_KEY))
			{
				err_str = "Registry access error!\n";
				err = -2;
				return false;
			}
			std::string server_name;
			if(srv_cert.GetSubjectEntry("commonName",server_name))
			{
				VS_RegistryKey reg_root(false, "", false, true);
				if(!reg_root.SetString(server_name.c_str(), VS_SERVER_ENDPOINT_ARG_PREF))
				{
					err_str = "Can't write server name to registry! server name = ";
					err_str += server_name;
					err_str += "\n";
					delete [] server_name.c_str();
					err = -2;
					return false;
				}
			}
		}

		void *buf_cnt(0);
		size_t buf_sz(0);

		cert_chain_cnt.SerializeAlloc(buf_cnt,buf_sz);

		if(!rkey.SetValue(buf_cnt, buf_sz,VS_REG_BINARY_VT, SRV_CERT_CHAIN_KEY))
		{
			err_str = "Registry access error!\n";
			err = -2;
			return false;
		}
		m_isCertVerifyed = true;
		free(buf_cnt);
		return true;
	}
	else
		return false;
}

bool VS_VerificationService::GetLicensingInfo(VS_Container &cnt)
{
	uint64_t id = 0;
	int32_t type = 0;
	const void* data = nullptr;
	size_t size = 0;
	int counter=0;
	VS_RegistryKey l_root(false, LICENSE_KEY, false, false);
	cnt.Reset();

	while(cnt.Next())
	{
		if(strcasecmp(cnt.GetName(),NAME_PARAM)==0)
		{
			const void* d;
			size_t s = 0;
			d=cnt.GetBinValueRef(s);
			if (s == sizeof(uint64_t))
			{
				if (id != 0)
				{
					ProcessLicense(l_root,id,type,data,size);
					counter++;
				}

				id = *static_cast<const uint64_t*>(d);
				type=0;data=0;
			}
		}
		else if(strcasecmp(cnt.GetName(),TYPE_PARAM)==0)
		{	cnt.GetValue(type); }
		else if(strcasecmp(cnt.GetName(),DATA_PARAM)==0)
		{ data=cnt.GetBinValueRef(size); }
	}
	if(0!=id)
	{
		ProcessLicense(l_root,id,type,data,size);
		counter++;
	}
	return counter>0;
}
bool VS_VerificationService::ProcessLicense(VS_RegistryKey& l_root,uint64_t id,long type,const void* data,int size)
{
	switch(type)
	{
	case LIC_DELETE:
		{
			char lic_name[128];
			sprintf(lic_name, "%s\\%016" PRIX64, LICENSE_KEY, id);
			if(!l_root.RemoveKey(lic_name))
				return false;
			break;
		};
	case LIC_ADD:
		{
			if(data==NULL || size==0)
				return false;
			char lic_name[128];
			sprintf(lic_name, "%s\\%016" PRIX64, LICENSE_KEY, id);
			VS_RegistryKey	lic_key(false, lic_name, false, true);
			if (!lic_key.IsValid())
				return false;
			if(!lic_key.SetValue(data, size, VS_REG_BINARY_VT, LICENSE_DATA_TAG))
				return false;
		}
	}
	return true;
}

void VS_VerificationService::RegisterServer_Method(VS_Container &cnt)
{
	RegStatus result = RegStatus::failed;
	cnt.GetValueI32(RESULT_PARAM, result);
	dstream3 << "RegisterServer_Method; result = " << static_cast<int32_t>(result);
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	VS_Container cert_chain_cnt;

	std::string err_descr;
	if (result == RegStatus::succeeded) {
		rkey.SetString(cnt.GetStrValueRef(KEY_PARAM), "Key");

		int verify_err = 0;
		if(!ApplyNewCert(cnt,false,verify_err,err_descr))
		{
			dprint0("Registration failed: err = %d, descr = %s\n", verify_err, err_descr.c_str());
			if (verify_err == X509_V_ERR_CERT_NOT_YET_VALID)
				result = RegStatus::certIsNotYetValid;
			else if (verify_err == X509_V_ERR_CERT_HAS_EXPIRED)
				result = RegStatus::certHasExpired;
			else if (verify_err == X509_V_ERR_CERT_SIGNATURE_FAILURE || verify_err == X509_V_ERR_CRL_SIGNATURE_FAILURE)
				result = RegStatus::certSignatureIsInvalid;
			else if (verify_err == -1)
				result = RegStatus::certIsAbsent; // cert is absent in container
			else if (verify_err == -2)
				result = RegStatus::cannotWriteCert; // can't write cert to Registry
			else
				result = RegStatus::certIsInvalid;
		}
		else
		{

			const char *dumb_terminals_data = cnt.GetStrValueRef(SCRIPT_PARAM);
			HandleCallCfgCorrectorData(dumb_terminals_data);
		}
	}
	rkey.SetValue(&result,sizeof(result),VS_REG_INTEGER_VT,"RegResult");

	dstream0 << "Registration result: " << static_cast<int32_t>(result) << ' ' <<
		(err_descr.size() > 2 /*>ok*/ ? err_descr.c_str() : vs::ErrorCodeToString(result)) << '\n';

	m_watchdog->Shutdown();
}

void VS_VerificationService::CertificateUpdate_Method(VS_Container &cnt)
{
	int32_t result = false;
	cnt.GetValue(RESULT_PARAM, result);
	VS_Container cert_chain_cnt;
	dprint3("CertificateUpdate_Method; result = %d\n",result);
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	if (result == 1) {
		int err(0);
		std::string err_descr;
		if(ApplyNewCert(cnt,false,err,err_descr))
		{
			dprint3("Certificate Updated.\n");
			unsigned long certUpdated(1);
			rkey.SetValue(&certUpdated,sizeof(certUpdated),VS_REG_INTEGER_VT,"cert_updated");
		}
		else
		{
			dprint0("Apply updated certificate is failed!err = %d, err_descr = %s\n", err, err_descr.c_str());
		}
	}
}

namespace {
/**
	crypt buf using CA public key
	encr data is decrypted by RegServer only

	buf_out must be freed by caller
**/
bool VS_CACryptDataAlloc(const void * in_buf, const uint32_t in_buf_sz, void *&buf_out, uint32_t &out_buf_sz)
{
	if(!in_buf || !in_buf_sz)
		return false;
	VS_PKey			key;
	VS_Certificate	ca_cert;
	VS_PKeyCrypt	crypt;
	VS_GET_PEM_CACERT


	if(!ca_cert.SetCert(PEM_CACERT,strlen(PEM_CACERT)+1, store_PEM_BUF) ||
		!ca_cert.GetCertPublicKey(&key) || !crypt.SetPublicKey(&key))
		return false;

	unsigned char *encrbuf(0);
	uint32_t encrlen(0);
	unsigned char iv[16] = {0};
	unsigned char *sesskey(0);
	uint32_t lensesskey(0);

	crypt.Encrypt((const unsigned char*)in_buf,in_buf_sz,alg_sym_AES256,encrbuf,&encrlen,
					iv,sesskey,&lensesskey);
	sesskey = new unsigned char[lensesskey];
	crypt.Encrypt((const unsigned char*)in_buf,in_buf_sz,alg_sym_AES256,encrbuf,&encrlen,
					iv,sesskey,&lensesskey);
	encrbuf = new unsigned char[encrlen];
	if(!crypt.Encrypt((const unsigned char*)in_buf,in_buf_sz,alg_sym_AES256,encrbuf,&encrlen,
						iv,sesskey,&lensesskey))
	{
		delete [] sesskey;
		delete [] encrbuf;
		return false;
	}

	VS_Container cnt;
	cnt.AddValue("key",(void*)sesskey,lensesskey);
	cnt.AddValue("iv",(void*)iv,16);
	cnt.AddValue("data",encrbuf,encrlen);
	size_t out_buf_sz_tmp;
	cnt.SerializeAlloc(buf_out, out_buf_sz_tmp);
	out_buf_sz = out_buf_sz_tmp;
	delete [] sesskey;
	delete [] encrbuf;
	return true;
}

bool VS_GetArmParamForOfflineReg(VS_Container &cnt)
{
	VS_PKey			key;
	VS_Certificate	ca_cert;
	VS_PKeyCrypt	crypt;
	VS_GET_PEM_CACERT

		if(!ca_cert.SetCert(PEM_CACERT,strlen(PEM_CACERT)+1,store_PEM_BUF) ||
			!ca_cert.GetCertPublicKey(&key) || !crypt.SetPublicKey(&key))
			return false;
	int32_t arm_hw_key = VS_ArmReadHardwareKey();
	if(!arm_hw_key)
		return false;

	unsigned char *encrbuf(0);
	uint32_t encrlen(0);
	unsigned char iv[16] = {0};
	unsigned char *sesskey(0);
	uint32_t lensesskey(0);
	crypt.Encrypt((const unsigned char*)&arm_hw_key,sizeof(arm_hw_key),alg_sym_AES256,encrbuf,&encrlen,
		iv,sesskey,&lensesskey);
	sesskey = new unsigned char[lensesskey];
	crypt.Encrypt((const unsigned char*)&arm_hw_key,sizeof(arm_hw_key),alg_sym_AES256,encrbuf,&encrlen,
		iv,sesskey,&lensesskey);
	encrbuf = new unsigned char[encrlen];
	if(!crypt.Encrypt((const unsigned char*)&arm_hw_key,sizeof(arm_hw_key),alg_sym_AES256,encrbuf,&encrlen,
		iv,sesskey,&lensesskey))
	{
		delete [] sesskey;
		delete [] encrbuf;
		return false;
	}

	cnt.AddValue("key",(void*)sesskey,lensesskey);
	cnt.AddValue("iv",(void*)iv,16);
	cnt.AddValue("data",encrbuf,encrlen);

	char hw_key[protection::HWKeyLength + 1];
	const auto hw_key_result = protection::ReadHWKey(hw_key);
	if (hw_key_result != 0)
	{
		dstream1 << "HWKey error: " << hw_key_result;
		delete [] sesskey;
		delete [] encrbuf;
		return false;
	}
	cnt.AddValue("hw",hw_key);
	VS_SimpleStr os_info, cpu_info;
	VSGetSystemInfo_OS(os_info);
	if(VS_ArmDetectedVM())
		os_info +=" virtual";
	VSGetSystemInfo_Processor(cpu_info);
	cnt.AddValue("OS Info",os_info);
	cnt.AddValue("CPU Info",cpu_info);
	delete [] sesskey;
	delete [] encrbuf;
	return true;
}
}

void VS_VerificationService::RegisterServer()
{
	dprint3("RegServ request\n");
	VS_Container rCnt;
	VS_SimpleStr certReq;
	VS_SimpleStr regSrv = !!m_SM ? m_SM.m_str : RegServerName; ///для VCS;
	const bool to_file = !m_rp.offline_reg_file.empty();

	VS_SimpleStr os_info, cpu_info;
	std::string server_product_version;
	VSGetSystemInfo_OS(os_info);
	if(VS_ArmDetectedVM())
		os_info +=" virtual";
	VSGetSystemInfo_Processor(cpu_info);

	// check server name length
	if (m_rp.server_name.length() > VS_ACS_MAX_SIZE_SERVER_NAME)
	{
		RegStatus res = RegStatus::serverNameIsTooLong;
		dstream0 << "Registration result: " << static_cast<int32_t>(res) << ' ' <<
			vs::ErrorCodeToString(res) << '\n' << "Max server name length: " << VS_ACS_MAX_SIZE_SERVER_NAME
			<< " Proposed server name length: " << m_rp.server_name.length();
		VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
		rkey.SetValue(&res, sizeof(res), VS_REG_INTEGER_VT, "RegResult");
		m_watchdog->Shutdown();
		return;
	}

	rCnt.AddValue(METHOD_PARAM, (to_file)? REGISTERSERVEROFFLINE_METHOD: REGISTERSERVER_METHOD);
	rCnt.AddValue(KEY_PARAM, (char*)m_Key);
	rCnt.AddValue(SERVERID_PARAM, m_rp.server_id);
	rCnt.AddValue(PASSWORD_PARAM, m_rp.serial);
	rCnt.AddValue(SERVERNAME_PARAM, m_rp.server_name);
	rCnt.AddValue("Srv Version",VCS_SERVER_VERSION);
	rCnt.AddValue("OS Info",os_info);
	rCnt.AddValue("CPU Info",cpu_info);
	if (m_server_version.Length() > 0)
	{
		rCnt.AddValue(SERVER_VERSION_PARAM, m_server_version.m_str);
	}

	if(GenerateCertRequest(m_rp.server_name, {}, {}, {}, {}, certReq))
		rCnt.AddValue(CERT_REQUEST_PARAM, (char*)certReq);
	else
	{
		RegStatus res = RegStatus::cannotGenerateCertRequest;
		dstream0 << "Registration result: " << static_cast<int32_t>(res) << ' ' <<
			vs::ErrorCodeToString(res) << '\n';
		VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
		rkey.SetValue(&res, sizeof(res), VS_REG_INTEGER_VT, "RegResult");
		m_watchdog->Shutdown();
		return;
	}
	if(to_file)
	{
		RegStatus result = RegStatus::unknownError;
		VS_SCOPE_EXIT
		{
			dstream0 << "Registration result: " << static_cast<int32_t>(result) << ' ' <<
				vs::ErrorCodeToString(result) << '\n';
		};

		rCnt.AddValue("Version", ARM_LICENSE_VERSION);
		VS_Container arm_cnt;
		if(!VS_GetArmParamForOfflineReg(arm_cnt))
		{
			result = RegStatus::regFileGenError;
			dprint0("REGOFFLINE: GetArmParam failed!\n");
			m_watchdog->Shutdown();
			return;
		}
		if (!rCnt.AddValue("arm", arm_cnt))
		{
			result = RegStatus::regFileGenError;
			dprint0("REGOFFLINE: Serialize container with arm hw key failed!\n");
			m_watchdog->Shutdown();
			return;
		}
		void* body;
		size_t bodySize;
		rCnt.SerializeAlloc(body, bodySize);

		void *encr_buf(0);
		uint32_t encr_buf_sz(0);
		if(!VS_CACryptDataAlloc(body,bodySize,encr_buf,encr_buf_sz))
		{
			result = RegStatus::regFileEncryptError;
			dprint0("REGOFFLINE:Crypt data failed!\n");
			m_watchdog->Shutdown();
			free(body);
			return;
		}
		free(body);
		body = 0;
		uint32_t b64_buf_sz(0);
		char *b64_buf = VS_Base64EncodeAlloc(encr_buf,encr_buf_sz,b64_buf_sz);
		free(encr_buf);

		VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
		if(!rkey.SetValue(m_PrivateKey.m_str, m_PrivateKey.Length() + 1, VS_REG_BINARY_VT, SRV_PRIVATE_KEY))
		{
			result = RegStatus::registryAccessError;
			dprint0("REGOFFLINE:Write to registry is failed!\n");
			m_watchdog->Shutdown();
			free(b64_buf);
			return;
		}
		VS_RegistryKey reg_root(false, "", false, true);
		if(!reg_root.SetString(m_rp.server_name.c_str(), VS_SERVER_ENDPOINT_ARG_PREF))
		{
			result = RegStatus::registryAccessError;
			dprint0("REGOFFLINE: Can't write server name to registry! server name = %s\n", m_rp.server_name.c_str());
			m_watchdog->Shutdown();
			return;
		}
		std::fstream f(m_rp.offline_reg_file, std::fstream::binary|std::fstream::out);
		if ( !f )
		{
			result = RegStatus::fileAccessError;
			dstream0 << "REGOFFLINE:Open file failed! fileName = " << m_rp.offline_reg_file;
			m_watchdog->Shutdown();
			free(b64_buf);
			return;
		}
		if(!f.write(b64_buf,b64_buf_sz))
		{
			result = RegStatus::fileAccessError;
			dstream0 << "REGOFFLINE:Write data to file failed! fileName = " << m_rp.offline_reg_file;
			m_watchdog->Shutdown();
			free(b64_buf);
			return;
		}
		f.close();
		free(b64_buf);
		result = RegStatus::succeeded;
		dstream0 << "REGOFFLINE: File for offline registration is generated. File location " << m_rp.offline_reg_file;
		m_watchdog->Shutdown();
		return;
	}
	PostRequest(regSrv,0,rCnt,0,m_registration_service);
}

void VS_VerificationService::RegFromFile()
{
	RegStatus result(RegStatus::failed);
	std::string err_descr;
	VS_SCOPE_EXIT
	{
		dstream0 << "Registration result: " << static_cast<int32_t>(result) << ' ' <<
			(err_descr.size() > 2 /*>ok*/ ? err_descr.c_str() : vs::ErrorCodeToString(result)) << '\n';
	};

	if(m_rp.offline_reg_file.empty())
	{
		result = RegStatus::badRegFile;
		dprint0("REGFROMFILE: file for reg is absent\n");
		return;
	}
	VS_PKey			private_key;
	VS_PKeyCrypt	crypt;

	std::fstream f(m_rp.offline_reg_file,std::fstream::binary|std::fstream::in|std::fstream::ate);
	if(!f || !f.is_open())
	{
		result = RegStatus::fileAccessError;
		dstream0 << "REGFROMFILE: error open file. File path = " << m_rp.offline_reg_file;
		return;
	}
	size_t b_size = static_cast<size_t>(f.tellg());
	std::vector<unsigned char> buf(b_size);
	f.seekg(0,std::ios::beg);
	if(!f.read(reinterpret_cast<char*>(buf.data()),b_size))
	{
		result = RegStatus::badRegFile;
		dstream0 << "REGFROMFILE: error read file. File path = " << m_rp.offline_reg_file;
		return;
	}
	f.close();
	std::unique_ptr<char, free_deleter> pkey_buf;
	int pkey_buf_ln(0);
	VS_RegistryKey rkey(false, CONFIGURATION_KEY);
	pkey_buf_ln = rkey.GetValue(pkey_buf, VS_REG_BINARY_VT, SRV_PRIVATE_KEY);
	if ((pkey_buf_ln <= 0) || (!private_key.SetPrivateKey(pkey_buf.get(), store_PEM_BUF))
		||(!crypt.SetPrivateKey(&private_key)))
	{
		result = RegStatus::badPrivateKey;
		dprint0("REGFROMFILE: Data in registry is not private key.\n");
		return;
	}

	uint32_t decoded_sz(0);
	void *decoded_buf = reinterpret_cast<void*>(VS_Base64DecodeAlloc(&buf[0],b_size,decoded_sz));
	VS_Container cnt;
	if((!decoded_buf) || !cnt.Deserialize(decoded_buf,decoded_sz))
	{
		result = RegStatus::badRegFile;
		free(decoded_buf);
		dprint0("REGFROMFILE: File has unsupported format;\n");
		return;
	}
	free(decoded_buf);
	size_t sess_key_ln(0);
	const unsigned char *sess_key = (const unsigned char*)cnt.GetBinValueRef("key",sess_key_ln);
	size_t iv_ln(0);
	const unsigned char	*iv = (const unsigned char *)cnt.GetBinValueRef("iv",iv_ln);
	size_t encr_data_ln(0);
	const unsigned char * encr_data = (const unsigned char *)cnt.GetBinValueRef("data",encr_data_ln);

	uint32_t decrlen(0);

	crypt.Decrypt(encr_data,encr_data_ln,alg_sym_AES256,iv,sess_key,sess_key_ln,0,&decrlen);
	if(decrlen<=0)
	{
		result = RegStatus::regFileDecryptError;
		dprint0("REGFROMFILE: Decrypt error;\n");
		return;
	}
	std::vector<unsigned char> decrdata(decrlen);


	if(!crypt.Decrypt(encr_data,encr_data_ln,alg_sym_AES256,iv,sess_key,sess_key_ln,&decrdata[0],&decrlen) ||
		!cnt.Deserialize(&decrdata[0],decrlen))
	{
		result = RegStatus::regFileDecryptError;
		dprint0("REGFROMFILE: Decrypt error;\n");
		return;
	}
	VS_RegistryKey key_write(false, CONFIGURATION_KEY, false, true);
	if(!key_write.SetString(cnt.GetStrValueRef(KEY_PARAM), "Key"))
	{
		result = RegStatus::registryAccessError;
		dprint0("REGFROMFILE: Registry access error!\n");
		return;
	}

	size_t ak_sz(0);
	const void * ak_buf = cnt.GetBinValueRef("ak",ak_sz);
	if(ak_buf)
	{
		if(!key_write.SetValue(ak_buf,ak_sz,VS_REG_BINARY_VT,"ak"))
		{
			result = RegStatus::registryAccessError;
			dprint0("REGFROMFILE: Registry access error!\n");
			return;
		}
	}

	int verify_err(0);
	if(!ApplyNewCert(cnt,true,verify_err,err_descr))
	{
		dprint0("REGFROMFILE: Apply new cert is failed. Err = %d, err descr = %s\n", verify_err, err_descr.c_str());
		if (verify_err == X509_V_ERR_CERT_NOT_YET_VALID)
			result = RegStatus::certIsNotYetValid;
		else if (verify_err == X509_V_ERR_CERT_HAS_EXPIRED)
			result = RegStatus::certHasExpired;
		else if (verify_err == X509_V_ERR_CERT_SIGNATURE_FAILURE || verify_err == X509_V_ERR_CRL_SIGNATURE_FAILURE)
			result = RegStatus::certSignatureIsInvalid;
		else if (verify_err == -1)
			result = RegStatus::certIsAbsent; // cert is absent in container
		else if (verify_err == -2)
			result = RegStatus::cannotWriteCert; // can't write cert to Registry
		else
			result = RegStatus::certIsInvalid;
		return;
	}
	else
	{
		const char *dumb_terminals_data = cnt.GetStrValueRef(SCRIPT_PARAM);
		HandleCallCfgCorrectorData(dumb_terminals_data);
	}
	if(!GetLicensingInfo(cnt))
	{
		result = RegStatus::badRegFile;
		dprint0("REGFROMFILE: GetLicensingInfo error;\n");
		return;
	}
	result = RegStatus::succeeded;
}

void VS_VerificationService::CertificateUpdate()
{
	VS_Container	cnt;
	VS_SimpleStr	certReq;

	dprint3("Cert Update\n");
	VS_SimpleStr regSrv = !!m_SM ? m_SM.m_str : RegServerName; ///для VCS;
	cnt.AddValue(METHOD_PARAM,CERTIFICATEUPDATE_METHOD);
	VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
	std::unique_ptr<char, free_deleter> cert_buf;
	rkey.GetValue(cert_buf, VS_REG_BINARY_VT, SRV_CERT_KEY);
	if(!cert_buf)
		return;
	cnt.AddValue(CERTIFICATE_PARAM, cert_buf.get());
	if (GenerateCertRequest(OurEndpoint(), {}, {}, {}, {}, certReq))
		cnt.AddValue(CERT_REQUEST_PARAM, (char*)certReq);
	else
	{
		dprint0("Certificate update failed: Cannot generate certificate request.\n");
	}
	PostRequest(regSrv,0,cnt,0,m_registration_service);
}

bool VS_VerificationService::GenerateCertRequest(string_view server_name, string_view organization_name, string_view country, string_view contact_person, string_view contact_email, VS_SimpleStr &certReq)
{
	VS_CertificateRequest	req;
	VS_PKey					pkey;
	char					*pem_buf(0);
	uint32_t				sz(0);
	if(!pkey.GenerateKeys(VS_DEFAULT_PKEY_LEN,alg_pk_RSA) ||
		!req.SetPKeys(&pkey,&pkey) || !req.SetEntry("commonName",server_name))
		return false;
	req.SetEntry("organizationName", organization_name);
	req.SetEntry("countryName", country);
	req.SetEntry("surname", contact_person);
	req.SetEntry("emailAddress", contact_email);

	// add server name as DNS SubjAltName
	auto sv = server_name.substr(0, server_name.find_first_of('#'));
	if (!sv.empty())
	{
		if (!req.SetSubjectAltName(SUBJ_ALT_NAME_DNS, sv))
			return false;
	}

	if(!req.SignRequest())
		return false;
	req.SaveTo(0,sz,store_PEM_BUF);
	if(!sz)
		return false;
	pem_buf = new char[sz+1];
	if(!req.SaveTo(pem_buf,sz,store_PEM_BUF))
	{
		delete [] pem_buf;
		return false;
	}
	pem_buf[sz] = 0;
	certReq = pem_buf;
	delete [] pem_buf;
	pem_buf = 0;
	sz = 0;
	pkey.WritePrivateKey(pem_buf,sz,store_PEM_BUF);
	if(!sz)
		return false;
	pem_buf = new char [sz + 1];
	pkey.WritePrivateKey(pem_buf,sz,store_PEM_BUF);
	pem_buf[sz] = 0;
	m_PrivateKey = pem_buf;
	delete [] pem_buf;
	sz = 0;
	return true;
}

void VS_VerificationService::PrepareCertDataToLog(VS_Certificate *cert, VS_SimpleStr &str)
{
	if(!cert)
		str = "Certificate is null";
	else
	{
		std::string buf;
		std::string notBeforeBuf, notAfterBuf;
		if(cert->GetSubjectEntry("commonName",buf))
		{
			str += "CommonName = ";
			str += buf.c_str();
			str += ";\n";
		}
		if(cert->GetSubjectEntry("countryName",buf))
		{
			str += "CountreName = ";
			str += buf.c_str();
			str += ";\n";
		}
		if(cert->GetSubjectEntry("organizationName",buf))
		{
			str += "organization = ";
			str += buf.c_str();
			str += ";\n";
		}
		if(cert->GetSubjectEntry("surname",buf))
		{
			str += "contact person = ";
			str += buf.c_str();
			str += ";\n";
		}
		if(cert->GetSubjectEntry("emailAddress",buf))
		{
			str += "Email = ";
			str += buf.c_str();
			str += ";\n";
		}
		if(cert->GetExpirationTime(notBeforeBuf,notAfterBuf))
		{
			str += "Not Before = ";
			str += notBeforeBuf.c_str();
			str += "; Not After = ";
			str += notAfterBuf.c_str();
			str +=";\n";
		}
	}
}

void VS_VerificationService::HandleCallCfgCorrectorData(const char * data)
{
	if (data != nullptr && m_call_cfg_correcto_data_handler != nullptr)
	{
		m_call_cfg_correcto_data_handler(data);
	}
}

void VS_VerificationService::SetComponents(VS_RoutersWatchdog* watchdog, const char *reg_service, const char *ver)
{
	m_watchdog = watchdog;
	m_registration_service = reg_service;
	m_server_version = ver;
}

void VS_VerificationService::SetCallCfgCorrectorDataHandler(const std::function<void(const char *)>& handler)
{
	m_call_cfg_correcto_data_handler = handler;
}

void VS_VerificationService::AsyncDestroy()
{}

bool VS_VerificationService::Timer(unsigned long tickcount)
{
	if(1 == m_rp.mode)
	{
		if(tickcount>1000*60*5)
		{
			dprint3("Trying registration more then 5 mins. Shutdowning...\n");
			RegStatus res(RegStatus::brokerIsNotAvailable);
			VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
			rkey.SetValue(&res,sizeof(res),VS_REG_INTEGER_VT,"RegResult");
			m_watchdog->Shutdown();
		}
	}
	else
	{
		if(!m_isCertVerifyed)
		{
			if(!m_noCertTick)
				m_noCertTick = tickcount;
			else if(tickcount - m_noCertTick >1000 * 60 * 5)
				m_watchdog->Shutdown();
			if(VS_CheckCert())
			{
				m_noCertTick = 0;
				m_isCertVerifyed = true;
			}
		}
		if(tickcount - m_lastCertCheck > m_check_cert_period)
		{
			m_check_cert_period = 1000*60*60*2; //check every 2 hours
			dprint3("periodic CheckCert\n");
			m_lastCertCheck = tickcount;
			if(!VS_CheckCert())
			{
				dprint0("Certificate failed!\n");
				m_watchdog->Shutdown();
			}
			else
			{
				/**
				проверить, если сертификат истекает менее, чем через 5 дней, то перезапросить
				*/
				std::unique_ptr<char, free_deleter> certBuf;
				VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
				int cert_buf_len = rkey.GetValue(certBuf, VS_REG_BINARY_VT, SRV_CERT_KEY);
				if(!certBuf || cert_buf_len<=0)
					return true;
				VS_Certificate cert;
				cert.SetCert(certBuf.get(), cert_buf_len, store_PEM_BUF);
				std::string notBeforeBuf, notAfterBuf;
				cert.GetExpirationTime(notBeforeBuf, notAfterBuf, false);
				char year[5] ={0};
				char month[3] = {0};
				char day[3] = {0};
				char hours[3] = {0};
				char mins[3] = {0};
				char sec[3] = {0};

				strncpy(year,notAfterBuf.c_str(),4);
				strncpy(month,notAfterBuf.c_str()+4,2);
				strncpy(day,notAfterBuf.c_str()+6,2);
				strncpy(hours,notAfterBuf.c_str()+8,2);
				strncpy(mins,notAfterBuf.c_str()+10,2);
				strncpy(sec,notAfterBuf.c_str()+12,2);

				time_t notAfterT;
				time_t nowT;
				time(&nowT);
				tm notAtm;
				notAtm.tm_sec = atoi(sec);
				notAtm.tm_min = atoi(mins);
				notAtm.tm_hour = atoi(hours);
				notAtm.tm_mday = atoi(day);
				notAtm.tm_mon = atoi(month) - 1;
				notAtm.tm_year = atoi(year) - 1900;
				notAfterT = mktime(&notAtm);
				if(5>=(notAfterT - nowT)/60/60/24)
				{
					dprint3("Certificate Expired less then 5 days\n");
					CertificateUpdate();
				}
			}
		}
	}
	return true;
}

void VS_VerificationService::ServerVerificationFailed()
{
	/**
		???
	*/
}
bool VS_VerificationService::OnPointConnected_Event(const VS_PointParams *prm)
{
	if(1 == m_rp.mode)
	{
		if(prm)
			dprint3("OnPointConnected to %s, reazon = %d\n",prm->uid,prm->reazon);
		else
			dprint1("OnPointConnected prm == 0!!!\n");
		VS_SimpleStr regSrv = !!m_SM ? m_SM.m_str : RegServerName;
		if(prm && (regSrv  == prm->uid) && (prm->reazon<=0))
		{
			RegStatus res(RegStatus::failed);
			switch(prm->reazon)
			{
			case -1:
				res = RegStatus::brokerIsNotAvailable; //Timeout
				break;
			case -2:
				res = RegStatus::brokerIsNotAvailable; //HS Err;
				break;
			default:
				res = RegStatus::unknownError;
			}
			VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);

			rkey.SetValue(&res,sizeof(res),VS_REG_INTEGER_VT,"RegResult");
			m_watchdog->Shutdown();
		}
	}
	return true;
}
bool VS_VerificationService::OnPointDisconnected_Event(const VS_PointParams *prm)
{
	if(1 == m_rp.mode)
	{
		if(prm)
			dprint3("OnPointDisconnected to %s, reazon = %d\n",prm->uid,prm->reazon);
		else
		{
			dprint1("OnPointDisconnected prm == 0!!!\n");
		}
		VS_SimpleStr regSrv = !!m_SM ? m_SM.m_str : RegServerName;
		RegStatus res(RegStatus::brokerIsNotAvailable);
		if(prm && (regSrv  == prm->uid))
		{
			VS_RegistryKey rkey(false, CONFIGURATION_KEY, false, true);
			if(!rkey.GetValue(&res,sizeof(res),VS_REG_INTEGER_VT,"RegResult"))
			{
				rkey.SetValue(&res,sizeof(res),VS_REG_INTEGER_VT,"RegResult");
				m_watchdog->Shutdown();
			}
		}
	}
	return true;
}
