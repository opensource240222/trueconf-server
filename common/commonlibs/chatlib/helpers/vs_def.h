#pragma once
#include "std-generic/cpplib/string_view.h"
#include "std-generic/cpplib/nod.hpp"

#include <memory>
#include <string>

namespace vs
{
class AccountInfo;
class ExternalComponentsInterface;
class ResolverInterface;

enum class CallIDType
{
	undef = 0,
	client,
	server
};

using AccountInfoPtr = std::shared_ptr<AccountInfo>;
using ExternalComponentsPtr = std::shared_ptr<ExternalComponentsInterface>;
using ResolverPtr = std::shared_ptr<ResolverInterface>;

using BSInfo = std::string;
using CallID = std::string;
using CallIDRef = string_view;

template<typename F>
using Signal = nod::signal<F>;
using SubscribeConnection = nod::connection;
using ScopedConnection = nod::scoped_connection;
}