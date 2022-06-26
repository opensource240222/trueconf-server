#include "VS_RegOfflineClient.h"
#include "../common/std/cpplib/VS_RegistryKey.h"
#include "SecureLib/VS_CryptoInit.h"
#include "SecureLib/VS_Certificate.h"
#include "SecureLib/VS_SecureConstants.h"
#include "SecureLib/VS_PublicKeyCrypt.h"
#include "SecureLib/VS_UtilsLib.h"
#include "transport/Client/VS_TransportClient.h"
#include "acs/ConnectionManager/VS_ConnectionManager.h"
#include "net/EndpointRegistry.h"
#include "std/cpplib/VS_Endpoint.h"
#include "std/VS_RegServer.h"
#include "acs/Lib/VS_AcsLib.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/RegistrationStatus.h"

#include <memory>

DWORD WINAPI ThreadProc(void* param)
{
	vs::SetThreadName("RegOffline");
	return static_cast<VS_RegOfflineClient*>(param)->ThreadCycle();
}

VS_RegOfflineClient::VS_RegOfflineClient(): m_hThread(0), m_Valid(false),m_hRegCompleteEvent(0),m_IsRegSuccess(false),m_reg_process(0)
{
	vs::InitOpenSSL();
	VS_RegistryKey::SetDefaultRoot("Visicron\\OfflineRegistrationClient");
	// FIXME: There should be a way to specify different backend.
	// RegOfflineClient should probably get support for command line options.
	if (!VS_RegistryKey::InitDefaultBackend("registry:force_lm=true"))
	{
		throw std::runtime_error("Can't initialize registry backend!");
	}
	m_hDie = CreateEvent(0,FALSE,FALSE,0);
}
VS_RegOfflineClient::~VS_RegOfflineClient()
{
	Release();
	if (m_hDie) CloseHandle(m_hDie);
}

bool VS_RegOfflineClient::Init(const char *reg_file_path)
{
	/**
		поднять транспор
	*/
	if(!reg_file_path)
		return false;
	char buff[256]; *buff = 0;

	VS_UninstallTransportClient();
	VS_UninstallConnectionManager();

	////VS_GenerateTempEndpoint(buff);
	VS_GenKeyByMD5(buff);
	m_Ep = buff;
	*buff = 0;

	net::endpoint::ClearAllConnectTCP(RegServerName);
	net::endpoint::AddConnectTCP({ RegServerHost, RegServerPort, RegServerProtocol }, RegServerName);

	m_RegEp = RegServerName;

	m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, &m_ThreadId);
	if (!m_hThread)
		return false;
	if ( !VS_AcsLibInitial())
		return false;
	if ( !VS_InstallConnectionManager(m_Ep))
		return false;
	if ( !VS_InstallTransportClient( ) )
		return false;
	if ( !VS_RegisterService(CLIENT_SRV, m_ThreadId, WM_SERVICE_RC) )
		return false;

	m_Valid = true;
	m_reg_file_path = reg_file_path;
	return true;
}
void VS_RegOfflineClient::Release()
{
	if (m_ThreadId) {
		PostThreadMessage(m_ThreadId, WM_RC_DESTROY ,0, 0);
		WaitForSingleObject(m_hDie, 10000);
		if (m_hThread) CloseHandle(m_hThread); 	m_hThread = 0;
		m_ThreadId = 0;
	}
	m_Valid = false;
}

DWORD VS_RegOfflineClient::ThreadCycle()
{
    MSG msg;
	DWORD dwObj;
	// force to create message queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	while(TRUE) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	{
			if(msg.message == WM_SERVICE_RC)
				Servise(&msg);
			if (msg.message == WM_RC_DESTROY) {
				VS_UninstallTransportClient();
				VS_UninstallConnectionManager();
				SetEvent(m_hDie);
				return NOERROR;
			}
		}
		dwObj = MsgWaitForMultipleObjects(0, NULL, FALSE, 1000, QS_ALLINPUT);
		////if (dwObj == WAIT_TIMEOUT) {
		////	unsigned int error =0;
		////	VS_TransportClientError(m_RegEp, &error, 0);
		////	if (error!=0)
		////		dwObj = error;
		////}
	}
	return -255;
}
bool VS_RegOfflineClient::IsRegSuccess()
{
	return m_IsRegSuccess;
}

bool VS_RegOfflineClient::MakeRegistration(HANDLE hRegComplete, const char *reg_result_file)
{
	m_Error = "";

	if(!m_Valid)
	{
		return false;
	}
	FILE *f = fopen(m_reg_file_path,"rb");
	if(!f)
	{
		m_Error = "open file error. File name: ";
		m_Error += m_reg_file_path;
		m_Error+= ";";
		return false;
	}
	unsigned long file_sz(0);
	if(fseek( f, 0, SEEK_END ) || (0>=(file_sz=ftell(f))))
	{
		fclose(f);
		char tmp[256] = {0};
		_itoa(file_sz,tmp,10);
		m_Error = "file operation error. File name = ";
		m_Error += m_reg_file_path;
		m_Error += "; file_sz = ";
		m_Error += tmp;
		m_Error+= ";";
		return false;
	}
	fseek(f,0,SEEK_SET);
	auto file_buf = std::make_unique<char[]>(file_sz);
	if (1 != fread(file_buf.get(), file_sz, 1, f))
	{
		int err = ferror(f);
		fclose(f);
		m_Error = "File read error. File name = ";
		m_Error += m_reg_file_path;
		m_Error+= "; err_no = ";
		char tmp[256] = {0};
		_itoa(err,tmp,10);
		m_Error += tmp;
		m_Error+= ";";
		return false;
	}
	fclose(f);

	VS_Container	cnt;
	cnt.AddValue("FileContent", file_buf.get(), file_sz);
	cnt.AddValue(METHOD_PARAM,REGISTERSERVEROFFLINEFROMFILE_METHOD);

	void* buf = nullptr;
	size_t sz(0);
	cnt.SerializeAlloc(buf,sz);
	VS_ClientMessage tMsg(CLIENT_SRV, 0, 0, /*MANAGER_SRV*/REGISTRATION_SRV, 30000, buf, sz,0,0,m_RegEp);
	///VS_ClientMessage tMsg(CLIENT_SRV, 0, m_RegEp, REGISTRATION_SRV, 30000, buf, sz);
	free(buf);

	m_reg_replay.Clear();
	m_reg_process = 0;
	m_reg_result_file = reg_result_file;
	m_hRegCompleteEvent = hRegComplete;
	m_IsRegSuccess = false;

	if(tMsg.Send()==0)
	{
		m_Error = "Send Message Error";
		m_Error+= ";";
		return false;
	}
	return true;;
}
void VS_RegOfflineClient::Servise(MSG *msg)
{
	VS_GET_PEM_CACERT

	VS_Container cnt;
	VS_ClientMessage tMsg(msg);
	switch(tMsg.Type())
	{
	case transport::MessageType::Reply:
		if (cnt.Deserialize(tMsg.Body(), tMsg.BodySize()))
		{
			const char *method = cnt.GetStrValueRef(METHOD_PARAM);
			if(method && !_stricmp(method,REGISTERSERVEROFFLINE_METHOD))
			{
				RegStatus result = RegStatus::failed;
				cnt.GetValueI32(RESULT_PARAM, result);
				m_server_id = cnt.GetStrValueRef(SERVERID_PARAM);
				m_server_name = cnt.GetStrValueRef(SERVERNAME_PARAM);
				if(RegStatus::succeeded == result)
				{
					const char *key = cnt.GetStrValueRef(KEY_PARAM);
					const char *cert = cnt.GetStrValueRef(CERTIFICATE_PARAM);
					if(key && cert)
					{
						VS_CertificateCheck	certCheck;
						certCheck.SetCert(cert,strlen(cert)+1,store_PEM_BUF);
						certCheck.SetCertToChain(PEM_CACERT,strlen(PEM_CACERT)+1,store_PEM_BUF);
						cnt.Reset();
						while(cnt.Next())
						{
							if(!!cnt.GetName() && (0 == _stricmp(cnt.GetName(),CERTIFICATE_CHAIN_PARAM)))
							{
								const char *cert_in_chain = cnt.GetStrValueRef();
								if(cert_in_chain)
								{
									certCheck.SetCertToChain(cert_in_chain,strlen(cert_in_chain) + 1,store_PEM_BUF);
									m_reg_replay.AddValue(CERTIFICATE_CHAIN_PARAM,cert_in_chain);
								}
							}
							else if (!!cnt.GetName() && (0 == _stricmp(cnt.GetName(), SCRIPT_PARAM)))
							{
								const char *script = cnt.GetStrValueRef();
								if (script != nullptr)
								{
									m_reg_replay.AddValue(SCRIPT_PARAM, script);
								}
							}
						}
						if(cnt.GetStrValueRef(PARENT_CERT_PARAM))
							m_reg_replay.AddValue(PARENT_CERT_PARAM,cnt.GetStrValueRef(PARENT_CERT_PARAM));


						std::string err_descr;
						if(certCheck.VerifyCert(nullptr, &err_descr))
						{
							m_reg_replay.AddValue(KEY_PARAM,key);
							m_reg_replay.AddValue(CERTIFICATE_PARAM,cert);
							m_reg_process |= 0x1; //получил сертификат
						}
						else
						{
							m_Error = "Verifying of issued the certificate was failed;";
							m_Error += err_descr.c_str();
							SetEvent(m_hRegCompleteEvent);
						}
					}
					else
					{
						m_Error = "Key or Certificate is absent";
						SetEvent(m_hRegCompleteEvent);
					}
				}
				else
				{
					char tmp[256] = {0};
					switch (result)
					{
					case RegStatus::failed:
						m_Error = "Incorrect code or license time is incorrect or issue certificate failed.";
						break;
					case RegStatus::changingHardwareIsNotAllowed:
						m_Error = "Computer change is not available for this server code.";
						break;
					case RegStatus::serverNameIsInUse:
						m_Error = "Server name is already registered.";
						break;
					case RegStatus::validLicenseIsNotAvailable:
						m_Error = "No Valid Lic";
						break;
					default:
						_itoa(static_cast<int32_t>(result),tmp,10);
						m_Error = "method = ";
						m_Error += method;
						m_Error+= ";";
						m_Error += "result = ";
						m_Error += tmp;
						m_Error+= ";";
					};
					SetEvent(m_hRegCompleteEvent);
				}
			}
			else if (method && !_stricmp(method, UPDATELICENSEOFFLINE_METHOD))
			{
				RegStatus result = RegStatus::failed;
				cnt.GetValueI32(RESULT_PARAM, result);
				if(RegStatus::succeeded == result)
				{
					cnt.Reset();
					while(cnt.Next())
					{
						if(_stricmp(cnt.GetName(),NAME_PARAM)==0)
						{
							size_t sz(0);
							const void *name_buf(0);
							name_buf = cnt.GetBinValueRef(sz);
							m_reg_replay.AddValue(cnt.GetName(),name_buf,sz);
						}
						else if(_stricmp(cnt.GetName(),TYPE_PARAM)==0)
						{
							int32_t type(0);
							cnt.GetValue(type);
							m_reg_replay.AddValue(cnt.GetName(),type);
						}
						else if(_stricmp(cnt.GetName(),DATA_PARAM)==0)
						{
							size_t sz(0);
							const void *buf = cnt.GetBinValueRef(sz);
							m_reg_replay.AddValue(cnt.GetName(),buf,sz);
						}
						else if(_stricmp(cnt.GetName(),"ak") == 0)
						{
							size_t sz(0);
							const void *buf = cnt.GetBinValueRef(sz);
							m_reg_replay.AddValue(cnt.GetName(),buf,sz);
						}
					}
					m_reg_process |= 0x2;
				}
				else
				{
					char tmp[256] = {0};
					_itoa(static_cast<int32_t>(result),tmp,10);
					m_Error = "method = ";
					m_Error += method;
					m_Error+= ";";
					m_Error += "result = ";
					m_Error += tmp;
					m_Error+= ";";
					SetEvent(m_hRegCompleteEvent);
				}
			}
		}
		break;
	};
	if((m_reg_process&0x1) && (m_reg_process & 0x2))
	{
		FILE *f = fopen(m_reg_result_file,"wb");
		if(f)
		{
			void *buf(0);
			unsigned long sz(0);
			if(PrepareRegDataAlloc(buf,sz))
			{
				if(1 == fwrite(buf,sz,1,f))
					m_IsRegSuccess = true;
				else
				{
					m_Error = "Write file error. File name: ";
					m_Error += m_reg_result_file;
					m_Error+= ";";
				}
				free(buf);
			}
			else
				m_Error = "Encryption or encoding error;";
			fclose(f);
		}
		else
		{
			m_Error = "open file error. File name: ";
			m_Error += m_reg_result_file;
			m_Error+= ";";
		}
		SetEvent(m_hRegCompleteEvent);

	}
}
bool VS_RegOfflineClient::PrepareRegDataAlloc(void *&buf_out, unsigned long &out_sz)
{
	if(!m_reg_replay.IsValid())
		return false;
	void *buf(0);
	size_t sz(0);
	VS_PKey			key;
	VS_Certificate	cert;
	VS_PKeyCrypt	crypt;
	if(!cert.SetCert(m_reg_replay.GetStrValueRef(CERTIFICATE_PARAM),strlen(m_reg_replay.GetStrValueRef(CERTIFICATE_PARAM))+1,store_PEM_BUF) || !cert.GetCertPublicKey(&key) ||
		!crypt.SetPublicKey(&key))
		return false;
	if(!m_reg_replay.SerializeAlloc(buf,sz))
		return false;

	unsigned char *encrbuf(0);
	uint32_t encrlen(0);
	unsigned char iv[16] = {0};
	unsigned char *sesskey(0);
	uint32_t lensesskey(0);

	crypt.Encrypt((const unsigned char*)buf,sz,alg_sym_AES256,encrbuf,&encrlen,
					iv,sesskey,&lensesskey);
	sesskey = new unsigned char[lensesskey];
	crypt.Encrypt((const unsigned char*)buf,sz,alg_sym_AES256,encrbuf,&encrlen,
					iv,sesskey,&lensesskey);
	encrbuf = new unsigned char[encrlen];
	if(!crypt.Encrypt((const unsigned char*)buf,sz,alg_sym_AES256,encrbuf,&encrlen,
						iv,sesskey,&lensesskey))
	{
		delete [] sesskey;
		delete [] encrbuf;
		free(buf);
		return false;
	}
	free(buf);
	sz = 0;
	buf = 0;

	VS_Container cnt;
	cnt.AddValue("key",(void*)sesskey,lensesskey);
	cnt.AddValue("iv",(void*)iv,16);
	cnt.AddValue("data",encrbuf,encrlen);
	delete [] sesskey;
	delete [] encrbuf;
	if(!cnt.SerializeAlloc(buf,sz))
		return false;
	uint32_t out_sz_(0);
	bool res = !!(buf_out = (void*)VS_Base64EncodeAlloc(buf,sz,out_sz_));
	out_sz = out_sz_;
		free(buf);
	return res;
}
const char *VS_RegOfflineClient::ErrorDescr()
{
	return m_Error.m_str;
}
