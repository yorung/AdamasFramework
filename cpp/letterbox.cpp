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
	afLayoutSamplerBindingManually(shader, "sampler", 0);
}

void LetterBox::Draw(AFRenderTarget& target, GLuint srcTex)
{
	LazyInit();

	target.BeginRenderToThis();
	afBindTextureToBindingPoint(srcTex, 0);
	afBindSamplerToBindingPoint(stockObjects.GetNoMipmapSampler(), 0);
	shaderMan.Apply(shader);
	stockObjects.ApplyFullScreenVAO();
	afDrawTriangleStrip(4);
}

void LetterBox::Destroy()
{
	shader = 0;
}
