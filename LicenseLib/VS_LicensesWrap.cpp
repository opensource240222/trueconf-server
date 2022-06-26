#include "VS_LicensesWrap.h"
#include "../common/std/cpplib/VS_Protocol.h"
#include "AppServer/Services/VS_Storage.h"


/*inline*/ void VS_LicensesWrap::Clear()
{
	VS_AutoLock	lock(this);
	m_licenses.clear();
	m_connectRequired = true;
	VS_License	lic;
	m_licenseSum = lic;
	m_licenseSum.m_error=VSS_LICENSE_NOT_VALID;
}

/*inline*/ void VS_LicensesWrap::AddLicense(uint64_t id, std::string key_name)
{
	VS_AutoLock	lock(this);
	m_licenses.emplace(id, std::move(key_name));
}

/*inline*/ void VS_LicensesWrap::MergeLicense(const VS_License& lic)
{
	VS_AutoLock	lock(this);
	if (lic.IsValid() && !(lic.m_restrict & VS_License::REG_CONNECTED))
		m_connectRequired = false;
	m_licenseSum.MergeLicense(lic);
}
/*inline*/ void VS_LicensesWrap::SetValidUntil(const std::chrono::system_clock::time_point valid_until)
{
	VS_AutoLock	lock(this);
	m_licenseSum.m_validuntil = valid_until;
}
/*inline*/ void VS_LicensesWrap::PrepareDataForUpdateLic(VS_Container &cnt)
{
	VS_AutoLock	lock(this);
	for (const auto& kv : m_licenses)
		cnt.AddValue(NAME_PARAM, &kv.first, sizeof(kv.first));
}

/*inline*/ bool VS_LicensesWrap::GetLicenseKey(uint64_t id, std::string& key)
{
	VS_AutoLock	lock(this);
	const auto it = m_licenses.find(id);
	if (it == m_licenses.end())
		return false;
	key = it->second;
	return true;
}
const VS_License VS_LicensesWrap::ShareMyLicense(const VS_License& to_share)
{
	if (!IsMaster()) return VS_License();

	auto used_by_me = GetMyUsedLicenseResources();
	auto available_license = m_licenseSum.DeductLicenseCopy(used_by_me);

	auto res_license = available_license.ShareAvailable(to_share);
	m_licenseSum.DeductLicense(res_license);
	return res_license;
}
void VS_LicensesWrap::ReturnBackSharedResourses(const VS_License & what_was_shared)
{
	m_licenseSum.AddLicence(what_was_shared);
}
void VS_LicensesWrap::AddSharedLicense(const VS_License& lic)
{
	if (!IsSlave()) return;
	m_received_shared_license.AddLicence(lic);
}
void VS_LicensesWrap::DeductSharedLicense(const VS_License & lic)
{
	if (!IsSlave()) return;
	m_received_shared_license.DeductLicense(lic);
}
const VS_License VS_LicensesWrap::GetMyUsedLicenseResources() const
{
	assert(g_storage != nullptr);

	VS_License lic_sum;
	lic_sum.m_onlineusers = g_storage->CountOnlineUsers();
	lic_sum.m_conferences = g_storage->CountGroupConferences();
	lic_sum.m_gateways = g_storage->CountOnlineGateways();
	lic_sum.m_max_guests = g_storage->CountOnlineGuests();
	lic_sum.m_terminal_pro_users = g_storage->CountOnlineTerminalPro();
	return lic_sum;
}
bool VS_LicensesWrap::IsMaster() const
{
	return m_licenseSum.m_restrict & VS_License::ENTERPRISE_MASTER;
}
bool VS_LicensesWrap::IsSlave() const
{
	return  m_licenseSum.m_restrict & VS_License::ENTERPRISE_SLAVE;
}
const VS_License & VS_LicensesWrap::GetSharedLicSum() const
{
	return m_received_shared_license;
}
const VS_License & VS_LicensesWrap::GetMyLicSum() const
{
	return m_licenseSum;
}
void VS_LicensesWrap::ClearSharedLicense()
{
	m_received_shared_license.Clear();
}
/*inline*/ const std::chrono::system_clock::time_point VS_LicensesWrap::GetValidUntil()
{
	VS_AutoLock	lock(this);
	return m_licenseSum.m_validuntil;
}
