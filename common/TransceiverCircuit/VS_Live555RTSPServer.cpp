#include "VS_Live555RTSPServer.h"
#include "VS_RTSPBroadcastMediaPeer.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/VS_TransceiverInfo.h"

#include <ServerMediaSessionProxy.hh>
#include <GroupsockHelper.hh>

#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>

#include <cstring>

class VS_Live555RTSPServer::ClientSession : public RTSPServer::RTSPClientSession
{
public:
	ClientSession(VS_Live555RTSPServer& ourServer, u_int32_t sessionId);

private:
    void handleCmd_SETUP(RTSPClientConnection* ourClientConnection, char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr) override;
};

VS_Live555RTSPServer::ClientSession::ClientSession(VS_Live555RTSPServer& ourServer, u_int32_t sessionId)
	: RTSPClientSession(ourServer, sessionId)
{
}

void VS_Live555RTSPServer::ClientSession::handleCmd_SETUP(RTSPClientConnection* ourClientConnection, char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr)
{
	VS_SCOPE_EXIT { RTSPClientSession::handleCmd_SETUP(ourClientConnection, urlPreSuffix, urlSuffix, fullRequestStr); };

	fUseClientDestination = False; // Reset to False to ensure that _this_ SETUP request has a proper secret.

	const string_view request(fullRequestStr);
	const string_view header_name(ts::RTSP_SECRET_HEADER);

	// Try to find our secret header
	auto header_pos = request.npos;
	for (string_view::size_type pos = 0; true ; /**/)
	{
		header_pos = request.find(header_name, pos);
		if (header_pos == request.npos)
			return;

		if (header_pos < 2 || request[header_pos-2] != '\r' || request[header_pos-1] != '\n')
		{
			// Headers must start at a new line and can't appear on the first line.
			pos = header_pos + header_name.size();
			continue;
		}
		if (request.substr(header_pos + header_name.size(), 1) != ":")
		{
			// Header name must be immediately followed by ":".
			pos = header_pos + header_name.size();
			continue;
		}

		break;
	}
	assert(header_pos != request.npos);

	const auto header_name_end_pos = header_pos + header_name.size() + 1/* ':' */;
	const auto header_end_pos = request.find("\r\n", header_name_end_pos);
	if (header_end_pos == request.npos)
		return; // There always should be an endline at the end.

	const auto value_pos = request.find_first_not_of(' ', header_name_end_pos);
	assert(value_pos <= header_end_pos); // '\r' is not ' '
	const auto value = request.substr(value_pos, header_end_pos - value_pos);

	if (value == static_cast<VS_Live555RTSPServer&>(fOurServer).m_secret)
		fUseClientDestination = True;
}

VS_Live555RTSPServer* VS_Live555RTSPServer::createNew(UsageEnvironment& env, Port& ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds, string_view secret)
{
	int ourSocket = setUpOurSocket(env, ourPort);
	if (ourSocket == -1) return NULL;
	return new VS_Live555RTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds, secret);
}

VS_Live555RTSPServer::VS_Live555RTSPServer(UsageEnvironment& env, int ourSocket, Port& ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds, string_view secret)
	: RTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds)
	, m_secret(secret)
{
}

RTSPServer::RTSPClientConnection* VS_Live555RTSPServer::createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr)
{
	increaseSendBufferTo(envir(), clientSocket, 1024*1024);
	return RTSPServer::createNewClientConnection(clientSocket, clientAddr);
}

static const boost::regex url_rtsp_re("(c/[^/]+)(?:/(.*))?", boost::regex::optimize);

ServerMediaSession* VS_Live555RTSPServer::lookupServerMediaSession(char const* streamName, Boolean isFirstLookupInSession)
{
	ServerMediaSession* sms;
	if (NULL != (sms = RTSPServer::lookupServerMediaSession(streamName)))
		return sms;

	if (!isFirstLookupInSession)
		return NULL;

	boost::cmatch url_match;
	if (!boost::regex_match(streamName, url_match, url_rtsp_re))
		return NULL;

	std::string name = url_match.str(1);
	std::string parameters = url_match.str(2);

	if (parameters.empty())
		return RTSPServer::lookupServerMediaSession(name.c_str());

	name += "/all";
	if (NULL == (sms = RTSPServer::lookupServerMediaSession(name.c_str())))
		return NULL;

	ServerMediaSessionProxy* sms_proxy = ServerMediaSessionProxy::createNew(envir(), sms, streamName);
	ServerMediaSubsessionIterator smss_it(*sms);
	ServerMediaSubsession* smss;
	while (NULL != (smss = smss_it.next()))
	{
		assert(dynamic_cast<VS_Live555ServerMediaSubsession*>(smss));
		auto our_smss = static_cast<VS_Live555ServerMediaSubsession*>(smss);

		bool matched = false;
		typedef boost::algorithm::find_iterator<std::string::const_iterator> find_iterator;
		for (find_iterator param_it(parameters, boost::algorithm::token_finder([](char x) { return x != ','; }, boost::algorithm::token_compress_on)); !matched && !param_it.eof(); ++param_it)
		{
			string_view param(&*param_it->begin(), param_it->end() - param_it->begin());

			VS_RTSPSourceType src_type;
			if      (boost::istarts_with(param, "mix:"))
			{
				src_type = VS_RTSPSourceType::Mix;
				param.remove_prefix(4);
			}
			else if (boost::istarts_with(param, "speaker:"))
			{
				src_type = VS_RTSPSourceType::Speaker;
				param.remove_prefix(8);
			}
			else
				src_type = VS_RTSPSourceType::Default;

			matched = static_cast<bool>(our_smss->GetSourceType() & src_type) && boost::istarts_with(our_smss->GetCodec(), param);
		}
		if (matched)
			sms_proxy->addSubsession(smss);
	}
	RTSPServer::addServerMediaSession(sms_proxy);
	return sms_proxy;
}

void VS_Live555RTSPServer::removeServerMediaSession(ServerMediaSession* serverMediaSession)
{
	if (serverMediaSession == NULL)
		return;

	string_view name(serverMediaSession->streamName());
	if (boost::ends_with(name, "/all"))
	{
		name.remove_suffix(3);
		ServerMediaSessionIterator sms_it(*this);
		ServerMediaSession* sms;
		while (NULL != (sms = sms_it.next()))
			if (sms != serverMediaSession && boost::algorithm::starts_with(string_view(sms->streamName()), name))
				RTSPServer::removeServerMediaSession(sms);
	}
	RTSPServer::removeServerMediaSession(serverMediaSession);
}

void VS_Live555RTSPServer::closeAllClientSessionsForServerMediaSession(ServerMediaSession* serverMediaSession)
{
	if (serverMediaSession == NULL)
		return;

	string_view name(serverMediaSession->streamName());
	if (boost::ends_with(name, "/all"))
	{
		name.remove_suffix(3);
		ServerMediaSessionIterator sms_it(*this);
		ServerMediaSession* sms;
		while (NULL != (sms = sms_it.next()))
			if (sms != serverMediaSession && boost::algorithm::starts_with(string_view(sms->streamName()), name))
				RTSPServer::closeAllClientSessionsForServerMediaSession(sms);
	}
	RTSPServer::closeAllClientSessionsForServerMediaSession(serverMediaSession);
}

RTSPServer::RTSPClientSession* VS_Live555RTSPServer::createNewClientSession(u_int32_t sessionId)
{
	return new ClientSession(*this, sessionId);
}
