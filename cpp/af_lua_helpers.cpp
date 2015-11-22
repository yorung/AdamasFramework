#include "stdafx.h"

void _aflDumpStack(lua_State* L, const char* func, int line)
{
	int top = lua_gettop(L);
	aflog("(%s,%d) top=%d\n", func, line, top);
	for (int i = 0; i < top; i++) {
		int positive = top - i;
		int negative = -(i + 1);
		int type = lua_type(L, positive);
		int typeN = lua_type(L, negative);
		assert(type == typeN);
		char buf[48] = "";
		switch (type) {
		case LUA_TNUMBER:
			snprintf(buf, sizeof(buf), "%f", lua_tonumber(L, positive));
			break;
		case LUA_TSTRING:
			snprintf(buf, sizeof(buf), "%s", lua_tostring(L, positive));
			break;
		case LUA_TFUNCTION:
			if (lua_iscfunction(L, positive)) {
				snprintf(buf, sizeof(buf), "C:%p", lua_tocfunction(L, positive));
			}
			break;
		}
		const char* typeName = lua_typename(L, type);
		aflog("%d/%d: type=%s value=%s\n", positive, negative, typeName, buf);
	}
}


static int CreateCppClassInstance(lua_State* L)
{
	aflDumpStack();
	const char* className = lua_tostring(L, lua_upvalueindex(1));
	lua_CFunction actualInstanceCreator = lua_tocfunction(L, lua_upvalueindex(2));
	actualInstanceCreator(L);
	aflDumpStack();
	luaL_getmetatable(L, className);
	aflDumpStack();
	lua_setmetatable(L, -2);
	aflDumpStack();
	return 1;
}

static void CreateClassMetatable(lua_State* L, const char* className, luaL_Reg methods[])
{
	int r = luaL_newmetatable(L, className);
	assert(r);

	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);

	luaL_setfuncs(L, methods, 0);
	lua_pop(L, 1);
}

static void CreateClassInstanceCreator(lua_State* L, const char* className, lua_CFunction creator)
{
	lua_pushstring(L, className);
	lua_pushcfunction(L, creator);
	aflDumpStack();
	lua_pushcclosure(L, CreateCppClassInstance, 2);
	aflDumpStack();
	lua_setglobal(L, className);
	aflDumpStack();
}

void aflBindClass(lua_State* L, const char* className, luaL_Reg methods[], lua_CFunction creator)
{
	CreateClassMetatable(L, className, methods);
	CreateClassInstanceCreator(L, className, creator);
}

void aflBindNamespace(lua_State* L, const char* nameSpace, luaL_Reg funcs[])
{
	lua_pushglobaltable(L);
	lua_pushstring(L, nameSpace);
	lua_newtable(L);
	luaL_setfuncs(L, funcs, 0);
	aflDumpStack();
	lua_settable(L, -3);
	lua_pop(L, 1);
	aflDumpStack();
}

bool aflDoFile(lua_State* L, const char* fileName)
{
	char* img = (char*)LoadFile(fileName);
	if (!img) {
		aflog("aflDoFile: could not load file %s\n", fileName);
		return false;
	}
	bool ok = true;
	if (luaL_loadbuffer(L, img, strlen(img), fileName)) {
		aflog("luaL_loadbuffer failed!\n%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		ok = false;
	}
	free(img);
	if (ok && lua_pcall(L, 0, LUA_MULTRET, 0)) {
		aflog("lua_pcall failed!\n%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		ok = false;
	}
	return ok;
}

int aflDoFileForReplace(lua_State* L)
{
	const char* fileName = lua_tostring(L, -1);
	char* img = (char*)LoadFile(fileName);
	if (!img) {
		luaL_error(L, "aflDoFile: could not load file %s", fileName);
		return false;
	}
	bool ok = true;
	if (luaL_loadbuffer(L, img, strlen(img), fileName)) {
		luaL_error(L, "luaL_loadbuffer failed!\n%s", lua_tostring(L, -1));
		lua_pop(L, 1);
		ok = false;
	}
	free(img);
	if (ok && lua_pcall(L, 0, LUA_MULTRET, 0)) {
		luaL_error(L, "lua_pcall failed!\n%s", lua_tostring(L, -1));
		lua_pop(L, 1);
		ok = false;
	}
	return lua_gettop(L) - 1;
}
