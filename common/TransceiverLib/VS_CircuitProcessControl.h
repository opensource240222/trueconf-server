#pragma once
#include "VS_RelayModule.h"
#include "std/cpplib/VS_ChildProcess.h"
#include "std-generic/asio_fwd.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <memory>
#include <chrono>

class VS_CircuitProcessControl : public VS_RelayModule, public std::enable_shared_from_this<VS_CircuitProcessControl>
{
	std::string							m_proc_name;
	std::string							m_transceiverName;
	std::function<int(std::string&)>	m_fireGetListeners;
	mutable std::unique_ptr<VS_ChildProcess> m_process;
	boost::asio::steady_timer timer;

	std::string m_serverAddress;
	bool						m_isStopped;
public:
	virtual ~VS_CircuitProcessControl();
	bool Start();
	void Stop();
	bool WaitStop();
	bool GetProcessName(std::string &name);
	const std::string& GetTransceiverName() const;
	void SheduleTimer(const std::chrono::milliseconds period = std::chrono::seconds(1));
	bool IsStarted() const;
protected:
	VS_CircuitProcessControl(const std::string &proc_name, boost::asio::io_service &ios, std::function<int(std::string &)> &&get_listeners);
	bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess) override;
	virtual void Timeout();
};
