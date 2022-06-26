#if defined(_WIN32) // Not ported yet

#include "std/cpplib/lua_deleters.h"

#include <utility>
#include <memory>
#include <iostream>

#include <gtest/gtest.h>
#include <lua.hpp>

typedef std::unique_ptr<lua_State, lua_State_deleter> lua_State_ptr;

static lua_State_ptr create_new_lua_State(void)
{
	return lua_State_ptr(luaL_newstate());
}


namespace lua_embedding_tests {
	class LuaEmbedding_SimpleTests : public ::testing::Test
	{
	protected:
		LuaEmbedding_SimpleTests()
		{}

		virtual void SetUp()
		{}

		virtual void TearDown()
		{}
	};

	TEST_F(LuaEmbedding_SimpleTests, SimpleCallToLua)
	{
		auto lua_ctx = create_new_lua_State();
		ASSERT_NE(lua_ctx, nullptr);

		auto L = lua_ctx.get();

		luaL_openlibs(L);

		luaL_dostring(L, "function add(a, b) return a + b; end");

		double a = 1,
			b = 2;
		double res = 0;
		int isnum = 0;

		lua_getglobal(L, "add");
		lua_pushnumber(L, a);
		lua_pushnumber(L, b);

		ASSERT_EQ(lua_pcall(L, 2, 1, 0), LUA_OK) << "Call to lua function failed: " << lua_tostring(L, -1) << std::endl;

		res = lua_tonumberx(L, -1, &isnum);
		ASSERT_TRUE(isnum != 0);

		ASSERT_EQ(res, 3);
	}

	TEST_F(LuaEmbedding_SimpleTests, SimpleCallToC)
	{
		auto lua_ctx = create_new_lua_State();
		ASSERT_NE(lua_ctx, nullptr);
		auto L = lua_ctx.get();

		int isnum = 0;
		double res;

		/* function sub(a, b) ... end */
		auto l_sub = [](lua_State *L) -> int
		{
			double a, b;
			a = lua_tonumber(L, 1);
			b = lua_tonumber(L, 2);

			lua_pushnumber(L, (a - b));

			return 1;
		};

		// register C function
		lua_pushcfunction(L, l_sub);
		lua_setglobal(L, "sub");

		// do operation
		lua_getglobal(L, "sub");
		lua_pushnumber(L, 3);
		lua_pushnumber(L, 2);

		ASSERT_EQ(lua_pcall(L, 2, 1, 0), LUA_OK) << "Call to lua function failed: " << lua_tostring(L, -1) << std::endl;

		res = lua_tonumberx(L, -1, &isnum);
		ASSERT_TRUE(isnum != 0);

		ASSERT_EQ(res, 1);

		// call to C from Lua
		luaL_dostring(L, "function call_sub() return sub(3, 2); end");

		// do operation
		lua_getglobal(L, "call_sub");

		ASSERT_EQ(lua_pcall(L, 0, 1, 0), LUA_OK) << "Call to lua function failed: " << lua_tostring(L, -1) << std::endl;

		res = lua_tonumberx(L, -1, &isnum);
		ASSERT_TRUE(isnum != 0);

		ASSERT_EQ(res, 1);
	}

	TEST_F(LuaEmbedding_SimpleTests, EmbedLibrary)
	{
		auto lua_ctx = create_new_lua_State();
		ASSERT_NE(lua_ctx, nullptr);
		auto L = lua_ctx.get();
		int isnum = 0;

		luaL_openlibs(L);

		static const struct luaL_Reg funcs[] = {
			{ "sub", [](lua_State *L) -> int
			{
				double a, b;
				a = lua_tonumber(L, 1);
				b = lua_tonumber(L, 2);

				lua_pushnumber(L, (a - b));

				return 1;
			} },
			{ "add", [](lua_State *L) -> int
			{
				double a, b;
				a = lua_tonumber(L, 1);
				b = lua_tonumber(L, 2);

				lua_pushnumber(L, (a + b));

				return 1;
			} },

			{ NULL, NULL }

		};

		// create library
		lua_newtable(L);
		luaL_setfuncs(L, funcs, 0);
		lua_setglobal(L, "arith");

		// define some code
		luaL_dostring(L, "function test_sub() return arith.sub(3, 2); end");
		luaL_dostring(L, "function test_add() return arith.add(3, 2); end");

		// test substraction
		lua_getglobal(L, "test_sub");

		ASSERT_EQ(lua_pcall(L, 0, 1, 0), LUA_OK) << "Call to lua function failed: " << lua_tostring(L, -1) << std::endl;

		double res = lua_tonumberx(L, -1, &isnum);
		ASSERT_TRUE(isnum != 0);

		ASSERT_EQ(res, 1);

		// test addition
		lua_getglobal(L, "test_add");

		ASSERT_EQ(lua_pcall(L, 0, 1, 0), LUA_OK) << "Call to lua function failed: " << lua_tostring(L, -1) << std::endl;

		res = lua_tonumberx(L, -1, &isnum);
		ASSERT_TRUE(isnum != 0);

		ASSERT_EQ(res, 5);
	}
}

#endif
