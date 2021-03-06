#include "stdafx.h"
#include "mesh_renderer.h"
#include "mesh_x.h"
#include "tex_man.h"

MeshRenderer meshRenderer;

static const InputElement elements[] =
{
	AF_INPUT_ELEMENT(0, "POSITION", AFF_R32G32B32_FLOAT, 0),
	AF_INPUT_ELEMENT(1, "NORMAL", AFF_R32G32B32_FLOAT, 12),
	AF_INPUT_ELEMENT(2, "vColor", AFF_R8G8B8A8_UNORM, 24),
	AF_INPUT_ELEMENT(3, "vTexcoord", AFF_R32G32_FLOAT, 28),
	AF_INPUT_ELEMENT(4, "vBlendWeights", AFF_R32G32B32_FLOAT, 36),
	AF_INPUT_ELEMENT(5, "vBlendIndices", AFF_R8G8B8A8_UINT, 48),
};

static const SamplerType samplers[] =
{
	AFST_MIPMAP_WRAP,
	AFST_MIPMAP_WRAP,
	AFST_MIPMAP_WRAP,
	AFST_MIPMAP_WRAP,	// s3 register
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

	int numVertices = (int)block.vertices.size();
	int numIndices = (int)block.indices.size();
	const AFIndex* indices = &block.indices[0];

	Destroy();

	int sizeVertex = numVertices * sizeof(MeshVertex);
	vbo = afCreateVertexBuffer(sizeVertex, &block.vertices[0]);
	ibo = afCreateIndexBuffer(numIndices, indices);

	materialMaps = block.materialMaps;
}

MeshRenderer::~MeshRenderer()
{
	assert(renderMeshes.size() == 1);
}

void MeshRenderer::Create()
{
	Destroy();

	renderStates.Create("skin_instanced", arrayparam(elements), AFRS_DEPTH_ENABLE | AFRS_CULL_CW | AFRS_PRIMITIVE_TRIANGLELIST | AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM | AFRS_AUTO_DEPTH_STENCIL, arrayparam(samplers));
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
	renderStates.Destroy();

	nStoredCommands = 0;
}

MRID MeshRenderer::CreateRenderMesh(const Block& block)
{
	RenderMesh* r = new RenderMesh;
	r->Init(block);
	renderMeshes.push_back(r);
	return (int)renderMeshes.size() - 1;
}

void MeshRenderer::SafeDestroyRenderMesh(MRID& id)
{
	if (id >= renderMeshes.size())
	{
		return;
	}
	if (id != INVALID_MRID)
	{
		afSafeDelete(renderMeshes[id]);
		id = INVALID_MRID;
	}
}

RenderMesh* MeshRenderer::GetMeshByMRID(MRID id)
{
	if (id >= renderMeshes.size())
	{
		return nullptr;
	}
	RenderMesh* r = renderMeshes[id];
	if (!r)
	{
		return nullptr;
	}
	return r;
}

void MeshRenderer::DrawRenderMesh(const ViewDesc& view, MRID id, const Mat& worldMat, const Mat BoneMatrices[], int nBones)
{
	if (!renderStates.IsReady())
	{
		return;
	}
	assert(GetMeshByMRID(id));

	if (nStoredCommands && id != perDrawCallUBO.commands[0].meshId)
	{
		Flush(view);
	}
	if (nStoredCommands == MAX_INSTANCES)
	{
		Flush(view);
	}
	if (nBones + renderBoneMatrices.size() > MAX_BONES_PER_DRAW_CALL)
	{
		Flush(view);
	}

	RenderCommand& c = perDrawCallUBO.commands[nStoredCommands++];
	c.matWorld = worldMat;
	c.meshId = id;
	c.boneStartIndex = (uint32_t)renderBoneMatrices.size();
	c.nBones = nBones;

	renderBoneMatrices.resize(c.boneStartIndex + nBones);
	memcpy(&renderBoneMatrices[0] + c.boneStartIndex, &BoneMatrices[0], sizeof(Mat) * nBones);
}

void MeshRenderer::Flush(const ViewDesc& view)
{
	if (!renderStates.IsReady()) {
		return;
	}
	if (!nStoredCommands) {
		return;
	}

	perDrawCallUBO.matV = view.matView;
	perDrawCallUBO.matP = view.matProj;

	AFCommandList& cmd = afGetCommandList();
	cmd.SetRenderStates(renderStates);

#ifdef AF_GLES
	struct PerDrawCallForES2
	{
		Mat v, p, w;
	}perDrawCallForES2;
	perDrawCallForES2.v = perDrawCallUBO.matV;
	perDrawCallForES2.p = perDrawCallUBO.matP;
	perDrawCallForES2.w = perDrawCallUBO.commands[0].matWorld;
	cmd.SetBuffer(sizeof(PerDrawCallUBO), &perDrawCallForES2, 0);
#else
	cmd.SetBuffer(sizeof(PerDrawCallUBO), &perDrawCallUBO, 0);
#endif
	cmd.SetBuffer(sizeof(Mat) * (int)renderBoneMatrices.size(), &renderBoneMatrices[0], 2);

	const RenderCommand& c = perDrawCallUBO.commands[0];
	RenderMesh* r = GetMeshByMRID(c.meshId);
	assert(r);

	cmd.SetIndexBuffer(r->ibo);
	cmd.SetVertexBuffer(r->vbo, sizeof(MeshVertex));
	for (auto it : r->materialMaps)
	{
		const Material* mat = meshRenderer.GetMaterial(it.materialId);
		assert(mat);
		cmd.SetTexture(texMan.IndexToTexture(mat->texture), 3);
		cmd.SetBuffer(sizeof(Material), mat, 1);
		int count = it.faces * 3;
		int start = it.faceStartIndex * 3;
		cmd.DrawIndexed(count, start, nStoredCommands);
	}

	renderBoneMatrices.clear();
	nStoredCommands = 0;
}

MMID MeshRenderer::CreateMaterial(const Material& mat)
{
	if (!renderStates.IsReady())
	{
		return 0;
	}
	auto it = std::find_if(materials.begin(), materials.end(), [&mat](const Material& m) { return m == mat; });
	if (it != materials.end())
	{
		int n = (int)std::distance(materials.begin(), it);
		return n;
	}
	materials.push_back(mat);
	return (int)materials.size() - 1;
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
