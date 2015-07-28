#include "stdafx.h"

const int tileMax = 180;
const int HEIGHT_MAP_W = tileMax;
const int HEIGHT_MAP_H = tileMax;

const float loopTime = 20.0;

#define V afHandleGLError

WaterSurface waterSurface;

static AFRenderTarget renderTarget[2];
static AFRenderTarget heightMap[2];
static AFRenderTarget normalMap;
extern AFRenderTarget glowMap[6];
static int heightCurrentWriteTarget;

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
static TexMan::TMID texId[dimof(texFiles)];


WaterSurface::WaterSurface()
{
	lastMouseDown = false;
}

WaterSurface::~WaterSurface()
{
	Destroy();
}

void WaterSurface::Destroy()
{
	for (auto& it : renderTarget) {
		it.Destroy();
	}
	for (auto& it : heightMap) {
		it.Destroy();
	}
	normalMap.Destroy();
}

void WaterSurface::Init()
{
	Destroy();
	for (auto& it : renderTarget) {
		it.Init(min(ivec2(1024, 1024), systemMetrics.GetScreenSize()), AFDT_R8G8B8A8_UINT, AFDT_INVALID);
		it.BeginRenderToThis();	// clear textures
	}
	for (auto& it : heightMap) {
		it.Init(ivec2(HEIGHT_MAP_W, HEIGHT_MAP_H), AFDT_R16G16B16A16_FLOAT, AFDT_INVALID);
		it.BeginRenderToThis();	// clear textures
	}
	normalMap.Init(ivec2(HEIGHT_MAP_W, HEIGHT_MAP_H), AFDT_R8G8B8A8_UINT, AFDT_INVALID);
	normalMap.BeginRenderToThis();	// clear textures

	V(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	lastTime = GetTime();

	shaderWaterLastPass = shaderMan.Create("water_lastpass");
	assert(shaderWaterLastPass);
	shaderHeightMap = shaderMan.Create("water_heightmap");
	assert(shaderHeightMap);
	shaderNormalMap = shaderMan.Create("water_normal");
	afLayoutSamplerBindingManually(shaderWaterLastPass, "waterHeightmap", 0);
	assert(shaderNormalMap);
	shaderMan.Apply(shaderNormalMap);

	Vec2 heightMapSize((float)HEIGHT_MAP_W, (float)HEIGHT_MAP_H);
	glUniform2fv(glGetUniformLocation(shaderNormalMap, "heightMapSize"), 1, (GLfloat*)&heightMapSize);


	glActiveTexture(GL_TEXTURE0);
	for (int i = 0; i < dimof(texFiles); i++) {
		texId[i] = texMan.Create(texFiles[i].name);
        if (!texFiles[i].clamp) {
            glBindTexture(GL_TEXTURE_2D, texId[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
	}

	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler0", 0);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler1", 1);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler2", 2);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler3", 3);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler4", 4);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler5", 5);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "waterHeightmap", 6);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "waterNormalmap", 7);
}

void WaterSurface::UpdateTime()
{
	double now = GetTime();
	elapsedTime += now - lastTime;
	lastTime = now;
}

void WaterSurface::Update()
{
	ivec2 scrSize = systemMetrics.GetScreenSize();
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

void WaterSurface::UpdateHeightMap(const UniformBuffer& hmub)
{
	auto& heightR = heightMap[heightCurrentWriteTarget];
	heightCurrentWriteTarget ^= 1;
	auto& heightW = heightMap[heightCurrentWriteTarget];
	afBindTextureToBindingPoint(heightR.GetTexture(), 0);
	heightW.BeginRenderToThis();

	shaderMan.Apply(shaderHeightMap);
	glUniform4fv(0, sizeof(hmub) / (sizeof(GLfloat) * 4), (GLfloat*)&hmub);

	stockObjects.ApplyFullScreenVAO();
	afDrawTriangleStrip(4);
}

void WaterSurface::UpdateNormalMap()
{
	auto& heightR = heightMap[heightCurrentWriteTarget];
	afBindTextureToBindingPoint(heightR.GetTexture(), 0);
	normalMap.BeginRenderToThis();
	shaderMan.Apply(shaderNormalMap);
	stockObjects.ApplyFullScreenVAO();
	afDrawTriangleStrip(4);
}

void WaterSurface::RenderWater(const UniformBuffer& hmub)
{
	ivec2 scrSize = systemMetrics.GetScreenSize();
	glViewport(0, 0, scrSize.x, scrSize.y);

	shaderMan.Apply(shaderWaterLastPass);

	for (int i = 0; i < dimof(texFiles); i++) {
		afBindTextureToBindingPoint(texId[i], i);
		glBindSampler(i, texFiles[i].clamp ? stockObjects.GetClampSampler() : stockObjects.GetRepeatSampler());
	}

	auto& curHeightMap = heightMap[heightCurrentWriteTarget];

	glUniform4fv(0, sizeof(hmub) / (sizeof(GLfloat) * 4), (GLfloat*)&hmub);

	renderTarget[0].BeginRenderToThis();
	stockObjects.ApplyFullScreenVAO();
	afBindTextureToBindingPoint(curHeightMap.GetTexture(), 6);
	afBindTextureToBindingPoint(normalMap.GetTexture(), 7);
	afDrawTriangleStrip(4);
	afBindTextureToBindingPoint(0, 6);
}

void WaterSurface::Draw()
{
	UpdateTime();

	bool mouseEdge = !lastMouseDown && systemMetrics.mouseDown;
	lastMouseDown = systemMetrics.mouseDown;

	static int frame = 0;
	static Vec2 lastMousePos;
	Vec2 mousePos = (Vec2)systemMetrics.GetMousePos() / (Vec2)systemMetrics.GetScreenSize() * Vec2(2, -2) + Vec2(-1, 1);
	UniformBuffer hmub;
	hmub.mousePos = mousePos;
	hmub.mouseDown = mouseEdge;
	hmub.elapsedTime = (float)elapsedTime;
	hmub.heightMapSize.x = HEIGHT_MAP_W;
	hmub.heightMapSize.y = HEIGHT_MAP_H;

	if (++frame % 2 == 0 && length(lastMousePos - mousePos) >= 0.01 && systemMetrics.mouseDown) {
		hmub.mouseDown = true;
	}
	if (hmub.mouseDown) {
		lastMousePos = mousePos;
	}


	double dummy;
	hmub.wrappedTime = (float)modf(elapsedTime * (1.0f / loopTime), &dummy) * loopTime;
	fontMan.DrawString(Vec2(300, 20), 10, SPrintf("%f, %f", hmub.mousePos.x, hmub.mousePos.y));

	afDepthStencilMode(false);
	afEnableBackFaceCulling(false);
	afBlendMode(BM_NONE);

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

	GLuint srcTex = 0;
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

	glBindVertexArray(0);
}
