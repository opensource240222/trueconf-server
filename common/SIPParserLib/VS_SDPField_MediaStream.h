#pragma once

#include "../SIPParserBase/VS_BaseField.h"
#include "VS_SIPGetInfoInterface.h"

#include <boost/regex.hpp>

#include <memory>
#include <vector>
#include <string>
#include "VS_RTSP_Transport.h"
#include "VS_BFCPAttribute.h"

#include "std-generic/undef_windows.h" // this should be last

class VS_SDPCodec;
class VS_SDPField_Connection;
class VS_SDPField_Bandwidth;

enum class SDPMediaType : int;
enum eSDP_RTPPROTO;
enum eSDP_MediaChannelDirection;
enum eSDP_ContentType : int;
enum eSDP_Setup;
enum eSDP_Connection;

class VS_SDPField_MediaStream: public VS_BaseField
{
public:
	const static boost::regex e1, e2, e3;

	TSIPErrorCodes Decode(VS_SIPBuffer &aBuffer) override;
	TSIPErrorCodes Encode(VS_SIPBuffer &aBuffer) const override;

	void Clean() noexcept override;

	SDPMediaType GetMediaType() const;
	void SetMediaType(const SDPMediaType _media);

	net::port GetLocalPort() const;
	void SetLocalPort(net::port port);
	net::port GetPort() const;
	void SetPort(net::port port);
	net::port GetPortRange() const;
	void SetPortRange(net::port port_range);

	eSDP_RTPPROTO GetProto() const;
	void SetProto(eSDP_RTPPROTO proto);

	std::unique_ptr<VS_SDPCodec> GetBestCodec(bool use_remote_codec = true, bool use_remote_pt = true) const;

	bool IsRecv() const;
	bool IsSend() const;
	bool IsEnabled() const;

	eSDP_MediaChannelDirection GetMediaDirection() const;
	void SetMediaDirection(eSDP_MediaChannelDirection direction);

	const std::string &GetHost() const;
	void SetHost(std::string host);

	VS_SDPField_Connection* GetConnection() const;
	bool SetConnection(const VS_SDPField_Connection* conn);

	void SetBandwidth(const unsigned int _bandwidth);
	unsigned int GetBandwidth() const;

	eSDP_ContentType GetContent() const;
	void SetContent(eSDP_ContentType content);

	const std::string &GetLabel() const;
	void SetLabel(std::string label);

	eSDP_Setup GetSetup() const;
	void SetSetup(eSDP_Setup setup);

	eSDP_Connection GetConnectionAttr() const;
	void SetConnectionAttr(eSDP_Connection connection);

	std::uint16_t GetBFCPFloorCtrl() const;
	void SetBFCPFloorCtrl(std::uint16_t role);

	bfcp::ConferenceID GetBFCPConfID() const;
	void SetBFCPConfID(bfcp::ConferenceID conf_id);

	bfcp::UserID GetBFCPUserID() const;
	void SetBFCPUserID(bfcp::UserID user_id);

	bfcp::FloorID GetBFCPFloorID() const;
	void SetBFCPFloorID(bfcp::FloorID floor_id);

	std::vector<std::string>& BFCPFloorLabels();

	const std::vector<std::shared_ptr<const VS_SDPCodec>>& GetLocalCodecs() const;
	void AddLocalCodec(std::shared_ptr<const VS_SDPCodec> codec);
	void ClearLocalCodecs();
	void CopyLocalCodecsFrom(const VS_SIPGetInfoInterface& ctx);

	const std::vector<std::unique_ptr<VS_SDPCodec>>& GetRemoteCodecs() const;
	void ClearRemoteCodecs();
	void CopyRemoteCodecsFrom(VS_SDPField_MediaStream* ms);

	bool GetFIRSupport() const;
	void SetFIRSupport(bool value);

	const std::string &GetControl() const;
	const std::string &GetRawFmtp() const;

	std::pair<uint32_t, uint32_t> GetOurSsrcRange() const { return m_our_ssrc_range; }
	void SetOurSsrcRange(const std::pair<uint32_t, uint32_t>& p) { m_our_ssrc_range = p; }

	const std::string &GetOurIceUfrag() const { return m_our_ice_ufrag; }
	void SetOurIceUfrag(std::string s) { m_our_ice_ufrag = std::move(s); }

	const std::string &GetRemoteIceUfrag() const { return m_remote_ice_ufrag; }
	void SetRemoteIceUfrag(std::string s) { m_remote_ice_ufrag = std::move(s); }

	const std::string &GetOurIcePwd() const { return m_our_ice_pwd; }
	void SetOurIcePwd(std::string s) { m_our_ice_pwd = std::move(s); }

	const std::string &GetRemoteIcePwd() const { return m_remote_ice_pwd; }
	void SetRemoteIcePwd(std::string s) { m_remote_ice_pwd = std::move(s); }

	const std::string &GetOurCryptoKey() const { return m_our_crypto_key; }
	void SetOurCryptoKey(std::string s) { m_our_crypto_key = std::move(s); }

	const std::string &GetRemoteCryptoKey() const { return m_remote_crypto_key; }
	void SetRemoteCryptoKey(std::string s) { m_remote_crypto_key = std::move(s); }

	const std::string &GetMessageURL() const { return m_message_url; }
	void SetMessageURL(std::string s) { m_message_url = std::move(s); }

	std::vector<std::string> &AcceptTypes() { return m_accept_types; }
	void SetAcceptTypes(std::vector<std::string> types) { m_accept_types = std::move(types); }

	VS_SDPField_MediaStream();
	~VS_SDPField_MediaStream();

private:
//	void ConfigureMyCodecTable();
//	bool FindBestCodec();
//	bool CheckCodec(const int codec_type) const;

	TSIPErrorCodes EncodeBFCPStream(VS_SIPBuffer &aBuffer) const;

	SDPMediaType										m_media_type;
	eSDP_MediaChannelDirection							m_direction;
	net::port											m_local_port;
	net::port											m_port;
	net::port											m_port_range;
	eSDP_RTPPROTO										m_proto;
	std::unique_ptr<VS_SDPField_Connection>				m_connection;
	std::unique_ptr<VS_SDPField_Bandwidth>				m_bandwidth;
	eSDP_ContentType									m_content;
	std::string											m_label;
	bool												m_is_fir_supported;
	std::vector<std::shared_ptr<const VS_SDPCodec>>		m_local_codecs;
	std::vector<std::unique_ptr<VS_SDPCodec>>			m_remote_codecs;
	std::vector<int>									m_his_pt_table; // TODO: Only used temporarily in Decode, should it be moved there?

	// RTSP
	std::string											m_control;
	std::string											m_raw_fmtp;

	// BFCP
	eSDP_Setup											m_setup;
	eSDP_Connection										m_connection_attr;
	std::uint16_t										m_bfcp_floor_ctrl;
	bfcp::ConferenceID									m_bfcp_conf_id;
	bfcp::UserID										m_bfcp_user_id;
	bfcp::FloorID										m_bfcp_floor_id;
	std::vector<std::string>							m_bfcp_floor_labels;

	bool												m_no_rtpmap_audio_static_pt;
	bool												m_no_rtpmap_video_static_pt;

	bool												m_ice_enabled;
	bool												m_srtp_enabled;
	std::pair<uint32_t, uint32_t>						m_our_ssrc_range;
	std::string											m_our_ice_ufrag, m_our_ice_pwd, m_remote_ice_ufrag, m_remote_ice_pwd;
	std::string											m_our_crypto_key, m_remote_crypto_key;

	std::string											m_message_url;
	std::vector<std::string>							m_accept_types;
};

std::unique_ptr<VS_BaseField> VS_SDPField_MediaStream_Instance();