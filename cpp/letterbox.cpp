#include "stdafx.h"

LetterBox letterBox;

void LetterBox::LazyInit()
{
	if (shader) {
		return;
	}

	int numElements = 0;
	const InputElement* elements = stockObjects.GetFullScreenInputElements(numElements);
	shader = shaderMan.Create("letterbox", elements, numElements, BM_NONE, DSM_DISABLE, CM_DISABLE);
}

void LetterBox::Draw(AFRenderTarget& target, SRVID srcTex)
{
	LazyInit();

	target.BeginRenderToThis();
	afBindTextureToBindingPoint(srcTex, 0);
	afBindSamplerToBindingPoint(stockObjects.GetNoMipmapSampler(), 0);
	shaderMan.Apply(shader);
	stockObjects.ApplyFullScreenVAO();
	afDraw(PT_TRIANGLESTRIP, 4);
}

void LetterBox::Destroy()
{
	shader = 0;
}
