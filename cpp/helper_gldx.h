typedef unsigned short AFIndex;

#ifdef GL_TRUE
#define AFIndexTypeToDevice GL_UNSIGNED_SHORT
typedef GLuint AFBufObj;
#endif

#ifdef __d3d11_h__
#define AFIndexTypeToDevice DXGI_FORMAT_R16_UINT
typedef ID3D11Buffer* AFBufObj;
#endif

#ifdef GL_TRUE
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
#endif

struct InputElement {
	int inputSlot;
	const char* name;
	ShaderFormat format;
	int offset;
	bool perInstance;
};

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

struct DrawElementsIndirectCommand
{
	GLuint count;
	GLuint instanceCount;
	GLuint firstIndex;
	GLuint baseVertex;
	GLuint baseInstance;
};

void afSetVertexAttributes(GLuint program, const InputElement elements[], int numElements, int numBuffers, GLuint const *vertexBufferIds, const GLsizei* strides);
GLuint afCreateVAO(GLuint program, const InputElement elements[], int numElements, int numBuffers, GLuint const *vertexBufferIds, const GLsizei* strides, GLuint ibo);

//#ifdef USE_FAKE_SAMPLER
#define glGenSamplers(a,b)
#define glSamplerParameteri(a,b,c)
#define glBindSampler(a,b)
#define glDeleteSamplers(a,b)
//#endif

#ifdef GL_TRUE
inline void afSafeDeleteBuffer(GLuint& b)
{
	if (b != 0) {
		glDeleteBuffers(1, &b);
		b = 0;
	}
}
inline void afSafeDeleteSampler(GLuint& s)
{
	if (s != 0) {
		glDeleteSamplers(1, &s);
		s = 0;
	}
}
inline void afSafeDeleteTexture(GLuint& t)
{
	if (t != 0) {
		glDeleteTextures(1, &t);
		t = 0;
	}
}
inline void afSafeDeleteVAO(GLuint& vao)
{
	if (vao != 0) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
}

enum AFDTFormat
{
	AFDT_R8G8B8A8_UINT,
	AFDT_R5G6B5_UINT,
};
GLuint afCreateDynamicTexture(int w, int h, AFDTFormat format);
#endif

#ifdef __d3d11_h__
inline void afSafeDeleteBuffer(AFBufObj& b)
{
	SAFE_RELEASE(b);
}
#endif

AFBufObj afCreateIndexBuffer(const AFIndex* indi, int numIndi);
AFBufObj afCreateQuadListIndexBuffer(int numQuads);
AFBufObj afCreateVertexBuffer(int size, const void* buf);
AFBufObj afCreateDynamicVertexBuffer(int size);
AFBufObj afCreateSSBO(int size);
void afWriteBuffer(AFBufObj bo, const void* buf, int size);
void afWriteSSBO(GLuint bufName, const void* buf, int size);
void afDrawIndexedTriangleList(AFBufObj ibo, int count, int start = 0);
