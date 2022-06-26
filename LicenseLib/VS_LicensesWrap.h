#pragma once
#include "../common/std/cpplib/VS_Lock.h"
#include "VS_License.h"

#include <atomic>
#include <map>

class VS_LicensesWrap : public VS_Lock
{
private:
	VS_License	m_licenseSum;
	VS_License	m_received_shared_license;	// for ENTERPRISE_SLAVE
	std::map<uint64_t /*id*/, std::string /*registry key*/> m_licenses;
	bool		m_connectRequired;

	std::atomic<unsigned> m_licenseFails[32] {};
public:
	VS_LicensesWrap() :m_connectRequired(true) {}
	virtual ~VS_LicensesWrap(){}

	/*inline*/ void Clear();
	bool HasLicense(uint64_t id)
	{
		VS_AutoLock	lock(this);
		return m_licenses.find(id) != m_licenses.end();
	}
	/*inline*/ void AddLicense(uint64_t id, std::string key_name);
	/*inline */void SetValidUntil(const std::chrono::system_clock::time_point);

	/*inline*/ void MergeLicense(const VS_License& lic);
	inline const VS_License GetLicSum()
	{
		return m_licenseSum.AddLicenceCopy(m_received_shared_license);
	}
	/*inline*/ const std::chrono::system_clock::time_point GetValidUntil();
	/*inline*/ void PrepareDataForUpdateLic(VS_Container &cnt);
	/*inline*/ bool GetLicenseKey(uint64_t id, std::string& key);
	bool IsConnectRequired()const
	{
		return m_connectRequired;
	}

	void IncrFailCounter(int i) {
		m_licenseFails[i].fetch_add(1, std::memory_order_relaxed);
	}

	unsigned GetFailCount(int i) {
		return m_licenseFails[i].exchange(0, std::memory_order_acq_rel);
	}

	// license sharing
	virtual const VS_License ShareMyLicense(const VS_License& to_share);	// returns how many was shared, it can be less than 'to_share'
	virtual void ReturnBackSharedResourses(const VS_License& what_was_shared);
	virtual void AddSharedLicense(const VS_License& lic);
	virtual void DeductSharedLicense(const VS_License& lic);
	const VS_License GetMyUsedLicenseResources() const;
	virtual bool IsMaster() const;
	virtual bool IsSlave() const;
	virtual const VS_License& GetSharedLicSum() const;
	const VS_License& GetMyLicSum() const;
	virtual void ClearSharedLicense();
};