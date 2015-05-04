#include "stdafx.h"

StockObjects stockObjects;

StockObjects::StockObjects()
{
	vboFullScr = 0;
	iboFullScr = 0;
	vaoFullScr = 0;
}

void StockObjects::Init()
{
/*
	AFIndex iboFullScrSrc[] = { 0, 1, 2, 3 };
	Vec2 vboFullScrSrc[] = { { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 } };

	vboFullScr = afCreateVertexBuffer(sizeof(vboFullScrSrc), &vboFullScrSrc[0]);
	iboFullScr = afCreateIndexBuffer(&iboFullScrSrc[0], dimof(iboFullScrSrc));

	VBOID vertexBufferIdsFullScr[] = { vboFullScr };
	GLsizei stridesFullScr[] = { sizeof(Vec2) };
	vaoFullScr = afCreateVAO(shaderIdFullScr, elementsFullScr, dimof(elementsFullScr), 1, vertexBufferIdsFullScr, stridesFullScr, iboFullScr);
	*/
}



void StockObjects::Destroy()
{
	afSafeDeleteBuffer(vboFullScr);
	afSafeDeleteBuffer(iboFullScr);
	afSafeDeleteVAO(vaoFullScr);
}