#include "stdafx.h"
#include "sky_man.h"
#include "stock_objects.h"

SkyMan skyMan;

static const SamplerType samplers[] = { AFST_LINEAR_WRAP };

SkyMan::~SkyMan()
{
	assert(!texRef);
	assert(!renderStates.IsReady());
}

void SkyMan::Create(const char *texFileName, const char* shader)
{
	Destroy();
	texRef = afLoadTexture(texFileName);
	int numElements = 0;
	const InputElement* elements = stockObjects.GetFullScreenInputElements(numElements);
	renderStates.Create(shader, numElements, elements, AFRS_DEPTH_CLOSEREQUAL_READONLY | AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM | AFRS_AUTO_DEPTH_STENCIL, arrayparam(samplers));
}

void SkyMan::Draw(AFCommandList& cmd, const ViewDesc& view)
{
	if (!renderStates.IsReady())
	{
		return;
	}
	cmd.SetRenderStates(renderStates);

	Mat matV = view.matView;
	matV._41 = matV._42 = matV._43 = 0;
	Mat invVP = inv(matV * view.matProj);
	cmd.SetBuffer(sizeof(invVP), &invVP, 1);
	stockObjects.ApplyFullScreenVertexBuffer(cmd);
	cmd.SetTexture(texRef, 0);
	cmd.Draw(4);
}

void SkyMan::Destroy()
{
	afSafeDeleteTexture(texRef);
	renderStates.Destroy();
}
