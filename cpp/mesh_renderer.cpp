#include "stdafx.h"

static const uint16_t perInstanceBufferSource[] = { 0, 1, 2 };

struct DrawElementsIndirectCommand
{
	GLuint count;
	GLuint instanceCount;
	GLuint firstIndex;
	GLuint baseVertex;
	GLuint baseInstance;
};

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
	drawIndirectBuffer = 0;
	pIndexBuffer = 0;
	perInstanceBuffer = 0;
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
	afSafeDeleteBuffer(drawIndirectBuffer);
	afSafeDeleteBuffer(perInstanceBuffer);
	afSafeDeleteVertexArray(vao);
}

static const InputElement elements[] = {
	CInputElement(0, "POSITION", SF_R32G32B32_FLOAT, 0),
	CInputElement(0, "NORMAL", SF_R32G32B32_FLOAT, 12),
	CInputElement(1, "vBlendWeights", SF_R32G32B32_FLOAT, 0),
	CInputElement(1, "vBlendIndices", SF_R8G8B8A8_UINT, 12),
	CInputElement(2, "vColor", SF_R8G8B8A8_UNORM, 0),
	CInputElement(2, "vTexcoord", SF_R32G32_FLOAT, 4),
	CInputElement(3, "drawId", SF_R16_UINT, 0, true),
};

void MeshRenderer::Init(const Block& block)
{
	block.Verify();

	if (block.vertices.empty() || block.indices.empty() || block.color.empty()) {
		return;
	}

	int numVertices = block.vertices.size();
	const MeshVertex* vertices = &block.vertices[0];
	const MeshColor* color = &block.color[0];
	const MeshSkin* skin = &block.skin[0];
	int numIndices = block.indices.size();
	const AFIndex* indices = &block.indices[0];

	Destroy();

	shaderId = shaderMan.Create("skin.400");
	assert(shaderId);
	posBuffer = afCreateVertexBuffer(numVertices * sizeof(MeshVertex), vertices);
	skinBuffer = afCreateVertexBuffer(numVertices * sizeof(MeshSkin), skin);
	colorBuffer = afCreateVertexBuffer(numVertices * sizeof(MeshColor), color);
	perInstanceBuffer = afCreateVertexBuffer(sizeof(perInstanceBufferSource), perInstanceBufferSource);
	pIndexBuffer = afCreateIndexBuffer(indices, numIndices);

	std::vector<DrawElementsIndirectCommand> cmds;
	for (int j = 0; (unsigned)j < block.materialMaps.size(); j++) {
		const MaterialMap& matMap = block.materialMaps[j];
		const Material* mat = matMan.Get(matMap.materialId);
		assert(mat);

		int count = matMap.faces * 3;
		int start = matMap.faceStartIndex * 3;
		DrawElementsIndirectCommand cmd = { count, 1, start, 0, 0 };
		cmds.push_back(cmd);
		DrawElementsIndirectCommand cmd2 = { count, 1, start, 0, 1 };
		cmds.push_back(cmd2);
		DrawElementsIndirectCommand cmd3 = { count, 1, start, 0, 2 };
		cmds.push_back(cmd3);
	}
	assert(cmds.size());
	glGenBuffers(1, &drawIndirectBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, cmds.size() * sizeof(DrawElementsIndirectCommand), &cmds[0], GL_STATIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint verts[] = { posBuffer, skinBuffer, colorBuffer, perInstanceBuffer };
	GLsizei strides[] = { sizeof(MeshVertex), sizeof(MeshSkin), sizeof(MeshColor), sizeof(perInstanceBufferSource[0]) };
	afSetVertexAttributes(shaderId, elements, dimof(elements), block.indices.size(), verts, strides);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexBuffer);
	glBindVertexArray(0);

}

void MeshRenderer::Draw(const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block) const
{
	shaderMan.Apply(shaderId);

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

	assert(block.materialMaps.size() > 0);
	const MaterialMap& matMap = block.materialMaps[0];
	const Material* mat = matMan.Get(matMap.materialId);
	glBindTexture(GL_TEXTURE_2D, mat->tmid);
	glBindVertexArray(vao);
	glMultiDrawElementsIndirect(GL_TRIANGLES, AFIndexTypeToDevice, nullptr, block.materialMaps.size() * dimof(perInstanceBufferSource), 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
