#pragma once
#include "std/cpplib/json/reader.h"
#include "TransceiverLib/VS_TransceiverProxy.h"
#include "TransceiverLib/VS_RelayModule.h"
#include "FakeClient/VS_JsonRequestHandler.h"
#include "std-generic/cpplib/synchronized.h"

#include <boost/shared_ptr.hpp>
#include <boost/container/flat_map.hpp>

#include "std-generic/compat/memory.h"

namespace vs_relaymodule { class VS_StreamsRelay; }
namespace ts { struct IPool; }

class VS_WebrtcPeerConnectionService :
	public VS_RelayModule,
	public VS_JsonRequestHandler,
	public vs::enable_shared_from_this<VS_WebrtcPeerConnectionService>
{
public:
	~VS_WebrtcPeerConnectionService();

	bool Init();
	void Shutdown();

	virtual void ProcessJsonRequest(json::Object &obj) override;
	virtual bool ProcessingMessage(const boost::shared_ptr<VS_NetworkRelayMessageBase> &mess) override;

	virtual void DeleteWebPeerCallback(string_view peerId) override;
	virtual void RegisterWebPeerCallback(string_view peerId, const JsonCallback& cb) override;

protected:
	VS_WebrtcPeerConnectionService(const std::weak_ptr<ts::IPool> &pool);

private:
	vs::Synchronized<boost::container::flat_map<std::string, JsonCallback>> m_peers;
	std::shared_ptr<vs_relaymodule::VS_StreamsRelay> m_streamsRelay;
	std::weak_ptr<ts::IPool>	m_transcPool;
};
