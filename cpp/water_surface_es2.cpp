#include "stdafx.h"

struct WaterVert
{
	Vec3 pos;
	Vec3 normal;
};

struct WaterRipple
{
	WaterRipple()
	{
		generatedTime = -10000;
	}
	double generatedTime;
	Vec2 centerPos;
};

struct WaterSurfaceClassicUBO {
	Mat matW, matV, matP;
	float time;
	float padding[3];
};

class WaterSurfaceES2
{
	WaterSurfaceClassicUBO uboBuf;
	AFRenderStates renderStateWater;
	AFRenderStates renderStatePostProcess;
	int lines;
	void UpdateVert(std::vector<WaterVert>& vert);
	void UpdateRipple();
	WaterRipple ripples[2];
	int ripplesNext;
	double elapsedTime = 0;
	double lastTime;
	double nextTime;
	VBOID vbo, vboFullScr;
	IBOID ibo, iboFullScr;
	VAOID vao, vaoFullScr;
	int nIndi;
	AFRenderTarget rt;
	std::vector<SRVID> texIds;
public:
	WaterSurfaceES2();
	~WaterSurfaceES2();
	void Destroy();
	void Init();
	void Update();
	void Draw();
	void CreateRipple(Vec2 pos);
};

static const char* waterSurfaceClassName = "WaterSurfaceES2";
#define GET_WSC \
	WaterSurfaceES2* p = (WaterSurfaceES2*)luaL_checkudata(L, 1, waterSurfaceClassName);	\
	if (!p) {	\
		return 0;	\
	}

class WaterSurfaceClassicBinder {
public:
	WaterSurfaceClassicBinder() {
		GetLuaBindFuncContainer().push_back([](lua_State* L) {
			static luaL_Reg methods[] = {
				{ "__gc", [](lua_State* L) { GET_WSC p->~WaterSurfaceES2(); return 0; } },
				{ "Update", [](lua_State* L) { GET_WSC p->Update(); return 0; } },
				{ "Draw", [](lua_State* L) { GET_WSC p->Draw(); return 0; }},
				{ nullptr, nullptr },
			};
			aflBindClass(L, waterSurfaceClassName, methods, [](lua_State* L) { void* u = lua_newuserdata(L, sizeof(WaterSurfaceES2)); new (u) WaterSurfaceES2(); return 1; });
		});
	}
} static waterSurfaceClassicBinder;

struct TexFiles
{
	const char *name;
};

static TexFiles texFiles[] = {
	{ "rose.jpg" },
	{ "autumn.jpg" },
	{ "pangyo.jpg" },
	{ "timeline.png" },
	{ "delaymap.png" },
	{ "sphere.jpg"},
};

const int tileMax = 50;
const int vertMax = tileMax + 1;
const float pitch = 2.0f / tileMax;
const float repeat = 2;
const float halflife = 1.5f;
//const float heightUnit = 0.00375f;
const float heightUnit = 0.02f;
const float rippleSpeed = 0.7f;
const float loopTime = 20.0;

static Vec3 MakePos(int x, int z, float hmap[vertMax][vertMax])
{
	float height = hmap[std::max(0,std::min(tileMax, x))][std::max(0,std::min(tileMax, z))];
	return Vec3(((float)x - tileMax / 2) * pitch, height, ((float)z - tileMax / 2) * pitch);
}

void WaterSurfaceES2::CreateRipple(Vec2 scrPos)
{
	Vec3 surfacePos = transform(Vec3(scrPos.x, scrPos.y, 0), inv(uboBuf.matV * uboBuf.matP));

	WaterRipple r;
	r.generatedTime = elapsedTime;
	r.centerPos = Vec2(surfacePos.x, surfacePos.y);
	ripples[ripplesNext++] = r;
	ripplesNext %= dimof(ripples);
}

void WaterSurfaceES2::UpdateVert(std::vector<WaterVert>& vert)
{
	struct RandWave {
		float degreePerSec;
		float xShift;
		float xMul;
		float timeMul;
		float strength;
	} static randWave[10];
	static bool t;
	if (!t) {
		t = true;
		for (int i = 0; i < (int)dimof(randWave); i++) {
			RandWave& r = randWave[i];
			r.degreePerSec = Random() * 15 - 7.5f;
			r.xShift = Random();
			r.xMul = 2 + std::pow(2.f, 1 + 3 * Random());
			r.timeMul = 2 + std::pow(2.f, 0.5f + 2 * Random());
			r.strength = 0.005f * std::pow(2.f, 0.1f + 0.3f * Random());
		}
	}
	float hmap[vertMax][vertMax];
	memset(hmap, 0, sizeof(hmap));
	for (int z = 0; z <= tileMax; z++) {
		for (int x = 0; x <= tileMax; x++) {
			Vec2 pos = Vec2((float)x, (float)z) / (float)tileMax * 2.0f - Vec2(1, 1);
			for (int i = 0; i < (int)dimof(ripples); i++) {
				const WaterRipple& r = ripples[i];
				float lifeTime = (float)(elapsedTime - r.generatedTime);
				float timeAfterArrived = lifeTime - length(r.centerPos - pos) / rippleSpeed;
				float h = timeAfterArrived > 0 ? std::sin(timeAfterArrived * ((float)M_PI * 2) * repeat) * heightUnit : 0;
				h *= std::min(1.0f, std::pow(0.5f, lifeTime / halflife));

				hmap[x][z] += h;
			}
#if 0
			for (int i = 0; i < (int)dimof(randWave); i++) {
				const RandWave& r = randWave[i];
				float rad = (elapsedTime * r.degreePerSec) * XM_2PI / 180;
				Vec2 posRot = pos * cosf(rad) + Vec2(-pos.y, pos.x) * sinf(rad);
				hmap[x][z] += sinf(r.xShift + posRot.x * r.xMul + elapsedTime * r.timeMul) * r.strength;
			}
#endif
		}
	}

	for (int z = 0; z <= tileMax; z++) {
		for (int x = 0; x <= tileMax; x++) {
			WaterVert v;
			v.pos = MakePos(x, z, hmap);
			Vec3 v1 = MakePos(x, z - 1, hmap);
			Vec3 v2 = MakePos(x - 1, z, hmap);
			Vec3 v3 = MakePos(x + 1, z + 1, hmap);
			v.normal = cross(v2 - v1, v3 - v2);
			vert.push_back(v);
		}
	}
}

WaterSurfaceES2::WaterSurfaceES2()
{
	ripplesNext = 0;
	Init();
}

WaterSurfaceES2::~WaterSurfaceES2()
{
	Destroy();
}

void WaterSurfaceES2::Destroy()
{
	afSafeDeleteBuffer(vbo);
	afSafeDeleteBuffer(ibo);
	afSafeDeleteBuffer(vboFullScr);
	afSafeDeleteBuffer(iboFullScr);
	afSafeDeleteVAO(vao);
	afSafeDeleteVAO(vaoFullScr);
	for (auto& it : texIds) {
		afSafeDeleteTexture(it);
	}
	rt.Destroy();
}

static const InputElement elements[] = {
	CInputElement("vPosition", SF_R32G32B32_FLOAT, 0),
	CInputElement("vNormal", SF_R32G32B32_FLOAT, 12),
};

static const InputElement elementsFullScr[] = {
	CInputElement("POSITION", SF_R32G32_FLOAT, 0),
};

static const SamplerType samplers[] = {
	AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP
};

void WaterSurfaceES2::Init()
{
	assert(dimof(samplers) >= dimof(texFiles));
	Destroy();

	lastTime = GetTime();

	std::vector<AFIndex> indi;
	std::vector<WaterVert> vert;
	UpdateVert(vert);

	for (int z = 0; z < tileMax; z++) {
		if (z != 0) {
			indi.push_back(z * vertMax);
		}
		indi.push_back(z * vertMax);
		for (int x = 0; x < tileMax; x++) {
			indi.push_back((z + 1) * vertMax + x);
			indi.push_back(z * vertMax + x + 1);
		}
		indi.push_back((z + 1) * vertMax + vertMax - 1);
		if (z != tileMax - 1) {
			indi.push_back((z + 1) * vertMax + vertMax - 1);
		}
	}

	lines = indi.size() / 2;
	nIndi = indi.size();

	vbo = afCreateDynamicVertexBuffer(vert.size() * sizeof(WaterVert));
	ibo = afCreateIndexBuffer(&indi[0], indi.size());

	AFIndex iboFullScrSrc[] = { 0, 1, 2, 3 };
	Vec2 vboFullScrSrc[] = {{-1, 1}, {-1, -1}, {1, 1}, {1, -1}};

	vboFullScr = afCreateVertexBuffer(sizeof(vboFullScrSrc), &vboFullScrSrc[0]);
	iboFullScr = afCreateIndexBuffer(&iboFullScrSrc[0], dimof(iboFullScrSrc));

	renderStateWater.Create("water_es2", dimof(elements), elements, BM_NONE, DSM_DISABLE, CM_DISABLE, dimof(samplers), samplers);

	const char* shaderName = "vivid";
//	const char* shaderName = "letterbox";
	renderStatePostProcess.Create(shaderName, dimof(elementsFullScr), elementsFullScr, BM_NONE, DSM_DISABLE, CM_DISABLE, dimof(samplers), samplers);

	texIds.resize(dimof(texFiles));
	for (int i = 0; i < (int)dimof(texFiles); i++) {
		texIds[i] = texMan.Create(texFiles[i].name);
	}

	rt.Init(systemMisc.GetScreenSize(), AFDT_R5G6B5_UINT);

	VBOID vertexBufferIds[] = { vbo };
	int strides[] = { sizeof(WaterVert) };
	vao = afCreateVAO(elements, dimof(elements), 1, vertexBufferIds, strides, ibo);

	VBOID vertexBufferIdsFullScr[] = { vboFullScr };
	int stridesFullScr[] = { sizeof(Vec2) };
	vaoFullScr = afCreateVAO(elementsFullScr, dimof(elementsFullScr), 1, vertexBufferIdsFullScr, stridesFullScr, iboFullScr);
}

void WaterSurfaceES2::UpdateRipple()
{
	double now = GetTime();
	elapsedTime += now - lastTime;
	lastTime = now;

	if (inputMan.GetInputCount(1) == 1) {
		Vec2 pos = (Vec2)systemMisc.GetMousePos() / (Vec2)systemMisc.GetScreenSize() * Vec2(2, -2) + Vec2(-1, 1);
		CreateRipple(pos);
	}

	std::vector<WaterVert> vert;
	UpdateVert(vert);
	afWriteBuffer(vbo, &vert[0], vert.size() * sizeof(WaterVert));
}

void WaterSurfaceES2::Update()
{
	IVec2 scrSize = systemMisc.GetScreenSize();
	float offset = 0.5f;
	float aspect = (float)scrSize.y / scrSize.x;
	if (aspect < 1) {
		uboBuf.matV = fastInv(translate(0, 0.5f * (1 - aspect), 0));
		uboBuf.matP = Mat(
			1, 0, 0, 0,
			0, 1 / aspect, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	} else {
		uboBuf.matV = fastInv(translate(offset * (1 - 1 / aspect), 0, 0));
		uboBuf.matP = Mat(
			aspect, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}
#ifndef GL_TURE
	uboBuf.matP *= Mat(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -0.5, 0, 0, 0, 0.5, 1);
#endif

	uboBuf.matW = q2m(Quat(Vec3(1, 0, 0), (float)M_PI / 180 * -90));
	double dummy;
	uboBuf.time = (float)modf(elapsedTime * (1.0f / loopTime), &dummy) * loopTime;
}

void WaterSurfaceES2::Draw()
{
	UpdateRipple();

	renderStateWater.Apply();
	for (int i = 0; i < (int)dimof(texFiles); i++) {
		afBindTextureToBindingPoint(texIds[i], i);
	}
	rt.BeginRenderToThis();
	UBOID ubo = afBindCbv0(&uboBuf, sizeof(uboBuf));
	afBindVAO(vao);
	afDrawIndexed(PT_TRIANGLESTRIP, nIndi);
	afBindVAO(0);
	afSafeDeleteBuffer(ubo);

	AFRenderTarget rtDefault;
	rtDefault.InitForDefaultRenderTarget();
	rtDefault.BeginRenderToThis();
	renderStatePostProcess.Apply();
	afBindSrv0(rt.GetTexture());
	afBindVAO(vaoFullScr);
	afDrawIndexed(PT_TRIANGLESTRIP, 4);
	afBindVAO(0);
}
