#include <stdafx.h>

SkyMan skyMan;

static const SamplerType samplers[] = { AFST_LINEAR_WRAP };

SkyMan::~SkyMan()
{
	assert(!texId);
}

void SkyMan::Create(const char *texFileName, const char* shader)
{
	Destroy();
	texId = afLoadTexture(texFileName, texDesc);
	renderStates.Create(shader, 0, nullptr, BM_NONE, DSM_DEPTH_CLOSEREQUAL_READONLY, CM_DISABLE, dimof(samplers), samplers);
}

void SkyMan::Draw()
{
	if (!texId) {
		return;
	}
	renderStates.Apply();

	Mat matV, matP;
	matrixMan.Get(MatrixMan::VIEW, matV);
	matrixMan.Get(MatrixMan::PROJ, matP);
	matV._41 = matV._42 = matV._43 = 0;
	Mat invVP = inv(matV * matP);

	UBOID uboId = afCreateUBO(sizeof(Mat));
	afWriteBuffer(uboId, &invVP, sizeof(invVP));
	afBindBufferToBindingPoint(uboId, 0);
	(texDesc.isCubeMap ? afBindCubeMapToBindingPoint : afBindTextureToBindingPoint)(texId, 0);

	afDraw(PT_TRIANGLESTRIP, 4);
	afBindTextureToBindingPoint(0, 0);
	afSafeDeleteBuffer(uboId);
}

void SkyMan::Destroy()
{
	afSafeDeleteTexture(texId);
}
