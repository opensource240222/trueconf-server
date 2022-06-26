#pragma once

#include "tests/common/GMockOverride.h"
#include "newtransport/Router/SecureProvider.h"
#include "SecureLib/VS_PublicKeyCrypt.h"

#include <gmock/gmock.h>

namespace transport_test {

class SecureProviderMock : public transport::SecureProvider
{
public:
	void DelegateTo(SecureProvider* impl)
	{
		using ::testing::_;
		using ::testing::Invoke;

		ON_CALL(*this, ReadCrypt()).WillByDefault(Invoke(impl, &transport::SecureProvider::ReadCrypt));
		ON_CALL(*this, WriteCrypt()).WillByDefault(Invoke(impl, &transport::SecureProvider::WriteCrypt));
		ON_CALL(*this, StartSecureHandshake(_,_,_,_)).WillByDefault(Invoke(impl, &transport::SecureProvider::StartSecureHandshake));
		ON_CALL(*this, HandlePacket(_,_)).WillByDefault(Invoke(impl, &transport::SecureProvider::HandlePacket));
		ON_CALL(*this, GetPacket()).WillByDefault(Invoke(impl, &transport::SecureProvider::GetPacket));
	}

	void SetState(State state)
	{
		m_state = state;
	}

	MOCK_CONST_METHOD0_OVERRIDE(ReadCrypt, const VS_SymmetricCrypt*());
	MOCK_CONST_METHOD0_OVERRIDE(WriteCrypt, const VS_SymmetricCrypt*());
	MOCK_METHOD4_OVERRIDE(StartSecureHandshake, void(unsigned version, VS_SecureHandshakeType type, VS_PKey& key, const VS_Container& cert_chain));
	MOCK_METHOD2_OVERRIDE(HandlePacket, size_t(const void* data, size_t size));
	MOCK_METHOD0_OVERRIDE(GetPacket, std::vector<uint8_t>());
};

}
