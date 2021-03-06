#include "stdafx.h"
#include "sky_man.h"
#include "voice.h"
#include "input_man.h"
#include "sprite_renderer.h"
#include "mesh_man.h"
#include "mesh_x.h"
#include "system_misc.h"
#include "lua_man.h"

SpriteCommands luaSpriteCommands;
ViewDesc* luaViewDesc;

#ifndef _MSC_VER
typedef int LONG;
struct RECT {
    int left;
    int top;
    int right;
    int bottom;
};
#endif

class MatrixStack
{
	std::stack<Mat> stack;
public:
	MatrixStack();
	void Reset();
	void Push();
	void Pop();
	void Mul(const Mat& m);
	const Mat& Get() const { return stack.top(); }
};

MatrixStack::MatrixStack()
{
	Reset();
}

void MatrixStack::Reset()
{
	std::stack<Mat> e;
	e.push(Mat());
	std::swap(stack, e);
}

void MatrixStack::Push()
{
	stack.push(stack.top());
}

void MatrixStack::Pop()
{
	assert(stack.size() > 1);
	if (stack.size() > 1) {
		stack.pop();
	}
}

void MatrixStack::Mul(const Mat& m)
{
	stack.top() = m * stack.top();
}

static const char* vec3ClassName = "Vec3";
static const char* vec4ClassName = "Vec4";
static const char* voiceClassName = "Voice";
static const char* rectClassName = "RECT";
static const char* matrixStackClassName = "MatrixStack";
const char* viewDescClassName = "ViewDesc";

static int LLookAt(lua_State* L)
{
	const Vec3* eye = (const Vec3*)luaL_checkudata(L, -2, vec3ClassName);
	const Vec3* at = (const Vec3*)luaL_checkudata(L, -1, vec3ClassName);
	if (!eye || !at) {
		return 0;
	}
	luaViewDesc->matView = lookatLH(*eye, *at, Vec3(0, 1, 0));
	return 0;
}

static IVec2 GetScreenPos(const MatrixStack* ms)
{
	Mat mW = ms ? ms->Get() : Mat();
	Mat mViewport = makeViewportMatrix(luaViewDesc->screenSize);
	Mat m = mW * luaViewDesc->matView * luaViewDesc->matProj * mViewport;
	return IVec2((int)(m._41 / m._44), (int)(m._42 / m._44));
}

#define GET_RECT \
	RECT* r = (RECT*)luaL_checkudata(L, 1, rectClassName); \
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
	luaL_getmetatable(L, rectClassName);
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

static void BindViewDesc(lua_State *L)
{
#define GET_VIEW_DESC \
		ViewDesc* p = (ViewDesc*)luaL_checkudata(L, 1, viewDescClassName); \
		if (!p) { return 0; }
	static luaL_Reg methods[] = {
		{ nullptr, nullptr },
	};
#undef GET_MATRIX_STACK
	aflBindClass(L, viewDescClassName, methods, [](lua_State* L) { void* u = lua_newuserdata(L, sizeof(ViewDesc)); new (u) ViewDesc(); return 1; });

}

static void BindMatrixStack(lua_State *L)
{
	#define GET_MATRIX_STACK \
		MatrixStack* p = (MatrixStack*)luaL_checkudata(L, 1, matrixStackClassName); \
		if (!p) { return 0; }
	static luaL_Reg methods[] = {
		{ "Push", [](lua_State* L) { GET_MATRIX_STACK p->Push(); return 0; } },
		{ "Pop", [](lua_State* L) { GET_MATRIX_STACK p->Pop(); return 0; } },
		{ "RotateX", [](lua_State* L) { GET_MATRIX_STACK p->Mul(q2m(Quat(Vec3(1, 0, 0), (float)lua_tonumber(L, -1) * (float)M_PI / 180))); return 0; } },
		{ "RotateY", [](lua_State* L) { GET_MATRIX_STACK p->Mul(q2m(Quat(Vec3(0, 1, 0), (float)lua_tonumber(L, -1) * (float)M_PI / 180))); return 0; } },
		{ "RotateZ", [](lua_State* L) { GET_MATRIX_STACK p->Mul(q2m(Quat(Vec3(0, 0, 1), (float)lua_tonumber(L, -1) * (float)M_PI / 180))); return 0; } },
		{ "Scale", [](lua_State* L) { GET_MATRIX_STACK p->Mul(scale((float)lua_tonumber(L, -3), (float)lua_tonumber(L, -2), (float)lua_tonumber(L, -1))); return 0; } },
		{ "Translate", [](lua_State* L) { GET_MATRIX_STACK p->Mul(translate((float)lua_tonumber(L, -3), (float)lua_tonumber(L, -2), (float)lua_tonumber(L, -1))); return 0; } },
		{ nullptr, nullptr },
	};
	#undef GET_MATRIX_STACK
	aflBindClass(L, matrixStackClassName, methods, [](lua_State* L) { void* u = lua_newuserdata(L, sizeof(MatrixStack)); new (u) MatrixStack(); return 1; });
}

static void BindImage(lua_State* L)
{
	class Image
	{
		AFTexRef texId;
		TexDesc desc;
		std::vector<Vec4> quads;
#ifdef _DEBUG
		std::string fileName;
#endif
	public:
		~Image()
		{
			afSafeDeleteTexture(texId);
		}

		Image(const char *fileName)
		{
			texId = afLoadTexture(fileName, desc);
#ifdef _DEBUG
			this->fileName = fileName;
#endif
		}

		void SetQuad(int id, const Vec4& ltrb)
		{
			if (id < 0) {
				return;
			}
			if (id >= (int)quads.size()) {
				quads.resize(id + 1);
			}
			quads[id] = Vec4(ltrb.x / desc.size.x, ltrb.y / desc.size.y, ltrb.z / desc.size.x, ltrb.w / desc.size.y);
		}

		void Draw(lua_State*, const MatrixStack* matrixStack, int id, const Vec4* color)
		{
			if (id < 0 || id >= (int)quads.size()) {
				return;
			}
			SpriteCommand s;
			s.quad = quads[id];
			s.matW = matrixStack ? matrixStack->Get() : Mat();
			s.matW = scale((float)desc.size.x * (s.quad.z - s.quad.x), (float)desc.size.y * (s.quad.w - s.quad.y), 1.0f) * s.matW;
			s.tex = texId;
			s.color = color ? Vec4ToUnorm(*color) : 0xffffffff;
			luaSpriteCommands.push_back(s);
		}
	};
	static const char* imageClassName = "Image";
#define GET_IMAGE \
	Image* p = (Image*)luaL_checkudata(L, 1, imageClassName); \
	if (!p) { return 0; }
	static struct luaL_Reg methods[] =
	{
		{ "__gc", [](lua_State* L) { GET_IMAGE p->~Image(); return 0; } },

		{ "DrawCell", [](lua_State* L) {
			GET_IMAGE
			p->Draw(L, (MatrixStack*)luaL_testudata(L, 2, matrixStackClassName), (int)lua_tointeger(L, 3), (Vec4*)luaL_testudata(L, 4, vec4ClassName));
			return 0;
		} },

		{ "SetCell", [](lua_State* L) {
			GET_IMAGE
//			const RECT* r = (RECT*)luaL_checkudata(L, -1, rectClassName);
//			p->SetQuad((int)lua_tointeger(L, -2), Vec4((float)r->left, (float)r->top, (float)r->right, (float)r->bottom));
//			return 0;

			int id = (int)lua_tointeger(L, 2);

			Vec4 ltrb;
			lua_pushstring(L, "left");
			lua_gettable(L, -2);
			ltrb.x = (float)lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_pushstring(L, "top");
			lua_gettable(L, -2);
			ltrb.y = (float)lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_pushstring(L, "right");
			lua_gettable(L, -2);
			ltrb.z = (float)lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_pushstring(L, "bottom");
			lua_gettable(L, -2);
			ltrb.w = (float)lua_tonumber(L, -1);
			lua_pop(L, 1);

			p->SetQuad(id, ltrb);
			return 0;
		} },
		{ nullptr, nullptr },
	};
	aflBindClass(L, imageClassName, methods, [](lua_State* L) { void* u = lua_newuserdata(L, sizeof(Image)); new (u) Image(lua_tostring(L, -2)); return 1; });
}

static void BindMesh(lua_State* L)
{
	class LMesh
	{
		MeshMan::MMID mmid;
	public:
		LMesh(const char *fileName)
		{
			mmid = meshMan.Create(fileName);
		}
		void Draw(lua_State*, const MatrixStack* m, int animId, double time)
		{
			MeshX* mesh = (MeshX*)meshMan.Get(mmid);
			if (!mesh) {
				return;
			}
			MeshXAnimResult r;
			mesh->CalcAnimation(animId, time, r);
			mesh->Draw(*luaMan.GetViewDesc(), r, m ? m->Get() : Mat());
		}
	};
	static const char* meshClassName = "Mesh";
#define GET_MESH \
		LMesh* p = (LMesh*)luaL_checkudata(L, 1, meshClassName); \
		if (!p) { return 0; }

	static struct luaL_Reg methods[] =
	{
		{ "__gc", [](lua_State* L) { GET_MESH p->~LMesh(); return 0; } },
		{ "Draw", [](lua_State* L) { GET_MESH p->Draw(L, (MatrixStack*)luaL_testudata(L, 2, matrixStackClassName), (int)lua_tointeger(L, -2), lua_tonumber(L, -1)); return 0; } },
		{ nullptr, nullptr },
	};
#undef GET_MESH
	aflBindClass(L, meshClassName, methods, [](lua_State* L) { void* u = lua_newuserdata(L, sizeof(LMesh));	new (u) LMesh(lua_tostring(L, -2)); return 1; });
}


void BindWin(lua_State *L)
{
	aflDumpStack();
	int r = luaL_newmetatable(L, rectClassName);
	afVerify(r);
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
	lua_register(L, rectClassName, RECTNew);
}

static void BindGlobalFuncs(lua_State* L)
{
	static luaL_Reg globalFuncs[] = {
		{ "PlayBgm", [](lua_State* L) { PlayBgm(lua_tostring(L, 1)); return 0; } },
		{ "AddMenu", [](lua_State* L) { AddMenu(lua_tostring(L, -2), lua_tostring(L, -1)); return 0; } },
		{ "GetKeyCount", [](lua_State* L) { lua_pushinteger(L, inputMan.GetInputCount((int)lua_tointeger(L, -1))); return 1; } },
		{ "LookAt", LLookAt },
		{ "LoadSkyBox", [](lua_State* L) { skyMan.Create(lua_tostring(L, 1), lua_tostring(L, 2)); return 0; } },
		{ "GetMousePos", [](lua_State* L) { aflPushIVec2(L, systemMisc.GetMousePos()); return 1; } },
		{ "GetScreenPos", [](lua_State* L) { aflPushIVec2(L, GetScreenPos((MatrixStack*)luaL_testudata(L, 1, matrixStackClassName))); return 1; } },
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
	aflBindClass(L, voiceClassName, methods, [](lua_State* L) { void* u = lua_newuserdata(L, sizeof(Voice)); new (u) Voice(lua_tostring(L, -2)); return 1; });
}

static int LVec3New(lua_State* L)
{
	Vec3 v((float)lua_tonumber(L, -3), (float)lua_tonumber(L, -2), (float)lua_tonumber(L, -1));
	Vec3* p = (Vec3*)lua_newuserdata(L, sizeof(Vec3));
	*p = v;
	luaL_getmetatable(L, vec3ClassName);
	lua_setmetatable(L, -2);
	return 1;
}

static int LVec3Index(lua_State* L)
{
	const char* key = lua_tostring(L, -1);
	Vec3* src = (Vec3*)luaL_checkudata(L, -2, vec3ClassName);
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

static int LVec3NewIndex(lua_State *L)
{
	Vec3* self = (Vec3*)luaL_checkudata(L, -3, vec3ClassName);
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

static void BindVec3(lua_State* L)
{
	luaL_newmetatable(L, vec3ClassName);
	static struct luaL_Reg methods[] =
	{
		{ "__index", LVec3Index },
		{ "__newindex", LVec3NewIndex },
		{ nullptr, nullptr },
	};
	luaL_setfuncs(L, methods, 0);
	lua_pop(L, 1);
	lua_register(L, vec3ClassName, LVec3New);
}

static int LVec4New(lua_State* L)
{
	Vec4 v((float)lua_tonumber(L, -4), (float)lua_tonumber(L, -3), (float)lua_tonumber(L, -2), (float)lua_tonumber(L, -1));
	Vec4* p = (Vec4*)lua_newuserdata(L, sizeof(Vec4));
	*p = v;
	luaL_getmetatable(L, vec4ClassName);
	lua_setmetatable(L, -2);
	return 1;
}

static int LVec4Index(lua_State* L)
{
	const char* key = lua_tostring(L, -1);
	Vec4* src = (Vec4*)luaL_checkudata(L, -2, vec4ClassName);
	if (!key || !src) {
		return 0;
	}
	switch (*key) {
	case 'x': lua_pushnumber(L, src->x); return 1;
	case 'y': lua_pushnumber(L, src->y); return 1;
	case 'z': lua_pushnumber(L, src->z); return 1;
	case 'w': lua_pushnumber(L, src->w); return 1;
	}
	return 0;
}

static int LVec4NewIndex(lua_State *L)
{
	Vec4* self = (Vec4*)luaL_checkudata(L, -3, vec4ClassName);
	const char* key = lua_tostring(L, -2);
	float val = (float)lua_tonumber(L, -1);
	if (!self || !key) {
		return 0;	// error
	}
	switch (*key) {
	case 'x': self->x = val; break;
	case 'y': self->y = val; break;
	case 'z': self->z = val; break;
	case 'w': self->w = val; break;
	}
	return 0;
}

static void BindVec4(lua_State* L)
{
	luaL_newmetatable(L, vec4ClassName);
	static struct luaL_Reg methods[] =
	{
		{ "__index", LVec4Index },
		{ "__newindex", LVec4NewIndex },
		{ nullptr, nullptr },
	};
	luaL_setfuncs(L, methods, 0);
	lua_pop(L, 1);
	lua_register(L, vec4ClassName, LVec4New);
}

static void ShareVariables(lua_State* L)
{
	IVec2 sz = systemMisc.GetScreenSize();
	lua_pushinteger(L, sz.x);
	lua_setglobal(L, "SCR_W");
	lua_pushinteger(L, sz.y);
	lua_setglobal(L, "SCR_H");
	lua_pushinteger(L, 60);
	lua_setglobal(L, "FPS");
}

static void ReplaceLuaStandardLibraryFunctions(lua_State* L)
{
	lua_register(L, "print", aflPrintForReplace);
	lua_register(L, "dofile", aflDoFileForReplace);
	luaL_dostring(L, "function require(m)\n"
		"package.loaded[m] = package.loaded[m] or dofile(m..'.lua') or true\n"
		"return package.loaded[m]\n"
		"end\n");
}

void LuaBind(lua_State* L)
{
	luaL_openlibs(L);
	BindWin(L);
	BindMesBox(L);
	BindMesh(L);
	BindVoice(L);
	BindVec3(L);
	BindVec4(L);
	BindImage(L);
	BindMatrixStack(L);
	BindViewDesc(L);
	BindGlobalFuncs(L);
	ReplaceLuaStandardLibraryFunctions(L);
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
