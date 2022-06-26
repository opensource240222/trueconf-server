#pragma once

#include <boost/weak_ptr.hpp>
#include "std-generic/compat/map.h"
#include "std-generic/compat/functional.h"
#include "std-generic/cpplib/VS_ClockWrapper.h"
#include "net/Port.h"
#include "std-generic/cpplib/StrCompare.h"
#include "std-generic/cpplib/string_view.h"

#include <chrono>
#include <string>
#include <vector>

enum eStartLineType : int;
class VS_SIPMessage;
class VS_SIPParserInfo;

typedef std::pair<std::shared_ptr<VS_SIPParserInfo>, std::shared_ptr<VS_SIPMessage>> CtxMsgPair;

class VS_SIPTransactionHandler
{
public:
	void ClientTransaction(const std::shared_ptr<VS_SIPMessage> &msg);
	void ClientTransactionUnreliable(const std::shared_ptr<VS_SIPMessage> &msg, const CtxMsgPair &message, string_view dialogId);
	void ClientTransactionCompleted(const std::shared_ptr<VS_SIPMessage> &msg);
	void ClientTransactionProceeding(const std::shared_ptr<VS_SIPMessage> &msg);
	void RequestCancelled(const std::shared_ptr<VS_SIPParserInfo> &ctxInfo, string_view branch, eStartLineType method);
	void DialogTerminated(string_view dialogId);

	void AckTransaction(string_view branch, const std::shared_ptr<VS_SIPMessage> &ack);

	void ServerTransactionFinalResponse(const std::shared_ptr<VS_SIPMessage> &msg, const CtxMsgPair& message, string_view dialogId);
	void ServerTransactionProceeding(const std::shared_ptr<VS_SIPMessage> &msg, const CtxMsgPair& message);
	void ServerTransactionCompleted(const std::shared_ptr<VS_SIPMessage> &msg);

	bool IsActiveClientTransaction(const std::shared_ptr<VS_SIPMessage> &msg) const;
	bool IsActiveServerTransaction(const std::shared_ptr<VS_SIPMessage> &msg) const;
	CtxMsgPair GetMessageForRetransmission();
	std::shared_ptr<VS_SIPMessage> GetResponse(const std::shared_ptr<VS_SIPMessage> &msg);
	std::shared_ptr<VS_SIPMessage> GetAck(const std::string &branch);

	void Cleanup() noexcept;

	void Clear() noexcept;

protected:
	steady_clock_wrapper &clock() { return m_clock; }
private:
	enum TransactionType
	{
		CLIENT_TRANSACTION, SERVER_TRANSACTION
	};

	struct TransactionInfo
	{
		std::string dialogId;
		CtxMsgPair msg;
		std::chrono::steady_clock::time_point timeOfTerminating;
		std::chrono::steady_clock::time_point timeOfNextSend;
		std::chrono::steady_clock::time_point nextInterval;
		TransactionType type;
	};

	struct AckInfo
	{
		std::shared_ptr<VS_SIPMessage> ack;
		std::chrono::steady_clock::time_point timeOfTerminating;
	};


	struct TransactionId
	{
		std::string branch;
		std::string viaTopHost;
		int cseq;
		eStartLineType method;
		net::port viaTopPort;

		friend bool operator<(const TransactionId& lhs, const TransactionId& rhs)
		{
			return std::tie(lhs.branch, lhs.viaTopHost, lhs.cseq, lhs.method, lhs.viaTopPort) < std::tie(
				rhs.branch, rhs.viaTopHost, rhs.cseq, rhs.method, rhs.viaTopPort);
		}
	};

	struct AckMatchInfo
	{
		std::string callId;
		int cseq;
		friend bool operator<(const AckMatchInfo& lhs, const AckMatchInfo& rhs)
		{
			return std::tie(lhs.callId, lhs.cseq) < std::tie(rhs.callId, rhs.cseq);
		}
	};
private:
	TransactionId GetClientTransactionId(const std::shared_ptr<VS_SIPMessage>& msg) const;
	TransactionId GetServerTransactionId(const std::shared_ptr<VS_SIPMessage>& msg) const;
	TransactionId GetTransactionId(const std::shared_ptr<VS_SIPParserInfo> &ctxInfo, string_view branch, eStartLineType method) const;
	AckMatchInfo GetAckMatchInfo(const std::shared_ptr<VS_SIPMessage>& msg) const;
private:
	typedef vs::map<TransactionId, TransactionInfo> Transactions;
	typedef vs::map<std::string, AckInfo, vs::str_less> Acks;
	typedef vs::map<AckMatchInfo, TransactionId> AckMatching;

	Transactions transactions_;
	Acks acks_;
	AckMatching ackMatching_;

private:
	steady_clock_wrapper m_clock;
};