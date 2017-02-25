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
	renderStates.Create("letterbox", numElements, elements, AFRS_NONE, arrayparam(samplers));
}

void LetterBox::Draw(AFCommandList& cmd, AFRenderTarget& target, AFTexRef srcTex)
{
	LazyInit();

	target.BeginRenderToThis();
	cmd.SetTexture(srcTex, 0);
	renderStates.Apply();
	stockObjects.ApplyFullScreenVertexBuffer(cmd);
	cmd.Draw(4);
}

void LetterBox::Destroy()
{
	renderStates.Destroy();
}
