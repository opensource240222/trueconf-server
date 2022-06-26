#pragma once

#include <string>

static const unsigned int WARNING_AB_MAXSIZE_BY_GROUPS = 700;
static const unsigned int AB_MAXSIZE_BY_GROUPS = 900;
static const unsigned int WARNING_AB_MAXSIZE_BY_CUSTOM_AB = 70;
static const unsigned int AB_MAXSIZE_BY_CUSTOM_AB = 100;

// commented to suppress unused variable warning
//static const char* WARNING_AB_MAXSIZE_BY_GROUPS_STR = "WARNING_AB_MAXSIZE_BY_GROUPS";
//static const char* AB_MAXSIZE_BY_GROUPS_STR = "AB_MAXSIZE_BY_GROUPS";
//static const char* WARNING_AB_MAXSIZE_BY_CUSTOM_AB_STR = "WARNING_AB_MAXSIZE_BY_CUSTOM_AB";
//static const char* AB_MAXSIZE_BY_CUSTOM_AB_STR = "AB_MAXSIZE_BY_CUSTOM_AB";

#define AB_LIMIT(x) VS_LogABLimit_Interface::GetABLimitOrDefault(#x, x)

class VS_LogABLimit_Interface
{
protected:
	VS_LogABLimit_Interface() : m_IsErrorShown(false)
	{}

	enum AB_LIMIT_TYPE
	{
		INVALID_LIMIT_TYPE,
		LIMIT_BY_GROUP_CUSTOM_CONTACTS,
		LIMIT_BY_ALL_USERS,
		LIMIT_BY_GROUP,

		LIMIT_BY_USER,

		WARN_BY_GROUP,
		WARN_BY_USER
	};

	bool m_IsErrorShown;

public:
	virtual ~VS_LogABLimit_Interface() { /*stub*/ };

	bool IsErrorShown() const
	{
		return m_IsErrorShown;
	}

	virtual void LimitByGroupCustomContacts(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group) = 0;
	virtual void LimitByGroupAllUsers(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group) = 0;
	virtual void LimitByGroup(const std::string& owner, const unsigned long before_size, const unsigned long add_items, const unsigned long limit_value, const std::string& gid_from_group, const std::string& gid_add_group) = 0;

	virtual void LimitByUser(const std::string& owner, const unsigned long limit_value) = 0;

	virtual void WarnByGroup(const std::string& owner, const unsigned long before_size, const unsigned long limit_value) = 0;
	virtual void WarnByUser(const std::string& owner, const unsigned long limit_value) = 0;

	static unsigned long GetABLimitOrDefault(const char* key_name, const long def);
};