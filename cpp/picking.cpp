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
	void Draw2D();
	void Draw3D();
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
				{ "Draw2D", [](lua_State* L) { GET_PICKING p->Draw2D(); return 0; } },
				{ "Draw3D", [](lua_State* L) { GET_PICKING p->Draw3D(); return 0; } },
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

void ScreenPosToRay(const Vec2& scrPos, Vec3& nearPos, Vec3& farPos)
{
	Mat v, p;
	matrixMan.Get(MatrixMan::VIEW, v);
	matrixMan.Get(MatrixMan::PROJ, p);
	Mat vp = makeViewportMatrix(systemMisc.GetScreenSize());
	Mat mInv = inv(v * p * vp);
	Vec4 nearPos4 = transform(Vec4(scrPos.x, scrPos.y, NDC_SPACE_NEAR, 1), mInv);
	Vec4 farPos4 = transform(Vec4(scrPos.x, scrPos.y, NDC_SPACE_FAR, 1), mInv);
	nearPos = Vec3(nearPos4.x, nearPos4.y, nearPos4.z) / nearPos4.w;
	farPos = Vec3(farPos4.x, farPos4.y, farPos4.z) / farPos4.w;
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
	}

	bool hit = true;

	Vec2 scrPos = systemMisc.GetScreenSize();
	Vec2 curPos = systemMisc.GetMousePos();

	Vec2 curPosInNDC = curPos / scrPos * Vec2(2, -2) + Vec2(-1, 1);

	// test
	Mat vp = makeViewportMatrix(scrPos);
	Vec3 transformed = transform(Vec3(curPos.x, curPos.y, 1), inv(vp));
	assert(std::abs(transformed.x - curPosInNDC.x) < 0.001f);
	assert(std::abs(transformed.y - curPosInNDC.y) < 0.001f);

	for (int i = 0; i < 3; i++) {
		Vec2 dir = v[(i + 1) % 3].pos - v[i].pos;
		Vec2 cursorDir = curPosInNDC - v[i].pos;
		if (cross2d(dir, cursorDir) > 0) {
			hit = false;
		}
	}

	for (int i = 0; i < 3; i++) {
		v[i].color = hit ? Vec3(i == 0, i == 1, i == 2) : Vec3(0.5, 0.5, 0.5);
	}

	afWriteBuffer(vbo, v, sizeof(v));
}

void Picking::Draw3D()
{
	Vec3 n, f;
	ScreenPosToRay(systemMisc.GetMousePos(), n, f);
	fontMan.DrawString(systemMisc.GetMousePos(), 15, SPrintf("near={%f,%f,%f} far={%f,%f,%f}", n.x, n.y, n.z, f.x, f.y, f.z));
}

void Picking::Draw2D()
{
	shaderMan.Apply(shader);
	VBOID vbos[] = {vbo};
	int strides[] = {sizeof(Vertex)};
	afSetVertexAttributes(elements, dimof(elements), 1, vbos, strides);
	afDrawTriangleStrip(3);




	// test
	ivec2 scrSize = systemMisc.GetScreenSize();
	float f = 1000;
	float n = 1;
	float aspect = (float)scrSize.x / scrSize.y;
	Mat proj = perspectiveLH(45.0f * (float)M_PI / 180.0f, aspect, n, f);

	Vec3 v1 = { 0, 0, 1 };
	Vec3 v2 = { 0, 0, 1000 };
	Vec3 v1trans = transform(v1, proj);
	Vec3 v2trans = transform(v2, proj);
	Vec4 v1m = transform(Vec4(v1.x, v1.y, v1.z, 1), proj);
	Vec4 v2m = transform(Vec4(v2.x, v2.y, v2.z, 1), proj);
}
