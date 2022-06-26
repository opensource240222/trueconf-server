#pragma once
#include "std-generic/cpplib/VS_Container.h"
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/synchronized.h"

#include <vector>
#include <string>
#include <map>
#include <set>
#include <boost/variant.hpp>

class VS_ExtendedStatusStorage
{
	static vs::Synchronized<std::set<std::string>> s_sticky_statuses_names;
public:
	using StatusValueType = boost::variant<bool, int32_t, int64_t, std::string, std::vector<uint8_t>>;
private:
	std::map<std::string, StatusValueType> m_statuses;
	mutable std::set<std::string> m_updated_statuses;
	mutable bool m_cause = false;
	bool IsEqual(const VS_ExtendedStatusStorage&) const;
public:
	VS_ExtendedStatusStorage();
	VS_ExtendedStatusStorage(VS_ExtendedStatusStorage&&src);
	VS_ExtendedStatusStorage(const VS_ExtendedStatusStorage &storage): m_statuses(storage.m_statuses),m_updated_statuses(storage.m_updated_statuses),m_cause(storage.m_cause)
	{}
	VS_ExtendedStatusStorage &operator=(const VS_ExtendedStatusStorage &src_storage);
	VS_ExtendedStatusStorage& operator=(VS_ExtendedStatusStorage&&src);
	bool operator==(const VS_ExtendedStatusStorage& comp) const;
	bool UpdateStatus(const VS_Container& statuses);
	bool UpdateStatus(const char *status_name, StatusValueType&& status_value);
	void DeleteStatus(const std::string &status_name);
	VS_ExtendedStatusStorage &operator+=(const VS_ExtendedStatusStorage &storage);
	void Clear()
	{
		m_statuses.clear();
	}
	bool operator!()const
	{
		return m_statuses.empty();
	}
	void ToContainer(VS_Container &cnt, bool insert_all) const;
	bool GetExtStatus(const char *name, StatusValueType &out);
	bool IsStatusExist(const std::string &) const;
	auto GetAllStatuses() const -> decltype(m_statuses);
	auto GetChangedStatuses() const -> std::pair<decltype(m_statuses),bool>;
	auto GetDeletedStatuses() const -> decltype(m_updated_statuses);
	void ResetChangesInfo() const;
	static bool IsStatusSticky(const char *name);
	static void SetStickyStatusesNames(std::set<std::string> allowed_statuses);
	static std::set<std::string> GetStickyStatusesNames();
};