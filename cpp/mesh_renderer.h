#pragma once

struct Block;
struct MeshVertex;
struct MeshColor;
struct MeshSkin;
struct MaterialMap;

typedef unsigned int MRID;
static const MRID INVALID_MRID = 0;

typedef int MMID;

class Material
{
public:
	Material() {}
	Material(const Material& r) { *this = r; }
	const Material& operator=(const Material& r);
	bool operator==(const Material& r) const;
	Vec4 faceColor;
	Vec3 specular;
	float power = 0;
	Vec3 emissive;
	int texture = 0;
};

struct RenderCommand
{
	Mat matWorld;
	MRID meshId;
	uint32_t boneStartIndex;
	int nBones;
	int padding;
};

class RenderMesh
{
public:
	AFBufferResource vbo;
	AFBufferResource ibo;
	std::vector<MaterialMap> materialMaps;
	~RenderMesh();
	void Destroy();
	void Init(const Block& block);
};

#ifdef AF_GLES
static const size_t MAX_INSTANCES = 1;				// Disable instancing for ES 2.0
static const size_t MAX_BONES_PER_DRAW_CALL = 50;	// This is limited by GL_MAX_VERTEX_UNIFORM_VECTORS. In case of Adreno GPU is 256.
#else
static const size_t MAX_INSTANCES = 10;
static const size_t MAX_BONES_PER_DRAW_CALL = 100;
#endif

struct PerDrawCallUBO
{
	Mat matV, matP;
	RenderCommand commands[MAX_INSTANCES];
};

class MeshRenderer
{
public:
	AFRenderStates renderStates;
	std::vector<RenderMesh*> renderMeshes;
	PerDrawCallUBO perDrawCallUBO;
	int nStoredCommands = 0;
	std::vector<Mat> renderBoneMatrices;
	std::vector<Material> materials;
	RenderMesh* GetMeshByMRID(MRID id);
public:
	~MeshRenderer();
	void Create();
	void Destroy();
	MRID CreateRenderMesh(const Block& block);
	void SafeDestroyRenderMesh(MRID& id);
	void DrawRenderMesh(const ViewDesc& view, MRID id, const Mat& worldMat, const Mat BoneMatrices[], int nBones);
	void Flush(const ViewDesc& view);
	MMID CreateMaterial(const Material& mat);
	const Material* GetMaterial(MMID id);
};

extern MeshRenderer meshRenderer;
