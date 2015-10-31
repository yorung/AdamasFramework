#include "stdafx.h"

MeshRenderer meshRenderer;

static int MAX_INSTANCES = 10;

static const int BONE_SSBO_SIZE = sizeof(Mat) * 1000;
static const int PER_INSTANCE_UBO_SIZE = sizeof(RenderCommand) * MAX_INSTANCES;

enum SSBOBindingPoints {
	SBP_BONES = 5,
};

enum UBOBindingPoints {
	UBP_PER_INSTANCE_DATAS = 2,
};

enum SamplerBindingPoints {
	SBP_DIFFUSE = 4,
};

RenderMesh::RenderMesh()
{
	vbo = 0;
	ibo = 0;
}

RenderMesh::~RenderMesh()
{
	Destroy();
}

void RenderMesh::Destroy()
{
	afSafeDeleteBuffer(ibo);
	afSafeDeleteBuffer(vbo);
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
	};

	vbo = afCreateVertexBuffer(sizeVertex, &block.vertices[0]);
	ibo = afCreateIndexBuffer(indices, numIndices);

	materialMaps = block.materialMaps;
	VBOID verts[] = { vbo };
	GLsizei strides[] = { sizeof(MeshVertex) };
	int shaderId = meshRenderer.GetShaderId();
	vao = afCreateVAO(shaderId, elements, dimof(elements), block.indices.size(), verts, strides, ibo);
}

void RenderMesh::Draw(const RenderCommand& c, int instanceCount) const
{
	int shaderId = meshRenderer.GetShaderId();

	afEnableBackFaceCulling(true);
	glActiveTexture(GL_TEXTURE0 + SBP_DIFFUSE);
	glBindVertexArray(vao);
	for (auto it : materialMaps) {
		const Material* mat = matMan.Get(it.materialId);
		assert(mat);
		glBindTexture(GL_TEXTURE_2D, mat->tmid);

		GLuint count = it.faces * 3;
		GLuint start = it.faceStartIndex * 3 * sizeof(AFIndex);
		DrawElementsIndirectCommand cmd = { count, (GLuint)instanceCount, start, 0, 0 };
		glDrawElementsInstanced/*BaseVertex*/(GL_TRIANGLES,
			cmd.count,
			AFIndexTypeToDevice,
			(void*)cmd.firstIndex,
			cmd.instanceCount/*, cmd.baseVertex*/);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

MeshRenderer::MeshRenderer()
{
	renderMeshes.push_back(nullptr);	// render mesh ID must not be 0
}

MeshRenderer::~MeshRenderer()
{
	assert(renderMeshes.empty());
}

void MeshRenderer::Create()
{
	ssboForBoneMatrices = afCreateSSBO(BONE_SSBO_SIZE);
	uboForPerInstanceData = afCreateUBO(PER_INSTANCE_UBO_SIZE);

	shaderId = shaderMan.Create("skin.400");
	assert(shaderId);

	shaderMan.Apply(shaderId);

	afBindBufferToBindingPoint(ssboForBoneMatrices, SBP_BONES);
	afBindBufferToBindingPoint(uboForPerInstanceData, UBP_PER_INSTANCE_DATAS);
}

void MeshRenderer::Destroy()
{
	for (int i = 0; i < (int)renderMeshes.size(); i++) {
		RenderMesh* m = renderMeshes[i];
		delete m;
	}
	renderMeshes.clear();
	afSafeDeleteBuffer(ssboForBoneMatrices);
	afSafeDeleteBuffer(uboForPerInstanceData);
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

void MeshRenderer::DrawRenderMesh(MRID id, const Mat& worldMat, const Mat BoneMatrices[], int nBones, const Block& block)
{
	assert(GetMeshByMRID(id));
	assert(!block.materialMaps.empty());

	if (!renderCommands.empty() && id != renderCommands[0].meshId) {
		Flush();
	}
	if (renderCommands.size() == MAX_INSTANCES) {
		Flush();
	}

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

	afWriteBuffer(ssboForBoneMatrices, &renderBoneMatrices[0], sizeof(Mat) * renderBoneMatrices.size());
	afWriteBuffer(uboForPerInstanceData, &renderCommands[0], sizeof(renderCommands[0]) * renderCommands.size());

	shaderMan.Apply(shaderId);

	Mat matV, matP;
	matrixMan.Get(MatrixMan::VIEW, matV);
	matrixMan.Get(MatrixMan::PROJ, matP);
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "matV"), 1, GL_FALSE, &matV.m[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderId, "matP"), 1, GL_FALSE, &matP.m[0][0]);

	RenderCommand c = renderCommands[0];
	RenderMesh* r = GetMeshByMRID(c.meshId);
	assert(r);
	r->Draw(c, renderCommands.size());
	renderBoneMatrices.clear();
	renderCommands.clear();
}
