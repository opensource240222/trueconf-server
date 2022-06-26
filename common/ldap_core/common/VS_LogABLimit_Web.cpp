#include "VS_LogABLimit_Web.h"

void VS_LogABLimit_Web::LimitByGroupCustomContacts(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group)
{
	WebABLimit limit{ LIMIT_BY_GROUP_CUSTOM_CONTACTS, owner, before_size, add_items, limit_value, gid_from_group, "" };
	m_limits.push_back(limit);
}

void VS_LogABLimit_Web::LimitByGroupAllUsers(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group)
{
	WebABLimit limit{ LIMIT_BY_ALL_USERS, owner, before_size, add_items, limit_value, gid_from_group, "" };
	m_limits.push_back(limit);
}

void VS_LogABLimit_Web::LimitByGroup(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group, const std::string& gid_add_group)
{
	WebABLimit limit{ LIMIT_BY_GROUP, owner, before_size, add_items, limit_value, gid_from_group, gid_add_group };
	m_limits.push_back(limit);
}

void VS_LogABLimit_Web::LimitByUser(const std::string& owner, const unsigned long limit_value)
{
	WebABLimit limit{ LIMIT_BY_USER, owner, 0, 0, limit_value, "", "" };
	m_limits.push_back(limit);
}

void VS_LogABLimit_Web::WarnByGroup(const std::string& owner, const unsigned long before_size, const unsigned long limit_value)
{
	WebABLimit limit{ WARN_BY_GROUP, owner, before_size, 0, limit_value, "", "" };
	m_limits.push_back(limit);
}

void VS_LogABLimit_Web::WarnByUser(const std::string& owner, const unsigned long limit_value)
{
	WebABLimit limit{ WARN_BY_USER, owner, 0, 0, limit_value, "", "" };
	m_limits.push_back(limit);
}

std::vector<VS_LogABLimit_Web::WebABLimit> VS_LogABLimit_Web::GetLimits() const
{
	return m_limits;
}