struct Block;
struct MeshVertex;
struct MeshColor;
struct MeshSkin;

class RenderMesh
{
	GLuint vao;
	GLuint vbo;
	GLuint perInstanceBuffer;
	GLuint pIndexBuffer;
	GLuint drawIndirectBuffer;
	ShaderMan::SMID shaderId;
public:
	RenderMesh();
	~RenderMesh();
	void Destroy();
	void Init(const Block& block);
	void Draw(const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block) const;
};

struct RenderCommand
{
	MeshRenderer::MRID id;
	int boneStartIndex;
	int nBones;
};

class MeshRenderer
{
	std::vector<RenderMesh*> renderMeshes;
	std::vector<RenderCommand> renderCommands;
	std::vector<Mat> renderBoneMatrices;
	RenderMesh* GetMeshByMRID(MRID id);
public:
	typedef unsigned int MRID;
	static const MRID INVALID_MRID = 0;
	MeshRenderer();
	~MeshRenderer();
	void Create();
	void Destroy();
	MRID CreateRenderMesh(const Block& block);
	void SafeDestroyRenderMesh(MRID& id);
	void DrawRenderMesh(MRID id, const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block) const;
	void Flush();
};

extern MeshRenderer meshRenderer;
