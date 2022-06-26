#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

#include <set>
#include <vector>

#include "mdnslib/addons/SRV.h"
#include "mdnslib/addons/TXT.h"
#include "mdnslib/Former.h"
#include "mdnslib/Parser.h"

#include "std-generic/compat/memory.h"

namespace mdns
{

class DataFetcherBase;
class ConnectionHandler;

// This class implements MDNS responder with logic *partially* corresponding to RFC 6762.
// Uses async write/read operations with the help of boost::asio::io_service::strand
class Responder: public vs::enable_shared_from_this<Responder>
{
public:

	void start();
	void stop();

protected:
	Responder(boost::asio::io_service& service, std::shared_ptr<DataFetcherBase> base);

private:
	enum ResponderState
	{
//		Responder should do nothing while in this state
		RS_NONE,
//		Preprobe state means waiting a random delay before sending the first probe query
//		(see RFC 6762:8)
		RS_PREPROBE,
//		Probing is checking if someone has already claimed ownership of resource records
//		that responder wants to have unique possession of
		RS_PROBE,
//		Reprobing occurs when responder receives a competing query while probing and also loses
//		the "competition". In this state, it waits for one second, then tries to probe again.
//		If fails again, it has to reconfigure name and start probing once again
		RS_REPROBE,
//		After probing, responder must send two or more messages containing all it's resource records
		RS_ANNOUNCE,
//		In this state responder is correctly responding to DNS-SD queries
		RS_STABLE
	};


	void sendMsg(const boost::asio::mutable_buffers_1& buffer, int index = -1);
	void recvMsg(int index = -1);

	bool setupSockets();
//	Fake name record is needed for listeners (since there is no domain name that resolves to them)
//	and connect TCP's that are specified as IP addresses.
	void formFakeNameRecord(uint16_t port, unsigned int priority);
	void addAddressRecord(std::vector<std::pair<mdns::RRecord, uint16_t>>& result,
		const boost::asio::ip::address& address, uint16_t port);
	void formConnectTcpRecords();
	void formListenersRecords();
//	There is always only one TXT record for the server
	void formTxtRecord();
	void formUniqueRecords();
	void formSharedRecords();
	bool configureNames();
//	Needed when responder loses the competition twice and has to change it's name
	void reconfigure();
	void reconfigureName();
//	Since forming queries/responses is kind of expensive (thanks to name compression mostly)
//	it's better to form every possible response at startup
	bool formProbeQuery();
	bool formAnnounceResponse();
	void formStableServicePtrResponse();
	void formStableInstanceSrvResponse();
	void formStableInstanceAResponse();
	void formStableInstanceAaaaResponse();
	void formStableInstanceTxtResponse();
	void formStableInstanceAnyResponse();
	bool formStableResponses();
	void preProbe();
	void probe();
	void announce();

	void timerHandler(const boost::system::error_code& error);
	void sendHandler(const boost::system::error_code& error, size_t count);
	void recvHandler(const boost::system::error_code& error, const void* data, size_t count, uint16_t socket);
	bool recvHandlerProbe(const void* data, size_t count);
	bool recvHandlerAnnounce(const void* data, size_t count);
	bool recvHandlerStable(const void* data, size_t count, uint16_t socket);


	std::shared_ptr<mdns::ConnectionHandler> connHandler_;

	boost::asio::steady_timer timer_;

	boost::asio::io_service::strand strand_;

	std::vector<char> instanceName_;
	std::vector<char> serverFakeName_;
	std::vector<char> dnsSdQueryName_;
	std::vector<char> serviceQueryName_;
	unsigned int instanceUniqueNumber_;
	std::vector<char> instanceQueryName_;
	std::vector<char> targetHostName_;

	size_t count_;

	mdns::Parser parser_;
	mdns::Former probeQuery_;
	mdns::Former announceResponse_;
	mdns::Former stableDnsSdResponse_;
	mdns::Former stableServicePtrResponse_;
	mdns::Former stableInstanceSrvResponse_;
	mdns::Former stableInstanceAResponse_;
	mdns::Former stableInstanceAaaaResponse_;
	mdns::Former stableInstanceTxtResponse_;
	mdns::Former stableInstanceAnyResponse_;

	std::vector<std::pair<mdns::RRecord, uint16_t>> connectTcpRecords_;
	std::vector<std::pair<mdns::RRecord, uint16_t>> listenerRecords_;
	std::vector<std::pair<mdns::RRecord, uint16_t>> fakeNameRecords_;
	mdns::RRecord recordTxt_;
	std::vector<mdns::RRecord> uniqueRecords_;
	std::vector<mdns::RRecord> sharedRecords_;
	std::set<mdns::RRecord> addressRecords_;
	unsigned int messageCounter_;
	ResponderState state_;

	std::shared_ptr<DataFetcherBase> data_;
};

}
