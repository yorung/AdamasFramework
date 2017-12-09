#include "stdafx.h"

LuaMan luaMan;
extern SpriteCommands luaSpriteCommands;
extern ViewDesc* luaViewDesc;

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
#ifndef NDEBUG
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
	if (!L) {
		return;
	}
	lua_getglobal(L, func);
	//	aflDumpStack();
	if (lua_pcall(L, 0, LUA_MULTRET, 0)) {
		printf("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	//	aflDumpStack();
	int top = lua_gettop(L);
	if (top > 0) {
		aflog("error! stack is not empty\n");
	}
}

void LuaMan::Update(ViewDesc& viewDesc)
{
	luaViewDesc = &viewDesc;
	CallGlobal("Update");
}

void LuaMan::Draw2D(AFCommandList& cmd)
{
//	assert(luaSpriteCommands.empty());
	CallGlobal("Draw2D");
	std::stable_sort(luaSpriteCommands.begin(), luaSpriteCommands.end(), [](const SpriteCommand& l, const SpriteCommand& r){ return l.matW._43 < r.matW._43; });
	spriteRenderer.Draw(cmd, luaSpriteCommands);
	luaSpriteCommands.clear();
}

void LuaMan::Draw3D(ViewDesc& viewDesc)
{
	luaViewDesc = &viewDesc;
	CallGlobal("Draw3D");
}

ViewDesc* LuaMan::GetViewDesc()
{
	return luaViewDesc;
}
