struct Block;
struct MeshVertex;
struct MeshColor;
struct MeshSkin;

class MeshRenderer
{
	GLuint posBuffer;
	GLuint colorBuffer;
	GLuint skinBuffer;
	GLuint pIndexBuffer;
	ShaderMan::SMID shaderId;
public:
	MeshRenderer();
	~MeshRenderer();
	void Destroy();
	void Init(int numVertices, const MeshVertex* vertices, const MeshColor* color, const MeshSkin* skin, int numIndices, const AFIndex* indices);
	void Init(const Block& block);
	void Draw(const Mat BoneMatrices[BONE_MAX], int nBones, const Block& block) const;
};

typedef MeshRenderer MeshRenderer;
