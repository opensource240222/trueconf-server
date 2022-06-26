#pragma once

#include "std-generic/compat/memory.h"
#include <unordered_map>
#include <set>

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/coroutine.hpp>

#include "VS_TransceiverProxy.h"
#include "std/cpplib/event.h"
#include "std/cpplib/VS_Utils.h"
#include "std/VS_TransceiverInfo.h"
#include "std-generic/cpplib/scope_exit.h"
#include "TransceiverLib/TransceiverConfiguration.h"
#include "TransceiversPoolInterface.h"

#include "std/debuglog/VS_Debug.h"
#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

/* transceiver */
namespace ts {

	template<class Components>
	struct Proxy
	{
		Proxy() :transceiverConnected(true) {};
		Proxy(const Proxy&) = delete;
		Proxy& operator=(const Proxy& p) = delete;

		bool CanBeUsed(unsigned maxConfs) {
			if (!transceiverConnected.try_wait()) {
				dstream4 << "transceiver connected event is not set\n";
				return false;
			}
			if (currentConferences.size() >= maxConfs) {
				dstream4 << "transceiver has '" << currentConferences.size() << "' conferences, which is maximum\n";
				return false;
			}
			if (!reservationToken.empty()) {
				dstream4 << "transceiver is reserved for future use\n";
				return false;
			}
			return true;
		}

		Components proxyComponents;
		vs::event transceiverConnected;
		std::string reservationToken;
		std::set<std::string> currentConferences;
		std::chrono::steady_clock::time_point last_used_time;
	};

	template<class Components, class NetChannel, class FrameTransmit>
	class ProxiesPoolBase
		: public vs::enable_shared_from_this<ProxiesPoolBase<Components, NetChannel, FrameTransmit>>
		, public ts::IPool
	{
	protected:
		using ProxiesMap = std::unordered_map<std::string, std::weak_ptr<Proxy<Components> > >;
		using ProxiesVector = std::vector < std::shared_ptr<Proxy<Components> > >;
		struct Proxies {
			ProxiesVector proxies;
			ProxiesMap usedProxies;	// mirror for quick search
		};

		Proxies	m_proxies;
		unsigned m_minFreeProxyes;
		std::function<void(const std::string&)> onTransceiverReady;
		boost::asio::io_service&	m_ios;
	private:
		boost::asio::io_service::strand m_strand;
		boost::asio::steady_timer	m_timer;
		std::chrono::minutes		m_maxTransceiverUnusedDuration;
		unsigned					m_maxAllowedConfByTransceiver;

		std::shared_ptr<Proxy<Components> > GetFromReserveImp();
		std::shared_ptr<NetChannel> GetFreeNetChannelImp(string_view transceiverLogin);
		void OnNetChannelDieImp(string_view transceiverLogin);
		std::shared_ptr<FrameTransmit> GetFrameTransmitImp(const std::string &conf_id);
		std::shared_ptr<Proxy<Components> > InitNewTransceiverProxyImp(bool createNewProcess);
		void CreateAndWaitNewTransceiverImp(std::chrono::milliseconds waitTime,
			boost::asio::coroutine coro,
			const std::function<void(const std::shared_ptr<Proxy<Components> > &/*foundProxy*/)> &&cb,
			bool transceiverConnected = false,
			const std::shared_ptr<Proxy<Components> > &createdProxy = nullptr);
		void Timeout();
		void TimeoutImp();
		void IncreaseTransceiverProxies(Proxies& proxies);
		void DecreaseTransceiverProxies(Proxies& proxies);
		void FreeUnusedReservedProxies(Proxies& proxies);
		void StopImp();
		void GetTransceiverProxyImp(
			const std::string& confId,
			bool createNewProxy,
			boost::asio::coroutine coro,
			const std::function<void(const std::shared_ptr<VS_TransceiverProxy> &res)> &&cb,
			const std::shared_ptr<Proxy<Components> > &foundProxy = nullptr);
		std::shared_ptr<VS_TransceiverProxy> RestoreProxyByTransNameImp(const std::string& transName, const std::string& restoredConference);
		void ReserveProxyImp(boost::asio::coroutine coro,
			const std::function<void(const std::shared_ptr<VS_TransceiverProxy>& proxy, const std::string& reservationToken)> &&cb,
			const std::shared_ptr<Proxy<Components> > &foundProxy = nullptr);
		void UnreserveProxyImpl(const std::string& reservationToken);
		std::shared_ptr<VS_TransceiverProxy> GetReservedProxyImp(const std::string& reservationToken, const std::string& confId);
		void ReturnProxyToPool(const std::string& confName);
		void CreateAndWaitNewTransceiver(std::chrono::milliseconds waitTime,
			boost::asio::coroutine coro,
			const std::function<void(const std::shared_ptr<Proxy<Components> > &/*foundProxy*/)> &&cb,
			bool transceiverConnected = false,
			const std::shared_ptr<Proxy<Components> > &createdProxy = nullptr);
	protected:
		virtual bool InitProxyComponents(Components &components, bool createTransceiverProcess = true) = 0;
		std::shared_ptr<Proxy<Components> > GetFromReserve();
		std::shared_ptr<NetChannel> GetFreeNetChannel(string_view transceiverLogin);
		std::shared_ptr<FrameTransmit> GetFrameTransmit(const std::string &conf_id);
		bool InitNewTransceiverProxy(bool createNewProcess);
		void ScheduleTimer(const std::chrono::milliseconds period = std::chrono::seconds(1));

		ProxiesPoolBase(boost::asio::io_service &ios, unsigned minFreeProxyes, unsigned maxAllowedConfByTransceiver, std::chrono::minutes maxTransceiverUnusedDuration)
			: m_minFreeProxyes(minFreeProxyes)
			, m_ios(ios)
			, m_strand(ios)
			, m_timer(ios)
			, m_maxTransceiverUnusedDuration(maxTransceiverUnusedDuration)
			, m_maxAllowedConfByTransceiver(maxAllowedConfByTransceiver)
		{
			dstream2 << "ProxiesPool::ProxiesPool()\n" <<
				"\tminimum free transceiver proxies=" << m_minFreeProxyes << '\n' <<
				"\tmax confs by one transceiver=" << m_maxAllowedConfByTransceiver << '\n' <<
				"\tmax transceiver unused duration=" << m_maxTransceiverUnusedDuration.count() << " minutes\n";
			ts::DeleteTransceiversKeys();
		}

	public:
		ProxiesPoolBase(const ProxiesPoolBase&) = delete;
		ProxiesPoolBase& operator=(const ProxiesPoolBase&) = delete;
		virtual ~ProxiesPoolBase() {
			ts::DeleteTransceiversKeys();
		}

		void Stop();

		template<class F>
		void SetOnTransceiverReady(F && callback) {
			onTransceiverReady = std::forward<F>(callback);
		}

		std::shared_ptr<VS_TransceiverProxy> GetTransceiverProxy(const std::string& confId, bool createNewProxy = false) override;
		std::shared_ptr<VS_TransceiverProxy> RestoreProxyByTransName(const std::string& transName, const std::string& restoredConference);
		std::shared_ptr<VS_TransceiverProxy> ReserveProxy(std::string& OUT_reservationToken) override;
		void UnreserveProxy(const std::string& reservationToken) override;
		std::shared_ptr<VS_TransceiverProxy> GetReservedProxy(const std::string& reservationToken, const std::string& confId);
		void OnNetChannelDie(string_view transceiverLogin);
		bool ConnectReservedProxyToConference(const std::string& reservationToken, const std::string& confId) override
		{
			return GetReservedProxy(reservationToken, confId) != nullptr;
		}

	};

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<Proxy<Components> > ProxiesPoolBase<Components, NetChannel, FrameTransmit>::GetFromReserveImp()
	{
		auto& readyProxies = m_proxies.proxies;
		auto readyProxyIt = std::find_if(readyProxies.begin(), readyProxies.end(), [this](std::shared_ptr<Proxy<Components> > &p) {
			return p->CanBeUsed(m_maxAllowedConfByTransceiver);
		});
		if (readyProxyIt == readyProxies.end()) return nullptr;

		assert(*readyProxyIt != nullptr);
		dstream3 << "Getting proxy with transceiver='" << (*readyProxyIt)->proxyComponents.GetTransceiverName() << "' from reserve\n";
		return *readyProxyIt;
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<Proxy<Components> > ProxiesPoolBase<Components, NetChannel, FrameTransmit>::GetFromReserve()
	{
		std::shared_ptr<Proxy<Components> > res;
		vs::event done(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			res = GetFromReserveImp();
		});
		done.wait();
		return res;
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::ReturnProxyToPool(const std::string& confName)
	{
		m_strand.dispatch([this, w_this = this->weak_from_this(), confName]() {
			auto self = w_this.lock();
			if (!self) return;

			auto &usedProxies = m_proxies.usedProxies;
			auto it = usedProxies.find(confName);

			if (it == usedProxies.end()) return;
			auto pProxyObj = it->second.lock();
			if (!pProxyObj) return;
			dstream2 << "ProxiesPool::ReturnProxyToPool transceiver name = '" << pProxyObj->proxyComponents.GetTransceiverName() << "'\n";

			auto &currConfs = pProxyObj->currentConferences;
			assert(currConfs.find(confName) != currConfs.end());
			currConfs.erase(confName);
			dstream3 << "ProxiesPool::ReturnProxyToPool transceiver has '" << currConfs.size() << "' conferences on it.\n";

			pProxyObj->last_used_time = std::chrono::steady_clock::now();
			usedProxies.erase(it);
			dstream2 << "ProxiesPool::ReturnProxyToPool, proxies in reserve ='" << m_proxies.proxies.size() - usedProxies.size() << "' proxies in use='" << usedProxies.size() << "'\n";
		});
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<NetChannel> ProxiesPoolBase<Components, NetChannel, FrameTransmit>::GetFreeNetChannel(string_view transceiverLogin) {
		std::shared_ptr<NetChannel> res;
		vs::event done(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			res = GetFreeNetChannelImp(transceiverLogin);
		});
		done.wait();
		return res;
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<NetChannel> ProxiesPoolBase<Components, NetChannel, FrameTransmit>::GetFreeNetChannelImp(string_view transceiverLogin)
	{
		bool useLocalTransceiver = ts::UseLocalTransceiver();
		auto &proxies = m_proxies.proxies;
		auto it = std::find_if(proxies.begin(), proxies.end(), [transceiverLogin, useLocalTransceiver](const std::shared_ptr<Proxy<Components> > &p) -> bool {
			assert(p != nullptr);
			if (useLocalTransceiver) {
				if (!p->proxyComponents.HasCreatedTransceiver())
					return false;
				return p->proxyComponents.GetTransceiverName() == transceiverLogin;
			}

			// when use remote transceivers just take first free channel
			return p->transceiverConnected.try_wait() == false;
		});

		std::shared_ptr < Proxy<Components> > pProxy(nullptr);
		if (it == proxies.end()) {
			pProxy = InitNewTransceiverProxyImp(false);
			if (!pProxy) return nullptr;

			dstream2 << "ProxiesPool::GetFreeNetChannel push transceiver proxy to pool. Current pool size = '" << proxies.size() << "'\n";
		}
		else {
			pProxy = (*it);
			if (!pProxy) return nullptr;
		}

		pProxy->transceiverConnected.set();
		return pProxy->proxyComponents.GetNetChannel();
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::OnNetChannelDieImp(string_view transceiverLogin) {
		auto &proxies = m_proxies.proxies;
		auto it = std::find_if(proxies.begin(), proxies.end(), [transceiverLogin](const std::shared_ptr<Proxy<Components> > &p) -> bool {
			return p && (p->proxyComponents.GetTransceiverName() == transceiverLogin);
		});
		if (it == proxies.end())
			return;

		std::shared_ptr < Proxy<Components> > &pProxy(*it);
		if (!pProxy)
			return;
		pProxy->transceiverConnected.reset();
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<FrameTransmit> ProxiesPoolBase<Components, NetChannel, FrameTransmit>::GetFrameTransmit(const std::string &conf_id) {
		std::shared_ptr<FrameTransmit> res;
		vs::event done(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			res = GetFrameTransmitImp(conf_id);
		});
		done.wait();
		return res;
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<FrameTransmit> ProxiesPoolBase<Components, NetChannel, FrameTransmit>::GetFrameTransmitImp(const std::string &conf_id)
	{
		auto& usedProxies = m_proxies.usedProxies;
		auto it = usedProxies.find(conf_id);
		if (it == usedProxies.end()) {
			auto proxy = GetFromReserve();
			if (!proxy) return nullptr;
			it = usedProxies.emplace(conf_id, proxy).first;
		}

		auto pProxyObj = it->second.lock();
		if (!pProxyObj) return nullptr;

		return pProxyObj->proxyComponents.GetCircuitFrameTransmit();
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline bool ProxiesPoolBase<Components, NetChannel, FrameTransmit>::InitNewTransceiverProxy(bool createNewProcess) {
		bool res(false);
		vs::event done(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			res = InitNewTransceiverProxyImp(createNewProcess) != nullptr;
		});
		done.wait();
		return res;
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<Proxy<Components> > ProxiesPoolBase<Components, NetChannel, FrameTransmit>::InitNewTransceiverProxyImp(bool createNewProcess)
	{
		auto &proxies = m_proxies.proxies;
		auto limit = ts::GetMaxTransceivers();
		if (proxies.size() >= limit) {
			dstream2 << "Warning: ProxiesPool::InitNewTransceiverProxy. Number of transceivers reached limit='" << limit << "'!\n";
			return nullptr;
		}

		auto components = std::make_shared<Proxy<Components> >();
		if (!InitProxyComponents(components->proxyComponents, createNewProcess)) {
			dstream2 << "Failed to init proxy components\n";
			return nullptr;
		}
		proxies.emplace_back(std::move(components));
		auto proxyIt = proxies.end() - 1;
		(*proxyIt)->last_used_time = std::chrono::steady_clock::now();
		return *proxyIt;
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::CreateAndWaitNewTransceiver(std::chrono::milliseconds waitTime,
		boost::asio::coroutine coro,
		const std::function<void(const std::shared_ptr<Proxy<Components> > &/*foundProxy*/)> &&cb,
		bool transceiverConnected,
		const std::shared_ptr<Proxy<Components> > &createdProxy)
	{
		m_strand.dispatch([w_this = this->weak_from_this(), waitTime, handler = std::move(cb), coro, transceiverConnected, createdProxy]() {
			if(auto self = w_this.lock())
				self->CreateAndWaitNewTransceiverImp(waitTime, coro, std::move(handler), transceiverConnected, createdProxy);
		});
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::CreateAndWaitNewTransceiverImp(std::chrono::milliseconds waitTime,
		boost::asio::coroutine coro,
		const std::function<void(const std::shared_ptr<Proxy<Components> > &/*foundProxy*/)> &&cb,
		bool transceiverConnected,
		const std::shared_ptr<Proxy<Components> > &createdProxy)
	{
		auto &proxies = m_proxies.proxies;
#if defined(__clang__) // Clang has problems tracking origin of unannotated fallthrough in Boost.Coroutine marcos
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif
	BOOST_ASIO_CORO_REENTER(coro)
	{
		BOOST_ASIO_CORO_YIELD{
		auto pProxy = InitNewTransceiverProxyImp(true);
		if (!pProxy) {
			cb(nullptr);
			return;
		}

		m_ios.post([w_this = this->weak_from_this(), waitTime, cb, pProxy, coro]() {
			bool transceiverIsConnected = pProxy->transceiverConnected.wait_for(waitTime);
			if(auto self = w_this.lock())
				self->CreateAndWaitNewTransceiver(waitTime, coro, std::move(cb), transceiverIsConnected, pProxy);
		});
		}	// end of yield => on next call of this function we will begin from here

		// update iterator
		{auto it = std::find_if(proxies.begin(), proxies.end(), [createdProxy](const std::shared_ptr<Proxy<Components> >& proxy) {return proxy == createdProxy; });
		if (createdProxy == nullptr || it == proxies.end()) {
			cb(nullptr);
			return;
		}

		if (!transceiverConnected) {
			proxies.erase(it);
			cb(nullptr);
			return;
		}

		cb(*it);
		return;
		}
	}	// end of reenter
#if defined(__clang__)
#pragma GCC diagnostic pop
#endif
	}


	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::ScheduleTimer(const std::chrono::milliseconds period)
	{
		m_timer.expires_from_now(period);
		m_timer.async_wait([w_this = this->weak_from_this(), period](const boost::system::error_code& ec) {
			if (ec == boost::asio::error::operation_aborted)
				return;

			if (auto self = w_this.lock()) {
				self->Timeout();
				self->ScheduleTimer(period);
			}
		});
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::Timeout() {
		m_strand.dispatch([this]() {
			TimeoutImp();
		});
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::TimeoutImp()
	{
		auto freeProxies = m_proxies.proxies.size() - m_proxies.usedProxies.size();
		if (freeProxies > m_minFreeProxyes) {
			DecreaseTransceiverProxies(m_proxies);
		}
		else if (freeProxies < m_minFreeProxyes) {
			auto limit = ts::GetMaxTransceivers();
			if(m_proxies.proxies.size() < limit)
				IncreaseTransceiverProxies(m_proxies);
		}
		FreeUnusedReservedProxies(m_proxies);
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::IncreaseTransceiverProxies(Proxies& proxies)
	{
		auto &proxiesPool = proxies.proxies;
		auto& usedProxies = proxies.usedProxies;
		size_t freeProxies = proxiesPool.size() - usedProxies.size();

		while (freeProxies < m_minFreeProxyes) {
			if (!InitNewTransceiverProxyImp(true)) {
				dstream2 << "ProxiesPool failed to increase transceiver proxies count to " << m_minFreeProxyes << ", current pool size is " << proxiesPool.size();
				break;
			}

			freeProxies = proxiesPool.size() - usedProxies.size();
			dstream2 << "ProxiesPool::IncreaseTransceiverProxies pushed transceiver proxy to pool. Current pool size = '" << proxiesPool.size() << "'\n";
		}
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::DecreaseTransceiverProxies(Proxies& proxies)
	{
		auto &proxiesPool = proxies.proxies;
		auto& usedProxies = proxies.usedProxies;

		for (auto it = proxiesPool.begin(); it != proxiesPool.end();) {
			size_t freeProxies = proxiesPool.size() - usedProxies.size();
			if (freeProxies <= m_minFreeProxyes) break;

			if (!(*it)) {
				it = proxiesPool.erase(it);
				dstream2 << "ProxiesPool::DecreaseTransceiverProxies. Removed transceiver. Current pool size = '" << proxiesPool.size() << "'\n";
				continue;
			}

			auto &proxy = *it;
			if (proxy->last_used_time == std::chrono::steady_clock::time_point() ||
				(std::chrono::steady_clock::now() - proxy->last_used_time) > m_maxTransceiverUnusedDuration)
			{
				if (!proxy->currentConferences.empty()) {
					++it; continue;
				}

				it = proxiesPool.erase(it);
				dstream2 << "ProxiesPool::DecreaseTransceiverProxies. Removed transceiver. Current pool size = '" << proxiesPool.size() << "'\n";
				continue;
			}
			else {
				++it;
			}
		}
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ts::ProxiesPoolBase<Components, NetChannel, FrameTransmit>::FreeUnusedReservedProxies(Proxies & proxies)
	{
		auto &proxiesPool = proxies.proxies;
		auto& usedProxies = proxies.usedProxies;
		for (auto& pProxy : proxiesPool)
		{
			assert(pProxy != nullptr);
			if (pProxy->reservationToken.empty())
				continue;
			if (pProxy->currentConferences.empty() && (std::chrono::steady_clock::now() - pProxy->last_used_time) > m_maxTransceiverUnusedDuration)
				UnreserveProxyImpl(pProxy->reservationToken);
		}
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::Stop() {
		m_strand.dispatch([w_this = this->weak_from_this()]() {
			if(auto self = w_this.lock())
				self->StopImp();
		});
	}
	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::StopImp() {
		m_timer.cancel();

		m_proxies.usedProxies.clear();
		// When removing elements from m_proxies.proxies we need to ensure that
		// no Proxy object itself will be deleted in the process.
		// This is important because deletion of the Proxy object may
		// (indirectly) iterate over m_proxies.proxies and thus that container
		// must be in a consistent state.
		// To achieve that we first move all elements out of m_proxies.proxies.
		auto temp = std::move(m_proxies.proxies);
		m_proxies.proxies.clear();
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<VS_TransceiverProxy> ProxiesPoolBase<Components, NetChannel, FrameTransmit>::GetTransceiverProxy(const std::string& confId, bool createNewProxy)
	{
		std::shared_ptr<VS_TransceiverProxy> resProxy;
		vs::event done(true);
		m_strand.dispatch([w_this = this->weak_from_this(), &resProxy, confId, createNewProxy, &done]() {
			if (auto self = w_this.lock()) {
				self->GetTransceiverProxyImp(confId, createNewProxy, boost::asio::coroutine(),
					[&resProxy, &done](const std::shared_ptr<VS_TransceiverProxy> &res) {
					VS_SCOPE_EXIT{ done.set(); };
					resProxy = res;

				});
			}
			else {
				done.set();
			}
		});
		done.wait();
		return resProxy;
	}

	template<class Proxy, class ReturnToPool>
	bool InitProxyAsUsed(Proxy &pProxy, const std::string& confId, ReturnToPool && cb) {
		if (!pProxy) return false;

		auto proxy = pProxy->proxyComponents.GetTransceiverProxy();
		if (!proxy) return false;

		pProxy->currentConferences.emplace(confId);
		proxy->SetReturnProxyToPool(std::forward<ReturnToPool>(cb));
		return true;
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::GetTransceiverProxyImp(
		const std::string& confId,
		bool createNewProxy,
		boost::asio::coroutine coro,
		const std::function<void(const std::shared_ptr<VS_TransceiverProxy> &res)> &&cb,
		const std::shared_ptr<Proxy<Components> > &foundProxy)
	{
		std::shared_ptr<VS_TransceiverProxy> proxy;
		auto& usedProxies = m_proxies.usedProxies;
		std::shared_ptr<Proxy<Components> > proxyObj = foundProxy;
		typename decltype(m_proxies.usedProxies)::iterator it;
#if defined(__clang__) // Clang has problems tracking origin of unannotated fallthrough in Boost.Coroutine marcos
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif
	BOOST_ASIO_CORO_REENTER(coro)
	{

		dstream2 << "ProxiesPool::GetTransceiverProxy Getting transceiver proxy from pool for conference = '" << confId << "'\n";
		assert(proxyObj == nullptr);	// we must not appear here with ready proxy

		it = usedProxies.find(confId);
		if (it != usedProxies.end())
			proxyObj = it->second.lock();
		else if (it == usedProxies.end() && createNewProxy) {
			proxyObj = GetFromReserve();
			if (!proxyObj) BOOST_ASIO_CORO_YIELD
			{
				if (!ts::UseLocalTransceiver()) {
					cb(nullptr);
					return;
				}

				CreateAndWaitNewTransceiver(std::chrono::seconds(5), boost::asio::coroutine(),
					[w_this = this->weak_from_this(), handler = std::move(cb), confId, createNewProxy, coro](const std::shared_ptr<Proxy<Components> > &foundProxy)
				{
					if (!foundProxy) {
						handler(nullptr);
						return;
					}

					if (auto self = w_this.lock())
						self->GetTransceiverProxyImp(confId, createNewProxy, coro, std::move(handler), foundProxy);
				});
			} // end of yield => next time we will begin from here

			if (!m_proxies.usedProxies.emplace(confId, proxyObj).second) {
				cb(nullptr);
				return;
			}

			auto &&returnToPool = [w_this = this->weak_from_this()](const std::string& confName) {
				if (auto pool = w_this.lock())
					pool->ReturnProxyToPool(confName);
			};
			if (!InitProxyAsUsed(proxyObj, confId, std::move(returnToPool))) {
				cb(nullptr);
				return;
			}
		}

		if (!proxyObj) {
			cb(nullptr);
			return;
		}

		proxy = proxyObj->proxyComponents.GetTransceiverProxy();
		if (!proxy) {
			cb(nullptr);
			return;
		}

		dstream2 << "Found proxy with transceiver name = '" << proxy->GetTransceiverName() << "'\n";
		dstream3 << "Transceiver has '" << proxyObj->currentConferences.size() << "' conferences on it.\n";
		dstream2 << "Proxies in use='" << usedProxies.size() << "', proxies in reserve = '" << m_proxies.proxies.size() - usedProxies.size() << "'\n";

		proxyObj->last_used_time = std::chrono::steady_clock::now();
		cb(proxy);
		return;
	}	// end of reenter
#if defined(__clang__)
#pragma GCC diagnostic pop
#endif
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<VS_TransceiverProxy> ProxiesPoolBase<Components, NetChannel, FrameTransmit>::RestoreProxyByTransName(const std::string& transName, const std::string& restoredConference)
	{
		std::shared_ptr<VS_TransceiverProxy> res;
		vs::event done(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			res = RestoreProxyByTransNameImp(transName, restoredConference);
		});
		done.wait();
		return res;
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<VS_TransceiverProxy> ProxiesPoolBase<Components, NetChannel, FrameTransmit>::RestoreProxyByTransNameImp(const std::string& transName, const std::string& restoredConference) {
		dstream2 << "ProxiesPool::RestoreProxyByTransName transName='" << transName << "', restoredConference='" << restoredConference << "'\n";

		auto& usedProxies = m_proxies.usedProxies;
		auto it = usedProxies.find(restoredConference);
		if (it == usedProxies.end()) return nullptr;

		auto proxyObj = it->second.lock();
		if (!proxyObj) return nullptr;
		if (proxyObj->proxyComponents.GetTransceiverProxy()->GetTransceiverName() != transName) return nullptr;

		dstream2 << "ProxiesPool::RestoreProxyByTransName proxies in use='" << usedProxies.size() << " proxies in reserve ='" << m_proxies.proxies.size() - usedProxies.size() << "'\n";;

		proxyObj->last_used_time = std::chrono::steady_clock::now();
		return proxyObj->proxyComponents.GetTransceiverProxy();
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<VS_TransceiverProxy> ProxiesPoolBase<Components, NetChannel, FrameTransmit>::ReserveProxy(std::string& OUT_reservationToken) {
		std::shared_ptr<VS_TransceiverProxy> res;
		vs::event done(true);

		m_strand.dispatch([w_this = this->weak_from_this(), &res, &done, &OUT_reservationToken]() {
			if (auto self = w_this.lock()) {
				self->ReserveProxyImp(boost::asio::coroutine(),
				[&res, &done, &OUT_reservationToken](const std::shared_ptr<VS_TransceiverProxy>& proxy, const std::string& reservationToken)
				{
					VS_SCOPE_EXIT{ done.set(); };
					OUT_reservationToken = reservationToken;
					res = proxy;
				});
			}
		});
		done.wait();
		return res;

	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ts::ProxiesPoolBase<Components, NetChannel, FrameTransmit>::UnreserveProxy(const std::string & reservationToken)
	{
		m_strand.dispatch([w_this = this->weak_from_this(), reservationToken]() {
			if (auto self = w_this.lock())
				self->UnreserveProxyImpl(reservationToken);
		});
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ts::ProxiesPoolBase<Components, NetChannel, FrameTransmit>::UnreserveProxyImpl(const std::string & reservationToken)
	{
		assert(m_strand.running_in_this_thread());
		auto& usedProxies = m_proxies.usedProxies;

		auto it = usedProxies.find(reservationToken);
		if (it == usedProxies.end())
			return;
		auto proxyObj = it->second.lock();
		usedProxies.erase(it);

		if (proxyObj == nullptr)
			return;

		proxyObj->reservationToken.clear();
		proxyObj->last_used_time = std::chrono::steady_clock::now();
		dstream4 << "ts::Pool Proxy with token=" << reservationToken << " was unreserved\n";
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ProxiesPoolBase<Components, NetChannel, FrameTransmit>::ReserveProxyImp(
		boost::asio::coroutine coro,
		const std::function<void(const std::shared_ptr<VS_TransceiverProxy>& proxy,	const std::string& reservationToken)> &&cb,
		const std::shared_ptr<Proxy<Components> > &foundProxy)
	{
		auto& proxies = m_proxies.proxies;
		std::shared_ptr<Proxy<Components> > proxyObj = foundProxy;
		typename decltype(m_proxies.proxies)::iterator it;
#if defined(__clang__) // Clang has problems tracking origin of unannotated fallthrough in Boost.Coroutine marcos
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif
	BOOST_ASIO_CORO_REENTER(coro)
	{
		dstream2 << "ts::ProxiesPool try to reserve proxy\n";
		assert(proxyObj == nullptr);	// we must not appear here with ready proxy

		it = std::find_if(proxies.begin(), proxies.end(), [this](std::shared_ptr<Proxy<Components> > &p) -> bool {
			assert(p != nullptr);
			return p->CanBeUsed(m_maxAllowedConfByTransceiver);
		});

		if (it == proxies.end()) BOOST_ASIO_CORO_YIELD {
			if (!ts::UseLocalTransceiver()) {
				cb(nullptr, "");
				return;
			}

			CreateAndWaitNewTransceiver(std::chrono::seconds(5), boost::asio::coroutine(),
				[w_this = this->weak_from_this(), handler = std::move(cb), coro](const std::shared_ptr<Proxy<Components> > &foundProxy)
			{
				if (!foundProxy){
					handler(nullptr, "");
					return;
				}

				if (auto self = w_this.lock())
					self->ReserveProxyImp(coro, std::move(handler), foundProxy);
			});
		}	// end of yield => next time we will begin from here

		proxyObj = *it;
		if (!proxyObj) {
			cb(nullptr, "");
			return;
		}

		const unsigned int c_MD5_HASH_SIZE = 32;
		char token[c_MD5_HASH_SIZE + 1] = {};
		VS_GenKeyByMD5(token);

		const std::string OUT_reservationToken = proxyObj->reservationToken = std::string(token, token + c_MD5_HASH_SIZE);
		m_proxies.usedProxies.emplace(OUT_reservationToken, proxyObj);

		auto proxy = proxyObj->proxyComponents.GetTransceiverProxy();
		if (!proxy) {
			cb(nullptr, "");
			return;
		}

		proxy->SetReturnProxyToPool([w_this = this->weak_from_this()](const std::string& confName) {
			if(auto self = w_this.lock())
				self->ReturnProxyToPool(confName);
		});

		dstream2 << "ts::ProxiesPool proxy reserved succesfully. Reservation token = '" << OUT_reservationToken << "'\n";
		proxyObj->last_used_time = std::chrono::steady_clock::now();

		cb(proxy, OUT_reservationToken);
		return;
	}	//	end of reenter
#if defined(__clang__)
#pragma GCC diagnostic pop
#endif
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<VS_TransceiverProxy> ProxiesPoolBase<Components, NetChannel, FrameTransmit>::GetReservedProxy(const std::string& reservationToken, const std::string& confId) {
		std::shared_ptr<VS_TransceiverProxy> res;
		vs::event done(true);
		m_strand.dispatch([&]() {
			VS_SCOPE_EXIT{ done.set(); };
			res = GetReservedProxyImp(reservationToken, confId);
		});
		done.wait();
		return res;
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline void ts::ProxiesPoolBase<Components, NetChannel, FrameTransmit>::OnNetChannelDie(string_view transceiverLogin)
	{
		m_strand.dispatch([this, w_this = this->weak_from_this(), login = std::string(transceiverLogin)]() {
			if (auto self = w_this.lock())
				self->OnNetChannelDieImp(login);
		});
	}

	template<class Components, class NetChannel, class FrameTransmit>
	inline std::shared_ptr<VS_TransceiverProxy> ProxiesPoolBase<Components, NetChannel, FrameTransmit>::GetReservedProxyImp(const std::string& reservationToken, const std::string& confId)
	{
		dstream2 << "ts::ProxiesPool getting reserved proxy for conference = '" << confId << "'. Reservation token = '" << reservationToken << "'\n";

		auto& usedProxies = m_proxies.usedProxies;
		auto it = usedProxies.find(reservationToken);
		if (it == usedProxies.end()) return nullptr;

		auto proxyObj = it->second.lock();
		if (proxyObj == nullptr) {
			usedProxies.erase(it);
			return nullptr;
		}

		proxyObj->reservationToken.clear();
		auto newIt = usedProxies.emplace(confId, proxyObj).first;
		usedProxies.erase(it);
		dstream2 << "ts::ProxiesPool token='" << reservationToken << "' changed to confID=" << confId;

		auto& currConfs = proxyObj->currentConferences;
		currConfs.erase(reservationToken);
		currConfs.emplace(confId);

		proxyObj->last_used_time = std::chrono::steady_clock::now();
		return proxyObj->proxyComponents.GetTransceiverProxy();

		return nullptr;
	}
}

#undef DEBUG_CURRENT_MODULE
