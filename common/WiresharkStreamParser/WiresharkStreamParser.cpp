#if defined(_WIN32) // Not ported yet

#include "WiresharkStreamParser.h"

#include <string>
#include <sstream>
#include <algorithm>

#include <cassert>
#include <cstring>
#include <cctype>

// !!! EBNF Grammar (unused at this point) !!!
//
// Packets = {PacketDescription} .
// PacketDescription = "char" PacketID "=" PacketData .
// PacketID = "packet" Number "_" Number .
// PacketData = "{" PacketBody "}" .
// PacketBody = HexNumber { HexNumber [","] }
//
// Number = Digit { Digit } .
// HexNumber = ("0x" | "0X")  HexDigit { HexDigit } .
// HexDigit = Digit | "A" ... "F" | "a" ... "f" .
// Digit = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" .
//

WiresharkStreamParser::Packet::Packet(void)
	: peer_no(0), peer_packet_no(0), stream_packet_no(0)
{
}

WiresharkStreamParser::Peer::Peer(void)
	: peer_no(0)
{
}

// !!! WiresharkStreamParser !!!
WiresharkStreamParser::WiresharkStreamParser(void)
{}

WiresharkStreamParser::~WiresharkStreamParser(void)
{}

const WiresharkStreamParser::StreamPtr WiresharkStreamParser::GetStream(void) const
{
	return m_stream;
}

enum States {
	StError = -1,
	StStartPacketParsing,
	StParsePacketInfo,
	StPreparePacket,
	StParsePacketBody,
	StEndPacketParsing,
	StEnd,
};

bool WiresharkStreamParser::Parse(std::istream &input, std::string &error_message)
{
	auto stream = std::make_shared<Stream>();
	size_t stream_packets_count = 0;
	size_t peers_count = 0;
	size_t lineno = 0;
	bool keep_line = false;

	auto state = StStartPacketParsing;
	auto status = true;

	std::string line;
	//std::string token;
	size_t current_peer_packet_no = 0;
	size_t current_peer_no   = 0;

	PacketPtr packet;

	if (!input.good())
		return false;

	auto get_line = [&](void) -> bool {
		line.clear();
		if (input.eof() || !input.good())
		{
			return false;
		}

		getline(input, line);
		lineno++;
		return true;
	};

	auto is_empty_line = [&line](void) -> bool {
		size_t i;

		if (line.empty())
		{
			return true;
		}

		// skip white spaces
		for (i = 0; i < line.size(); i++)
		{
			if (!isspace(line[i]))
			{
				break;
			}
		}

		// check fo C++ style comment
		auto p = strstr(&line[i], "//");
		if (p == &line[i])
		{
			return true;
		}

		return false;
	};

	while (state != StEnd)
	{
		if (state == StError)
		{
			error_message = "Unexpected input at line " + std::to_string(lineno) + ".";
			status = false;
			state = StEnd;
			continue;
		}

		if (!keep_line)
		{
			if (!get_line())
			{
				if (state == StStartPacketParsing)
				{
					status = true;
				}
				else
				{
					error_message = "Unexpected EOF at line " + std::to_string(lineno) + ".";
					status = false;
				}

				state = StEnd;
				continue;
			}
		}

		// skip empty lines
		if (!keep_line && is_empty_line())
		{
			continue;
		}

		keep_line = false;

		switch (state)
		{
			case StStartPacketParsing:
			{
				packet = nullptr;
				keep_line = true;
				state = StParsePacketInfo;
			}
				break;
			case StParsePacketInfo:
			{
				long n;
				const char *p;

				assert(packet == nullptr);

				if ((p = strstr(line.c_str(), "char")) == NULL)
				{
					break;
				}

				p += 4; // skip "char"
				if ((p = strstr(p, "peer")) == NULL)
				{
					state = StError;
					break;
				}

				p += 4; // skip "peer"

				// read peer number
				n = strtol(p, NULL, 10);
				current_peer_no = (size_t)n;

				if ((p = strstr(p, "_")) == NULL)
				{
					state = StError;
					break;
				}

				p += 1; // skip "_"

				// read packet number
				n = strtol(p, NULL, 10);
				current_peer_packet_no = n;

				// validate data
				if ((p = strstr(p, "=")) == NULL)
				{
					state = StError;
					break;
				}

				p += 1; // skip "="

				if ((p = strstr(p, "{")) == NULL)
				{
					state = StError;
					break;
				}

				// handle packet body
				state = StPreparePacket;
				break;
			}
			break;
			case StPreparePacket:
			{
				assert(packet == nullptr);
				//assert(stream_packets_count > 0);
				assert(current_peer_no <= stream->peers.size());
				packet = std::make_shared<Packet>();

				state = StParsePacketBody;
				keep_line = true;
				break;
			}
			break;
			case StParsePacketBody:
			{
				// convert line to lower space
				std::transform(line.begin(), line.end(), line.begin(), ::tolower);

				auto *p = strstr(line.c_str(), "0x");
				// read bytes
				while (p != NULL)
				{
					p += 2; // skip "0x"
					long n = strtol(p, NULL, 16);

					packet->data.push_back((char)n);
					p = strstr(p, "0x");
				}

				if ((p = strstr(line.c_str(), "}")) != NULL)
				{
					state = StEndPacketParsing;
					keep_line = true;
				}
			}
			break;
			case StEndPacketParsing:
			{
				packet->peer_no = current_peer_no;
				packet->peer_packet_no = current_peer_packet_no;
				packet->stream_packet_no = stream_packets_count;

				assert(current_peer_no <= stream->peers.size());

				// create new peer
				if (stream->peers.size() == current_peer_no)
				{
					auto peer = std::make_shared<Peer>();
					peer->peer_no = current_peer_no;
					stream->peers.push_back(peer);
				}

				// save information
				stream->peers[current_peer_no]->packets.push_back(packet);
				stream->packets.push_back(packet);

				// increase total read number of packets
				stream_packets_count++;
				state = StStartPacketParsing;
			}
			break;
			default:
			{
				state = StError;
			}
			break;
		}
	}

	if (status)
	{
		m_stream = stream;
	}

	return status;
}

bool WiresharkStreamParser::Parse(const char *input_data, std::string &error_message)
{
	if (input_data == nullptr)
	{
		return false;
	}
	std::stringstream input(input_data);

	return Parse(input, error_message);
}

void WiresharkStreamParser::ResetStream(void)
{
	m_stream = nullptr;
}

#endif
