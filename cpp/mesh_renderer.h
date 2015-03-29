struct Block;
struct MeshVertex;
struct MeshColor;
struct MeshSkin;

typedef unsigned int MRID;
static const MRID INVALID_MRID = 0;

struct RenderCommand
{
	Mat matWorld;
	MRID meshId;
	MMID materialId;
	int boneStartIndex;
	int nBones;
};

class RenderMesh
{
	GLuint vao;
	GLuint vbo;
	GLuint perInstanceBuffer;
	GLuint pIndexBuffer;
	GLuint drawIndirectBuffer;
public:
	RenderMesh();
	~RenderMesh();
	void Destroy();
	void Init(const Block& block);
	void Draw(const RenderCommand& c) const;
};

class MeshRenderer
{
public:
	ShaderMan::SMID shaderId;
	std::vector<RenderMesh*> renderMeshes;
	std::vector<RenderCommand> renderCommands;
	std::vector<Mat> renderBoneMatrices;
	GLuint ssboForBoneMatrices;
	GLuint ssboForPerInstanceData;
	RenderMesh* GetMeshByMRID(MRID id);
public:
	MeshRenderer();
	~MeshRenderer();
	void Create();
	void Destroy();
	ShaderMan::SMID GetShaderId() { return shaderId; }
	MRID CreateRenderMesh(const Block& block);
	void SafeDestroyRenderMesh(MRID& id);
	void DrawRenderMesh(MRID id, const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block);
	void Flush();
};

extern MeshRenderer meshRenderer;
