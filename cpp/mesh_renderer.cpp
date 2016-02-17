#include "stdafx.h"

MeshRenderer meshRenderer;

static const size_t MAX_MATERIALS = 100;
static const size_t MAX_BONE_SSBOS = 100;
static const size_t MATERIAL_SSBO_SIZE = sizeof(Material) * MAX_MATERIALS;

static const InputElement elements[] = {
	CInputElement("POSITION", SF_R32G32B32_FLOAT, 0),
	CInputElement("NORMAL", SF_R32G32B32_FLOAT, 12),
	CInputElement("vColor", SF_R8G8B8A8_UNORM, 24),
	CInputElement("vTexcoord", SF_R32G32_FLOAT, 28),
	CInputElement("vBlendWeights", SF_R32G32B32_FLOAT, 36),
	CInputElement("vBlendIndices", SF_R8G8B8A8_UINT, 48),
	CInputElement("materialId", SF_R32_UINT, 52),
};

enum SSBOBindingPoints {
	SBP_MATERIALS = 0,
	SBP_BONES = 1,
};

enum UBOBindingPoints {
	UBP_PER_INSTANCE_DATAS = 1,
};

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
	vbo = afCreateVertexBuffer(sizeVertex, &block.vertices[0]);
	ibo = afCreateIndexBuffer(indices, numIndices);

	materialMaps = block.materialMaps;
	VBOID verts[] = { vbo };
	GLsizei strides[] = { sizeof(MeshVertex) };
	int shaderId = meshRenderer.GetShaderId();
	vao = afCreateVAO(elements, dimof(elements), block.indices.size(), verts, strides, ibo);
	aflog("RenderMesh::Init created vao=%d\n", vao.x);
	assert(vao);
}

void RenderMesh::Draw(int instanceCount) const
{
	assert(vao);
	afBindVAO(vao);
	for (auto it : materialMaps) {
		const Material* mat = meshRenderer.GetMaterial(it.materialId);
		assert(mat);
		afBindTextureToBindingPoint(mat->texture, 0);
		GLuint count = it.faces * 3;
		GLuint start = it.faceStartIndex * 3 * sizeof(AFIndex);
		afDrawIndexedInstancedTriangleList(instanceCount, count, start);
	}
	afBindTextureToBindingPoint(0, 0);
	afBindVAO(0);
}

MeshRenderer::MeshRenderer()
{
}

MeshRenderer::~MeshRenderer()
{
	assert(renderMeshes.size() == 1);
}

void MeshRenderer::Create()
{
	Destroy();

	ssboForBoneMatrices = afCreateSSBO(sizeof(Mat) * MAX_BONE_SSBOS);
	uboForPerDrawCall = afCreateUBO(sizeof(PerDrawCallUBO));
	ssboForMaterials = afCreateSSBO(MATERIAL_SSBO_SIZE);
	shaderId = shaderMan.Create("skin.400", elements, dimof(elements), BM_NONE, DSM_DEPTH_ENABLE, CM_CW);
	assert(shaderId);

	shaderMan.Apply(shaderId);

	afBindBufferToBindingPoint(ssboForBoneMatrices, SBP_BONES);
	afBindBufferToBindingPoint(ssboForMaterials, SBP_MATERIALS);
	afBindBufferToBindingPoint(uboForPerDrawCall, UBP_PER_INSTANCE_DATAS);
}

void MeshRenderer::Destroy()
{
	for (int i = 0; i < (int)renderMeshes.size(); i++) {
		RenderMesh* m = renderMeshes[i];
		delete m;
	}
	renderMeshes.clear();
	renderMeshes.push_back(nullptr);	// render mesh ID must not be 0
	materials.clear();
	materials.push_back(Material());	// make id 0 invalid

	afSafeDeleteBuffer(ssboForBoneMatrices);
	afSafeDeleteBuffer(ssboForMaterials);
	afSafeDeleteBuffer(uboForPerDrawCall);

	nStoredCommands = 0;
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

	if (nStoredCommands && id != perDrawCallUBO.commands[0].meshId) {
		Flush();
	}
	if (nStoredCommands == MAX_INSTANCES) {
		Flush();
	}
	if (nBones + renderBoneMatrices.size() > MAX_BONE_SSBOS) {
		Flush();
	}

	RenderCommand& c = perDrawCallUBO.commands[nStoredCommands++];
	c.matWorld = worldMat;
	c.meshId = id;
	c.boneStartIndex = renderBoneMatrices.size();
	c.nBones = nBones;

	renderBoneMatrices.resize(c.boneStartIndex + nBones);
	memcpy(&renderBoneMatrices[0] + c.boneStartIndex, &BoneMatrices[0], sizeof(Mat) * nBones);
}

void MeshRenderer::Flush()
{
	if (!nStoredCommands) {
		return;
	}
	afWriteBuffer(ssboForBoneMatrices, &renderBoneMatrices[0], sizeof(Mat) * renderBoneMatrices.size());
	matrixMan.Get(MatrixMan::VIEW, perDrawCallUBO.matV);
	matrixMan.Get(MatrixMan::PROJ, perDrawCallUBO.matP);
	afWriteBuffer(uboForPerDrawCall, &perDrawCallUBO, sizeof(PerDrawCallUBO));

	shaderMan.Apply(shaderId);

//	aflog("ubo pos = %d %d\n", glGetUniformLocation(shaderId, "matV"), glGetUniformLocation(shaderId, "matP"));

	const RenderCommand& c = perDrawCallUBO.commands[0];
	RenderMesh* r = GetMeshByMRID(c.meshId);
	assert(r);
	r->Draw(nStoredCommands);
	renderBoneMatrices.clear();
	nStoredCommands = 0;
}

MMID MeshRenderer::CreateMaterial(const Material& mat)
{
	auto it = std::find_if(materials.begin(), materials.end(), [&mat](const Material& m) { return m == mat; });
	if (it != materials.end()) {
		int n = (int)std::distance(materials.begin(), it);
		return n;
	}
	materials.push_back(mat);

	assert(materials.size() <= MAX_MATERIALS);
	afWriteBuffer(ssboForMaterials, &materials[0], sizeof(materials[0]) * materials.size());

	return materials.size() - 1;
}


const Material* MeshRenderer::GetMaterial(MMID id)
{
	if (id >= 0 && id < (MMID)materials.size())
	{
		return &materials[id];
	}
	return nullptr;
}

bool Material::operator==(const Material& r) const
{
	return !memcmp(this, &r, sizeof(Material));
}
