#pragma once
#include "../transport/Router/VS_RouterMessage.h"
#include <functional>
#include <memory>
#include <map>
#include <queue>
#include <boost/signals2/signal.hpp>
class VS_PointParams;
namespace tc3_test
{
class FakeRouter
{
	struct comb
	{
		using result_type = bool;

		template<typename InputIter>
		result_type operator()(InputIter first, InputIter end) const
		{
			while (first != end)
			{
				if (*first)
					return true;
				first++;
			}
			return false;
		}
	};
	boost::signals2::signal<bool(const VS_RouterMessage*), comb> check_for_skip_;
	boost::signals2::signal<bool(VS_RouterMessage*), comb> try_external_processing_;

public:
	using ProcessingFuncT = std::function<void(VS_RouterMessage*)>;
	using OnPointDisconnectFuncT = std::function < void(const VS_PointParams* prm)>;

	void RegisterEndpoint(const std::string &id, const ProcessingFuncT& processingFunc, const OnPointDisconnectFuncT &on_point_disc_func);
	void UnregisterEndpoint(const std::string&id);

	void SendMsg(VS_RouterMessage *);
	bool SkipMsg(const VS_RouterMessage*msg) const;
	bool IsEndpointExist(const char *ep_name);
	bool DisconnectEndpoint(const char *ep_name);
	boost::signals2::connection AddTesterForSkip(const decltype(check_for_skip_)::slot_type &slot)
	{
		return check_for_skip_.connect(slot);
	}
	boost::signals2::connection RegisterExtHandler(const decltype(try_external_processing_)::slot_type &slot)
	{
		return try_external_processing_.connect(slot);
	}
	void ProcessQueue();

	static void ChangeCurrentEndpoint(const char *curr_ep);
private:
	uint32_t processing_recursive_depth_ = 0;

	std::map<std::string, std::pair<ProcessingFuncT, OnPointDisconnectFuncT>> msg_dispatcher_;
	std::queue<VS_RouterMessage*> msg_queue_;

};
}