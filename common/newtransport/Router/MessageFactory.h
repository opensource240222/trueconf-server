#pragma once

#include "../Const.h"
#include "../../transport/Message.h"

namespace transport {

inline Message CreatePing(string_view endpoint_id, bool is_server, bool is_authorized)
{
	const char opcode[] = { c_ping_opcode, '\0' };
	if (is_server)
		return Message::Make().SeqNumber(0xffffffff).AddString(opcode).TimeLimit(c_ping_time_limit).Body("", 1).DstServer(endpoint_id);
	else if (is_authorized)
		return Message::Make().SeqNumber(0xffffffff).AddString(opcode).TimeLimit(c_ping_time_limit).Body("", 1).DstUser(endpoint_id);
	else
		return Message::Make().SeqNumber(0xffffffff).AddString(opcode).TimeLimit(c_ping_time_limit).Body("", 1).DstCID(endpoint_id);
}

inline Message CreateDisconnect(string_view router_id, string_view endpoint_id, bool is_server, bool is_authorized)
{
	const char opcode[] = { c_disconnect_opcode, '\0' };
	if (is_server)
		return Message::Make().SeqNumber(0xffffffff).AddString(opcode).TimeLimit(c_disconnect_time_limit).Body("", 1).DstServer(endpoint_id);
	else if (!is_authorized)
		return Message::Make().SeqNumber(0xffffffff).AddString(opcode).TimeLimit(c_disconnect_time_limit).Body("", 1).DstUser(endpoint_id).SrcServer(router_id);
	else
		return Message::Make().SeqNumber(0xffffffff).AddString(opcode).TimeLimit(c_disconnect_time_limit).Body("", 1).DstCID(endpoint_id).SrcServer(router_id);
}

}
