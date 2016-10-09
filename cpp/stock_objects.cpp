#include "stdafx.h"

static const InputElement elements[] = {
	CInputElement("POSITION", AFF_R32G32_FLOAT, 0),
};

StockObjects stockObjects;

void StockObjects::CreateFullScreenVAO()
{
	AFIndex iboFullScrSrc[] = { 0, 1, 2, 3 };
	Vec2 vboFullScrSrc[] = { { -1, 1 }, { -1, -1 }, { 1, 1 }, { 1, -1 } };

	vboFullScr = afCreateVertexBuffer(sizeof(vboFullScrSrc), &vboFullScrSrc[0]);
	iboFullScr = afCreateIndexBuffer(arrayparam(iboFullScrSrc));
}

void StockObjects::Create()
{
	CreateFullScreenVAO();
#ifndef AF_DX12
	for (int i = 0; i < AFST_MAX; i++)
	{
		builtInSamplers[i] = afCreateSampler((SamplerType)i);
	}
#endif
}

void StockObjects::Destroy()
{
	afSafeDeleteBuffer(vboFullScr);
	afSafeDeleteBuffer(iboFullScr);
#ifndef AF_DX12
	for (SAMPLERID& sampler : builtInSamplers)
	{
		afSafeDeleteSampler(sampler);
	}
#endif
}

void StockObjects::ApplyFullScreenVertexBuffer() const
{
	afSetVertexBuffer(vboFullScr, sizeof(Vec2));
	afSetIndexBuffer(iboFullScr);
}

const InputElement* StockObjects::GetFullScreenInputElements(int& numElements) const
{
	numElements = dimof(elements);
	return elements;
}
