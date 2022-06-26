#include "VS_RelayProc.h"

#include "rtc_base/ssladapter.h"
#include "std/cpplib/VS_TimeoutHandler.h"
#include "std/cpplib/MakeShared.h"
#include "std-generic/cpplib/scope_exit.h"
#include "std/debuglog/VS_Debug.h"
#include "TransceiverCircuit/VS_CommandReceiver.h"
#include "TransceiverCircuit/VS_ConfControlReceiver.h"
#include "TransceiverCircuit/VS_ConfRecorderModuleReceiver.h"
#include "TransceiverCircuit/VS_FrameReceiver.h"
#include "TransceiverCircuit/VS_MediaSourceCollection.h"
#include "TransceiverCircuit/VS_RTPModuleReceiver.h"
#include "TransceiverCircuit/VS_RTSPBroadcastModuleReceiver.h"
#include "TransceiverCircuit/VS_TransceiverPartsMgr.h"
#include "TransceiverCircuit/VS_WebRTCBroadcaster.h"
#include "TransceiverLib/VS_ProcessCtrlRelayMsg.h"
#include "TransceiverLib_v2/NetChannel.h"
#include "Transcoder/LoadBalancing/VS_LoadBalancer.h"

#include <boost/asio/steady_timer.hpp>
#include <chrono>

static const unsigned n_threads = std::max(4u, std::thread::hardware_concurrency());

class VS_DebugKeysReader final
	: public VS_TimeoutHandler
	, public std::enable_shared_from_this<VS_DebugKeysReader>
{
public:
	explicit VS_DebugKeysReader(boost::asio::io_service &ios);
	void Timeout() override;
	void SheduleTimer(const std::chrono::steady_clock::duration interval);
private:
	boost::asio::steady_timer	m_timer;
};

VS_DebugKeysReader::VS_DebugKeysReader(boost::asio::io_service &ios)
	:m_timer(ios)
{
}

void VS_DebugKeysReader::Timeout()
{
	VS_ReadDebugKeys();
}

void VS_DebugKeysReader::SheduleTimer(const std::chrono::steady_clock::duration interval)
{
	m_timer.expires_from_now(interval);
	m_timer.async_wait([self = this->shared_from_this(), interval](const boost::system::error_code& ec) {
		if (ec == boost::asio::error::operation_aborted)
			return;

		self->Timeout();
		self->SheduleTimer(interval);
	});
}

VS_RelayProc::VS_RelayProc(const char *serverEP,const char *addrs, std::string circuit_name, const unsigned char *secretData, const unsigned long sz)
	: VS_RelayModule(VS_ProcessCtrlRelayMsg::module_name)
	, m_circuit_name(std::move(circuit_name))
	, m_secretData(secretData, secretData + sz)
	, m_serverList(addrs)
	, m_serverEP(serverEP)
	, m_atp(n_threads)
	, m_stop_event(true)
{
}

void VS_RelayProc::Run()
{
	rtc::InitializeSSL();
	LoadBalancingHardware::GetLoadBalancing().SoftwareBenchmark();

	m_atp.Start();
	VS_SCOPE_EXIT {
		m_atp.get_io_service().stop(); // FIXME: Ideally we should be able stop without this.
		m_atp.Stop();
	};

	m_debug_keys_reader = std::make_shared<VS_DebugKeysReader>(m_atp.get_io_service());
	m_debug_keys_reader->SheduleTimer(std::chrono::seconds(4));

	m_partsMgr = std::make_shared<VS_TransceiverPartsMgr>(m_serverEP, m_serverList, m_atp.get_io_service());
	m_cmdReceiver = vs::MakeShared<VS_CommandReceiver>(m_serverList, m_circuit_name, &m_secretData[0], m_secretData.size(),
		m_atp.get_io_service());
	m_frameReceiver = std::make_shared<VS_FrameReceiver>(m_serverList, m_circuit_name, &m_secretData[0],m_secretData.size(), m_atp.get_io_service());

	m_confControlReceiver = std::make_shared<VS_ConfControlReceiver>();
	auto source_collection = boost::make_shared<VS_MediaSourceCollection>(m_frameReceiver, m_cmdReceiver, m_confControlReceiver);
	m_confControlReceiver->SetMediaSoreceCollection( source_collection );
	m_broadcaster = vs::MakeShared<VS_WebRTCBroadcaster>(m_frameReceiver, m_cmdReceiver, m_atp.get_io_service(), source_collection, m_partsMgr);
	m_confWriteModule = std::make_shared<VS_ConfRecorderModuleReceiver>(source_collection);
	m_RTSPBroadcastModule = std::make_shared<VS_RTSPBroadcastModuleReceiver>(source_collection);
	m_RTPModule = std::make_shared<VS_RTPModuleReceiver>(m_atp.get_io_service(), source_collection, m_partsMgr);

	m_cmdReceiver->ConnectToAllConfControl(m_frameReceiver);
	m_cmdReceiver->ConnectToAllConfControl(m_broadcaster);
	m_cmdReceiver->ConnectToAllConfControl(m_RTPModule);

	const bool started =
		true
		&& m_cmdReceiver->RegisterModule(m_broadcaster)
		&& m_cmdReceiver->RegisterModule(m_confWriteModule)
		&& m_cmdReceiver->RegisterModule(m_RTSPBroadcastModule)
		&& m_cmdReceiver->RegisterModule(m_RTPModule)
		&& m_cmdReceiver->RegisterModule(m_this_weak.lock())
		&& m_cmdReceiver->RegisterModule(m_confControlReceiver)
		&& m_cmdReceiver->ConnectToServer(
			[this]()
			{
				m_stop_event.set();
			}
		);
	if (!started)
		return;

	m_stop_event.wait();
	m_cmdReceiver->UnregisterModule(m_this_weak.lock());
	m_cmdReceiver->UnregisterModule(m_broadcaster);
	m_cmdReceiver->UnregisterModule(m_confWriteModule);
	m_cmdReceiver->UnregisterModule(m_RTPModule);
	m_cmdReceiver->UnregisterModule(m_RTSPBroadcastModule);
	m_cmdReceiver->DisconnectFromAllConfControl(m_frameReceiver);
	m_cmdReceiver->DisconnectFromAllConfControl(m_broadcaster);
	m_broadcaster->Close();
	m_partsMgr->StopAndWait();
	m_cmdReceiver->GetNetChannel()->StopActivity();
	m_frameReceiver->Stop();
	m_frameReceiver->WaitForStop();
}

bool VS_RelayProc::ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess)
{
	if(!mess)
		return false;
	boost::shared_ptr<VS_ProcessCtrlRelayMsg> proc_ctrl_mess(new VS_ProcessCtrlRelayMsg);
	if(proc_ctrl_mess->SetMessage(mess->GetMess()))
	{
		switch(proc_ctrl_mess->GetProcCtrlMessageType())
		{
		case VS_ProcessCtrlRelayMsg::e_stop:
			m_stop_event.set();
			return true;
		default:
			return false;
		}
	}
	else
		return false;

}
