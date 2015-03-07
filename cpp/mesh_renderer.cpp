#include "stdafx.h"

struct MeshConstantBuffer
{
	Mat matW;
	Mat matV;
	Mat matP;
	Vec4 faceColor;
	Vec4 emissive;
	Vec3 camPos;
	float padding1;
};

MeshRenderer::MeshRenderer()
{
	posBuffer = 0;
	colorBuffer = 0;
	skinBuffer = 0;
	pIndexBuffer = 0;
}

MeshRenderer::~MeshRenderer()
{
	Destroy();
}

void MeshRenderer::Destroy()
{
	afSafeDeleteBuffer(pIndexBuffer);
	afSafeDeleteBuffer(posBuffer);
	afSafeDeleteBuffer(colorBuffer);
	afSafeDeleteBuffer(skinBuffer);
}

void MeshRenderer::Init(const Block& block)
{
	block.Verify();
	if (!block.vertices.empty() && !block.indices.empty() && !block.color.empty()) {
		Init(block.vertices.size(), &block.vertices[0], &block.color[0], &block.skin[0], block.indices.size(), &block.indices[0]);
	}
}

void MeshRenderer::Init(int numVertices, const MeshVertex* vertices, const MeshColor* color, const MeshSkin* skin, int numIndices, const AFIndex* indices)
{
	Destroy();
	static const InputElement elements[] = {
		{ 0, "POSITION", SF_R32G32B32_FLOAT, 0 },
		{ 0, "NORMAL", SF_R32G32B32_FLOAT, 12 },
		{ 1, "vBlendWeights", SF_R32G32B32_FLOAT, 0 },
		{ 1, "vBlendIndices", SF_R8G8B8A8_UINT, 12 },
		{ 2, "vColor", SF_R8G8B8A8_UNORM, 0 },
		{ 2, "vTexcoord", SF_R32G32_FLOAT, 4 },
	};
	shaderId = shaderMan.Create("skin", elements, dimof(elements));
	assert(shaderId);
	posBuffer = afCreateVertexBuffer(numVertices * sizeof(MeshVertex), vertices);
	skinBuffer = afCreateVertexBuffer(numVertices * sizeof(MeshSkin), skin);
	colorBuffer = afCreateVertexBuffer(numVertices * sizeof(MeshColor), color);
	pIndexBuffer = afCreateIndexBuffer(indices, numIndices);
}
/*
void MeshRenderer::Calc(const Mat BoneMatrices[BONE_MAX], const Block& block) const
{
	ID3D11ShaderResourceView* srvPos;
	ID3D11ShaderResourceView* srvSkin;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.BufferEx.FirstElement = 0;
		desc.BufferEx.NumElements = block.vertices.size();
		HRESULT hr = deviceMan11.GetDevice()->CreateShaderResourceView(posBuffer, &desc, &srvPos);
		hr = deviceMan11.GetDevice()->CreateShaderResourceView(skinBuffer, &desc, &srvSkin);
	}

	ID3D11UnorderedAccessView* uav;
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = block.vertices.size();
		HRESULT hr = deviceMan11.GetDevice()->CreateUnorderedAccessView(skinnedPosBuffer, &desc, &uav);
	}

	computeShaderSkinning.Dispatch(BoneMatrices, srvPos, srvSkin, uav, block.vertices.size());
	SAFE_RELEASE(srvPos);
	SAFE_RELEASE(srvSkin);
	SAFE_RELEASE(uav);
}

void MeshRenderer11::Draw(const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block) const
{
	Calc(BoneMatrices, block);

	deviceMan11.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shaderMan.Apply(shaderId);

	Mat matWorld, matView, matProj;
	matrixMan.Get(MatrixMan::WORLD, matWorld);
	matrixMan.Get(MatrixMan::VIEW, matView);
	matrixMan.Get(MatrixMan::PROJ, matProj);

	deviceMan11.GetContext()->OMSetDepthStencilState(pDSState, 1);
	deviceMan11.GetContext()->PSSetSamplers(0, 1, &pSamplerState);

	deviceMan11.GetContext()->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	UINT strides[] = { sizeof(MeshColor) };
	UINT offsets[] = { 0 };
	deviceMan11.GetContext()->IASetVertexBuffers(0, 1, &colorBuffer, strides, offsets);

	ID3D11ShaderResourceView* srvSkinnedPos;
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.BufferEx.FirstElement = 0;
		desc.BufferEx.NumElements = block.vertices.size();
		HRESULT hr = deviceMan11.GetDevice()->CreateShaderResourceView(skinnedPosBuffer, &desc, &srvSkinnedPos);
	}
	deviceMan11.GetContext()->VSSetShaderResources(0, 1, &srvSkinnedPos);
	SAFE_RELEASE(srvSkinnedPos);

	for (int j = 0; (unsigned)j < block.materialMaps.size(); j++) {
		const MaterialMap& matMap = block.materialMaps[j];
		const Material* mat = matMan.Get(matMap.materialId);
		assert(mat);
		ID3D11ShaderResourceView* tx = texMan.Get(mat->tmid);
		deviceMan11.GetContext()->PSSetShaderResources(0, 1, &tx);

		MeshConstantBuffer cBuf;
		cBuf.matW = matWorld;
		cBuf.matV = matView;
		cBuf.matP = matProj;
		cBuf.faceColor = mat->faceColor;
		cBuf.emissive = mat->emissive;
		cBuf.camPos = fastInv(matView).GetRow(3);
		bufferMan.Write(constantBufferId, &cBuf);
		const auto buf = bufferMan.Get(constantBufferId);
		deviceMan11.GetContext()->VSSetConstantBuffers(0, 1, &buf);
		deviceMan11.GetContext()->PSSetConstantBuffers(0, 1, &buf);

		deviceMan11.GetContext()->DrawIndexed(matMap.faces * 3, matMap.faceStartIndex * 3, 0);
	}
}

*/

void MeshRenderer::Draw(const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block) const
{
	shaderMan.Apply(shaderId);
	GLuint verts[] = { posBuffer, skinBuffer, colorBuffer };
	GLsizei strides[] = { sizeof(MeshVertex), sizeof(MeshSkin), sizeof(MeshColor) };
	shaderMan.SetVertexBuffers(shaderId, block.indices.size(), verts, strides);

	Mat matW, matV, matP;
	matrixMan.Get(MatrixMan::WORLD, matW);
	matrixMan.Get(MatrixMan::VIEW, matV);
	matrixMan.Get(MatrixMan::PROJ, matP);

	glUniformMatrix4fv(glGetUniformLocation(shaderId, "matW"), 1, GL_FALSE, &matW.m[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "matV"), 1, GL_FALSE, &matV.m[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "matP"), 1, GL_FALSE, &matP.m[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "bones"), BONE_MAX, GL_FALSE, &BoneMatrices[0].m[0][0]);
	glUniform1i(glGetUniformLocation(shaderId, "sampler"), 0);

	glActiveTexture(GL_TEXTURE0);

	for (int j = 0; (unsigned)j < block.materialMaps.size(); j++) {
		const MaterialMap& matMap = block.materialMaps[j];
		const Material* mat = matMan.Get(matMap.materialId);
		assert(mat);

		glBindTexture(GL_TEXTURE_2D, mat->tmid);
		/*
		MeshConstantBuffer cBuf;
		cBuf.matW = matWorld;
		cBuf.matV = matView;
		cBuf.matP = matProj;
		cBuf.faceColor = mat->faceColor;
		cBuf.emissive = mat->emissive;
		cBuf.camPos = fastInv(matView).GetRow(3);
		bufferMan.Write(constantBufferId, &cBuf);
		const auto buf = bufferMan.Get(constantBufferId);
		deviceMan11.GetContext()->VSSetConstantBuffers(0, 1, &buf);
		deviceMan11.GetContext()->PSSetConstantBuffers(0, 1, &buf);
		*/
		afDrawIndexedTriangleList(pIndexBuffer, matMap.faces * 3, matMap.faceStartIndex * 3);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
