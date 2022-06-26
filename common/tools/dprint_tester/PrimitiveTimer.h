#pragma once

#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdint>

class PrimitiveTimer {
public:
	typedef std::function<void(void)> Callback;
public:
	PrimitiveTimer(void);
	virtual ~PrimitiveTimer(void);

	void SetCallback(const Callback &callback);
	void Start(uint32_t timeout_sec, bool once = false, uint32_t resolution_millis = 250);
	void Stop(void);
	bool IsRunning(void);
private:
	Callback m_cb;
	std::thread m_timer_thread;
	std::atomic_bool m_stop_flag;
	std::recursive_mutex m_lock;
};
