#include "stdafx.h"

MeshRenderer meshRenderer;

static const uint16_t perInstanceBufferSource[] = { 0, 1, 2 };

static const int BONE_SSBO_SIZE = sizeof(Mat) * 1000;
static const int PER_INSTANCE_SSBO_SIZE = sizeof(RenderCommand) * 100;

enum SSBOBindingPoints {
	SBP_BONES = 5,
	SBP_PER_INSTANCE_DATAS = 7,
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
	afSafeDeleteVAO(vao);
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

		if (j == 0) {
			indirectCommand = cmd;
		}
	}
	assert(cmds.size());
	glGenBuffers(1, &drawIndirectBuffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, cmds.size() * sizeof(DrawElementsIndirectCommand), &cmds[0], GL_STATIC_DRAW);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	GLuint verts[] = { vbo, perInstanceBuffer };
	GLsizei strides[] = { sizeof(MeshVertex), sizeof(perInstanceBufferSource[0]) };
	int shaderId = meshRenderer.GetShaderId();
	vao = afCreateVAO(shaderId, elements, dimof(elements), block.indices.size(), verts, strides, pIndexBuffer);
}

void RenderMesh::Draw(const RenderCommand& c, int instanceCount) const
{
	int shaderId = meshRenderer.GetShaderId();
	glUniform1i(glGetUniformLocation(shaderId, "boneStartIndex"), c.boneStartIndex);


	const Material* mat = matMan.Get(c.materialId);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, drawIndirectBuffer);
	glBindVertexArray(vao);
	glMultiDrawElementsIndirect(GL_TRIANGLES, AFIndexTypeToDevice, nullptr, dimof(perInstanceBufferSource), 0);
	glBindVertexArray(0);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);


//	DrawElementsIndirectCommand cmd = indirectCommand;
//	cmd.instanceCount = instanceCount;
//	glDrawElementsIndirect(GL_TRIANGLES, AFIndexTypeToDevice, &cmd);

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
	glGenBuffers(1, &ssboForBoneMatrices);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboForBoneMatrices);
	glBufferData(GL_SHADER_STORAGE_BUFFER, BONE_SSBO_SIZE, nullptr, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &ssboForPerInstanceData);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboForPerInstanceData);
	glBufferData(GL_SHADER_STORAGE_BUFFER, PER_INSTANCE_SSBO_SIZE, nullptr, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	shaderId = shaderMan.Create("skin.400");
	assert(shaderId);

	glUniform1i(glGetUniformLocation(shaderId, "sampler"), 0);

	glShaderStorageBlockBinding(shaderId, glGetProgramResourceIndex(shaderId, GL_SHADER_STORAGE_BLOCK, "boneSSBO"), SBP_BONES);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SBP_BONES, ssboForBoneMatrices);

	glShaderStorageBlockBinding(shaderId, glGetProgramResourceIndex(shaderId, GL_SHADER_STORAGE_BLOCK, "perInstanceSSBO"), SBP_PER_INSTANCE_DATAS);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SBP_PER_INSTANCE_DATAS, ssboForPerInstanceData);
}

void MeshRenderer::Destroy()
{
	for (int i = 0; i < (int)renderMeshes.size(); i++) {
		RenderMesh* m = renderMeshes[i];
		delete m;
	}
	renderMeshes.clear();
	afSafeDeleteBuffer(ssboForBoneMatrices);
	afSafeDeleteBuffer(ssboForPerInstanceData);
}

MRID MeshRenderer::CreateRenderMesh(const Block& block)
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

void MeshRenderer::DrawRenderMesh(MRID id, const Mat& worldMat, const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block)
{
	assert(GetMeshByMRID(id));
	assert(!block.materialMaps.empty());
	RenderCommand c;
	c.matWorld = worldMat;
	c.meshId = id;
	c.materialId = block.materialMaps[0].materialId;
	c.boneStartIndex = renderBoneMatrices.size();
	c.nBones = nBones;
	renderCommands.push_back(c);

	renderBoneMatrices.resize(c.boneStartIndex + nBones);
	memcpy(&renderBoneMatrices[0] + c.boneStartIndex, &BoneMatrices[0], sizeof(Mat) * nBones);
}

void MeshRenderer::Flush()
{
	if (renderCommands.empty()) {
		return;
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboForBoneMatrices);
	GLvoid* buf = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	memcpy(buf, &renderBoneMatrices[0], sizeof(Mat) * renderBoneMatrices.size());
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboForPerInstanceData);
	buf = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	memcpy(buf, &renderCommands[0], sizeof(renderCommands[0]) * renderCommands.size());
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	shaderMan.Apply(shaderId);

	Mat matW, matV, matP;
	matrixMan.Get(MatrixMan::WORLD, matW);
	matrixMan.Get(MatrixMan::VIEW, matV);
	matrixMan.Get(MatrixMan::PROJ, matP);
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "matW"), 1, GL_FALSE, &matW.m[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "matV"), 1, GL_FALSE, &matV.m[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "matP"), 1, GL_FALSE, &matP.m[0][0]);

	glActiveTexture(GL_TEXTURE0);

#if 0
	RenderCommand c = renderCommands[0];
	const Material* mat = matMan.Get(c.materialId);
	RenderMesh* r = GetMeshByMRID(c.meshId);
	assert(r);
	glBindTexture(GL_TEXTURE_2D, mat->tmid);
	r->Draw(c, renderCommands.size());
#endif
#if 1

	TexMan::TMID lastTex = ~0;



	for (int i = 0; i < (int)renderCommands.size(); i++) {
		RenderCommand c = renderCommands[i];
		const Material* mat = matMan.Get(c.materialId);


		if (lastTex != mat->tmid) {
			glBindTexture(GL_TEXTURE_2D, mat->tmid);
		}

		RenderMesh* r = GetMeshByMRID(c.meshId);
		assert(r);
		if (!r) {
			continue;
		}
		r->Draw(c, 1);
	}

#endif

	glBindTexture(GL_TEXTURE_2D, 0);
	renderBoneMatrices.clear();
	renderCommands.clear();
}
