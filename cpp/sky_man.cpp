#include <stdafx.h>

SkyMan skyMan;

SkyMan::SkyMan()
{
	texId = TexMan::INVALID_TMID;
	shaderId = ShaderMan::INVALID_SMID;
	uboId = 0;
}

SkyMan::~SkyMan()
{
}

void SkyMan::Create(const char *strCubeMapFile, const char *shader)
{
	Destroy();

	texId = texMan.Create(strCubeMapFile);
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

	char buf[100];
	Vec3 pos(0, 0, 0);
	Vec3 dir = transform(pos, invVP);
	snprintf(buf, sizeof(buf), "(0,0) = %f,%f,%f", dir.x, dir.y, dir.z);
	fontMan.DrawString(Vec2(100, 100), 20, buf);

	afWriteBuffer(uboId, &invVP, sizeof(invVP));
	afBindBufferToBindingPoint(uboId, 0);
	afBindTextureToBindingPoint(texId, 0);

	stockObjects.ApplyFullScreenVAO();	// dummy
	afDrawIndexedTriangleStrip(4);
}

void SkyMan::Destroy()
{
}
