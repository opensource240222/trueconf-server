#pragma once

#include "VS_LicensesWrap.h"
#include "transport/Router/VS_TransportRouterServiceHelper.h"

#include <unordered_map>
#include <chrono>

extern VS_LicensesWrap	*p_licWrap;
extern const char SHARED_LIC_KEY_NAME[];
extern const char LICENSE_TAG[];
extern const char LICENSE_MASTER_TAG[];
extern const char ALLOW_ANY_SLAVE_TAG[];

namespace tc3_test { struct SharedLicenseTest; }

/* license */
namespace lic {
	enum class ShareResult : int32_t {
		OK = 0,
		NO_SHARED
	};

	enum class LicenseCheckStatus : int32_t {
		checking,
		no_answer,		// have no answer 10 sec
		not_allowed,	// our server is not on aloowed list
		allowed,		// all ok, we can request license share
		check_failed	//licenses on slaves and masters side are not equal
	};

	extern int32_t REQ_LIMIT;	// can't request more that 20 units of license resource

	class Sharer {
		friend tc3_test::SharedLicenseTest;
	public:
		struct ShareLicenseInfo {
			ShareLicenseInfo() {}
			explicit ShareLicenseInfo(const VS_License& l) :lic(l) {}
			ShareLicenseInfo(const VS_License& l, const std::chrono::system_clock::time_point last_c) :lic(l), last_check(last_c) {}
			VS_License lic;
			std::chrono::system_clock::time_point last_connection;
			std::chrono::system_clock::time_point last_check;
		};

	private:
		// TODO: map with allowed servers
		std::unordered_map<std::string /* server */, ShareLicenseInfo> m_shared_licenses;
		VS_TransportRouterServiceHelper *m_transport = nullptr;
		unsigned short changeLic_requests_to_master = 0;
		std::chrono::steady_clock::time_point m_last_check_request;

		struct ReturnLicenseReq {
			VS_License lic;
			std::chrono::steady_clock::time_point reqTime;
		};
		std::map<int, ReturnLicenseReq> m_licOverhead;
		unsigned m_licID = 1;	// global ctr to identify every license

		bool SaveLicense(const VS_License& l, const std::string& reg_path);
		bool LoadLicense(const std::string& reg_path, VS_License& OUTlic);
		bool LoadLicense(VS_RegistryKey &k, VS_License& OUTlic);

		bool SaveSlavesSharedLicence(const lic::Sharer::ShareLicenseInfo& info, const std::string& server, bool save_connection_time = false);
		bool SaveReceivedSharedLicence(const VS_License& l);
		void LogEvent(string_view object_type, string_view object_name, string_view event_type, string_view message = "{}");
		unsigned GetLicID();

	public:
		Sharer();
		bool Init();
		void SetTransport(VS_TransportRouterServiceHelper *transport);
		void RequestLicense(const VS_License& l, const std::string &master_server);
		void ShareLicense(const ShareResult& r, const std::string &slave_server,const VS_License* l = nullptr);
		void ProcessShareLicenseRequest(const VS_Container & cnt, const std::string & from);
		bool ReceiveLicenseShare(const VS_Container &cnt, uint64_t& recvLicId);
		void RestoreSharedLicenses();
		void ClearSharedLicenseOnMasterSide(const std::string &master);
		void SendSharedLicenseCheck(const std::string &master);										// send periodic check to master, peroid <= 1 hour
		void ReceiveSharedLicenseCheck(const VS_Container& cnt, const std::string &slave_server);	// receive periodic check from slave
		void ReceiveSharedLicenseCheckResponse(const VS_Container& cnt, VS_License& overhead);
		void ObserveSharedResourses();			// master will free shared resources if last connection was more that 1 hour ago
		void ObserveRestriction(const std::pair<std::string /* server */, ShareLicenseInfo>& slave_info);
		void ReturnLicense(const std::string & slave, const VS_License& to_return);
		bool ObserveReceivedSharedResources(VS_License &OUT_to_free);	// slave will free shared resources
		void ObserveLicenseOverhead();
		void ReturnLicenseOverhead(const string_view master_server);
		void SetStartLicenseForSlave();
		void ReturnSharedResoursesToMaster(VS_License& l, const string_view master_server);
		void ReceiveReturnedSharedResources(const VS_Container& cnt, const std::string& slave_server);
		void ReceiveReturnedSharedResourcesResp(const VS_Container& cnt, VS_License& overhead);
		void SlaveConnectedEvent(const string_view slave_id);
		void SlaveDisconnectedEvent(const string_view slave_id);
		void MasterDisconnectedEvent(const string_view master_id);
		void VerifyMasterStatus();

		// helper function
		static bool GetLicense(const VS_Container&cnt, const char* funcName, VS_License& res);

		const static std::chrono::minutes LAST_CHECK_TIMEOUT;
		static std::string GetMasterServer(bool do_resolve = false);
		static std::string ResolveMasterServer();
		static bool s_resolvedMasterActive;
	};
}