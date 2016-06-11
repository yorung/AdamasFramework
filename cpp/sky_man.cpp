#include <stdafx.h>

SkyMan skyMan;

SkyMan::~SkyMan()
{
	assert(!uboId);
	assert(!texId);
	assert(!sampler);
}

void SkyMan::Create(const char *texFileName, const char* shader)
{
	Destroy();
	texId = afLoadTexture(texFileName, texDesc);
	shaderId = shaderMan.Create(shader, nullptr, 0);
	renderStates.Init(BM_NONE, DSM_DEPTH_CLOSEREQUAL_READONLY, CM_DISABLE);
	uboId = afCreateUBO(sizeof(Mat));
	sampler = afCreateSampler(SF_LINEAR, SW_REPEAT);
}

void SkyMan::Draw()
{
	if (!texId) {
		return;
	}
	if (!uboId) {
		return;
	}

	if (!shaderId) {
		return;
	}

	shaderMan.Apply(shaderId);
	renderStates.Apply();

	Mat matV, matP;
	matrixMan.Get(MatrixMan::VIEW, matV);
	matrixMan.Get(MatrixMan::PROJ, matP);
	matV._41 = matV._42 = matV._43 = 0;
	Mat invVP = inv(matV * matP);

	afWriteBuffer(uboId, &invVP, sizeof(invVP));
	afBindBufferToBindingPoint(uboId, 0);
	(texDesc.isCubeMap ? afBindCubeMapToBindingPoint : afBindTextureToBindingPoint)(texId, 0);

	afBindSamplerToBindingPoint(sampler, 0);
	afDraw(PT_TRIANGLESTRIP, 4);
	afBindTextureToBindingPoint(0, 0);
}

void SkyMan::Destroy()
{
	afSafeDeleteBuffer(uboId);
	afSafeDeleteTexture(texId);
	afSafeDeleteSampler(sampler);
}
