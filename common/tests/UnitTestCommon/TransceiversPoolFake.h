#pragma once

#include "TransceiverLib/VS_TransceiverProxy.h"
#include "TransceiverLib/TransceiversPoolInterface.h"
#include "std-generic/asio_fwd.h"

namespace test {
	struct TransceiversPoolFake final : public ts::IPool {
		std::shared_ptr<VS_TransceiverProxy> GetTransceiverProxy(const std::string& confId, bool createNewProxy) override {
			return m_proxy;
		}
		TransceiversPoolFake()
			: m_proxy(std::make_shared<VS_TransceiverProxy>(nullptr, nullptr, nullptr, nullptr, nullptr))
		{
			m_proxy->SetReturnProxyToPool([](const std::string&)->void{});
		}

		std::shared_ptr<VS_TransceiverProxy> ReserveProxy(std::string& OUT_reservationToken) override { assert(false); return nullptr; };
		bool ConnectReservedProxyToConference(const std::string& reservationToken, const std::string& confId)override { assert(false); return false; };
		void UnreserveProxy(const std::string& /*reservationToken*/) override { assert(false); }
		std::shared_ptr<VS_TransceiverProxy> m_proxy;
	};
}