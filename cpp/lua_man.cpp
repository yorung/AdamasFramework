#include "stdafx.h"

LuaMan luaMan;
extern MatrixStack luaMatrixStack;

LuaBindFuncContainer& GetLuaBindFuncContainer()
{
	static LuaBindFuncContainer c;
	return c;
}

LuaMan::LuaMan()
{
	L = nullptr;
}

LuaMan::~LuaMan()
{
	assert(!L);
}

void LuaMan::Create()
{
	Destroy();
	L = luaL_newstate();
	LuaBind(L);
	for (auto it : GetLuaBindFuncContainer()) {
		it(L);
	}
#ifdef _DEBUG
	const char* start = "lua/startD.lua";
#else
	const char* start = "lua/start.lua";
#endif
	if (!aflDoFile(L, start)) {
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

void LuaMan::CallGlobal(const char* func)
{
	luaMatrixStack.Reset();
	lua_getglobal(L, func);
	//	aflDumpStack();
	lua_call(L, 0, 0);
	//	aflDumpStack();
	int top = lua_gettop(L);
	if (top > 0) {
		aflog("error! stack is not empty\n");
	}
	luaMatrixStack.Reset();
}

void LuaMan::Update()
{
	CallGlobal("Update");
}

void LuaMan::DrawSprites()
{
	CallGlobal("DrawSprites");
}
