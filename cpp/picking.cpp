#include "stdafx.h"

static InputElement elements[] = {
	CInputElement(0, "POSITION", SF_R32G32_FLOAT, 0),
	CInputElement(0, "COLOR", SF_R32G32B32_FLOAT, 8),
};

struct Vertex {
	Vec2 pos;
	Vec3 color;
};

class Picking {
	VBOID vbo;
	ShaderMan::SMID shader;
public:
	Picking();
	~Picking();
	void Update();
	void Draw();
};

#define CLASSNAME "Picking"
#define GET_PICKING \
	Picking* p = (Picking*)luaL_checkudata(L, 1, CLASSNAME);	\
	if (!p) {	\
		return 0;	\
	}

class PickingBinder {
public:
	PickingBinder() {
		GetLuaBindFuncContainer().push_back([](lua_State* L) {
			static luaL_Reg methods[] = {
				{ "Update", [](lua_State* L) { GET_PICKING p->Update(); return 0; } },
				{ "Draw", [](lua_State* L) { GET_PICKING p->Draw(); return 0; } },
				{ nullptr, nullptr },
			};
			aflBindClass(L, CLASSNAME, methods, [](lua_State* L) { void* u = lua_newuserdata(L, sizeof(Picking)); new (u) Picking(); return 1; });
		});
	}
} static binder;

Picking::Picking()
{
	vbo = afCreateDynamicVertexBuffer(sizeof(Vertex) * 3);
	shader = shaderMan.Create("solid", elements, dimof(elements), BM_NONE, DSM_DISABLE, CM_DISABLE);
	Update();
}

Picking::~Picking()
{
	afSafeDeleteBuffer(vbo);
}

void Picking::Update()
{
	Vertex v[3];
	float radian = (float)(std::fmod(GetTime(), 3) * M_PI * 2 / 3);
	float radius = 0.5f;
	float aspect = (float)systemMisc.GetScreenSize().x / systemMisc.GetScreenSize().y;
	for (int i = 0; i < 3; i++) {
		radian += (float)M_PI * 2 / 3;
		v[i].pos = Vec2(std::sin(radian) / aspect, std::cos(radian)) * radius;
		v[i].color = Vec3(i == 0, i == 1, i == 2);
	}
	afWriteBuffer(vbo, v, sizeof(v));
}

void Picking::Draw()
{
	shaderMan.Apply(shader);
	VBOID vbos[] = {vbo};
	int strides[] = {sizeof(Vertex)};
	afSetVertexAttributes(elements, dimof(elements), 1, vbos, strides);
	afDrawTriangleStrip(3);
}
