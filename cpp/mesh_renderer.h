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
	VBOID vbo;
	IBOID ibo;
	std::vector<MaterialMap> materialMaps;
	~RenderMesh();
	void Destroy();
	void Init(const Block& block);
};

static const size_t MAX_INSTANCES = 10;
struct PerDrawCallUBO {
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
#ifdef AF_GLES31
	UBOID uboBones;
	UBOID uboPerDrawCall;
#endif
public:
	~MeshRenderer();
	void Create();
	void Destroy();
	MRID CreateRenderMesh(const Block& block);
	void SafeDestroyRenderMesh(MRID& id);
	void DrawRenderMesh(MRID id, const Mat& worldMat, const Mat BoneMatrices[], int nBones, const Block& block);
	void Flush();
	MMID CreateMaterial(const Material& mat);
	const Material* GetMaterial(MMID id);
};

extern MeshRenderer meshRenderer;
