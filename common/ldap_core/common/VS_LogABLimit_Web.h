#pragma once
#include "VS_LogABLimit_Interface.h"
#include <vector>

class VS_LogABLimit_Web: public VS_LogABLimit_Interface
{
public:
	struct WebABLimit
	{
		AB_LIMIT_TYPE type;
		std::string owner;
		unsigned long before_size;
		unsigned long add_items;
		unsigned long limit_value;

		std::string gid_from_group;
		std::string gid_add_group;
	};
private:
	std::vector<WebABLimit> m_limits;
public:
	void LimitByGroupCustomContacts(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group) override;
	void LimitByGroupAllUsers(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group) override;
	void LimitByGroup(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group, const std::string& gid_add_group) override;

	void LimitByUser(const std::string& owner, const unsigned long limit_value) override;

	void WarnByGroup(const std::string& owner, const unsigned long before_size, const unsigned long limit_value) override;
	void WarnByUser(const std::string& owner, const unsigned long limit_value) override;

	std::vector<WebABLimit> GetLimits() const;
};