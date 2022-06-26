
#pragma once

#include "Handshake.h"
#include "../acs_v2/Handler.h"
#include "../streams/Protocol.h"

namespace bwt
{
	acs::Handler::stream_buffer create_handshake(const Handshake& handshake, const Endpoint& endpoint)
	{
		acs::Handler::stream_buffer handshake_request;
		handshake_request.resize(sizeof(Handshake) + 1 + endpoint.endpoint.size() + 1);
		Handshake* handshake_ptr = (Handshake*)handshake_request.data();
		strncpy(handshake_ptr->hs.primary_field, VS_Bwt_PrimaryField, sizeof(handshake_ptr->hs.primary_field));
		handshake_ptr->hs.version = 0x0D;
		handshake_ptr->hs.body_length = sizeof(Handshake) - sizeof(net::HandshakeHeader) + 1 + endpoint.endpoint.size();
		handshake_ptr->type = !handshake.type;
		handshake_ptr->send_time_ms = handshake.send_time_ms;
		handshake_ptr->content_size = handshake.content_size;
		handshake_ptr->send_period_ms = handshake.send_period_ms;
		unsigned char   *ptr = ((unsigned char *)handshake_ptr) + sizeof(Handshake);
		*ptr = (unsigned char)endpoint.endpoint.size();
		++ptr;
		strcpy((char *)ptr, endpoint.endpoint.c_str());
		net::UpdateHandshakeChecksums(handshake_ptr->hs);
		return handshake_request;
	}


	acs::Handler::stream_buffer create_handshake_reply(const Handshake&, const Endpoint& )
	{
		acs::Handler::stream_buffer handshake_reply;
		handshake_reply.resize(sizeof(HandshakeReply) + 1);
		HandshakeReply *handshake_reply_ptr = (HandshakeReply*)handshake_reply.data();
		handshake_reply_ptr->resultCode = 0x00;
		strncpy((char*)handshake_reply.data(), VS_Bwt_PrimaryField, sizeof(handshake_reply_ptr->hs.primary_field));
		//char server_name[512];
		//VS_GetServerName(server_name, sizeof(server_name));
		handshake_reply_ptr->hs.body_length = sizeof(handshake_reply_ptr->resultCode) - 1;
		//if (!our_name || strcmp(our_name, my_endpoint))	hr.resultCode = 1;
		net::UpdateHandshakeChecksums(handshake_reply_ptr->hs);
		return handshake_reply;
	}


	acs::Handler::stream_buffer create_test_frame(const Handshake& handshake)
	{
		acs::Handler::stream_buffer test_frame;
		test_frame.resize(handshake.content_size + sizeof(stream::FrameHeader));
		auto& head = *reinterpret_cast<stream::FrameHeader*>(test_frame.data());
		head.length = handshake.content_size;
		head.track = stream::Track::old_command;
		head.tick_count = 0;
		head.cksum = stream::GetFrameBodyChecksum(test_frame.data() + sizeof(stream::FrameHeader), test_frame.size());
		return test_frame;
	}

}
