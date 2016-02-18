#include "stdafx.h"

static const InputElement elements[] = {
	CInputElement("POSITION", SF_R32G32_FLOAT, 0),
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

void StockObjects::CreateSamplers()
{
	samplerRepeat = afCreateSampler(SF_MIPMAP, SW_REPEAT);
	samplerClamp = afCreateSampler(SF_MIPMAP, SW_CLAMP);
	samplerNoMipmap = afCreateSampler(SF_LINEAR, SW_CLAMP);
}

void StockObjects::Init()
{
	CreateFullScreenVAO();
	CreateSamplers();
}

void StockObjects::Destroy()
{
	afSafeDeleteBuffer(vboFullScr);
	afSafeDeleteBuffer(iboFullScr);
	afSafeDeleteVAO(vaoFullScr);

	afSafeDeleteSampler(samplerRepeat);
	afSafeDeleteSampler(samplerClamp);
	afSafeDeleteSampler(samplerNoMipmap);
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
