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
static AFRenderTarget heightMap;

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

void WaterSurface::CreateRipple(Vec2 scrPos)
{
	Vec3 surfacePos = transform(Vec3(scrPos.x, scrPos.y, 0), inv(matView * matProj));

	WaterRipple r;
	r.generatedTime = elapsedTime;
	r.centerPos = Vec2(surfacePos.x, surfacePos.y);
	ripples[ripplesNext++] = r;
	ripplesNext %= dimof(ripples);
}

void WaterSurface::UpdateVert(std::vector<WaterVert>& vert)
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
		for (int i = 0; i < dimof(randWave); i++) {
			RandWave& r = randWave[i];
			r.degreePerSec = Random() * 15 - 7.5f;
			r.xShift = Random();
			r.xMul = 2 + powf(2, 1 + 3 * Random());
			r.timeMul = 2 + powf(2, 0.5f + 2 * Random());
			r.strength = 0.005f * powf(2, 0.1f + 0.3f * Random());
		}
	}
	float hmap[vertMax][vertMax];
	memset(hmap, 0, sizeof(hmap));
	for (int z = 0; z <= tileMax; z++) {
		for (int x = 0; x <= tileMax; x++) {
			Vec2 pos = Vec2((float)x, (float)z) / tileMax * 2 - Vec2(1, 1);
			for (int i = 0; i < dimof(ripples); i++) {
				const WaterRipple& r = ripples[i];
				float lifeTime = (float)(elapsedTime - r.generatedTime);
				float timeAfterArrived = lifeTime - length(r.centerPos - pos) / rippleSpeed;
				float h = timeAfterArrived > 0 ? (float)sin(timeAfterArrived * (M_PI * 2) * repeat) * heightUnit : 0;
				h *= std::min(1.0f, powf(0.5f, lifeTime / halflife));

				hmap[x][z] += h;
			}
#if 0
			for (int i = 0; i < dimof(randWave); i++) {
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

WaterSurface::WaterSurface()
{
	ibo = 0;
	vbo = 0;
	vao = 0;
	vaoEmpty = 0;
	samplerClamp = 0;
	samplerRepeat = 0;
	samplerNoMipmap = 0;
	ripplesNext = 0;
	storedW = 0;
	storedH = 0;
	shaderId = 0;
	shaderIdFullScr = 0;
	heightMapGenShaderId = 0;
}

WaterSurface::~WaterSurface()
{
	Destroy();
}

void WaterSurface::Destroy()
{
	afSafeDeleteBuffer(vbo);
	afSafeDeleteBuffer(ibo);
	afSafeDeleteSampler(samplerRepeat);
	afSafeDeleteSampler(samplerClamp);
	afSafeDeleteSampler(samplerNoMipmap);
	afSafeDeleteVAO(vao);
	afSafeDeleteVAO(vaoEmpty);
	rt.Destroy();
	heightMap.Destroy();
}

static const InputElement elements[] = {
	{ 0, "vPosition", SF_R32G32B32_FLOAT, 0 },
	{ 0, "vNormal", SF_R32G32B32_FLOAT, 12 },
};

void WaterSurface::Init()
{
	Destroy();
	rt.Init(systemMetrics.GetScreenSize());
	heightMap.Init(ivec2(512, 512));


	heightMap.Apply();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	V(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	lastTime = GetTime();

	storedW = 0;
	storedH = 0;

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

//	const char* shaderName = "vivid";
	const char* shaderName = "letterbox";
	shaderIdFullScr = shaderMan.Create(shaderName);
	shaderId = shaderMan.Create("water");
	assert(shaderId);
	heightMapGenShaderId = shaderMan.Create("water_heightmap");
	assert(heightMapGenShaderId);

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

	glGenSamplers(1, &samplerRepeat);
	glSamplerParameteri(samplerRepeat, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(samplerRepeat, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(samplerRepeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(samplerRepeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glGenSamplers(1, &samplerClamp);
	glSamplerParameteri(samplerClamp, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(samplerClamp, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(samplerClamp, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(samplerClamp, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glGenSamplers(1, &samplerNoMipmap);
	glSamplerParameteri(samplerNoMipmap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(samplerNoMipmap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(samplerNoMipmap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(samplerNoMipmap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	VBOID vertexBufferIds[] = { vbo };
	GLsizei strides[] = { sizeof(WaterVert) };
	vao = afCreateVAO(shaderId, elements, dimof(elements), 1, vertexBufferIds, strides, ibo);

	IBOID noIBO;
	noIBO.x = 0;
	vaoEmpty = afCreateVAO(shaderIdFullScr, nullptr, 0, 0, nullptr, nullptr, noIBO);
}

void WaterSurface::UpdateRipple()
{
	double now = GetTime();
	elapsedTime += now - lastTime;
	lastTime = now;

	std::vector<WaterVert> vert;
	UpdateVert(vert);

	glBufferSubData(GL_ARRAY_BUFFER, 0, vert.size() * sizeof(WaterVert), &vert[0]);

	if (0) {
//	if (elapsedTime >= nextTime) {
		nextTime = elapsedTime + 0.5 + Random() * 2;
		CreateRipple(Vec2(Random(), Random()) * 4 - Vec2(2, 2));
	}
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
	afDepthStencilMode(false);
	afBlendMode(BM_NONE);


	heightMap.Apply();
	shaderMan.Apply(heightMapGenShaderId);
	struct HeightMapUniformBuffer {
		Vec2 mousePos;
		float mouseDown;
		float elapsedTime;
	};
	HeightMapUniformBuffer hmub;
	hmub.mousePos = (Vec2)systemMetrics.GetMousePos() / (Vec2)systemMetrics.GetScreenSize() * 2 - Vec2(1, 1);
	hmub.mouseDown = (float)systemMetrics.mouseDown;
	hmub.elapsedTime = (float)elapsedTime;
	glUniform4fv(0, sizeof(hmub) / (sizeof(GLfloat) * 4), (GLfloat*)&hmub);
	fontMan.DrawString(Vec2(300, 20), 10, SPrintf("%f, %f", hmub.mousePos.x, hmub.mousePos.y));

	glViewport(0, 0, 512, 512);
	afDrawTriangleStrip(4);

	ivec2 scrSize = systemMetrics.GetScreenSize();
	glViewport(0, 0, scrSize.x, scrSize.y);

	V(glBindBuffer(GL_ARRAY_BUFFER, vbo));

	UpdateRipple();

	shaderMan.Apply(shaderId);

	for (int i = 0; i < dimof(texFiles); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texId[i]);
		glBindSampler(i, texFiles[i].clamp ? samplerClamp : samplerRepeat);
	}

	afLayoutSamplerBindingManually(shaderId, "sampler0", 0);
	afLayoutSamplerBindingManually(shaderId, "sampler1", 1);
	afLayoutSamplerBindingManually(shaderId, "sampler2", 2);
	afLayoutSamplerBindingManually(shaderId, "sampler3", 3);
	afLayoutSamplerBindingManually(shaderId, "sampler4", 4);
	afLayoutSamplerBindingManually(shaderId, "sampler5", 5);
	afLayoutSamplerBindingManually(shaderId, "waterHeightmap", 6);
	afBindTextureToBindingPoint(heightMap.GetTexture(), 6);

	double dummy;
	glUniform1f(glGetUniformLocation(shaderId, "time"), (float)modf(elapsedTime * (1.0f / loopTime), &dummy) * loopTime);

	struct UniformBuffer
	{
		Mat matW, matV, matP;
	}buf;
	buf.matW = q2m(Quat(Vec3(1,0,0), (float)M_PI / 180 * -90));
	buf.matV = matView;
	buf.matP = matProj;
	glUniform4fv(glGetUniformLocation(shaderId, "fakeUBO"), sizeof(buf) / (sizeof(GLfloat) * 4), (GLfloat*)&buf);

	rt.Apply();
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLE_STRIP, nIndi, GL_UNSIGNED_SHORT, 0);
		glBindVertexArray(0);

		V(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderMan.Apply(shaderIdFullScr);
		glUniform1i(glGetUniformLocation(shaderIdFullScr, "sampler"), 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rt.GetTexture());
		glBindSampler(0, samplerNoMipmap);

		glBindVertexArray(vaoEmpty);
		afDrawTriangleStrip(4);
		glBindVertexArray(0);
	}
}
