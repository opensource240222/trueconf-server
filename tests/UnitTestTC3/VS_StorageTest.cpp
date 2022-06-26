#ifdef _WIN32 // not ported
#include <gmock/gmock.h>

#include "AppServer/Services/VS_Storage.h"
#include "AppServer/Services/VS_AppConfRestrict.h"
#include <boost/make_shared.hpp>

namespace tc3_test
{
TEST(Storage, UniqueASConferenceName) {
	using ::testing::EndsWith;
	using ::testing::StrNe;

	const char* our_server = "test.server.name#as";
	VS_Storage s(our_server);
	s.SetConfRestrict(boost::make_shared<VS_AppConfRestrict>());
	s.Init();

	// 1. Insert conference with no name
	VS_ConferenceDescription cd;
	s.InsertConference(cd);

	// 2. Make sure conference name was generated
	ASSERT_TRUE(cd.m_name.Length() > 0);
	ASSERT_THAT(cd.m_name.m_str, EndsWith(our_server));

	// 3. Insert conference with the same name
	auto old_name = cd.m_name;
	s.InsertConference(cd);

	// 4. Make sure new unique name was generated
	ASSERT_TRUE(cd.m_name.Length() > 0);
	ASSERT_THAT(cd.m_name.m_str, StrNe(old_name.m_str));
	ASSERT_THAT(cd.m_name.m_str, EndsWith(our_server));
}
}
#endif