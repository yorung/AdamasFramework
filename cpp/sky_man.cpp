#include <stdafx.h>

SkyMan skyMan;

SkyMan::~SkyMan()
{
	assert(!uboId);
	assert(!texId);
}

void SkyMan::Create(const char *texFileName, const char* shader)
{
	Destroy();

	texId = afLoadTexture(texFileName, texDesc);
	shaderId = shaderMan.Create(shader);
	uboId = afCreateUBO(sizeof(Mat));
}

void SkyMan::Draw()
{
	if (!uboId) {
		return;
	}

	if (!shaderId) {
		return;
	}

	shaderMan.Apply(shaderId);

	Mat matV, matP;
	matrixMan.Get(MatrixMan::VIEW, matV);
	matrixMan.Get(MatrixMan::PROJ, matP);
	matV._41 = matV._42 = matV._43 = 0;
	Mat invVP = inv(matV * matP);

	afWriteBuffer(uboId, &invVP, sizeof(invVP));
	afBindBufferToBindingPoint(uboId, 0);
	(texDesc.isCubeMap ? afBindCubeMapToBindingPoint : afBindTextureToBindingPoint)(texId, 0);

	stockObjects.ApplyFullScreenVAO();
	afDrawTriangleStrip(4);
	afBindVAO(0);
	afBindTextureToBindingPoint(0, 0);
}

void SkyMan::Destroy()
{
	afSafeDeleteBuffer(uboId);
	afSafeDeleteTexture(texId);
}
