#pragma once

#include <map>
#include <memory>
#include <vector>

#include "VS_H235_DiffieHellman.h"
#include "VS_H235SecurityCapability.h"
#include "../VS_H323Lib.h"

class VS_H235Authenticator{
	std::map<dh_oid, dh_params> m_dh_map;
	std::vector<uint8_t> m_crytoMasterKey;
	const std::map<encryption_mode, VS_GwAsnObjectId> *m_pAvailable_encryption_algms;
	bool m_security_enabled = false;
public:
	VS_H235Authenticator();
	VS_H235Authenticator(const VS_H235Authenticator&) = delete;
	VS_H235Authenticator& operator=(const VS_H235Authenticator&) = delete;

	void SetSecurityEnabled(const bool val) { m_security_enabled = val;}

	bool PrepareTokens(VS_H225ArrayOf_ClearToken &tokens);
	bool ValidateTokens(VS_H225ArrayOf_ClearToken &tokens);
	bool InitH235MasterKey(const dh_oid oid, const VS_H235ClearToken& token, VS_H235_DiffieHellman& our_dh_params);

	/**
	Make  secure capability refers to non secure capability with capabilityTableEntryNumber=refer_to_entry  that do
	contain a transmit, receive, or receiveAndTransmit AudioCapability, VideoCapability,
	DataApplicationCapability, or similar capability indicated by a NonStandardParameter only capability
	*/
	bool AddSecureCapability(VS_H245CapabilityTableEntry   &table_entry, const std::uint32_t my_entry_number, const std::uint32_t refer_to_entry) const;
	encryption_mode FindH235EncryptionMode(VS_H245TerminalCapabilitySet * tcs, const std::uint32_t capabilityTableEntryNumberReferTo) const;

	VS_H245H235Media* InitGenericH235Media(const VS_Asn * capability_refer_to, const int tag_refer_to, const encryption_mode m) const;
	std::tuple<bool/*res*/, encryption_mode, unsigned /*media_data tag*/> ReceiveH235Media(const VS_H245H235Media *media_params) const;

	std::vector<uint8_t> GetShortKeyFromMasterKey(const encryption_mode m) const;
	std::vector<uint8_t> GenerateSessionKey(const encryption_mode m) const;

	bool  ReadEncryptionSync(const VS_H245EncryptionSync & sync, VS_H235SecurityCapability &inOutsec_cap) const;
	bool  BuildEncryptionSync(VS_H245EncryptionSync & OUT_sync, VS_H235SecurityCapability &inOutsec_cap) const;
};