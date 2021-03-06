#include "stdafx.h"
#include "letterbox.h"
#include "stock_objects.h"

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
	renderStates.Create("letterbox", numElements, elements, AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM | AFRS_AUTO_DEPTH_STENCIL, arrayparam(samplers));
}

void LetterBox::Draw(AFCommandList& cmd, AFRenderTarget& target, AFTexRef srcTex)
{
	LazyInit();

	cmd.SetTexture(srcTex, 0);
	renderStates.Apply();
	stockObjects.ApplyFullScreenVertexBuffer(cmd);

	target.BeginRenderToThis();
	cmd.Draw(4);
//	target.EndRenderToThis();	// workaround: EndRenderToThis called in app.cpp
}

void LetterBox::Destroy()
{
	renderStates.Destroy();
}
