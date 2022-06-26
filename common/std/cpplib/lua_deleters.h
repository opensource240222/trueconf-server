#pragma once

#include <lua.hpp>

struct lua_State_deleter { void operator()(lua_State* p) const { ::lua_close(p); } };
