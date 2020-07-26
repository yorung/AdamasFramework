#include "stdafx.h"
#include "letterbox.h"
#include "input_man.h"
#include "font_man.h"
#include "stock_objects.h"
#include "glow.h"
#include "ModuleManager.h"

class WaterSurfaceES3 : AFModule
{
	struct UniformBuffer {
		Vec2 mousePos;
		float mouseDown;
		float padding;
		float elapsedTime;
		float wrappedTime;
		Vec2 heightMapSize;
	};

	Mat matProj, matView;
	AFRenderStates renderStateHeightMap;
	AFRenderStates renderStateNormalMap;
	AFRenderStates renderStateWaterLastPass;
	std::vector<AFTexRef> texId;
	AFRenderTarget renderTarget[2];
	AFRenderTarget heightMap[2];
	AFRenderTarget normalMap;
	int heightCurrentWriteTarget = 0;

	double elapsedTime = 0;
	double lastTime = 0;
	bool lastMouseDown = false;
	void UpdateTime();
	void UpdateHeightMap(AFCommandList& cmd, const UniformBuffer&);
	void UpdateNormalMap(AFCommandList& cmd);
	void RenderWater(AFCommandList& cmd, const UniformBuffer&);
	void Init();
	void Destroy();
public:
	WaterSurfaceES3();
	~WaterSurfaceES3();
	void Update() override;
	void Draw2D(AFCommandList& cmd, AFRenderTarget& rt) override;
};


static const char* waterSurfaceClassName = "WaterSurfaceES3";
#define GET_WS \
	WaterSurfaceES3* p = (WaterSurfaceES3*)luaL_checkudata(L, 1, waterSurfaceClassName); \
	if (!p) {	\
		return 0;	\
	} \

class WaterSurfaceBinder
{
public:
	WaterSurfaceBinder()
	{
		GetLuaBindFuncContainer().push_back([](lua_State* L) {
			static luaL_Reg methods[] = {
				{ "__gc", [](lua_State* L) { GET_WS p->~WaterSurfaceES3(); return 0; } },
				{ nullptr, nullptr },
			};
			aflBindClass(L, waterSurfaceClassName, methods, [](lua_State* L) { void* u = lua_newuserdata(L, sizeof(WaterSurfaceES3)); new (u) WaterSurfaceES3(); return 1; });
		});
	}
} static waterSurfaceClassicBinder;


const int tileMax = 180;
const int HEIGHT_MAP_W = tileMax;
const int HEIGHT_MAP_H = tileMax;

const float loopTime = 20.0;

extern AFRenderTarget glowMap[6];

struct TexFiles
{
	const char *name;
	bool clamp;
};

static TexFiles texFiles[] =
{
	{ "rose.jpg", true },
	{ "autumn.jpg", true },
	{ "pangyo.jpg", true },
	{ "timeline.png", false },
	{ "delaymap.png", true },
	{ "sphere.jpg", true },
};

static const SamplerType samplersLastPass[] = { AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_WRAP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP };
static const SamplerType samplersHeightMap[] = { AFST_LINEAR_CLAMP };
static const SamplerType samplerNormalMap[] = { AFST_LINEAR_CLAMP };

WaterSurfaceES3::WaterSurfaceES3()
{
	Init();
}

WaterSurfaceES3::~WaterSurfaceES3()
{
	Destroy();
}

void WaterSurfaceES3::Destroy()
{
	for (auto& it : texId)
	{
		afSafeDeleteTexture(it);
	}
	texId.clear();
	for (auto& it : renderTarget)
	{
		it.Destroy();
	}
	for (auto& it : heightMap)
	{
		it.Destroy();
	}
	normalMap.Destroy();

	renderStateHeightMap.Destroy();
	renderStateNormalMap.Destroy();
	renderStateWaterLastPass.Destroy();
}

void WaterSurfaceES3::Init()
{
	Destroy();
	for (auto& it : renderTarget)
	{
		it.Init(min(IVec2(1024, 1024), systemMisc.GetScreenSize()), AFF_R8G8B8A8_UNORM, AFF_INVALID);
	}
	for (auto& it : heightMap)
	{
		it.Init(IVec2(HEIGHT_MAP_W, HEIGHT_MAP_H), AFF_R16G16B16A16_FLOAT, AFF_INVALID);
		it.BeginRenderToThis();	// clear textures
		it.EndRenderToThis();
	}
	normalMap.Init(IVec2(HEIGHT_MAP_W, HEIGHT_MAP_H), AFF_R8G8B8A8_UNORM, AFF_INVALID);

	lastTime = GetTime();
	renderStateWaterLastPass.Create("water_es3_lastpass", 0, nullptr, AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM, arrayparam(samplersLastPass));
	renderStateHeightMap.Create("water_es3_heightmap", 0, nullptr, AFRS_OFFSCREEN_RENDER_TARGET_R16G16B16A16_FLOAT, arrayparam(samplersHeightMap));

	{
		int numElements = 0;
		const InputElement* elements = stockObjects.GetFullScreenInputElements(numElements);
		renderStateNormalMap.Create("water_es3_normal", numElements, elements, AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM, arrayparam(samplerNormalMap));
	}

	aflog("WaterSurface::Init shaders are ready!\n");

	texId.resize(dimof(texFiles));
	for (int i = 0; i < (int)dimof(texFiles); i++)
	{
		texId[i] = afLoadTexture(texFiles[i].name);
		aflog("WaterSurface::Init tex %s %s\n", texFiles[i].name, (!!texId[i] ? "OK" : "NG"));
	}

	aflog("WaterSurface::Init finished!\n");
}

void WaterSurfaceES3::UpdateTime()
{
	double now = GetTime();
	elapsedTime += now - lastTime;
	lastTime = now;
}

void WaterSurfaceES3::Update()
{
	IVec2 scrSize = systemMisc.GetScreenSize();
	float offset = 0.5f;
	float aspect = (float)scrSize.x / scrSize.y;
	if (aspect > 1) {
//		matView = fastInv(translate(0, 0.5f * (1 - aspect), 0));
		matView = fastInv(translate(0, 0, 0));
		matProj = Mat(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	} else {
		matView = fastInv(translate(offset * (1 - 1 / aspect), 0, 0));
		matProj = Mat(
			1 / aspect, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}
}

void WaterSurfaceES3::UpdateHeightMap(AFCommandList& cmd, const UniformBuffer& hmub)
{
	cmd.SetRenderStates(renderStateHeightMap);
	auto& heightR = heightMap[heightCurrentWriteTarget];
	heightCurrentWriteTarget ^= 1;
	auto& heightW = heightMap[heightCurrentWriteTarget];

	cmd.SetTexture(heightR.GetTexture(), 0);
	cmd.SetBuffer(sizeof(hmub), &hmub, 1);
	stockObjects.ApplyFullScreenVertexBuffer(cmd);

	heightW.BeginRenderToThis();
	cmd.Draw(4);
	heightW.EndRenderToThis();

#ifdef AF_DX11
	cmd.SetTexture(SRVID(), 0);
	ID3D11RenderTargetView* view = nullptr;
	deviceMan11.GetContext()->OMSetRenderTargets(1, &view, nullptr);	// prevent DEVICE_PSSETSHADERRESOURCES_HAZARD in next pass
#endif
}

void WaterSurfaceES3::UpdateNormalMap(AFCommandList& cmd)
{
	cmd.SetRenderStates(renderStateNormalMap);
	auto& heightR = heightMap[heightCurrentWriteTarget];
	cmd.SetTexture(heightR.GetTexture(), 1);
	Vec4 heightMapSize((float)HEIGHT_MAP_W, (float)HEIGHT_MAP_H, 0, 0);
	cmd.SetBuffer(sizeof(heightMapSize), &heightMapSize, 0);
	stockObjects.ApplyFullScreenVertexBuffer(cmd);

	normalMap.BeginRenderToThis();
	cmd.Draw(4);
	normalMap.EndRenderToThis();
#ifdef AF_DX11
	cmd.SetTexture(SRVID(), 0);
#endif
}

void WaterSurfaceES3::RenderWater(AFCommandList& cmd, const UniformBuffer& hmub)
{
	cmd.SetRenderStates(renderStateWaterLastPass);

	for (int i = 0; i < (int)dimof(texFiles); i++)
	{
		cmd.SetTexture(texId[i], i);
	}

	cmd.SetBuffer(sizeof(hmub), &hmub, 7);
	stockObjects.ApplyFullScreenVertexBuffer(cmd);
	cmd.SetTexture(normalMap.GetTexture(), 6);

	renderTarget[0].BeginRenderToThis();
	cmd.Draw(4);
	renderTarget[0].EndRenderToThis();
#ifdef AF_DX11
	cmd.SetTexture(SRVID(), 6);
	cmd.SetTexture(SRVID(), 7);
#endif
}

void WaterSurfaceES3::Draw2D(AFCommandList& cmd, AFRenderTarget& rt)
{
	rt.EndRenderToThis();//workaround
	UpdateTime();

	bool mouseEdge = !lastMouseDown && systemMisc.mouseDown;
	lastMouseDown = systemMisc.mouseDown;

	static Vec2 lastMousePos;
	Vec2 mousePos = (Vec2)systemMisc.GetMousePos() / (Vec2)systemMisc.GetScreenSize() * Vec2(2, -2) + Vec2(-1, 1);
	UniformBuffer hmub;
	hmub.mousePos = mousePos;
	hmub.mouseDown = mouseEdge;
	hmub.elapsedTime = (float)elapsedTime;
	hmub.heightMapSize.x = HEIGHT_MAP_W;
	hmub.heightMapSize.y = HEIGHT_MAP_H;

//	if (++frame % 2 == 0 && length(lastMousePos - mousePos) >= 0.01 && systemMetrics.mouseDown) {
	if (systemMisc.mouseDown) {
		hmub.mouseDown = true;
	}
	if (hmub.mouseDown) {
		lastMousePos = mousePos;
	}

	double dummy;
	hmub.wrappedTime = (float)std::modf(elapsedTime * (1.0f / loopTime), &dummy) * loopTime;
	fontMan.DrawString(Vec2(300, 20), 10, SPrintf("%f, %f", hmub.mousePos.x, hmub.mousePos.y), 0xffffffff);

	UpdateHeightMap(cmd, hmub);
	UpdateNormalMap(cmd);
	RenderWater(cmd, hmub);
	glow.MakeGlow(renderTarget[1], renderTarget[0].GetTexture());

	static int num = 8;
	if (inputMan.GetInputCount('\t') == 1) {
		num = (num + 1) % 9;
	}

	AFTexRef srcTex;
	switch (num) {
	case 0:
		srcTex = renderTarget[0].GetTexture();
		break;
	case 1:
	{
		auto& curHeightMap = heightMap[heightCurrentWriteTarget];
		srcTex = curHeightMap.GetTexture();
		break;
	}
	case 2:
		srcTex = glowMap[0].GetTexture();
		break;
	case 3:
		srcTex = glowMap[1].GetTexture();
		break;
	case 4:
		srcTex = glowMap[2].GetTexture();
		break;
	case 5:
		srcTex = glowMap[3].GetTexture();
		break;
	case 6:
		srcTex = glowMap[4].GetTexture();
		break;
	case 7:
		srcTex = glowMap[5].GetTexture();
		break;
	case 8:
		srcTex = renderTarget[1].GetTexture();
		break;
	}

	letterBox.Draw(cmd, rt, srcTex);
}
