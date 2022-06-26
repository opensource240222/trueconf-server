#pragma once

#include <vector>
#include <memory>
#include <iostream>

//
// Artem Boldarev (02.11.16):
// This is a class which parses stream data exported from Wireshark as C array and
// returns structured representation of it which is suitable for automitised processing.
//
// Tested with Wireshark 2.2.1.
//

class WiresharkStreamParser {
public:
	// public types
	struct Packet
	{
		Packet(void);

		size_t  peer_no;
		size_t  peer_packet_no;
		size_t  stream_packet_no;
		std::vector<char> data;
	};

	typedef std::shared_ptr<Packet> PacketPtr;

	struct Peer
	{
		Peer(void);

		size_t peer_no;
		std::vector<PacketPtr> packets;
	};

	typedef std::shared_ptr<Peer>   PeerPtr;

	struct Stream
	{
		std::vector<PeerPtr> peers;
		std::vector<PacketPtr> packets;
	};

	typedef std::shared_ptr<Stream> StreamPtr;
public:
	WiresharkStreamParser(void);
	~WiresharkStreamParser(void);

	// public methods
	bool Parse(std::istream &input, std::string &error_message);
	bool Parse(const char *input_data, std::string &error_message);

	const StreamPtr GetStream(void) const;
    void ResetStream(void);

	// private methods
private:
	// class data
private:
	StreamPtr m_stream;
};
