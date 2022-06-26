#ifdef _WIN32
#include "ResponderWrapperOldACS.h"
#include "../std/debuglog/VS_Debug.h"

#include "std/cpplib/MakeShared.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

namespace
{
const char responderError[]="MDNS responder: error: ";
const char responderFatalError[]="MDNS responder: fatal error: ";
const char responderNote[]="MDNS responder: note: ";
}
namespace mdns
{
bool ResponderWrapperOldACS::Init(boost::asio::io_service& service, VS_AccessConnectionSystem* acs)
{
	VS_Certificate certificate;
	VS_RegistryKey rKey(false, CONFIGURATION_KEY, false, true);
	std::unique_ptr<char, free_deleter> certBuf;
	int certBufSz = rKey.GetValue(certBuf, VS_REG_BINARY_VT, SRV_CERT_KEY);
	if (!certificate.SetCert(certBuf.get(), certBufSz, store_PEM_BUF))
	{
		dstream1 << responderFatalError << "Error setting up certificate!\n";
		return false;
	}
	std::string companyName;
	certificate.GetSubjectEntry("organizationName", companyName);
	std::string webURL;
	VS_RegistryKey urlKey{ false, "AppProperties" };
	(void)urlKey.GetString(webURL, "site_url");
	fetcher = std::make_shared<mdns::DataFetcherOldACS>(acs, std::move(companyName), std::move(webURL));
	responder = vs::MakeShared<mdns::Responder>(service, fetcher);
	return true;
}

bool ResponderWrapperOldACS::EnabledInRegistry()
{
	VS_RegistryKey rKey(false, CONFIGURATION_KEY);
	int32_t value = 0;
	return !(rKey.GetValue(&value, sizeof(value), VS_REG_INTEGER_VT, "MdnsResponderEnabled") > 0 && value == 0);
}

void ResponderWrapperOldACS::Stop()
{
	if (responder)
	{
		responder->stop();
		responder.reset();
	}
}

}
#endif
