#pragma once

#include "interfaces/VS_ConferenceProtocolInterface.h"
#include "net/Address.h"
#include "net/Port.h"
#include "net/UDPRouter.h"
#include "tools/Server/CommonTypes.h"
#include "statuslib/status_types.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/synchronized.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2/connection.hpp>

#include "std-generic/compat/memory.h"
#include <atomic>
#include <chrono>
#include <list>
#include <map>
#include <string>

struct VS_CallConfig;
class VS_CallConfigStorage;
class VS_ParserInterface;
class VS_ClientControlInterface;
class VS_Policy;
class VS_UserData;
namespace net {
	class LoggerInterface;
} //namespace net
namespace ts { struct IPool; }

class VS_TransportConnection
	: public VS_ConferenceProtocolInterface
	, public vs::enable_shared_from_this<VS_TransportConnection>
{
	class Channel;
	struct TranscoderInfo;

protected:
	VS_TransportConnection(
		boost::asio::io_service::strand& strand,
		std::shared_ptr<VS_ParserInterface> parser,
		const UserStatusFunction& get_status,
		const std::weak_ptr<ts::IPool>& transc_pool,
		const std::shared_ptr<net::LoggerInterface>& logger
	);
	static void PostConstruct(std::shared_ptr<VS_TransportConnection>& p)
	{
		p->Init();
	}

public:
	virtual ~VS_TransportConnection();

	void Close();

	void Accept(boost::asio::ip::tcp::socket&& socket, const void* data, size_t size);
	void Accept(net::UDPConnection&& connection, const void* data, size_t size);

	bool IsClosed() const;
	bool IsTrunkFull() const;
	void ResetInactivity();
	void ResetConnectionTimeout();

	net::protocol ConnectionProtocol() const
	{
		return m_connection_protocol;
	}
	bool IsIPv4() const
	{
		return m_is_ipv4;
	}

	bool PrepareTranscportConnection(const net::address& address, net::port port, net::protocol protocols, VS_ChannelID channel_id = e_noChannelID);
	std::string PrepareCallToSIP(string_view to_sip_id, const VS_UserData* from_user, const VS_CallConfig& config);
	boost::shared_ptr<VS_ClientControlInterface> FindLoggedinTranscoder(string_view to_call_id);

	void SetRegistrationConfiguration(VS_CallConfig config);
	void UpdateStatusRegistration(std::function<void(const std::shared_ptr<VS_ParserInterface>&)>&& exec);
	void ResetAllConfigsStatus();

private:
	// Called by Channel
	void OnChannelOpen(Channel* channel);
	void OnChannelClose(Channel* channel);

	void Init();
	Channel* GetCSChannel() const;
	void ScheduleTimer();
	void Timeout();

	// VS_ConferenceProtocolInterface
	void LoginUser(string_view dialog_id, string_view login, string_view password, std::chrono::steady_clock::time_point expire_time, string_view external_name, std::function<void (bool)> result_callback, std::function<void (void)> logout_cb, const std::vector<std::string>& aliases) override;
	void Logout(string_view dialog_id) override;
	void SetUserEndpointAppInfo(string_view dialog_id, string_view app_name, string_view version) override;
	bool InviteMethod(string_view dialog_id, string_view from_id, string_view to_id, const VS_ConferenceInfo& info, string_view dn_from_utf8 = {}, bool create_session = true, bool force_create = false) override;
	bool InviteReplay(string_view dialog_id, VS_CallConfirmCode confirm_code, bool isGroupConf, string_view conf_name, string_view to_display_name) override;
	void Hangup(string_view dialog_id) override;
	void LoggedOutAsUser(string_view dialog_id) override;
	bool SetMediaChannels(string_view dialog_id, const std::vector<VS_MediaChannelInfo>& channels, const std::string& existingConfID, int32_t bandw_rcv = 0) override;
	void UpdateDisplayName(string_view dialog_id, string_view display_name, bool update_immediately) override;
	void FastUpdatePicture(string_view dialog_id) override;
	void BitrateRestriction(string_view dialog_id, int type, int value, int scope) override;
	void AsyncInvite(string_view dialog_id, const gw::Participant &from, string_view to_id, const VS_ConferenceInfo& ci, std::function<void(bool redirect, ConferenceStatus status, const std::string& ip)> InviteResult, string_view dn_from_utf8, bool new_session = true, bool force_create = false) override;
	void RegisterNewConnection(const net::address& address, net::port port, net::protocol protocols, const VS_ChannelID channel_id) override;
	void CloseConnection(const net::address& address, net::port port, net::protocol protocol) override;
	// Put existing transcoder (that have another owner) to transcoder's table.
	void PutSharedTranscoder(string_view dialog_id, boost::shared_ptr<VS_ClientControlInterface> transcoder) override;
	// Used for cancel an outgoing call.
	// If call was not established yet, invokes InviteReply() with 'reject' state.
	// If call was already established, invokes Hangup().
	// For h323 call.
	void HangupOutcomingCall(string_view dialog_id) override;
	void SetForRegisteredUser() override;
	boost::shared_ptr<VS_ClientControlInterface> GetTranscoder(string_view dialog_id) override;

	// Returns existing or creates new transcoder.
	boost::shared_ptr<VS_ClientControlInterface> NewTranscoder(string_view dialog_id);
	void PutTranscoder(string_view dialog_id, boost::shared_ptr<VS_ClientControlInterface> transcoder, bool shared);

	void InviteFromVisi(std::string&& dialog_id, std::string&& from, std::string&& to, bool is_group_conf, bool use_new_dialog_id, std::string&& dn_from_utf8);
	void SetUserDialogID(string_view login, string_view dialog_id);

	static bool IsTCPKeepAliveAllowed();

	mutable boost::asio::io_service::strand m_strand;
	boost::asio::steady_timer m_timer;

	const std::shared_ptr<VS_ParserInterface> m_parser;
	const UserStatusFunction m_get_user_status;
	const std::weak_ptr<ts::IPool> m_transc_pool;
	std::shared_ptr<net::LoggerInterface> m_logger;

	std::vector<Channel*> m_channels;
	std::atomic<bool> m_is_closed;
	std::atomic<unsigned> m_n_requested_connections;
	// Cached data from the first (CS) channel
	net::protocol m_connection_protocol;
	bool m_is_ipv4;

	using transcoders_map_t = vs::map<std::string /*dialog_id*/, std::shared_ptr<TranscoderInfo>, vs::str_less>;
	vs::Synchronized<transcoders_map_t> m_transcoders;

	struct DelayedInvite
	{
		std::string dialog_id;
		std::string from;
		std::string to;
		bool is_group_conf;
		bool use_new_dialog_id;
		std::string dn_from_utf8;
	} m_DelayedInvite;

	// *) True: this connection was created to support terminal, that was registred
	// as TrueConf user, so the first connection in m_connections array is
	// a registration connection. That means, this TranscportConnection should
	// free transcoders and logout them only after registration connection die.
	// *) False: this connection was created for simple h323 or sip call.
	// That means, TransportConnection should close transcoders, logout them and
	// destroy itself after any connection die.
	bool m_for_registred_user;

	std::chrono::steady_clock::time_point m_inactivity_start_time;
	boost::signals2::scoped_connection m_DialogFinishedConn;

	struct LogPrefix;
};
