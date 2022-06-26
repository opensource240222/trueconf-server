#pragma once

#include <boost/signals2.hpp>
#include <memory>

#include "TrueGateway/interfaces/VS_ConferenceProtocolInterface.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/compat/functional.h"
#include "std-generic/compat/map.h"

class VS_SIPCallResolver;
class VS_VisiSIPProxy;

class VS_TranscoderKeeper
{
public:
	struct Transcoder_Descr
	{
		boost::shared_ptr<VS_ClientControlInterface> trans;

		boost::signals2::connection onZombieConn;
		boost::signals2::connection HangupFromVisiConn;
		boost::signals2::connection FastUpdatePicConn;
		boost::signals2::connection LoggedOutAsUserConn;
		boost::signals2::connection SetMediaChannelsConn;
		boost::signals2::connection	InviteReplayConn;
		boost::signals2::connection	InviteConn;
		boost::signals2::connection	ChatConn;
		boost::signals2::connection FileConn;
		boost::signals2::connection	CommandConn;

		~Transcoder_Descr();
	};

	virtual ~VS_TranscoderKeeper() {}
	void SetVisiToSip(std::shared_ptr<VS_VisiSIPProxy> visi2sip);
	void SetSipCallResolver(const std::shared_ptr<VS_SIPCallResolver> &sipCallResolver);
	void AddTranscoder(string_view dialogId, boost::shared_ptr<Transcoder_Descr> tr);
	void FreeTranscoder(string_view dialogId);
	virtual boost::shared_ptr<Transcoder_Descr> NewTranscoder(string_view dialogId);
	boost::shared_ptr<Transcoder_Descr> GetTranscoder(string_view dialogId);
	boost::shared_ptr<Transcoder_Descr> GetLoggedIn(string_view login, bool exactMatch = true);
	void SetUserDialogID(string_view login, string_view dialogId);
	void GetDialogList(string_view name, std::vector<std::string> &dlgList);
	bool IsRegisteredTransID(string_view transId);
	template<class Function>
	void SetConnectClientToTransceiver(Function&& f) {
		m_fireConnectClientToTransceiver = std::forward<Function>(f);
	}
private:
	typedef std::recursive_mutex mutex_t;
private:
	mutex_t m_mutex;
	vs::map<std::string, boost::shared_ptr<Transcoder_Descr>, vs::str_less> m_transcoders_tbl;
	std::weak_ptr<VS_VisiSIPProxy> m_visi2sip;
	std::weak_ptr<VS_SIPCallResolver> m_sip_call_resolver;
	std::function<bool(const std::string &/*dialog*/, const std::string &/*confID*/)> m_fireConnectClientToTransceiver;
};