#include "stdafx.h"

const int tileMax = 180;
const int HEIGHT_MAP_W = tileMax;
const int HEIGHT_MAP_H = tileMax;

const int GLOW_WH = 128;

const float loopTime = 20.0;

#define V afHandleGLError

WaterSurface waterSurface;

class AFRenderTarget
{
	ivec2 texSize;
	GLuint texColor, texDepth;
	GLuint framebufferObject;
	GLuint renderbufferObject;
public:
	AFRenderTarget();
	void Init(ivec2 size);
	void Destroy();
	void BeginRenderToThis();
	GLuint GetTexture() { return texColor; }
};

static AFRenderTarget rt;
static AFRenderTarget heightMap[2];
static AFRenderTarget glowMap[6];
static int heightCurrentWriteTarget;

AFRenderTarget::AFRenderTarget()
{
	texColor = 0;
	texDepth = 0;
	framebufferObject = 0;
	renderbufferObject = 0;
}

void AFRenderTarget::Destroy()
{
	afSafeDeleteTexture(texColor);
	afSafeDeleteTexture(texDepth);
	if (framebufferObject) {
		glDeleteFramebuffers(1, &framebufferObject);
		framebufferObject = 0;
	}
	if (renderbufferObject) {
		glDeleteRenderbuffers(1, &renderbufferObject);
		renderbufferObject = 0;
	}
}

void AFRenderTarget::Init(ivec2 size)
{
	texSize = size;
	texColor = afCreateDynamicTexture(size.x, size.y, AFDT_R16G16B16A16_FLOAT);
//	texColor = afCreateDynamicTexture(size.x, size.y, AFDT_R5G6B5_UINT);
	texDepth = afCreateDynamicTexture(size.x, size.y, AFDT_DEPTH_STENCIL);

//	glGenRenderbuffers(1, &renderbufferObject);
//	glBindRenderbuffer(GL_RENDERBUFFER, renderbufferObject);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size.x, size.y);
//	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &framebufferObject);
	V(glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject));
	V(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0));
//	V(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texDepth, 0));
	V(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texDepth, 0));
//	V(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbufferObject));
	V(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void AFRenderTarget::BeginRenderToThis()
{
	glViewport(0, 0, texSize.x, texSize.y);
	V(glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject));
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

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
	vaoEmpty = 0;
	samplerClamp = 0;
	samplerRepeat = 0;
	samplerNoMipmap = 0;
	shaderWaterLastPass = 0;
	shaderFullScr = 0;
	shaderHeightMap = 0;
	shaderNormalMap = 0;
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
	afSafeDeleteVAO(vaoEmpty);
	rt.Destroy();
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
	rt.Init(systemMetrics.GetScreenSize());
	for (auto& it : heightMap) {
		it.Init(ivec2(HEIGHT_MAP_W, HEIGHT_MAP_H));
		it.BeginRenderToThis();	// clear textures
	}

	int texSize = GLOW_WH;
	for (auto& it : glowMap) {
		it.Init(ivec2(texSize, texSize));
		it.BeginRenderToThis();	// clear textures
		texSize /= 2;
	}

	V(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	lastTime = GetTime();

	shaderFullScr = shaderMan.Create("letterbox");
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

	IBOID noIBO;
	noIBO.x = 0;
	vaoEmpty = afCreateVAO(shaderFullScr, nullptr, 0, 0, nullptr, nullptr, noIBO);

	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler0", 0);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler1", 1);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler2", 2);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler3", 3);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler4", 4);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "sampler5", 5);
	afLayoutSamplerBindingManually(shaderWaterLastPass, "waterHeightmap", 6);
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

	afBindVAO(vaoEmpty);
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

	afBindVAO(vaoEmpty);
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

	rt.BeginRenderToThis();
	afBindVAO(vaoEmpty);
	afBindTextureToBindingPoint(curHeightMap.GetTexture(), 6);
	afDrawTriangleStrip(4);
	afBindTextureToBindingPoint(0, 6);
}


int GetMSB(int n)
{
	DWORD result;
	if (_BitScanReverse(&result, (DWORD)n)) {
		return (int)result;
	}
	return 0;
}

void WaterSurface::MakeGlow(const UniformBuffer& hmub)
{
	glBindSampler(0, samplerClamp);
	afBindVAO(vaoEmpty);

	shaderMan.Apply(shaderNormalMap);
	struct GlowExtractionUniform {
		Vec2 uv;
		float padding[2];
	}ub;
	memset(&ub, 0, sizeof(ub));
	ivec2 screenSize = systemMetrics.GetScreenSize();
	ivec2 screenSizePOT = ivec2(2 << GetMSB(screenSize.x - 1), 2 << GetMSB(screenSize.y - 1));
	ub.uv = (Vec2)screenSize / (Vec2)screenSizePOT;
	glUniform4fv(0, sizeof(ub) / (sizeof(GLfloat) * 4), (GLfloat*)&ub);

	GLuint shader = shaderMan.Create("glow_extraction");
	assert(shader);
	shaderMan.Apply(shader);
	glowMap[0].BeginRenderToThis();
	afBindTextureToBindingPoint(rt.GetTexture(), 0);
	afDrawTriangleStrip(4);

	shader = shaderMan.Create("glow_copy");
	assert(shader);
	shaderMan.Apply(shader);

	for (int i = 1; i < dimof(glowMap); i++) {
		glowMap[i].BeginRenderToThis();
		afBindTextureToBindingPoint(glowMap[i - 1].GetTexture(), 0);
		afDrawTriangleStrip(4);
	}
}

void WaterSurface::PostProcess()
{
	V(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderMan.Apply(shaderFullScr);
	glUniform1i(glGetUniformLocation(shaderFullScr, "sampler"), 0);

	static int num = 0;
	if (inputMan.GetInputCount('\t') == 1) {
		num = (num + 1) % 8;
	}

	switch (num) {
	case 0:
		afBindTextureToBindingPoint(rt.GetTexture(), 0);
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
	}
	glBindSampler(0, samplerNoMipmap);
//	glBindSampler(0, samplerRepeat);
	afBindVAO(vaoEmpty);

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
