#include "VS_BFCPSession.h"
#include "VS_BFCPAttribute.h"
#include "VS_BFCPMessage.h"
#include "std-generic/compat/memory.h"

namespace bfcp {

SessionBase::SessionBase(bool udp)
	: m_udp(udp)
	, m_queue_in(8*1024)
{
}

SessionBase::~SessionBase()
{
}

bool SessionBase::SetRecvData(const void* p, size_t size)
{
	Tick();

	if (!p || size == 0)
		return true;

	std::unique_lock<std::mutex> lock(m_mutex);
	m_queue_in.insert(m_queue_in.end(), reinterpret_cast<const char*>(p), reinterpret_cast<const char*>(p)+size);

	while (!m_queue_in.empty())
	{
		size_t msg_size = m_queue_in.size();
		DecodeStatus status;
		auto msg = Message::DecodeAny(m_queue_in.linearize(), msg_size, status);
		switch (status.type)
		{
		case DecodeStatus::Success:
			m_queue_in.erase_begin(std::min(m_queue_in.size(), msg_size));
			lock.unlock();
			HandleMessage(std::move(msg));
			lock.lock();
			break;
		case DecodeStatus::PartialData:
			return true;
		case DecodeStatus::MalformedData:
		case DecodeStatus::UnknownVersion:
			m_queue_in.erase_begin(std::min(m_queue_in.size(), msg_size));
			if (msg_size == 0)
			{
				m_queue_in.clear();
				return false;
			}
			break;
		case DecodeStatus::UnknownMandatoryAttribute:
		{
			m_queue_in.erase_begin(std::min(m_queue_in.size(), msg_size));
			auto attr_ErrorCode = vs::make_unique<Attribute_ERROR_CODE>(4);
			attr_ErrorCode->AddUnknownAttribute(status.attribute_type);
			SendMessage(vs::make_unique<Message_Error>(1, 0, 0, 0, std::move(attr_ErrorCode)));
			if (msg_size == 0)
			{
				m_queue_in.clear();
				return false;
			}
		}
			break;
		case DecodeStatus::UnknownPrimitive:
			m_queue_in.erase_begin(std::min(m_queue_in.size(), msg_size));
			SendMessage(vs::make_unique<Message_Error>(1, 0, 0, 0, vs::make_unique<Attribute_ERROR_CODE>(3)));
			if (msg_size == 0)
			{
				m_queue_in.clear();
				return false;
			}
			break;
		default:
			return false;
		}
	}
	return true;
}

bool SessionBase::GetSendData(void* p, size_t& size)
{
	Tick();

	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_queue_out.empty())
	{
		size = 0;
		return true;
	}
	size_t msg_size = 0;
	m_queue_out.front()->Encode(nullptr, msg_size);
	if (!p || size < msg_size)
	{
		size = msg_size;
		return false;
	}

	if (!m_queue_out.front()->Encode(p, msg_size))
	{
		size = 0;
		return false;
	}
	m_queue_out.pop();
	size = msg_size;
	return true;
}

void SessionBase::SendMessage(std::unique_ptr<Message>&& msg)
{
	m_queue_out.emplace(std::move(msg));
}

ClientSession::ClientSession(ConferenceID conf_id, UserID user_id, bool udp)
	: SessionBase(udp)
	, m_conf_id(conf_id)
	, m_user_id(user_id)
	, m_last_transaction_id(0)
{
	if (m_udp)
		SendMessage(vs::make_unique<Message_Hello>(1, m_conf_id, ++m_last_transaction_id, m_user_id));
}

void ClientSession::RequestFloor_impl(FloorID floor_id)
{
	SendMessage(vs::make_unique<Message_FloorRequest>(1, m_conf_id, ++m_last_transaction_id, m_user_id,
		vs::make_unique<Attribute_FLOOR_ID>(floor_id)
	));
}

void ClientSession::RequestFloor(FloorID floor_id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	RequestFloor_impl(floor_id);
}

void ClientSession::RequestFloor_sync(FloorID floor_id, RequestStatus& status, std::chrono::steady_clock::duration timeout)
{
	return RequestFloor_sync(floor_id, status, std::chrono::steady_clock::now() + timeout);
}

void ClientSession::RequestFloor_sync(FloorID floor_id, RequestStatus& status, std::chrono::steady_clock::time_point expire_time)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	RequestFloor_impl(floor_id);
	auto transaction_id = m_last_transaction_id;

	while (true)
	{
		auto it = std::find_if(m_floor_rrs.begin(), m_floor_rrs.end(), [floor_id, transaction_id](const FloorRRState& x) {
			return x.transaction_id == transaction_id && x.floor_id == floor_id;
		});
		if (it != m_floor_rrs.end())
		{
			status = it->status;
			m_floor_rrs.erase(it);
			return;
		}
		if (m_floor_rr_cv.wait_until(lock, expire_time) == std::cv_status::timeout)
		{
			status = RequestStatus::Denied;
			return;
		}
	}
}

bool ClientSession::ReleaseFloor_impl(FloorID floor_id)
{
	auto f_it = m_floors.find(floor_id);
	if (f_it == m_floors.end())
		return false;

	SendMessage(vs::make_unique<Message_FloorRelease>(1, m_conf_id, ++m_last_transaction_id, m_user_id,
		vs::make_unique<Attribute_FLOOR_REQUEST_ID>(f_it->second.request_id)
	));
	return true;
}

bool ClientSession::ReleaseFloor(FloorID floor_id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return ReleaseFloor_impl(floor_id);
}

bool ClientSession::ReleaseFloor_sync(FloorID floor_id, bool& success, std::chrono::steady_clock::duration timeout)
{
	return ReleaseFloor_sync(floor_id, success, std::chrono::steady_clock::now() + timeout);
}

bool ClientSession::ReleaseFloor_sync(FloorID floor_id, bool& success, std::chrono::steady_clock::time_point expire_time)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (!ReleaseFloor_impl(floor_id))
		return false;
	auto transaction_id = m_last_transaction_id;

	while (true)
	{
		auto it = std::find_if(m_floor_rrs.begin(), m_floor_rrs.end(), [floor_id, transaction_id](const FloorRRState& x) {
			return x.transaction_id == transaction_id && x.floor_id == floor_id;
		});
		if (it != m_floor_rrs.end())
		{
			success = it->status == RequestStatus::Released || it->status == RequestStatus::Cancelled;
			m_floor_rrs.erase(it);
			return true;
		}
		if (m_floor_rr_cv.wait_until(lock, expire_time) == std::cv_status::timeout)
		{
			success = false;
			return true;
		}
	}
}

void ClientSession::SubscribeToFloorStatus(std::initializer_list<FloorID> ids)
{
	auto initial_size = m_subscribed_floors.size();
	m_subscribed_floors.insert(ids);
	if (m_subscribed_floors.size() != initial_size)
		SendFloorQuery();
}

void ClientSession::UnsubscribeToFloorStatus(std::initializer_list<FloorID> ids)
{
	auto initial_size = m_subscribed_floors.size();
	for (auto id: ids)
		m_subscribed_floors.erase(id);
	if (m_subscribed_floors.size() != initial_size)
		SendFloorQuery();
}

void ClientSession::SendFloorQuery()
{
	auto msg = vs::make_unique<Message_FloorQuery>(1, m_conf_id, ++m_last_transaction_id, m_user_id);
	for (auto id: m_subscribed_floors)
		msg->attrs.push_back(vs::make_unique<Attribute_FLOOR_ID>(id));
	SendMessage(std::move(msg));
}

void ClientSession::HandleMessage(std::unique_ptr<Message>&& msg_)
{
#if 0
	if (msg_->conference_id != m_conf_id)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		SendMessage(vs::make_unique<Message_Error>(msg_->version, msg_->conference_id, msg_->transaction_id, msg_->user_id,
			vs::make_unique<Attribute_ERROR_CODE(1)
		));
		return;
	}
	if (msg_->user_id != m_user_id)
		return;
#endif

	switch (msg_->type)
	{
	case PrimitiveType::FloorRequestStatus:
	{
		auto msg = msg_->CastTo<Message_FloorRequestStatus>();

		if (m_udp)
			SendMessage(vs::make_unique<Message_FloorRequestStatusAck>(msg->version, msg->conference_id, msg->transaction_id, msg->user_id));

		FloorRRState state;
		if (msg->attr_FLOOR_REQUEST_INFORMATION)
			state.request_id = msg->attr_FLOOR_REQUEST_INFORMATION->floor_request_id;
		else
			return;
		state.transaction_id = msg->transaction_id;
		if (msg->attr_FLOOR_REQUEST_INFORMATION
		 && !msg->attr_FLOOR_REQUEST_INFORMATION->attrs_FLOOR_REQUEST_STATUS.empty()
		)
			state.floor_id = msg->attr_FLOOR_REQUEST_INFORMATION->attrs_FLOOR_REQUEST_STATUS[0]->floor_id;
		else if (msg->attr_FLOOR_REQUEST_INFORMATION
		      && msg->attr_FLOOR_REQUEST_INFORMATION->attr_OVERALL_REQUEST_STATUS
		      && msg->attr_FLOOR_REQUEST_INFORMATION->attr_OVERALL_REQUEST_STATUS->attr_FLOOR_REQUEST_STATUS
		) // Polycom puts FLOOR-REQUEST-STATUS in a wrong place
			state.floor_id = msg->attr_FLOOR_REQUEST_INFORMATION->attr_OVERALL_REQUEST_STATUS->attr_FLOOR_REQUEST_STATUS->floor_id;
		else
			return;
		if (msg->attr_FLOOR_REQUEST_INFORMATION
		 && msg->attr_FLOOR_REQUEST_INFORMATION->attr_OVERALL_REQUEST_STATUS
		 && msg->attr_FLOOR_REQUEST_INFORMATION->attr_OVERALL_REQUEST_STATUS->attr_REQUEST_STATUS
		)
			state.status = msg->attr_FLOOR_REQUEST_INFORMATION->attr_OVERALL_REQUEST_STATUS->attr_REQUEST_STATUS->request_status;
		else
			return;

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			switch (state.status)
			{
			case RequestStatus::Granted:
				m_floors[state.floor_id].request_id = state.request_id;
				break;
			case RequestStatus::Denied:
			case RequestStatus::Cancelled:
			case RequestStatus::Released:
			case RequestStatus::Revoked:
				m_floors.erase(state.floor_id);
				break;
			}

			auto frr_it = std::find_if(m_floor_rrs.begin(), m_floor_rrs.end(), [&state](const FloorRRState& x) {
				return x.request_id == state.request_id;
			});
			if (frr_it == m_floor_rrs.end())
				frr_it = m_floor_rrs.emplace(m_floor_rrs.end(), state);
			else
				frr_it->status = state.status;
		}

		m_signal_FloorRequestStatus(state.floor_id, state.status);
		if (state.status != RequestStatus::Pending)
			m_floor_rr_cv.notify_all();
	}
		break;
	case PrimitiveType::FloorStatus:
	{
		auto msg = msg_->CastTo<Message_FloorStatus>();

		if (m_udp)
			SendMessage(vs::make_unique<Message_FloorStatusAck>(msg->version, msg->conference_id, msg->transaction_id, msg->user_id));

		FloorID floor_id;
		if (msg->attr_FLOOR_ID)
			floor_id = msg->attr_FLOOR_ID->value;
		else
			return;

		for (auto attr_FLOOR_REQUEST_INFORMATION: msg->attrs_FLOOR_REQUEST_INFORMATION)
		{
			UserID user_id = 0;
			if (attr_FLOOR_REQUEST_INFORMATION->attr_BENEFICIARY_INFORMATION	)
				user_id = attr_FLOOR_REQUEST_INFORMATION->attr_BENEFICIARY_INFORMATION->beneficiary_id;
			RequestStatus status;
			if (attr_FLOOR_REQUEST_INFORMATION->attr_OVERALL_REQUEST_STATUS
			 && attr_FLOOR_REQUEST_INFORMATION->attr_OVERALL_REQUEST_STATUS->attr_REQUEST_STATUS
			)
				status = attr_FLOOR_REQUEST_INFORMATION->attr_OVERALL_REQUEST_STATUS->attr_REQUEST_STATUS->request_status;
			else
				continue;
			m_signal_FloorStatus(floor_id, status, user_id);
		}
	}
		break;
	case PrimitiveType::Hello:
	{
		auto msg = msg_->CastTo<Message_Hello>();

		auto response_msg = vs::make_unique<Message_HelloAck>(msg->version, msg->conference_id, msg->transaction_id, msg->user_id,
			vs::make_unique<Attribute_SUPPORTED_PRIMITIVES>(
				PrimitiveType::FloorRequest,
				PrimitiveType::FloorRelease,
				PrimitiveType::FloorRequestStatus,
				PrimitiveType::FloorQuery,
				PrimitiveType::FloorStatus,
				PrimitiveType::Hello,
				PrimitiveType::HelloAck,
				PrimitiveType::Error
			),
			vs::make_unique<Attribute_SUPPORTED_ATTRIBUTES>(
				AttributeType::BENEFICIARY_ID,
				AttributeType::FLOOR_ID,
				AttributeType::FLOOR_REQUEST_ID,
				AttributeType::REQUEST_STATUS,
				AttributeType::ERROR_CODE,
				AttributeType::SUPPORTED_ATTRIBUTES,
				AttributeType::SUPPORTED_PRIMITIVES,
				AttributeType::FLOOR_REQUEST_INFORMATION,
				AttributeType::FLOOR_REQUEST_STATUS,
				AttributeType::OVERALL_REQUEST_STATUS
			)
		);
		if (m_udp)
		{
			response_msg->UpdateView();
			if (msg->version == 1)
				push_back_into(response_msg->attr_SUPPORTED_PRIMITIVES->values,
					PrimitiveType::FloorRequestStatusAck,
					PrimitiveType::FloorStatusAck_v1,
					PrimitiveType::Goodbye_v1,
					PrimitiveType::GoodbyeAck_v1
				);
			else
				push_back_into(response_msg->attr_SUPPORTED_PRIMITIVES->values,
					PrimitiveType::FloorRequestStatusAck,
					PrimitiveType::FloorStatusAck,
					PrimitiveType::Goodbye,
					PrimitiveType::GoodbyeAck
				);
		}
		SendMessage(std::move(response_msg));
	}
		break;
	case PrimitiveType::Goodbye:
	{
		auto msg = msg_->CastTo<Message_Goodbye>();

		if (m_udp)
			SendMessage(vs::make_unique<Message_GoodbyeAck>(msg->version, msg->conference_id, msg->transaction_id, msg->user_id));
	}
		break;
	default:
		break;
	}
}

ServerSession::ServerSession(ConferenceID conf_id, bool udp)
	: SessionBase(udp)
	, m_conf_id(conf_id)
	, m_send_status_for_all_floors(false)
	, m_floor_status_period(std::chrono::seconds(10))
	, m_last_request_id(0)
{
}

bool ServerSession::SendStatusForAllFloors()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_send_status_for_all_floors;
}

void ServerSession::SendStatusForAllFloors(bool value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_send_status_for_all_floors = value;
}

std::chrono::steady_clock::duration ServerSession::GetFloorStatusPeriod()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_floor_status_period;
}

void ServerSession::SetFloorStatusPeriod(std::chrono::steady_clock::duration period)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_floor_status_period = period;
}

void ServerSession::AddUser(UserID user_id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto& user_state = m_users[user_id];
}

void ServerSession::RemoveUser(UserID user_id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_users.erase(user_id);
}

RequestStatus ServerSession::RequestFloor(FloorID floor_id, UserID user_id)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	auto result = RequestFloor_impl(floor_id, user_id);
	DoDeferredActions(std::move(lock));
	return result;
}

RequestStatus ServerSession::RequestFloor_impl(FloorID floor_id, UserID user_id)
{
	auto& state = m_floors[floor_id];
	if (state.granted && state.user_id == user_id)
		return RequestStatus::Granted;

	if (state.granted)
	{
		SendMessage(vs::make_unique<Message_FloorRequestStatus>(1, m_conf_id, 0, state.user_id,
			vs::make_unique<Attribute_FLOOR_REQUEST_INFORMATION>(state.request_id,
				vs::make_unique<Attribute_FLOOR_REQUEST_STATUS>(floor_id),
				vs::make_unique<Attribute_OVERALL_REQUEST_STATUS>(state.request_id,
					vs::make_unique<Attribute_REQUEST_STATUS>(RequestStatus::Revoked, 0),
					vs::make_unique<Attribute_FLOOR_REQUEST_STATUS>(floor_id) // For Polycom
				)
			)
		));
		m_deferred_actions.emplace_back(std::bind(std::ref(m_signal_FloorStatusChange), floor_id, RequestStatus::Revoked, state.user_id));
		m_floor_requests.erase(state.request_id);
	}
	state.granted = true;
	state.user_id = user_id;
	state.request_id = ++m_last_request_id;
	for (const auto& u_kv: m_users)
	{
		if (u_kv.first == user_id)
			continue;
		if (m_send_status_for_all_floors || u_kv.second.queried_floors.count(floor_id))
			SendFloorStatus(floor_id, u_kv.first, 0, false);
	}
	m_deferred_actions.emplace_back(std::bind(std::ref(m_signal_FloorStatusChange), floor_id, RequestStatus::Granted, user_id));
	m_floor_requests[state.request_id] = floor_id;

	return RequestStatus::Granted;
}

RequestStatus ServerSession::ReleaseFloor(FloorID floor_id, UserID user_id)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	auto result = ReleaseFloor_impl(floor_id, user_id);
	DoDeferredActions(std::move(lock));
	return result;
}

RequestStatus ServerSession::ReleaseFloor_impl(FloorID floor_id, UserID user_id)
{
	auto& state = m_floors[floor_id];
	if (!state.granted)
		return RequestStatus::Released;
	if (state.user_id != user_id)
		return RequestStatus::Cancelled;

	state.granted = false;
	for (const auto& u_kv: m_users)
	{
		if (u_kv.first == user_id)
			continue;
		if (m_send_status_for_all_floors || u_kv.second.queried_floors.count(floor_id))
			SendFloorStatus(floor_id, u_kv.first, 0, false);
	}
	m_deferred_actions.emplace_back(std::bind(std::ref(m_signal_FloorStatusChange), floor_id, RequestStatus::Released, user_id));
	m_floor_requests.erase(state.request_id);

	return RequestStatus::Released;
}

void ServerSession::SendFloorStatus(FloorID floor_id, UserID user_id, TransactionID transaction_id, bool only_granted)
{
	auto f_it = m_floors.find(floor_id);
	if (f_it == m_floors.end())
		return;
	if (only_granted && !f_it->second.granted)
		return;

	SendMessage(vs::make_unique<Message_FloorStatus>(1, m_conf_id, transaction_id, user_id,
		vs::make_unique<Attribute_FLOOR_ID>(floor_id),
		vs::make_unique<Attribute_FLOOR_REQUEST_INFORMATION>(f_it->second.request_id,
			vs::make_unique<Attribute_FLOOR_REQUEST_STATUS>(floor_id),
			vs::make_unique<Attribute_OVERALL_REQUEST_STATUS>(f_it->second.request_id,
				vs::make_unique<Attribute_REQUEST_STATUS>(f_it->second.granted ? RequestStatus::Granted : RequestStatus::Released, 0),
				vs::make_unique<Attribute_FLOOR_REQUEST_STATUS>(floor_id) // For Polycom
			),
			vs::make_unique<Attribute_BENEFICIARY_INFORMATION>(f_it->second.user_id)
		)
	));
}

void ServerSession::DoDeferredActions(std::unique_lock<std::mutex>&& lock)
{
	assert(lock && lock.mutex() == &m_mutex);
	decltype(m_deferred_actions) actions;
	m_deferred_actions.swap(actions);
	lock.unlock();
	for (auto& x: actions)
		x();
}

void ServerSession::HandleMessage(std::unique_ptr<Message>&& msg_)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (msg_->conference_id != m_conf_id)
	{
		SendMessage(vs::make_unique<Message_Error>(msg_->version, msg_->conference_id, msg_->transaction_id, msg_->user_id,
			vs::make_unique<Attribute_ERROR_CODE>(1)
		));
		return;
	}

	switch (msg_->type)
	{
	case PrimitiveType::FloorRequest:
	{
		auto msg = msg_->CastTo<Message_FloorRequest>();

		FloorID floor_id;
		if (msg->attrs_FLOOR_ID.size() >= 1)
			floor_id = msg->attrs_FLOOR_ID[0]->value;
		else
		{
			SendMessage(vs::make_unique<Message_Error>(msg->version, msg->conference_id, msg->transaction_id, msg->user_id,
				vs::make_unique<Attribute_ERROR_CODE>(6)
			));
			return;
		}

		auto result = RequestFloor_impl(floor_id, msg->user_id);
		SendMessage(vs::make_unique<Message_FloorRequestStatus>(msg->version, msg->conference_id, msg->transaction_id, msg->user_id,
			vs::make_unique<Attribute_FLOOR_REQUEST_INFORMATION>(m_last_request_id,
				vs::make_unique<Attribute_FLOOR_REQUEST_STATUS>(floor_id),
				vs::make_unique<Attribute_OVERALL_REQUEST_STATUS>(m_last_request_id,
					vs::make_unique<Attribute_REQUEST_STATUS>(result, 0),
					vs::make_unique<Attribute_FLOOR_REQUEST_STATUS>(floor_id) // For Polycom
				)
			)
		));
	}
		break;
	case PrimitiveType::FloorRelease:
	{
		auto msg = msg_->CastTo<Message_FloorRelease>();

		FloorRequestID request_id;
		if (msg->attr_FLOOR_REQUEST_ID)
			request_id = msg->attr_FLOOR_REQUEST_ID->value;
		else
		{
			SendMessage(vs::make_unique<Message_Error>(msg->version, msg->conference_id, msg->transaction_id, msg->user_id,
				vs::make_unique<Attribute_ERROR_CODE>(7)
			));
			return;
		}

		auto fr_it = m_floor_requests.find(request_id);
		if (fr_it == m_floor_requests.end())
		{
			SendMessage(vs::make_unique<Message_Error>(msg->version, msg->conference_id, msg->transaction_id, msg->user_id,
				vs::make_unique<Attribute_ERROR_CODE>(7)
			));
			return;
		}
		FloorID floor_id = fr_it->second;

		auto result = ReleaseFloor_impl(floor_id, msg->user_id);
		SendMessage(vs::make_unique<Message_FloorRequestStatus>(msg->version, msg->conference_id, msg->transaction_id, msg->user_id,
			vs::make_unique<Attribute_FLOOR_REQUEST_INFORMATION>(request_id,
				vs::make_unique<Attribute_FLOOR_REQUEST_STATUS>(floor_id),
				vs::make_unique<Attribute_OVERALL_REQUEST_STATUS>(request_id,
					vs::make_unique<Attribute_REQUEST_STATUS>(result, 0),
					vs::make_unique<Attribute_FLOOR_REQUEST_STATUS>(floor_id) // For Polycom
				)
			)
		));
	}
		break;
	case PrimitiveType::FloorQuery:
	{
		auto msg = msg_->CastTo<Message_FloorQuery>();

		auto& user_state = m_users[msg->user_id];

		user_state.queried_floors.clear();
		for (auto attr_FLOOR_ID: msg->attrs_FLOOR_ID)
			user_state.queried_floors.insert(attr_FLOOR_ID->value);

		bool first = true;
		if (m_send_status_for_all_floors)
			for (auto& floor_kv: m_floors)
			{
				if (floor_kv.second.user_id == msg->user_id)
					continue;
				SendFloorStatus(floor_kv.first, msg->user_id, first ? msg->transaction_id : 0);
				first = false;
			}
		else
			for (FloorID floor_id: user_state.queried_floors)
			{
				SendFloorStatus(floor_id, msg->user_id, first ? msg->transaction_id : 0);
				first = false;
			}
	}
		break;
	case PrimitiveType::Hello:
	{
		auto msg = msg_->CastTo<Message_Hello>();

		auto response_msg = vs::make_unique<Message_HelloAck>(msg->version, msg->conference_id, msg->transaction_id, msg->user_id,
			vs::make_unique<Attribute_SUPPORTED_PRIMITIVES>(
				PrimitiveType::FloorRequest,
				PrimitiveType::FloorRelease,
				PrimitiveType::FloorRequestStatus,
				PrimitiveType::FloorQuery,
				PrimitiveType::FloorStatus,
				PrimitiveType::Hello,
				PrimitiveType::HelloAck,
				PrimitiveType::Error
			),
			vs::make_unique<Attribute_SUPPORTED_ATTRIBUTES>(
				AttributeType::BENEFICIARY_ID,
				AttributeType::FLOOR_ID,
				AttributeType::FLOOR_REQUEST_ID,
				AttributeType::REQUEST_STATUS,
				AttributeType::ERROR_CODE,
				AttributeType::SUPPORTED_ATTRIBUTES,
				AttributeType::SUPPORTED_PRIMITIVES,
				AttributeType::FLOOR_REQUEST_INFORMATION,
				AttributeType::FLOOR_REQUEST_STATUS,
				AttributeType::OVERALL_REQUEST_STATUS
			)
		);
		if (m_udp)
		{
			response_msg->UpdateView();
			if (msg->version == 1)
				push_back_into(response_msg->attr_SUPPORTED_PRIMITIVES->values,
					PrimitiveType::FloorRequestStatusAck,
					PrimitiveType::FloorStatusAck_v1,
					PrimitiveType::Goodbye_v1,
					PrimitiveType::GoodbyeAck_v1
				);
			else
				push_back_into(response_msg->attr_SUPPORTED_PRIMITIVES->values,
					PrimitiveType::FloorRequestStatusAck,
					PrimitiveType::FloorStatusAck,
					PrimitiveType::Goodbye,
					PrimitiveType::GoodbyeAck
				);
		}
		SendMessage(std::move(response_msg));
	}
		break;
	case PrimitiveType::Goodbye:
	{
		auto msg = msg_->CastTo<Message_Goodbye>();

		if (m_udp)
			SendMessage(vs::make_unique<Message_GoodbyeAck>(msg->version, msg->conference_id, msg->transaction_id, msg->user_id));
	}
		break;
	default:
		break;
	}
	DoDeferredActions(std::move(lock));
}

void ServerSession::Tick()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	const auto now = std::chrono::steady_clock::now();
	if (m_last_floor_status_time + m_floor_status_period >= now)
		return;

	m_last_floor_status_time = now;
	for (const auto& u_kv: m_users)
		if (m_send_status_for_all_floors)
			for (auto& floor_kv: m_floors)
			{
				if (floor_kv.second.user_id == u_kv.first)
					continue;
				SendFloorStatus(floor_kv.first, u_kv.first, 0);
			}
		else
			for (FloorID floor_id: u_kv.second.queried_floors)
				SendFloorStatus(floor_id, u_kv.first, 0);
}

}
