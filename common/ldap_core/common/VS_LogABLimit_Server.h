#pragma once
#include "VS_LogABLimit_Interface.h"
#include <vector>

class VS_LogABLimit_Server: public VS_LogABLimit_Interface
{

public:
	void LimitByGroupCustomContacts(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group) override;
	void LimitByGroupAllUsers(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group) override;
	void LimitByGroup(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group, const std::string& gid_add_group) override;

	void LimitByUser(const std::string& owner, const unsigned long limit_value) override;

	void WarnByGroup(const std::string& owner, const unsigned long before_size, const unsigned long limit_value) override;
	void WarnByUser(const std::string& owner, const unsigned long limit_value) override;
};