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
	renderStates.Create("letterbox", numElements, elements, AFRS_OFFSCREEN_RENDER_TARGET_B8G8R8A8_UNORM, arrayparam(samplers));
}

void LetterBox::Draw(AFCommandList& cmd, AFRenderTarget& target, AFTexRef srcTex)
{
	LazyInit();

	cmd.SetTexture(srcTex, 0);
	renderStates.Apply();
	stockObjects.ApplyFullScreenVertexBuffer(cmd);

	target.BeginRenderToThis();
	cmd.Draw(4);
	target.EndRenderToThis();
}

void LetterBox::Destroy()
{
	renderStates.Destroy();
}
