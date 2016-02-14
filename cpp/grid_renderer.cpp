#include "stdafx.h"

class GridRenderer
{
	UBOID ubo;
	VBOID vbo;
	IBOID ibo;
	VAOID vao;
	ShaderMan::SMID shaderId;
	int lines;
	int numGrid;
	float pitch;
public:
	GridRenderer(int numGrid, float pitch);
	~GridRenderer();
	void Draw();
	IVec2 GetMousePosInGrid();
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
				{ "Draw", [](lua_State* L) { GET_GR p->Draw(); return 0; } },
				{ "GetMousePosInGrid", [](lua_State* L) { GET_GR aflPushIVec2(L, p->GetMousePosInGrid()); return 1; } },
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
	afSafeDeleteBuffer(ubo);
	afSafeDeleteBuffer(ibo);
	afSafeDeleteBuffer(vbo);
	afSafeDeleteVAO(vao);

	assert(!ubo);
	assert(!vbo);
	assert(!ibo);
	assert(!vao);
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

	static InputElement layout[] = {
		CInputElement("POSITION", SF_R32G32B32_FLOAT, 0),
		CInputElement("COLOR", SF_R32G32B32_FLOAT, 12),
	};
	shaderId = shaderMan.Create("solid", layout, dimof(layout), BM_NONE, DSM_DEPTH_ENABLE, CM_DISABLE);

	vbo = afCreateVertexBuffer(sizeVertices, &vert[0]);
	ibo = afCreateIndexBuffer(&indi[0], indi.size());
	ubo = afCreateUBO(sizeof(Mat));

	int strides[] = {sizeof(GridVert)};
	VBOID vbos[] = {vbo};
	vao = afCreateVAO(layout, dimof(layout), 1, vbos, strides, ibo);
}

void GridRenderer::Draw()
{
	shaderMan.Apply(shaderId);
	Mat matView, matProj;
	matrixMan.Get(MatrixMan::VIEW, matView);
	matrixMan.Get(MatrixMan::PROJ, matProj);
	Mat matVP = matView * matProj;
	afWriteBuffer(ubo, &matVP, sizeof(Mat));
	afBindBufferToBindingPoint(ubo, 0);
	afBindVAO(vao);
	afDrawLineList(lines * 2);
	afBindVAO(0);
}

void ScreenPosToRay(const Vec2& scrPos, Vec3& nearPos, Vec3& farPos);

IVec2 GridRenderer::GetMousePosInGrid()
{
	Vec2 strPos = systemMisc.GetMousePos();
	auto drawStr = [&](const char* s) {
		fontMan.DrawString(strPos, 15, s);
		strPos.y += 16;
	};

	// make a ray from cursor pos
	Vec3 n, f;
	ScreenPosToRay(systemMisc.GetMousePos(), n, f);
	drawStr(SPrintf("near={%f,%f,%f}", n.x, n.y, n.z));
	drawStr(SPrintf("far={%f,%f,%f}", f.x, f.y, f.z));

	// ray-grid intersection
	Vec3 planeCenter = { 0, 0, 0 };
	Vec3 planeNormal = { 0, 1, 0 };
	float nDotPlane = dot(n, planeNormal);
	float fDotPlane = dot(f, planeNormal);
	if (nDotPlane * fDotPlane < 0) {
		nDotPlane = abs(nDotPlane);
		fDotPlane = abs(fDotPlane);
		Vec3 hitPos = n + (f - n) * nDotPlane / (nDotPlane + fDotPlane);
		drawStr(SPrintf("hit={%f,%f,%f}", hitPos.x, hitPos.y, hitPos.z));
		float half = pitch * numGrid / 2;
		return IVec2((int)((hitPos.x + half) / pitch), (int)((hitPos.z + half) / pitch));
	}
	return IVec2();
}
