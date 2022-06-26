#include "VS_H235_Authenticator.h"

#include "tools/H323Gateway/Lib/h235/VS_H235CryptoEngine.h"
#include "tools/H323Gateway/Lib/src/VS_H225Messages.h"

#include "std/debuglog/VS_Debug.h"

#include <openssl/bn.h>

#include <algorithm>
#include <cassert>

#define DEBUG_CURRENT_MODULE VS_DM_H323PARSER

const VS_GwAsnObjectId h235ProtocolIdentifier{ 0, 0, 8, 235, 0, 3, 24 };

#define DES_56_CBC {1,3,14,3,2,7}
#define DES_56_EOFB_64 {0,0,8,235,0,3,28}
#define TripleDES_168_EOFB_64 {0,0,8,235,0,3,29}
#define TripleDES_168_CBC {1,3,14,3,2,17}
#define AES_128_EOFB {0,0,8,235,0,3,30}
#define AES_128_CBC {2,16,840,1,101,3,4,1,2}
#define NULL_ENCRYPTION {0,0,8,235,0,3,26}

const std::map<dh_oid/*dh_group*/, std::map<encryption_mode, VS_GwAsnObjectId>/*available algms for this group*/> g_encryption_algms = {
	/*{ dh_oid::DHdummy,
	{ { encryption_mode::DES_56CBC, DES_56_CBC }, { encryption_mode::DES_56EOFB_64, DES_56_EOFB_64 }, { encryption_mode::no_encryption, NULL_ENCRYPTION } } },*/
	{ dh_oid::DH1024,
		{
		  { encryption_mode::AES_128CBC, static_cast<VS_GwAsnObjectId>(std::initializer_list<unsigned>(AES_128_CBC)) },
		/*{ encryption_mode::AES_128EOFB, AES_128_EOFB },*/ /*{ encryption_mode::TripleDES_168CBC, TripleDES_168_CBC },
		  { encryption_mode::TripleDES_168EOFB_64, TripleDES_168_EOFB_64 },*/
		  { encryption_mode::no_encryption, static_cast<VS_GwAsnObjectId>(std::initializer_list<unsigned>(NULL_ENCRYPTION)) }
		}
	}
	/*{ dh_oid::DH1536,
	{ {encryption_mode::AES_128CBC, AES_128_CBC } }}*/
};

const std::map<encryption_mode, VS_GwAsnObjectId> null_algs = { { encryption_mode::no_encryption, static_cast<VS_GwAsnObjectId>(std::initializer_list<unsigned>(NULL_ENCRYPTION)) } };

VS_H235Authenticator::VS_H235Authenticator() :m_dh_map(VS_H235_DiffieHellman::CreateDiffieHellmanParams()), m_pAvailable_encryption_algms(nullptr){}

bool VS_H235Authenticator::InitH235MasterKey(const dh_oid oid, const VS_H235ClearToken& token, VS_H235_DiffieHellman& our_dh_params){
	const auto& dh_set = token.dhkey;
	if (!dh_set.filled || !dh_set.halfkey.filled) {
		dstream3 << "Missing params for H235 key exchange for tokenOID=" << token.tokenOID << " \n";
		return false;
	}

	VS_H235_DiffieHellman remote_dh(our_dh_params); // new token with same p and g
	remote_dh.Decode_HalfKey(dh_set.halfkey.value);
	if (dh_set.generator.filled) remote_dh.Decode_G(dh_set.generator.value);
	if (dh_set.modSize.filled) remote_dh.Decode_P(dh_set.modSize.value);

	if (remote_dh.IsZeroDHGroup()){
		dstream3 << "Warning\tReceived NULL DH Group. Using no encryption.\n";
		m_pAvailable_encryption_algms = &null_algs;
		return true;
	}

	if (!remote_dh.TestParamsEqual(our_dh_params)) {
		dstream3 << "Error\tDiffie-Hellman params are not equal!\n";
		return false;
	}

	dstream3 << "Setting Diffie-Hellman group " << token.tokenOID << " \n";

	our_dh_params.SetRemoteKey(remote_dh.GetPublicKey<const BIGNUM*>());
	auto algms = g_encryption_algms.find(oid);
	if (algms == g_encryption_algms.end()) {
		dstream3 << "Can't find Diffie-Hellman group\n";
		return false;
	}

	m_pAvailable_encryption_algms = &algms->second;
	assert(m_pAvailable_encryption_algms != nullptr);

	if (our_dh_params.ComputeMasterKey(m_crytoMasterKey)) {
		unsigned gen(0), modSize(0);
		switch (oid)
		{
		case dh_oid::DH1024:
			gen = 2;
			modSize = 1024;
			break;
		default:
			assert(false);
			break;
		}

		dstream4 << "MasterKey generated successfully! generator='" << gen << "', modSize='" << modSize << "'\n";
		return true;
	}
	else
		return false;
}

bool VS_H235Authenticator::ValidateTokens(VS_H225ArrayOf_ClearToken &tokens){
	if (!m_security_enabled || !tokens.filled || tokens.empty()) return true;	// no security
	bool found(false);

	for (auto& it : m_dh_map){
		auto& tokenOID = std::get<0>(it.second);
		auto& our_dh = std::get<1>(it.second);

		if (found){
			dstream3 << "Skipping Lower Diffie-Hellman group " << tokenOID << " \n";
			continue;
		}

		for (std::size_t i = 0; i < tokens.size(); ++i){
			const VS_H235ClearToken& token = tokens[i];
			if (tokenOID == token.tokenOID){
				found = InitH235MasterKey(it.first, token, our_dh);
				break;
			}
		}
	}

	if (!found)	dstream3 << "Mutual encription params for H235 weren't found.\n";
	return found;
}

bool FillDHToken(VS_H235ClearToken &token, dh_params &params) {
	auto &oid = std::get<0>(params);
	auto &df = std::get<1>(params);

	VS_H235DHset &dh = token.dhkey;
	df.Encode_G(dh.generator.value);
	dh.generator.filled = true;

	df.Encode_P(dh.modSize.value);
	dh.modSize.filled = true;

	if (!df.GenerateHalfKey()) return false;
	df.Encode_HalfKey(dh.halfkey.value);
	dh.halfkey.filled = true;
	dh.filled = true;

	token.tokenOID = oid;
	token.filled = true;
	return true;
}

bool VS_H235Authenticator::PrepareTokens(VS_H225ArrayOf_ClearToken &tokens) {
	if (!m_security_enabled) return true;
	const auto size = m_dh_map.size() + 1;
	VS_H235ClearToken * clear_tokens = new VS_H235ClearToken[size];

	VS_H235ClearToken &version_token = clear_tokens[0];
	version_token.tokenOID = h235ProtocolIdentifier;
	version_token.filled = true;

	size_t i = 1;
	for (auto it = m_dh_map.begin(), _end = m_dh_map.end(); it != _end; ++it, ++i){
		FillDHToken(clear_tokens[i], it->second);
	}

	tokens.reset(clear_tokens, size);
	return true;
}

void AddEncryptionAlgorithm(VS_H245MediaEncryptionAlgorithm &alg, const VS_GwAsnObjectId& encryption){
	alg.tag = VS_H245MediaEncryptionAlgorithm::e_algorithm;
	VS_AsnObjectId *id = new VS_AsnObjectId(encryption);
	alg.choice = id;
	alg.filled = true;

}

bool VS_H235Authenticator::AddSecureCapability(VS_H245CapabilityTableEntry   &table_entry, const std::uint32_t my_entry_number, const std::uint32_t refer_to_entry) const{
	if (refer_to_entry < 1 || !m_pAvailable_encryption_algms) return false;

	VS_H245MediaEncryptionAlgorithm *algs = new VS_H245MediaEncryptionAlgorithm[m_pAvailable_encryption_algms->size()];

	size_t i = 0;
	for (const auto& algorithm : *m_pAvailable_encryption_algms){
		VS_H245MediaEncryptionAlgorithm &alg = algs[i];
		AddEncryptionAlgorithm(alg, algorithm.second);
		++i;
	}

	VS_H245H235SecurityCapability* cap = new VS_H245H235SecurityCapability;
	cap->mediaCapability.value = refer_to_entry;
	cap->mediaCapability.filled = true;
	cap->filled = true;

	VS_H245EncryptionCapability &encryptions = cap->encryptionAuthenticationAndIntegrity.encryptionCapability;
	cap->encryptionAuthenticationAndIntegrity.filled = true;
	encryptions.reset(algs, m_pAvailable_encryption_algms->size());

	table_entry.capability.tag = VS_H245Capability::e_h235SecurityCapability;
	table_entry.capability.filled = true;
	table_entry.capability.choice = cap;
	table_entry.capabilityTableEntryNumber.value = my_entry_number;
	table_entry.capabilityTableEntryNumber.filled = true;
	table_entry.filled = true;

	return true;
}

encryption_mode VS_H235Authenticator::FindH235EncryptionMode(VS_H245TerminalCapabilitySet * tcs, const std::uint32_t capabilityTableEntryNumberReferTo) const{
	assert(tcs != nullptr);
	if (!m_pAvailable_encryption_algms) {
		dstream3 << "Use no encryption!\n";
		return encryption_mode::no_encryption;
	}

	encryption_mode res = encryption_mode::no_encryption;
	bool found(false);

	for (const auto& table_entry : tcs->capabilityTable){
		if (!table_entry.capabilityTableEntryNumber.filled || !table_entry.capability.filled) continue;
		if (table_entry.capability.tag != VS_H245Capability::e_h235SecurityCapability) continue;

		VS_H245H235SecurityCapability *cap = static_cast<VS_H245H235SecurityCapability *>(table_entry.capability.choice);
		if (!cap->mediaCapability.filled || cap->mediaCapability.value != capabilityTableEntryNumberReferTo) continue;

		if (!cap->encryptionAuthenticationAndIntegrity.filled) continue;
		if (!cap->encryptionAuthenticationAndIntegrity.encryptionCapability.filled) continue;

		const auto &encryptions = cap->encryptionAuthenticationAndIntegrity.encryptionCapability;
		//VS_H245MediaEncryptionAlgorithm *algs = static_cast<VS_H245MediaEncryptionAlgorithm *>(encryptions.asns);
		auto begin = encryptions.begin(), end = encryptions.end();

		for (const auto& algorithm : *m_pAvailable_encryption_algms){
			if (found){
				dstream3 << "Skipping  algorithm " << algorithm.second;
				continue;
			}

			auto it = std::find_if(begin, end, [&algorithm](const VS_H245MediaEncryptionAlgorithm& el) -> bool {
				if (!el.filled || el.tag != VS_H245MediaEncryptionAlgorithm::e_algorithm) return false;

				VS_AsnObjectId *id = static_cast<VS_AsnObjectId *>(el.choice);
				return algorithm.second == *id;
			});

			if (it != end){
				dstream3 << "Found encryption mode " << algorithm.second << " with entry = " << table_entry.capabilityTableEntryNumber.value << " ,refers to entry " << capabilityTableEntryNumberReferTo << " \n";
				res = algorithm.first;
				found = true;
			}
		}
	}

	if (!found) dstream3 << "Encryption mode wasn't found\n";
	return res;
}

unsigned ConvertH245toH235(const unsigned h245tag) {
	switch (h245tag)
	{
	case VS_H245DataType::e_audioData:
		return VS_H245H235Media_MediaType::e_audioData;
	case VS_H245DataType::e_videoData:
		return VS_H245H235Media_MediaType::e_videoData;
	case VS_H245DataType::e_data:
		return VS_H245H235Media_MediaType::e_data;
	case VS_H245DataType::e_redundancyEncoding:
		return VS_H245H235Media_MediaType::e_redundancyEncoding;
	case VS_H245DataType::e_multiplePayloadStream:
		return VS_H245H235Media_MediaType::e_multiplePayloadStream;
	case VS_H245DataType::e_depFfec:
		return VS_H245H235Media_MediaType::e_depFfec;
	case VS_H245DataType::e_fec:
		return VS_H245H235Media_MediaType::e_fec;
	default:
		return h245tag;
	}
}

unsigned ConvertH235toH245(const unsigned h235tag) {
	switch (h235tag)
	{
	case VS_H245H235Media_MediaType::e_audioData:
		return VS_H245DataType::e_audioData;
	case VS_H245H235Media_MediaType::e_videoData:
		return VS_H245DataType::e_videoData;
	case VS_H245H235Media_MediaType::e_data:
		return VS_H245DataType::e_data;
	case VS_H245H235Media_MediaType::e_redundancyEncoding:
		return VS_H245DataType::e_redundancyEncoding;
	case VS_H245H235Media_MediaType::e_multiplePayloadStream:
		return VS_H245DataType::e_multiplePayloadStream;
	case VS_H245H235Media_MediaType::e_depFfec:
		return VS_H245DataType::e_depFfec;
	case VS_H245H235Media_MediaType::e_fec:
		return VS_H245DataType::e_fec;
	default:
		return h235tag;
	}
}

VS_H245H235Media* VS_H235Authenticator::InitGenericH235Media(const VS_Asn * capability_refer_to, const int tag_refer_to, const encryption_mode m) const{
	assert(m != encryption_mode::no_encryption);

	const std::size_t size = 1;
	const auto& algorithm = m_pAvailable_encryption_algms->find(m);
	assert(algorithm != m_pAvailable_encryption_algms->end());
	if (algorithm == m_pAvailable_encryption_algms->end()) return nullptr;	// it means we have made mistake in or after VS_H235Authenticator::FindH235EncryptionMode

	VS_H245MediaEncryptionAlgorithm *algs = new VS_H245MediaEncryptionAlgorithm[size];
	AddEncryptionAlgorithm(algs[0], algorithm->second);

	VS_H245H235Media *result = new VS_H245H235Media;
	VS_H245EncryptionCapability &encryptions = result->encryptionAuthenticationAndIntegrity.encryptionCapability;
	result->encryptionAuthenticationAndIntegrity.filled = true;
	encryptions.reset(algs, size);

	result->mediaType.choice = const_cast<VS_Asn *>(capability_refer_to);
	result->mediaType.tag = ConvertH245toH235(tag_refer_to);
	result->mediaType.filled = true;
	result->filled = true;

	return result;
}

std::tuple<bool/*res*/, encryption_mode, unsigned /*media_data tag*/> VS_H235Authenticator::ReceiveH235Media(const VS_H245H235Media *media_params) const{
	encryption_mode OUT_mode = encryption_mode::no_encryption;
	unsigned OUT_data_type_tag(media_params->mediaType.tag);

	try{
		if (!media_params || !media_params->filled) throw 0;
		const auto &encryption = media_params->encryptionAuthenticationAndIntegrity;
		const auto &cap = encryption.encryptionCapability;
		if (!encryption.filled || !cap.filled || cap.empty()) throw 0;

		//auto &&algs = static_cast<const VS_H245MediaEncryptionAlgorithm *>(cap.data());
		auto begin = cap.begin(), end = cap.end();

		for (const auto& algorithm : *m_pAvailable_encryption_algms){
			auto it = std::find_if(begin, end, [&algorithm](const VS_H245MediaEncryptionAlgorithm& el) -> bool {
				if (!el.filled || el.tag != VS_H245MediaEncryptionAlgorithm::e_algorithm) return false;

				VS_AsnObjectId *id = static_cast<VS_AsnObjectId *>(el.choice);
				return algorithm.second == *id;
			});

			if (it != end){
				dstream3 << "Negotiated receive encryption algorithm " << algorithm.second << "\n";
				OUT_mode = algorithm.first;
				break;
			}
		}
	}
	catch (int){
		dstream3 << "No encryption algorithms provided in received OLC\n";
		return std::make_tuple(false, OUT_mode, OUT_data_type_tag);
	}

	OUT_data_type_tag = ConvertH235toH245(OUT_data_type_tag);
	return std::make_tuple(true, OUT_mode, OUT_data_type_tag);
}

unsigned GetKeySize(const encryption_mode m){
	switch (m)
	{
	case encryption_mode::AES_128CBC:
	//case encryption_mode::AES_128EOFB:
		return 16;
	case encryption_mode::no_encryption:
	default:
		break;
	}
	return 0;
}

std::vector<uint8_t> VS_H235Authenticator::GetShortKeyFromMasterKey(const encryption_mode m) const{
	if (m_crytoMasterKey.empty()){
		assert(false);
		dstream3 << "ERROR\tAttempt to retrieve short key from empty master key.\n";
		return std::vector<uint8_t>();
	}

	unsigned key_size = GetKeySize(m);
	key_size = std::min(static_cast<unsigned>(m_crytoMasterKey.size()), key_size);

	/*
		from standart: "Each entity shall take the appropriate least significant bits from the common shared Diffie-Hellman
		secret for the key encryption key (master key)"
	*/
	std::vector<uint8_t> short_key(m_crytoMasterKey.end() - key_size, m_crytoMasterKey.end());
	return short_key;
}

bool  VS_H235Authenticator::ReadEncryptionSync(const VS_H245EncryptionSync & sync, VS_H235SecurityCapability &inOutsec_cap) const{
	if (!sync.synchFlag.filled) {
		dstream3 << "H235\tError: No synchFlag provided in encryptionSync\n";
		return false;
	}
	inOutsec_cap.syncFlag = sync.synchFlag.value;

	VS_H323H235Key h235key;
	if (!const_cast<VS_H245EncryptionSync &>(sync).h235Key.DecodeSubType(&h235key)) {
		dstream3 << "H235\tError: H235key decoding failed.\n";
		return false;
	}

	switch (h235key.tag)
	{
	case VS_H323H235Key::e_secureSharedSecret:
	{
		VS_H323V3KeySyncMaterial  *v3data = h235key;
		assert(v3data != nullptr);
		if (!v3data) return false;

		if (!v3data->algorithmOID.filled)	dstream3 << "H235\tWarning: No algo set in encryptionSync\n";
		if (!v3data->encryptedSessionKey.filled){
			dstream3 << "H235\tError: No session key provided in encryptionSync\n";
			return false;
		}

		std::vector<uint8_t> key = GetShortKeyFromMasterKey(inOutsec_cap.m);
		VS_H235CryptoEngine h235crypto;
		if (!h235crypto.Init(key, inOutsec_cap.m)) return false;

		auto &keyBit_buff = v3data->encryptedSessionKey.value;
		uint8_t * keyData = static_cast<uint8_t *>(keyBit_buff.GetData());
		if (!h235crypto.Decrypt(keyData, keyBit_buff.ByteSize(), nullptr, false, inOutsec_cap.h235_sessionKey)){
			dstream3 << "H235 Error\tCan't decrypt session key!\n";
			return false;
		}
	}
	return true;
	case VS_H323H235Key::e_sharedSecret:
		dstream3 << "H235Key\tShared Secret not supported\n";
		return false;
	case VS_H323H235Key::e_certProtectedKey:
		dstream3 << "H235Key\tProtected Key not supported\n";
		return false;
	case VS_H323H235Key::e_secureChannel:
		dstream3 << "H235Key\tSecure Channel not supported\n";
		return false;
	default:
		dstream3 << "H235Key\tUnknown key type\n";
		return false;
	}
	return true;
}

VS_GwAsnObjectId GetAlgOID(encryption_mode m){
	switch (m)
	{
	case encryption_mode::no_encryption:
		return static_cast<VS_GwAsnObjectId>(std::initializer_list<unsigned>(NULL_ENCRYPTION));
	case encryption_mode::AES_128CBC:
		return static_cast<VS_GwAsnObjectId>(std::initializer_list<unsigned>(AES_128_CBC));
	/*case encryption_mode::AES_128EOFB:
		return AES_128_EOFB;*/
	default:
		assert(false);
		return static_cast<VS_GwAsnObjectId>(std::initializer_list<unsigned>(NULL_ENCRYPTION));
	}
}

std::vector<uint8_t> VS_H235Authenticator::GenerateSessionKey(const encryption_mode m) const
{
	VS_H235CryptoEngine crypter;
	if (!crypter.Init(GetShortKeyFromMasterKey(m), m)) {
		dstream3 << "ERROR\tEncyption instance can't be init!\n";
		return std::vector<uint8_t>();
	}
	return crypter.GenerateKey(GetKeySize(m));
}

bool  VS_H235Authenticator::BuildEncryptionSync(VS_H245EncryptionSync & OUT_sync, VS_H235SecurityCapability &inOutsec_cap) const{
	OUT_sync.synchFlag.value = inOutsec_cap.syncFlag;
	OUT_sync.synchFlag.filled = true;

	VS_H323H235Key h235key;
	h235key.tag = VS_H323H235Key::e_secureSharedSecret;

	VS_H323V3KeySyncMaterial  *v3data = new VS_H323V3KeySyncMaterial;
	v3data->algorithmOID = GetAlgOID(inOutsec_cap.m);
	v3data->algorithmOID.filled = true;

	VS_H235CryptoEngine crypter;
	if (!crypter.Init(GetShortKeyFromMasterKey(inOutsec_cap.m), inOutsec_cap.m)){
		dstream3 << "ERROR\tEncyption instance can't be init!\n";
		return false;
	}

	if(inOutsec_cap.h235_sessionKey.empty()) inOutsec_cap.h235_sessionKey = crypter.GenerateKey(GetKeySize(inOutsec_cap.m));

	bool ecryption_success(false);
	std::vector<uint8_t> encr_session_key;
	std::tie(ecryption_success, std::ignore) = crypter.Encrypt(inOutsec_cap.h235_sessionKey, nullptr, encr_session_key);
	if (!ecryption_success){
		dstream3 << "ERROR\tH235 session key wasn't encrypted.\n";
		return false;
	}

	v3data->encryptedSessionKey.value.AddBits(encr_session_key.data(), encr_session_key.size() * 8);
	v3data->encryptedSessionKey.filled = true;
	v3data->paramS.filled = true;		 		// just set as filled to achieve encoding success. From standart: "This field shall not be used for the CBC mode and shall be set to NULL"
	v3data->filled = true;

	h235key.choice = v3data;
	h235key.filled = true;
	OUT_sync.h235Key.filled = true;
	OUT_sync.filled = true;
	return OUT_sync.h235Key.EncodeSubType(&h235key);
}