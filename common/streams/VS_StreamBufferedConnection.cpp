#include "VS_StreamBufferedConnection.h"
#include "Protocol.h"
#include "Handshake.h"
#include "net/tls/socket.h"
#include "std-generic/cpplib/SharedBuffer.h"

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_STREAMS

VS_StreamBufferedConnection::VS_StreamBufferedConnection(std::shared_ptr<net::BufferedConnectionInterface> connection)
	: m_connection(std::move(connection))
{
}

VS_StreamBufferedConnection::~VS_StreamBufferedConnection(void)
{
}

void VS_StreamBufferedConnection::StartHandshake(stream::ClientType type, string_view serverEP, string_view confName, string_view userName)
{
	const stream::Track tracks[] = {
		stream::Track::audio,
		stream::Track::video,
		stream::Track::old_command,
		stream::Track::data,
		stream::Track::command,
	};
	uint8_t ftracks[256];
	stream::TracksToFTracks(ftracks, tracks, sizeof(tracks)/sizeof(tracks[0]));
	uint8_t mtracks[32];
	stream::FTracksToMTracks(mtracks, ftracks);
	auto hs = stream::CreateHandshake(type, confName, userName, userName, serverEP, mtracks);
	if (!hs)
	{
		m_connection->Shutdown();
		return;
	}

	const size_t size = sizeof(net::HandshakeHeader) + hs->body_length + 1;
	Send(vs::SharedBuffer(std::move(hs), size));
}

size_t VS_StreamBufferedConnection::OnReceive(const void* data, size_t size)
{
	if (!m_handshakeDone)
	{
		if(size < sizeof(net::HandshakeHeader))
			return 0;
		const net::HandshakeHeader* hs = reinterpret_cast<const net::HandshakeHeader*>(data);
		if (hs->head_cksum != net::GetHandshakeHeaderChecksum(*hs))
		{
			m_connection->Shutdown();
			return 0;
		}

		const size_t body_size = hs->body_length + 1;
		if (size < sizeof(net::HandshakeHeader) + body_size)
			return 0;

		const uint8_t* read_mtracks;
		if (!stream::GetHandshakeResponseFields(hs, read_mtracks))
		{
			m_connection->Shutdown();
			return 0;
		}

		m_handshakeDone = true;
		return sizeof(net::HandshakeHeader) + body_size;
	} else
	{
		if (size < sizeof(stream::FrameHeader))
			return 0;

		stream::FrameHeader f;
		std::memcpy(&f, data, sizeof(stream::FrameHeader));
		const size_t framesize = sizeof(stream::FrameHeader) + f.length;

		if (size < framesize)
			return 0;

		if (f.cksum != stream::GetFrameBodyChecksum(static_cast<const char*>(data) + sizeof(stream::FrameHeader), f.length))
			return framesize;

		vs::SharedBuffer sb(framesize);
		std::memcpy(sb.data<void>(), data, framesize);

		// ok.
		m_fireNewBuffer(std::move(sb));
		return framesize;
	}

	return size;
}

bool VS_StreamBufferedConnection::OnError(const boost::system::error_code& err)
{
	if (err == boost::asio::error::no_buffer_space) {
		dstream4 << "VS_StreamBufferedConnection error " << err.message();
		return true;
	}

	if (!m_closed)
		m_fireDisconnect();
	m_closed = true;
	return true;
}

void VS_StreamBufferedConnection::OnClose() {
	if (!m_closed)
		m_fireDisconnect();
	m_closed = true;
}

bool VS_StreamBufferedConnection::IsHandshakeDone(void)
{
	return m_handshakeDone;
}

void VS_StreamBufferedConnection::Send(vs::SharedBuffer&& frame) {
	m_connection->Send(std::move(frame));
}

void VS_StreamBufferedConnection::Shutdown()
{
	m_connection->Shutdown();
}