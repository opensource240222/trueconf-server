#pragma once
#ifdef _WIN32
#include <memory>
#include <string>

#include "SecureLib/VS_Certificate.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/debuglog/VS_Debug.h"
#include "MdnsResponder.h"
#include "DataFetcherBase.h"
#include "DataFetcherOldACS.h"
#include "std/cpplib/VS_RegistryConst.h"

namespace mdns
{
struct ResponderWrapperOldACS
{
public:
	std::shared_ptr<mdns::Responder> responder;
	std::shared_ptr<mdns::DataFetcherBase> fetcher;

	bool Init(boost::asio::io_service& service, VS_AccessConnectionSystem* acs);
	static bool EnabledInRegistry();
	void Stop();
private:
};
}
#endif
