#include "stdafx.h"

LuaMan luaMan;

LuaMan::LuaMan()
{
	L = nullptr;
}

LuaMan::~LuaMan()
{
	Destroy();
}

void LuaMan::Create()
{
	Destroy();
	L = luaL_newstate();
	LuaBind(L);
	if (!aflDoFile(L, "lua/main.lua")) {
		printf("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

void LuaMan::Destroy()
{
	if (!L) {
		return;
	}
	int top = lua_gettop(L);
	if (top > 0) {
		aflog("error! stack is not empty\n");
	}
	lua_close(L);
	L = nullptr;
}

void LuaMan::Update()
{
	lua_getglobal(L, "Update");
//	aflDumpStack();
	lua_call(L, 0, 0);
//	aflDumpStack();
	int top = lua_gettop(L);
	if (top > 0) {
		aflog("error! stack is not empty\n");
	}
}
