#include "stdafx.h"

static const InputElement elements[] =
{
	AF_INPUT_ELEMENT(0, "POSITION", AFF_R32G32_FLOAT, 0),
};

StockObjects stockObjects;

void StockObjects::CreateFullScreenVAO()
{
	Vec2 vboFullScrSrc[] = { { -1, 1 }, { -1, -1 }, { 1, 1 }, { 1, -1 } };
	vboFullScr = afCreateVertexBuffer(sizeof(vboFullScrSrc), &vboFullScrSrc[0]);
}

void StockObjects::Create()
{
	CreateFullScreenVAO();
#if defined(AF_GLES31) || defined(AF_DX11)
	for (int i = 0; i < AFST_MAX; i++)
	{
		builtInSamplers[i] = afCreateSampler((SamplerType)i);
	}
#endif
}

void StockObjects::Destroy()
{
	afSafeDeleteBuffer(vboFullScr);
#if defined(AF_GLES31) || defined(AF_DX11)
	for (SAMPLERID& sampler : builtInSamplers)
	{
		afSafeDeleteSampler(sampler);
	}
#endif
}

void StockObjects::ApplyFullScreenVertexBuffer(AFCommandList& cmd) const
{
	cmd.SetVertexBuffer(vboFullScr, sizeof(Vec2));
}

const InputElement* StockObjects::GetFullScreenInputElements(int& numElements) const
{
	numElements = dimof(elements);
	return elements;
}
