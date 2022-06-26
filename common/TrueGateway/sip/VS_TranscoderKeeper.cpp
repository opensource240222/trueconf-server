#include "VS_TranscoderKeeper.h"

#include <boost/make_shared.hpp>

#include "../clientcontrols/VS_ClientControlAllocatorInterface.h"
#include "../clientcontrols/VS_ClientControlInterface.h"
#include "VS_SIPCallResolver.h"
#include "VS_VisiSIPProxy.h"
#include <boost/algorithm/string.hpp>

VS_TranscoderKeeper::Transcoder_Descr::~Transcoder_Descr() {
	onZombieConn.disconnect();
	HangupFromVisiConn.disconnect();
	FastUpdatePicConn.disconnect();
	LoggedOutAsUserConn.disconnect();
	SetMediaChannelsConn.disconnect();
	InviteReplayConn.disconnect();
	InviteConn.disconnect();
	ChatConn.disconnect();
	FileConn.disconnect();
	CommandConn.disconnect();
	if (trans) {
		trans->ReleaseCallbacks();
		if (auto disp = VS_ClientControlAllocatorInterface::Instance())
			disp->ReleaseTranscoder(trans);
	}
}

void VS_TranscoderKeeper::SetVisiToSip(std::shared_ptr<VS_VisiSIPProxy> visi2sip) {
	m_visi2sip = visi2sip;
}

void VS_TranscoderKeeper::SetSipCallResolver(const std::shared_ptr<VS_SIPCallResolver> &sipCallResolver) {
	m_sip_call_resolver = sipCallResolver;
}

void VS_TranscoderKeeper::AddTranscoder(string_view dialogId, boost::shared_ptr<Transcoder_Descr> tr) {
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	auto it = m_transcoders_tbl.find(dialogId);
	if(it == m_transcoders_tbl.cend())
	{
		m_transcoders_tbl.emplace(std::string(dialogId), std::move(tr));
	}
	else
	{
		it->second = std::move(tr);
	}
}

void VS_TranscoderKeeper::FreeTranscoder(string_view dialogId) {
	boost::shared_ptr<Transcoder_Descr> t;
	std::size_t use_count = 0;

	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		const auto it = m_transcoders_tbl.find(dialogId);
		if (it == m_transcoders_tbl.cend())
		{
			return;
		}
		t = std::move(it->second);
		m_transcoders_tbl.erase(it);

		for (auto &tr : m_transcoders_tbl) {
			if (tr.second == t) {
				use_count++; // t.use_count() may be different
			}
		}
	}

	if (!t || !t->trans) return;

	t->trans->ClearDialogId(dialogId);

	if (!use_count) {
		t->trans->Hangup();
		t->trans->LogoutUser(std::function< void(void) >());
	}
}

boost::shared_ptr<VS_TranscoderKeeper::Transcoder_Descr> VS_TranscoderKeeper::NewTranscoder(string_view dialogId) {
	
	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		assert(!m_visi2sip.expired());

		const auto it = m_transcoders_tbl.find(dialogId);
		if (it != m_transcoders_tbl.cend()) {
			return (*it).second;
		}
	}

	auto disp = VS_ClientControlAllocatorInterface::Instance();
	if (!disp) {
		return nullptr;
	}
	auto trans_descr = boost::make_shared<Transcoder_Descr>();
	do {
		trans_descr->trans = disp->GetTranscoder();

		if (!trans_descr->trans) {
			break;
		}

		trans_descr->onZombieConn = trans_descr->trans->ConnectOnZombie(boost::bind(&VS_VisiSIPProxy::OnZombieTranscoder, m_visi2sip.lock(), _1));
		trans_descr->HangupFromVisiConn = trans_descr->trans->ConnectHangupFromVisi(boost::bind(&VS_VisiSIPProxy::HangupFromVisi, m_visi2sip.lock(), _1, _2));
		trans_descr->FastUpdatePicConn = trans_descr->trans->ConnectFastUpdatePicture(boost::bind(&VS_VisiSIPProxy::FastUpdatePictureFromVisi, m_visi2sip.lock(), _1));
		trans_descr->LoggedOutAsUserConn = trans_descr->trans->ConnectLoggedOutAsUser(boost::bind(&VS_VisiSIPProxy::LoggedOutAsUser, m_visi2sip.lock(), _1));
		trans_descr->SetMediaChannelsConn = trans_descr->trans->ConnectSetMediaChannels(boost::bind(&VS_VisiSIPProxy::SetMediaChannelsFromVisi, m_visi2sip.lock(), _1, _2, _3));
		trans_descr->InviteReplayConn = trans_descr->trans->ConnectInviteReply(boost::bind(&VS_VisiSIPProxy::InviteReplyFromVisi, m_visi2sip.lock(), _1, _2, _3, _4, _5));
		trans_descr->InviteConn = trans_descr->trans->ConnectInvite(boost::bind(&VS_VisiSIPProxy::InviteFromVisi, m_visi2sip.lock(), _1, _2, _3, _4, _5, _6, _7));
		trans_descr->ChatConn = trans_descr->trans->ConnectChat(boost::bind(&VS_VisiSIPProxy::ChatFromVisi, m_visi2sip.lock(), _1, _2, _3, _4, _5));
		trans_descr->FileConn = trans_descr->trans->ConnectFile(boost::bind(&VS_VisiSIPProxy::FileFromVisi, m_visi2sip.lock(), _1, _2, _3, _4, _5, _6));
		trans_descr->CommandConn = trans_descr->trans->ConnectCommand(boost::bind(&VS_VisiSIPProxy::CommandFromVisi, m_visi2sip.lock(), _1, _2, _3));
		trans_descr->trans->ConnectUpdateStatus(boost::bind(&VS_SIPCallResolver::UpdateStatus, m_sip_call_resolver.lock(), _1, _2));
		trans_descr->trans->ConnectReqInvite(boost::bind(&VS_SIPCallResolver::ReqInvite, m_sip_call_resolver.lock(), _1, _2));
		trans_descr->trans->ConnectTakeTribuneReply(boost::bind(&VS_SIPCallResolver::TakeTribuneReply, m_sip_call_resolver.lock(), _1, _2));
		trans_descr->trans->ConnectLeaveTribuneReply(boost::bind(&VS_SIPCallResolver::LeaveTribuneReply, m_sip_call_resolver.lock(), _1, _2));
		trans_descr->trans->SetConnectMeToTransceiver(m_fireConnectClientToTransceiver);

		if (!trans_descr->trans->IsReady()) {
			trans_descr->onZombieConn.disconnect();
			trans_descr->HangupFromVisiConn.disconnect();
			trans_descr->FastUpdatePicConn.disconnect();
			trans_descr->LoggedOutAsUserConn.disconnect();
			trans_descr->SetMediaChannelsConn.disconnect();
			trans_descr->InviteReplayConn.disconnect();
			trans_descr->InviteConn.disconnect();
			trans_descr->ChatConn.disconnect();
			trans_descr->FileConn.disconnect();
			trans_descr->CommandConn.disconnect();

			disp->ReleaseTranscoder(trans_descr->trans);
			trans_descr->trans.reset();
		}
	} while (!trans_descr->trans); /// попытаться получить транскодер несколько раз

	if (trans_descr->trans)
	{
		trans_descr->trans->SetDialogId(dialogId);
		{
			std::lock_guard<decltype(m_mutex)> lock(m_mutex);
			m_transcoders_tbl.emplace(std::string(dialogId), trans_descr);
		}
		return trans_descr;
	}
	return nullptr;
}


boost::shared_ptr<VS_TranscoderKeeper::Transcoder_Descr> VS_TranscoderKeeper::GetTranscoder(string_view dialogId) {
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	const auto it = m_transcoders_tbl.find(dialogId);
	if (it != m_transcoders_tbl.cend())
	{
		return (*it).second;
	}
	return nullptr;
}

boost::shared_ptr<VS_TranscoderKeeper::Transcoder_Descr> VS_TranscoderKeeper::GetLoggedIn(string_view login, bool exactMatch) {
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	for (auto &t : m_transcoders_tbl)
	{
		if (t.second->trans->IsLoggedIn(login, exactMatch))
		{
			return t.second;
		}
	}
	return nullptr;
}

void VS_TranscoderKeeper::SetUserDialogID(string_view login, string_view dialogId) {
	boost::shared_ptr<Transcoder_Descr> t;

	{
		std::lock_guard<decltype(m_mutex)> lock(m_mutex);
		const auto it = m_transcoders_tbl.find(login);
		if (it == m_transcoders_tbl.cend())
		{
			return;
		}
		t = (*it).second;
		m_transcoders_tbl.emplace(std::string(dialogId), t);
	}

	if (t && t->trans)
	{
		t->trans->SetDialogId(dialogId);
	}
}

void VS_TranscoderKeeper::GetDialogList(string_view name, std::vector<std::string> &dlgList)
{
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	for (auto &t : m_transcoders_tbl)
	{
		if (t.second->trans->GetTranscoderID() == name)
		{
			dlgList.push_back(t.first);
		}
	}
}

bool VS_TranscoderKeeper::IsRegisteredTransID(string_view transId){
	std::lock_guard<decltype(m_mutex)> lock(m_mutex);
	for (const auto &t : m_transcoders_tbl)
	{
		string_view id = t.second->trans->GetTranscoderID();
		if(id.length() == transId.length() && boost::iequals(id, transId))
		{
			return true;
		}
	}
	return false;
}
