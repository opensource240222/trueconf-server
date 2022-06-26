#pragma once

#include "../SIPParserBase/VS_SIPError.h"
#include "VS_SIPObjectFactory.h"
#include "VS_SDPObjectFactory.h"

#include "VS_SIPUpdateInfoInterface.h"
#include "VS_SIPGetInfoInterface.h"
#include <string>
#include "std-generic/attributes.h"


class VS_SIPRequest;
class VS_SIPResponse;
class VS_SIPParserInfo;
class VS_SIPMetaField;
class VS_SDPMetaField;
class VS_MetaField_MediaControl_XML;
class VS_MetaField_DTMF_Relay;
class VS_MetaField_PIDF_XML;
class VS_SIPAuthInfo;
class VS_SIPInstantMessage;
class VS_SDPField_MediaStream;

enum eContentType : int;
enum eMessageType;
enum eStartLineType : int;
enum eConnectionType : int;

class VS_SIPMessage: public VS_SIPError
{
public:
	TSIPErrorCodes Decode(const char* aInput, std::size_t aSize);
	TSIPErrorCodes Encode(char* aOutput, std::size_t &aSize) const;
	TSIPErrorCodes Encode(std::string &aOutput) const;

	void SetContentType(const eContentType type);
	eContentType GetContentType() const;

	eMessageType GetMessageType() const;

	const VS_SIPMetaField* GetSIPMetaField() const;
	const VS_SDPMetaField* GetSDPMetaField() const;
	bool IsFastUpdatePicture() const;
	std::shared_ptr<VS_SIPAuthInfo> GetAuthInfo() const;
	const VS_SIPInstantMessage* GetSIPInstantMessage() const;
	std::vector<std::string> GetAcceptedTextFormats() const;

	bool InsertSIPField(const VS_SIPObjectFactory::SIPHeader header, const VS_SIPGetInfoInterface& info);
	bool InsertSDPField(const VS_SDPObjectFactory::SDPHeader header, const VS_SIPGetInfoInterface& info);
	bool InsertMediaStreams(const VS_SIPGetInfoInterface& getInfo);
	bool InsertField_MediaControl_FastUpdatePicture(const VS_SIPGetInfoInterface& info);
	bool InsertField_DTMF_Relay(const VS_SIPGetInfoInterface& info, const char dtmf_digit);
	bool InsertField_PIDF(const VS_SIPGetInfoInterface& info);
	bool InsertInstantMessage(const VS_SIPGetInfoInterface& info, string_view message);

	void EraseSIPField(const VS_SIPObjectFactory::SIPHeader header) const;
	void EraseSDPField(const VS_SDPObjectFactory::SDPHeader header) const;

	bool HasSIPField(const VS_SIPObjectFactory::SIPHeader header) const;
	bool HasSDPField(const VS_SDPObjectFactory::SDPHeader header) const;

	bool UpdateOrIgnoreSIPField(const VS_SIPObjectFactory::SIPHeader header, const VS_SIPGetInfoInterface& info);
	bool UpdateOrInsertSIPField(const VS_SIPObjectFactory::SIPHeader header, const VS_SIPGetInfoInterface& info);

	bool UpdateOrIgnoreSDPField(const VS_SDPObjectFactory::SDPHeader header, const VS_SIPGetInfoInterface& info);
	bool UpdateOrInsertSDPField(const VS_SDPObjectFactory::SDPHeader header, const VS_SIPGetInfoInterface& info);

// Only for Incom SIP-message
	bool FillInfoByInviteMessage(VS_SipUpdateInfoInterface& updateInfo);
	bool FillInfoFromSDP(const VS_SIPGetInfoInterface& getInfo,
	                     VS_SipUpdateInfoInterface& updateInfo, const bool is_offer);
	bool FillInfoUserAgent(VS_SipUpdateInfoInterface& updateInfo);

	bool CheckAuth(const VS_SIPAuthInfo* info);

	VS_SIPMessage();
	VS_SIPMessage(const char* aInput, const std::size_t aSize);
	~VS_SIPMessage();

	string_view CallID() const;
	string_view Branch() const;
	eStartLineType GetMethod() const;
	std::int32_t GetCSeq() const;

	std::string GetFrom() const;
	std::string GetTo() const;

	string_view DisplayNameMy() const;

	net::protocol GetConnectionType() const;

	string_view UserAgent() const;

	std::chrono::steady_clock::duration GetRetryAfterInterval() const;
	void CopyAllContacts(std::vector<std::string>& OUT_contacts) const;

	string_view Server() const;

	int GetResponseCode() const;
	int GetResponseCodeClass() const;

	static void InsertMediaStreamICE(const VS_SIPGetInfoInterface& info, VS_SDPField_MediaStream* ms);
	static void InsertMediaStreamSRTP(const VS_SIPGetInfoInterface& info, VS_SDPField_MediaStream* ms);
protected:
	eContentType										m_content_type;
	std::unique_ptr<VS_SIPMetaField>			m_sip_meta_field;
	std::unique_ptr<VS_SDPMetaField>			m_sdp_meta_field;
	std::unique_ptr<VS_MetaField_MediaControl_XML>			m_mc_meta_field;
	std::unique_ptr<VS_MetaField_PIDF_XML>				m_pidf_meta_field;
	std::unique_ptr<VS_MetaField_DTMF_Relay>				m_dtmf_relay;
	std::unique_ptr<VS_SIPInstantMessage>					m_sip_message;

};