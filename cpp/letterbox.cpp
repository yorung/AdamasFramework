#include "stdafx.h"

LetterBox letterBox;

void LetterBox::LazyInit()
{
	if (renderStates.IsReady()) {
		return;
	}

	const static SamplerType samplers[] = {
		AFST_LINEAR_CLAMP,
	};
	int numElements = 0;
	const InputElement* elements = stockObjects.GetFullScreenInputElements(numElements);
	renderStates.Create("letterbox", numElements, elements, BM_NONE, DSM_DISABLE, CM_DISABLE, dimof(samplers), samplers);
}

void LetterBox::Draw(AFRenderTarget& target, SRVID srcTex)
{
	LazyInit();

	target.BeginRenderToThis();
	afBindSrv0(srcTex);
	renderStates.Apply();
	stockObjects.ApplyFullScreenVAO();
	afDraw(PT_TRIANGLESTRIP, 4);
}

void LetterBox::Destroy()
{
}
