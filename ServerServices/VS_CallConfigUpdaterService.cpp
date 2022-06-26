#include <string>

#include "VS_CallConfigUpdaterService.h"
#include "../common/std/cpplib/VS_RegistryKey.h"
#include "../common/std/cpplib/VS_RegistryConst.h"
#include "../common/std/VS_RegServer.h"
#include "../common/TrueGateway/CallConfig/VS_CallConfigCorrector.h"

const unsigned long VS_CallConfigUpdaterService::CHECK_GRANULARITY_TIMEOUT_MILLIS;

VS_CallConfigUpdaterService::VS_CallConfigUpdaterService()
	: m_srv_start_mode(0)
	, m_registration_service(REGISTRATION_SRV)
{
	m_TimeInterval = std::chrono::milliseconds(CHECK_GRANULARITY_TIMEOUT_MILLIS);
}


VS_CallConfigUpdaterService::~VS_CallConfigUpdaterService()
{
}

void VS_CallConfigUpdaterService::SetConfig(const long mode, const char *reg_service)
{
	m_srv_start_mode = mode;
	m_registration_service = reg_service;
}

bool VS_CallConfigUpdaterService::Init(const char *our_endpoint, const char* /*our_service*/, const bool /*permittedAll*/)
{
	char buff[256] = { 0 };

	VS_RegistryKey	cfg(false, CONFIGURATION_KEY, false, true);
	if (cfg.GetValue(buff, sizeof(buff), VS_REG_STRING_VT, SERVER_MANAGER_TAG) > 0 && *buff) {
		m_SM = buff;
	}

	InitiateUpdate();
	m_our_endpoint = our_endpoint;
	return true;
}

bool VS_CallConfigUpdaterService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	if (recvMess == nullptr)
		return true;

	switch (recvMess->Type())
	{
	case transport::MessageType::Reply:
		{
			VS_Container cnt;
			if (cnt.Deserialize(recvMess->Body(), recvMess->BodySize()))
			{
				const char* method = 0;
				if ((method = cnt.GetStrValueRef(METHOD_PARAM)) != 0)
				{
					// Process methods
					if (strcasecmp(method, UPDATE_CALLCFGCORRECTOR_METHOD) == 0)
					{
						const char *data = cnt.GetStrValueRef(SCRIPT_PARAM);
						if (VS_CallConfigCorrector::IsValidData(data))
						{
							if (VS_CallConfigCorrector::UpdateDataInRegistry(data))
							{
								VS_CallConfigCorrector::GetInstance().UpdateCorrectorData(data);
							}
						}
					}
				}
			}
		}
		break;
		default:
			break;
	}
	return true;
}

bool VS_CallConfigUpdaterService::InitiateUpdate(void)
{
	VS_SimpleStr regSrv = !!m_SM ? m_SM.m_str : RegServerName;
	VS_Container cnt;
	std::string hash;

	cnt.AddValue(METHOD_PARAM, UPDATE_CALLCFGCORRECTOR_METHOD);
	VS_CallConfigCorrector::GetDataHash(hash);
	cnt.AddValue(HASH_PARAM, hash.c_str());

	return PostRequest(regSrv, 0, cnt, 0, m_registration_service);
}

bool VS_CallConfigUpdaterService::Timer(unsigned long /*tickcount*/)
{
	if (m_srv_start_mode == 1 || m_srv_start_mode == 2)
		return true;

	if (VS_CallConfigCorrector::IsObsolete(UPDATE_CHECK_TIMEOUT_SEC))
	{
		VS_CallConfigCorrector::UpdateTimestamp();
		InitiateUpdate();
	}

	return true;
}
