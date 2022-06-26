#include "VS_AbstractJsonConnection.h"
#include <string>

#include "VS_FakeEndpoint.h"
#include "VS_FakeClientManager.h"
#include "VS_JsonClient.h"

#include "std/cpplib/json/writer.h"
#include "std/cpplib/json/reader.h"
#include "std/cpplib/MakeShared.h"
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "std/debuglog/VS_Debug.h"


#define DEBUG_CURRENT_MODULE VS_DM_FAKE_CLIENT

VS_AbstractJsonConnection::VS_AbstractJsonConnection()
{

}

#if defined(_WIN32) // Not ported
void VS_AbstractJsonConnection::PostConstruct(
						  std::string remote_ip,
						  const VS_IPPortAddress &bindAddr,
						  const VS_IPPortAddress &peerAddr,
						  std::weak_ptr<void> _this
						  )
{
	m_remote_ip = std::move(remote_ip);
	m_bind_addr = bindAddr;
	m_peer_addr = peerAddr;
	m_this = _this;
}
#endif

void VS_AbstractJsonConnection::PostConstruct(const net::address& remote_ip, std::weak_ptr<void> _this)
{
	boost::system::error_code ec;
	m_remote_ip = remote_ip.to_string(ec);
	if (ec) dstream4 << "VS_AbstractJsonConnection::PostConstruct fail to init remote ip!\n";
	m_this = _this;
}

VS_AbstractJsonConnection::~VS_AbstractJsonConnection(void)
{
}

#define GET_STRING_FOROM_JSON(name,str_name,container, default_value) std::string name;	\
	do { json::Object::const_iterator it = container.Find( #str_name );		\
	if (it != container.End() ) name = (const json::String) it->element;	\
	else	name = default_value; \
	} while( 0 )

#define RETURN_RESPONSE(x) do { SendResponse((x)); return ; } while ( 0 )

void VS_AbstractJsonConnection::ProcessRequest(const std::string& message)
{
	json::Object obj;
	std::stringstream data(message);

	try
	{
		json::Reader::Read(obj, data);
		GET_STRING_FOROM_JSON(method, method, obj, "");
		GET_STRING_FOROM_JSON(CID, CID, obj, "");

		if (CID.empty() && method == "ping")	RETURN_RESPONSE((std::string("{\"method\": \"pong\", \"version\":\"") + VS_JsonClient::Version() + "\"}").c_str());

		if (CID.empty() && method == "loginUser")
		{
			std::unique_ptr<VS_FakeEndpoint> ep;
#if defined(_WIN32)
			if(!m_bind_addr.isZero() && !m_peer_addr.isZero())
				ep = VS_FakeEndpointFactory::Instance().Create(m_bind_addr, m_peer_addr);	// for old server with VS_TransportRouter_SetConnection
			else
				ep = VS_FakeEndpointFactory::Instance().Create();							// for new server with transport::Router
#else
			ep = VS_FakeEndpointFactory::Instance().Create();								// on Linux we have only new version
#endif
			if (!ep)
				RETURN_RESPONSE("{\"error\": \"internalServerError\"}");
			auto jc = vs::MakeShared<VS_JsonClient>(std::move(ep));
			VS_FakeClientManager::Instance().RegisterClient(jc);
			CID = jc->CID();
			m_CID = CID;
			m_json_client = jc;
			if (CID.empty()) RETURN_RESPONSE("{\"error\": \"internalServerError\"}");
			obj["CID"] = (json::String) CID;
		}

		if (CID.empty())		RETURN_RESPONSE("{\"error\": \"nothing to do\"}");

		bool NewWS(false);
		if (m_CID.empty() && !CID.empty())
		{
			// new ws connection, need to set callback at webrtc_peer_srv
			NewWS = true;
			m_CID = CID;
		}

		 assert(m_json_client == std::dynamic_pointer_cast<VS_JsonClient>(VS_FakeClientManager::Instance().GetClient(CID)));

		if (NewWS && !!m_json_client)
			m_json_client->RegisterAtWebrtcSRV();

		if (!m_json_client.get())
			RETURN_RESPONSE( (std::string("{ \"error\": \"session not found\", \"CID\":\"") +  CID + "\"}").c_str());

		m_json_client->SetResponceCallBack(m_this.lock(), MakeJSONClientCallBack());

		if (method != "longPolling" && !m_json_client->Request( obj, m_remote_ip ) )
			RETURN_RESPONSE("{\"error\": \"unknown method or not all parameters are given\"}");
	}
	catch (json::Exception &e)
	{
		RETURN_RESPONSE( (std::string("{\"error\": \"Exception :") + e.what() + "\"}" ).c_str() );
	}
}


bool VS_AbstractJsonConnection::ProcessResponse(const json::Object &resp, std::shared_ptr<VS_JsonClient> c)
{
	std::stringstream ss;
	json::Writer::Write(resp, ss);
	if (!ss.str().length())	return false;

	bool sendResult = SendResponse( ss.str().c_str() );

	if (sendResult)
	{
		c->SetResponceCallBack(m_this.lock(), MakeJSONClientCallBack());
	}

	return sendResult;
}

std::function<bool(const json::Object&resp)> VS_AbstractJsonConnection::MakeJSONClientCallBack(void)
{
	std::weak_ptr<VS_JsonClient> jc_wptr = m_json_client;

	auto cb = [this, jc_wptr](const json::Object &resp) -> bool {
		auto jc = jc_wptr.lock();
		if (jc == nullptr)
		{
			return false;
		}
		assert(jc == m_json_client);
		return ProcessResponse(resp, jc);
	};

	return cb;
}

void VS_AbstractJsonConnection::onError(unsigned err)
{
	if (!m_CID.empty() && m_json_client != nullptr)
	{
		assert(std::dynamic_pointer_cast<VS_JsonClient>(VS_FakeClientManager::Instance().GetClient(m_CID)) == m_json_client);
		m_json_client->onWSError(err);
	}
}
