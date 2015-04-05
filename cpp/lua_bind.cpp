#include "stdafx.h"

lua_State *L;

static const char* myClassName = "RECT";

#define GET_RECT \
	RECT* r = (RECT*)luaL_checkudata(L, 1, myClassName); \
	if (!r) {	\
		return 0;	\
	} \


static int RECTToString(lua_State *L)
{
	GET_RECT
	char buf[64];
	sprintf_s(buf, sizeof(buf), "(%d, %d, %d, %d)", r->left, r->top, r->right, r->bottom);
	lua_pushstring(L, buf);
	return 1;
}

static int RECTIndex(lua_State *L)
{
	GET_RECT
	const char* key = lua_tostring(L, 2);
	aflDumpStack();
	if (!strcmp(key, "left")) {
		lua_pushnumber(L, r->left);
	}
	else if (!strcmp(key, "top")) {
		lua_pushnumber(L, r->top);
	}
	else if (!strcmp(key, "right")) {
		lua_pushnumber(L, r->right);
	}
	else if (!strcmp(key, "bottom")) {
		lua_pushnumber(L, r->bottom);
	}
	else {
		return 0;
	}
	return 1;
}

static int RECTNewIndex(lua_State *L)
{
	GET_RECT
	const char* key = lua_tostring(L, 2);
	LONG val = (LONG)lua_tointeger(L, 3);
	aflDumpStack();
	if (!strcmp(key, "left")) {
		r->left = val;
	} else if (!strcmp(key, "top")) {
		r->top = val;
	} else if (!strcmp(key, "right")) {
		r->right = val;
	} else if (!strcmp(key, "bottom")) {
		r->bottom = val;
	}
	return 0;
}

static int RECTNew(lua_State *L)
{
	int top = lua_gettop(L);
	RECT r;
	r.left = top < 4 ? 0 : (LONG)lua_tointeger(L, -4);
	r.top = top < 3 ? 0 : (LONG)lua_tointeger(L, -3);
	r.right = top < 2 ? 0 : (LONG)lua_tointeger(L, -2);
	r.bottom = top < 1 ? 0 : (LONG)lua_tointeger(L, -1);
	aflDumpStack();
	RECT* p = (RECT*)lua_newuserdata(L, sizeof(RECT));
	*p = r;
	aflDumpStack();
	luaL_getmetatable(L, myClassName);
	aflDumpStack();
	lua_setmetatable(L, -2);
	aflDumpStack();
	return 1;
}

static void BindMesBox()
{
	static luaL_Reg globalFuncs[] = {
		{ "MesBox", [](lua_State* L){ MessageBoxA(nullptr, lua_tostring(L, -1), "lambda box", MB_OK); return 0; } },
		{ nullptr, nullptr },
	};
	lua_pushglobaltable(L);
	aflDumpStack();
	luaL_setfuncs(L, globalFuncs, 0);
	lua_pop(L, 1);
	aflDumpStack();
}

void BindWin()
{
	aflDumpStack();
	int r = luaL_newmetatable(L, myClassName);
	assert(r);
	aflDumpStack();

	static struct luaL_Reg indexMethods[] =
	{
		{ "__index", RECTIndex },
		{ "__newindex", RECTNewIndex },
		{ "__tostring", RECTToString },
		{ nullptr, nullptr },
	};
	luaL_setfuncs(L, indexMethods, 0);
	aflDumpStack();

	lua_pop(L, 1);
	lua_register(L, myClassName, RECTNew);
}

void LuaBindTest()
{
	L = luaL_newstate();
	luaL_openlibs(L);
	BindWin();
	BindMesBox();
	//	luaL_dostring(L, "Printer()");

	if (luaL_dofile(L, "lua/start.lua")) {	// fopen/fclose to read this file
//	if (!aflDoFile(L, "lua/start.lua")) {	// LoadFile to read this file
		printf("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	int top = lua_gettop(L);
	if (top > 0) {
		printf("error! stack is not empty\n");
	}
	lua_close(L);
}
