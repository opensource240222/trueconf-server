#pragma once
#include "streams/fwd.h"
#include "std/cpplib/VS_MediaFormat.h"
#include "std-generic/cpplib/SharedBuffer.h"
#include "std-generic/asio_fwd.h"

#include "std-generic/compat/memory.h"
#include <string>

#include "boost/signals2.hpp"
#include <boost/asio/ip/tcp.hpp>
#include "std-generic/cpplib/synchronized.h"

class VS_StreamBufferedConnection;

class VS_TransceiverParticipant
	: public vs::enable_shared_from_this<VS_TransceiverParticipant>
{

public:
	virtual ~VS_TransceiverParticipant();

	boost::signals2::connection ConnectToNewFrame(const boost::signals2::signal<void (const vs::SharedBuffer &)>::slot_type &slot)
	{
		return m_fireNewFrame.connect(slot);
	}
	boost::signals2::connection ConnectToKeyFrameRequest(const boost::signals2::signal<void(void)>::slot_type &slot)
	{
		return m_fireKeyFrameRequest.connect(slot);
	}
	boost::signals2::connection ConnectToSetBitrateRequest(const boost::signals2::signal<void(unsigned int)>::slot_type &slot)
	{
		return m_fireSetBitrateRequest.connect(slot);
	}
	boost::signals2::connection ConnectToChangeSndFormatRequest(const boost::signals2::signal<void(VS_MediaFormat &mf)>::slot_type &slot)
	{
		return m_fireChangeSndFormatRequest.connect(slot);
	}
	boost::signals2::connection ConnectToChangeRcvFormatRequest(const boost::signals2::signal<void (VS_MediaFormat &mf, bool need_reply)>::slot_type &slot)
	{
		return m_fireChangeRcvFormatRequest.connect(slot);
	}
	boost::signals2::connection ConnectToChangeRcvFormatReply(const boost::signals2::signal<void(VS_MediaFormat &mf)>::slot_type &slot)
	{
		return m_fireChangeRcvFormatReply.connect(slot);
	}
	boost::signals2::connection ConnectToConnectionDie(const boost::signals2::signal<void(void)>::slot_type &slot)
	{
		return m_fireConnectionDie.connect(slot);
	}

	const char* GetPartName() const;
	const char* GetConfName() const;

	bool ConnectStream();
	void Stop();
	void WaitForStop();

	void SendFrame(const unsigned char *buf, const unsigned long size, stream::Track track);
	void SendFrame(vs::SharedBuffer&& buffer);
	void SendFrame(const vs::SharedBuffer& buffer)
	{
		return SendFrame(vs::SharedBuffer(buffer));
	}

protected:
	VS_TransceiverParticipant(
		std::string ServerAddr, std::string ServerEP,
		std::string confName, std::string partName,
		boost::asio::io_service &ios);

private:
	void OnNewBuffer(const vs::SharedBuffer& buffer);
	bool TryConnect(unsigned try_id);
	void OnConnect(unsigned try_id, std::shared_ptr<VS_StreamBufferedConnection>&& connection, bool is_input);
	void SendOrSaveBuffer(vs::SharedBuffer&& buffer);

	std::shared_ptr<VS_StreamBufferedConnection> m_in;
	std::shared_ptr<VS_StreamBufferedConnection> m_out;

	std::vector<boost::asio::ip::tcp::endpoint> m_serverAddrs;
	std::string m_serverEP;
	std::string	m_conf_name;
	std::string	m_part_name;
	bool m_isStopped;
	bool m_tryingTls = false;
	bool m_tlsEnabled;
	boost::asio::io_service& m_ios;

	std::vector<uint8_t> m_out_buffer;
	size_t maxReadBuffSize = 32 * 1024;
	size_t maxWriteBuffSize = 1024 * 1024;

	struct ConnectionsStatus {
		bool inputConnectionActive = false;
		bool outputConnectionActive = false;
		unsigned try_id = 0;
	};
	vs::Synchronized<ConnectionsStatus> m_connsStatus;

	boost::signals2::signal<void (const vs::SharedBuffer&)> m_fireNewFrame;
	boost::signals2::signal<void (void)> m_fireKeyFrameRequest;
	boost::signals2::signal<void (unsigned int)> m_fireSetBitrateRequest;
	boost::signals2::signal<void (VS_MediaFormat &mf)> m_fireChangeSndFormatRequest;
	boost::signals2::signal<void (VS_MediaFormat &mf, bool need_reply)> m_fireChangeRcvFormatRequest;
	boost::signals2::signal<void (VS_MediaFormat &mf)> m_fireChangeRcvFormatReply;
	boost::signals2::signal<void (void)> m_fireConnectionDie;
};