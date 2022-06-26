#include "VS_WsChannelBase.h"
#include "std-generic/cpplib/SharedBuffer.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4355)
#endif
#include <websocketpp/frame.hpp>
#include <websocketpp/connection.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <algorithm>
#include <utility>

#include "../std/debuglog/VS_Debug.h"

#define DEBUG_CURRENT_MODULE VS_DM_FAKE_CLIENT

const size_t  ws::ChannelBase::c_max_frame_length = 0x0600;

void ws::ChannelBase::on_msg(websocketpp::server<websocketpp::config::core>* s, websocketpp::lib::weak_ptr<void> hdl, websocketpp::processor::hybi13<websocketpp::config::core>::message_ptr msg)
{
	switch (msg->get_opcode())
	{
	case websocketpp::frame::opcode::text:
		if (!ProcTextMsg(msg->get_payload().c_str(), msg->get_payload().size()))
			m_fireShutdown();
		break;
	case websocketpp::frame::opcode::binary:
		if (!ProcBinaryMsg(msg->get_payload().c_str(), msg->get_payload().size()))
			m_fireShutdown();
		break;
	default:
		m_fireShutdown();
	}
}

void ws::ChannelBase::on_close(websocketpp::lib::weak_ptr<void> hdl)
{
	m_closing = true;
	// check for error
	if (m_cptr->get_ec())
	{
		std::cout << m_cptr->get_ec().message() << std::endl;
		m_fireShutdown();
		return;
	}
	// send Close WebSocket frame
	{
		auto answer = GetOutgoingData();
		if (!answer.empty())
			m_fireSend(std::move(answer));
	}
}

ws::ChannelBase::ChannelBase(std::function<void()> &&fireShutdown, std::function<bool(vs::SharedBuffer &&)> &&fireSend)
	: m_ostr(&m_buffer)
	, m_fireShutdown(fireShutdown)
	, m_fireSend(fireSend)
{
	using namespace std::placeholders;

	//disable logging
	m_srv.clear_access_channels(websocketpp::log::alevel::all);
	m_srv.clear_error_channels(websocketpp::log::elevel::all);
	//set handlers
	m_srv.set_message_handler([this](websocketpp::lib::weak_ptr<void> hdl, websocketpp::processor::hybi13<websocketpp::config::core>::message_ptr msg) {on_msg(&m_srv, hdl, msg); });
	m_srv.set_open_handler([this](websocketpp::lib::weak_ptr<void> hdl) { processing_stream(hdl); });
	m_srv.set_close_handler([this](websocketpp::lib::weak_ptr<void> hdl) { on_close(hdl); });
	m_srv.set_ping_handler([this](websocketpp::lib::weak_ptr<void> hdl, std::string payload) { return on_ping(hdl, payload); });
	m_srv.set_pong_handler([this](websocketpp::lib::weak_ptr<void> hdl, std::string payload) { return on_pong(hdl, payload); });
	m_srv.set_fail_handler([this](websocketpp::lib::weak_ptr<void> hdl) { processing_stream(hdl); });

	m_srv.register_ostream(&m_ostr);
	m_srv.set_user_agent("");
	m_cptr = m_srv.get_connection();
	m_cptr->start();
}

void ws::ChannelBase::processing_stream(websocketpp::lib::weak_ptr<void> hdl)
{
	if (m_cptr->get_ec()) {
		dstream3 << "Websocket proc err: " << m_cptr->get_ec().message();
		m_fireShutdown();
		return;
	}

	auto answer = GetOutgoingData();
	if (!answer.empty())
		m_fireSend(std::move(answer));
}

bool ws::ChannelBase::on_ping(websocketpp::lib::weak_ptr<void> hdl, std::string payload)
{
	websocketpp::lib::error_code ec;
	m_srv.pong(hdl, payload, ec);
	if (ec) {
		dstream3 << "Websocket err: " << ec.message();
		return false;
	}
	processing_stream(hdl);
	return false;
}

void ws::ChannelBase::on_pong(websocketpp::lib::weak_ptr<void> hdl, std::string payload)
{
	Pong(payload);
}

vs::SharedBuffer ws::ChannelBase::GetOutgoingData()
{
	size_t data_size;
	auto data = m_buffer.release(data_size);
	return { std::move(data), data_size };
}

bool ws::ChannelBase::Send(const char* buf, const unsigned long size, websocketpp::frame::opcode::value op)
{
	websocketpp::lib::error_code ec;
	m_srv.send(m_cptr->get_handle(), buf, size, op, ec);
	if (ec) {
		dstream3 << "Websocket send err: " << ec.message();
		return false;
	}

	auto answer = GetOutgoingData();
	if (answer.empty())
		return false;

	for (size_t offset = 0; offset < answer.size(); offset += c_max_frame_length)
	{
		const size_t part_len = std::min(c_max_frame_length, answer.size() - offset);
		auto part = answer;
		part.shrink(offset, part_len);
		if (!m_fireSend(std::move(part)))
			return false;
	}
	return true;
}

bool ws::ChannelBase::SendTextMsg(const char* buf, const unsigned long size)
{
	return Send(buf, size, websocketpp::frame::opcode::text);
}

bool ws::ChannelBase::SendBinaryMsg(const void* buf, const unsigned long size)
{
	return Send((char*)buf, size, websocketpp::frame::opcode::binary);
}

bool ws::ChannelBase::Ping(std::string payload)
{
	auto hdl = m_cptr->get_handle();
	websocketpp::lib::error_code ec;
	m_srv.ping(hdl, payload, ec);
	if (ec) {
		dstream3 << "Websocket ping err: " << ec.message();
		return false;
	}
	processing_stream(hdl);
	return true;
}
