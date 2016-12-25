#include "stdafx.h"

#ifndef AF_VULKAN

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
	renderStateGlowExtraction.Create("glow_extraction", numElements, elements, AFRS_NONE, 1, samplerTypes);
	renderStateGlowCopy.Create("glow_copy", numElements, elements, AFRS_NONE, 1, samplerTypes);
	renderStateGlowLastPass.Create("glow_lastpass", numElements, elements, AFRS_NONE, dimof(samplerTypes), samplerTypes);
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
	AFCommandList& cmd = afGetCommandList();
	stockObjects.ApplyFullScreenVertexBuffer(cmd);
	cmd.SetRenderStates(renderStateGlowExtraction);
	glowMap[0].BeginRenderToThis();
	cmd.SetTexture(srcTex, 0);
	cmd.Draw(4);

	cmd.SetRenderStates(renderStateGlowCopy);
	for (int i = 1; i < (int)dimof(glowMap); i++)
	{
		glowMap[i].BeginRenderToThis();
		cmd.SetTexture(glowMap[i - 1].GetTexture(), 0);
		cmd.Draw(4);
		cmd.SetTexture(SRVID(), 0);
	}

	cmd.SetRenderStates(renderStateGlowLastPass);
	target.BeginRenderToThis();
	for (int i = 0; i < (int)dimof(glowMap); i++)
	{
		cmd.SetTexture(glowMap[i].GetTexture(), i);
	}
	cmd.SetTexture(srcTex, 6);
	cmd.Draw(4);
	cmd.SetTexture(SRVID(), 6);
	for (int i = 0; i < (int)dimof(glowMap); i++)
	{
//		cmd.SetTexture(SRVID(), i);
	}
}
#endif
