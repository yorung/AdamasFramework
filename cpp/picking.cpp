#include "stdafx.h"

const static InputElement elements[] = {
	CInputElement("POSITION", SF_R32G32B32_FLOAT, 0),
	CInputElement("COLOR", SF_R32G32B32_FLOAT, 12),
};

struct Vertex {
	Vec3 pos;
	Vec3 color;
};

class Picking {
	VBOID vbo2d, vbo3d;
	VAOID vao2d, vao3d;
	AFRenderStates renderStates;
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
				{ "__gc", [](lua_State* L) { GET_PICKING p->~Picking(); return 0; } },
				{ nullptr, nullptr },
			};
			aflBindClass(L, CLASSNAME, methods, [](lua_State* L) { void* u = lua_newuserdata(L, sizeof(Picking)); new (u) Picking(); return 1; });
		});
	}
} static binder;

Picking::Picking()
{
	vbo2d = afCreateDynamicVertexBuffer(sizeof(Vertex) * 3);
	int strides = sizeof(Vertex);
	VBOID vboIds2d[] = { vbo2d };
	vao2d = afCreateVAO(elements, dimof(elements), 1, vboIds2d, &strides, IBOID());
	vbo3d = afCreateDynamicVertexBuffer(sizeof(Vertex) * 3);
	VBOID vboIds3d[] = { vbo3d };
	vao3d = afCreateVAO(elements, dimof(elements), 1, vboIds3d, &strides, IBOID());
	renderStates.Create("solid", dimof(elements), elements, BM_NONE, DSM_DEPTH_ENABLE, CM_DISABLE);
	Update();
}

Picking::~Picking()
{
	afSafeDeleteVAO(vao2d);
	afSafeDeleteVAO(vao3d);
	afSafeDeleteBuffer(vbo2d);
	afSafeDeleteBuffer(vbo3d);
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

bool RayVsTriangle(const Vec3& ray1, const Vec3& ray2, const Vec3 triangle[])
{
	Vec3 rayDir = ray2 - ray1;
	float inner[3];
	for (int i = 0; i < 3; i++) {
		const Vec3& a = triangle[i];
		const Vec3& b = triangle[(i + 1) % 3];
		inner[i] = dot(cross(ray1 - a, a - b), rayDir);
	}
	return (inner[0] < 0 && inner[1] < 0 && inner[2] < 0) || (inner[0] > 0 && inner[1] > 0 && inner[2] > 0);
}

void Picking::Update()
{
	Vertex v[3];
	float radian = (float)(std::fmod(GetTime(), 3) * M_PI * 2 / 3);
	float radius = 0.5f;
	float aspect = (float)systemMisc.GetScreenSize().x / systemMisc.GetScreenSize().y;
	for (int i = 0; i < 3; i++) {
		static Vec3 center(-0.7f, 0.7f, 0.f);
		radian += (float)M_PI * 2 / 3;
		v[i].pos = Vec3(std::sin(radian) / aspect, std::cos(radian), 0) * radius + center;
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
		Vec3 dir = v[(i + 1) % 3].pos - v[i].pos;
		Vec3 cursorDir = Vec3(curPosInNDC.x, curPosInNDC.y, 0) - v[i].pos;
		if (cross(dir, cursorDir).z > 0) {
			hit = false;
		}
	}

	for (int i = 0; i < 3; i++) {
		v[i].color = hit ? Vec3(i == 0, i == 1, i == 2) : Vec3(0.5, 0.5, 0.5);
	}

	afWriteBuffer(vbo2d, v, sizeof(v));

	radian = (float)(std::fmod(GetTime(), 10) * M_PI * 2 / 10);
	radius = 50.f;
	Vec3 triangle[3];
	for (int i = 0; i < 3; i++) {
		radian += (float)M_PI * 2 / 3;
		triangle[i] = v[i].pos = Vec3(std::sin(radian) * radius, std::cos(radian) * radius, 50.f);
		v[i].color = Vec3(0.5, 0.5, 0.5);
	}

	Vec3 n, f;
	ScreenPosToRay(systemMisc.GetMousePos(), n, f);
	hit = RayVsTriangle(n, f, triangle);
	for (int i = 0; i < 3; i++) {
		v[i].color = hit ? Vec3(i == 0, i == 1, i == 2) : Vec3(0.5, 0.5, 0.5);
	}

	afWriteBuffer(vbo3d, v, sizeof(v));
}

void Picking::Draw3D()
{
	renderStates.Apply();
	afBindVAO(vao3d);
	Mat mView, mProj;
	matrixMan.Get(MatrixMan::VIEW, mView);
	matrixMan.Get(MatrixMan::PROJ, mProj);
	Mat mVP = mView * mProj;
	UBOID ubo = afBindCbv0(&mVP, sizeof(Mat));
	afDraw(PT_TRIANGLESTRIP, 3);
	afSafeDeleteBuffer(ubo);
	afBindVAO(0);
}

void Picking::Draw2D()
{
	renderStates.Apply();
	afBindVAO(vao2d);
	Mat matIdentity;
	UBOID ubo = afBindCbv0(&matIdentity, sizeof(Mat));
	afDraw(PT_TRIANGLESTRIP, 3);
	afSafeDeleteBuffer(ubo);
	afBindVAO(0);
}
