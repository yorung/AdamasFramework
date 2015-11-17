#include "stdafx.h"

WaterSurfaceClassic waterSurfaceClassic;

struct TexFiles
{
	const char *name;
	bool clamp;
};

static TexFiles texFiles[] = {
	{ "rose.jpg", true },
	{ "autumn.jpg", true },
	{ "pangyo.jpg", true },
	{ "timeline.png", false },
	{ "delaymap.png", true },
	{ "sphere.jpg", true },
};

static TexMan::TMID texId[dimof(texFiles)];


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

void WaterSurfaceClassic::CreateRipple(Vec2 scrPos)
{
	Vec3 surfacePos = transform(Vec3(scrPos.x, scrPos.y, 0), inv(matView * matProj));

	WaterRipple r;
	r.generatedTime = elapsedTime;
	r.centerPos = Vec2(surfacePos.x, surfacePos.y);
	ripples[ripplesNext++] = r;
	ripplesNext %= dimof(ripples);
}

void WaterSurfaceClassic::UpdateVert(std::vector<WaterVert>& vert)
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
			Vec2 pos = Vec2((float)x, (float)z) / (float)tileMax * 2.0f - Vec2(1, 1);
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

WaterSurfaceClassic::WaterSurfaceClassic()
{
	ibo = 0;
	vbo = 0;
	vao = 0;
	vboFullScr = 0;
	iboFullScr = 0;
	vaoFullScr = 0;
	samplerClamp = 0;
	samplerRepeat = 0;
	samplerNoMipmap = 0;
	ripplesNext = 0;
	texRenderTarget = 0;
	framebufferObject = 0;
	renderbufferObject = 0;
	storedW = 0;
	storedH = 0;
}

WaterSurfaceClassic::~WaterSurfaceClassic()
{
	Destroy();
}

void WaterSurfaceClassic::Destroy()
{
	afSafeDeleteBuffer(vbo);
	afSafeDeleteBuffer(ibo);
	afSafeDeleteBuffer(vboFullScr);
	afSafeDeleteBuffer(iboFullScr);
	afSafeDeleteSampler(samplerRepeat);
	afSafeDeleteSampler(samplerClamp);
	afSafeDeleteSampler(samplerNoMipmap);
	afSafeDeleteTexture(texRenderTarget);
	if (framebufferObject) {
		glDeleteFramebuffers(1, &framebufferObject);
		framebufferObject = 0;
	}
	if (renderbufferObject) {
		glDeleteRenderbuffers(1, &renderbufferObject);
		renderbufferObject = 0;
	}
	afSafeDeleteVAO(vao);
	afSafeDeleteVAO(vaoFullScr);
}

static void HandleGLError(const char* func, int line, const char* command)
{
	GLenum r = glGetError();
	if (r != GL_NO_ERROR) {
		const char *err = nullptr;
		switch (r) {
#define E(er) case er: err = #er; break;
		E(GL_INVALID_ENUM)
		E(GL_INVALID_VALUE)
		E(GL_INVALID_OPERATION)
		E(GL_INVALID_FRAMEBUFFER_OPERATION)
#undef E
		default:
			printf("%s(%d): err=%d %s\n", func, line, r, command);
			return;
		}
		printf("%s(%d): %s %s\n", func, line, err, command);
	}
}

#define V(command) do{ command; HandleGLError(__FUNCTION__, __LINE__, #command); } while(0)

static const InputElement elements[] = {
	CInputElement(0, "vPosition", SF_R32G32B32_FLOAT, 0),
	CInputElement(0, "vNormal", SF_R32G32B32_FLOAT, 12),
};

static const InputElement elementsFullScr[] = {
	CInputElement(0, "vPosition", SF_R32G32B32_FLOAT, 0),
};

void WaterSurfaceClassic::Init()
{
	Destroy();

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

	AFIndex iboFullScrSrc[] = {0, 1, 2, 3};
	Vec2 vboFullScrSrc[] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

	vboFullScr = afCreateVertexBuffer(sizeof(vboFullScrSrc), &vboFullScrSrc[0]);
	iboFullScr = afCreateIndexBuffer(&iboFullScrSrc[0], dimof(iboFullScrSrc));

	shaderId = shaderMan.Create("water_classic");


//	const char* shaderName = "vivid";
	const char* shaderName = "letterbox";
	shaderIdFullScr = shaderMan.Create(shaderName);

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

	texRenderTarget = afCreateDynamicTexture(512, 512, AFDT_R5G6B5_UINT);

	glGenRenderbuffers(1, &renderbufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbufferObject);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 512, 512);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &framebufferObject);

	VBOID vertexBufferIds[] = { vbo };
	GLsizei strides[] = { sizeof(WaterVert) };
	vao = afCreateVAO(shaderId, elements, dimof(elements), 1, vertexBufferIds, strides, ibo);

	VBOID vertexBufferIdsFullScr[] = { vboFullScr };
	GLsizei stridesFullScr[] = { sizeof(Vec2) };
	vaoFullScr = afCreateVAO(shaderIdFullScr, elementsFullScr, dimof(elementsFullScr), 1, vertexBufferIdsFullScr, stridesFullScr, iboFullScr);
}

void WaterSurfaceClassic::UpdateRipple()
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

void WaterSurfaceClassic::Update()
{
	if (!shaderId) {
		return;
	}

	ivec2 scrSize = systemMetrics.GetScreenSize();
	float offset = 0.5f;
	float aspect = (float)scrSize.y / scrSize.x;
	if (aspect < 1) {
		matView = fastInv(translate(0, 0.5f * (1 - aspect), 0));
		matProj = Mat(
			1, 0, 0, 0,
			0, 1 / aspect, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	} else {
		matView = fastInv(translate(offset * (1 - 1 / aspect), 0, 0));
		matProj = Mat(
			aspect, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}

#if 0	// if glGetTextureLevelParameteriv available
	int storedW, storedH;
	glBindTexture(GL_TEXTURE_2D, texRenderTarget);
	glGetTextureLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &storedW);
	glGetTextureLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &storedH);
	if (w != storedW || h != storedH) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, nullptr);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, renderbufferObject);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &storedW);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &storedH);
	if (w != storedW || h != storedH) {
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
	}
#else
	if (scrSize.x != storedW || scrSize.y != storedH) {
		storedW = scrSize.x;
		storedH = scrSize.y;
		glBindTexture(GL_TEXTURE_2D, texRenderTarget);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, storedW, storedH, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, nullptr);
		glBindRenderbuffer(GL_RENDERBUFFER, renderbufferObject);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, storedW, storedH);
	}
#endif
}

void WaterSurfaceClassic::Draw()
{
	afDepthStencilMode(false);
	afBlendMode(BM_NONE);

	V(glBindBuffer(GL_ARRAY_BUFFER, vbo));

	UpdateRipple();

	shaderMan.Apply(shaderId);

	for (int i = 0; i < dimof(texFiles); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texId[i]);
		glBindSampler(i, texFiles[i].clamp ? samplerClamp : samplerRepeat);
	}

	glUniform1i(glGetUniformLocation(shaderId, "sampler0"), 0);
	glUniform1i(glGetUniformLocation(shaderId, "sampler1"), 1);
	glUniform1i(glGetUniformLocation(shaderId, "sampler2"), 2);
	glUniform1i(glGetUniformLocation(shaderId, "sampler3"), 3);
	glUniform1i(glGetUniformLocation(shaderId, "sampler4"), 4);
	glUniform1i(glGetUniformLocation(shaderId, "sampler5"), 5);
	double dummy;
	glUniform1f(glGetUniformLocation(shaderId, "time"), (float)modf(elapsedTime * (1.0f / loopTime), &dummy) * loopTime);

	Mat matW = q2m(Quat(Vec3(1,0,0), (float)M_PI / 180 * -90));
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "matW"), 1, GL_FALSE, &matW.m[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "matV"), 1, GL_FALSE, &matView.m[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "matP"), 1, GL_FALSE, &matProj.m[0][0]);

	V(glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject));
	V(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texRenderTarget, 0));
	V(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbufferObject));
	V(glBindRenderbuffer(GL_RENDERBUFFER, renderbufferObject));
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE) {
		glClearColor(0, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLE_STRIP, nIndi, GL_UNSIGNED_SHORT, 0);
		glBindVertexArray(0);
		V(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0));
		V(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0));

		V(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	//	V(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0));
//		glDrawElements(GL_TRIANGLE_STRIP, nIndi, GL_UNSIGNED_SHORT, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderMan.Apply(shaderIdFullScr);
		glUniform1i(glGetUniformLocation(shaderIdFullScr, "sampler"), 0);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texRenderTarget);
//		glBindTexture(GL_TEXTURE_2D, texId[1]);
		glBindSampler(0, samplerNoMipmap);

		glBindVertexArray(vaoFullScr);
		V(glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0));
		glBindVertexArray(0);
	}
}
