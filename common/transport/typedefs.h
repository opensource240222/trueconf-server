#pragma once
#include "std-generic/cpplib/string_view.h"
#include <chrono>
#include <functional>
#include <future>
#include <boost/signals2.hpp>

class VS_RouterMessage;
using ResponseCallBackT = std::function<void(VS_RouterMessage*)>;
using RequestLifeTimeT = std::chrono::steady_clock::duration;
using ResponseFutureT = std::future<VS_RouterMessage*>;
using CheckEndpointPredT = std::function<bool(
	string_view/*cid*/, string_view/*endpoint*/, unsigned char /*hops*/)>;

typedef boost::signals2::signal<void(const std::multimap<std::string, std::string>&)> AliasesChangedSignal;
typedef AliasesChangedSignal::slot_type AliasesChangedSlot;