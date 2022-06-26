#include "VS_VisiSIPProxy.h"

#include "../clientcontrols/VS_ClientControlInterface.h"
#include "TrueGateway/sip/VS_SIPParser.h"
#include "TrueGateway/sip/SIPTransportLayer.h"
#include "VS_TranscoderKeeper.h"
#include "../CallConfig/VS_CallConfig.h"
#include "../CallConfig/VS_CallConfigStorage.h"
#include "std/cpplib/VS_UserData.h"
#include "../../FakeClient/VS_ConferenceInfo.h"
#include "std/cpplib/event.h"
#include "std-generic/cpplib/deleters.h"


VS_VisiSIPProxy::VS_VisiSIPProxy(boost::asio::io_service::strand &strand, InitInfo &&init)
	: m_strand(strand)
	, m_parser(std::move(init.parser))
	, m_peer_config(std::move(init.peerConfig))
	, m_tr_keeper(std::move(init.trKeeper))
	, m_sip_transport(std::move(init.sipTransport))
{
	assert(m_parser);
	assert(m_peer_config);
}

void VS_VisiSIPProxy::OnZombieTranscoder(string_view name) {
	auto &&tr_keeper = m_tr_keeper.lock();
	if (!tr_keeper) return;

	std::vector<std::string> dlg_lst;
	tr_keeper->GetDialogList(name, dlg_lst);
	std::for_each(dlg_lst.cbegin(), dlg_lst.cend(), boost::bind(&VS_VisiSIPProxy::HangupFromVisiStr, this, _1));
}

void VS_VisiSIPProxy::HangupFromVisi(string_view dialogId, string_view method)
{
	const bool reject = (method == "reject");
	m_strand.dispatch([parser = m_parser,
		dialog_id = std::string(dialogId), reject]()
	{
		if (reject)
		{
			parser->NotificateAboutReject(dialog_id);
		}
		parser->Hangup(dialog_id);
	});
}

void VS_VisiSIPProxy::FastUpdatePictureFromVisi(string_view dialogId) {

	m_strand.dispatch([parser = m_parser,
		dialog_id = std::string(dialogId)]()
	{
		parser->FastUpdatePicture(dialog_id);
	});
}

void VS_VisiSIPProxy::LoggedOutAsUser(string_view dialogId) {

	m_strand.dispatch([parser = m_parser, sip_transport = m_sip_transport.lock(),
		dialog_id = std::string(dialogId)]()
	{
		parser->LoggedOutAsUser(dialog_id);
		if (sip_transport)
		{
			sip_transport->Write(dialog_id);
		}
	});
}

bool VS_VisiSIPProxy::SetMediaChannelsFromVisi(string_view dialogId, const std::vector<VS_MediaChannelInfo> &channels, std::int32_t bandwRcv) {

	vs::event ev{ false };
	bool res;
	m_strand.dispatch([&]()
	{
		res = m_parser->SetMediaChannels(dialogId, channels, {}, bandwRcv);
		ev.set();
	});
	ev.wait();
	return res;
}

void VS_VisiSIPProxy::InviteReplyFromVisi(string_view dialogId, VS_CallConfirmCode confirmCode, bool isGroupConf, string_view confName, string_view to_displayName)
{
	m_strand.dispatch([self = shared_from_this(), this,
	dialog_id = std::string(dialogId),
	confirmCode, isGroupConf,
	conf_name = std::string(confName),
	to_display_name = std::string(to_displayName)]()
	{
		m_parser->InviteReplay(dialog_id, confirmCode, isGroupConf, conf_name, to_display_name);
		auto &&sip_transport = m_sip_transport.lock();
		if (sip_transport)
		{
			sip_transport->Write(dialog_id);
		}
	});
}

void VS_VisiSIPProxy::InviteFromVisi(string_view dialogId, string_view from, string_view to, bool isGroupConf, bool isPublicConf, bool useNewDialogId, string_view dnFromUtf8)
{
	auto tr_keeper = m_tr_keeper.lock();
	if (!tr_keeper)
		return;

	m_strand.dispatch([=, self = shared_from_this(), tr_keeper = std::move(tr_keeper),
		dialogId = std::string(dialogId), from = std::string(from), to = std::string(to), dn_from_utf8 = std::string(dnFromUtf8)]() mutable
	{
		if (useNewDialogId) {
			VS_UserData user{ from.c_str() };
			VS_CallConfig config;
			const bool res = m_peer_config->Resolve(config, to, &user);
			assert(res);

			assert(!dialogId.empty());
			auto newDialog = m_parser->NewDialogID(to, {}, config);
			tr_keeper->SetUserDialogID(dialogId, newDialog);
			dialogId = newDialog;
		}

		if (!m_parser->InviteMethod(dialogId, from, to, VS_ConferenceInfo(isGroupConf, isPublicConf), dn_from_utf8)) {
			auto t = tr_keeper->GetTranscoder(dialogId);
			if (t) {
				t->trans->InviteReply(e_call_none);
			}
		}

		auto sip_transport = m_sip_transport.lock();
		if (sip_transport) {
			sip_transport->Write(dialogId);
		}
	});
}

void VS_VisiSIPProxy::ChatFromVisi(string_view dialogId, string_view from, string_view /*to*/, string_view dn, const char* mess)
{
	m_strand.dispatch([self = shared_from_this(), this,
		dialog_id = std::string(dialogId), from = std::string(from), /*to = std::string(to),*/ dn = std::string(dn), mess = strdup(mess) /*TODO:test nullptr mess*/]()
	{
		std::unique_ptr<char, free_deleter> mess_free{ mess };
		// it is chat in call
		auto &&existing_ctx = m_parser->GetParserContext(dialog_id, false);
		assert(existing_ctx != nullptr);
		if (!existing_ctx) return;

		// use existing remote alias
		auto &&new_dialog = m_parser->NewDialogID(existing_ctx->GetAliasRemote(), {}, existing_ctx->GetConfig(), from);

		auto new_ctx = m_parser->GetParserContext(new_dialog, false);
		if (!new_ctx) return;
		new_ctx->SetGroupConf(existing_ctx->IsGroupConf());
		m_parser->Chat(new_dialog, from, existing_ctx->GetAliasRemote(), dn, mess);

		auto &&sip_transport = m_sip_transport.lock();
		if (sip_transport)
		{
			sip_transport->Write(dialog_id);
		}
	});
}

void VS_VisiSIPProxy::FileFromVisi(string_view dialogId, string_view from, string_view to, string_view dn, const char *mess, const FileTransferInfo &i)
{
	return ChatFromVisi(dialogId, from, to, dn, mess);	// for now it's just chat with link for file downloading
}

void VS_VisiSIPProxy::CommandFromVisi(string_view dialogId, string_view from, string_view command)
{
	if(m_strand.running_in_this_thread())
	{
		m_parser->Command(dialogId, from, command);
		auto sip_transport = m_sip_transport.lock();
		if (sip_transport)
		{
			sip_transport->Write(dialogId);
		}
	}
	else
	{
		m_strand.post([self = shared_from_this(),
			dialog_id = std::string(dialogId), from = std::string(from), command = std::string(command)]()
		{
			self->CommandFromVisi(dialog_id, from, command);
		});
	}
}