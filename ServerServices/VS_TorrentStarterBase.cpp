#include "VS_TorrentStarterBase.h"

#include <boost/filesystem.hpp>
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/debuglog/VS_Debug.h"

#include "transport/Router/TransportRouterInterface.h"

#include "VS_TorrentService.h"
#include "VS_TRStorageInterface.h"
#include "std/cpplib/MakeShared.h"

#define DEBUG_CURRENT_MODULE VS_DM_FILETRANSFER
//#define TEST_ADO_DB

#if defined(TEST_ADO_DB)
#include "../BaseServer/Services/VS_TRStorageAdo.h"
#include "../BaseServer/Services/VS_DBStorage_TrueConf.h"
namespace { std::unique_ptr<VS_DBStorage_TrueConf> _db; }
#endif

namespace
{
	const char TR_PROXY_HANDLER_NAME[] = "TorrentProxy";
}

bool VS_TorrentStarterBase::Start(transport::IRouter *tr, const std::shared_ptr<VS_TRStorageInterface>& dbStorage)
{

	if (m_service) return true;
	if (!dbStorage) return false;
	if (!tr) return false;

	const char   sSd[] = "\t started successfully.", sFd[] = "\t failed.";
	STARTMESS(TORRENT_SRV);

	try {

		auto storage_dir = FileStorageDir(true);

		boost::system::error_code ec;
		boost::filesystem::path file_save_path = boost::filesystem::path(storage_dir);
		if (!boost::filesystem::exists(file_save_path, ec) || !boost::filesystem::is_directory(file_save_path, ec)) {
			return false;
		}

#if defined(TEST_ADO_DB)
		const std::string bs_name = "bs.t.trueconf.com#bs";

		_db = vs::make_unique<VS_DBStorage_TrueConf>();
		_db->Init(bs_name.c_str());

		m_service = vs::MakeShared<VS_TorrentService>(m_ios, tr->EndpointName(), storage_dir, std::make_shared<VS_TRStorageAdo>(_db->dbo_pool()));
#else
		m_service = vs::MakeShared<VS_TorrentService>(m_ios, tr->EndpointName(), storage_dir, dbStorage, m_getServerId);
#endif
	}
	catch (const std::runtime_error &e) {
		puts(e.what());
		return false;
	}

	bool res = tr->AddService(TORRENT_SRV, m_service.get()) && m_service->SetThread();
	if (!res) {
		puts(sFd);
		m_service.reset();
	}
	else {
		m_service->InitTrackersProperty(Listeners_ACS());
		AddHandler_ACS(TR_PROXY_HANDLER_NAME);
		puts(sSd);
	}

	return res;
}

void VS_TorrentStarterBase::Stop(transport::IRouter *tr)
{
	if (!m_service) return;

	m_service->ClearTrackersProperty();
	m_service->ResetThread();
	if (tr) {
		tr->RemoveService(TORRENT_SRV);
	}
	m_service.reset();
	RemoveHandler_ACS(TR_PROXY_HANDLER_NAME);
}

std::string VS_TorrentStarterBase::FileStorageDir(bool create)
{
	char sep = boost::filesystem::path::preferred_separator;
	std::string storage_path = "files";
	storage_path += sep;

	char buffer[1024] = {};

	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);
	if (key.IsValid() && key.GetValue(buffer, sizeof(buffer) - 1, VS_REG_STRING_VT, WORKING_DIRECTORY_TAG_NAME) > 0)
	{
		storage_path = buffer;
		if (storage_path.back() != sep) {
			storage_path += sep;
		}
		storage_path += "files";
		storage_path += sep;
	}

	VS_RegistryKey reg(false, CONFIGURATION_KEY);
	if (reg.GetValue(buffer, sizeof(buffer) - 1, VS_REG_STRING_VT, TR_FILE_STORAGE_KEY) && buffer[0] != 0) {
		storage_path = buffer;
	}
	if (storage_path.back() != sep) {
		storage_path += sep;
	}

	if (create) {
		boost::system::error_code ec;
		boost::filesystem::path file_save_path = boost::filesystem::path(storage_path);
		if (!boost::filesystem::exists(file_save_path, ec) || !boost::filesystem::is_directory(file_save_path, ec)) {
			bool b = boost::filesystem::create_directories(file_save_path, ec);
			if (!b && ec) {
				dstream0 << "Failed to create directory '" << file_save_path << "': " << ec.message();
			}
		}
	}

	return storage_path;
}

inline void VS_TorrentStarterBase::STARTMESS(const char* service)
{
	time_t start_vaule(0);
	time(&start_vaule);
	printf("\t %-20s: at %s", service, ctime(&start_vaule));
}


std::shared_ptr<http::handlers::Interface> VS_TorrentStarterBase::AsHandler()
{
	return std::static_pointer_cast<http::handlers::Interface>(m_service);
}

#undef DEBUG_CURRENT_MODULE
#undef TEST_ADO_DB