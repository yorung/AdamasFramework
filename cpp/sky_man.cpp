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
	renderStates.Create(
#ifdef AF_DX12
		AFDL_CBV0_SRV0,
#endif
		shader, 0, nullptr, BM_NONE, DSM_DEPTH_CLOSEREQUAL_READONLY, CM_DISABLE, dimof(samplers), samplers);
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
#ifdef AF_DX12
	afBindCbv0Srv0(&invVP, sizeof(invVP), texId);
#else
	UBOID ubo = afBindCbv0(&invVP, sizeof(invVP));
	(texDesc.isCubeMap ? afBindCubeMapToBindingPoint : afBindTextureToBindingPoint)(texId, 0);
#endif
	afDraw(PT_TRIANGLESTRIP, 4);
#ifndef AF_DX12
	afSafeDeleteBuffer(ubo);
#endif
}

void SkyMan::Destroy()
{
	afSafeDeleteTexture(texId);
	renderStates.Destroy();
}
