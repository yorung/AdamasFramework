typedef unsigned short AFIndex;
#define AFIndexTypeToDevice GL_UNSIGNED_SHORT

enum ShaderFormat {
	SF_INVALID,
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
	SF_R32_UINT,
	SF_R32G32_UINT,
	SF_R32G32B32_UINT,
	SF_R32G32B32A32_UINT,
	SF_R8_UINT_TO_FLOAT,
	SF_R8G8_UINT_TO_FLOAT,
	SF_R8G8B8_UINT_TO_FLOAT,
	SF_R8G8B8A8_UINT_TO_FLOAT,
	SF_R16_UINT_TO_FLOAT,
	SF_R16G16_UINT_TO_FLOAT,
	SF_R16G16B16_UINT_TO_FLOAT,
	SF_R16G16B16A16_UINT_TO_FLOAT,
};

struct InputElement {
	int inputSlot = 0;
	const char* name = nullptr;
	ShaderFormat format = SF_INVALID;
	int offset = 0;
	bool perInstance = false;
	int attributeIndex = 0;
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
	CInputElement(ShaderFormat format, int attributeIndex, int offset, bool perInstance = false, int inputSlot = 0) {
		this->attributeIndex = attributeIndex;
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
	GLuint x = 0;
//	TBufName(GLuint r = 0) { x = r; }
	TBufName operator=(GLuint r) { x = r; return *this; }
	operator GLuint() const { return x; }
};
typedef TBufName<GL_ELEMENT_ARRAY_BUFFER> IBOID;
typedef TBufName<GL_ARRAY_BUFFER> VBOID;
typedef GLuint SAMPLERID;

void afSetVertexAttributes(GLuint program, const InputElement elements[], int numElements, int numBuffers, VBOID const *vertexBufferIds, const GLsizei* strides);


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
inline void afSafeDeleteTexture(GLuint& t)
{
	if (t != 0) {
		glDeleteTextures(1, &t);
		t = 0;
	}
}

enum AFDTFormat
{
	AFDT_INVALID,
	AFDT_R8G8B8A8_UINT,
	AFDT_R5G6B5_UINT,
	AFDT_R32G32B32A32_FLOAT,
	AFDT_R16G16B16A16_FLOAT,
	AFDT_DEPTH,
	AFDT_DEPTH_STENCIL,
};
GLuint afCreateDynamicTexture(int w, int h, AFDTFormat format);
GLuint afCreateWhiteTexture();

IBOID afCreateIndexBuffer(const AFIndex* indi, int numIndi);
IBOID afCreateQuadListIndexBuffer(int numQuads);
VBOID afCreateVertexBuffer(int size, const void* buf);
VBOID afCreateDynamicVertexBuffer(int size);

enum SamplerWrap {
	SW_REPEAT = GL_REPEAT,
	SW_CLAMP = GL_CLAMP_TO_EDGE,
};

enum SamplerFilter {
	SF_POINT,
	SF_LINEAR,
	SF_MIPMAP,
};
SAMPLERID afCreateSampler(SamplerFilter samplerFilter, SamplerWrap wrap);

 // without "binding" Layout Qualifier
void afLayoutSamplerBindingManually(GLuint program, const GLchar* name, GLuint samplerBinding);
#if 0
void afLayoutSSBOBindingManually(GLuint program, const GLchar* name, GLuint storageBlockBinding);
void afLayoutUBOBindingManually(GLuint program, const GLchar* name, GLuint uniformBlockBinding);
#endif
void afBindTextureToBindingPoint(GLuint tex, GLuint textureBindingPoint);
void afBindCubeMapToBindingPoint(GLuint tex, GLuint textureBindingPoint);

void afDrawIndexedTriangleList(int numIndices, int start = 0);
void afDrawIndexedTriangleStrip(int numIndices, int start = 0);
void afDrawTriangleStrip(int numVertices, int start = 0);
void afEnableBackFaceCulling(bool cullBack);

enum BlendMode {
	BM_NONE,
	BM_ALPHA,
};
void afBlendMode(BlendMode mode);
void afDepthStencilMode(bool depth);
#define afBindVAO glBindVertexArray
#define afBindSamplerToBindingPoint(samp,pnt) glBindSampler(pnt, samp)
GLenum _afHandleGLError(const char* file, const char* func, int line, const char* command);
#define afHandleGLError(command) do{ command; _afHandleGLError(__FILE__, __FUNCTION__, __LINE__, #command); } while(0)

void afDumpCaps();
void afDumpIsEnabled();

class AFRenderTarget
{
	ivec2 texSize;
	GLuint texColor = 0;
	GLuint texDepth = 0;
	GLuint framebufferObject = 0;
	GLuint renderbufferObject = 0;
public:
	void InitForDefaultRenderTarget();
	void Init(ivec2 size, AFDTFormat colorFormat, AFDTFormat depthStencilFormat);
	void Destroy();
	void BeginRenderToThis();
	GLuint GetTexture() { return texColor; }
};

#ifdef AF_GLES31
typedef TBufName<GL_SHADER_STORAGE_BUFFER> SSBOID;
typedef TBufName<GL_UNIFORM_BUFFER> UBOID;
typedef GLuint VAOID;
SSBOID afCreateSSBO(int size);
UBOID afCreateUBO(int size);
void afBindBufferToBindingPoint(SSBOID ssbo, GLuint storageBlockBinding);
void afBindBufferToBindingPoint(UBOID ubo, GLuint uniformBlockBinding);
VAOID afCreateVAO(GLuint program, const InputElement elements[], int numElements, int numBuffers, VBOID const *vertexBufferIds, const GLsizei* strides, IBOID ibo);
inline void afSafeDeleteVAO(GLuint& vao)
{
	if (vao != 0) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
}
inline void afSafeDeleteSampler(GLuint& s)
{
	if (s != 0) {
		glDeleteSamplers(1, &s);
		s = 0;
	}
}
ivec2 afGetTextureSize(GLuint tex);
ivec2 afGetRenderbufferSize(GLuint renderbuffer);

#endif
