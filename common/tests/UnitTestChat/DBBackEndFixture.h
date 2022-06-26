#pragma once

#include "tests/UnitTestChat/TestHelpers.h"

namespace chat_test
{
class DBBackEndFixture : public ::testing::TestWithParam<DBBackEnd>
{
protected:
	std::string GetConnString() const;
	void CleanChat(const chat::ChatID& chat_id);
};
}
