#include "stdafx.h"

LetterBox letterBox;

void LetterBox::LazyInit()
{
	if (shader) {
		return;
	}

	const static SamplerType samplers[] = {
		AFST_LINEAR_CLAMP,
	};
	int numElements = 0;
	const InputElement* elements = stockObjects.GetFullScreenInputElements(numElements);
	shader = shaderMan.Create("letterbox", elements, numElements);
	renderStates.Create(BM_NONE, DSM_DISABLE, CM_DISABLE, dimof(samplers), samplers);
}

void LetterBox::Draw(AFRenderTarget& target, SRVID srcTex)
{
	LazyInit();

	target.BeginRenderToThis();
	afBindTextureToBindingPoint(srcTex, 0);
	shaderMan.Apply(shader);
	renderStates.Apply();
	stockObjects.ApplyFullScreenVAO();
	afDraw(PT_TRIANGLESTRIP, 4);
}

void LetterBox::Destroy()
{
	shader = 0;
}
