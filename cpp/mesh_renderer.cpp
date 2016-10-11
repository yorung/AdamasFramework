#include "stdafx.h"

MeshRenderer meshRenderer;

static const size_t MAX_MATERIALS = 100;
static const size_t MAX_BONE_SSBOS = 100;
static const size_t MATERIAL_UBO_SIZE = sizeof(Material) * MAX_MATERIALS;

#ifdef AF_VULKAN
static const InputElement elements[] =
{
	CInputElement(0, AFF_R32G32B32_FLOAT, 0),
	CInputElement(1, AFF_R32G32B32_FLOAT, 12),
	CInputElement(2, AFF_R8G8B8A8_UNORM, 24),
	CInputElement(3, AFF_R32G32_FLOAT, 28),
	CInputElement(4, AFF_R32G32B32_FLOAT, 36),
	CInputElement(5, AFF_R8G8B8A8_UINT, 48),
	CInputElement(6, AFF_R32_UINT, 52),
};
#else
static const InputElement elements[] =
{
	CInputElement("POSITION", AFF_R32G32B32_FLOAT, 0),
	CInputElement("NORMAL", AFF_R32G32B32_FLOAT, 12),
	CInputElement("vColor", AFF_R8G8B8A8_UNORM, 24),
	CInputElement("vTexcoord", AFF_R32G32_FLOAT, 28),
	CInputElement("vBlendWeights", AFF_R32G32B32_FLOAT, 36),
	CInputElement("vBlendIndices", AFF_R8G8B8A8_UINT, 48),
	CInputElement("materialId", AFF_R32_UINT, 52),
};
#endif

static const SamplerType samplers[] = {
	AFST_MIPMAP_WRAP,
};

RenderMesh::~RenderMesh()
{
	Destroy();
}

void RenderMesh::Destroy()
{
	afSafeDeleteBuffer(ibo);
	afSafeDeleteBuffer(vbo);
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
	ibo = afCreateIndexBuffer(numIndices, indices);

	materialMaps = block.materialMaps;
	VBOID verts[] = { vbo };
	int strides[] = { sizeof(MeshVertex) };
}

MeshRenderer::~MeshRenderer()
{
	assert(renderMeshes.size() == 1);
}

void MeshRenderer::Create()
{
	Destroy();
	uboForMaterials = afCreateUBO(MATERIAL_UBO_SIZE);

#ifdef AF_VULKAN
	const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, deviceMan.descriptorPool, 1, &deviceMan.commonUboDescriptorSetLayout };
	afHandleVKError(vkAllocateDescriptorSets(deviceMan.GetDevice(), &descriptorSetAllocateInfo, &uboDescriptorSet));
#endif

	renderStates.Create("skin_instanced", arrayparam(elements), AFRS_DEPTH_ENABLE | AFRS_CULL_CW | AFRS_PRIMITIVE_TRIANGLELIST, arrayparam(samplers));
}

void MeshRenderer::Destroy()
{
#ifdef AF_VULKAN
	if (uboDescriptorSet)
	{
		afHandleVKError(vkFreeDescriptorSets(deviceMan.GetDevice(), deviceMan.descriptorPool, 1, &uboDescriptorSet));
		uboDescriptorSet = 0;
	}
#endif

	for (int i = 0; i < (int)renderMeshes.size(); i++) {
		RenderMesh* m = renderMeshes[i];
		delete m;
	}
	renderMeshes.clear();
	renderMeshes.push_back(nullptr);	// render mesh ID must not be 0
	materials.clear();
	materials.push_back(Material());	// make id 0 invalid
	afSafeDeleteBuffer(uboForMaterials);
	renderStates.Destroy();

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
	if (!uboForMaterials) {
		return;
	}
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
	if (!uboForMaterials) {
		return;
	}
	if (!nStoredCommands) {
		return;
	}

	matrixMan.Get(MatrixMan::VIEW, perDrawCallUBO.matV);
	matrixMan.Get(MatrixMan::PROJ, perDrawCallUBO.matP);

	renderStates.Apply();
	afBindBuffer(sizeof(PerDrawCallUBO), &perDrawCallUBO, 0);
#ifdef AF_VULKAN
	uint32_t dynamicOffset = 0;
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	const uint32_t descritorSetIndex = 1;
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderStates.GetPipelineLayout(), descritorSetIndex, 1, &deviceMan.commonUboDescriptorSet, 1, &dynamicOffset);
#else
	afBindBuffer(uboForMaterials, 1);
#endif
	afBindBuffer(sizeof(Mat) * renderBoneMatrices.size(), &renderBoneMatrices[0], 2);

	const RenderCommand& c = perDrawCallUBO.commands[0];
	RenderMesh* r = GetMeshByMRID(c.meshId);
	assert(r);

	afSetIndexBuffer(r->ibo);
	afSetVertexBuffer(r->vbo, sizeof(MeshVertex));
	for (auto it : r->materialMaps) {
		const Material* mat = meshRenderer.GetMaterial(it.materialId);
		assert(mat);
		afBindTexture(texMan.IndexToTexture(mat->texture), 3);
		int count = it.faces * 3;
		int start = it.faceStartIndex * 3;
		afDrawIndexed(count, start, nStoredCommands);
	}

	renderBoneMatrices.clear();
	nStoredCommands = 0;
}

MMID MeshRenderer::CreateMaterial(const Material& mat)
{
	if (!uboForMaterials) {
		return 0;
	}
	auto it = std::find_if(materials.begin(), materials.end(), [&mat](const Material& m) { return m == mat; });
	if (it != materials.end()) {
		int n = (int)std::distance(materials.begin(), it);
		return n;
	}
	materials.push_back(mat);

	assert(materials.size() <= MAX_MATERIALS);
	afWriteBuffer(uboForMaterials, sizeof(materials[0]) * materials.size(), &materials[0]);

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

const Material& Material::operator=(const Material& r)
{
	faceColor = r.faceColor;
	power = r.power;
	specular = r.specular;
	emissive = r.emissive;
	texture = r.texture;
	return *this;
}

bool Material::operator==(const Material& r) const
{
	return faceColor == r.faceColor && power == r.power && specular == r.specular && emissive == r.emissive && texture == r.texture;
}
