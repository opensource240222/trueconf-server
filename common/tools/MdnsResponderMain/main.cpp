#include "MdnsResponder.h"
#include "std/cpplib/VS_RegistryKey.h"
//#include "acs/Lib/VS_AcsLib.cpp"
#include "SecureLib/VS_Certificate.h"
#include "std/cpplib/VS_RegistryConst.cpp"
#include "tools/Server/VS_ApplicationInfo.h"
#include "acs_v2/Service.h"
#include "DataFetcherNewACS.h"
#include "std/cpplib/ASIOThreadPool.h"
#include "std/debuglog/VS_Debug.h"
#include "net/InterfaceInfo.h"

#include <vector>

std::string g_tr_endpoint_name = "seliverstov.hornsandhooves.ru#vcs";

int main(int argc, char** argv)
{
	vs::ASIOThreadPool serviceController(2);
	serviceController.Start();

//	if (!VS_RegistryKey::InitDefaultBackend("registry:force_lm=true"))
	if (!VS_RegistryKey::InitDefaultBackend("postgresql:host=192.168.62.136;port=5433;dbname=TrueConf;user=trueconf;password=123"))
	{
		std::cout << "Error:" << ": Can't initialize registry backend!\n";
		return -1;
	}

	VS_SetDebug(4, 0xffffffff);

	VS_RegistryKey::SetDefaultRoot(VS_TRUECONF_WS_ROOT_KEY_NAME);

	VS_Certificate certificate;
	if (!certificate.Init(prov_OPENSSL))
		std::cout << "Error initialising certificate!" << std::endl;
	VS_RegistryKey rKey(false, CONFIGURATION_KEY, false, true);
	char* certBuf(0);
	int certBufSz = rKey.GetValue((void*&)certBuf, VS_REG_BINARY_VT, "Certificate");
	if (!certificate.SetCert(certBuf, certBufSz, store_PEM_BUF))
		std::cout << "Error setting up certificate!" << std::endl;

	std::shared_ptr<acs::Service> newService = acs::Service::Create(serviceController.get_io_service());
	std::future<bool> future = newService->Start();
	if (std::future_status::ready != future.wait_for(std::chrono::milliseconds(1000)))
	{
		std::cout << "Couldn't start new ACS!" << std::endl;
		return -1;
	}
	const auto interfaces = net::GetInterfaceInfo();
	for (const auto& ii : *interfaces)
	{
		for (const auto& i_addr: ii.addr_local_v4)
			newService->AddListener(i_addr, 4307, net::protocol::TCP);
		for (const auto& i_addr: ii.addr_local_v6)
			newService->AddListener(i_addr, 4307, net::protocol::TCP);
	}
	char name[255] = {0};
	uint32_t size = 255;
	certificate.GetSubjectEntry("organizationName", name, size);
	std::string companyName(name);
	net::interface_info_list interfaces = net::GetInterfaceInfo(true);
	auto data = std::make_shared<mdns::DataFetcherNewACS>(companyName, newService);
	mdns::Responder responder(serviceController.get_io_service(), data);
	responder.start();
	std::this_thread::sleep_for(std::chrono::seconds(3600));
	return 0;
}
