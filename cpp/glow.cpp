#include "stdafx.h"

Glow glow;
AFRenderTarget glowMap[6];

const int GLOW_WH = 128;

void Glow::LazyInit()
{
	if (shaderGlowExtraction) {
		return;
	}

	int texSize = GLOW_WH;
	for (auto& it : glowMap) {
		it.Init(ivec2(texSize, texSize), AFDT_R8G8B8A8_UNORM, AFDT_INVALID);
		it.BeginRenderToThis();	// clear textures
		texSize /= 2;
	}

	shaderGlowExtraction = shaderMan.Create("glow_extraction", stockObjects.GetFullScreenVertexAttributeLayout());
	assert(shaderGlowExtraction);
	shaderGlowCopy = shaderMan.Create("glow_copy", stockObjects.GetFullScreenVertexAttributeLayout());
	assert(shaderGlowCopy);
	shaderGlowLastPass = shaderMan.Create("glow_lastpass", stockObjects.GetFullScreenVertexAttributeLayout());
	assert(shaderGlowLastPass);

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

void Glow::Destroy()
{
	for (auto& it : glowMap) {
		it.Destroy();
	}
	shaderGlowExtraction = 0;
	shaderGlowCopy = 0;
	shaderGlowLastPass = 0;
}

void Glow::MakeGlow(AFRenderTarget& target, GLuint srcTex)
{
	LazyInit();
	glBindSampler(0, stockObjects.GetClampSampler());
	stockObjects.ApplyFullScreenVAO();

	shaderMan.Apply(shaderGlowExtraction);
	glowMap[0].BeginRenderToThis();
	afBindTextureToBindingPoint(srcTex, 0);
	afDrawTriangleStrip(4);

	shaderMan.Apply(shaderGlowCopy);
	for (int i = 1; i < (int)dimof(glowMap); i++) {
		glowMap[i].BeginRenderToThis();
		afBindTextureToBindingPoint(glowMap[i - 1].GetTexture(), 0);
		afDrawTriangleStrip(4);
	}

	shaderMan.Apply(shaderGlowLastPass);
	target.BeginRenderToThis();
	for (int i = 1; i < (int)dimof(glowMap); i++) {
		afBindTextureToBindingPoint(glowMap[i].GetTexture(), i);
	}
	afBindTextureToBindingPoint(srcTex, 6);
	afDrawTriangleStrip(4);
	afBindVAO(0);
}
