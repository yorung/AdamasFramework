#include <stdafx.h>

SkyMan skyMan;

static const SamplerType samplers[] = { AFST_LINEAR_WRAP };

SkyMan::~SkyMan()
{
	assert(!texId);
	assert(!renderStates.IsReady());
}

void SkyMan::Create(const char *texFileName, const char* shader)
{
	Destroy();
	texId = afLoadTexture(texFileName, texDesc);
	renderStates.Create(shader, 0, nullptr, AFRS_DEPTH_CLOSEREQUAL_READONLY, arrayparam(samplers));
}

void SkyMan::Draw()
{
	if (!renderStates.IsReady()) {
		return;
	}
	renderStates.Apply();

	Mat matV, matP;
	matrixMan.Get(MatrixMan::VIEW, matV);
	matrixMan.Get(MatrixMan::PROJ, matP);
	matV._41 = matV._42 = matV._43 = 0;
	Mat invVP = inv(matV * matP);
	afBindBuffer(sizeof(invVP), &invVP, 1);
#ifdef AF_VULKAN
	afBindTexture(texId, 0);
#else
	(texDesc.isCubeMap ? afBindCubeMap : afBindTexture)(texId, 0);
#endif
	afDraw(4);
}

void SkyMan::Destroy()
{
	afSafeDeleteTexture(texId);
	renderStates.Destroy();
}
