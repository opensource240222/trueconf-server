#include "ThreadDecompressor.h"
#include "std/debuglog/VS_Debug.h"
#include "std/cpplib/ASIOThreadPool.h"
#include <atomic>

#define DEBUG_CURRENT_MODULE VS_DM_TRANSCEIVER

ThreadDecompressor::ThreadDecompressor() : BalancingModule(balancing_module::Type::decompressor),
										   m_localEvents(tde_size, false)
{
	m_receivers = 0;
}

ThreadDecompressor::~ThreadDecompressor()
{
	Stop();
}

int ThreadDecompressor::Init(atomicReceivers *receivers)
{
	m_numThreads = std::min<uint32_t>(64, std::thread::hardware_concurrency());
	m_decoderPool = std::make_unique<vs::ASIOThreadPoolBalancing>([this] (bool reg) -> void { (reg) ? RegisterThread(balancing_module::Thread::decoder) : UnregisterThread(); }, m_numThreads, "DecoderPool");
	m_receivers = receivers;
	return 0;
}

void ThreadDecompressor::Go()
{
	if (!m_localLooper.joinable())
	{
		m_localEvents.reset();
		m_localLooper = std::thread([this] () {
			vs::SetThreadName("Decompressor");
			RegisterThread(balancing_module::Thread::control);
			Loop();
			UnregisterThread();
		});
		vs::thread::SetPriority(m_localLooper, vs::thread::high);
		dprint4("Start ThreadDecompressor thread\n");
	}
}

void ThreadDecompressor::Stop()
{
	if (m_localLooper.joinable())
	{
		dprint4("Stop ThreadDecompressor thread\n");
		m_localEvents.kill_listener();
		m_localLooper.join();
	}
}

void ThreadDecompressor::Loop()
{
	int exit = 0;
	uint32_t waitRes;
	uint64_t lastCalcT(0);
	avmuxer::LoadStatistic load;
	std::map<std::string /* rcv id */, avmuxer::LoadStatisticItem> mli;
	vs::event synchEvent(false);
	m_decoderPool->Start();
	while (true) {

		waitRes = m_localEvents.wait_for(std::chrono::milliseconds(16), false);
		switch (waitRes)
		{
		case vs::wait_result::time_to_die:
			exit = 1;
			break;
		case tde_newData:
		case vs::wait_result::timeout:
			break;
		}
		if (exit)
			break;
		// read all
		auto rcvs = m_receivers->load();
		if (!rcvs->empty()) {
			std::atomic<uint64_t> vt(0);
			std::atomic<uint64_t> at(0);
			std::atomic_int counter(rcvs->size());
			for (auto & it : *rcvs) {
				if (mli.find(it.first) == mli.end()) {
					mli.emplace(it.first, avmuxer::LoadStatisticItem());
				}
			}
			for (auto & it : *rcvs) {
				m_decoderPool->get_io_service().post(
					[this, id = it.first, info = it.second, &synchEvent, &counter, &mli, &vt, &at] ()
					{
						auto &receiver = info->rcv;
						if (info->track == stream::Track::video) {
							/// read audio for only track video == Track::video
							auto ct = avmuxer::getTickMs();
							while (receiver->ReceiveAudio() == 0) {

							}
							auto dt = avmuxer::getTickMs() - ct;
							mli[id].at += dt;
							vt += dt;
						}
						if (info->type == stream::TrackType::video || info->type == stream::TrackType::slide) {
							/// read video
							int32_t ret(0);
							uint32_t packets(0);
							bool needKey(false);
							auto ct = avmuxer::getTickMs();
							while ((ret = receiver->ReceiveVideo(info->track, packets, needKey)) == 0) {

							}
							if (needKey && info->type == stream::TrackType::video) {
								fireKeyFrameRequest(id);
							}
							auto dt = avmuxer::getTickMs() - ct;
							mli[id].vt += dt;
							mli[id].vp = packets;
							at += dt;
						}
						if (counter.fetch_sub(1, std::memory_order_acq_rel) == 1) {
							synchEvent.set();
						}
					}
				);
			}
			synchEvent.wait();
			{
				load.li.at += at.load();
				load.li.vt += vt.load();
			}
			{
				/// load stat
				auto ct = avmuxer::getTickMs();
				if (lastCalcT == 0) {
					lastCalcT = ct;
				}
				load.dt = ct - lastCalcT;
				if (load.dt >= 2000) {
					load.dt *= m_numThreads;
					load.li.load = static_cast<float>(load.li.at + load.li.vt) / static_cast<float>(load.dt);
					for (auto &it : mli) {
						it.second.load = static_cast<float>(it.second.vt) / static_cast<float>(load.li.vt);
						load.mli.emplace_back(it.first, it.second);
						load.li.vp += it.second.vp;
					}
					auto cmp = [] (const std::pair<std::string, avmuxer::LoadStatisticItem> &a, const std::pair<std::string, avmuxer::LoadStatisticItem> &b)
					{
						return a.second.load > b.second.load;
					};
					std::sort(load.mli.begin(), load.mli.end(), cmp);
					fireUpdateLoadStatistic(load);
					mli.clear();
					load.Reset();
					lastCalcT = ct;
				}
			}
		}
	}
	m_decoderPool->Stop();
}
