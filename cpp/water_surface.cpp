#include "stdafx.h"

#define V afHandleGLError

WaterSurface waterSurface;

class AFRenderTarget
{
	GLuint texColor, texDepth;
	GLuint framebufferObject;
	GLuint renderbufferObject;
public:
	AFRenderTarget();
	void Init(ivec2 size);
	void Destroy();
	void Apply();
	GLuint GetTexture() { return texColor; }
};

static AFRenderTarget rt;
static AFRenderTarget heightMap[2];
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

void AFRenderTarget::Apply()
{
	V(glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject));
}

struct TexFiles
{
	const char *name;
	bool clamp;
};

TexFiles texFiles[] = {
	{ "rose.jpg", true },
	{ "autumn.jpg", true },
	{ "pangyo.jpg", true },
	{ "timeline.png", false },
	{ "delaymap.png", true },
	{ "sphere.jpg", true },
};

TexMan::TMID texId[dimof(texFiles)];


const int tileMax = 128;
const int vertMax = tileMax + 1;
const float pitch = 2.0f / tileMax;
const float repeat = 2;
const float halflife = 1.5f;
const float heightUnit = 0.02f;
const float rippleSpeed = 0.7f;
const float loopTime = 20.0;

const int HEIGHT_MAP_W = 128;
const int HEIGHT_MAP_H = 128;


WaterSurface::WaterSurface()
{
	vaoEmpty = 0;
	samplerClamp = 0;
	samplerRepeat = 0;
	samplerNoMipmap = 0;
	shaderId = 0;
	shaderIdFullScr = 0;
	heightMapGenShaderId = 0;

	iboTiledPlane = 0;
	vboTiledPlane = 0;
	vaoWater = 0;
}

WaterSurface::~WaterSurface()
{
	Destroy();
}

void WaterSurface::Destroy()
{
	afSafeDeleteBuffer(vboTiledPlane);
	afSafeDeleteBuffer(iboTiledPlane);
	afSafeDeleteVAO(vaoWater);

	afSafeDeleteSampler(samplerRepeat);
	afSafeDeleteSampler(samplerClamp);
	afSafeDeleteSampler(samplerNoMipmap);
	afSafeDeleteVAO(vaoEmpty);
	rt.Destroy();
	for (auto& it : heightMap) {
		it.Destroy();
	}
}

void WaterSurface::InitBuffers()
{
	iboTiledPlane = afCreateTiledPlaneIBO(tileMax, &numIndi);
	vboTiledPlane = afCreateTiledPlaneVBO(tileMax);

	static const InputElement elements[] = {
		CInputElement(0, "vCoord", SF_R32G32_FLOAT, 0),
	};
	VBOID vbos[] = { vboTiledPlane };
	const int strides[] = { sizeof(Vec2) };
	vaoWater = afCreateVAO(shaderId, elements, dimof(elements), dimof(vbos), vbos, strides, iboTiledPlane);
}

void WaterSurface::Init()
{
	Destroy();
	rt.Init(systemMetrics.GetScreenSize());
	for (auto& it : heightMap) {
		it.Init(ivec2(HEIGHT_MAP_W, HEIGHT_MAP_H));
		it.Apply();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	V(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	lastTime = GetTime();

//	const char* shaderName = "vivid";
	const char* shaderName = "letterbox";
	shaderIdFullScr = shaderMan.Create(shaderName);
	shaderId = shaderMan.Create("water");
	assert(shaderId);
	heightMapGenShaderId = shaderMan.Create("water_heightmap");
	assert(heightMapGenShaderId);

	InitBuffers();

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
	vaoEmpty = afCreateVAO(shaderIdFullScr, nullptr, 0, 0, nullptr, nullptr, noIBO);

	afLayoutSamplerBindingManually(shaderId, "sampler0", 0);
	afLayoutSamplerBindingManually(shaderId, "sampler1", 1);
	afLayoutSamplerBindingManually(shaderId, "sampler2", 2);
	afLayoutSamplerBindingManually(shaderId, "sampler3", 3);
	afLayoutSamplerBindingManually(shaderId, "sampler4", 4);
	afLayoutSamplerBindingManually(shaderId, "sampler5", 5);
	afLayoutSamplerBindingManually(shaderId, "waterHeightmap", 6);
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

void WaterSurface::Draw()
{
	UpdateTime();

	afDepthStencilMode(false);
	afEnableBackFaceCulling(false);
	afBlendMode(BM_NONE);

	auto& heightW = heightMap[heightCurrentWriteTarget];
	heightCurrentWriteTarget ^= 1;
	auto& heightR = heightMap[heightCurrentWriteTarget];
	afBindTextureToBindingPoint(heightR.GetTexture(), 0);
	heightW.Apply();
	shaderMan.Apply(heightMapGenShaderId);
	struct UniformBuffer {
		Vec2 mousePos;
		float mouseDown;
		float padding;
		float elapsedTime;
		float wrappedTime;
		Vec2 heightMapSize;
	};
	UniformBuffer hmub;
	hmub.mousePos = (Vec2)systemMetrics.GetMousePos() / (Vec2)systemMetrics.GetScreenSize() * Vec2(2, -2) + Vec2(-1, 1);
	hmub.mouseDown = (float)systemMetrics.mouseDown;
	hmub.elapsedTime = (float)elapsedTime;
	hmub.heightMapSize.x = HEIGHT_MAP_W;
	hmub.heightMapSize.y = HEIGHT_MAP_H;
	double dummy;
	hmub.wrappedTime = (float)modf(elapsedTime * (1.0f / loopTime), &dummy) * loopTime;
	glUniform4fv(0, sizeof(hmub) / (sizeof(GLfloat) * 4), (GLfloat*)&hmub);
	fontMan.DrawString(Vec2(300, 20), 10, SPrintf("%f, %f", hmub.mousePos.x, hmub.mousePos.y));

	glViewport(0, 0, HEIGHT_MAP_W, HEIGHT_MAP_H);
	afBindVAO(vaoEmpty);
	afDrawTriangleStrip(4);

	ivec2 scrSize = systemMetrics.GetScreenSize();
	glViewport(0, 0, scrSize.x, scrSize.y);

	shaderMan.Apply(shaderId);

	for (int i = 0; i < dimof(texFiles); i++) {
		afBindTextureToBindingPoint(texId[i], i);
		glBindSampler(i, texFiles[i].clamp ? samplerClamp : samplerRepeat);
	}

	afBindTextureToBindingPoint(heightW.GetTexture(), 6);

	glUniform4fv(0, sizeof(hmub) / (sizeof(GLfloat) * 4), (GLfloat*)&hmub);

	rt.Apply();
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	afBindVAO(vaoWater);
	afDrawIndexedTriangleStrip(numIndi);
	afBindTextureToBindingPoint(0, 6);

	V(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shaderMan.Apply(shaderIdFullScr);
	glUniform1i(glGetUniformLocation(shaderIdFullScr, "sampler"), 0);
	static bool toggledTab;
	toggledTab ^= inputMan.GetInputCount('\t') == 1;
	if (toggledTab) {
		afBindTextureToBindingPoint(heightW.GetTexture(), 0);
	} else {
		afBindTextureToBindingPoint(rt.GetTexture(), 0);
	}
	glBindSampler(0, samplerNoMipmap);
	afDrawTriangleStrip(4);

	glBindVertexArray(0);
}
