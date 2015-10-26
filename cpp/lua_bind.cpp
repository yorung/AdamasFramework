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

static const char* vecClassName = "Vec3";
static const char* voiceClassName = "Voice";
static const char* myClassName = "RECT";

static int LLookAt(lua_State* L)
{
	const Vec3* eye = (const Vec3*)luaL_checkudata(L, -2, vecClassName);
	const Vec3* at = (const Vec3*)luaL_checkudata(L, -1, vecClassName);
	if (!eye || !at) {
		return 0;
	}
	matrixMan.Set(MatrixMan::VIEW, lookat(*eye, *at, Vec3(0, 1, 0)));
	return 0;
}

static void LoadSkyBox(const char *fileName, const char* mappingType)
{
	SkyMan::MappingType type = SkyMan::CUBEMAP;
	if (mappingType) {
		if (!stricmp(mappingType, "photosphere")) {
			type = SkyMan::PHOTOSPHERE;
		}
	}
	skyMan.Create(fileName, type);
}

static ivec2 GetScreenPos()
{
	Mat mW, mV, mP;
	mW = luaMatrixStack.Get();
	matrixMan.Get(MatrixMan::VIEW, mV);
	matrixMan.Get(MatrixMan::PROJ, mP);
	Mat mViewport;
	Vec2 sz = (Vec2)systemMetrics.GetScreenSize() / 2;
	mViewport._11 = sz.x;
	mViewport._22 = -sz.y;
	mViewport._41 = sz.x;
	mViewport._42 = sz.y;
	Mat m = mW * mV * mP * mViewport;
	return ivec2((int)(m._41 / m._44), (int)(m._42 / m._44));
}

static ivec2 GetMousePos()
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(GetActiveWindow(), &p);
	return ivec2(p.x, p.y);
}

static void PushPoint(lua_State* L, const ivec2& pt)
{
	lua_newtable(L);
	lua_pushstring(L, "x");
	lua_pushinteger(L, pt.x);
	lua_rawset(L, -3);
	lua_pushstring(L, "y");
	lua_pushinteger(L, pt.y);
	lua_rawset(L, -3);
}

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

static void BindImage(lua_State* L)
{
	class Image
	{
		TexMan::TMID texId;
		std::vector<Vec4> quads;
	public:
		Image(const char *fileName)
		{
			texId = texMan.Create(fileName);
		}

		void SetQuad(int id, const Vec4& ltrb)
		{
			if (id < 0) {
				return;
			}
			if (id >= (int)quads.size()) {
				quads.resize(id + 1);
			}
			quads[id] = ltrb;
		}

		void Draw(int id, uint32_t color)
		{
			if (id < 0 || id >= (int)quads.size()) {
				return;
			}
			// WIP
		}
	};
	static const char* imageClassName = "Image";
#define GET_IMAGE \
	Image* p = (Image*)luaL_checkudata(L, 1, imageClassName); \
	if (!p) { return 0; }
	static struct luaL_Reg methods[] =
	{
		{ "__gc", [](lua_State* L) { GET_IMAGE p->~Image(); return 0; } },
		{ "DrawCell", [](lua_State* L) { GET_IMAGE p->Draw((int)lua_tointeger(L, 2), 0xffffffff); return 0; } },
		{ "SetCell", [](lua_State* L) { return 0; } },
		{ nullptr, nullptr },
	};
	aflBindClass(L, imageClassName, methods, [](lua_State* L) {new (lua_newuserdata(L, sizeof(Image))) Image(lua_tostring(L, -2)); return 1; });
}

static void BindMesh(lua_State* L)
{
	class LMesh
	{
		MeshMan::MMID mmid;
	public:
		LMesh(const char *fileName) {
			mmid = meshMan.Create(fileName);
		}
		void Draw(int animId, double time) {
			MeshX* mesh = (MeshX*)meshMan.Get(mmid);
			if (mesh) {
				MeshXAnimResult r;
				mesh->CalcAnimation(animId, time, r);
				mesh->Draw(r, luaMatrixStack.Get());
			}
		}
	};
	static const char* meshClassName = "Mesh";
#define GET_MESH \
		LMesh* p = (LMesh*)luaL_checkudata(L, 1, meshClassName); \
		if (!p) { return 0; }

	static struct luaL_Reg methods[] =
	{
		{ "__gc", [](lua_State* L) { GET_MESH p->~LMesh(); return 0; } },
		{ "Draw", [](lua_State* L) { GET_MESH p->Draw((int)lua_tointeger(L, -2), lua_tonumber(L, -1)); return 0; } },
		{ nullptr, nullptr },
	};
#undef GET_MESH
	aflBindClass(L, meshClassName, methods, [](lua_State* L) { new (lua_newuserdata(L, sizeof(LMesh))) LMesh(lua_tostring(L, -2)); return 1; });
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

static void BindGlobalFuncs(lua_State* L)
{
	void AddMenu(const char *name, const char *cmd);
	void PostCommand(const char* cmdString);
	static luaL_Reg globalFuncs[] = {
		{ "AddMenu", [](lua_State* L) { AddMenu(lua_tostring(L, -2), lua_tostring(L, -1)); return 0; } },
		{ "GetKeyCount", [](lua_State* L) { lua_pushinteger(L, inputMan.GetInputCount((int)lua_tointeger(L, -1))); return 1; } },
		{ "LookAt", LLookAt },
		{ "LoadSkyBox", [](lua_State* L) { LoadSkyBox(lua_tostring(L, 1), lua_tostring(L, 2)); return 0; } },
		{ "GetMousePos", [](lua_State* L) { PushPoint(L, GetMousePos()); return 1; } },
		{ "GetScreenPos", [](lua_State* L) { PushPoint(L, GetScreenPos()); return 1; } },
		{ "MessageBox", [](lua_State* L) { lua_pushstring(L, StrMessageBox(lua_tostring(L, -2), lua_tostring(L, -1))); return 1; } },
		{ "PostCommand", [](lua_State* L) { PostCommand(lua_tostring(L, -1)); return 0; } },
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

static int LVectorNew(lua_State* L)
{
	Vec3 v((float)lua_tonumber(L, -3), (float)lua_tonumber(L, -2), (float)lua_tonumber(L, -1));
	Vec3* p = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
	*p = v;
	luaL_getmetatable(L, vecClassName);
	lua_setmetatable(L, -2);
	return 1;
}

static int LVectorIndex(lua_State* L)
{
	const char* key = lua_tostring(L, -1);
	Vec3* src = (Vec3*)luaL_checkudata(L, -2, vecClassName);
	if (!key || !src) {
		return 0;
	}
	switch (*key) {
	case 'x': lua_pushnumber(L, src->x); return 1;
	case 'y': lua_pushnumber(L, src->y); return 1;
	case 'z': lua_pushnumber(L, src->z); return 1;
	}
	return 0;
}

static int LVectorNewIndex(lua_State *L)
{
	Vec3* self = (Vec3*)luaL_checkudata(L, -3, vecClassName);
	const char* key = lua_tostring(L, -2);
	float val = (float)lua_tonumber(L, -1);
	if (!self || !key) {
		return 0;	// error
	}
	switch (*key) {
	case 'x': self->x = val; break;
	case 'y': self->y = val; break;
	case 'z': self->z = val; break;
	}
	return 0;
}

static void BindVector(lua_State* L)
{
	luaL_newmetatable(L, vecClassName);
	static struct luaL_Reg methods[] =
	{
		{ "__index", LVectorIndex },
		{ "__newindex", LVectorNewIndex },
		{ nullptr, nullptr },
	};
	luaL_setfuncs(L, methods, 0);
	lua_pop(L, 1);
	lua_register(L, vecClassName, LVectorNew);
}

void ShareVariables(lua_State* L)
{
	RECT rc;
	GetClientRect(GetActiveWindow(), &rc);
	lua_pushinteger(L, rc.right - rc.left);
	lua_setglobal(L, "SCR_W");
	lua_pushinteger(L, rc.bottom - rc.top);
	lua_setglobal(L, "SCR_H");
	lua_pushinteger(L, 60);
	lua_setglobal(L, "FPS");
}

void LuaBind(lua_State* L)
{
	luaL_openlibs(L);
	BindWin(L);
	BindMesBox(L);
	BindMesh(L);
	BindVoice(L);
	BindVector(L);
	BindImage(L);
	BindMatrixStack(L);
	BindGlobalFuncs(L);
	ShareVariables(L);
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
