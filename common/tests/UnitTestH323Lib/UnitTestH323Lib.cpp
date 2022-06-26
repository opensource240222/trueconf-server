#include "UnitTestH323Lib.h"
#include "tools/H323Gateway/Lib/src/VS_AsnBuffers.h"
#include "tools/H323Gateway/Lib/src/VS_RasMessages.h"
#include "std-generic/cpplib/string_view.h"
#include "tools/H323Gateway/Lib/src/VS_Q931.h"
#include "tools/H323Gateway/Lib/VS_H323Lib.h"
#include "std-generic/compat/iterator.h"

#define EXPECT_ARRAY_EQ(TARTYPE, actual, reference, element_count) \
    {\
    TARTYPE* reference_ = static_cast<TARTYPE *> (reference); \
    TARTYPE* actual_ = static_cast<TARTYPE *> (actual); \
    for(std::size_t cmp_i = 0; cmp_i < element_count; cmp_i++ ){\
		EXPECT_EQ(reference_[cmp_i], actual_[cmp_i]) << "x and y differ at index " << cmp_i;\
	}\
    }\


static const auto SIZE_DISPLAY_NAME = 82 + 1 ;
static const auto SIZE_E163_NAME = 50;

namespace
{
	const unsigned char REGISTRATION_REQUEST[] = {
		0x0e, 0x80, 0x3e, 0x0a, 0x06, 0x00, 0x08, 0x91,
		0x4a, 0x00, 0x04, 0x00, 0x01, 0x00, 0x0a, 0x82,
		0x01, 0xb7, 0x06, 0xb8, 0x01, 0x00, 0x0a, 0x82,
		0x01, 0xb7, 0x06, 0xb7, 0x22, 0xc0, 0xb8, 0x00,
		0x00, 0x00, 0x0f, 0x54, 0x72, 0x75, 0x65, 0x43,
		0x6f, 0x6e, 0x66, 0x20, 0x47, 0x61, 0x74, 0x65,
		0x77, 0x61, 0x79, 0x02, 0x34, 0x2e, 0x33, 0x00,
		0x02, 0x40, 0x05, 0x00, 0x73, 0x00, 0x64, 0x00,
		0x66, 0x00, 0x73, 0x00, 0x64, 0x00, 0x66, 0x01,
		0x00, 0x44, 0x46, 0xb8, 0x00, 0x00, 0x00, 0x0f,
		0x54, 0x72, 0x75, 0x65, 0x43, 0x6f, 0x6e, 0x66,
		0x20, 0x47, 0x61, 0x74, 0x65, 0x77, 0x61, 0x79,
		0x02, 0x34, 0x2e, 0x33, 0x2e, 0xab, 0x00, 0x0a,
		0x03, 0x40, 0x01, 0x2b, 0x54, 0x02, 0x04, 0x05,
		0x00, 0x73, 0x00, 0x64, 0x00, 0x66, 0x00, 0x73,
		0x00, 0x64, 0x00, 0x66, 0xc0, 0x5b, 0x1a, 0x98,
		0x9e, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
		0x02, 0x05, 0x00, 0x80, 0x80, 0x1b, 0x97, 0xe8,
		0xcd, 0x68, 0x41, 0x98, 0x00, 0xfa, 0xf7, 0x31,
		0xfb, 0xfd, 0xdc, 0x04, 0x23, 0x00, 0x10, 0x44,
		0x4c, 0x5b, 0x1a, 0x98, 0x9e, 0x08, 0x2a, 0x86,
		0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05, 0x00, 0x80,
		0x80, 0x0f, 0x52, 0xc8, 0x22, 0x1f, 0xc0, 0x1d,
		0x5c, 0xd1, 0xb2, 0xe4, 0x3f, 0x1d, 0x31, 0x88,
		0x1a, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
		0x00, 0x01, 0x00 };

	const unsigned char REGISTRATION_REJECT[] = {
		0x14, 0x80, 0x3e, 0x0a, 0x06, 0x00, 0x08, 0x91,
		0x4a, 0x00, 0x04, 0x81, 0x01, 0x00, 0x1c, 0x00,
		0x54, 0x00, 0x72, 0x00, 0x75, 0x00, 0x65, 0x00,
		0x43, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x66, 0x00,
		0x20, 0x00, 0x53, 0x00, 0x65, 0x00, 0x72, 0x00,
		0x76, 0x00, 0x65, 0x00, 0x72 };


	const unsigned char REGISTRATION_CONFIRM[] = {
		0x12, 0xc0, 0xc2, 0x31, 0x06, 0x00, 0x08, 0x91,
		0x4a, 0x00, 0x04, 0x01, 0x00, 0x0a, 0x82, 0x01,
		0xb2, 0x06, 0xb8, 0x01, 0x40, 0x00, 0x00, 0x31,
		0x1c, 0x00, 0x54, 0x00, 0x72, 0x00, 0x75, 0x00,
		0x65, 0x00, 0x43, 0x00, 0x6f, 0x00, 0x6e, 0x00,
		0x66, 0x00, 0x20, 0x00, 0x53, 0x00, 0x65, 0x00,
		0x72, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x3e,
		0x00, 0x38, 0x00, 0x41, 0x00, 0x31, 0x00, 0x45,
		0x00, 0x44, 0x00, 0x35, 0x00, 0x44, 0x00, 0x36,
		0x00, 0x42, 0x00, 0x39, 0x00, 0x35, 0x00, 0x33,
		0x00, 0x44, 0x00, 0x32, 0x00, 0x41, 0x00, 0x43,
		0x00, 0x33, 0x00, 0x35, 0x00, 0x32, 0x00, 0x43,
		0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x45,
		0x00, 0x39, 0x00, 0x38, 0x00, 0x41, 0x00, 0x41,
		0x00, 0x45, 0x00, 0x35, 0x00, 0x33, 0x00, 0x35,
		0x20, 0x8a, 0x80, 0x03, 0x40, 0x01, 0x2b, 0x01,
		0x00, 0x01, 0x00, 0x01, 0x00 };

	const unsigned char UNREGISTRATION_REQUEST[] = {
		0x1a, 0x40, 0xc2, 0x3a, 0x01, 0x00, 0x0a, 0x82,
		0x01, 0x29, 0x06, 0xb8, 0x3e, 0x00, 0x38, 0x00,
		0x41, 0x00, 0x31, 0x00, 0x45, 0x00, 0x44, 0x00,
		0x35, 0x00, 0x44, 0x00, 0x36, 0x00, 0x42, 0x00,
		0x39, 0x00, 0x35, 0x00, 0x33, 0x00, 0x44, 0x00,
		0x32, 0x00, 0x41, 0x00, 0x43, 0x00, 0x33, 0x00,
		0x35, 0x00, 0x32, 0x00, 0x43, 0x00, 0x30, 0x00,
		0x30, 0x00, 0x30, 0x00, 0x45, 0x00, 0x39, 0x00,
		0x38, 0x00, 0x41, 0x00, 0x41, 0x00, 0x45, 0x00,
		0x35, 0x00, 0x33, 0x00, 0x35, 0x14, 0x80, 0x00,
		0x1f, 0x1c, 0x00, 0x54, 0x00, 0x72, 0x00, 0x75,
		0x00, 0x65, 0x00, 0x43, 0x00, 0x6f, 0x00, 0x6e,
		0x00, 0x66, 0x00, 0x20, 0x00, 0x53, 0x00, 0x65,
		0x00, 0x72, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72 };

	const unsigned char UNREGISTRATION_CONFIRM[] = {
		0x1c, 0xc2, 0x3a };

	const unsigned char UNREGISTRATION_REJECT[] = {
		0x20, 0x09, 0x08, 0x00 };


	const unsigned char DISENGAGE_REQUEST[] = {
		0x3e, 0x6a, 0x0a, 0x10, 0x00, 0x34, 0x00, 0x36,
		0x00, 0x30, 0x00, 0x31, 0x00, 0x5f, 0x00, 0x65,
		0x00, 0x6e, 0x00, 0x64, 0x00, 0x70, 0x38, 0xa0,
		0x39, 0xf6, 0xe1, 0xb4, 0x24, 0x13, 0x57, 0xce,
		0x24, 0x5e, 0x33, 0x2e, 0x83, 0x42, 0x02, 0x99,
		0x23, 0x21, 0x00, 0x11, 0x00, 0x38, 0xa0, 0x39,
		0xf6, 0xe1, 0xb4, 0x24, 0x13, 0x57, 0xce, 0x24,
		0x5e, 0x33, 0x2e, 0x83, 0x42, 0x01, 0x00
	};
}

TEST_F(H225RasMessageDecodeTest, H225RasRegistrationRequest)
{
	VS_PerBuffer buff{ REGISTRATION_REQUEST, vs::size(REGISTRATION_REQUEST) * 8 };
	VS_RasMessage mess{};
	ASSERT_TRUE(mess.Decode(buff));
	ASSERT_EQ(mess.tag, VS_H225RasMessage::e_registrationRequest);
	auto rrq = dynamic_cast<VS_RasRegistrationRequest*>(mess.choice);
	ASSERT_TRUE(rrq);

	ASSERT_EQ(rrq->requestSeqNum.value, 15883);
	ASSERT_EQ(rrq->timeToLive.value, 300);
	ASSERT_EQ(rrq->keepAlive.value, false);
	ASSERT_EQ(rrq->willSupplyUUIEs.value, false);
	ASSERT_EQ(rrq->maintainConnection.value, false);
	VS_AsnNull asn_null;
	ASSERT_EQ(asn_null, rrq->restart);

	const std::uint32_t str_identifier[VS_AsnObjectId::max_values] = { 0, 0, 8, 2250, 0, 4 };
	EXPECT_ARRAY_EQ(const std::uint32_t, rrq->protocolIdentifier.value, str_identifier, VS_AsnObjectId::max_values);

	ASSERT_EQ(rrq->discoveryComplete.value, false);
	//callSignalAddress
	ASSERT_EQ(rrq->callSignalAddress.size(), 1);
	ASSERT_EQ(rrq->callSignalAddress[0].tag, VS_H225TransportAddress::e_ipAddress);
	VS_H225TransportAddress_IpAddress *transport_address = rrq->callSignalAddress[0];
	ASSERT_TRUE(transport_address);
	auto ip_data = static_cast<unsigned char *>(transport_address->ip.value.GetData());

	const unsigned char ip_call_signl_addr[]{ 10, 130, 1, 183 };
	ASSERT_EQ(vs::size(ip_call_signl_addr), transport_address->ip.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, ip_data, ip_call_signl_addr, transport_address->ip.value.ByteSize());

	ASSERT_EQ(transport_address->port.value, 1720);

	//rasAddress
	ASSERT_EQ(rrq->rasAddress.size(), 1);
	ASSERT_EQ(rrq->rasAddress[0].tag, VS_H225TransportAddress::e_ipAddress);
	transport_address = rrq->rasAddress[0];
	ASSERT_TRUE(transport_address);
	ip_data = static_cast<unsigned char *>((*transport_address).ip.value.GetData());
	const unsigned char ip_ras_addr[] = { 10,130,1,183 };
	ASSERT_EQ(vs::size(ip_ras_addr), transport_address->ip.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, ip_data, ip_ras_addr, transport_address->ip.value.ByteSize());

	EXPECT_EQ((*transport_address).port.value, 1719);

	//terminalType
	EXPECT_EQ((string_view{ static_cast<char*>(rrq->terminalType.vendor.productId.value.GetData()), rrq->terminalType.vendor.productId.value.
		ByteSize() }), "TrueConf Gateway");
	EXPECT_EQ((string_view{ (static_cast<char*>(rrq->terminalType.vendor.versionId.value.GetData())), rrq->terminalType.vendor.versionId.value.
		ByteSize() }), "4.3");
	EXPECT_EQ(rrq->terminalType.mc.value, false);
	EXPECT_EQ(rrq->terminalType.undefinedNode.value, false);
	EXPECT_EQ(rrq->terminalType.vendor.h221NonStandard.t35CountryCode.value, 184);
	EXPECT_EQ(rrq->terminalType.vendor.h221NonStandard.t35Extension.value, 0);
	EXPECT_EQ(rrq->terminalType.vendor.h221NonStandard.manufacturerCode.value, 0);

	const char * const array_names[] = { "sdfsdf", "111" };
	const unsigned alias_item_type[] = { VS_H225AliasAddress::e_h323_ID, VS_H225AliasAddress::e_dialedDigits };
	{
		ASSERT_EQ(vs::size(array_names), rrq->terminalAlias.size());
		for (std::size_t i = 0; i < rrq->terminalAlias.size(); ++i)
		{
			ASSERT_EQ(alias_item_type[i], rrq->terminalAlias[i].tag);
			const auto terminal_addr = rrq->terminalAlias[i].String();
			EXPECT_EQ(terminal_addr.length(), ::strlen(array_names[i]));
			EXPECT_STREQ(terminal_addr.c_str(), array_names[i]);
		}
	}
	{
		const std::uint32_t algorithm_OID[VS_AsnObjectId::max_values] = { 1, 2, 840, 113549, 2, 5 };
		const std::size_t size_item_hash = 16;
		const unsigned char item_hash[][size_item_hash] =
		{
			{ 0x1b, 0x97, 0xe8, 0xcd, 0x68, 0x41, 0x98, 0x00, 0xfa, 0xf7, 0x31, 0xfb, 0xfd, 0xdc, 0x04, 0x23 },
			{ 0x0f, 0x52, 0xc8, 0x22, 0x1f, 0xc0, 0x1d, 0x5c, 0xd1, 0xb2, 0xe4, 0x3f, 0x1d, 0x31, 0x88, 0x1a }
		};

		ASSERT_EQ(vs::size(item_hash), rrq->cryptoTokens.size());
		const std::uint32_t time_stamp = 1528469663;
		const unsigned item_type[] = { VS_H225CryptoH323Token::e_cryptoEPPwdHash, VS_H225CryptoH323Token::e_cryptoEPPwdHash };

		for (std::size_t i = 0; i < rrq->cryptoTokens.size(); ++i)
		{
			ASSERT_EQ(item_type[i], rrq->cryptoTokens[i].tag);
			VS_H225CryptoEPPwdHash *crypto_token = static_cast<VS_H225CryptoEPPwdHash *>(rrq->cryptoTokens[i].choice);
			ASSERT_EQ(alias_item_type[i], crypto_token->alias.tag);

			auto alias_str = crypto_token->alias.String();
			EXPECT_EQ(alias_str.length(), ::strlen(array_names[i]));
			EXPECT_STREQ(alias_str.c_str(), array_names[i]);

			EXPECT_EQ(crypto_token->timestamp.value, time_stamp);
			EXPECT_ARRAY_EQ(const std::uint32_t, crypto_token->token.algorithmOID.value, algorithm_OID, VS_AsnObjectId::max_values);

			const auto hash_data = static_cast<unsigned char *>(crypto_token->token.hash.value.GetData());
			ASSERT_EQ(crypto_token->token.hash.value.ByteSize(), size_item_hash);
			EXPECT_ARRAY_EQ(const unsigned char, hash_data, item_hash[i], size_item_hash);
		}
	}
}


TEST_F(H225RasMessageDecodeTest, H225RasRegistrationReject)
{
	VS_PerBuffer buff{ REGISTRATION_REJECT, vs::size(REGISTRATION_REJECT) * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));
	ASSERT_EQ(mess.tag, VS_H225RasMessage::e_registrationReject);
	auto rrj = dynamic_cast<VS_RasRegistrationReject*>(mess.choice);
	ASSERT_TRUE(rrj);

	ASSERT_EQ(rrj->requestSeqNum.value, 15883);
	const std::uint32_t str_identifier[VS_AsnObjectId::max_values] = { 0, 0, 8, 2250, 0, 4 };
	EXPECT_ARRAY_EQ(const std::uint32_t, rrj->protocolIdentifier.value, str_identifier, VS_AsnObjectId::max_values);
	ASSERT_EQ(rrj->rejectReason.tag, VS_H225RegistrationRejectReason::e_resourceUnavailable);
	auto &&unavailable = *static_cast<VS_AsnNull *>(rrj->rejectReason.choice);
	const VS_AsnNull null;
	ASSERT_TRUE((null == unavailable));
	const char gatekeeper_ident_expect[] = "TrueConf Server";
	auto gatekeeper_ident = VS_H323String(rrj->gatekeeperIdentifier.value).MakeString();
	EXPECT_EQ(gatekeeper_ident.length(), sizeof(gatekeeper_ident_expect) - 1);
	EXPECT_STREQ(gatekeeper_ident.c_str(), gatekeeper_ident_expect);
}


TEST_F(H225RasMessageDecodeTest, H225RasRegistrationConfirm)
{
	VS_PerBuffer buff{ REGISTRATION_CONFIRM, vs::size(REGISTRATION_CONFIRM) * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));
	ASSERT_EQ(mess.tag, VS_H225RasMessage::e_registrationConfirm);
	auto rcf = dynamic_cast<VS_RasRegistrationConfirm*>(mess.choice);
	ASSERT_TRUE(rcf);

	EXPECT_EQ(rcf->requestSeqNum.value, 49714);
	const std::uint32_t str_identifier[VS_AsnObjectId::max_values] = { 0, 0, 8, 2250, 0, 4 };
	EXPECT_ARRAY_EQ(const std::uint32_t, rcf->protocolIdentifier.value, str_identifier, VS_AsnObjectId::max_values);

	//callSignalAddress
	ASSERT_EQ(rcf->callSignalAddress[0].tag, VS_H225TransportAddress::e_ipAddress);
	VS_H225TransportAddress_IpAddress *transport_address = rcf->callSignalAddress[0];
	ASSERT_TRUE(transport_address);
	const auto ip_data = static_cast<unsigned char *>(transport_address->ip.value.GetData());
	const unsigned char ip_call_signl_addr[]{ 10, 130, 1, 178 };
	ASSERT_EQ(transport_address->ip.value.ByteSize(), vs::size(ip_call_signl_addr));
	EXPECT_ARRAY_EQ(const unsigned char, ip_data, ip_call_signl_addr, transport_address->ip.value.ByteSize());
	EXPECT_EQ(transport_address->port.value, 1720);

	//terminalAlias
	const char * const array_names[] = { "1" };
	const unsigned alias_item_type[] = { VS_H225AliasAddress::e_h323_ID };
	{
		ASSERT_EQ(vs::size(array_names), rcf->terminalAlias.size());
		for (std::size_t i = 0; i < rcf->terminalAlias.size(); ++i)
		{
			ASSERT_EQ(alias_item_type[i], rcf->terminalAlias[i].tag);
			auto terminal_alias = rcf->terminalAlias[i].String();
			EXPECT_EQ(terminal_alias.length(), ::strlen(array_names[i]));
			EXPECT_STREQ(terminal_alias.c_str(), array_names[i]);
		}
	}

	const char gatekeeper_ident_expect[] = "TrueConf Server";
	const auto gatekeeper_ident = VS_H323String(rcf->gatekeeperIdentifier.value).MakeString();

	EXPECT_EQ(gatekeeper_ident.length(), sizeof(gatekeeper_ident_expect) - 1);
	EXPECT_STREQ(gatekeeper_ident.c_str(), gatekeeper_ident_expect);

	const char endpoint_ident_expect[] = "8A1ED5D6B953D2AC352C000E98AAE535";
	const auto endpoint_ident = VS_H323String(rcf->endpointIdentifier.value).MakeString();

	EXPECT_EQ(endpoint_ident.length(), sizeof(endpoint_ident_expect) - 1);
	EXPECT_STREQ(endpoint_ident.c_str(), endpoint_ident_expect);

	EXPECT_EQ(rcf->timeToLive.value, 300);
	EXPECT_EQ(rcf->willRespondToIRR.value, false);
	EXPECT_EQ(rcf->maintainConnection.value, false);
}


TEST_F(H225RasMessageDecodeTest, H224RasUnregistrationRequest)
{
	VS_PerBuffer buff{ UNREGISTRATION_REQUEST, vs::size(UNREGISTRATION_REQUEST) * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));
	ASSERT_EQ(mess.tag, VS_H225RasMessage::e_unregistrationRequest);
	auto urq = dynamic_cast<VS_RasUnregistrationRequest*>(mess.choice);
	ASSERT_TRUE(urq);

	EXPECT_EQ(urq->requestSeqNum.value, 49723);

	//callSignalAddress
	ASSERT_EQ(urq->callSignalAddress[0].tag, VS_H225TransportAddress::e_ipAddress);
	VS_H225TransportAddress_IpAddress *transport_address = urq->callSignalAddress[0];
	ASSERT_TRUE(transport_address);
	const auto ip_data = static_cast<unsigned char *>(transport_address->ip.value.GetData());
	const unsigned char ip_call_signl_addr[]{ 10, 130, 1, 41 };
	EXPECT_ARRAY_EQ(const unsigned char, ip_data, ip_call_signl_addr, transport_address->ip.value.ByteSize());

	EXPECT_EQ(transport_address->port.value, 1720);

	const char gatekeeper_ident_expect[] = "TrueConf Server";
	const auto gatekeeper_ident = VS_H323String(urq->gatekeeperIdentifier.value).MakeString();

	EXPECT_EQ(gatekeeper_ident.length(), sizeof(gatekeeper_ident_expect) - 1);
	EXPECT_STREQ(gatekeeper_ident.c_str(), gatekeeper_ident_expect);

	const char endpoint_ident_expect[] = "8A1ED5D6B953D2AC352C000E98AAE535";
	const auto endpoint_ident = VS_H323String(urq->endpointIdentifier.value).MakeString();

	EXPECT_EQ(endpoint_ident.length(), sizeof(endpoint_ident_expect) - 1);
	EXPECT_STREQ(endpoint_ident.c_str(), endpoint_ident_expect);
}


TEST_F(H225RasMessageDecodeTest, H224RasUnregistrationConfirm)
{
	VS_PerBuffer buff{ UNREGISTRATION_CONFIRM, vs::size(UNREGISTRATION_CONFIRM) * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));
	ASSERT_EQ(mess.tag, VS_H225RasMessage::e_unregistrationConfirm);
	auto ucf = dynamic_cast<VS_RasUnregistrationConfirm*>(mess.choice);
	ASSERT_TRUE(ucf);

	EXPECT_EQ(ucf->requestSeqNum.value, 49723);
}

TEST_F(H225RasMessageDecodeTest, H224RasUnregistrationReject)
{
	VS_PerBuffer buff{ UNREGISTRATION_REJECT, vs::size(UNREGISTRATION_REJECT) * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));
	ASSERT_EQ(mess.tag, VS_H225RasMessage::e_unregistrationReject);
	auto urj = dynamic_cast<VS_RasUnregistrationReject*>(mess.choice);
	ASSERT_TRUE(urj);

	EXPECT_EQ(urj->requestSeqNum.value, 2313);

	ASSERT_EQ(urj->rejectReason.tag, VS_H225UnregRejectReason::e_notCurrentlyRegistered);
	auto &&unavailable = *static_cast<VS_AsnNull *>(urj->rejectReason.choice);
	const VS_AsnNull null;
	EXPECT_TRUE((null == unavailable));
}

TEST_F(H225RasMessageDecodeTest, H225RasDisengageRequest)
{
	VS_PerBuffer buff{ DISENGAGE_REQUEST, sizeof(DISENGAGE_REQUEST) * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));
	ASSERT_EQ(mess.tag, VS_H225RasMessage::e_disengageRequest);

	auto drq = dynamic_cast<VS_RasDisengageRequest*>(mess.choice);
	ASSERT_TRUE(drq);

	EXPECT_EQ(drq->requestSeqNum.value, 27147);
	const unsigned char endpoint_identifier[] = {
		0x00, 0x34, 0x00, 0x36, 0x00, 0x30, 0x00, 0x31,
		0x00, 0x5f, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x64,
		0x00, 0x70
	};
	ASSERT_EQ(vs::size(endpoint_identifier), drq->endpointIdentifier.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, drq->endpointIdentifier.value.GetData(), endpoint_identifier, drq->endpointIdentifier.value.ByteSize());

	const unsigned char conference_id[] = {
		0x38, 0xa0, 0x39, 0xf6, 0xe1, 0xb4, 0x24, 0x13,
		0x57, 0xce, 0x24, 0x5e, 0x33, 0x2e, 0x83, 0x42
	};

	ASSERT_EQ(vs::size(conference_id), drq->conferenceID.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, drq->conferenceID.value.GetData(), conference_id, drq->conferenceID.value.ByteSize());
	EXPECT_EQ(drq->callReferenceValue.value, 665);
	EXPECT_EQ(drq->disengageReason.tag, VS_H225DisengageReason::e_normalDrop);
	const VS_AsnNull null;
	const auto disengage_reason = static_cast<VS_AsnNull*>(drq->disengageReason.choice);
	EXPECT_TRUE((null == *disengage_reason));

	const std::uint8_t guid[] = { 0x38, 0xa0, 0x39, 0xf6, 0xe1, 0xb4, 0x24, 0x13,
		0x57, 0xce, 0x24, 0x5e, 0x33, 0x2e, 0x83, 0x42 };

	const auto guid_actual = static_cast<unsigned char *>(drq->callIdentifier.guid.value.GetData());
	ASSERT_EQ(vs::size(guid), drq->callIdentifier.guid.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, guid_actual, guid, drq->callIdentifier.guid.value.ByteSize());

	EXPECT_EQ(drq->answeredCall.value, false);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace
{
	const unsigned char SETUP[] = {
		0x08, 0x02, 0x48, 0x23, 0x05, 0x04, 0x04, 0x88,
		0x18, 0x9e, 0xa5, 0x28, 0x1b, 0x74, 0x65, 0x73,
		0x74, 0x31, 0x40, 0x76, 0x79, 0x64, 0x72, 0x31,
		0x31, 0x31, 0x2e, 0x74, 0x72, 0x75, 0x65, 0x63,
		0x6f, 0x6e, 0x66, 0x2e, 0x6e, 0x61, 0x6d, 0x65,
		0x7e, 0x02, 0x6f, 0x05, 0x20, 0xb8, 0x06, 0x00,
		0x08, 0x91, 0x4a, 0x00, 0x04, 0x01, 0x40, 0x1a,
		0x00, 0x74, 0x00, 0x65, 0x00, 0x73, 0x00, 0x74,
		0x00, 0x31, 0x00, 0x40, 0x00, 0x76, 0x00, 0x79,
		0x00, 0x64, 0x00, 0x72, 0x00, 0x31, 0x00, 0x31,
		0x00, 0x31, 0x00, 0x2e, 0x00, 0x74, 0x00, 0x72,
		0x00, 0x75, 0x00, 0x65, 0x00, 0x63, 0x00, 0x6f,
		0x00, 0x6e, 0x00, 0x66, 0x00, 0x2e, 0x00, 0x6e,
		0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x22, 0xc0,
		0xb8, 0x00, 0x00, 0x00, 0x0f, 0x54, 0x72, 0x75,
		0x65, 0x43, 0x6f, 0x6e, 0x66, 0x20, 0x47, 0x61,
		0x74, 0x65, 0x77, 0x61, 0x79, 0x02, 0x34, 0x2e,
		0x33, 0x00, 0x01, 0x40, 0x15, 0x00, 0x23, 0x00,
		0x68, 0x00, 0x64, 0x00, 0x78, 0x00, 0x36, 0x00,
		0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x40, 0x00,
		0x31, 0x00, 0x39, 0x00, 0x32, 0x00, 0x2e, 0x00,
		0x31, 0x00, 0x36, 0x00, 0x38, 0x00, 0x2e, 0x00,
		0x36, 0x00, 0x32, 0x00, 0x2e, 0x00, 0x34, 0x00,
		0x35, 0x00, 0xc0, 0xa8, 0x3e, 0x2d, 0x06, 0xb8,
		0x00, 0x9a, 0xa1, 0xc2, 0x3a, 0x39, 0x64, 0x43,
		0x0f, 0x25, 0x36, 0x77, 0xb0, 0x48, 0x44, 0x16,
		0x7e, 0x00, 0xcd, 0x4d, 0x80, 0x00, 0x07, 0x00,
		0x0a, 0x82, 0x01, 0xb2, 0xaa, 0x23, 0x11, 0x00,
		0x9a, 0xa1, 0xc2, 0x3a, 0x39, 0x64, 0x43, 0x0f,
		0x25, 0x36, 0x77, 0xb0, 0x48, 0x44, 0x16, 0x7e,
		0x81, 0x9c, 0x02, 0x00, 0x00, 0x07, 0x00, 0x08,
		0x81, 0x6b, 0x00, 0x03, 0x18, 0x10, 0x00, 0x07,
		0x00, 0x08, 0x81, 0x6b, 0x00, 0x03, 0x2b, 0x00,
		0x04, 0x00, 0x0c, 0xa3, 0x53, 0x62, 0xba, 0xab,
		0x17, 0x05, 0x9e, 0x1d, 0x49, 0x81, 0x71, 0x60,
		0x25, 0x73, 0xe4, 0x20, 0x08, 0x5a, 0x73, 0xe2,
		0xfa, 0x18, 0xda, 0x08, 0x8e, 0xe8, 0x09, 0xeb,
		0x62, 0x1b, 0xef, 0x71, 0x87, 0xc4, 0x6d, 0xd6,
		0x17, 0xc1, 0x4e, 0xdf, 0xef, 0x22, 0xcf, 0x0f,
		0xeb, 0x81, 0xee, 0x6e, 0x41, 0x3f, 0xa9, 0x1a,
		0xe9, 0x85, 0x02, 0x90, 0x0a, 0xd5, 0xb1, 0x53,
		0xe5, 0x45, 0x4a, 0x1d, 0x27, 0x07, 0xca, 0xf7,
		0x6e, 0x4b, 0x33, 0x71, 0x2b, 0x79, 0x2e, 0x6f,
		0xae, 0xa6, 0x8f, 0xde, 0x44, 0x3b, 0x99, 0x75,
		0xe5, 0xcc, 0x97, 0x57, 0x9e, 0xf1, 0x46, 0x2c,
		0x62, 0xf9, 0xa0, 0x7f, 0x08, 0xaa, 0xae, 0x6a,
		0x76, 0xad, 0x8e, 0x8f, 0x3b, 0x7d, 0x4b, 0x80,
		0x09, 0xc9, 0xfb, 0x8b, 0x42, 0xc7, 0x67, 0x40,
		0xff, 0x2e, 0xd0, 0xe7, 0xfb, 0xf8, 0xbc, 0xbe,
		0x40, 0xb1, 0x04, 0x00, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xc9, 0x0f, 0xda, 0xa2,
		0x21, 0x68, 0xc2, 0x34, 0xc4, 0xc6, 0x62, 0x8b,
		0x80, 0xdc, 0x1c, 0xd1, 0x29, 0x02, 0x4e, 0x08,
		0x8a, 0x67, 0xcc, 0x74, 0x02, 0x0b, 0xbe, 0xa6,
		0x3b, 0x13, 0x9b, 0x22, 0x51, 0x4a, 0x08, 0x79,
		0x8e, 0x34, 0x04, 0xdd, 0xef, 0x95, 0x19, 0xb3,
		0xcd, 0x3a, 0x43, 0x1b, 0x30, 0x2b, 0x0a, 0x6d,
		0xf2, 0x5f, 0x14, 0x37, 0x4f, 0xe1, 0x35, 0x6d,
		0x6d, 0x51, 0xc2, 0x45, 0xe4, 0x85, 0xb5, 0x76,
		0x62, 0x5e, 0x7e, 0xc6, 0xf4, 0x4c, 0x42, 0xe9,
		0xa6, 0x37, 0xed, 0x6b, 0x0b, 0xff, 0x5c, 0xb6,
		0xf4, 0x06, 0xb7, 0xed, 0xee, 0x38, 0x6b, 0xfb,
		0x5a, 0x89, 0x9f, 0xa5, 0xae, 0x9f, 0x24, 0x11,
		0x7c, 0x4b, 0x1f, 0xe6, 0x49, 0x28, 0x66, 0x51,
		0xec, 0xe6, 0x53, 0x81, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0x04, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00,
		0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x80,
		0x01, 0x00
	};

	const unsigned char ALERTING[] = {
		0x23, 0x80, 0x06, 0x00, 0x08, 0x91, 0x4a,
		0x00, 0x05, 0x22, 0xc0, 0xb5, 0x00, 0x23, 0x31,
		0x0a, 0x48, 0x44, 0x58, 0x20, 0x36, 0x30, 0x30,
		0x30, 0x20, 0x48, 0x44, 0x17, 0x52, 0x65, 0x6c,
		0x65, 0x61, 0x73, 0x65, 0x20, 0x2d, 0x20, 0x33,
		0x2e, 0x30, 0x2e, 0x31, 0x5f, 0x6e, 0x65, 0x2d,
		0x31, 0x30, 0x36, 0x32, 0x38, 0x01, 0xb0, 0xd8,
		0x00, 0x11, 0x00, 0x9a, 0xa1, 0xc2, 0x3a, 0x39,
		0x64, 0x43, 0x0f, 0x25, 0x36, 0x77, 0xb0, 0x48,
		0x44, 0x16, 0x7e, 0x01, 0x00, 0x01, 0x80, 0x01,
		0x00, 0x01, 0x40, 0x10, 0x80, 0x01, 0x00
	};


	const unsigned char CONNECT[] = {
		0x08, 0x02, 0xc8, 0x23,
		0x07, 0x28, 0x07, 0x68, 0x64, 0x78, 0x36, 0x30,
		0x30, 0x30, 0x7e, 0x00, 0x6e, 0x05, 0x22, 0xc0,
		0x06, 0x00, 0x08, 0x91, 0x4a, 0x00, 0x05, 0x00,
		0xc0, 0xa8, 0x3e, 0x2d, 0x9d, 0x5b, 0x22, 0xc0,
		0xb5, 0x00, 0x23, 0x31, 0x0a, 0x48, 0x44, 0x58,
		0x20, 0x36, 0x30, 0x30, 0x30, 0x20, 0x48, 0x44,
		0x17, 0x52, 0x65, 0x6c, 0x65, 0x61, 0x73, 0x65,
		0x20, 0x2d, 0x20, 0x33, 0x2e, 0x30, 0x2e, 0x31,
		0x5f, 0x6e, 0x65, 0x2d, 0x31, 0x30, 0x36, 0x32,
		0x38, 0x00, 0x9a, 0xa1, 0xc2, 0x3a, 0x39, 0x64,
		0x43, 0x0f, 0x25, 0x36, 0x77, 0xb0, 0x48, 0x44,
		0x16, 0x7e, 0x1d, 0x0c, 0xc0, 0x11, 0x00, 0x9a,
		0xa1, 0xc2, 0x3a, 0x39, 0x64, 0x43, 0x0f, 0x25,
		0x36, 0x77, 0xb0, 0x48, 0x44, 0x16, 0x7e, 0x01,
		0x00, 0x01, 0x80, 0x01, 0x00, 0x01, 0x40, 0x10,
		0x80, 0x01, 0x00 };

	const unsigned char RELEASE_COMPLETE[] = {
		0x25, 0x00, 0x06, 0x00, 0x08, 0x91, 0x4a, 0x00,
		0x04, 0x10, 0x04, 0x01, 0x00
	};

}


TEST_F(H225MessageDecodeTest, H225Setup)
{
	VS_PerBuffer buff{ SETUP, sizeof(SETUP) * 8 };

	VS_Q931 q931;
	ASSERT_TRUE(q931.DecodeMHeader(static_cast<VS_BitBuffer&>(buff)));
	ASSERT_EQ(q931.protocolDiscriminator, VS_Q931::e_causeIE);
	ASSERT_EQ(q931.lengthCallReference, 2);
	ASSERT_EQ(q931.messageType, VS_Q931::e_setupMsg);
	//need test call reference value and e.t.

	std::uint8_t dn[SIZE_DISPLAY_NAME] = { 0 };
	std::uint8_t e164[SIZE_E163_NAME] = { 0 };
	ASSERT_TRUE(q931.GetUserUserIE(buff, dn, e164));

	const char display_name[] = "test1@vydr111.trueconf.name";
	EXPECT_EQ((string_view{ display_name, sizeof(display_name) - 1 }), (reinterpret_cast<char *>(dn)));
	EXPECT_TRUE((string_view{ reinterpret_cast<char*>(e164) }.empty()));

	VS_CsH323UserInformation ui;
	ASSERT_TRUE(ui.Decode(buff));
	ASSERT_EQ(ui.h323UuPdu.h323MessageBody.tag, VS_CsH323MessageBody::e_setup);
	VS_CsSetupUuie *setup = ui.h323UuPdu.h323MessageBody;
	ASSERT_TRUE(setup);

	const std::uint32_t str_identifier[VS_AsnObjectId::max_values] = { 0, 0, 8, 2250, 0, 4 };
	EXPECT_ARRAY_EQ(const std::uint32_t, setup->protocolIdentifier.value, str_identifier, VS_AsnObjectId::max_values);

	ASSERT_EQ(setup->sourceAddress.size(), 1);
	const auto src_addr = setup->sourceAddress[0].String();
	EXPECT_EQ(src_addr.length(), sizeof(display_name) - 1);
	EXPECT_STREQ(src_addr.c_str(), display_name);

	//TODO:h.225. Manufacturer maybe bug
	EXPECT_EQ((string_view{ static_cast<char*>(setup->sourceInfo.vendor.productId.value.GetData()),
		setup->sourceInfo.vendor.productId.value.ByteSize() }), "TrueConf Gateway");
	EXPECT_EQ((string_view{ (static_cast<char*>(setup->sourceInfo.vendor.versionId.value.GetData())),
		setup->sourceInfo.vendor.versionId.value.ByteSize() }), "4.3");
	EXPECT_EQ(setup->sourceInfo.vendor.h221NonStandard.t35CountryCode.value, 184);
	EXPECT_EQ(setup->sourceInfo.vendor.h221NonStandard.t35Extension.value, 0);
	EXPECT_EQ(setup->sourceInfo.vendor.h221NonStandard.manufacturerCode.value, 0);
	EXPECT_EQ(!!setup->sourceInfo.mc.value, false);
	EXPECT_EQ(!!setup->sourceInfo.undefinedNode.value, false);

	ASSERT_EQ(setup->destinationAddress.size(), 1);
	ASSERT_EQ(setup->destinationAddress[0].tag, VS_H225AliasAddress::e_h323_ID);
	const char dst_addr_expect[] = "#hdx6000@192.168.62.45";
	const auto dst_addr = setup->destinationAddress[0].String();
	EXPECT_EQ(dst_addr.length(), sizeof(dst_addr_expect) - 1);
	EXPECT_STREQ(dst_addr.c_str(), dst_addr_expect);

	ASSERT_EQ(setup->destCallSignalAddress.tag, VS_H225TransportAddress::e_ipAddress);
	VS_H225TransportAddress_IpAddress *ip_address = setup->destCallSignalAddress;
	ASSERT_TRUE(ip_address);

	const unsigned char ip[] = { 192, 168, 62, 45 };
	ASSERT_EQ(vs::size(ip), ip_address->ip.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, ip_address->ip.value.GetData(), ip, ip_address->ip.value.ByteSize());

	EXPECT_EQ(ip_address->port.value, 1720);

	EXPECT_EQ(setup->activeMC.value, false);

	const std::uint8_t conference_id[] = { 0x9a, 0xa1, 0xc2, 0x3a, 0x39, 0x64, 0x43, 0x0f, 0x25, 0x36, 0x77, 0xb0, 0x48, 0x44, 0x16, 0x7e };
	ASSERT_EQ(vs::size(conference_id), setup->conferenceID.value.ByteSize());
	EXPECT_ARRAY_EQ(const std::uint8_t, setup->conferenceID.value.GetData(), conference_id, setup->conferenceID.value.ByteSize());


 	const VS_AsnNull null{};

	ASSERT_EQ(setup->conferenceGoal.tag, VS_CsSetupUuie_ConferenceGoal::e_create);
	auto &conference_goal = (*static_cast<VS_AsnNull *>(setup->conferenceGoal.choice));
	ASSERT_TRUE((conference_goal == null));

	ASSERT_EQ(setup->callType.tag, VS_H225CallType::e_pointToPoint);
	auto &call_type = (*static_cast<VS_AsnNull *>(setup->callType.choice));
	ASSERT_TRUE((call_type == null));

	ASSERT_EQ(setup->sourceCallSignalAddress.tag, VS_H225TransportAddress::e_ipAddress);
	VS_H225TransportAddress_IpAddress *source_ip_address = setup->sourceCallSignalAddress;
	ASSERT_TRUE(source_ip_address);

	const unsigned char source_ip[] = { 10, 130, 1, 178 };
	ASSERT_EQ(vs::size(source_ip), source_ip_address->ip.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, source_ip_address->ip.value.GetData(), source_ip, source_ip_address->ip.value.ByteSize());

	EXPECT_EQ(source_ip_address->port.value, 43555);

	const std::uint8_t guid[] = { 0x9a, 0xa1, 0xc2, 0x3a, 0x39, 0x64, 0x43, 0x0f, 0x25, 0x36, 0x77, 0xb0, 0x48, 0x44, 0x16, 0x7e };

	const auto guid_actual = static_cast<unsigned char *>(setup->callIdentifier.guid.value.GetData());
	ASSERT_EQ(vs::size(guid), setup->callIdentifier.guid.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, setup->callIdentifier.guid.value.GetData(), guid_actual, setup->callIdentifier.guid.value.ByteSize());

	ASSERT_EQ(setup->tokens.size(), 2);
	const std::uint32_t tokens[][VS_AsnObjectId::max_values] =
	{
		{0, 0, 8, 235, 0, 3, 24},
		{0 ,0 ,8, 235, 0, 3, 43}
	};

	for(decltype(setup->tokens.size()) i = 0; i < setup->tokens.size(); ++i)
	{
		EXPECT_ARRAY_EQ(const std::uint32_t, setup->tokens[i].tokenOID.value, tokens[i], VS_AsnObjectId::max_values);
	}


	const unsigned char halfkey[] = {
		0x0c, 0xa3, 0x53, 0x62, 0xba, 0xab, 0x17, 0x05,
		0x9e, 0x1d, 0x49, 0x81, 0x71, 0x60, 0x25, 0x73,
		0xe4, 0x20, 0x08, 0x5a, 0x73, 0xe2, 0xfa, 0x18,
		0xda, 0x08, 0x8e, 0xe8, 0x09, 0xeb, 0x62, 0x1b,
		0xef, 0x71, 0x87, 0xc4, 0x6d, 0xd6, 0x17, 0xc1,
		0x4e, 0xdf, 0xef, 0x22, 0xcf, 0x0f, 0xeb, 0x81,
		0xee, 0x6e, 0x41, 0x3f, 0xa9, 0x1a, 0xe9, 0x85,
		0x02, 0x90, 0x0a, 0xd5, 0xb1, 0x53, 0xe5, 0x45,
		0x4a, 0x1d, 0x27, 0x07, 0xca, 0xf7, 0x6e, 0x4b,
		0x33, 0x71, 0x2b, 0x79, 0x2e, 0x6f, 0xae, 0xa6,
		0x8f, 0xde, 0x44, 0x3b, 0x99, 0x75, 0xe5, 0xcc,
		0x97, 0x57, 0x9e, 0xf1, 0x46, 0x2c, 0x62, 0xf9,
		0xa0, 0x7f, 0x08, 0xaa, 0xae, 0x6a, 0x76, 0xad,
		0x8e, 0x8f, 0x3b, 0x7d, 0x4b, 0x80, 0x09, 0xc9,
		0xfb, 0x8b, 0x42, 0xc7, 0x67, 0x40, 0xff, 0x2e,
		0xd0, 0xe7, 0xfb, 0xf8, 0xbc, 0xbe, 0x40, 0xb1
	};

	ASSERT_EQ(vs::size(halfkey), setup->tokens[1].dhkey.halfkey.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, setup->tokens[1].dhkey.halfkey.value.GetData(), halfkey, setup->tokens[1].dhkey.halfkey.value.ByteSize());

	const unsigned char mod_size[] = { 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xc9, 0x0f, 0xda, 0xa2,
		0x21, 0x68, 0xc2, 0x34, 0xc4, 0xc6, 0x62, 0x8b,
		0x80, 0xdc, 0x1c, 0xd1, 0x29, 0x02, 0x4e, 0x08,
		0x8a, 0x67, 0xcc, 0x74, 0x02, 0x0b, 0xbe, 0xa6,
		0x3b, 0x13, 0x9b, 0x22, 0x51, 0x4a, 0x08, 0x79,
		0x8e, 0x34, 0x04, 0xdd, 0xef, 0x95, 0x19, 0xb3,
		0xcd, 0x3a, 0x43, 0x1b, 0x30, 0x2b, 0x0a, 0x6d,
		0xf2, 0x5f, 0x14, 0x37, 0x4f, 0xe1, 0x35, 0x6d,
		0x6d, 0x51, 0xc2, 0x45, 0xe4, 0x85, 0xb5, 0x76,
		0x62, 0x5e, 0x7e, 0xc6, 0xf4, 0x4c, 0x42, 0xe9,
		0xa6, 0x37, 0xed, 0x6b, 0x0b, 0xff, 0x5c, 0xb6,
		0xf4, 0x06, 0xb7, 0xed, 0xee, 0x38, 0x6b, 0xfb,
		0x5a, 0x89, 0x9f, 0xa5, 0xae, 0x9f, 0x24, 0x11,
		0x7c, 0x4b, 0x1f, 0xe6, 0x49, 0x28, 0x66, 0x51,
		0xec, 0xe6, 0x53, 0x81, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff };

	ASSERT_EQ(vs::size(mod_size),  setup->tokens[1].dhkey.modSize.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, setup->tokens[1].dhkey.modSize.value.GetData(), mod_size, setup->tokens[1].dhkey.modSize.value.ByteSize());

	const unsigned char generator[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02
	};
	ASSERT_EQ(vs::size(generator), setup->tokens[1].dhkey.generator.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, setup->tokens[1].dhkey.generator.value.GetData(), generator, setup->tokens[1].dhkey.generator.value.ByteSize());

	ASSERT_EQ(setup->mediaWaitForConnect.value, false);
	ASSERT_EQ(setup->canOverlapSend.value, false);
	ASSERT_EQ(setup->multipleCalls.value, false);
	ASSERT_EQ(setup->maintainConnection.value, false);
}

TEST_F(H225MessageDecodeTest, H225Alerting)
{
	VS_PerBuffer buff{ ALERTING, sizeof(ALERTING) * 8 };

	//TODO: no tests for q931. - failed

	VS_CsH323UserInformation ui;
	ASSERT_TRUE(ui.Decode(buff));
	ASSERT_EQ(ui.h323UuPdu.h323MessageBody.tag, VS_CsH323MessageBody::e_alerting);
	VS_CsAlertingUuie *alerting = ui.h323UuPdu.h323MessageBody;
	ASSERT_TRUE(alerting);

	const std::uint32_t str_identifier[VS_AsnObjectId::max_values] = { 0, 0, 8, 2250, 0, 5 };
	EXPECT_ARRAY_EQ(const std::uint32_t, alerting->protocolIdentifier.value, str_identifier, VS_AsnObjectId::max_values);

	ASSERT_EQ((string_view{ static_cast<char*>(alerting->destinationInfo.vendor.productId.value.GetData()),
		alerting->destinationInfo.vendor.productId.value.ByteSize() }), "HDX 6000 HD");
	ASSERT_EQ((string_view{ (static_cast<char*>(alerting->destinationInfo.vendor.versionId.value.GetData())),
		alerting->destinationInfo.vendor.versionId.value.ByteSize() }), "Release - 3.0.1_ne-10628");

	//TODO:h.225. Manufacturer maybe bug
	EXPECT_EQ(!!alerting->destinationInfo.mc.value, false);
	EXPECT_EQ(!!alerting->destinationInfo.undefinedNode.value, false);

	const std::uint8_t guid[] = { 0x9a, 0xa1, 0xc2, 0x3a, 0x39, 0x64, 0x43, 0x0f, 0x25, 0x36, 0x77, 0xb0, 0x48, 0x44, 0x16, 0x7e };
	const auto guid_actual = static_cast<unsigned char *>(alerting->callIdentifier.guid.value.GetData());
	EXPECT_ARRAY_EQ(const std::uint8_t, guid_actual, guid, alerting->callIdentifier.guid.value.ByteSize());

	EXPECT_EQ(!!alerting->multipleCalls.value, false);
	EXPECT_EQ(!!alerting->maintainConnection.value, true);

	ASSERT_EQ(alerting->presentationIndicator.tag, VS_H225PresentationIndicator::e_presentationAllowed);
	auto &present_indicator = *(static_cast<VS_AsnNull *>(alerting->presentationIndicator.choice));

	const VS_AsnNull null{};
	EXPECT_TRUE((null == present_indicator));
	EXPECT_EQ(alerting->screeningIndicator.value, VS_H225ScreeningIndicator::e_userProvidedVerifiedAndFaile);
}


TEST_F(H225MessageDecodeTest, H225Connect)
{
	VS_PerBuffer buff{ CONNECT, sizeof(CONNECT) * 8 };

	VS_Q931 q931;
	ASSERT_TRUE(q931.DecodeMHeader(static_cast<VS_BitBuffer&>(buff)));
	ASSERT_EQ(q931.protocolDiscriminator, VS_Q931::e_causeIE);
	ASSERT_EQ(q931.lengthCallReference, 2);
	ASSERT_EQ(q931.messageType, VS_Q931::e_connectMsg);
	//need test call reference value and e.tc

	std::uint8_t dn[SIZE_DISPLAY_NAME] = { 0 };
	std::uint8_t e164[SIZE_E163_NAME] = { 0 };
	ASSERT_TRUE(q931.GetUserUserIE(buff, dn, e164));

	const char display_name[] = "hdx6000";
	EXPECT_EQ((string_view{ display_name, vs::size(display_name) - 1}),  reinterpret_cast<char *>(dn));
	EXPECT_TRUE((string_view{ reinterpret_cast<char*>(e164)}.empty()));


	VS_CsH323UserInformation ui;
	ASSERT_TRUE(ui.Decode(buff));
	ASSERT_EQ(ui.h323UuPdu.h323MessageBody.tag, VS_CsH323MessageBody::e_connect);
	VS_CsConnectUuie *connect = ui.h323UuPdu.h323MessageBody;

	const std::uint32_t str_identifier[VS_AsnObjectId::max_values] = { 0, 0, 8, 2250, 0, 5 };
	EXPECT_ARRAY_EQ(const std::uint32_t, connect->protocolIdentifier.value, str_identifier, VS_AsnObjectId::max_values);

	ASSERT_EQ(connect->h245Address.tag, VS_H225TransportAddress::e_ipAddress);
	VS_H225TransportAddress_IpAddress* h245_address = connect->h245Address;
	ASSERT_TRUE(h245_address);

	const auto ip_data = static_cast<unsigned char *>(h245_address->ip.value.GetData());
	const unsigned char ip_call_signl_addr[]{ 192, 168, 62, 45 };
	ASSERT_EQ(vs::size(ip_call_signl_addr), h245_address->ip.value.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, ip_data, ip_call_signl_addr, h245_address->ip.value.ByteSize());

	EXPECT_EQ(h245_address->port.value, 40283);

	EXPECT_EQ((string_view{ static_cast<char*>(connect->destinationInfo.vendor.productId.value.GetData()),
		connect->destinationInfo.vendor.productId.value.ByteSize() }), "HDX 6000 HD");
	EXPECT_EQ((string_view{ (static_cast<char*>(connect->destinationInfo.vendor.versionId.value.GetData())),
		connect->destinationInfo.vendor.versionId.value.ByteSize() }), "Release - 3.0.1_ne-10628");

	//TODO:h.225. Manufacturer maybe bug: 0xb5002331(sniff) != 2331(our code).
	EXPECT_EQ(!!connect->destinationInfo.mc.value, false);
	EXPECT_EQ(!!connect->destinationInfo.undefinedNode.value, false);

	 auto conference_id_actual = static_cast<unsigned char *>(connect->conferenceID.value.GetData());

	const std::uint8_t conference_id[] = { 0x9a, 0xa1, 0xc2, 0x3a, 0x39, 0x64, 0x43, 0x0f, 0x25, 0x36, 0x77, 0xb0, 0x48, 0x44, 0x16, 0x7e };
	ASSERT_EQ(vs::size(conference_id), connect->conferenceID.value.ByteSize());
	EXPECT_ARRAY_EQ(const std::uint8_t, conference_id_actual, conference_id, connect->conferenceID.value.ByteSize());

	conference_id_actual = static_cast<unsigned char *>(connect->callIdentifier.guid.value.GetData());
	ASSERT_EQ(vs::size(conference_id), connect->callIdentifier.guid.value.ByteSize());
	EXPECT_ARRAY_EQ(const std::uint8_t, conference_id_actual, conference_id, connect->callIdentifier.guid.value.ByteSize());

	EXPECT_EQ(!!connect->multipleCalls.value, false);
	EXPECT_EQ(!!connect->maintainConnection.value, true);

	ASSERT_EQ(connect->presentationIndicator.tag, VS_H225PresentationIndicator::e_presentationAllowed);
	auto &present_indicator = *(static_cast<VS_AsnNull *>(connect->presentationIndicator.choice));

	const VS_AsnNull null{};
	EXPECT_TRUE((null == present_indicator));
	EXPECT_EQ(connect->screeningIndicator.value, VS_H225ScreeningIndicator::e_userProvidedVerifiedAndFaile);
}

TEST_F(H225MessageDecodeTest, H225ReleaseComplete)
{
	VS_PerBuffer buff{ RELEASE_COMPLETE, sizeof(RELEASE_COMPLETE) * 8 };

	VS_CsH323UserInformation ui;
	ASSERT_TRUE(ui.Decode(buff));
	ASSERT_EQ(ui.h323UuPdu.h323MessageBody.tag, VS_CsH323MessageBody::e_releaseComplete);
	VS_CsReleaseCompleteUuie *release_complete = ui.h323UuPdu.h323MessageBody;
	ASSERT_TRUE(release_complete);

	const std::uint32_t protocol_identifier[VS_AsnObjectId::max_values] = {
		0, 0, 8, 2250, 0, 4
	};
	EXPECT_ARRAY_EQ(const std::uint32_t, release_complete->protocolIdentifier.value, protocol_identifier, VS_AsnObjectId::max_values);
	//Tunnelling
}


//////////////////////////////////////////////////////////////////////////

namespace
{
	const unsigned char TERMINAL_CAPABILITY_SET[] = {
		0x02, 0x70, 0x01, 0x06, 0x00, 0x08, 0x81, 0x75,
		0x00, 0x0d, 0x80, 0x34, 0x80, 0x00, 0x3c, 0x00,
		0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00,
		0x04, 0x02, 0x18, 0x02, 0x08, 0x09, 0x7c, 0x10,
		0x08, 0x00, 0x08, 0x81, 0x71, 0x00, 0x00, 0x00,
		0x00, 0x10, 0x08, 0x00, 0x08, 0x81, 0x71, 0x00,
		0x00, 0x00, 0x01, 0x07, 0x60, 0x06, 0x30, 0x00,
		0x24, 0x00, 0x00, 0x40, 0x01, 0x00, 0x01, 0x00,
		0x1a, 0x80, 0x00, 0x00, 0x86, 0x09, 0x00, 0x00,
		0x06, 0x00, 0x08, 0x81, 0x6f, 0x01, 0x01, 0x80,
		0x00, 0x01, 0x86, 0x21, 0x10, 0xc0, 0xb5, 0x00,
		0x00, 0x01, 0x01, 0xac, 0x06, 0x00, 0x12, 0x00,
		0x01, 0x00, 0x22, 0x00, 0x96, 0x00, 0x32, 0x00,
		0x96, 0x00, 0x42, 0x00, 0x34, 0x00, 0x52, 0x00,
		0x0a, 0x00, 0x62, 0x05, 0x78, 0x80, 0x00, 0x02,
		0x24, 0x30, 0x20, 0x60, 0xc0, 0xb5, 0x00, 0x00,
		0x01, 0x01, 0xad, 0x40, 0xfa, 0x00, 0x02, 0x0c,
		0xb5, 0x00, 0x00, 0x01, 0x01, 0x01, 0x20, 0x00,
		0x01, 0x0c, 0xb5, 0x00, 0x00, 0x01, 0x01, 0x01,
		0x20, 0x00, 0x01, 0x80, 0x00, 0x03, 0x24, 0x30,
		0x16, 0x60, 0xc0, 0xb5, 0x00, 0x00, 0x01, 0x01,
		0xa7, 0x40, 0xfa, 0x00, 0x01, 0x0c, 0xb5, 0x00,
		0x00, 0x01, 0x01, 0x01, 0x20, 0x00, 0x01, 0x80,
		0x00, 0x04, 0x24, 0x30, 0x16, 0x60, 0xc0, 0xb5,
		0x00, 0x00, 0x01, 0x01, 0xa6, 0x40, 0xbb, 0x80,
		0x01, 0x0c, 0xb5, 0x00, 0x00, 0x01, 0x01, 0x01,
		0x20, 0x00, 0x01, 0x80, 0x00, 0x05, 0x24, 0x30,
		0x16, 0x60, 0xc0, 0xb5, 0x00, 0x00, 0x01, 0x01,
		0xa5, 0x40, 0x7d, 0x00, 0x01, 0x0c, 0xb5, 0x00,
		0x00, 0x01, 0x01, 0x01, 0x20, 0x00, 0x01, 0x80,
		0x00, 0x06, 0x24, 0x30, 0x15, 0x60, 0x00, 0x07,
		0x00, 0x07, 0xb8, 0x35, 0x01, 0x01, 0x00, 0x40,
		0x01, 0xe0, 0x02, 0x00, 0x12, 0x00, 0x01, 0x00,
		0x21, 0x70, 0x80, 0x00, 0x07, 0x24, 0x30, 0x16,
		0x60, 0xc0, 0xb5, 0x00, 0x00, 0x01, 0x01, 0x93,
		0x40, 0xbb, 0x80, 0x01, 0x0c, 0xb5, 0x00, 0x00,
		0x01, 0x01, 0x01, 0x20, 0x00, 0x01, 0x80, 0x00,
		0x08, 0x24, 0x30, 0x11, 0x60, 0x00, 0x06, 0x00,
		0x07, 0xb8, 0x35, 0x01, 0x00, 0x40, 0x7d, 0x00,
		0x01, 0x00, 0x12, 0x00, 0x01, 0x80, 0x00, 0x09,
		0x24, 0x30, 0x11, 0x60, 0x00, 0x06, 0x00, 0x07,
		0xb8, 0x35, 0x01, 0x00, 0x40, 0x5d, 0xc0, 0x01,
		0x00, 0x12, 0x00, 0x01, 0x80, 0x00, 0x0a, 0x24,
		0x30, 0x16, 0x60, 0xc0, 0xb5, 0x00, 0x00, 0x01,
		0x01, 0x96, 0x40, 0x3e, 0x80, 0x01, 0x0c, 0xb5,
		0x00, 0x00, 0x01, 0x01, 0x01, 0x20, 0x00, 0x01,
		0x80, 0x00, 0x0b, 0x21, 0x40, 0x13, 0x80, 0x00,
		0x0c, 0x22, 0x40, 0x07, 0x80, 0x00, 0x0d, 0x20,
		0xc0, 0x13, 0x80, 0x00, 0x0e, 0x20, 0x40, 0x13,
		0x80, 0x00, 0x0f, 0x22, 0xc0, 0x01, 0x80, 0x00,
		0x10, 0x0c, 0x00, 0x25, 0x60, 0x00, 0x07, 0x00,
		0x08, 0x81, 0x71, 0x00, 0x00, 0x01, 0x40, 0x4b,
		0x00, 0x06, 0x02, 0x91, 0x40, 0x02, 0xa2, 0x00,
		0x24, 0x00, 0x32, 0x00, 0xd8, 0x00, 0x62, 0x00,
		0x40, 0x00, 0x42, 0x00, 0x0f, 0x00, 0xa2, 0x00,
		0x0d, 0x80, 0x00, 0x11, 0x0c, 0x00, 0x25, 0x60,
		0x00, 0x07, 0x00, 0x08, 0x81, 0x71, 0x00, 0x00,
		0x01, 0x40, 0x4b, 0x00, 0x06, 0x02, 0x91, 0x08,
		0x02, 0xa2, 0x00, 0x24, 0x00, 0x32, 0x00, 0xd8,
		0x00, 0x62, 0x00, 0x40, 0x00, 0x42, 0x00, 0x0f,
		0x00, 0xa2, 0x00, 0x0d, 0x80, 0x00, 0x12, 0x09,
		0xf8, 0x00, 0x00, 0x40, 0x4a, 0xff, 0x00, 0x70,
		0x40, 0x01, 0x00, 0x80, 0x00, 0x13, 0x08, 0xb0,
		0x00, 0x4a, 0xff, 0x40, 0x00, 0x14, 0x0c, 0x00,
		0x0c, 0x10, 0xc0, 0xb5, 0x00, 0x00, 0x01, 0x01,
		0xab, 0x01, 0x00, 0x11, 0x09, 0x80, 0x00, 0x15,
		0x0c, 0x08, 0x78, 0x40, 0x01, 0x3b, 0x80, 0x00,
		0x68, 0x4a, 0xff, 0x00, 0x70, 0x50, 0x01, 0x00,
		0x5a, 0xa8, 0x00, 0x00, 0x13, 0x7c, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x50,
		0x00, 0x57, 0x00, 0x47, 0x00, 0x57, 0x00, 0x47,
		0x20, 0x23, 0x00, 0x00, 0x20, 0x20, 0x00, 0xff,
		0x00, 0xbf, 0x00, 0xff, 0x00, 0xbf, 0x49, 0x20,
		0x00, 0x00, 0xc7, 0x00, 0x95, 0x00, 0xc7, 0x00,
		0x95, 0x45, 0x20, 0x00, 0x00, 0x9f, 0x00, 0x77,
		0x00, 0x9f, 0x00, 0x77, 0x43, 0x20, 0x00, 0x00,
		0xaf, 0x00, 0x77, 0x00, 0xaf, 0x00, 0x77, 0x43,
		0x20, 0x40, 0x00, 0x57, 0x00, 0x3b, 0x00, 0x57,
		0x00, 0x3b, 0x40, 0x20, 0x40, 0x70, 0x01, 0x00,
		0x02, 0x10, 0x00, 0x01, 0x60, 0x00, 0x06, 0x00,
		0x08, 0x81, 0x6f, 0x01, 0x02, 0x00, 0x00, 0x01,
		0x00, 0x11, 0x01, 0x80, 0x00, 0x16, 0x0c, 0x00,
		0x08, 0x00, 0xc0, 0xb5, 0x00, 0x00, 0x01, 0x01,
		0x9f, 0x80, 0x00, 0x17, 0x0c, 0x08, 0x39, 0x40,
		0x01, 0x80, 0x25, 0x60, 0x00, 0x07, 0x00, 0x08,
		0x81, 0x71, 0x00, 0x00, 0x01, 0x40, 0x4b, 0x00,
		0x06, 0x02, 0x91, 0x40, 0x02, 0xa2, 0x00, 0x24,
		0x00, 0x32, 0x00, 0x2f, 0x00, 0x62, 0x00, 0x40,
		0x00, 0x42, 0x00, 0x0f, 0x00, 0xa2, 0x00, 0x0d,
		0x01, 0x60, 0x00, 0x06, 0x00, 0x08, 0x81, 0x6f,
		0x01, 0x02, 0x00, 0x00, 0x01, 0x00, 0x11, 0x01,
		0x80, 0x00, 0x18, 0x48, 0xc6, 0x00, 0x32, 0x80,
		0x00, 0x19, 0x4a, 0x0c, 0x09, 0x00, 0x00, 0x06,
		0x00, 0x08, 0x81, 0x60, 0x01, 0x00, 0x00, 0x32,
		0x80, 0x00, 0x1a, 0x04, 0xb5, 0x00, 0x23, 0x31,
		0x02, 0x51, 0x53, 0x00, 0x80, 0x00, 0x05, 0x01,
		0x00, 0x00, 0x00, 0x01, 0x0d, 0x00, 0x02, 0x00,
		0x03, 0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00,
		0x07, 0x00, 0x08, 0x00, 0x09, 0x00, 0x0a, 0x00,
		0x0b, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x0e, 0x00,
		0x0f, 0x04, 0x00, 0x10, 0x00, 0x11, 0x00, 0x12,
		0x00, 0x13, 0x00, 0x14, 0x02, 0x00, 0x15, 0x00,
		0x16, 0x00, 0x17, 0x01, 0x00, 0x18, 0x00, 0x19,
		0x00, 0x00, 0x1a
	};
}


TEST_F(H245MessagDecodeTest, H245TerminalCapabilitySet)
{
	VS_PerBuffer buffer((TERMINAL_CAPABILITY_SET), sizeof(TERMINAL_CAPABILITY_SET) * 8);
	VS_H245MultimediaSystemControlMessage msg;
	ASSERT_TRUE(msg.Decode(buffer));
	ASSERT_EQ(msg.tag, VS_H245MultimediaSystemControlMessage::e_request);
	auto rsp = dynamic_cast<VS_H245RequestMessage*>(msg.choice);
	ASSERT_TRUE(rsp);
	ASSERT_EQ(rsp->tag, VS_H245RequestMessage::e_terminalCapabilitySet);

	auto tcs = static_cast<VS_H245TerminalCapabilitySet *>(rsp->choice);

	ASSERT_EQ(tcs->sequenceNumber.value, 1);

	std::uint32_t product_identifier[VS_AsnObjectId::max_values] = { 0, 0, 8, 245, 0, 13 };
	EXPECT_ARRAY_EQ(const std::uint32_t, tcs->protocolIdentifier.value, product_identifier, VS_AsnObjectId::max_values);

	ASSERT_EQ(tcs->multiplexCapability.tag, VS_H245MultiplexCapability::e_h2250Capability);

	VS_H245H2250Capability *h2250_capability = tcs->multiplexCapability;
	ASSERT_TRUE(h2250_capability);

	EXPECT_EQ(h2250_capability->maximumAudioDelayJitter.value, 60);
	EXPECT_EQ(h2250_capability->receiveMultipointCapability.multicastCapability.value, false);
	EXPECT_EQ(h2250_capability->receiveMultipointCapability.multiUniCastConference.value, false);
	ASSERT_EQ(h2250_capability->receiveMultipointCapability.mediaDistributionCapability.size(), 1);
	EXPECT_EQ(h2250_capability->receiveMultipointCapability.mediaDistributionCapability[0].centralizedControl.value, false);
	EXPECT_EQ(h2250_capability->receiveMultipointCapability.mediaDistributionCapability[0].distributedControl.value, false);
	EXPECT_EQ(h2250_capability->receiveMultipointCapability.mediaDistributionCapability[0].centralizedAudio.value, false);
	EXPECT_EQ(h2250_capability->receiveMultipointCapability.mediaDistributionCapability[0].distributedAudio.value, false);
	EXPECT_EQ(h2250_capability->receiveMultipointCapability.mediaDistributionCapability[0].centralizedVideo.value, false);
	EXPECT_EQ(h2250_capability->receiveMultipointCapability.mediaDistributionCapability[0].distributedVideo.value, false);

	EXPECT_EQ(h2250_capability->transmitMultipointCapability.multicastCapability.value, false);
	EXPECT_EQ(h2250_capability->transmitMultipointCapability.multiUniCastConference.value, false);
	ASSERT_EQ(h2250_capability->transmitMultipointCapability.mediaDistributionCapability.size(), 1);
	EXPECT_EQ(h2250_capability->transmitMultipointCapability.mediaDistributionCapability[0].centralizedControl.value, false);
	EXPECT_EQ(h2250_capability->transmitMultipointCapability.mediaDistributionCapability[0].distributedControl.value, false);
	EXPECT_EQ(h2250_capability->transmitMultipointCapability.mediaDistributionCapability[0].centralizedAudio.value, false);
	EXPECT_EQ(h2250_capability->transmitMultipointCapability.mediaDistributionCapability[0].distributedAudio.value, false);
	EXPECT_EQ(h2250_capability->transmitMultipointCapability.mediaDistributionCapability[0].centralizedVideo.value, false);
	EXPECT_EQ(h2250_capability->transmitMultipointCapability.mediaDistributionCapability[0].distributedVideo.value, false);

	EXPECT_EQ(h2250_capability->receiveAndTransmitMultipointCapability.multicastCapability.value, false);
	EXPECT_EQ(h2250_capability->receiveAndTransmitMultipointCapability.multiUniCastConference.value, false);
	ASSERT_EQ(h2250_capability->receiveAndTransmitMultipointCapability.mediaDistributionCapability.size(), 1);
	EXPECT_EQ(h2250_capability->receiveAndTransmitMultipointCapability.mediaDistributionCapability[0].centralizedControl.value, false);
	EXPECT_EQ(h2250_capability->receiveAndTransmitMultipointCapability.mediaDistributionCapability[0].distributedControl.value, false);
	EXPECT_EQ(h2250_capability->receiveAndTransmitMultipointCapability.mediaDistributionCapability[0].centralizedAudio.value, false);
	EXPECT_EQ(h2250_capability->receiveAndTransmitMultipointCapability.mediaDistributionCapability[0].distributedAudio.value, false);
	EXPECT_EQ(h2250_capability->receiveAndTransmitMultipointCapability.mediaDistributionCapability[0].centralizedVideo.value, false);
	EXPECT_EQ(h2250_capability->receiveAndTransmitMultipointCapability.mediaDistributionCapability[0].distributedVideo.value, false);

	EXPECT_EQ(h2250_capability->mcCapability.centralizedConferenceMC.value, false);
	EXPECT_EQ(h2250_capability->mcCapability.decentralizedConferenceMC.value, false);
	EXPECT_EQ(h2250_capability->rtcpVideoControlCapability.value, false);

	EXPECT_EQ(h2250_capability->rtcpVideoControlCapability.value, false);
	EXPECT_EQ(h2250_capability->mediaPacketizationCapability.h261aVideoPacketization.value, false);

	ASSERT_EQ(h2250_capability->mediaPacketizationCapability.rtpPayloadType.size(), 3);

	const unsigned char rfc_number_data[] = {
		0x09, 0x7c
	}; //2429
	ASSERT_EQ(h2250_capability->mediaPacketizationCapability.rtpPayloadType[0].payloadDescriptor.tag, VS_H245RTPPayloadType_PayloadDescriptor::e_rfc_number);

	auto rfc_number = static_cast<VS_H245NonStandardParameter *>(h2250_capability->mediaPacketizationCapability.rtpPayloadType[0].
		payloadDescriptor.choice);
	//Todo: may be bug
//	ASSERT_TRUE((vs::size(rfc_number_data) == rfc_number->data.value.ByteSize()
//		&& std::equal(std::begin(rfc_number_data), std::end(rfc_number_data), static_cast<unsigned char *>(rfc_number->data.value.GetData()))));


	ASSERT_EQ(h2250_capability->mediaPacketizationCapability.rtpPayloadType[1].payloadDescriptor.tag, VS_H245RTPPayloadType_PayloadDescriptor::e_oid);
	auto oid_1 = static_cast<VS_AsnObjectId *>(h2250_capability->mediaPacketizationCapability.rtpPayloadType[1].
		payloadDescriptor.choice);

	std::uint32_t oid_value_1[VS_AsnObjectId::max_values] = { 0, 0, 8, 241, 0, 0, 0, 0 };
	EXPECT_ARRAY_EQ(const std::uint32_t, oid_1->value, oid_value_1, VS_AsnObjectId::max_values);

	ASSERT_EQ(h2250_capability->mediaPacketizationCapability.rtpPayloadType[2].payloadDescriptor.tag, VS_H245RTPPayloadType_PayloadDescriptor::e_oid);
	auto oid_2 = static_cast<VS_AsnObjectId *>(h2250_capability->mediaPacketizationCapability.rtpPayloadType[2].
		payloadDescriptor.choice);

	std::uint32_t oid_value_2[VS_AsnObjectId::max_values] = { 0, 0, 8, 241, 0, 0, 0, 1 };
	EXPECT_ARRAY_EQ(const std::uint32_t, oid_2->value, oid_value_2, VS_AsnObjectId::max_values);

	ASSERT_EQ(h2250_capability->transportCapability.qOSCapabilities.size(), 1);
	ASSERT_EQ(h2250_capability->transportCapability.qOSCapabilities[0].rsvpParameters.qosMode.tag, VS_H245QOSMode::e_guaranteedQOS);

	const VS_AsnNull null;
	auto &guaranteed_qos = *static_cast<VS_AsnNull*>(h2250_capability->transportCapability.qOSCapabilities[0].rsvpParameters.qosMode.choice);
	EXPECT_TRUE((null == guaranteed_qos));

	ASSERT_EQ(h2250_capability->transportCapability.mediaChannelCapabilities.size(), 1);
	ASSERT_EQ(h2250_capability->transportCapability.mediaChannelCapabilities[0].mediaTransport.tag, VS_H245MediaTransportType::e_ip_UDP);
	auto &ip_udp = *static_cast<VS_AsnNull*>(h2250_capability->transportCapability.mediaChannelCapabilities[0].mediaTransport.choice);
	EXPECT_TRUE((null == ip_udp));

	EXPECT_EQ(h2250_capability->logicalChannelSwitchingCapability.value, false);
	EXPECT_EQ(h2250_capability->t120DynamicPortCapability.value, false);

	ASSERT_EQ(tcs->capabilityTable.size(), 27);

	struct Standart
	{
		std::uint32_t standart[VS_AsnObjectId::max_values];
	};

	std::size_t intex_start = 0;

	const Standart capabilitys_0[] = { { { 0, 0, 8, 239, 1, 1 } } };

	const auto exec_0 = [&](const Standart capabilitys[], const std::size_t sizeCapability,
		const std::size_t start)
	{
		ASSERT_LE(sizeCapability, tcs->capabilityTable.size() - start);
		for (std::size_t i = start, j = 0; i < tcs->capabilityTable.size() && j < sizeCapability; ++i, ++j)
		{
			ASSERT_EQ(tcs->capabilityTable[i].capabilityTableEntryNumber.value, i + 1);
			ASSERT_EQ(tcs->capabilityTable[i].capability.tag, VS_H245Capability::e_genericControlCapability);
			VS_H245GenericCapability *generic_control_capability = tcs->capabilityTable[i].capability;
			ASSERT_TRUE(generic_control_capability);
			ASSERT_EQ(generic_control_capability->capabilityIdentifier.tag, VS_H245CapabilityIdentifier::e_standard);
			auto capability_identifier_standart = static_cast<VS_AsnObjectId *>(generic_control_capability->capabilityIdentifier.choice);
			EXPECT_ARRAY_EQ(const std::uint32_t, capability_identifier_standart->value, capabilitys[i].standart, VS_AsnObjectId::max_values);
		}
	};

	auto size = vs::size(capabilitys_0);
	exec_0(capabilitys_0, size, intex_start);
	intex_start += size;

	struct H221NonStandart
	{
		H221NonStandart(unsigned t32CountryCode, unsigned tc32Extension, unsigned manufacturerCode, unsigned char data)
			: t32CountryCode(t32CountryCode),
			tc32Extension(tc32Extension),
			manufacturerCode(manufacturerCode),
			data(data)
		{
		}

		unsigned t32CountryCode;
		unsigned tc32Extension;
		unsigned manufacturerCode;
		unsigned char data;
	};

	struct NonCollapsing
	{
		unsigned paramIdent;
		std::pair<unsigned, unsigned> paramValue;
	};

	struct H221NonStandartNonCollapsing
	{
		H221NonStandart h221NonStandart;
		std::vector<NonCollapsing> nonCollapsing;
	};


	const H221NonStandartNonCollapsing capabilitys_1[] =
	{
		{ { 181, 0, 1, 0xac }, { { 1, { 1, VS_H245ParameterValue::e_unsignedMin} }, {2,  { 150, VS_H245ParameterValue::e_unsignedMin }}, {3,  { 150, VS_H245ParameterValue::e_unsignedMin } },
			{4, { 52, VS_H245ParameterValue::e_unsignedMin } }, {5, { 10, VS_H245ParameterValue::e_unsignedMin } }, {6, { 1400, VS_H245ParameterValue::e_unsignedMin } } } }
	};

	const auto exec_1 = [&](const H221NonStandartNonCollapsing capabilitys[], const std::size_t sizeCapability,
		const std::size_t start)
	{
		ASSERT_LE(sizeCapability, tcs->capabilityTable.size() - start);
		for (std::size_t i = start, j = 0; i < tcs->capabilityTable.size() && j < sizeCapability; ++i, ++j)
		{
			ASSERT_EQ(tcs->capabilityTable[i].capabilityTableEntryNumber.value, i + 1);
			ASSERT_EQ(tcs->capabilityTable[i].capability.tag, VS_H245Capability::e_genericControlCapability);
			VS_H245GenericCapability *generic_control_capability = tcs->capabilityTable[i].capability;
			ASSERT_TRUE(generic_control_capability);

			ASSERT_EQ(generic_control_capability->capabilityIdentifier.tag, VS_H245CapabilityIdentifier::e_h221NonStandard);
			auto h221_non_standart = static_cast<VS_H245NonStandardParameter *>(generic_control_capability->capabilityIdentifier.choice);
			ASSERT_EQ(h221_non_standart->nonStandardIdentifier.tag, VS_H245NonStandardIdentifier::e_h221NonStandard);
			auto h221_non_standart_item = static_cast<VS_H245NonStandardIdentifier_H221NonStandard*>(h221_non_standart->nonStandardIdentifier.choice);
			ASSERT_EQ(h221_non_standart_item->t35CountryCode.value, capabilitys[j].h221NonStandart.t32CountryCode);
			ASSERT_EQ(h221_non_standart_item->t35Extension.value, capabilitys[j].h221NonStandart.tc32Extension);
			ASSERT_EQ(h221_non_standart_item->manufacturerCode.value, capabilitys[j].h221NonStandart.manufacturerCode);
			//TODO:h.245 manufacturer
			ASSERT_EQ(h221_non_standart->data.value.ByteSize(), 1);
			ASSERT_TRUE((*(static_cast<unsigned char *>(h221_non_standart->data.value.GetData())) == capabilitys[j].h221NonStandart.data));

			ASSERT_EQ(generic_control_capability->nonCollapsing.size(), capabilitys[j].nonCollapsing.size());

			for (std::size_t k = 0; i < generic_control_capability->nonCollapsing.size(); i++)
			{
				ASSERT_EQ(generic_control_capability->nonCollapsing[k].parameterIdentifier.tag, VS_H245ParameterIdentifier::e_standard);
				TemplInteger<0, 127, VS_Asn::FixedConstraint, false> * param_ident =
					static_cast<TemplInteger<0, 127, VS_Asn::FixedConstraint, false> *>(generic_control_capability->nonCollapsing[k].
						parameterIdentifier.choice);
				ASSERT_EQ(param_ident->value, capabilitys[j].nonCollapsing[k].paramIdent);
				if (capabilitys[j].nonCollapsing[k].paramValue.second == VS_H245ParameterValue::e_unsignedMin)
				{
					ASSERT_EQ(generic_control_capability->nonCollapsing[k].parameterValue.tag, VS_H245ParameterValue::e_unsignedMin);
					auto unsigned_min_val =
						static_cast<TemplInteger<0, 65535, VS_Asn::FixedConstraint, false> *>(generic_control_capability->nonCollapsing[k].parameterValue.choice);
					ASSERT_EQ(capabilitys[j].nonCollapsing[k].paramValue.first, unsigned_min_val->value);
				}
				else if(capabilitys[j].nonCollapsing[k].paramValue.second == VS_H245ParameterValue::e_booleanArray)
				{
					ASSERT_EQ(generic_control_capability->nonCollapsing[k].parameterValue.tag, VS_H245ParameterValue::e_booleanArray);
					TemplInteger<0, 255, VS_Asn::FixedConstraint, false> *boolean_array =
						static_cast<TemplInteger<0, 255, VS_Asn::FixedConstraint, false> *>(generic_control_capability->nonCollapsing[k].parameterValue.choice);
					ASSERT_EQ(capabilitys[j].nonCollapsing[k].paramValue.first, boolean_array->value);
				}
				else
				{
					FAIL();
				}
			}
		}
	};

	size = vs::size(capabilitys_1);
	exec_1(capabilitys_1, size, intex_start);
	intex_start += size;

	struct Collapsing : public H221NonStandart
	{
		Collapsing(unsigned t32CountryCode, unsigned tc32Extension, unsigned manufacturerCode, unsigned char data,
			unsigned unsignedMin)
			: H221NonStandart(t32CountryCode, tc32Extension, manufacturerCode, data),
			unsignedMin(unsignedMin)
		{
		}
		unsigned unsignedMin;
	};

	struct H221NonStandartBitRateCollapsings
	{
		H221NonStandart h221NonStandart;
		unsigned maxBitRate;
		std::vector<Collapsing> collapsings;
	};


	const H221NonStandartBitRateCollapsings capabilitys_2[] =
	{
		{ { 181, 0, 1, 0xad }, 64000,{ { 181, 0, 1, 0x01, 1 },{ 181, 0, 1, 0x01, 1 } } },
		{ { 181, 0, 1, 0xa7 }, 64000,{ { 181, 0, 1, 0x01, 1 } } },
		{ { 181, 0, 1, 0xa6 }, 48000,{ { 181, 0, 1, 0x01, 1 } } },
		{ { 181, 0, 1, 0xa5 }, 32000,{ { 181, 0, 1, 0x01, 1 } } }
	};

	const auto exec_2 = [&](const H221NonStandartBitRateCollapsings capabilitys[], const std::size_t sizeCapability,
		const std::size_t start)
	{
		ASSERT_LE(sizeCapability, tcs->capabilityTable.size() - start);
		for (std::size_t i = start, j = 0; i < tcs->capabilityTable.size() && j < sizeCapability; ++i, ++j)
		{
			ASSERT_EQ(tcs->capabilityTable[i].capabilityTableEntryNumber.value, i + 1);
			ASSERT_EQ(tcs->capabilityTable[i].capability.tag, VS_H245Capability::e_receiveAudioCapability);
			VS_H245AudioCapability *receive_capability = tcs->capabilityTable[i].capability;
			ASSERT_TRUE(receive_capability);
			ASSERT_EQ(receive_capability->tag, VS_H245AudioCapability::e_genericAudioCapability);
			VS_H245GenericCapability *generic_audio_capability = (*receive_capability);
			ASSERT_TRUE(generic_audio_capability);
			ASSERT_EQ(generic_audio_capability->capabilityIdentifier.tag, VS_H245CapabilityIdentifier::e_h221NonStandard);
			VS_H245NonStandardParameter *generic_audio_h221_non_standart = generic_audio_capability->capabilityIdentifier;
			ASSERT_TRUE(generic_audio_h221_non_standart);
			ASSERT_EQ(generic_audio_h221_non_standart->nonStandardIdentifier.tag, VS_H245NonStandardIdentifier::e_h221NonStandard);
			VS_H245NonStandardIdentifier_H221NonStandard *generic_audio_h221_non_standart_item =
				static_cast<VS_H245NonStandardIdentifier_H221NonStandard *>(generic_audio_h221_non_standart->nonStandardIdentifier.
					choice);
			ASSERT_EQ(generic_audio_h221_non_standart_item->t35CountryCode.value, capabilitys[j].h221NonStandart.t32CountryCode);
			ASSERT_EQ(generic_audio_h221_non_standart_item->t35Extension.value, capabilitys[j].h221NonStandart.tc32Extension);
			ASSERT_EQ(generic_audio_h221_non_standart_item->manufacturerCode.value, capabilitys[j].h221NonStandart.manufacturerCode);
			//TODO: h.245 manufacturer

			ASSERT_EQ(generic_audio_h221_non_standart->data.value.ByteSize(), 1);
			ASSERT_TRUE((*static_cast<unsigned char *>(generic_audio_h221_non_standart->data.value.GetData()) == capabilitys[j].h221NonStandart.data));
			ASSERT_EQ(generic_audio_capability->maxBitRate.value, capabilitys[j].maxBitRate);

			ASSERT_EQ(capabilitys[j].collapsings.size(), generic_audio_capability->collapsing.size());
			for (std::size_t k = 0; k < generic_audio_capability->collapsing.size(); ++k)
			{
				ASSERT_EQ(generic_audio_capability->collapsing[k].parameterIdentifier.tag, VS_H245ParameterIdentifier::e_h221NonStandard);
				VS_H245NonStandardParameter *collapsing_h221_not_standaet = generic_audio_capability->collapsing[k].parameterIdentifier;
				ASSERT_TRUE(collapsing_h221_not_standaet);
				ASSERT_EQ(collapsing_h221_not_standaet->nonStandardIdentifier.tag, VS_H245NonStandardIdentifier::e_h221NonStandard);
				VS_H245NonStandardIdentifier_H221NonStandard *collapsing_h221_not_standaet_item =
					static_cast<VS_H245NonStandardIdentifier_H221NonStandard *>(collapsing_h221_not_standaet->nonStandardIdentifier.choice);

				ASSERT_EQ(collapsing_h221_not_standaet_item->t35CountryCode.value, capabilitys[j].collapsings[k].t32CountryCode);
				ASSERT_EQ(collapsing_h221_not_standaet_item->t35Extension.value, capabilitys[j].collapsings[k].tc32Extension);
				ASSERT_EQ(collapsing_h221_not_standaet_item->manufacturerCode.value, capabilitys[j].collapsings[k].manufacturerCode);

				//TODO: h245 manufacturer

				ASSERT_EQ(collapsing_h221_not_standaet->data.value.ByteSize(), 1);
				ASSERT_TRUE((*static_cast<unsigned char *>(collapsing_h221_not_standaet->data.value.GetData()) == capabilitys[j].collapsings[k].data));

				ASSERT_EQ(generic_audio_capability->collapsing[k].parameterValue.tag, VS_H245ParameterValue::e_unsignedMin);
				TemplInteger<0, 65535, VS_Asn::FixedConstraint, false> *unsigned_min_collapsing =
					static_cast<TemplInteger<0, 65535, VS_Asn::FixedConstraint, false> *>(generic_audio_capability->collapsing[k].
						parameterValue.choice);
				ASSERT_EQ(unsigned_min_collapsing->value, capabilitys[j].collapsings[k].unsignedMin);
			}
		}
	};

	size = vs::size(capabilitys_2);
	exec_2(capabilitys_2, size, intex_start);
	intex_start += size;

	struct StandartMaxBitRateCollapsing
	{
		Standart standart;
		unsigned maxBitRate;
		std::vector<NonCollapsing> collapsing;
	};

	StandartMaxBitRateCollapsing capabilitys_3[] =
	{
		{ { { 0, 0, 7, 7221, 1, 1, 0 } }, 480, { { 1, { 1, VS_H245ParameterValue::e_unsignedMin } }, { 2,  { 112, VS_H245ParameterValue::e_booleanArray } } } }
	};

	const auto exec_3 = [&](const StandartMaxBitRateCollapsing capabilitys[], const std::size_t sizeCapability,
		const std::size_t start)
	{
		ASSERT_LE(sizeCapability, tcs->capabilityTable.size() - start);
		for (std::size_t i = start, j = 0; i < tcs->capabilityTable.size() && j < sizeCapability; ++i, ++j)
		{
			ASSERT_EQ(tcs->capabilityTable[i].capabilityTableEntryNumber.value, i + 1);
			ASSERT_EQ(tcs->capabilityTable[i].capability.tag, VS_H245Capability::e_receiveAudioCapability);
			VS_H245AudioCapability *audio_capability = tcs->capabilityTable[i].capability;
			ASSERT_TRUE(audio_capability);
			ASSERT_EQ(audio_capability->tag, VS_H245AudioCapability::e_genericAudioCapability);
			VS_H245GenericCapability *receive_capability = static_cast<VS_H245GenericCapability *>(audio_capability->choice);
			ASSERT_EQ(receive_capability->capabilityIdentifier.tag, VS_H245CapabilityIdentifier::e_standard);

			auto capability_identifier_standart = static_cast<VS_AsnObjectId *>(receive_capability->capabilityIdentifier.choice);
			EXPECT_ARRAY_EQ(const std::uint32_t, capability_identifier_standart->value, capabilitys[j].standart.standart, VS_AsnObjectId::max_values);

			ASSERT_EQ(receive_capability->maxBitRate.value, capabilitys[j].maxBitRate);

			for(std::size_t k = 0; k < capabilitys[j].collapsing.size(); k++)
			{
				ASSERT_EQ(receive_capability->collapsing[k].parameterIdentifier.tag, VS_H245ParameterIdentifier::e_standard);
				TemplInteger<0, 127, VS_Asn::FixedConstraint, false> *p =
					static_cast<TemplInteger<0, 127, VS_Asn::FixedConstraint, false> *>(receive_capability->collapsing[k].parameterIdentifier.choice);
				ASSERT_EQ(p->value, capabilitys[j].collapsing[k].paramIdent);

				if (capabilitys[j].collapsing[k].paramValue.second == VS_H245ParameterValue::e_unsignedMin)
				{
					ASSERT_EQ(receive_capability->collapsing[k].parameterValue.tag, VS_H245ParameterValue::e_unsignedMin);
					TemplInteger<0, 65535, VS_Asn::FixedConstraint, false> *p1 =
						static_cast<TemplInteger<0, 65535, VS_Asn::FixedConstraint, false> *>(receive_capability->collapsing[k].parameterValue.choice);
					ASSERT_EQ(p1->value, capabilitys[j].collapsing[k].paramValue.first);
				}
				else if (capabilitys[j].collapsing[k].paramValue.second == VS_H245ParameterValue::e_booleanArray)
				{
					ASSERT_EQ(receive_capability->collapsing[k].parameterValue.tag, VS_H245ParameterValue::e_booleanArray);
					TemplInteger<0, 255, VS_Asn::FixedConstraint, false> *boolean_array =
						static_cast<TemplInteger<0, 255, VS_Asn::FixedConstraint, false> *>(receive_capability->collapsing[k].parameterValue.choice);
					ASSERT_EQ(capabilitys[j].collapsing[k].paramValue.first, boolean_array->value);
				}
				else
				{
					FAIL();
				}
			}
		}
	};

	size = vs::size(capabilitys_3);
	exec_3(capabilitys_3, size, intex_start);
	intex_start += size;

	const H221NonStandartBitRateCollapsings capabilitys_4[] =
	{
		{ { 181, 0, 1, 0x93 }, 48000,{ { 181, 0, 1, 0x01, 1 } } }
	};

	size = vs::size(capabilitys_4);
	exec_2(capabilitys_4, size, intex_start);
	intex_start += size;


	StandartMaxBitRateCollapsing capabilitys_5[] =
	{
		{ { { 0, 0, 7, 7221, 1, 0 } }, 32000 ,{ { 1, { 1, VS_H245ParameterValue::e_unsignedMin } } } },
		{ { { 0, 0, 7, 7221, 1, 0 } }, 24000 ,{ { 1, { 1, VS_H245ParameterValue::e_unsignedMin } } } }
	}; //8 - 9

	size = vs::size(capabilitys_5);
	exec_3(capabilitys_5, size, intex_start);
	intex_start += size;

	const H221NonStandartBitRateCollapsings capabilitys_6[] =
	{
		{ { 181, 0, 1, 0x96 }, 16000, { { 181, 0, 1, 0x01, 1 } } },
	}; //10

	size = vs::size(capabilitys_6);
	exec_2(capabilitys_6, size, intex_start);
	intex_start += size;


	const auto exec_4 = [&](const unsigned tag, std::uint32_t value)
	{
		ASSERT_EQ(tcs->capabilityTable[intex_start].capabilityTableEntryNumber.value, intex_start + 1);
		ASSERT_EQ(tcs->capabilityTable[intex_start].capability.tag, VS_H245Capability::e_receiveAudioCapability);
		VS_H245AudioCapability *audio_capability = tcs->capabilityTable[intex_start].capability;
		ASSERT_TRUE(audio_capability);
		ASSERT_EQ(audio_capability->tag, tag);
		TemplInteger<1, 256, VS_Asn::FixedConstraint, false> *capability = static_cast<TemplInteger<1, 256, VS_Asn::FixedConstraint, false> *>(
			audio_capability->choice);
		ASSERT_EQ(capability->value, value);
		intex_start++;
	};

	exec_4(VS_H245AudioCapability::e_g722_64k, 20);
	exec_4(VS_H245AudioCapability::e_g728, 8);
	exec_4(VS_H245AudioCapability::e_g711Ulaw64k, 20);
	exec_4(VS_H245AudioCapability::e_g711Alaw64k, 20);
	exec_4(VS_H245AudioCapability::e_g729AnnexA, 2);

	const auto exec_5 = [&](const StandartMaxBitRateCollapsing capabilitys[], const std::size_t sizeCapability,
		const std::size_t start)
	{
		ASSERT_LE(sizeCapability, tcs->capabilityTable.size() - start);
		for (std::size_t i = start, j = 0; i < tcs->capabilityTable.size() && j < sizeCapability; ++i, ++j)
		{
			ASSERT_EQ(tcs->capabilityTable[i].capabilityTableEntryNumber.value, i + 1);
			ASSERT_EQ(tcs->capabilityTable[i].capability.tag, VS_H245Capability::e_receiveVideoCapability);
			VS_H245VideoCapability *video_capability = tcs->capabilityTable[i].capability;
			ASSERT_TRUE(video_capability);
			ASSERT_EQ(video_capability->tag, VS_H245VideoCapability::e_genericVideoCapability);
			VS_H245GenericCapability *receive_capability = static_cast<VS_H245GenericCapability *>(video_capability->choice);
			ASSERT_EQ(receive_capability->capabilityIdentifier.tag, VS_H245CapabilityIdentifier::e_standard);

			auto capability_identifier_standart = static_cast<VS_AsnObjectId *>(receive_capability->capabilityIdentifier.choice);
			EXPECT_ARRAY_EQ(const std::uint32_t, capability_identifier_standart->value, capabilitys[j].standart.standart, VS_AsnObjectId::max_values);

			ASSERT_EQ(receive_capability->maxBitRate.value, capabilitys[j].maxBitRate);

			for (std::size_t k = 0; k < capabilitys[j].collapsing.size(); k++)
			{
				ASSERT_EQ(receive_capability->collapsing[k].parameterIdentifier.tag, VS_H245ParameterIdentifier::e_standard);
				TemplInteger<0, 127, VS_Asn::FixedConstraint, false> *p =
					static_cast<TemplInteger<0, 127, VS_Asn::FixedConstraint, false> *>(receive_capability->collapsing[k].parameterIdentifier.choice);
				ASSERT_EQ(p->value, capabilitys[j].collapsing[k].paramIdent);

				if (capabilitys[j].collapsing[k].paramValue.second == VS_H245ParameterValue::e_unsignedMin)
				{
					ASSERT_EQ(receive_capability->collapsing[k].parameterValue.tag, VS_H245ParameterValue::e_unsignedMin);
					TemplInteger<0, 65535, VS_Asn::FixedConstraint, false> *p1 =
						static_cast<TemplInteger<0, 65535, VS_Asn::FixedConstraint, false> *>(receive_capability->collapsing[k].parameterValue.choice);
					ASSERT_EQ(p1->value, capabilitys[j].collapsing[k].paramValue.first);
				}
				else if (capabilitys[j].collapsing[k].paramValue.second == VS_H245ParameterValue::e_booleanArray)
				{
					ASSERT_EQ(receive_capability->collapsing[k].parameterValue.tag, VS_H245ParameterValue::e_booleanArray);
					TemplInteger<0, 255, VS_Asn::FixedConstraint, false> *boolean_array =
						static_cast<TemplInteger<0, 255, VS_Asn::FixedConstraint, false> *>(receive_capability->collapsing[k].parameterValue.choice);
					ASSERT_EQ(capabilitys[j].collapsing[k].paramValue.first, boolean_array->value);
				}
				else
				{
					FAIL();
				}
			}
		}
	};

	const StandartMaxBitRateCollapsing capabilitys_7[] =
	{
		{ { { 0, 0, 8, 241, 0, 0, 1 } }, 19200 , { { 41, { 64, VS_H245ParameterValue::e_booleanArray } },
			{ 42, { 36, VS_H245ParameterValue::e_unsignedMin } }, { 3, { 216, VS_H245ParameterValue::e_unsignedMin } },
			{ 6,  { 64, VS_H245ParameterValue::e_unsignedMin } }, { 4, { 15, VS_H245ParameterValue::e_unsignedMin  } },
			{ 10, { 13, VS_H245ParameterValue::e_unsignedMin } } }
		},
		{
		  { { 0, 0, 8, 241, 0, 0, 1 } }, 19200 ,{ { 41, { 8, VS_H245ParameterValue::e_booleanArray } },
			{ 42, { 36, VS_H245ParameterValue::e_unsignedMin } },{ 3,{ 216, VS_H245ParameterValue::e_unsignedMin } },
			{ 6,{ 64, VS_H245ParameterValue::e_unsignedMin } },{ 4,{ 15, VS_H245ParameterValue::e_unsignedMin } },
			{ 10,{ 13, VS_H245ParameterValue::e_unsignedMin } } }
		}
	};

	size = vs::size(capabilitys_7);
	exec_5(capabilitys_7, size, intex_start);
	intex_start += size;

	///-----/// equivalent capabilities ///-----///

	struct AlternativeCapabilitySet final
	{
		std::vector<std::uint32_t> items;
	};

	ASSERT_EQ(tcs->capabilityDescriptors.size(), 1);
	EXPECT_EQ(tcs->capabilityDescriptors[0].capabilityDescriptorNumber.value, 0);

	const AlternativeCapabilitySet capability_set[] =
	{
		{ { 1, 2 } },
		{ { 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 } },
		{ { 17, 18, 19, 20, 21 } },
		{ { 22, 23, 24 } },
		{ { 25, 26 } },
		{ { 27 } }
	};

	ASSERT_EQ(tcs->capabilityDescriptors.size(), 1);
	ASSERT_EQ(vs::size(capability_set), tcs->capabilityDescriptors[0].simultaneousCapabilities.size());

	std::size_t j = 0;
	for (auto it_capab = tcs->capabilityDescriptors[0].simultaneousCapabilities.begin(); it_capab !=
		tcs->capabilityDescriptors[0].simultaneousCapabilities.end(); ++j, ++it_capab)
	{
		for (size_t i = 0; i < capability_set[j].items.size(); i++)
		{
			EXPECT_EQ((it_capab->begin() + i)->value, capability_set[j].items[i]);
		}
	}
}

/*
 * !!! Проблема:
 * в отличии 2-x байт;
 * в размере сообщения, которое на выходе Encode;

 Expected: reference_[cmp_i]
	  Which is: '\0'
To be equal to: actual_[cmp_i]
	  Which is: 'P' (80, 0x50)
x and y differ at index 102
----------------------------------------------
Expected: reference_[cmp_i]
	  Which is: '\n' (10, 0xA)
To be equal to: actual_[cmp_i]
      Which is: '\xE' (14)
With diff:
	  @@ -1,2 +1,1 @@
	-'
	-' (10, 0xA)
	+'\xE' (14)

x and y differ at index 103
----------------------------------------------

* !!! Мои предположения/поиски "приводили" меня к VS_AsnSequence::extensionMap !!!
 */
TEST_F(H225RasMessageEncodeTest, DISABLED_H225RasRegistrationRequest)
{
	const std::size_t size_buff = vs::size(REGISTRATION_REQUEST);
	VS_PerBuffer buff{ REGISTRATION_REQUEST, size_buff * 8, true };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(mess.Encode(theBuffer));
	ASSERT_EQ(size_buff, theBuffer.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), REGISTRATION_REQUEST, size_buff);
}


TEST_F(H225RasMessageEncodeTest, H225RasRegistrationReject)
{
	const std::size_t size_buff = vs::size(REGISTRATION_REJECT);
	VS_PerBuffer buff{ REGISTRATION_REJECT, size_buff * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(mess.Encode(theBuffer));
	ASSERT_EQ(size_buff, theBuffer.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), REGISTRATION_REJECT, size_buff);
}

TEST_F(H225RasMessageEncodeTest, H225RasRegistrationConfirm)
{
	const std::size_t size_buff = vs::size(REGISTRATION_CONFIRM);
	VS_PerBuffer buff{ REGISTRATION_CONFIRM, size_buff * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(mess.Encode(theBuffer));
	ASSERT_EQ(size_buff, theBuffer.ByteSize());

	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), REGISTRATION_CONFIRM, size_buff);
}

/*
* !!! Предположительно, что проблема с VS_AsnSequence::extensionMap !!!
*
Failure
Expected: reference_[cmp_i]
	Which is: '\x14' (20)
To be equal to: actual_[cmp_i]
	Which is: '\x12' (18)
x and y differ at index 77
*/
TEST_F(H225RasMessageEncodeTest, DISABLED_H224RasUnregistrationRequest)
{
	const std::size_t size_buff = vs::size(UNREGISTRATION_REQUEST);
	VS_PerBuffer buff{ UNREGISTRATION_REQUEST, size_buff * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(mess.Encode(theBuffer));
	ASSERT_EQ(size_buff, theBuffer.ByteSize());

	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), UNREGISTRATION_REQUEST, size_buff);
}

TEST_F(H225RasMessageEncodeTest, H224RasUnregistrationConfirm)
{
	const std::size_t size_buff = vs::size(UNREGISTRATION_CONFIRM);
	VS_PerBuffer buff{ UNREGISTRATION_CONFIRM, size_buff * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(mess.Encode(theBuffer));
	ASSERT_EQ(size_buff, theBuffer.ByteSize());

	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), UNREGISTRATION_CONFIRM, size_buff);
}


TEST_F(H225RasMessageEncodeTest, H224RasUnregistrationReject)
{
	const std::size_t size_buff = vs::size(UNREGISTRATION_REJECT);
	VS_PerBuffer buff{ UNREGISTRATION_REJECT, size_buff * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(mess.Encode(theBuffer));
	ASSERT_EQ(size_buff, theBuffer.ByteSize());

	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), UNREGISTRATION_REJECT, size_buff);
}

TEST_F(H245MessagEncodeTest, H245TerminalCapabilitySet)
{
	const std::size_t size_buff = vs::size(TERMINAL_CAPABILITY_SET);
	VS_PerBuffer buffer((TERMINAL_CAPABILITY_SET), size_buff * 8);
	VS_H245MultimediaSystemControlMessage msg;
	ASSERT_TRUE(msg.Decode(buffer));

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(msg.Encode(theBuffer));
	ASSERT_EQ(size_buff, theBuffer.ByteSize());

	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), TERMINAL_CAPABILITY_SET, size_buff);
}

/*
 * !!! На выходе Encode получаем различный размер сообщений.
 */
TEST_F(H225MessageEncodeTest, DISABLED_H225Setup)
{
	std::size_t size_buff = vs::size(SETUP);
	VS_PerBuffer buff{ SETUP, size_buff * 8 };

	VS_Q931 q931{};
	ASSERT_TRUE(q931.DecodeMHeader(static_cast<VS_BitBuffer&>(buff)));
	ASSERT_EQ(q931.protocolDiscriminator, VS_Q931::e_causeIE);
	ASSERT_EQ(q931.lengthCallReference, 2);
	ASSERT_EQ(q931.messageType, VS_Q931::e_setupMsg);
	//need test call reference value and e.t.

	std::uint8_t dn[SIZE_DISPLAY_NAME] = { 0 };
	std::uint8_t e164[SIZE_E163_NAME] = { 0 };
	ASSERT_TRUE(q931.GetUserUserIE(buff, dn, e164));

	const char display_name[] = "test1@vydr111.trueconf.name";
	EXPECT_EQ((string_view{ display_name, sizeof(display_name) - 1 }), (reinterpret_cast<char *>(dn)));
	EXPECT_TRUE((string_view{ reinterpret_cast<char*>(e164) }.empty()));

	VS_CsH323UserInformation ui;
	ASSERT_TRUE(ui.Decode(buff));

	auto size_decode = buff.ByteSize();

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(ui.Encode(theBuffer));
	ASSERT_EQ(size_buff, theBuffer.ByteSize());

	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), SETUP, size_buff);
}


TEST_F(H225MessageEncodeTest, H225ReleaseComplete)
{
	std::size_t size_buff = vs::size(RELEASE_COMPLETE);
	VS_PerBuffer buff{ RELEASE_COMPLETE, size_buff * 8 };

	VS_CsH323UserInformation ui;
	ASSERT_TRUE(ui.Decode(buff));

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(ui.Encode(theBuffer));

	ASSERT_EQ(size_buff, theBuffer.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), RELEASE_COMPLETE, size_buff);
}


TEST_F(H225RasMessageEncodeTest, H225RasDisengageRequest)
{
	std::size_t size_buff = vs::size(DISENGAGE_REQUEST);
	VS_PerBuffer buff{ DISENGAGE_REQUEST, size_buff * 8 };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(mess.Encode(theBuffer));

	ASSERT_EQ(size_buff, theBuffer.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), DISENGAGE_REQUEST, size_buff);
}

/*
*************************************************************************************************************
 Expected: size_buff
	  Which is: 86
To be equal to: theBuffer.ByteSize()
	  Which is: 90
 *************************************************************************************************************
 * E:\tc3\tc3\common\tests\UnitTestH323Lib\UnitTestH323Lib.cpp(1660): error:       Expected: reference_[cmp_i]
	  Which is: '\xD8' (216)
To be equal to: actual_[cmp_i]
	  Which is: '\xDC' (220)
x and y differ at index 54
E:\tc3\tc3\common\tests\UnitTestH323Lib\UnitTestH323Lib.cpp(1660): error:       Expected: reference_[cmp_i]
	  Which is: '\x10' (16)
To be equal to: actual_[cmp_i]
	  Which is: '\x1' (1)
x and y differ at index 82
E:\tc3\tc3\common\tests\UnitTestH323Lib\UnitTestH323Lib.cpp(1660): error:       Expected: reference_[cmp_i]
	  Which is: '\x80' (128)
To be equal to: actual_[cmp_i]
	  Which is: '\0'
x and y differ at index 83
E:\tc3\tc3\common\tests\UnitTestH323Lib\UnitTestH323Lib.cpp(1660): error:       Expected: reference_[cmp_i]
	  Which is: '\x1' (1)
To be equal to: actual_[cmp_i]
	  Which is: '\x10' (16)
x and y differ at index 84
E:\tc3\tc3\common\tests\UnitTestH323Lib\UnitTestH323Lib.cpp(1660): error:       Expected: reference_[cmp_i]
	  Which is: '\0'
To be equal to: actual_[cmp_i]
	  Which is: '\x84' (132)
x and y differ at index 85
 */
TEST_F(H225MessageEncodeTest, DISABLED_H225Alerting)
{
	std::size_t size_buff = vs::size(ALERTING);
	VS_PerBuffer buff{ ALERTING, size_buff * 8 };

	VS_CsH323UserInformation ui;
	ASSERT_TRUE(ui.Decode(buff));

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(ui.Encode(theBuffer));

	ASSERT_EQ(size_buff, theBuffer.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), ALERTING, size_buff);
}


TEST_F(H225MessageEncodeTest, DISABLED_H225Connect)
{
	std::size_t size_buff = vs::size(CONNECT);
	VS_PerBuffer buff{ CONNECT, size_buff * 8 };

	VS_Q931 q931;
	ASSERT_TRUE(q931.DecodeMHeader(static_cast<VS_BitBuffer&>(buff)));
	ASSERT_EQ(q931.protocolDiscriminator, VS_Q931::e_causeIE);
	ASSERT_EQ(q931.lengthCallReference, 2);
	ASSERT_EQ(q931.messageType, VS_Q931::e_connectMsg);
	//need test call reference value and e.tc

	std::uint8_t dn[SIZE_DISPLAY_NAME] = { 0 };
	std::uint8_t e164[SIZE_E163_NAME] = { 0 };
	ASSERT_TRUE(q931.GetUserUserIE(buff, dn, e164));

	const char display_name[] = "hdx6000";
	EXPECT_EQ((string_view{ display_name, sizeof(display_name) - 1 }), reinterpret_cast<char *>(dn));
	EXPECT_TRUE((string_view{ reinterpret_cast<char*>(e164) }.empty()));

	VS_CsH323UserInformation ui;
	ASSERT_TRUE(ui.Decode(buff));

	VS_PerBuffer theBuffer{};
	ASSERT_TRUE(ui.Encode(theBuffer));

	ASSERT_EQ(size_buff, theBuffer.ByteSize());
	EXPECT_ARRAY_EQ(const unsigned char, theBuffer.GetData(), CONNECT, size_buff);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////Regression/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
0000   0e 80 12 6d 06 00 08 91 4a 00 06 00 01 00 0a 6e   ...m....J......n
0010   01 ce 06 b8 01 00 0a 6e 01 ce 06 b7 02 00 01 40   .......n.......@
0020   03 00 74 00 65 00 73 00 74 60 26 00 00 3d 30 56   ..t.e.s.t`&..=0V
0030   61 6c 75 65 48 44 20 43 6f 72 70 6f 72 61 74 69   alueHD Corporati
0040   6f 6e 20 56 69 64 65 6f 20 43 6f 6e 66 65 72 65   on Video Confere
0050   6e 63 69 6e 67 20 45 6e 64 70 6f 69 6e 74 00 00   ncing Endpoint..
0060   13 56 31 2e 31 2e 33 2d 31 37 30 34 32 38 2e 31   .V1.1.3-170428.1
0070   38 34 33 00 00 26 eb 19 20 02 00 09 31 01 4d 00   843..&.. ...1.M.
0080   0a 2a 86 48 86 f7 0c 0a 01 02 01 c0 5d 28 51 8a   .*.H........](Q.
0090   10 87 a0 17 bd 32 8a cc 33 ce 88 a6 a4 7a d9 35   .....2..3....z.5
00a0   fa 01 19 08 00 74 00 65 00 73 00 74 00 00 2e 01   .....t.e.s.t....
00b0   04 04 00 74 00 65 00 73 00 74 00 00 c0 5d 28 51   ...t.e.s.t...](Q
00c0   8a 08 2a 86 48 86 f7 0d 02 05 00 80 80 a8 d2 de   ..*.H...........
00d0   71 91 34 be b5 a9 d8 5f bf 07 22 00 88 01 00 01   q.4...._..".....
00e0   80 01 00 01 00 02 70 00 01 30 05 10 01 00 00 12   ......p..0......
 */

namespace
{
	const unsigned char REGISTRATION_REQUEST_53855[] =
	{
	0x0e, 0x80, 0x12, 0x6d, 0x06, 0x00, 0x08, 0x91,
	0x4a, 0x00, 0x06, 0x00, 0x01, 0x00, 0x0a, 0x6e,
	0x01, 0xce, 0x06, 0xb8, 0x01, 0x00, 0x0a, 0x6e,
	0x01, 0xce, 0x06, 0xb7, 0x02, 0x00, 0x01, 0x40,
	0x03, 0x00, 0x74, 0x00, 0x65, 0x00, 0x73, 0x00,
	0x74, 0x60, 0x26, 0x00, 0x00, 0x3d, 0x30, 0x56,
	0x61, 0x6c, 0x75, 0x65, 0x48, 0x44, 0x20, 0x43,
	0x6f, 0x72, 0x70, 0x6f, 0x72, 0x61, 0x74, 0x69,
	0x6f, 0x6e, 0x20, 0x56, 0x69, 0x64, 0x65, 0x6f,
	0x20, 0x43, 0x6f, 0x6e, 0x66, 0x65, 0x72, 0x65,
	0x6e, 0x63, 0x69, 0x6e, 0x67, 0x20, 0x45, 0x6e,
	0x64, 0x70, 0x6f, 0x69, 0x6e, 0x74, 0x00, 0x00,
	0x13, 0x56, 0x31, 0x2e, 0x31, 0x2e, 0x33, 0x2d,
	0x31, 0x37, 0x30, 0x34, 0x32, 0x38, 0x2e, 0x31,
	0x38, 0x34, 0x33, 0x00, 0x00, 0x26, 0xeb, 0x19,
	0x20, 0x02, 0x00, 0x09, 0x31, 0x01, 0x4d, 0x00,
	0x0a, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0c, 0x0a,
	0x01, 0x02, 0x01, 0xc0, 0x5d, 0x28, 0x51, 0x8a,
	0x10, 0x87, 0xa0, 0x17, 0xbd, 0x32, 0x8a, 0xcc,
	0x33, 0xce, 0x88, 0xa6, 0xa4, 0x7a, 0xd9, 0x35,
	0xfa, 0x01, 0x19, 0x08, 0x00, 0x74, 0x00, 0x65,
	0x00, 0x73, 0x00, 0x74, 0x00, 0x00, 0x2e, 0x01,
	0x04, 0x04, 0x00, 0x74, 0x00, 0x65, 0x00, 0x73,
	0x00, 0x74, 0x00, 0x00, 0xc0, 0x5d, 0x28, 0x51,
	0x8a, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
	0x02, 0x05, 0x00, 0x80, 0x80, 0xa8, 0xd2, 0xde,
	0x71, 0x91, 0x34, 0xbe, 0xb5, 0xa9, 0xd8, 0x5f,
	0xbf, 0x07, 0x22, 0x00, 0x88, 0x01, 0x00, 0x01,
	0x80, 0x01, 0x00, 0x01, 0x00, 0x02, 0x70, 0x00,
	0x01, 0x30, 0x05, 0x10, 0x01, 0x00, 0x00, 0x12
	};
}

TEST_F(H225RasRegressionTest, Bug53855)
{
	const std::size_t size_buff = sizeof(REGISTRATION_REQUEST_53855);
	VS_PerBuffer buff{ REGISTRATION_REQUEST_53855, size_buff * 8, true };
	VS_RasMessage mess;
	ASSERT_TRUE(mess.Decode(buff));

	ASSERT_EQ(mess.tag, VS_RasMessage::e_registrationRequest);

	VS_RasRegistrationRequest* rrq = mess;
	ASSERT_NE(rrq, nullptr);

	ASSERT_EQ(rrq->cryptoTokens.size(), 1);
	ASSERT_EQ(rrq->cryptoTokens[0].tag, VS_H225CryptoH323Token::e_cryptoEPPwdHash);
	auto crypto_token = static_cast<VS_H225CryptoEPPwdHash*>(rrq->cryptoTokens[0].choice);

	ASSERT_EQ(crypto_token->alias.tag, VS_H225AliasAddress::e_h323_ID);

	const char h323_id[] = "test";

	auto crypto_token_alias = crypto_token->alias.String();
	EXPECT_EQ(crypto_token_alias.length(), sizeof(h323_id) - 1);
	EXPECT_STREQ(crypto_token_alias.c_str(), h323_id);

	ASSERT_EQ(rrq->terminalAlias.size(), 1);
	ASSERT_EQ(rrq->terminalAlias[0].tag, VS_H225AliasAddress::e_h323_ID);

	auto terminal_alias = rrq->terminalAlias[0].String();
	EXPECT_EQ(terminal_alias.length(), sizeof(h323_id) - 1);
	EXPECT_STREQ(terminal_alias.c_str(), h323_id);
}


#undef EXPECT_ARRAY_EQ