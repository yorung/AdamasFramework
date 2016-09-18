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
	iboFullScr = afCreateIndexBuffer(&iboFullScrSrc[0], dimof(iboFullScrSrc));

	VBOID vertexBufferIdsFullScr[] = { vboFullScr };
	int stridesFullScr[] = { sizeof(Vec2) };

	vaoFullScr = afCreateVAO(elements, dimof(elements), 1, vertexBufferIdsFullScr, stridesFullScr, iboFullScr);
}

void StockObjects::Init()
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
	afSafeDeleteVAO(vaoFullScr);
#ifndef AF_DX12
	for (SAMPLERID& sampler : builtInSamplers)
	{
		afSafeDeleteSampler(sampler);
	}
#endif
}

void StockObjects::ApplyFullScreenVAO() const
{
	afBindVAO(vaoFullScr);
}

const InputElement* StockObjects::GetFullScreenInputElements(int& numElements) const
{
	numElements = dimof(elements);
	return elements;
}
