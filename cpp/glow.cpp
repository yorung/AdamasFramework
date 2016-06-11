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
		it.Init(IVec2(texSize, texSize), AFDT_R8G8B8A8_UNORM, AFDT_INVALID);
		it.BeginRenderToThis();	// clear textures
		texSize /= 2;
	}

	int numElements = 0;
	const InputElement* elements = stockObjects.GetFullScreenInputElements(numElements);
	shaderGlowExtraction = shaderMan.Create("glow_extraction", elements, numElements);
	assert(shaderGlowExtraction);
	shaderGlowCopy = shaderMan.Create("glow_copy", elements, numElements);
	assert(shaderGlowCopy);
	shaderGlowLastPass = shaderMan.Create("glow_lastpass", elements, numElements);
	assert(shaderGlowLastPass);

	renderStates.Init(BM_NONE, DSM_DISABLE, CM_DISABLE);
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

void Glow::MakeGlow(AFRenderTarget& target, SRVID srcTex)
{
	LazyInit();
	afBindSamplerToBindingPoint(stockObjects.GetClampSampler(), 0);
	stockObjects.ApplyFullScreenVAO();

	renderStates.Apply();

	shaderMan.Apply(shaderGlowExtraction);
	glowMap[0].BeginRenderToThis();
	afBindTextureToBindingPoint(srcTex, 0);
	afDraw(PT_TRIANGLESTRIP, 4);

	shaderMan.Apply(shaderGlowCopy);
	for (int i = 1; i < (int)dimof(glowMap); i++) {
		glowMap[i].BeginRenderToThis();
		afBindTextureToBindingPoint(glowMap[i - 1].GetTexture(), 0);
		afDraw(PT_TRIANGLESTRIP, 4);
	}

	shaderMan.Apply(shaderGlowLastPass);
	target.BeginRenderToThis();
	for (int i = 1; i < (int)dimof(glowMap); i++) {
		afBindTextureToBindingPoint(glowMap[i].GetTexture(), i);
	}
	afBindTextureToBindingPoint(srcTex, 6);
	afDraw(PT_TRIANGLESTRIP, 4);
	afBindVAO(0);
}
