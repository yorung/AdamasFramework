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
	void Draw(const Mat BoneMatrices[BONE_MAX], int nBones, MatMan::MMID materialId) const;
};

class MeshRenderer
{
public:
	typedef unsigned int MRID;
	static const MRID INVALID_MRID = 0;
private:
	struct RenderCommand
	{
		MeshRenderer::MRID meshId;
		MatMan::MMID materialId;
		int boneStartIndex;
		int nBones;
	};
	std::vector<RenderMesh*> renderMeshes;
	std::vector<RenderCommand> renderCommands;
	std::vector<Mat> renderBoneMatrices;
	GLuint ssboForBoneMatrices;
	RenderMesh* GetMeshByMRID(MRID id);
public:
	MeshRenderer();
	~MeshRenderer();
	void Create();
	void Destroy();
	MRID CreateRenderMesh(const Block& block);
	void SafeDestroyRenderMesh(MRID& id);
	void DrawRenderMesh(MRID id, const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block);
	void Flush();
};

extern MeshRenderer meshRenderer;
