#include "stdafx.h"

MeshRenderer meshRenderer;

static const uint16_t perInstanceBufferSource[] = { 0, 1, 2 };

struct DrawElementsIndirectCommand
{
	GLuint count;
	GLuint instanceCount;
	GLuint firstIndex;
	GLuint baseVertex;
	GLuint baseInstance;
};

RenderMesh::RenderMesh()
{
	vbo = 0;
	drawIndirectBuffer = 0;
	pIndexBuffer = 0;
	perInstanceBuffer = 0;
}

RenderMesh::~RenderMesh()
{
	Destroy();
}

void RenderMesh::Destroy()
{
	afSafeDeleteBuffer(pIndexBuffer);
	afSafeDeleteBuffer(vbo);
	afSafeDeleteBuffer(drawIndirectBuffer);
	afSafeDeleteBuffer(perInstanceBuffer);
	afSafeDeleteVertexArray(vao);
}

void RenderMesh::Init(const Block& block)
{
	if (block.vertices.empty() || block.indices.empty()) {
		return;
	}

	int numVertices = block.vertices.size();
	int numIndices = block.indices.size();
	const AFIndex* indices = &block.indices[0];

	Destroy();

	shaderId = shaderMan.Create("skin.400");
	assert(shaderId);

	int sizeVertex = numVertices * sizeof(MeshVertex);

	static const InputElement elements[] = {
		CInputElement(0, "POSITION", SF_R32G32B32_FLOAT, 0),
		CInputElement(0, "NORMAL", SF_R32G32B32_FLOAT, 12),
		CInputElement(0, "vColor", SF_R8G8B8A8_UNORM, 24),
		CInputElement(0, "vTexcoord", SF_R32G32_FLOAT, 28),
		CInputElement(0, "vBlendWeights", SF_R32G32B32_FLOAT, 36),
		CInputElement(0, "vBlendIndices", SF_R8G8B8A8_UINT, 48),
		CInputElement(1, "drawId", SF_R16_UINT, 0, true),
	};

	vbo = afCreateVertexBuffer(sizeVertex, &block.vertices[0]);
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
	GLuint verts[] = { vbo, perInstanceBuffer };
	GLsizei strides[] = { sizeof(MeshVertex), sizeof(perInstanceBufferSource[0]) };
	afSetVertexAttributes(shaderId, elements, dimof(elements), block.indices.size(), verts, strides);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIndexBuffer);
	glBindVertexArray(0);

}

void RenderMesh::Draw(const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block) const
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
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
	glBindVertexArray(vao);
	glMultiDrawElementsIndirect(GL_TRIANGLES, AFIndexTypeToDevice, nullptr, block.materialMaps.size() * dimof(perInstanceBufferSource), 0);
	glBindVertexArray(0);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

MeshRenderer::MeshRenderer()
{
}

MeshRenderer::~MeshRenderer()
{
	assert(renderMeshes.empty());
}

void MeshRenderer::Create()
{
	renderMeshes.push_back(nullptr);	// render mesh ID must not be 0
}

void MeshRenderer::Destroy()
{
	for (int i = 0; i < (int)renderMeshes.size(); i++) {
		RenderMesh* m = renderMeshes[i];
		delete m;
	}
	renderMeshes.clear();
}

MeshRenderer::MRID MeshRenderer::CreateRenderMesh(const Block& block)
{
	RenderMesh* r = new RenderMesh;
	r->Init(block);
	renderMeshes.push_back(r);
	return renderMeshes.size() - 1;
}

void MeshRenderer::SafeDestroyRenderMesh(MRID& id)
{
	if (id >= renderMeshes.size()) {
		return;
	}
	if (id != INVALID_MRID) {
		SAFE_DELETE(renderMeshes[id]);
		id = INVALID_MRID;
	}
}

RenderMesh* MeshRenderer::GetMeshByMRID(MRID id)
{
	if (id >= renderMeshes.size()) {
		return nullptr;
	}
	RenderMesh* r = renderMeshes[id];
	if (!r) {
		return nullptr;
	}
	return r;
}

void MeshRenderer::DrawRenderMesh(MRID id, const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block) const
{
	assert(GetMeshByMRID(id));
	RenderCommand c;
	c.id = id;
	c.boneStartIndex = renderBoneMatrices.size();
	c.nBones = nBones;
	renderCommands.push_back(c);

	renderBoneMatrices.resize(c.boneStartIndex + nBones);
	std::copy_n(BoneMatrices, nBones, renderBoneMatrices.begin() + c.boneStartIndex);
}

void MeshRenderer::Flush()
{
	for (int i = 0; i < renderCommands.size(); i++) {
		RenderCommand c = renderCommands[i];
		RenderMesh* r = GetMeshByMRID(c.id);
		assert(r);
		if (!r) {
			continue;
		}
		r->Draw(&renderBoneMatrices[c.boneStartIndex], c.nBones, block);
	}
}