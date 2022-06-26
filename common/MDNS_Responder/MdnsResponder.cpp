#include "MdnsResponder.h"

#include <boost/asio/error.hpp>
#include <boost/asio/ip/multicast.hpp>

#include <chrono>
#include <random>

#include "ConnectionHandler.h"
#include "DataFetcherBase.h"

#include "mdnslib/tools.h"
#include "net/Address.h"
#include "net/Lib.h"
#include "std-generic/attributes.h"
#include "std-generic/compat/iterator.h"
#include "std-generic/cpplib/string_view.h"
#include "std/cpplib/ThreadUtils.h"
#include "std/cpplib/MakeShared.h"

#include "std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_TRANSPORT

namespace mdns
{

namespace {
const char dnsSdQueryName[] = "\x09_services\x07_dns-sd\x4_udp\x5local";
const char queryNameEnding[] = "\x04_tcp\x5local";
const char serviceName[] = "\x09_trueconf";
const char instanceName[] = "\x0bseliverstov";

const std::string txtVarCompanyName{ "company_name" };
const std::string txtVarServerName{ "server_name" };
const std::string txtVarWebURL{ "web_url" };

const char TXT_CompanyName[]="Horns_And_Hooves";
const char TXT_ServerName[]="seliverstov.trueconf.local#vcs";
const char SRV_HOSTNAME[]="\013seliverstov\010trueconf\005local";

const uint16_t PROBE_DELAY = 250;
const uint16_t REPROBE_DELAY = 1000;
const uint16_t ANNOUNCE_DELAY = 1000;
const size_t RFC_LABEL_SIZE_LIMIT = 63;

const uint32_t SRV_TTL = 120;
const uint32_t A_TTL = 120;
const uint32_t AAAA_TTL = A_TTL;
const uint32_t TXT_TTL = 7500;
const uint32_t PTR_TTL = 120;

const char responderError[]="MDNS responder: error: ";
const char responderFatalError[]="MDNS responder: fatal error: ";
const char responderNote[]="MDNS responder: note: ";

}

Responder::Responder(boost::asio::io_service& service, std::shared_ptr<DataFetcherBase> base)
	: connHandler_(vs::MakeShared<mdns::ConnectionHandler>(service, strand_))
	, timer_(service)
	, strand_(service)
	, instanceUniqueNumber_(0)
	, count_(0)
	, messageCounter_(0)
	, state_(RS_NONE)
	, data_(base)
	{}

void Responder::start()
{
	vs::FixThreadSystemMessages();

	if (!setupSockets())
	{
		dstream1 << "MDNS responder shutting down...\n";
		state_ = RS_NONE;
		return;
	}
	configureNames();
	formUniqueRecords(); // Unique records must be separate from others since they must be probed
	formSharedRecords();
	messageCounter_ = 0;
	if (!(formProbeQuery() &&
			formAnnounceResponse() &&
			formStableResponses()))
	{
		dstream1 << "MDNS responder shutting down...\n";
		state_ = RS_NONE;
		return;
	}
	state_ = RS_PREPROBE;
	preProbe();
}

bool Responder::configureNames()
{
	dnsSdQueryName_.clear();
	serviceQueryName_.clear();
	instanceQueryName_.clear();
	serverFakeName_.clear();

	dnsSdQueryName_.insert(dnsSdQueryName_.end(),
		dnsSdQueryName, dnsSdQueryName + vs::size(dnsSdQueryName));

	serviceQueryName_.insert(serviceQueryName_.end(),
		serviceName, serviceName + vs::size(serviceName) - 1);// skip the terminator
	serviceQueryName_.insert(serviceQueryName_.end(),
		queryNameEnding, queryNameEnding + vs::size(queryNameEnding));

	std::string instanceNameString = data_->getServerName();
	instanceName_ = {instanceNameString.begin(), instanceNameString.end()};
	if (instanceName_.empty())
	{
		dstream1 << responderFatalError << "failed to get a server name!\n";
		return false;
	}
	if (instanceName_.size() > RFC_LABEL_SIZE_LIMIT)
		instanceName_.resize(RFC_LABEL_SIZE_LIMIT - 1);
	instanceName_.insert(instanceName_.begin(), static_cast<char>(instanceName_.size()));
	instanceQueryName_.insert(instanceQueryName_.end(),
		instanceName_.begin(), instanceName_.end());
	instanceQueryName_.insert(instanceQueryName_.end(),
		serviceName, serviceName + vs::size(serviceName) - 1);
	instanceQueryName_.insert(instanceQueryName_.end(),
		queryNameEnding, queryNameEnding + vs::size(queryNameEnding));

	std::string serverFakeNameString = data_->getServerName();
	serverFakeName_ = {serverFakeNameString.begin(), serverFakeNameString.end()};
	auto iter = std::find(serverFakeName_.begin(), serverFakeName_.end(), '#');
	if (iter != serverFakeName_.end())
	{
		serverFakeName_.erase(iter, serverFakeName_.end());
		mdns::fromDotted(serverFakeName_);
	}
	return true;
}

void Responder::formFakeNameRecord(uint16_t port, unsigned int priority)
{
	std::vector<char> srvData;
	mdns::addons::SRV::form(srvData, priority, 100, port, serverFakeName_);
	fakeNameRecords_.emplace_back(mdns::RRecord(instanceQueryName_, mdns::TYPE::SRV, SRV_TTL, srvData), port);
}

namespace
{
bool is_in (const std::vector<std::pair<mdns::RRecord, uint16_t>>& records, uint16_t port)
{

	return (records.end() != std::find_if(records.begin(), records.end(),
		[port](const std::pair<mdns::RRecord, uint16_t>& record)
		{
			return record.second == port;
		}));
}
bool is_in (
	const std::vector<std::pair<mdns::RRecord, uint16_t>>& records,
	const std::set<unsigned int>& indices,
	uint16_t port)
{
	return (indices.end() != std::find_if(indices.begin(), indices.end(),
		[records, port](const unsigned int& index)
		{
			return records[index].second == port;
		})
	);
}

void searchAndInsertAddressRecords(
	mdns::Former& target,
	const std::vector<std::pair<mdns::RRecord, uint16_t>>& source,
	std::set<unsigned int>& indicesSource,
	uint16_t port)
{
	for (auto i = indicesSource.begin(); i != indicesSource.end();)
		if (source[*i].second == port &&
			(source[*i].first.rtype == mdns::TYPE::A ||
			 source[*i].first.rtype == mdns::TYPE::AAAA))
		{
			target.addRRecord(source[*i].first);
			i = indicesSource.erase(i);
		} else
			++i;
}

}

void Responder::addAddressRecord(std::vector<std::pair<mdns::RRecord, uint16_t>>& result,
	const boost::asio::ip::address& address, uint16_t port)
{
	std::vector<char> rawAddress;
	mdns::TYPE rtype;
	if (address.is_v4())
	{
		auto addressBytes = address.to_v4().to_bytes();
		rawAddress.insert(rawAddress.end(), addressBytes.begin(), addressBytes.end());
		rtype = mdns::TYPE::A;
	} else if (address.is_v6())
	{
		auto addressBytes = address.to_v6().to_bytes();
		rawAddress.insert(rawAddress.end(), addressBytes.begin(), addressBytes.end());
		rtype = mdns::TYPE::AAAA;
	} else
		return;
	mdns::RRecord addressRecord(serverFakeName_, rtype, A_TTL, rawAddress);
	if (addressRecords_.end() != addressRecords_.find(addressRecord))
		return;
	addressRecords_.insert(addressRecord);
	result.emplace_back(addressRecord, port);
	if (!is_in(fakeNameRecords_, port))
		formFakeNameRecord(port, data_->getConnectTcpCount() + fakeNameRecords_.size());

}

void Responder::formConnectTcpRecords()
{
	unsigned int srvPriority = 0;
	unsigned int connectTcpCount = data_->getConnectTcpCount();
	for (unsigned int i = 1; i <= connectTcpCount; i++)
	{
		auto endpoint = data_->getConnectTcp(i);
		if (net::is_domain_name(string_view(endpoint.first.data(), endpoint.first.size())))
		{
			std::vector<char> srvData;
			mdns::fromDotted(endpoint.first);
			mdns::addons::SRV::form(srvData, srvPriority, 100, endpoint.second, endpoint.first);
			connectTcpRecords_.emplace_back(
				mdns::RRecord(instanceQueryName_, mdns::TYPE::SRV, SRV_TTL, srvData), endpoint.second);
			++srvPriority;
		} else
		{
			boost::asio::ip::address address = boost::asio::ip::address::from_string(endpoint.first);
			addAddressRecord(connectTcpRecords_, address, endpoint.second);
		}
	}
}

void Responder::formListenersRecords()
{
	std::vector<boost::asio::ip::tcp::endpoint> listeners;
	data_->getListenerList(listeners);
	for (const auto& listener: listeners)
		addAddressRecord(listenerRecords_, listener.address(), listener.port());
}

void Responder::formTxtRecord()
{
	mdns::addons::TXT txtData;
	txtData.addPair(txtVarCompanyName, data_->getCompanyName());
	txtData.addPair(txtVarServerName, data_->getServerName());
	txtData.addPair(txtVarWebURL, data_->getWebURL());
	recordTxt_ = mdns::RRecord(instanceQueryName_, mdns::TYPE::TXT, TXT_TTL, txtData.container());
}

void Responder::formUniqueRecords()
{
	connectTcpRecords_.clear();
	listenerRecords_.clear();
	fakeNameRecords_.clear();
	recordTxt_.clear();
	addressRecords_.clear();
	uniqueRecords_.clear();

	formConnectTcpRecords();
	formListenersRecords();

	formTxtRecord();
}

void Responder::formSharedRecords()
{
	sharedRecords_.clear();

	sharedRecords_.emplace_back(dnsSdQueryName_, mdns::TYPE::PTR, PTR_TTL, serviceQueryName_);
	sharedRecords_.emplace_back(serviceQueryName_, mdns::TYPE::PTR, PTR_TTL, instanceQueryName_);
}

bool Responder::formProbeQuery()
{
	probeQuery_.clear();
	probeQuery_.setHeader(mdns::QR::QUERY, 1, 0,
		static_cast<mdns::NS>(connectTcpRecords_.size() + listenerRecords_.size() +
			fakeNameRecords_.size() + 1), 0);
	probeQuery_.addQuery(instanceQueryName_, mdns::TYPE::ANY);
	for (const auto& record: connectTcpRecords_)
		probeQuery_.addRRecord(record.first);
	for (const auto& record: listenerRecords_)
		probeQuery_.addRRecord(record.first);
	for (const auto& record: fakeNameRecords_)
		probeQuery_.addRRecord(record.first);
	probeQuery_.addRRecord(recordTxt_);
	uniqueRecords_ = probeQuery_.records();
	if (nullptr == probeQuery_.form())
	{
		dstream1 << responderFatalError << "failed to form the probe query!\n";
		return false;
	}
	return true;
}

bool Responder::formAnnounceResponse()
{
	announceResponse_.clear();
	// Maybe use single records instead of sharedRecords_?
	announceResponse_.setHeader(mdns::QR::RESPONSE, 0,
		static_cast<mdns::AN>(sharedRecords_.size() + connectTcpRecords_.size() +
			listenerRecords_.size() + fakeNameRecords_.size() + 1), 0, 0);
	for (auto& record: sharedRecords_)
	{
		record.cflush = mdns::CACHEFLUSH::YES;
		announceResponse_.addRRecord(record);
		record.cflush = mdns::CACHEFLUSH::NO;
	}
	for (auto& record: fakeNameRecords_)
	{
		record.first.cflush = mdns::CACHEFLUSH::YES;
		announceResponse_.addRRecord(record.first);
		record.first.cflush = mdns::CACHEFLUSH::NO;
	}
	for (auto& record: connectTcpRecords_)
	{
		record.first.cflush = mdns::CACHEFLUSH::YES;
		announceResponse_.addRRecord(record.first);
		record.first.cflush = mdns::CACHEFLUSH::NO;
	}
	for (auto& record: listenerRecords_)
	{
		record.first.cflush = mdns::CACHEFLUSH::YES;
		announceResponse_.addRRecord(record.first);
		record.first.cflush = mdns::CACHEFLUSH::NO;
	}
	recordTxt_.cflush = mdns::CACHEFLUSH::YES;
	announceResponse_.addRRecord(recordTxt_);
	recordTxt_.cflush = mdns::CACHEFLUSH::NO;
	if (nullptr == announceResponse_.form())
	{
		dstream1 << responderFatalError << "failed to form the announce response!\n";
		return false;
	}
	return true;
}

void Responder::formStableInstanceSrvResponse()
{
	mdns::AN an;// Answers count
	mdns::AR ar;// Additional records count
	std::set<uint16_t> fakePortNumbersCovered;
	for (const auto& record: connectTcpRecords_)
	{
		if (record.first.rtype == mdns::TYPE::SRV)
		{
			stableInstanceSrvResponse_.addRRecord(record.first);
			if (mdns::addons::SRV::copySrvTarget(record.first.rData) == serverFakeName_)
				fakePortNumbersCovered.insert(record.second);
		}
	}
	for (const auto& record: fakeNameRecords_)
		if (fakePortNumbersCovered.end() == fakePortNumbersCovered.find(record.second))
			stableInstanceSrvResponse_.addRRecord(record.first);
	an = static_cast<mdns::AN>(stableInstanceSrvResponse_.records().size());
	for (const auto& record: connectTcpRecords_)
		if (record.first.rtype == mdns::TYPE::A ||
			record.first.rtype == mdns::TYPE::AAAA)
			stableInstanceSrvResponse_.addRRecord(record.first);
	for (const auto& record: listenerRecords_)
		stableInstanceSrvResponse_.addRRecord(record.first);
	stableInstanceSrvResponse_.addRRecord(recordTxt_);
	ar = static_cast<mdns::AR>(stableInstanceSrvResponse_.records().size() - an);
	stableInstanceSrvResponse_.setHeader(mdns::QR::RESPONSE, 0, an, 0, ar);
}

void Responder::formStableServicePtrResponse()
{
	std::set<unsigned int> listenersPending;
	std::set<unsigned int> connectTcpPending;

	for (unsigned int i = 0; i < listenerRecords_.size(); ++i)
		listenersPending.insert(i);
	for (unsigned int i = 0; i < connectTcpRecords_.size(); ++i)
		connectTcpPending.insert(i);

	stableServicePtrResponse_.addRRecord(sharedRecords_[1]);

	for (auto i = connectTcpPending.begin(); i != connectTcpPending.end();)
	{
		auto element = connectTcpRecords_[*i];
		if (element.first.rtype == mdns::TYPE::SRV)
		{
			stableServicePtrResponse_.addRRecord(element.first);
			if (mdns::addons::SRV::copySrvTarget(element.first.rData) == serverFakeName_)
			{
				searchAndInsertAddressRecords(stableServicePtrResponse_, connectTcpRecords_,
					connectTcpPending, element.second);
				searchAndInsertAddressRecords(stableServicePtrResponse_, listenerRecords_,
					listenersPending, element.second);
			}
			i = connectTcpPending.erase(i);
		} else
			++i;
	}
	for (const auto& fakeRecord: fakeNameRecords_)
	{
		if (is_in(connectTcpRecords_, connectTcpPending, fakeRecord.second) ||
			is_in(listenerRecords_, listenersPending, fakeRecord.second))
		{
			stableServicePtrResponse_.addRRecord(fakeRecord.first);
			searchAndInsertAddressRecords(stableServicePtrResponse_, connectTcpRecords_,
				connectTcpPending, fakeRecord.second);
			if (is_in(listenerRecords_, listenersPending, fakeRecord.second))
				searchAndInsertAddressRecords(stableServicePtrResponse_, listenerRecords_,
					listenersPending, fakeRecord.second);
		}
	}
	if (!listenersPending.empty())
		dstream2 << responderError << "listenersPending set not empty!\n";
	if (!connectTcpPending.empty())
		dstream2 << responderError << "connectTcpPending set not empty!\n";

	stableServicePtrResponse_.addRRecord(recordTxt_);
	stableServicePtrResponse_.setHeader(mdns::QR::RESPONSE,
		0, 1, 0, static_cast<mdns::AR>(stableServicePtrResponse_.records().size()) - 1);
}

void Responder::formStableInstanceAResponse()
{
	for (const auto& record: connectTcpRecords_)
		if (record.first.rtype == mdns::TYPE::A)
			stableInstanceAResponse_.addRRecord(record.first);
	for (const auto& record: listenerRecords_)
		if (record.first.rtype == mdns::TYPE::A)
			stableInstanceAResponse_.addRRecord(record.first);
	stableInstanceAResponse_.setHeader(mdns::QR::RESPONSE,
		0, static_cast<mdns::AN>(stableInstanceAResponse_.records().size()), 0, 0);
}

void Responder::formStableInstanceAaaaResponse()
{
	for (const auto& record: connectTcpRecords_)
		if (record.first.rtype == mdns::TYPE::AAAA)
			stableInstanceAaaaResponse_.addRRecord(record.first);
	for (const auto& record: listenerRecords_)
		if (record.first.rtype == mdns::TYPE::AAAA)
			stableInstanceAaaaResponse_.addRRecord(record.first);
	stableInstanceAaaaResponse_.setHeader(mdns::QR::RESPONSE,
		0, static_cast<mdns::AN>(stableInstanceAaaaResponse_.records().size()), 0, 0);
}

void Responder::formStableInstanceTxtResponse()
{
	stableInstanceTxtResponse_.setHeader(mdns::QR::RESPONSE,
		0, 1, 0, 0);
	stableInstanceTxtResponse_.addRRecord(recordTxt_);
}

void Responder::formStableInstanceAnyResponse()
{
	for (const auto& record: connectTcpRecords_)
		if (record.first.name == instanceQueryName_ &&
			record.first.rtype != mdns::TYPE::A && record.first.rtype != mdns::TYPE::AAAA)
			stableInstanceAnyResponse_.addRRecord(record.first);

	for (const auto& record: fakeNameRecords_)
		if (record.first.name == instanceQueryName_ &&
			record.first.rtype != mdns::TYPE::A && record.first.rtype != mdns::TYPE::AAAA)
			stableInstanceAnyResponse_.addRRecord(record.first);

	stableInstanceAnyResponse_.addRRecord(recordTxt_);

	mdns::AN answersCount = static_cast<mdns::AN>(stableInstanceAnyResponse_.records().size());

	for (const auto& record: connectTcpRecords_)
		if (record.first.rtype == mdns::TYPE::A || record.first.rtype == mdns::TYPE::AAAA)
			stableInstanceAnyResponse_.addRRecord(record.first);

	for (const auto& record: fakeNameRecords_)
		if (record.first.rtype == mdns::TYPE::A || record.first.rtype == mdns::TYPE::AAAA)
			stableInstanceAnyResponse_.addRRecord(record.first);

	stableInstanceAnyResponse_.setHeader(mdns::QR::RESPONSE, 0, answersCount, 0,
		static_cast<mdns::AR>(stableInstanceAnyResponse_.records().size() - answersCount));
}

bool Responder::formStableResponses()
{
	stableDnsSdResponse_.clear();
	stableServicePtrResponse_.clear();
	stableInstanceSrvResponse_.clear();
	stableInstanceAResponse_.clear();
	stableInstanceAaaaResponse_.clear();
	stableInstanceTxtResponse_.clear();
	stableInstanceAnyResponse_.clear();

	stableDnsSdResponse_.setHeader(mdns::QR::RESPONSE, 0, 1, 0, 0);
	stableDnsSdResponse_.addRRecord(sharedRecords_[0]);

	formStableServicePtrResponse();
	formStableInstanceSrvResponse();
	formStableInstanceAResponse();
	formStableInstanceAaaaResponse();
	formStableInstanceTxtResponse();
	formStableInstanceAnyResponse();


	if (nullptr == stableDnsSdResponse_.form())
	{
		dstream1 << responderFatalError << "failed to form the stable DNS-SD response!\n";
		return false;
	}
	if (nullptr == stableServicePtrResponse_.form())
	{
		dstream1 << responderFatalError << "failed to form the stable service PTR response!\n";
		return false;
	}
	if (nullptr == stableInstanceSrvResponse_.form())
	{
		dstream1 << responderFatalError << "failed to form the stable instance SRV response!\n";
		return false;
	}
	if (nullptr == stableInstanceAResponse_.form())
	{
		dstream1 << responderFatalError << "failed to form the stable instance A response!\n";
		return false;
	}
	if (nullptr == stableInstanceAaaaResponse_.form())
	{
		dstream1 << responderFatalError << "failed to form the stable instance AAAA response!\n";
		return false;
	}
	if (nullptr == stableInstanceTxtResponse_.form())
	{
		dstream1 << responderFatalError << "failed to form the stable instance TXT response!\n";
		return false;
	}
	if (nullptr == stableInstanceAnyResponse_.form())
	{
		dstream1 << responderFatalError << "failed to form the stable instance ANY response!\n";
		return false;
	}
	return true;
}

void Responder::stop()
{
	strand_.post(
		[shared_this = shared_from_this()]()
		{
			shared_this->connHandler_->shutdown();
			shared_this->state_ = RS_NONE;
		}
	);
}

void Responder::sendMsg(const boost::asio::mutable_buffers_1& buffer, int index)
{
	connHandler_->sendMsg(buffer,
		[weak_this = weak_from_this()]
		(const boost::system::error_code& error, size_t count, uint16_t /*socket*/)
		{
			if (auto shared_this = weak_this.lock())
			{
				shared_this->strand_.post(
					[shared_this = std::move(shared_this), error, count]()
					{
						shared_this->sendHandler(error, count);
					}
				);
			}
		},
		index);
}

void Responder::recvMsg(int index)
{
	connHandler_->recvMsg(
		[weak_this = weak_from_this()]
		(const boost::system::error_code& error, const void* buffer, size_t count, uint16_t socket)
		{
			if (auto shared_this = weak_this.lock())
			{
				shared_this->strand_.post(
					[shared_this = std::move(shared_this), buffer, error, count, socket]()
					{
						shared_this->recvHandler(error, buffer, count, socket);
					}
				);
			}
		},
		index);
}

void Responder::preProbe()
{
	dstream4 << responderNote << "switched to state: preprobe\n";
//	Start receiving ASAP but take care as to not try receiving while responder is shutting down
	strand_.post(
		[weak_this = weak_from_this()] ()
		{
			if (auto shared_this = weak_this.lock())
				shared_this->recvMsg();
		}
	);

	messageCounter_ = 0;
	std::random_device randomDevice;
	std::mt19937 generator(randomDevice());
	std::uniform_int_distribution<short> distribution(1, 250);
	timer_.expires_from_now(std::chrono::milliseconds(distribution(generator)));
	timer_.async_wait(
		[weak_this = weak_from_this()] (const boost::system::error_code& error)
		{
			if (auto shared_this = weak_this.lock())
			{
				shared_this->strand_.post(
					[shared_this = std::move(shared_this), error]()
					{
						shared_this->timerHandler(error);
					}
				);
			}
		}
	);
}

void Responder::probe()
{
	dstream4 << responderNote << "sending probe query #" << messageCounter_ + 1 << '\n';
	++messageCounter_;
	sendMsg(boost::asio::buffer(probeQuery_.data(), probeQuery_.size()));
}

void Responder::announce()
{
	dstream4 << responderNote << "sending announce #" << messageCounter_ + 1 << '\n';
	++messageCounter_;
	sendMsg(boost::asio::buffer(announceResponse_.data(), announceResponse_.size()));
}

void Responder::timerHandler(const boost::system::error_code& error)
{
	if (error)
	{
		if (error.value() == boost::asio::error::operation_aborted)
			return;
		dstream4 << responderError << error.message();
		return;
	}
	switch(state_)
	{
	case RS_PREPROBE: state_ = RS_PROBE; VS_FALLTHROUGH;
	case RS_PROBE: probe(); break;
	case RS_REPROBE: probe(); break;
	case RS_ANNOUNCE: announce(); break;
	default:
		break;
	}
}

void Responder::sendHandler(const boost::system::error_code& error, size_t count)
{
	if (error)
	{
		if (error.value() == boost::asio::error::operation_aborted)
			return;
		dstream3 << responderError << error.message();
		return;
	}
	dstream4 << responderNote << "successfully sent the message of size "
		<< count << '\n';
	switch(state_)
	{
	case RS_PROBE:
		if (messageCounter_ > 2)
		{
			state_ = RS_ANNOUNCE;
			messageCounter_ = 0;
		}
		timer_.expires_from_now(std::chrono::milliseconds(PROBE_DELAY));
		timer_.async_wait(
			[weak_this = weak_from_this()] (const boost::system::error_code& error)
			{
				if (auto shared_this = weak_this.lock())
				{
					shared_this->strand_.post(
						[shared_this = std::move(shared_this), error]()
						{
							shared_this->timerHandler(error);
						}
					);
				}
			}
		);
		break;
	case RS_ANNOUNCE:
		if (messageCounter_ > 1)
		{
			state_ = RS_STABLE;
			messageCounter_ = 0;
		}
		timer_.expires_from_now(std::chrono::milliseconds(ANNOUNCE_DELAY));
		timer_.async_wait(
			[weak_this = weak_from_this()] (const boost::system::error_code& error)
			{
				if (auto shared_this = weak_this.lock())
				{
					shared_this->strand_.post(
						[shared_this = std::move(shared_this), error]()
						{
							shared_this->timerHandler(error);
						}
					);
				}
			}
		);
		break;
	default:
		break;
	}
}

void Responder::reconfigure()
{
	reconfigureName();
	uniqueRecords_.clear();
	formUniqueRecords(); // Unique records must be separate from others since they must be probed
	formSharedRecords();
	messageCounter_ = 0;
	formProbeQuery();
	formAnnounceResponse();
	formStableResponses();
	state_ = RS_PREPROBE;
	preProbe();
}

// In case someone uses our name, we need to change it, making it unique
void Responder::reconfigureName()
{
	++instanceUniqueNumber_;
	std::string number = std::to_string(instanceUniqueNumber_);
	if (instanceName[0] + number.length() > 63)
	{
		if (number.length() == 1)
			return;
		instanceUniqueNumber_ = 0;
		number = std::to_string(instanceUniqueNumber_);
	}
	instanceQueryName_.clear();
	instanceQueryName_.insert(instanceQueryName_.end(),
		instanceName, instanceName + vs::size(instanceName) - 1);
	instanceQueryName_.insert(instanceQueryName_.end(),
		number.begin(), number.end());
	instanceQueryName_.insert(instanceQueryName_.end(),
		serviceName, serviceName + vs::size(serviceName) - 1);
	instanceQueryName_.insert(instanceQueryName_.end(),
		queryNameEnding, queryNameEnding + vs::size(queryNameEnding));

	instanceQueryName_[0] += static_cast<char>(number.length());
}

bool Responder::recvHandlerProbe(const void* buffer, size_t count)
{
	parser_.clear();
	parser_.filter(FILTER::ANY);
	parser_.filterName(instanceQueryName_);
//	Did we find 'ANY' request and our name somewhere?
	if (parser_.parse(buffer, count))
	{
		if (parser_.header.qr == mdns::QR::RESPONSE)
		{
//		Someone responded my probe query, meaning that he probably won the clash
			if (state_ == RS_PROBE)
			{
				dstream3 << responderNote << "someone responded the probe query with server's name, "
					"trying to reprobe...\n";
				state_ = RS_REPROBE;
				messageCounter_ = 0;
				timer_.expires_from_now(std::chrono::milliseconds(REPROBE_DELAY));
				timer_.async_wait(
					[weak_this = weak_from_this()] (const boost::system::error_code& error)
					{
						if (auto shared_this = weak_this.lock())
						{
							shared_this->strand_.post(
								[shared_this = std::move(shared_this), error]()
								{
									shared_this->timerHandler(error);
								}
							);
						}
					}
				);
				return true;
			}
//			Failed second time, time to reconfigure
			dstream3 << responderNote << "someone responded the probe query with server's name (again), "
				"have to reconfigure\n";
			reconfigure();
			return false;
		}
		int clashResult = 0;
		if (parser_.header.qr == mdns::QR::QUERY)
//		Someone is probing my name; is he a competitor? If authRecs section isn't empty, than he probably is
			clashResult = clashCompetingRecords(uniqueRecords_, parser_.authRecs);
		else if (parser_.header.qr == mdns::QR::QUERY && -1 == clashResult)
		{
			if (state_ == RS_PROBE)
			{
				dstream3 << responderNote << "met competitor while probing, trying to reprobe...\n";
				state_ = RS_REPROBE;
				messageCounter_ = 0;
				timer_.expires_from_now(std::chrono::milliseconds(REPROBE_DELAY));
				timer_.async_wait(
					[weak_this = weak_from_this()] (const boost::system::error_code& error)
					{
						if (auto shared_this = weak_this.lock())
						{
							shared_this->strand_.post(
								[shared_this = std::move(shared_this), error]()
								{
									shared_this->timerHandler(error);
								}
							);
						}
					}
				);
			} else
			{
				dstream3 << responderNote << "reprobing failed, reconfiguring...\n";
				reconfigure();
				return false;
			}
		}
	}
	return true;
}

bool Responder::recvHandlerAnnounce(const void* buffer, size_t count)
{
	parser_.clear();
	parser_.filterName(instanceQueryName_);
	parser_.filter(static_cast<uint32_t>(
		mdns::FILTER::RESPONSE |
		mdns::FILTER::SRV |
		mdns::FILTER::PTR |
		mdns::FILTER::TXT));
//	Received another response with my name?
	if (parser_.parse(buffer, count))
	{
//		Check: maybe it's just my packet that returned to me?
		for (const auto& record: parser_.responses)
			if (record.name == instanceQueryName_)
			{
				bool weCool = true;
				for (const auto& unique : uniqueRecords_)
					if (unique.rtype == record.rtype)
					{
						if (unique.rData == record.rData)
						{
							weCool = true;
							break;
						}
						else
							weCool = false;
					}
				if (!weCool)
				{
					dstream2 << responderNote << "something is interfering with responder"
						" on announce stage! Trying to probe again...\n"
						"If you see this message for more then 5-10 times, this means either"
						" that there is a logical error in the program, or someone is trying to"
						" interfere with responder!\n";
					state_ = RS_PREPROBE;
					preProbe();
					return false;
				}
			}
	}
	return true;
}

bool Responder::recvHandlerStable(const void* buffer, size_t count, uint16_t socket)
{
	parser_.clear();
	parser_.filterName(dnsSdQueryName_);
	parser_.filterName(serviceQueryName_);
	parser_.filterName(instanceQueryName_);
//	Did someone mention me or my service or requested all services?
	if (parser_.parse(buffer, count))
	{
		if (parser_.header.qr == mdns::QR::RESPONSE)
		{
//			Does response include my instance name?
			if (!parser_.nameFound(2))
				return true;
//			It does. Maybe it's just my own packet?
			for (const auto& record: parser_.responses)
				if (record.name == instanceQueryName_)
				{
					bool weCool = true;
					for (const auto& unique : uniqueRecords_)
						if (unique.rtype == record.rtype)
						{
							if (unique.rData == record.rData)
							{
								weCool = true;
								break;
							}
							else
								weCool = false;
						}
					if (!weCool)
					{
//						A conflict! Time to probe again and possibly reconfigure
						dstream3 << responderNote << "Someone seems to be trying to impersonate"
							" the responder; switching to probing state again...\n";
						state_ = RS_PREPROBE;
						preProbe();
						return false;
					}
				}
			return true;
		}
		for (const auto& query: parser_.queries)
		{
			bool needToRespond = false;
			boost::asio::mutable_buffers_1 message =
				boost::asio::buffer(stableDnsSdResponse_.data(), stableDnsSdResponse_.size());
			if (parser_.nameFound(0) &&
				query.qtype == mdns::TYPE::PTR &&
				query.name == dnsSdQueryName_)
			{
//				Maybe the requester already knows us?
				bool foundSame = false;
				for (const auto& knownRecord: parser_.responses)
					if (knownRecord.rtype == mdns::TYPE::PTR &&
						knownRecord.rData == serviceQueryName_ &&
						knownRecord.name == dnsSdQueryName_)
					{
						foundSame = true;
						break;
					}
				if (!foundSame)
				{
					needToRespond = true;
					message = boost::asio::buffer(stableDnsSdResponse_.data(), stableDnsSdResponse_.size());
				}
			}
			else if ((parser_.nameFound(1) &&
				query.name == serviceQueryName_) ||
				(parser_.nameFound(2) &&
				query.name == instanceQueryName_))
			{
//				Maybe the requester already knows us?
				bool foundSame = false;
				for (const auto& knownRecord: parser_.responses)
				{
					if (knownRecord.name == serviceQueryName_)
					{
						if (knownRecord.rtype == mdns::TYPE::PTR &&
							sharedRecords_[1].rData == knownRecord.rData)
							foundSame = true;
					}
					else if (knownRecord.name == instanceQueryName_)
					{
						for (const auto& myRecord: uniqueRecords_)
							if (myRecord.rtype == knownRecord.rtype &&
								myRecord.rdLength == knownRecord.rdLength &&
								myRecord.rData == knownRecord.rData)
							{
								foundSame = true;
								break;
							}
					}
					if (foundSame)
						break;
				}
				if (!foundSame)
				{
					needToRespond = true;
					if (query.name == serviceQueryName_ && query.qtype == mdns::TYPE::PTR)
						message = boost::asio::buffer(
							stableServicePtrResponse_.data(), stableServicePtrResponse_.size());

					else if (query.name == instanceQueryName_)
					{
						if (query.qtype == mdns::TYPE::SRV)
							message = boost::asio::buffer(
								stableInstanceSrvResponse_.data(), stableInstanceSrvResponse_.size());
						if (query.qtype == mdns::TYPE::A)
							message = boost::asio::buffer(
								stableInstanceAResponse_.data(), stableInstanceAResponse_.size());
						if (query.qtype == mdns::TYPE::AAAA)
							message = boost::asio::buffer(
								stableInstanceAaaaResponse_.data(), stableInstanceAaaaResponse_.size());
						if (query.qtype == mdns::TYPE::TXT)
							message = boost::asio::buffer(
								stableInstanceTxtResponse_.data(), stableInstanceTxtResponse_.size());
						if (query.qtype == mdns::TYPE::ANY)
							message = boost::asio::buffer(
								announceResponse_.data(), announceResponse_.size());
					}
				}
			}
			if (!needToRespond)
				continue;
			sendMsg(message, socket);
		}

	}
	return true;
}

void Responder::recvHandler(const boost::system::error_code& error, const void* buffer, size_t count,
	uint16_t socket)
{
	if (error)
	{
		if (error.value() == boost::asio::error::operation_aborted)
			return;
		dstream3 << responderError << error.message();
		if (net::IsRecoverableUDPReadError(error))
			recvMsg(socket);
		return;
	}
	bool resetHappened = false;
	switch(state_)
	{
	case RS_REPROBE: VS_FALLTHROUGH;
	case RS_PROBE:
		resetHappened = !recvHandlerProbe(buffer, count);
		break;
	case RS_ANNOUNCE:
		resetHappened = !recvHandlerAnnounce(buffer, count);
		break;
	case RS_STABLE:
		resetHappened = !recvHandlerStable(buffer, count, socket);
		break;
	default:
		break;
	}
	if (resetHappened) // Meaning the preprobe timer has been set and read will be requested shortly
	{
		connHandler_->cancel();
		return;
	}
	recvMsg(socket);
}

bool Responder::setupSockets()
{
	return connHandler_->init();
}
}
