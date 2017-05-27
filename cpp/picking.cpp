#include "stdafx.h"

const static InputElement elements[] =
{
	AF_INPUT_ELEMENT(0, "POSITION", AFF_R32G32B32_FLOAT, 0),
	AF_INPUT_ELEMENT(1, "COLOR", AFF_R32G32B32_FLOAT, 12),
};

struct Vertex
{
	Vec3 pos;
	Vec3 color;
};

class Picking
{
	AFRenderStates polygonRenderStates;
	AFRenderStates lineRenderStates;
public:
	Picking();
	void Update2D(Vertex v[3]);
	void Update3D(Vertex poly[3], Vertex lines[6]);
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
			static luaL_Reg methods[] =
			{
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
	polygonRenderStates.Create("solid", arrayparam(elements), AFRS_DEPTH_ENABLE | AFRS_WIREFRAME);
	lineRenderStates.Create("solid", arrayparam(elements), AFRS_DEPTH_ENABLE | AFRS_PRIMITIVE_LINELIST);
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

bool RayVsTriangleMollerTrumbore(const Vec3& ray1, const Vec3& ray2, const Vec3 triangle[], Vec3& hitPos)
{
	Vec3 ray1relative = ray1 - triangle[0];
	Vec3 axes[] = { triangle[1] - triangle[0], triangle[2] - triangle[0], ray1 - ray2 };
	Vec3 crosses[] = { cross(axes[1], axes[2]), cross(axes[2], axes[0]), cross(axes[0], axes[1]) };
	Vec3 uvt = Vec3(dot(ray1relative, crosses[0]), dot(ray1relative, crosses[1]), dot(ray1relative, crosses[2])) / std::max(dot(axes[0], crosses[0]), 0.000001f);
	hitPos = lerp(ray1, ray2, uvt.z);
	return uvt.x >= 0 && uvt.y >= 0 && (uvt.x + uvt.y) <= 1.f;
}

void Picking::Update2D(Vertex v[3])
{
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

	Vec2 curPosIn2D = curPos / scrPos * Vec2(2, -2) + Vec2(-1, 1);

	Mat proj2d;
#ifdef AF_VULKAN
	proj2d._22 = -1;
#endif
	Mat projVp = proj2d * makeViewportMatrix(scrPos);
	Vec3 transformed = transform(Vec3(curPos.x, curPos.y, 1), inv(projVp));
	assert(std::abs(transformed.x - curPosIn2D.x) < 0.001f);
	assert(std::abs(transformed.y - curPosIn2D.y) < 0.001f);

	for (int i = 0; i < 3; i++) {
		Vec3 dir = v[(i + 1) % 3].pos - v[i].pos;
		Vec3 cursorDir = Vec3(curPosIn2D.x, curPosIn2D.y, 0) - v[i].pos;
		if (cross(dir, cursorDir).z > 0) {
			hit = false;
		}
	}

	for (int i = 0; i < 3; i++) {
		v[i].color = hit ? Vec3(i == 0, i == 1, i == 2) : Vec3(0.5, 0.5, 0.5);
	}
}

void Picking::Update3D(Vertex poly[3], Vertex lines[6])
{
	float radian = (float)(std::fmod(GetTime(), 10) * M_PI * 2 / 10);
	float radius = 50.f;
	Vec3 triangle[3];
	for (int i = 0; i < 3; i++) {
		radian += (float)M_PI * 2 / 3;
		triangle[i] = poly[i].pos = Vec3(std::sin(radian) * radius, std::cos(radian) * radius, 50.f);
		poly[i].color = Vec3(0.5, 0.5, 0.5);
	}

	Vec3 n, f;
	ScreenPosToRay(systemMisc.GetMousePos(), n, f);
	Vec3 hitPos;
	bool hit = RayVsTriangleMollerTrumbore(n, f, triangle, hitPos);
	for (int i = 0; i < 3; i++)
	{
		poly[i].color = hit ? Vec3(i == 0, i == 1, i == 2) : Vec3(0.5, 0.5, 0.5);
	}

	for (int i = 0; i < 3; i++)
	{
		Vec3 axis = Vec3(i == 0, i == 1, i == 2);
		lines[i * 2].color = lines[i * 2 + 1].color = axis;
		lines[i * 2].pos = hitPos + axis * 10;
		lines[i * 2 + 1].pos = hitPos + axis * -10;
	}
}

static void DrawDynamicVertexBuffer(AFCommandList& cmd, int nVertices, Vertex vertices[])
{
	cmd.SetVertexBuffer(sizeof(Vertex) * nVertices, vertices, sizeof(Vertex));
	cmd.Draw(nVertices);
}

void Picking::Draw3D()
{
	Vertex poly[3], lines[6];
	Update3D(poly, lines);
	Mat mView, mProj;
	matrixMan.Get(MatrixMan::VIEW, mView);
	matrixMan.Get(MatrixMan::PROJ, mProj);
	Mat mVP = mView * mProj;

	AFCommandList& cmd = afGetCommandList();
	cmd.SetRenderStates(polygonRenderStates);
	cmd.SetBuffer(sizeof(Mat), &mVP, 0);
	DrawDynamicVertexBuffer(cmd, arrayparam(poly));

	cmd.SetRenderStates(lineRenderStates);
	cmd.SetBuffer(sizeof(Mat), &mVP, 0);
	DrawDynamicVertexBuffer(cmd, arrayparam(lines));
}

void Picking::Draw2D()
{
	AFCommandList& cmd = afGetCommandList();
	cmd.SetRenderStates(polygonRenderStates);
	Vertex v[3];
	Update2D(v);
	Mat proj2d;
#ifdef AF_VULKAN
	proj2d._22 = -1;
#endif
	cmd.SetBuffer(sizeof(Mat), &proj2d, 0);
	DrawDynamicVertexBuffer(cmd, arrayparam(v));
}
