#pragma once

#include <lua.hpp>

#include "af_lua_helpers.h"

class LuaMan
{
	lua_State* L;
	void CallGlobal(const char* func);
public:
	LuaMan();
	~LuaMan();
	void Create();
	void Destroy();
	void Update(ViewDesc& viewDesc);
	void Draw2D(AFCommandList& cmd);
	void Draw3D(ViewDesc& viewDesc);
	ViewDesc* GetViewDesc();	// workaround to access 'ViewDesc' from lua managed instances.
	lua_State* GetState() { return L; }
};

typedef std::vector<void(*)(lua_State* L)> LuaBindFuncContainer;
LuaBindFuncContainer& GetLuaBindFuncContainer();

extern LuaMan luaMan;
