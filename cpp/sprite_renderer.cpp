#include "stdafx.h"

SpriteRenderer spriteRenderer;

static const int MAX_SPRITES_IN_ONE_DRAW_CALL = 10;

struct SpriteVertex {
	Vec3 pos;
	uint32_t color;
	Vec2 uv;
};

SpriteRenderer::SpriteRenderer()
{
	sampler = 0;
	vbo = 0;
	ibo = 0;
	vao = 0;
	ubo = 0;
	shaderId = ShaderMan::INVALID_SMID;
}

SpriteRenderer::~SpriteRenderer()
{
	Destroy();
}

void SpriteRenderer::Destroy()
{
	afSafeDeleteSampler(sampler);
	afSafeDeleteBuffer(vbo);
	afSafeDeleteBuffer(ibo);
	afSafeDeleteBuffer(ubo);
	afSafeDeleteVAO(vao);
}

void SpriteRenderer::Init()
{
	Destroy();
	InputElement layout[] = {
		CInputElement(0, "POSITION", SF_R32G32B32_FLOAT, 0),
		CInputElement(0, "COLOR", SF_R8G8B8A8_UNORM, 12),
		CInputElement(0, "TEXCOORD", SF_R32G32_FLOAT, 16),
	};
	shaderId = shaderMan.Create("sprite", layout, dimof(layout), BM_ALPHA, DSM_DISABLE);

	sampler = afCreateSampler(SF_LINEAR, SW_CLAMP);
	vbo = afCreateDynamicVertexBuffer(sizeof(SpriteVertex) * MAX_SPRITES_IN_ONE_DRAW_CALL * 6);
	ubo = afCreateUBO(sizeof(Mat));
	ibo = afCreateQuadListIndexBuffer(MAX_SPRITES_IN_ONE_DRAW_CALL);

	int strides[] = { sizeof(SpriteVertex) };
	vao = afCreateVAO(layout, dimof(layout), 1, &vbo, strides, ibo);
}

static void StoreVertices(SpriteVertex v[4], float width, float height, uint32_t color, float uLeft, float vTop, float uRight, float vBottom, const Mat& matW)
{
	memset(v, 0, sizeof(SpriteVertex) * 4);
	v[2].pos.x = v[3].pos.x = width;
	v[1].pos.y = v[3].pos.y = height;
	v[0].uv.x = v[1].uv.x = uLeft;
	v[2].uv.x = v[3].uv.x = uRight;
	v[0].uv.y = v[2].uv.y = vTop;
	v[1].uv.y = v[3].uv.y = vBottom;
	for (int i = 0; i < 4; i++) {
		v[i].pos = transform(v[i].pos, matW);
		v[i].color = color;
	}
}

void SpriteRenderer::Draw(const SpriteCommands& sprites)
{
	Vec2 scrSize = systemMisc.GetScreenSize();
	Mat proj = ortho(0, scrSize.x, scrSize.y, 0, -1000, 1000);

	shaderMan.Apply(shaderId);
	afEnableBackFaceCulling(false);

	afWriteBuffer(ubo, &proj, sizeof(Mat));
	afBindBufferToBindingPoint(ubo, 0);
	afBindSamplerToBindingPoint(sampler, 0);
	afBindVAO(vao);

	SpriteVertex v[MAX_SPRITES_IN_ONE_DRAW_CALL][4];
	int numStoredSprites = 0;
	TexMan::TMID curTex = TexMan::INVALID_TMID;
	auto flush = [&] {
		if (numStoredSprites > 0) {
			afBindTextureToBindingPoint(curTex, 0);
			afWriteBuffer(vbo, v, sizeof(SpriteVertex) * 4 * numStoredSprites);
			afDrawIndexedTriangleList(6 * numStoredSprites, 0);
			numStoredSprites = 0;
		}
	};
	for (auto it : sprites) {
		if (curTex != it.tex || numStoredSprites == MAX_SPRITES_IN_ONE_DRAW_CALL) {
			flush();
		}
		if (numStoredSprites == 0) {
			curTex = it.tex;
		}
		const TexDesc* desc = texMan.GetTexDesc(it.tex);
		StoreVertices(
			v[numStoredSprites++],
			float(it.quad.z - it.quad.x),
			float(it.quad.w - it.quad.y),
			it.color,
			it.quad.x / (float)desc->size.x,
			it.quad.y / (float)desc->size.y,
			it.quad.z / (float)desc->size.x,
			it.quad.w / (float)desc->size.y,
			it.matW
		);
	}
	flush();
}
