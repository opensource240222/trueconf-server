#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>

#include "std-generic/compat/memory.h"
#include "tools/Server/CommonTypes.h"

class VS_CallConfigStorage;
class VS_MessageData;
class VS_SIPParser;
namespace sip { class TransportLayer; }
class VS_TranscoderKeeper;
class VS_H281Frame;

struct VS_MediaChannelInfo;

class VS_VisiSIPProxy : public vs::enable_shared_from_this<VS_VisiSIPProxy>
{
public:
	struct InitInfo final
	{
		std::shared_ptr<VS_SIPParser> parser;
		std::shared_ptr<VS_CallConfigStorage> peerConfig;
		std::weak_ptr<VS_TranscoderKeeper> trKeeper;
		std::weak_ptr<sip::TransportLayer> sipTransport;
	};

	void HangupFromVisiStr(const string_view id)
	{
		HangupFromVisi(id, {});
	}

	void HangupFromVisi(string_view dialogId, string_view method);
	void FastUpdatePictureFromVisi(string_view dialogId);
	void LoggedOutAsUser(string_view dialogId);
	bool SetMediaChannelsFromVisi(string_view dialogId, const std::vector<VS_MediaChannelInfo> &channels, std::int32_t bandwRcv);
	void InviteReplyFromVisi(string_view dialogId, VS_CallConfirmCode confirmCode, bool isGroupConf, string_view confName, string_view to_displayName);
	void InviteFromVisi(string_view dialogId, string_view from, string_view to, bool isGroupConf, bool isPublicConf, bool useNewDialogID, string_view dnFromUtf8);
	void ChatFromVisi(string_view dialogId, string_view from, string_view to, string_view dn, const char *mess);
	void FileFromVisi(string_view dialogId, string_view from, string_view to, string_view dn, const char* mess, const FileTransferInfo &i);
	void CommandFromVisi(string_view dialogid, string_view from, string_view command);
	void OnZombieTranscoder(string_view name);
protected:
	VS_VisiSIPProxy(boost::asio::io_service::strand &strand, InitInfo &&init);
private:
	boost::asio::io_service::strand &m_strand;
	std::shared_ptr<VS_SIPParser> m_parser;
	std::shared_ptr<VS_CallConfigStorage> m_peer_config;
	std::weak_ptr<VS_TranscoderKeeper> m_tr_keeper;
	std::weak_ptr<sip::TransportLayer> m_sip_transport;
};