#pragma once

#include "VS_CheckSrvPack.h"
#include "net/EndpointRegistry_fwd.h"
#include "std/cpplib/VS_SimpleStr.h"

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include <thread>

/**
результат может быть следующим:
	1. не удалось приконнектиться
	2. по коннекшену пришли левые данные
	3. Соединение и хендшейк прошли успешно
*/

enum VS_CheckSrvResult
{
	e_chksrv_error				= 0,
	e_chksrv_connect_failed		= 1,
	e_chksrv_handshake_failed	= 2,
	e_chksrv_ok					= 3
};
class VS_CheckSrvResultCallBack
{
public:
	virtual bool Result(const unsigned id, const VS_CheckSrvResult ceck_srv_res,  VS_CheckSrvPack *res_pack) = 0;
};
/*
Connect, задавать, как подключаться по TCP или UDP
передавать, пожалуй, Co
*/
class VS_ClientCheckSrv
{
private:
	unsigned m_id;
	VS_CheckSrvResultCallBack *m_callback;
	std::unique_ptr<net::endpoint::ConnectTCP> m_check_reg_conn;
	unsigned m_padding_sz;
	std::unique_ptr<unsigned char[]> m_padding_buf;
	unsigned long m_timeout;
	std::thread	 m_thread;

	unsigned Thread();
public:
	VS_ClientCheckSrv();
	virtual ~VS_ClientCheckSrv();
	bool Init(std::unique_ptr<net::endpoint::ConnectTCP> reg_conn, VS_CheckSrvResultCallBack* callback, const unsigned check_id, const unsigned padding_sz, unsigned char* padding = 0);
	bool	AsyncCheck(const unsigned long mills);
};

class VS_ClientCheckSrvFast
{
public:
	typedef boost::signals2::signal<void (const VS_CheckSrvResult, uint32_t)> ResultSignalType;
	VS_ClientCheckSrvFast(std::unique_ptr<net::endpoint::ConnectTCP> reg_conn, const unsigned long mills, const unsigned padding_sz, unsigned char* padding = 0);
	boost::signals2::connection ConnectToSignal(const ResultSignalType::slot_type &slot)
	{
		return m_fireResult.connect(slot);
	}
	void Check();
private:
	ResultSignalType m_fireResult;
	std::unique_ptr<net::endpoint::ConnectTCP>	m_reg_conn;
	std::vector<unsigned char>					m_padding_buf;
	unsigned long								m_timeout;
};

class VS_LocatorCheck
{
	VS_SimpleStr	m_ep;

	void Result(VS_CheckSrvResult ceck_srv_res, unsigned long server_response_mills) {
		//dprint3("LocatorCheck of %s, res = %d, time = %3lu \n", m_ep.m_str, ceck_srv_res, server_response_mills);
		if (ceck_srv_res == e_chksrv_ok)
			m_res = server_response_mills;
	}
public:
	unsigned long m_res;
	VS_LocatorCheck(const char* ep) {
		m_ep = ep;
		m_res = -1;
	}
	void Run(unsigned long checktime);
};
