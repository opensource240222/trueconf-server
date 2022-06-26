#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "../../SIPParserLib/VS_SIPField_Date.h"
#include "../../SIPParserBase/VS_SIPBuffer.h"
#include "../../TrueGateway/sip/VS_SIPParserInfo.h"

struct SIPFieldDate : public testing::Test
{
	VS_SIPField_Date date_field;
	void SetUp() override
	{
	}
	void TearDown() override
	{
	}
};