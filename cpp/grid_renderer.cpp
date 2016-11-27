#include "stdafx.h"

class GridRenderer
{
	VBOID vbo;
	IBOID ibo;
	AFRenderStates renderStates;
	int lines;
	int numGrid;
	float pitch;
public:
	GridRenderer(int numGrid, float pitch);
	~GridRenderer();
	void Draw();
	bool GetMousePosInGrid(Vec2& v);
};

#define GET_GR \
	GridRenderer* p = (GridRenderer*)luaL_checkudata(L, 1, "GridRenderer");	\
	if (!p) {	\
		return 0;	\
	}

class GridRendererBinder {
public:
	GridRendererBinder() {
		GetLuaBindFuncContainer().push_back([](lua_State* L) {
			static luaL_Reg methods[] = {
				{ "__gc", [](lua_State* L) { GET_GR p->~GridRenderer(); return 0; } },
				{ "Draw", [](lua_State* L) { GET_GR p->Draw(); return 0; } },
				{ "GetMousePosInGrid", [](lua_State* L) {
					GET_GR
					Vec2 v;
					if (p->GetMousePosInGrid(v)) {
						aflPushVec2(L, v);
						return 1;
					}
					return 0; } },
				{ nullptr, nullptr },
			};
			aflBindClass(L, "GridRenderer", methods, [](lua_State* L) { void* u = lua_newuserdata(L, sizeof(GridRenderer)); new (u) GridRenderer((int)lua_tointeger(L, 1), (float)lua_tonumber(L, 2)); return 1; });
		});
	}
} static gridRendererBinder;

struct GridVert {
	Vec3 pos;
	Vec3 color;
};

GridRenderer::~GridRenderer()
{
	renderStates.Destroy();
	afSafeDeleteBuffer(ibo);
	afSafeDeleteBuffer(vbo);

	assert(!vbo);
	assert(!ibo);
}

GridRenderer::GridRenderer(int numGrid_, float pitch_)
{
	std::vector<GridVert> vert;
	std::vector<AFIndex> indi;

	auto add = [&](float x, float z) {
		GridVert v;
		v.color = Vec3(0.5, 0.5, 0.5);
		v.pos = Vec3(x, 0, z);
		vert.push_back(v);
		indi.push_back((AFIndex)indi.size());
	};

	numGrid = numGrid_;
	pitch = pitch_;
	float half = pitch * numGrid / 2;
	for(int i = 0; i <= numGrid; i++) {
		add(i * pitch - half, -half);
		add(i * pitch - half, half);
		add(-half, i * pitch - half);
		add(half, i * pitch - half);
	}

	int sizeVertices = vert.size() * sizeof(GridVert);
	lines = indi.size() / 2;

	const static InputElement layout[] =
	{
		AF_INPUT_ELEMENT(0, "POSITION", AFF_R32G32B32_FLOAT, 0),
		AF_INPUT_ELEMENT(1, "COLOR", AFF_R32G32B32_FLOAT, 12),
	};
	renderStates.Create("solid", arrayparam(layout), AFRS_DEPTH_ENABLE | AFRS_PRIMITIVE_LINELIST);

	vbo = afCreateVertexBuffer(sizeVertices, &vert[0]);
	ibo = afCreateIndexBuffer(indi.size(), &indi[0]);
}

void GridRenderer::Draw()
{
	AFCommandList& cmd = afGetCommandList();
	cmd.SetRenderStates(renderStates);
	Mat matView, matProj;
	matrixMan.Get(MatrixMan::VIEW, matView);
	matrixMan.Get(MatrixMan::PROJ, matProj);
	Mat matVP = matView * matProj;
#ifdef AF_GLES
	afHandleGLError(glUniform4fv(glGetUniformLocation(renderStates.GetShaderId(), "b0"), sizeof(Mat) / 16, (GLfloat*)&matVP));
#else
	cmd.SetBuffer(sizeof(Mat), &matVP, 0);
#endif
	cmd.SetVertexBuffer(vbo, sizeof(GridVert));
	cmd.SetIndexBuffer(ibo);
	cmd.Draw(lines * 2);

#ifndef NDEBUG
	Vec2 v;
	if (GetMousePosInGrid(v)) {
		fontMan.DrawString(systemMisc.GetMousePos(), 15, SPrintf("hit={%f,%f}", v.x, v.y));
	}
	fontMan.DrawString(IVec2(5, 70), 15, SPrintf("scr pos={%d,%d}", systemMisc.GetMousePos().x, systemMisc.GetMousePos().y));
	fontMan.DrawString(IVec2(5, 90), 15, SPrintf("scr size={%d,%d}", systemMisc.GetScreenSize().x, systemMisc.GetScreenSize().y));
#endif
}

void ScreenPosToRay(const Vec2& scrPos, Vec3& nearPos, Vec3& farPos);

bool GridRenderer::GetMousePosInGrid(Vec2& v)
{
	// make a ray from cursor pos
	Vec3 n, f;
	ScreenPosToRay(systemMisc.GetMousePos(), n, f);

	// ray-grid intersection
	Vec3 planeCenter = { 0, 0, 0 };
	Vec3 planeNormal = { 0, 1, 0 };
	float nDotPlane = dot(n, planeNormal);
	float fDotPlane = dot(f, planeNormal);
	if (nDotPlane * fDotPlane < 0) {
		nDotPlane = std::abs(nDotPlane);
		fDotPlane = std::abs(fDotPlane);
		Vec3 hitPos = n + (f - n) * nDotPlane / (nDotPlane + fDotPlane);
		float half = pitch * numGrid / 2;
		v = Vec2(dot(hitPos, Vec3(1, 0, 0)), dot(hitPos, Vec3(0, 0, 1)));
		return true;
	}
	return false;
}
