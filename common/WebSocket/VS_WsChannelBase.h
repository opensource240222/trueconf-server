#pragma once

#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS_DISABLE
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS_DISABLE
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4355)
#endif
#if defined(_MSC_VER) && _MSC_VER < 1900
#define _WEBSOCKETPP_CPP11_THREAD_
#else
#define _WEBSOCKETPP_CPP11_STL_
#endif
#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#ifdef _SCL_SECURE_NO_WARNINGS_DISABLE
#undef _SCL_SECURE_NO_WARNINGS
#undef _SCL_SECURE_NO_WARNINGS_DISABLE
#endif

#ifdef _CRT_SECURE_NO_WARNINGS_DISABLE
#undef _CRT_SECURE_NO_WARNINGS
#undef _CRT_SECURE_NO_WARNINGS_DISABLE
#endif


#include "std/cpplib/ts_membuf.h"

#include <cstddef>
#include <functional>
#include <ostream>
#include <string>

namespace vs { class SharedBuffer; }

namespace ws {

class ChannelBase{
	vs::SharedBuffer GetOutgoingData();
	void processing_stream(websocketpp::lib::weak_ptr<void> hdl);
	void on_msg(websocketpp::server<websocketpp::config::core> *s, websocketpp::lib::weak_ptr<void> hdl, websocketpp::processor::hybi13<websocketpp::config::core>::message_ptr msg);
	bool on_ping(websocketpp::lib::weak_ptr<void> hdl, std::string payload);
	void on_pong(websocketpp::lib::weak_ptr<void> hdl, std::string payload);
	void on_close(websocketpp::lib::weak_ptr<void> hdl);

	static const size_t c_max_frame_length;
protected:
	bool m_was_handshake = false;
	bool m_closing = false;
	websocketpp::server<websocketpp::config::core> m_srv;
	websocketpp::server<websocketpp::config::core>::connection_ptr m_cptr;
private:
	vs::ts_membuf m_buffer;
	std::ostream m_ostr;

	std::function<void()> m_fireShutdown;
	std::function<bool(vs::SharedBuffer &&)> m_fireSend;
public:
	ChannelBase(std::function<void()> &&fireShutdown, std::function<bool(vs::SharedBuffer &&)> &&fireSend);
	virtual ~ChannelBase() {}

	virtual bool ProcTextMsg(const char* msg, unsigned long len) = 0;
	virtual bool ProcBinaryMsg(const void* msg, unsigned long len) = 0;
	virtual void Pong(std::string payload) {};

	bool Send(const char* buf, const unsigned long size, websocketpp::frame::opcode::value op);
	bool SendTextMsg(const char* buf, const unsigned long size);
	bool SendBinaryMsg(const void* buf, const unsigned long size);
	bool Ping(std::string payload);
};

}