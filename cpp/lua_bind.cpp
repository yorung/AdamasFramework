#include "stdafx.h"

MatrixStack luaMatrixStack;

#ifndef _MSC_VER
typedef int LONG;
struct RECT {
    int left;
    int top;
    int right;
    int bottom;
};
#endif

static const char* voiceClassName = "Voice";
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
	snprintf(buf, sizeof(buf), "(%d, %d, %d, %d)", r->left, r->top, r->right, r->bottom);
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

static void BindMesBox(lua_State *L)
{
	static luaL_Reg globalFuncs[] = {
#ifdef _MSC_VER
		{ "MesBox", [](lua_State* L){ MessageBoxA(nullptr, lua_tostring(L, -1), "lambda box", MB_OK); return 0; } },
#else
		{ "MesBox", [](lua_State* L){ Toast(lua_tostring(L, -1)); return 0; } },
#endif
		{ nullptr, nullptr },
	};
	lua_pushglobaltable(L);
	aflDumpStack();
	luaL_setfuncs(L, globalFuncs, 0);
	lua_pop(L, 1);
	aflDumpStack();
}

static void BindMatrixStack(lua_State *L)
{
	static luaL_Reg inNamespaceFuncs[] = {
		{ "Push", [](lua_State* L) { luaMatrixStack.Push(); return 0; } },
		{ "Pop", [](lua_State* L) { luaMatrixStack.Pop(); return 0; } },
		{ "RotateX", [](lua_State* L) { luaMatrixStack.Mul(q2m(Quat(Vec3(1, 0, 0), (float)lua_tonumber(L, -1) * (float)M_PI / 180))); return 0; } },
		{ "RotateY", [](lua_State* L) { luaMatrixStack.Mul(q2m(Quat(Vec3(0, 1, 0), (float)lua_tonumber(L, -1) * (float)M_PI / 180))); return 0; } },
		{ "RotateZ", [](lua_State* L) { luaMatrixStack.Mul(q2m(Quat(Vec3(0, 0, 1), (float)lua_tonumber(L, -1) * (float)M_PI / 180))); return 0; } },
		{ "Scale", [](lua_State* L) { luaMatrixStack.Mul(scale((float)lua_tonumber(L, -3), (float)lua_tonumber(L, -2), (float)lua_tonumber(L, -1))); return 0; } },
		{ "Translate", [](lua_State* L) { luaMatrixStack.Mul(translate((float)lua_tonumber(L, -3), (float)lua_tonumber(L, -2), (float)lua_tonumber(L, -1))); return 0; } },
		{ nullptr, nullptr },
	};
	aflBindNamespace(L, "matrixStack", inNamespaceFuncs);
}

static void BindMeshMan(lua_State* L)
{
	static luaL_Reg inNamespaceFuncs[] = {
		{ "Create", [](lua_State* L) { lua_pushinteger(L, meshMan.Create(lua_tostring(L, -1))); return 1; } },
		{ "Draw", [](lua_State* L) {
			MeshX* mesh = (MeshX*)meshMan.Get((MMID)lua_tointeger(L, -1));
			if (mesh) {
				MeshXAnimResult r;
				mesh->CalcAnimation(0, GetTime(), r);
				mesh->Draw(r, luaMatrixStack.Get());
			}
			return 0; } },
		{ nullptr, nullptr },
	};
	aflBindNamespace(L, "meshMan", inNamespaceFuncs);
}

void BindWin(lua_State *L)
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

static const char* IdToStr(int id)
{
	switch (id) {
	case IDOK: return "ok";
	case IDCANCEL: return "cancel";
	case IDYES: return "yes";
	case IDNO: return "no";
	}
	return "unknown";
}

static UINT StrToType(const char* type)
{
	if (!strcmp(type, "okcancel")) {
		return MB_OKCANCEL;
	} else if (!strcmp(type, "yesno")) {
		return MB_YESNO;
	}
	return MB_OK;
}

static const char* StrMessageBox(const char* txt, const char* type)
{
	return IdToStr(MessageBoxA(GetActiveWindow(), txt, "Lua Message", StrToType(type)));
}

void AddMenu(const char *name, const char *cmd);

static void BindGlobalFuncs(lua_State* L)
{
	static luaL_Reg globalFuncs[] = {
		{ "AddMenu", [](lua_State* L) { AddMenu(lua_tostring(L, -2), lua_tostring(L, -1)); return 0; } },
		{ "GetKeyCount", [](lua_State* L) { lua_pushinteger(L, inputMan.GetInputCount((int)lua_tointeger(L, -1))); return 1; } },
		{ "MessageBox", [](lua_State* L) { lua_pushstring(L, StrMessageBox(lua_tostring(L, -2), lua_tostring(L, -1))); return 1; } },
		{ nullptr, nullptr },
	};

	lua_pushglobaltable(L);
	luaL_setfuncs(L, globalFuncs, 0);
	lua_pop(L, 1);
}

#define GET_VOICE \
	Voice* p = (Voice*)luaL_checkudata(L, 1, voiceClassName); \
	if (!p) { return 0; }

static void BindVoice(lua_State* L)
{
	static struct luaL_Reg methods[] =
	{
		{ "__gc", [](lua_State* L) { GET_VOICE p->~Voice(); return 0; } },
		{ "Play", [](lua_State* L) { GET_VOICE p->Play(!!lua_toboolean(L, 2)); return 0; } },
		{ "Stop", [](lua_State* L) { GET_VOICE p->Stop(); return 0; } },
		{ nullptr, nullptr },
	};
	aflBindClass(L, voiceClassName, methods, [](lua_State* L) { new (lua_newuserdata(L, sizeof(Voice))) Voice(lua_tostring(L, -2)); return 1; });
}

void LuaBind(lua_State* L)
{
	luaL_openlibs(L);
	BindWin(L);
	BindMesBox(L);
	BindMeshMan(L);
	BindVoice(L);
	BindMatrixStack(L);
	BindGlobalFuncs(L);
}

void LuaBindTest()
{
	lua_State* L = luaL_newstate();
	LuaBind(L);
	//	luaL_dostring(L, "Printer()");

//	if (luaL_dofile(L, "lua/test.lua")) {	// fopen/fclose to read this file
	if (!aflDoFile(L, "lua/test.lua")) {	// LoadFile to read this file
		printf("%s\n", lua_tostring(L, -1));
    	lua_pop(L, 1);
	}
	int top = lua_gettop(L);
	if (top > 0) {
		printf("error! stack is not empty\n");
	}
	lua_close(L);
}
