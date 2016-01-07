struct Block;
struct MeshVertex;
struct MeshColor;
struct MeshSkin;
struct MaterialMap;

typedef unsigned int MRID;
static const MRID INVALID_MRID = 0;

typedef int MMID;
//static int INVALID_MMID = 0;

struct Material
{
	Material() { memset(this, 0, sizeof(*this)); }
	Vec4 faceColor;
	Vec3 specular;
	float power;
	Vec3 emissive;
	TexMan::TMID tmid;
	bool operator==(const Material& r) const;
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
	GLuint vao;
	VBOID vbo;
	IBOID ibo;
	std::vector<MaterialMap> materialMaps;
public:
	RenderMesh();
	~RenderMesh();
	void Destroy();
	void Init(const Block& block);
	void Draw(int instanceCount) const;
};

static const size_t MAX_INSTANCES = 10;
struct PerDrawCallUBO {
	Mat matV, matP;
	RenderCommand commands[MAX_INSTANCES];
};

class MeshRenderer
{
public:
	ShaderMan::SMID shaderId;
	std::vector<RenderMesh*> renderMeshes;
	PerDrawCallUBO perDrawCallUBO;
	int nStoredCommands = 0;
	std::vector<Mat> renderBoneMatrices;
	std::vector<Material> materials;
	SSBOID ssboForBoneMatrices;
	SSBOID ssboForMaterials;
	UBOID uboForPerDrawCall;
	RenderMesh* GetMeshByMRID(MRID id);
public:
	MeshRenderer();
	~MeshRenderer();
	void Create();
	void Destroy();
	ShaderMan::SMID GetShaderId() { return shaderId; }
	MRID CreateRenderMesh(const Block& block);
	void SafeDestroyRenderMesh(MRID& id);
	void DrawRenderMesh(MRID id, const Mat& worldMat, const Mat BoneMatrices[], int nBones, const Block& block);
	void Flush();
	MMID CreateMaterial(const Material& mat);
	const Material* GetMaterial(MMID id);
};

extern MeshRenderer meshRenderer;
