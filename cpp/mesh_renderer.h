struct Block;
struct MeshVertex;
struct MeshColor;
struct MeshSkin;

class MeshRenderer
{
	GLuint posBuffer;
	GLuint colorBuffer;
	GLuint skinBuffer;
	GLuint perInstanceBuffer;
	GLuint pIndexBuffer;
	GLuint drawIndirectBuffer;
	ShaderMan::SMID shaderId;
public:
	MeshRenderer();
	~MeshRenderer();
	void Destroy();
	void Init(const Block& block);
	void Draw(const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block) const;
};

typedef MeshRenderer MeshRenderer;
