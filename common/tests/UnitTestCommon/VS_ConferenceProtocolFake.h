#pragma once

#include "TrueGateway/interfaces/VS_ConferenceProtocolInterface.h"
#include "TrueGateway/interfaces/VS_ParserInterface.h"
#include "tools/Server/CommonTypes.h"
#include "tools/Server/VS_MediaChannelInfo.h"

class VS_ConferenceProtocolFake : public VS_ConferenceProtocolInterface
{
	typedef boost::asio::ip::udp::endpoint udp_endpoint_t;

	net::Endpoint vcs_addr, terminal_addr;

public:
	VS_ConferenceProtocolFake(VS_ParserInterface* parser_ = nullptr)
		: parser(parser_)
	{
	}

	bool InviteMethod(string_view /*dialogId*/, string_view /*fromId*/, string_view /*toId*/, const VS_ConferenceInfo &/*cfgInfo*/,
		string_view /*dnFromUTF8*/ = {}, bool /*newSession*/ = true, bool /*forceCreate*/ = false) override
	{
		return true;
	}


	bool InviteReplay(string_view /*dialogId*/, VS_CallConfirmCode /*confirm_code*/, bool /*isGroupConf*/, string_view /*confName*/, string_view /*toDisplayName*/) override
	{
		return true;
	}


	void Hangup(string_view /*dialogId*/) override
	{
	}

	void LoggedOutAsUser(string_view /*dialogId*/) override
	{
	}

	bool SetMediaChannels(string_view dialogId, const std::vector<VS_MediaChannelInfo>& channels, const std::string& existingConfID, std::int32_t bandwRcv) override
	{
		if (!parser)
			return false;

		last_media_channels = channels;
		net::port port = 6000;
		for (auto& x : last_media_channels)
		{
			x.our_rtcp_address = x.our_rtp_address = udp_endpoint_t{ vcs_addr.addr, vcs_addr.port };
			x.our_rtp_address.port(port++);
			x.our_rtcp_address.port(port++);

			x.remote_rtcp_address = x.remote_rtp_address = udp_endpoint_t{ terminal_addr.addr, terminal_addr.port };
		}
		parser->SetMediaChannels(dialogId, last_media_channels, existingConfID);
		return true;
	}


	void AsyncInvite(string_view, const gw::Participant &, string_view, const VS_ConferenceInfo&,
		std::function<void(bool, ConferenceStatus, const std::string&)> /*inviteResult*/, string_view /*dnFromUTF8*/, bool, bool) override
	{

	}

	void SetOurAddr(net::Endpoint addr)
	{
		vcs_addr = std::move(addr);
	}

	void SetRemoteAddr(net::Endpoint addr)
	{
		terminal_addr = std::move(addr);
	}

	bool S4B_InitBeforeCall(string_view, string_view, bool) override
	{
		return true;
	}

	bool PrepareForCall(string_view/*dialog_id*/, string_view /*from_id*/, bool /*create_session*/) override { return true; }

	VS_ParserInterface* parser;
	std::vector<VS_MediaChannelInfo> last_media_channels;

};
