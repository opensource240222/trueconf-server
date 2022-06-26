#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

struct UnitTestTranscoder : public testing::Test{
	virtual void SetUp(){
	}
	virtual void TearDown(){
	}
};
