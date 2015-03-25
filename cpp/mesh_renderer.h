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

class MeshRenderer
{
	std::vector<RenderMesh*> renderMeshes;
public:
	typedef unsigned int MRID;
	MeshRenderer();
	~MeshRenderer();
	void Create();
	void Destroy();
	MRID CreateRenderMesh(const Block& block);
	void DestroyRenderMesh(MRID id);
};

extern MeshRenderer meshRenderer;
