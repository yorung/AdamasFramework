#include "stdafx.h"

SpriteRenderer spriteRenderer;

static const int MAX_SPRITES_IN_ONE_DRAW_CALL = 10;

struct SpriteVertex {
	Vec3 pos;
	uint32_t color;
	Vec2 uv;
};

SpriteRenderer::~SpriteRenderer()
{
	Destroy();
}

void SpriteRenderer::Destroy()
{
	quadListVertexBuffer.Destroy();
}

void SpriteRenderer::Init()
{
	Destroy();
	static InputElement layout[] = {
		CInputElement("POSITION", SF_R32G32B32_FLOAT, 0),
		CInputElement("COLOR", SF_R8G8B8A8_UNORM, 12),
		CInputElement("TEXCOORD", SF_R32G32_FLOAT, 16),
	};
	const static SamplerType samplers[] = { AFST_LINEAR_CLAMP };
	renderStates.Create("sprite", dimof(layout), layout, BM_ALPHA, DSM_DISABLE, CM_DISABLE, dimof(samplers), samplers);
	quadListVertexBuffer.Create(layout, dimof(layout), sizeof(SpriteVertex), MAX_SPRITES_IN_ONE_DRAW_CALL);
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

	renderStates.Apply();

	UBOID ubo = afCreateUBO(sizeof(Mat));
	afWriteBuffer(ubo, &proj, sizeof(Mat));
	afBindBufferToBindingPoint(ubo, 0);

	SpriteVertex v[MAX_SPRITES_IN_ONE_DRAW_CALL][4];
	int numStoredSprites = 0;
	SRVID curTex;
	auto flush = [&] {
		if (numStoredSprites > 0) {
			afBindTextureToBindingPoint(curTex, 0);
			quadListVertexBuffer.Apply(v, sizeof(SpriteVertex) * 4 * numStoredSprites);
			afDrawIndexed(PT_TRIANGLELIST, 6 * numStoredSprites, 0);
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
		IVec2 size = afGetTextureSize(it.tex);
		StoreVertices(
			v[numStoredSprites++],
			float(it.quad.z - it.quad.x),
			float(it.quad.w - it.quad.y),
			it.color,
			it.quad.x / (float)size.x,
			it.quad.y / (float)size.y,
			it.quad.z / (float)size.x,
			it.quad.w / (float)size.y,
			it.matW
		);
	}
	flush();
	afSafeDeleteBuffer(ubo);
}
