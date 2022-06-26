#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <unordered_map>
#include <string>
#include <thread>
#include "std-generic/compat/memory.h"

#include "std-generic/cpplib/VS_Container.h"

#include <ldap.h>

namespace tc_ldap {

template<class Protocol = boost::asio::ip::tcp>
struct BindCtx {
	explicit BindCtx(typename Protocol::socket&& s)
		: sock(std::move(s))
		, sb(nullptr)
		, msgID(0)
	{
		auto rawSock = sock.native_handle();
		sb = ber_sockbuf_alloc();
		ber_sockbuf_ctrl(sb, LBER_SB_OPT_SET_FD, &rawSock);
		assert(sb != nullptr);
	}
	~BindCtx() {
		sock.close();
		if (sb) {
			ber_sockbuf_free(sb);
			sb = nullptr;
		}
	}
	BindCtx(const BindCtx<Protocol>&) = delete;
	BindCtx<Protocol>& operator=(const BindCtx<Protocol>& rval) = delete;
	BindCtx(BindCtx<Protocol>&& rval)
		: sock(std::move(rval.sock))
		, sb(rval.sb)
		, msgID(rval.msgID)
	{
		rval.sb = nullptr;
		rval.msgID = 0;
	}
	BindCtx<Protocol>& operator=(BindCtx<Protocol>&& rval) {
		~BindCtx();
		sock = std::move(rval.sock);
		sb = rval.sb;
		msgID = rval.msgID;
	}

	typename Protocol::socket sock;
	Sockbuf* sb;
	int msgID;
};

class NTLMAuthorizer : public vs::enable_shared_from_this<NTLMAuthorizer>
{
	boost::asio::io_service::strand m_strand;
	std::string m_ldap_hostname;
	std::string m_service;
	std::unordered_map<std::string, BindCtx<>> m_binders;	// can be more than 1000 users in domain => use unordered_map
#ifdef _DEBUG
	std::thread::id m_threadID;
#endif
	void CheckThread() const {
		assert(std::this_thread::get_id() == m_threadID);
	}

public:
	bool Init(const std::string& ldap_hostname, const std::string& service);
	int DoAuthorize(const std::string& epID, const void* inCrededinals, const size_t credSize, VS_Container& outCnt);
#ifdef _DEBUG
	void SetHomeThread() {
		m_threadID = std::this_thread::get_id();
	}
#endif

protected:
	explicit NTLMAuthorizer(boost::asio::io_service &ios)
		: m_strand(ios) {}
};

}