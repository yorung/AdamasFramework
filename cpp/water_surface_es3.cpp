#include "stdafx.h"
#ifdef AF_GLES31

class WaterSurfaceES3
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
	std::vector<SRVID> texId;
	AFRenderTarget renderTarget[2];
	AFRenderTarget heightMap[2];
	AFRenderTarget normalMap;
	int heightCurrentWriteTarget = 0;

	double elapsedTime = 0;
	double lastTime = 0;
	bool lastMouseDown = false;
	void UpdateTime();
	void UpdateHeightMap(const UniformBuffer&);
	void UpdateNormalMap();
	void RenderWater(const UniformBuffer&);
	void Init();
	void Destroy();
public:
	WaterSurfaceES3();
	~WaterSurfaceES3();
	void Update();
	void Draw();
};


static const char* waterSurfaceClassName = "WaterSurfaceES3";
#define GET_WS \
	WaterSurfaceES3* p = (WaterSurfaceES3*)luaL_checkudata(L, 1, waterSurfaceClassName); \
	if (!p) {	\
		return 0;	\
	} \

class WaterSurfaceBinder {
public:
	WaterSurfaceBinder() {
		GetLuaBindFuncContainer().push_back([](lua_State* L) {
			static luaL_Reg methods[] = {
				{ "__gc", [](lua_State* L) { GET_WS p->~WaterSurfaceES3(); return 0; } },
				{ "Update", [](lua_State* L) { GET_WS p->Update(); return 0; } },
				{ "Draw", [](lua_State* L) { GET_WS p->Draw(); return 0; } },
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

#define V afHandleGLError

extern AFRenderTarget glowMap[6];

struct TexFiles
{
	const char *name;
	bool clamp;
};

#if 1
static TexFiles texFiles[] = {
	{ "rose.jpg", true },
	{ "autumn.jpg", true },
	{ "pangyo.jpg", true },
	{ "timeline.png", false },
	{ "delaymap.png", true },
	{ "sphere.jpg", true },
};
#else
static TexFiles texFiles[] = {
	{ "D:\\github\\BingmuWP\\app\\src\\summer\\assets\\hyomu_sm_1.jpg", true },
	{ "D:\\github\\BingmuWP\\app\\src\\summer\\assets\\hyomu_sm_2.jpg", true },
	{ "D:\\github\\BingmuWP\\app\\src\\summer\\assets\\hyomu_sm_3.jpg", true },
	{ "timeline.png", false },
	{ "delaymap.png", true },
	{ "sphere.jpg", true },
};
#endif


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
	for (auto& it : renderTarget) {
		it.Destroy();
	}
	for (auto& it : heightMap) {
		it.Destroy();
	}
	normalMap.Destroy();
}

void WaterSurfaceES3::Init()
{
	Destroy();
	for (auto& it : renderTarget) {
		it.Init(min(IVec2(1024, 1024), systemMisc.GetScreenSize()), AFF_R8G8B8A8_UNORM, AFF_INVALID);
		it.BeginRenderToThis();	// clear textures
	}
	for (auto& it : heightMap) {
		it.Init(IVec2(HEIGHT_MAP_W, HEIGHT_MAP_H), AFF_R16G16B16A16_FLOAT, AFF_INVALID);
		it.BeginRenderToThis();	// clear textures
	}
	normalMap.Init(IVec2(HEIGHT_MAP_W, HEIGHT_MAP_H), AFF_R8G8B8A8_UNORM, AFF_INVALID);
	normalMap.BeginRenderToThis();	// clear textures

	AFRenderTarget rt;
	rt.InitForDefaultRenderTarget();
	rt.BeginRenderToThis();

	lastTime = GetTime();

	renderStateWaterLastPass.Create("water_es3_lastpass");
	renderStateHeightMap.Create("water_es3_heightmap");

	{
		int numElements = 0;
		const InputElement* elements = stockObjects.GetFullScreenInputElements(numElements);
		renderStateNormalMap.Create("water_es3_normal", numElements, elements);
	}

	shaderMan.Apply(renderStateNormalMap.GetShaderId());
	Vec2 heightMapSize((float)HEIGHT_MAP_W, (float)HEIGHT_MAP_H);
	glUniform2fv(glGetUniformLocation(renderStateNormalMap.GetShaderId(), "heightMapSize"), 1, (GLfloat*)&heightMapSize);

	aflog("WaterSurface::Init shaders are ready!\n");

	glActiveTexture(GL_TEXTURE0);
	texId.resize(dimof(texFiles));
	for (int i = 0; i < (int)dimof(texFiles); i++) {
		texId[i] = texMan.Create(texFiles[i].name);
		aflog("WaterSurface::Init tex %s %s\n", texFiles[i].name, (texId[i] ? "OK" : "NG"));
		if (!texFiles[i].clamp) {
            glBindTexture(GL_TEXTURE_2D, texId[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
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

void WaterSurfaceES3::UpdateHeightMap(const UniformBuffer& hmub)
{
	auto& heightR = heightMap[heightCurrentWriteTarget];
	heightCurrentWriteTarget ^= 1;
	auto& heightW = heightMap[heightCurrentWriteTarget];
	afBindTexture(heightR.GetTexture(), 0);
	heightW.BeginRenderToThis();

	renderStateHeightMap.Apply();
//	GLint loc = glGetUniformLocation(shaderHeightMap, "fakeUBO");
//	aflog("shaderHeightMap loc = %d\n", loc);
	afHandleGLError(glUniform4fv(0, sizeof(hmub) / (sizeof(GLfloat) * 4), (GLfloat*)&hmub));

	stockObjects.ApplyFullScreenVertexBuffer();
	afDraw(4);
	afBindVAO(0);
}

void WaterSurfaceES3::UpdateNormalMap()
{
	auto& heightR = heightMap[heightCurrentWriteTarget];
	afBindTexture(heightR.GetTexture(), 0);
	normalMap.BeginRenderToThis();
	renderStateNormalMap.Apply();
	stockObjects.ApplyFullScreenVertexBuffer();
	afDraw(4);
	afBindVAO(0);
}

void WaterSurfaceES3::RenderWater(const UniformBuffer& hmub)
{
	renderStateWaterLastPass.Apply();

	for (int i = 0; i < (int)dimof(texFiles); i++) {
		afBindTexture(texId[i], i);
		afSetSampler(texFiles[i].clamp ? AFST_LINEAR_CLAMP : AFST_LINEAR_WRAP, i);
	}

	auto& curHeightMap = heightMap[heightCurrentWriteTarget];

//	GLint loc = glGetUniformLocation(shaderWaterLastPass, "fakeUBO");
//	aflog("shaderWaterLastPass loc = %d\n", loc);
	afHandleGLError(glUniform4fv(0, sizeof(hmub) / (sizeof(GLfloat) * 4), (GLfloat*)&hmub));

	renderTarget[0].BeginRenderToThis();
	stockObjects.ApplyFullScreenVertexBuffer();
	afBindTexture(curHeightMap.GetTexture(), 6);
	afBindTexture(normalMap.GetTexture(), 7);
	afDraw(4);
	afBindTexture(0, 6);
	afBindVAO(0);
}

void WaterSurfaceES3::Draw()
{
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
	fontMan.DrawString(Vec2(300, 20), 10, SPrintf("%f, %f", hmub.mousePos.x, hmub.mousePos.y));

	UpdateHeightMap(hmub);
	UpdateNormalMap();
	RenderWater(hmub);
	glow.MakeGlow(renderTarget[1], renderTarget[0].GetTexture());

	AFRenderTarget rt;
	rt.InitForDefaultRenderTarget();

	static int num = 8;
	if (inputMan.GetInputCount('\t') == 1) {
		num = (num + 1) % 9;
	}

	SRVID srcTex;
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

	letterBox.Draw(rt, srcTex);

	afBindVAO(0);
}
#endif