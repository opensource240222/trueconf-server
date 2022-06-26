#pragma once

#include "VS_BFCPTypes.h"
#include "VS_BFCPMessage_fwd.h"
#include "std-generic/compat/condition_variable.h"

#include <boost/signals2/signal.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/circular_buffer.hpp>

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <queue>
#include <vector>

#include "std-generic/undef_windows.h" // this should be last

namespace bfcp {

class SessionBase
{
public:
	virtual ~SessionBase();

	bool SetRecvData(const void* p, size_t size);
	bool GetSendData(void* p, size_t& size);
	virtual void Tick() {}

protected:
	explicit SessionBase(bool udp);

	virtual void HandleMessage(std::unique_ptr<Message>&& msg) = 0;
	void SendMessage(std::unique_ptr<Message>&& msg);

protected:
	const bool m_udp;
	std::mutex m_mutex;

private:
	boost::circular_buffer<uint8_t> m_queue_in;
	std::queue<std::unique_ptr<Message>> m_queue_out;
};

class ClientSession : public SessionBase
{
public:
	ClientSession(ConferenceID conf_id, UserID user_id, bool udp);

	ConferenceID GetConferenceID() const
	{
		return m_conf_id;
	}

	UserID GetUserID() const
	{
		return m_user_id;
	}

	void RequestFloor(FloorID floor_id);
	void RequestFloor_sync(FloorID floor_id, RequestStatus& status, std::chrono::steady_clock::duration timeout);
	void RequestFloor_sync(FloorID floor_id, RequestStatus& status, std::chrono::steady_clock::time_point expire_time);

	bool ReleaseFloor(FloorID floor_id);
	bool ReleaseFloor_sync(FloorID floor_id, bool& success, std::chrono::steady_clock::duration timeout);
	bool ReleaseFloor_sync(FloorID floor_id, bool& success, std::chrono::steady_clock::time_point expire_time);

	void SubscribeToFloorStatus(std::initializer_list<FloorID> ids);
	void UnsubscribeToFloorStatus(std::initializer_list<FloorID> ids);

	typedef boost::signals2::signal<void(FloorID floor_id, RequestStatus status)> FloorRequestStatusSignalType;
	boost::signals2::connection ConnectToFloorRequestStatus(const FloorRequestStatusSignalType::slot_type& slot)
	{
		return m_signal_FloorRequestStatus.connect(slot);
	}

	typedef boost::signals2::signal<void(FloorID floor_id, RequestStatus status, UserID user_id)> FloorStatusSignalType;
	boost::signals2::connection ConnectToFloorStatus(const FloorStatusSignalType::slot_type& slot)
	{
		return m_signal_FloorStatus.connect(slot);
	}

private:
	void RequestFloor_impl(FloorID floor_id);
	bool ReleaseFloor_impl(FloorID floor_id);
	void SendFloorQuery();

	void HandleMessage(std::unique_ptr<Message>&& msg) override;

private:
	const ConferenceID m_conf_id;
	const UserID m_user_id;

	TransactionID m_last_transaction_id;
	struct FloorState
	{
		FloorState() : request_id(0) {}
		FloorRequestID request_id;
	};
	std::map<FloorID, FloorState> m_floors;
	std::set<FloorID> m_subscribed_floors;

	vs::condition_variable m_floor_rr_cv;
	struct FloorRRState // Floor request or release state
	{
		FloorRequestID request_id;
		TransactionID transaction_id;
		FloorID floor_id;
		RequestStatus status;
	};
	typedef std::vector<FloorRRState> floor_rrs_t; //TODO: boost::multi_index
	floor_rrs_t m_floor_rrs;

	FloorRequestStatusSignalType m_signal_FloorRequestStatus;
	FloorStatusSignalType m_signal_FloorStatus;
};

class ServerSession : public SessionBase
{
public:
	ServerSession(ConferenceID conf_id, bool udp);

	ConferenceID GetConferenceID() const
	{
		return m_conf_id;
	}

	bool SendStatusForAllFloors();
	void SendStatusForAllFloors(bool value);
	std::chrono::steady_clock::duration GetFloorStatusPeriod();
	void SetFloorStatusPeriod(std::chrono::steady_clock::duration period);
	void AddUser(UserID user_id);
	void RemoveUser(UserID user_id);

	RequestStatus RequestFloor(FloorID floor_id, UserID user_id);
	RequestStatus ReleaseFloor(FloorID floor_id, UserID user_id);

	typedef boost::signals2::signal<void(FloorID floor_id, RequestStatus status, UserID user_id)> FloorStatusChangeSignalType;
	boost::signals2::connection ConnectToFloorStatusChange(const FloorStatusChangeSignalType::slot_type& slot)
	{
		return m_signal_FloorStatusChange.connect(slot);
	}

	void Tick() override;

private:
	RequestStatus RequestFloor_impl(FloorID floor_id, UserID user_id);
	RequestStatus ReleaseFloor_impl(FloorID floor_id, UserID user_id);
	void SendFloorStatus(FloorID floor_id, UserID user_id, TransactionID transaction_id, bool only_granted = true);
	void DoDeferredActions(std::unique_lock<std::mutex>&& lock);

	void HandleMessage(std::unique_ptr<Message>&& msg) override;

private:
	const ConferenceID m_conf_id;

	bool m_send_status_for_all_floors;
	std::chrono::steady_clock::duration m_floor_status_period;
	std::chrono::steady_clock::time_point m_last_floor_status_time;
	FloorRequestID m_last_request_id;
	struct FloorState
	{
		FloorState() : granted(false), user_id(0), request_id(0) {}
		bool granted;
		UserID user_id;
		FloorRequestID request_id;
	};
	std::map<FloorID, FloorState> m_floors;
	std::map<FloorRequestID, FloorID> m_floor_requests;

	struct UserState
	{
		std::set<FloorID> queried_floors;
	};
	std::map<UserID, UserState> m_users;

	FloorStatusChangeSignalType m_signal_FloorStatusChange;
	std::vector<std::function<void()>> m_deferred_actions;
};

}
