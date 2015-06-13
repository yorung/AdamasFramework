#include "stdafx.h"

const int tileMax = 180;
const int HEIGHT_MAP_W = tileMax;
const int HEIGHT_MAP_H = tileMax;

const int GLOW_WH = 128;

const float loopTime = 20.0;

#define V afHandleGLError

WaterSurface waterSurface;

static AFRenderTarget rt[2];
static AFRenderTarget heightMap[2];
static AFRenderTarget glowMap[6];
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
	samplerClamp = 0;
	samplerRepeat = 0;
	samplerNoMipmap = 0;
	lastMouseDown = false;
}

WaterSurface::~WaterSurface()
{
	Destroy();
}

void WaterSurface::Destroy()
{
	afSafeDeleteSampler(samplerRepeat);
	afSafeDeleteSampler(samplerClamp);
	afSafeDeleteSampler(samplerNoMipmap);
	for (auto& it : rt) {
		it.Destroy();
	}
	for (auto& it : heightMap) {
		it.Destroy();
	}
	for (auto& it : glowMap) {
		it.Destroy();
	}
}

void WaterSurface::Init()
{
	Destroy();
	for (auto& it : rt) {
		it.Init(min(ivec2(1024, 1024), systemMetrics.GetScreenSize()), AFDT_R8G8B8A8_UINT, AFDT_INVALID);
		it.BeginRenderToThis();	// clear textures
	}
	for (auto& it : heightMap) {
		it.Init(ivec2(HEIGHT_MAP_W, HEIGHT_MAP_H), AFDT_R16G16B16A16_FLOAT, AFDT_INVALID);
		it.BeginRenderToThis();	// clear textures
	}

	int texSize = GLOW_WH;
	for (auto& it : glowMap) {
		it.Init(ivec2(texSize, texSize), AFDT_R8G8B8A8_UINT, AFDT_INVALID);
		it.BeginRenderToThis();	// clear textures
		texSize /= 2;
	}

	V(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	lastTime = GetTime();

	shaderGlowExtraction = shaderMan.Create("glow_extraction", stockObjects.GetFullScreenVertexAttributeLayout());
	assert(shaderGlowExtraction);
	shaderGlowCopy = shaderMan.Create("glow_copy", stockObjects.GetFullScreenVertexAttributeLayout());
	assert(shaderGlowCopy);
	shaderGlowLastPass = shaderMan.Create("glow_lastpass", stockObjects.GetFullScreenVertexAttributeLayout());
	assert(shaderGlowLastPass);
	shaderFullScr = shaderMan.Create("letterbox", stockObjects.GetFullScreenVertexAttributeLayout());
	assert(shaderFullScr);
	shaderWaterLastPass = shaderMan.Create("water_lastpass");
	assert(shaderWaterLastPass);
	shaderHeightMap = shaderMan.Create("water_heightmap");
	assert(shaderHeightMap);
	shaderNormalMap = shaderMan.Create("water_normal");
	assert(shaderNormalMap);


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

	samplerRepeat = afCreateSampler(SF_MIPMAP, SW_REPEAT);
	samplerClamp = afCreateSampler(SF_MIPMAP, SW_CLAMP);
	samplerNoMipmap = afCreateSampler(SF_LINEAR, SW_CLAMP);

	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler0", 0);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler1", 1);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler2", 2);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler3", 3);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler4", 4);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler5", 5);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "waterHeightmap", 6);

	afLayoutSamplerBindingManually(shaderGlowExtraction, "sourceMap", 0);
	afLayoutSamplerBindingManually(shaderGlowCopy, "sourceMap", 0);
	afLayoutSamplerBindingManually(shaderGlowLastPass, "glow0", 0);
	afLayoutSamplerBindingManually(shaderGlowLastPass, "glow1", 1);
	afLayoutSamplerBindingManually(shaderGlowLastPass, "glow2", 2);
	afLayoutSamplerBindingManually(shaderGlowLastPass, "glow3", 3);
	afLayoutSamplerBindingManually(shaderGlowLastPass, "glow4", 4);
	afLayoutSamplerBindingManually(shaderGlowLastPass, "glow5", 5);
	afLayoutSamplerBindingManually(shaderGlowLastPass, "org", 6);
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

void WaterSurface::UpdateNormalMap(const UniformBuffer& hmub)
{
	auto& heightR = heightMap[heightCurrentWriteTarget];
	heightCurrentWriteTarget ^= 1;
	auto& heightW = heightMap[heightCurrentWriteTarget];
	afBindTextureToBindingPoint(heightR.GetTexture(), 0);
	heightW.BeginRenderToThis();

	shaderMan.Apply(shaderNormalMap);
	glUniform4fv(0, sizeof(hmub) / (sizeof(GLfloat) * 4), (GLfloat*)&hmub);

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
		glBindSampler(i, texFiles[i].clamp ? samplerClamp : samplerRepeat);
	}

	auto& curHeightMap = heightMap[heightCurrentWriteTarget];

	glUniform4fv(0, sizeof(hmub) / (sizeof(GLfloat) * 4), (GLfloat*)&hmub);

	rt[0].BeginRenderToThis();
	stockObjects.ApplyFullScreenVAO();
	afBindTextureToBindingPoint(curHeightMap.GetTexture(), 6);
	afDrawTriangleStrip(4);
	afBindTextureToBindingPoint(0, 6);
}

void WaterSurface::MakeGlow(const UniformBuffer& hmub)
{
	glBindSampler(0, samplerClamp);
	stockObjects.ApplyFullScreenVAO();
	shaderMan.Apply(shaderNormalMap);

	shaderMan.Apply(shaderGlowExtraction);
	glowMap[0].BeginRenderToThis();
	afBindTextureToBindingPoint(rt[0].GetTexture(), 0);
	afDrawTriangleStrip(4);

	shaderMan.Apply(shaderGlowCopy);
	for (int i = 1; i < dimof(glowMap); i++) {
		glowMap[i].BeginRenderToThis();
		afBindTextureToBindingPoint(glowMap[i - 1].GetTexture(), 0);
		afDrawTriangleStrip(4);
	}

	shaderMan.Apply(shaderGlowLastPass);
	rt[1].BeginRenderToThis();
	for (int i = 1; i < dimof(glowMap); i++) {
		afBindTextureToBindingPoint(glowMap[i].GetTexture(), i);
	}
	afBindTextureToBindingPoint(rt[0].GetTexture(), 6);
	afDrawTriangleStrip(4);
}

void WaterSurface::PostProcess()
{
	V(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderMan.Apply(shaderFullScr);
	glUniform1i(glGetUniformLocation(shaderFullScr, "sampler"), 0);

	static int num = 8;
	if (inputMan.GetInputCount('\t') == 1) {
		num = (num + 1) % 9;
	}

	switch (num) {
	case 0:
		afBindTextureToBindingPoint(rt[0].GetTexture(), 0);
		break;
	case 1:
	{
		auto& curHeightMap = heightMap[heightCurrentWriteTarget];
		afBindTextureToBindingPoint(curHeightMap.GetTexture(), 0);
		break;
	}
	case 2:
		afBindTextureToBindingPoint(glowMap[0].GetTexture(), 0);
		break;
	case 3:
		afBindTextureToBindingPoint(glowMap[1].GetTexture(), 0);
		break;
	case 4:
		afBindTextureToBindingPoint(glowMap[2].GetTexture(), 0);
		break;
	case 5:
		afBindTextureToBindingPoint(glowMap[3].GetTexture(), 0);
		break;
	case 6:
		afBindTextureToBindingPoint(glowMap[4].GetTexture(), 0);
		break;
	case 7:
		afBindTextureToBindingPoint(glowMap[5].GetTexture(), 0);
		break;
	case 8:
		afBindTextureToBindingPoint(rt[1].GetTexture(), 0);
		break;
	}
	glBindSampler(0, samplerNoMipmap);
//	glBindSampler(0, samplerRepeat);
	stockObjects.ApplyFullScreenVAO();

	ivec2 scrSize = systemMetrics.GetScreenSize();
	glViewport(0, 0, scrSize.x, scrSize.y);

	afDrawTriangleStrip(4);
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
	UpdateNormalMap(hmub);
	RenderWater(hmub);
	MakeGlow(hmub);
	PostProcess();

	glBindVertexArray(0);
}
