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

	int numElements = 0;
	const InputElement* elements = stockObjects.GetFullScreenInputElements(numElements);
	renderStates.Create(shader, numElements, elements, AFRS_DEPTH_CLOSEREQUAL_READONLY, arrayparam(samplers));
}

void SkyMan::Draw(AFCommandList& cmd)
{
	if (!renderStates.IsReady()) {
		return;
	}
	cmd.SetRenderStates(renderStates);

	Mat matV, matP;
	matrixMan.Get(MatrixMan::VIEW, matV);
	matrixMan.Get(MatrixMan::PROJ, matP);
	matV._41 = matV._42 = matV._43 = 0;
	Mat invVP = inv(matV * matP);
	cmd.SetBuffer(sizeof(invVP), &invVP, 1);
	stockObjects.ApplyFullScreenVertexBuffer(cmd);
#ifdef AF_GLES
	if (texDesc.isCubeMap)
	{
		afBindCubeMap(texId, 0);
		cmd.Draw(4);
		return;
	}
#endif
	cmd.SetTexture(texId, 0);
	cmd.Draw(4);
}

void SkyMan::Destroy()
{
	afSafeDeleteTexture(texId);
	renderStates.Destroy();
}
