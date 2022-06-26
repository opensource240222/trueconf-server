#include "VS_SIPTransactionHandler.h"
#include "../SIPParserBase/VS_Const.h"

#include <algorithm>
#include "VS_SIPMessage.h"
#include "VS_SIPMetaField.h"
#include "VS_SIPField_Via.h"
#include "TrueGateway/sip/VS_SIPParserInfo.h"
#include <assert.h>

namespace
{
	const std::chrono::milliseconds T1(500);
	const std::chrono::milliseconds T2(4 * 1000);
	const std::chrono::milliseconds B_F_H_J(64 * T1);

	const std::chrono::milliseconds ACK_RETRANSMISSION_TIMEOUT(32 * 1000);
	const std::chrono::milliseconds INVITE_FINAL_RESPONSE_TIMEOUT(2 * 60 * 1000);
}


CtxMsgPair VS_SIPTransactionHandler::GetMessageForRetransmission()
{
	auto cur_time = m_clock.now();

	Transactions::iterator it = transactions_.begin();
	while (it != transactions_.end())
	{
		TransactionInfo &info = it->second;

		if (cur_time >= info.timeOfTerminating)
		{
			it = transactions_.erase(it);
		}
		else
		{
			if (info.timeOfNextSend != std::chrono::steady_clock::time_point() && cur_time >= info.timeOfNextSend)
			{
				info.timeOfNextSend = cur_time.time_since_epoch() + info.nextInterval;
				if (it->first.method == TYPE_INVITE && it->second.type == CLIENT_TRANSACTION)
				{
					info.nextInterval = std::chrono::steady_clock::time_point(2 * info.nextInterval.time_since_epoch());
				}
				else
				{
					info.nextInterval = std::chrono::steady_clock::time_point(
						std::min(std::chrono::duration_cast<std::chrono::milliseconds>(info.nextInterval.time_since_epoch() * 2), T2));
				}
				return info.msg;
			}

			++it;
		}
	}

	return std::make_pair(CtxMsgPair::first_type(), nullptr);
}

std::shared_ptr<VS_SIPMessage> VS_SIPTransactionHandler::GetResponse(const std::shared_ptr<VS_SIPMessage> &msg)
{
	Transactions::iterator it = transactions_.find(GetServerTransactionId(msg));
	if (it != transactions_.cend())
	{
		TransactionInfo &info = it->second;
		if (m_clock.now() < info.timeOfTerminating)
		{
			return info.msg.second;
		}
		transactions_.erase(it);
	}
	return {};
}

std::shared_ptr<VS_SIPMessage> VS_SIPTransactionHandler::GetAck(const std::string &branch)
{
	Acks::iterator it = acks_.find(branch);
	if (it != acks_.end())
	{
		AckInfo &info = it->second;
		if (m_clock.now() < info.timeOfTerminating)
		{
			return info.ack;
		}
		acks_.erase(it);
	}
	return {};
}

void VS_SIPTransactionHandler::ClientTransaction(const std::shared_ptr<VS_SIPMessage> &msg)
{
	transactions_[GetClientTransactionId(msg)] = { std::string(msg->CallID()), std::make_pair(CtxMsgPair::first_type(), CtxMsgPair::second_type()),
		m_clock.now() + B_F_H_J, std::chrono::steady_clock::time_point(), std::chrono::steady_clock::time_point(), CLIENT_TRANSACTION };
}

void VS_SIPTransactionHandler::ClientTransactionUnreliable(const std::shared_ptr<VS_SIPMessage> &msg, const CtxMsgPair &message, string_view dialogId)
{
	auto id = GetClientTransactionId(msg);

	if (message.second->GetMethod() == TYPE_BYE) // dialog/session terminated, delete related messages
	{
		DialogTerminated(dialogId);
	}

	const auto cur_time = m_clock.now();
	transactions_[id] = { std::string(dialogId), message, cur_time + B_F_H_J, cur_time + T1,
		std::chrono::steady_clock::time_point(2 * T1), CLIENT_TRANSACTION };
}

void VS_SIPTransactionHandler::ClientTransactionCompleted(const std::shared_ptr<VS_SIPMessage> &msg)
{
	transactions_.erase(GetClientTransactionId(msg));
}

void VS_SIPTransactionHandler::ClientTransactionProceeding(const std::shared_ptr<VS_SIPMessage> &msg)
{
	if (msg->GetMethod() == TYPE_INVITE)
	{
		auto &&id = GetClientTransactionId(msg);
		transactions_[id].timeOfTerminating = { m_clock.now() + INVITE_FINAL_RESPONSE_TIMEOUT };
		transactions_[id].timeOfNextSend = std::chrono::steady_clock::time_point();
	}
}

void VS_SIPTransactionHandler::RequestCancelled(const std::shared_ptr<VS_SIPParserInfo> &ctxInfo, string_view branch, eStartLineType method)
{
	auto id = GetTransactionId(ctxInfo, branch, method);
	transactions_[id].timeOfTerminating = { m_clock.now() + B_F_H_J };
	transactions_[id].timeOfNextSend = {};
}

void VS_SIPTransactionHandler::DialogTerminated(string_view dialogId)
{
	auto it = transactions_.begin();
	while (it != transactions_.cend())
	{
		TransactionInfo &info = it->second;
		const bool autoRetransmitted = info.type == CLIENT_TRANSACTION || (info.type == SERVER_TRANSACTION && it->first.method == TYPE_INVITE);
		if (info.dialogId == dialogId && autoRetransmitted)
		{
			it = transactions_.erase(it);
		}
		else
		{
			it = std::next(it);
		}
	}
}

void VS_SIPTransactionHandler::AckTransaction(string_view branch, const std::shared_ptr<VS_SIPMessage> &ack)
{
	auto res = acks_.find(branch);
	if(res != acks_.cend())
	{
		res->second = AckInfo{ ack, m_clock.now() + ACK_RETRANSMISSION_TIMEOUT };
	}
	else
	{
		acks_.emplace(branch, AckInfo{ ack, m_clock.now() + ACK_RETRANSMISSION_TIMEOUT});
	}
}

void VS_SIPTransactionHandler::ServerTransactionFinalResponse(const std::shared_ptr<VS_SIPMessage> &msg, const CtxMsgPair& message, string_view dialogId)
{
	auto id = GetServerTransactionId(msg);
	const auto cur_time = m_clock.now();
	transactions_[id] = { std::string(dialogId), std::move(message), cur_time + B_F_H_J,
		id.method == TYPE_INVITE ? cur_time + T1 : std::chrono::steady_clock::time_point(),
		id.method == TYPE_INVITE ? std::chrono::steady_clock::time_point(2 * T1) : std::chrono::steady_clock::time_point(), SERVER_TRANSACTION };

	if (id.method == TYPE_INVITE)
	{
		ackMatching_[GetAckMatchInfo(msg)] = std::move(id);
	}
}

void VS_SIPTransactionHandler::ServerTransactionProceeding(const std::shared_ptr<VS_SIPMessage> &msg, const CtxMsgPair
                                                           &message)
{
	transactions_[GetServerTransactionId(msg)] = TransactionInfo{ "", message, m_clock.now() + INVITE_FINAL_RESPONSE_TIMEOUT,
		std::chrono::steady_clock::time_point(), std::chrono::steady_clock::time_point(), SERVER_TRANSACTION };

}

void VS_SIPTransactionHandler::ServerTransactionCompleted(const std::shared_ptr<VS_SIPMessage> &msg)
{
	auto match_info = GetAckMatchInfo(msg);
	transactions_.erase(ackMatching_[match_info]);
	ackMatching_.erase(match_info);
}

bool VS_SIPTransactionHandler::IsActiveClientTransaction(const std::shared_ptr<VS_SIPMessage> &msg) const
{
	return transactions_.find(GetClientTransactionId(msg)) != transactions_.cend();
}

bool VS_SIPTransactionHandler::IsActiveServerTransaction(const std::shared_ptr<VS_SIPMessage>& msg) const
{
	return transactions_.find(GetServerTransactionId(msg)) != transactions_.cend();
}

void VS_SIPTransactionHandler::Cleanup() noexcept
{
	const auto cur_time = m_clock.now();
	Transactions::iterator it1 = transactions_.begin();
	while (it1 != transactions_.cend())
	{
		TransactionInfo &info = it1->second;
		if (cur_time >= info.timeOfTerminating)
		{
			it1 = transactions_.erase(it1);
		}
		else
		{
			++it1;
		}
	}

	Acks::iterator it2 = acks_.begin();
	while (it2 != acks_.cend())
	{
		AckInfo &info = it2->second;
		if (cur_time >= info.timeOfTerminating)
		{
			it2 = acks_.erase(it2);
		}
		else
		{
			++it2;
		}
	}

	auto it3 = ackMatching_.begin();
	while (it3 != ackMatching_.cend())
	{
		auto &id = it3->second;
		if (transactions_.find(id) == transactions_.cend())
		{
			it3 = ackMatching_.erase(it3);
		}
		else
		{
			++it3;
		}
	}
}

void VS_SIPTransactionHandler::Clear() noexcept
{
	transactions_.clear();
}

inline VS_SIPTransactionHandler::TransactionId VS_SIPTransactionHandler::GetClientTransactionId(const std::shared_ptr<VS_SIPMessage>& msg) const
{
	return TransactionId{ std::string(msg->Branch()), "", msg->GetCSeq(), msg->GetMethod(), 0 };
}

inline VS_SIPTransactionHandler::TransactionId VS_SIPTransactionHandler::GetServerTransactionId(const std::shared_ptr<VS_SIPMessage>& msg) const
{
	return TransactionId{ std::string(msg->Branch()), msg->GetSIPMetaField()->iVia.front()->Host(), msg->GetCSeq(), msg->GetMethod(),
	msg->GetSIPMetaField()->iVia.front()->Port() };
}

inline VS_SIPTransactionHandler::TransactionId VS_SIPTransactionHandler::GetTransactionId(
	const std::shared_ptr<VS_SIPParserInfo> &ctxInfo, string_view branch, eStartLineType method) const
{
	return TransactionId{ std::string(branch), {}, ctxInfo->GetSIPSequenceNumber(), method, 0 };
}

inline VS_SIPTransactionHandler::AckMatchInfo VS_SIPTransactionHandler::GetAckMatchInfo(const std::shared_ptr<VS_SIPMessage>& msg) const
{
	return { std::string(msg->CallID()), msg->GetCSeq() };
}
