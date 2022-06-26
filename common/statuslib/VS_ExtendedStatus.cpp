#include "VS_ExtendedStatus.h"
#include "../std/cpplib/VS_Protocol.h"
#include "std-generic/cpplib/VS_Container_io.h"
#include "../std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_OTHER

namespace
{
	class to_container:public boost::static_visitor<>
	{
		VS_Container &out_container_;
		string_view name_;
		public:
			to_container(string_view name, VS_Container &cnt) : out_container_(cnt), name_(name)
			{}
			template<typename T>
			void operator()(const T& val) const
			{
				out_container_.AddValue(name_,val);
			}

			void operator()(const std::vector<uint8_t> &val) const
			{
				out_container_.AddValue(name_, val.data(), val.size());
			}

			void operator()(const std::string &val) const
			{
				out_container_.AddValue(name_, val);
			}
			void operator()(int32_t val) const
			{
				out_container_.AddValueI32(name_, val);
			}
			void operator()(int64_t val) const
			{
				out_container_.AddValueI64(name_, val);
			}
	};
}
vs::Synchronized<std::set<std::string>> VS_ExtendedStatusStorage::s_sticky_statuses_names;
bool VS_ExtendedStatusStorage::IsStatusSticky(const char *name)
{
	{
		auto cnt = s_sticky_statuses_names.lock();
		if (!cnt->empty())
			return cnt->count(name) > 0;
	}
	string_view status_name = name;
	if (EXTSTATUS_NAME_EXT_STATUS == status_name ||
		EXTSTATUS_NAME_DESCRIPTION == status_name ||
		EXTSTATUS_NAME_LAST_ONLINE_TIME == status_name ||
		EXTSTATUS_NAME_CAMERA == status_name ||
		EXTSTATUS_NAME_IN_CLOUD == status_name ||
		EXTSTATUS_NAME_FWD_TYPE == status_name ||
		EXTSTATUS_NAME_FWD_CALL_ID == status_name ||
		EXTSTATUS_NAME_FWD_TIMEOUT == status_name ||
		EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID == status_name
		)
		return true;
	else
		return false;
}
std::set<std::string> VS_ExtendedStatusStorage::GetStickyStatusesNames()
{
	if (s_sticky_statuses_names->empty())
		return{
			EXTSTATUS_NAME_EXT_STATUS,
			EXTSTATUS_NAME_DESCRIPTION,
			EXTSTATUS_NAME_LAST_ONLINE_TIME,
			EXTSTATUS_NAME_CAMERA,
			EXTSTATUS_NAME_IN_CLOUD,
			EXTSTATUS_NAME_FWD_TYPE,
			EXTSTATUS_NAME_FWD_CALL_ID,
			EXTSTATUS_NAME_FWD_TIMEOUT,
			EXTSTATUS_NAME_FWD_TIMEOUT_CALL_ID
	};
	else
		return *s_sticky_statuses_names.lock();
}
void VS_ExtendedStatusStorage::SetStickyStatusesNames(std::set<std::string> names)
{
	s_sticky_statuses_names = std::move(names);
}
VS_ExtendedStatusStorage::VS_ExtendedStatusStorage()
{
}
VS_ExtendedStatusStorage::VS_ExtendedStatusStorage(VS_ExtendedStatusStorage &&src)
{
	*this = std::move(src);
}
bool VS_ExtendedStatusStorage::UpdateStatus(const VS_Container& statuses)
{
	/**
		Get container by EXTSTATUS_PARAM and parse all statuses
		If EXTSTATUS_PARAM is absent parse cnt_with_statuses;
	*/
	int32_t cause(0);
	if (statuses.GetValue(CAUSE_PARAM, cause) && 1 == cause)
	{
		Clear();
		m_cause = true;
	}
	statuses.Reset();
	while(statuses.Next())
	{
		const char* p = statuses.GetName();
		if (!p)
			continue;
		string_view name = p;
		if (name == CAUSE_PARAM)
			continue;
		if (name == DELETE_EXT_STATUS_PARAM)
		{
			p = statuses.GetStrValueRef();
			if (!p)
				continue;
			DeleteStatus(p);
			continue;
		}
		switch(statuses.GetType())
		{
			case ContainerVT::VS_CNT_BOOL_VT:
				{
					bool res (false);
					if(statuses.GetValue(res))
						UpdateStatus(p, res);
				}
				break;
			case ContainerVT::VS_CNT_INTEGER_VT:
				{
					int32_t res(0);
					if(statuses.GetValue(res))
						UpdateStatus(p, res);
				}
				break;
			case ContainerVT::VS_CNT_INT64_VT:
				{
					int64_t res(0);
					if(statuses.GetValue(res))
						UpdateStatus(p, res);
				}
				break;
			case ContainerVT::VS_CNT_STRING_VT:
				{
					const char * res = statuses.GetStrValueRef();
					if(res != nullptr)
						UpdateStatus(p, std::string(res));
				}
				break;
			case ContainerVT::VS_CNT_BINARY_VT:
				{
					size_t sz = 0;
					const void * buf = statuses.GetBinValueRef(sz);
					if(sz>0 && buf!=nullptr)
					{
						std::vector<uint8_t> res(static_cast<const uint8_t*>(buf),static_cast<const uint8_t*>(buf)+sz);
						UpdateStatus(p, std::move(res));
					}
				}
				break;
			default:
				break;
		}
	}
	return true;
}
bool VS_ExtendedStatusStorage::UpdateStatus(const char *status_name, StatusValueType&& status_value)
{
	auto it = m_statuses.find(status_name);
	if (it != m_statuses.end() && it->second == status_value)
		return true; //no changes
	m_statuses[status_name] = std::move(status_value);
	m_updated_statuses.insert(status_name);
	return true;
}
void VS_ExtendedStatusStorage::DeleteStatus(const std::string &status_name)
{
	auto iter = m_statuses.find(status_name);
	if (iter == m_statuses.end())
		return; //no changes
	m_updated_statuses.insert(status_name);
	m_statuses.erase(iter);
}
VS_ExtendedStatusStorage & VS_ExtendedStatusStorage::operator =(const VS_ExtendedStatusStorage &storage)
{
	m_statuses = storage.m_statuses;
	m_updated_statuses = storage.m_updated_statuses;
	m_cause = storage.m_cause;
	return *this;
}
VS_ExtendedStatusStorage &VS_ExtendedStatusStorage::operator=(VS_ExtendedStatusStorage&&src)
{
	m_statuses = std::move(src.m_statuses);
	m_cause = src.m_cause;
	src.m_cause = false;
	m_updated_statuses = std::move(src.m_updated_statuses);
	return *this;
}

VS_ExtendedStatusStorage &VS_ExtendedStatusStorage::operator +=(const VS_ExtendedStatusStorage &storage)
{
	if (storage.m_cause)
	{
		Clear();
		m_cause = storage.m_cause;
	}
	for(auto& i:storage.m_statuses)
		UpdateStatus(i.first.c_str(),StatusValueType(i.second));
	for (const auto&i : storage.GetDeletedStatuses())
		DeleteStatus(i);
	return *this;
}
bool VS_ExtendedStatusStorage::operator==(const VS_ExtendedStatusStorage &cmp) const
{
	VS_ExtendedStatusStorage temp = *this;
	temp += cmp;
	return IsEqual(temp);
}
bool VS_ExtendedStatusStorage::IsEqual(const VS_ExtendedStatusStorage &cmp) const
{
	return m_statuses == cmp.m_statuses;
}

void VS_ExtendedStatusStorage::ToContainer(VS_Container &cnt, bool insert_all) const
{
	if (m_statuses.empty() || (m_updated_statuses.empty() && !insert_all))
		return;
	VS_Container statuses;
	std::vector<string_view> deleted;
	if (insert_all || m_cause)
	{
		statuses.AddValueI32(CAUSE_PARAM, 1);
		for (const auto &i : m_statuses) boost::apply_visitor(to_container(i.first, statuses), i.second);
		m_cause = false;
	}
	else
	{
		for (const auto &i : m_updated_statuses)
		{
			const auto iter = m_statuses.find(i);
			if (iter != m_statuses.end())
				boost::apply_visitor(to_container(iter->first, statuses), iter->second);
			else
				deleted.emplace_back(i);
		}
	}
	for (const auto i : deleted)
		statuses.AddValue(DELETE_EXT_STATUS_PARAM, i);
	cnt.AddValue(EXTSTATUS_PARAM, statuses);
	m_updated_statuses.clear();
}
auto VS_ExtendedStatusStorage::GetAllStatuses() const -> decltype(m_statuses)
{
	return m_statuses;
}
auto VS_ExtendedStatusStorage::GetChangedStatuses() const -> std::pair<decltype(m_statuses), bool>
{
	auto res = std::make_pair(decltype(m_statuses)(),m_cause);
	for (const auto &i : m_updated_statuses)
	{
		const auto &iter = m_statuses.find(i);
		if(iter!=m_statuses.end())
			res.first.insert(*iter);
	}
	return res;
}
auto VS_ExtendedStatusStorage::GetDeletedStatuses() const -> decltype(m_updated_statuses)
{
	decltype(m_updated_statuses) res;
	for (const auto &i : m_updated_statuses)
		if (m_statuses.find(i) == m_statuses.end())
			res.insert(i);
	return res;
}
void VS_ExtendedStatusStorage::ResetChangesInfo() const
{
	m_updated_statuses.clear();
}
bool VS_ExtendedStatusStorage::GetExtStatus(const char* name, StatusValueType&out)
{
	auto i = m_statuses.find(name);
	if(i == m_statuses.end())
		return false;
	out = i->second;
	return true;
}
bool VS_ExtendedStatusStorage::IsStatusExist(const std::string &name) const
{
	return m_statuses.end() != m_statuses.find(name);
}