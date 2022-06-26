#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifndef _WIN32	// linux only
#define LDAP_DEPRECATED 1
#include <ldap.h>
#endif

struct UnitTestLDAP : public testing::Test{
	virtual void SetUp(){
	}
	virtual void TearDown(){
	}
};

