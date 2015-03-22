enum ShaderFormat {
	SF_R32_FLOAT,
	SF_R32G32_FLOAT,
	SF_R32G32B32_FLOAT,
	SF_R32G32B32A32_FLOAT,
	SF_R8_UNORM,
	SF_R8G8_UNORM,
	SF_R8G8B8_UNORM,
	SF_R8G8B8A8_UNORM,
	SF_R8_UINT,
	SF_R8G8_UINT,
	SF_R8G8B8_UINT,
	SF_R8G8B8A8_UINT,
	SF_R16_UINT,
	SF_R16G16_UINT,
	SF_R16G16B16_UINT,
	SF_R16G16B16A16_UINT,
};

struct InputElement {
	int inputSlot;
	const char* name;
	ShaderFormat format;
	int offset;
	bool perInstance;
};

void afSetVertexAttributes(GLuint program, const InputElement elements[], int numElements, int numBuffers, GLuint const *vertexBufferIds, const GLsizei* strides);

class CInputElement : public InputElement {
public:
	CInputElement(int inputSlot, const char* name, ShaderFormat format, int offset, bool perInstance = false) {
		this->name = name;
		this->format = format;
		this->inputSlot = inputSlot;
		this->offset = offset;
		this->perInstance = perInstance;
	}
};

class ShaderMan
{
public:
	typedef GLuint SMID;
	static const SMID INVALID_SMID = 0;
private:
	typedef std::map<std::string, SMID> NameToId;
	NameToId nameToId;
public:
	SMID Create(const char *name);
	void Destroy();
	void Reload();
	void Apply(SMID id);
};

extern ShaderMan shaderMan;

