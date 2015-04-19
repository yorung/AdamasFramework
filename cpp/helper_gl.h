typedef unsigned short AFIndex;

#define AFIndexTypeToDevice GL_UNSIGNED_SHORT

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

template <GLenum bufType_>
struct TBufName {
	static const GLenum bufType = bufType_;
	GLuint x;
//	TBufName(GLuint r = 0) { x = r; }
	TBufName operator=(GLuint r) { x = r; return *this; }
	operator GLuint() const { return x; }
};
typedef TBufName<GL_UNIFORM_BUFFER> UBOID;
typedef TBufName<GL_ELEMENT_ARRAY_BUFFER> IBOID;
typedef TBufName<GL_ARRAY_BUFFER> VBOID;
typedef TBufName<GL_SHADER_STORAGE_BUFFER> SSBOID;
typedef GLuint VAOID;
typedef GLuint SAMPLERID;

void afSetVertexAttributes(GLuint program, const InputElement elements[], int numElements, int numBuffers, VBOID const *vertexBufferIds, const GLsizei* strides);
VAOID afCreateVAO(GLuint program, const InputElement elements[], int numElements, int numBuffers, VBOID const *vertexBufferIds, const GLsizei* strides, IBOID ibo);

#ifdef USE_FAKE_SAMPLER
#define glGenSamplers(a,b)
#define glSamplerParameteri(a,b,c)
#define glBindSampler(a,b)
#define glDeleteSamplers(a,b)
#endif

template <class BufName>
void afWriteBuffer(BufName bufName, const void* buf, int size)
{
	glBindBuffer(BufName::bufType, bufName);
	glBufferSubData(BufName::bufType, 0, size, buf);
	glBindBuffer(BufName::bufType, 0);
}

template <class BufName>
inline void afSafeDeleteBuffer(BufName& b)
{
	if (b.x != 0) {
		glDeleteBuffers(1, &b.x);
		b.x = 0;
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
GLuint afCreateWhiteTexture();

IBOID afCreateIndexBuffer(const AFIndex* indi, int numIndi);
IBOID afCreateQuadListIndexBuffer(int numQuads);
VBOID afCreateVertexBuffer(int size, const void* buf);
VBOID afCreateDynamicVertexBuffer(int size);
SSBOID afCreateSSBO(int size);
UBOID afCreateUBO(int size);
SAMPLERID afCreateSampler();
#if 0 // without "binding" Layout Qualifier
void afLayoutSamplerBindingManually(GLuint program, const GLchar* name, GLuint samplerBinding);
void afLayoutSSBOBindingManually(GLuint program, const GLchar* name, GLuint storageBlockBinding);
void afLayoutUBOBindingManually(GLuint program, const GLchar* name, GLuint uniformBlockBinding);
#endif
void afBindBufferToBindingPoint(SSBOID ssbo, GLuint storageBlockBinding);
void afBindBufferToBindingPoint(UBOID ubo, GLuint uniformBlockBinding);
void afBindTextureToBindingPoint(GLuint tex, UINT textureBindingPoint);

void afDrawIndexedTriangleList(IBOID ibo, int count, int start = 0);
void afEnableBackFaceCulling(bool cullBack);

enum BlendMode {
	BM_NONE,
	BM_ALPHA,
};
void afBlendMode(BlendMode mode);
void afDepthStencilMode(bool depth);
#define afBindVAO glBindVertexArray
#define afBindSamplerToBindingPoint(samp,pnt) glBindSampler(pnt, samp)
void afDumpCaps();
void afDumpIsEnabled();
