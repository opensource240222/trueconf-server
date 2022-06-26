#include "VS_CircuitProcessControl.h"
#include "std/cpplib/VS_Utils.h"
#include "std/cpplib/VS_WideStr.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "tools/Server/VS_Server.h"
#include "VS_ProcessCtrlRelayMsg.h"
#include "std/VS_TransceiverInfo.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "VS_TransceiverAuthenticator.h"
#include "std/cpplib/VS_Replace.h"
#include "std/Globals.h"

#include <string>
#include <array>

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

VS_CircuitProcessControl::VS_CircuitProcessControl(const std::string& proc_name, boost::asio::io_service& ios, std::function<int(std::string&)> && get_listeners)
	: VS_RelayModule(VS_ProcessCtrlRelayMsg::module_name)
	, m_proc_name(proc_name)
	, m_fireGetListeners(std::move(get_listeners))
	, m_process(std::make_unique<VS_ChildProcess>())
	, timer(ios)
	, m_isStopped(false)
{
}

VS_CircuitProcessControl::~VS_CircuitProcessControl()
{
}

template<class Container>
std::string MakeCmdLine(const Container& m) {
	std::string res;

	for (const auto& it : m) {
		res += " --"; res += it.first;	// option name
		res += "="; res += it.second;	// option value
	}
	return res;
}

bool VS_CircuitProcessControl::Start()
{
	m_serverAddress.clear();
	if( m_proc_name.empty() || !m_fireGetListeners || IsStarted() || m_isStopped || 0 >= m_fireGetListeners(m_serverAddress) )
		return false;

	const auto exe_dir = VS_GetExecutableDirectory();
	if (exe_dir.empty())
	{
		return false;
	}

	const unsigned int c_MD5_HASH_SIZE = 32;
	if (m_transceiverName.empty()) {
		char unique_tranceiver_name[c_MD5_HASH_SIZE + 1] = { 0 };
		VS_GenKeyByMD5(unique_tranceiver_name);
		m_transceiverName = std::string(unique_tranceiver_name, unique_tranceiver_name + c_MD5_HASH_SIZE);
	}

	if (auth::Transceiver::ReadSharedKey(m_transceiverName).empty()) {
		char shared_key[c_MD5_HASH_SIZE + 1] = { 0 };
		VS_GenKeyByMD5(shared_key);
		auth::Transceiver::SaveSharedKey(m_transceiverName, shared_key);
	}

	if (g_tr_endpoint_name.empty()) {
		dstream0 << "VS_CircuitProcessControl(" << m_transceiverName << "): can't start transceiver process, server name is not set";
		return false;
	}

	const std::array<std::pair<std::string /*name*/, std::string /*value*/>, 7> cmd_options = {
		std::make_pair(ts::NAME_OPTION_TAG, m_transceiverName),
		std::make_pair(ts::REGISTRY_BACKEND_OPTION_TAG, VS_RegistryKey::GetDefaultBackendConfiguration()),
		std::make_pair(ts::ROOT_OPTION_TAG, VS_RegistryKey::GetDefaultRoot() ),
		std::make_pair(ts::SERVER_ADDRESS_OPTION_TAG, m_serverAddress),
		std::make_pair(ts::SERVER_ENDPOINT_OPTION_TAG, g_tr_endpoint_name),
		std::make_pair(ts::DEBUG_OPTION_TAG, std::to_string(VS_GetDebugLevel())),
		std::make_pair(ts::LOG_DIRECTORY_OPTION_TAG, vs::GetLogDirectory()),
	};

	std::string cmd_line = "\"";
	cmd_line += exe_dir;
	cmd_line += m_proc_name;
	cmd_line += "\" ";
	cmd_line += MakeCmdLine(cmd_options);

#ifndef _WIN32
	VS_ReplaceAll(cmd_line, "\\", "\\\\");
#endif // !_WIN32


	if (!IsStarted())
	{
		m_process = std::make_unique<VS_ChildProcess>();
	}
	else
	{
		return true;
	}

	dstream3 << "VS_CircuitProcessControl(" << m_transceiverName << "): starting: " << cmd_line;
	auto res = m_process->Start(cmd_line.c_str());
	if (res)
		dstream3 << "VS_CircuitProcessControl(" << m_transceiverName << "): started, pid=" << m_process->GetPID();
	else
		dstream0 << "VS_CircuitProcessControl(" << m_transceiverName << "): start failed";
	return res;
}

void VS_CircuitProcessControl::Stop()
{
	dstream3 << "VS_CircuitProcessControl(" << m_transceiverName << "): stopping, pid=" << m_process->GetPID();
	boost::shared_ptr<VS_ProcessCtrlRelayMsg> msg(new VS_ProcessCtrlRelayMsg);
	msg->MakeStop();
	m_isStopped = SendMsg(msg);
}
bool VS_CircuitProcessControl::WaitStop()
{
	if(!m_isStopped)
		return false;

	if (m_process->Wait(std::chrono::seconds(5)))
	{
		int exit_code;
		if (m_process->GetExitCode(exit_code) && exit_code != 0)
			dstream0 << "VS_CircuitProcessControl(" << m_transceiverName << "): stopped unexpectedly, exit_code=" << exit_code;
		else
			dstream3 << "VS_CircuitProcessControl(" << m_transceiverName << "): stopped";
		return true;
	}

	dstream0 << "VS_CircuitProcessControl(" << m_transceiverName << "): stop is taking too long, terminating the process, pid=" << m_process->GetPID();
#if defined(_DEBUG)
	// Wait for termination indefinitely in debug builds.
	// Transceiver may be actively debugged and we don't want to interrupt that.
	m_process->Wait();
#endif
	m_process->Terminate(-1);
	return true;
}

bool VS_CircuitProcessControl::IsStarted() const
{
	return m_process->Alive();
}
bool VS_CircuitProcessControl::ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess)
{
	return false;
}

void VS_CircuitProcessControl::Timeout()
{
	if(m_isStopped)
		return;

	if (!m_process->Alive())
	{
		int exit_code;
		if (m_process->GetExitCode(exit_code) && exit_code != 0)
			dstream0 << "VS_CircuitProcessControl(" << m_transceiverName << "): stopped unexpectedly, exit_code=" << exit_code;
		Start();
	}
}

bool VS_CircuitProcessControl::GetProcessName(std::string &name)
{
	name = m_proc_name;
	return true;
}

const std::string& VS_CircuitProcessControl::GetTransceiverName() const
{
	return m_transceiverName;
}

void VS_CircuitProcessControl::SheduleTimer(const std::chrono::milliseconds period)
{
	timer.expires_from_now(period);
	timer.async_wait([self = shared_from_this(), period](const boost::system::error_code& ec) {
		if (ec == boost::asio::error::operation_aborted)
			return;

		self->Timeout();
		self->SheduleTimer(period);
	});
}

#undef DEBUG_CURRENT_MODULE
