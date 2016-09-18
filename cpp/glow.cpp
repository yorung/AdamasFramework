#include "stdafx.h"

Glow glow;
AFRenderTarget glowMap[6];
const static SamplerType samplerTypes[] = { AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP, AFST_LINEAR_CLAMP };

const int GLOW_WH = 128;

void Glow::LazyInit()
{
	assert(dimof(glowMap) + 1 == dimof(samplerTypes));
	if (renderStateGlowExtraction.IsReady()) {
		return;
	}

	int texSize = GLOW_WH;
	for (auto& it : glowMap) {
		it.Init(IVec2(texSize, texSize), AFF_R8G8B8A8_UNORM, AFF_INVALID);
		it.BeginRenderToThis();	// clear textures
		texSize /= 2;
	}

	int numElements = 0;
	const InputElement* elements = stockObjects.GetFullScreenInputElements(numElements);
	renderStateGlowExtraction.Create("glow_extraction", numElements, elements, BM_NONE, DSM_DISABLE, CM_DISABLE, 1, samplerTypes);
	renderStateGlowCopy.Create("glow_copy", numElements, elements, BM_NONE, DSM_DISABLE, CM_DISABLE, 1, samplerTypes);
	renderStateGlowLastPass.Create("glow_lastpass", numElements, elements, BM_NONE, DSM_DISABLE, CM_DISABLE, dimof(samplerTypes), samplerTypes);
}

void Glow::Destroy()
{
	for (auto& it : glowMap) {
		it.Destroy();
	}
	renderStateGlowLastPass.Destroy();
	renderStateGlowExtraction.Destroy();
	renderStateGlowCopy.Destroy();
}

void Glow::MakeGlow(AFRenderTarget& target, SRVID srcTex)
{
	LazyInit();
	stockObjects.ApplyFullScreenVAO();
	renderStateGlowExtraction.Apply();
	glowMap[0].BeginRenderToThis();
	afBindTextureToBindingPoint(srcTex, 0);
	afDraw(4);

	renderStateGlowCopy.Apply();
	for (int i = 1; i < (int)dimof(glowMap); i++) {
		glowMap[i].BeginRenderToThis();
		afBindTextureToBindingPoint(glowMap[i - 1].GetTexture(), 0);
		afDraw(4);
	}

	renderStateGlowLastPass.Apply();
	target.BeginRenderToThis();
	for (int i = 0; i < (int)dimof(glowMap); i++) {
		afBindTextureToBindingPoint(glowMap[i].GetTexture(), i);
	}
	afBindTextureToBindingPoint(srcTex, 6);
	afDraw(4);
	afBindVAO(0);
}
