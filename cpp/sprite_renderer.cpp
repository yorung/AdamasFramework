#include "stdafx.h"
#include "sprite_renderer.h"
#include "system_misc.h"

SpriteRenderer spriteRenderer;

static const int MAX_SPRITES_IN_ONE_DRAW_CALL = 10;

struct SpriteVertex
{
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
	renderStates.Destroy();
}

void SpriteRenderer::Create()
{
	Destroy();
	static InputElement layout[] =
	{
		AF_INPUT_ELEMENT(0, "POSITION", AFF_R32G32B32_FLOAT, 0),
		AF_INPUT_ELEMENT(1, "COLOR", AFF_R8G8B8A8_UNORM, 12),
		AF_INPUT_ELEMENT(2, "TEXCOORD", AFF_R32G32_FLOAT, 16),
	};
	const static SamplerType samplers[] = { AFST_LINEAR_CLAMP };
	renderStates.Create("sprite", arrayparam(layout), AFRS_ALPHA_BLEND | AFRS_PRIMITIVE_TRIANGLELIST | AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM | AFRS_AUTO_DEPTH_STENCIL, arrayparam(samplers));
	quadListVertexBuffer.Create(sizeof(SpriteVertex), MAX_SPRITES_IN_ONE_DRAW_CALL);
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

void SpriteRenderer::Draw(AFCommandList& cmd, const SpriteCommands& sprites)
{
	if (sprites.empty())
	{
		return;
	}

	Vec2 scrSize = systemMisc.GetScreenSize();
	Mat proj = ortho(0, scrSize.x, scrSize.y, 0, -1000, 1000);

	cmd.SetRenderStates(renderStates);
	cmd.SetBuffer(sizeof(Mat), &proj, 1);

	SpriteVertex v[MAX_SPRITES_IN_ONE_DRAW_CALL][4];
	int numStoredSprites = 0;
	AFTexRef curTex;
	auto flush = [&] {
		if (numStoredSprites > 0)
		{
			cmd.SetTexture(curTex, 0);
			quadListVertexBuffer.Apply(cmd, v, sizeof(SpriteVertex) * 4 * numStoredSprites);
			cmd.DrawIndexed(6 * numStoredSprites, 0);
			numStoredSprites = 0;
		}
	};
	for (auto it : sprites)
	{
		if (curTex != it.tex || numStoredSprites == MAX_SPRITES_IN_ONE_DRAW_CALL)
		{
			flush();
		}
		if (numStoredSprites == 0)
		{
			curTex = it.tex;
		}
		StoreVertices(v[numStoredSprites++], 1.f, 1.f, it.color, it.quad.x, it.quad.y, it.quad.z, it.quad.w, it.matW);
	}
	flush();
}
