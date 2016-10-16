#include "AFGraphicsDefinitions.inl"

typedef unsigned short AFIndex;
#define AFIndexTypeToDevice GL_UNSIGNED_SHORT

GLenum _afHandleGLError(const char* file, const char* func, int line, const char* command);
#define afHandleGLError(command) do{ command; _afHandleGLError(__FILE__, __FUNCTION__, __LINE__, #command); } while(0)

enum AFFormat {
	AFF_INVALID,
	AFF_R32_FLOAT,
	AFF_R32G32_FLOAT,
	AFF_R32G32B32_FLOAT,
	AFF_R32G32B32A32_FLOAT,
	AFF_R8_UNORM,
	AFF_R8G8_UNORM,
	AFF_R8G8B8_UNORM,
	AFF_R8G8B8A8_UNORM,
	AFF_R8_UINT,
	AFF_R8G8_UINT,
	AFF_R8G8B8_UINT,
	AFF_R8G8B8A8_UINT,
	AFF_R16_UINT,
	AFF_R16G16_UINT,
	AFF_R16G16B16_UINT,
	AFF_R16G16B16A16_UINT,
	AFF_R32_UINT,
	AFF_R32G32_UINT,
	AFF_R32G32B32_UINT,
	AFF_R32G32B32A32_UINT,
	AFF_R8G8B8A8_UNORM_SRGB,
	AFF_R5G6B5_UINT,
	AFF_DEPTH,
	AFF_DEPTH_STENCIL,
	AFF_R16G16B16A16_FLOAT,
	AFF_BC1_UNORM = 0x83F1,	// GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	AFF_BC2_UNORM = 0x83F2,	// GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
	AFF_BC3_UNORM = 0x83F3,	// GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
};

struct InputElement {
	int inputSlot = 0;
	const char* name = nullptr;
	AFFormat format = AFF_INVALID;
	int offset = 0;
	bool perInstance = false;
};

class CInputElement : public InputElement {
public:
	CInputElement(const char* name, AFFormat format, int offset, int inputSlot = 0, bool perInstance = false) {
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
	GLuint x = 0;
	operator GLuint() const { return x; }
};
struct AFGLName {
	GLuint x = 0;
	operator GLuint() const { return x; }
};

typedef TBufName<GL_ELEMENT_ARRAY_BUFFER> IBOID;
typedef TBufName<GL_ARRAY_BUFFER> VBOID;
typedef AFGLName SAMPLERID;
typedef AFGLName SRVID;

void DiscardIntermediateGLBuffers();

void afSetVertexAttributes(const InputElement elements[], int numElements, int numBuffers, VBOID const vertexBufferIds[], const int strides[]);
void afSetVertexBuffer(VBOID id, int stride);
void afSetIndexBuffer(IBOID indexBuffer);

template <class BufName>
void afWriteBuffer(BufName bufName, int size, const void* buf)
{
	afHandleGLError(glBindBuffer(BufName::bufType, bufName));
	afHandleGLError(glBufferSubData(BufName::bufType, 0, size, buf));
	afHandleGLError(glBindBuffer(BufName::bufType, 0));
}

template <class BufName>
inline void afSafeDeleteBuffer(BufName& b)
{
	if (b.x != 0) {
		glDeleteBuffers(1, &b.x);
		b.x = 0;
	}
}
inline void afSafeDeleteTexture(SRVID& t)
{
	if (t != 0) {
		glDeleteTextures(1, &t.x);
		t.x = 0;
	}
}

struct AFTexSubresourceData
{
	const void* ptr;
	uint32_t pitch;
	uint32_t pitchSlice;
};

SRVID afCreateTexture2D(AFFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
SRVID afCreateDynamicTexture(AFFormat format, const IVec2& size);

void afWriteTexture(SRVID srv, const TexDesc& desc, const void* buf);

IBOID afCreateIndexBuffer(int numIndi, const AFIndex* indi);
VBOID afCreateVertexBuffer(int size, const void* buf);
VBOID afCreateDynamicVertexBuffer(int size);

#ifdef AF_GLES31
inline void afBindSamplerToBindingPoint(SAMPLERID samp, int pnt)
{
	afHandleGLError(glBindSampler(pnt, samp));
}

inline void afSafeDeleteSampler(SAMPLERID& s)
{
	if (s != 0) {
		glDeleteSamplers(1, &s.x);
		s.x = 0;
	}
}
#endif


 // without "binding" Layout Qualifier
void afLayoutSamplerBindingManually(GLuint program, const GLchar* name, GLuint samplerBinding);
#if 0
void afLayoutSSBOBindingManually(GLuint program, const GLchar* name, GLuint storageBlockBinding);
void afLayoutUBOBindingManually(GLuint program, const GLchar* name, GLuint uniformBlockBinding);
#endif
void afBindTexture(GLuint tex, GLuint textureBindingPoint);
void afBindCubeMap(GLuint tex, GLuint textureBindingPoint);

enum PrimitiveTopology {
	PT_TRIANGLESTRIP = GL_TRIANGLE_STRIP,
	PT_TRIANGLELIST = GL_TRIANGLES,
	PT_LINELIST = GL_LINES,
};

void afDrawIndexed(int numIndices, int start = 0, int instanceCount = 1);
void afDraw(int numVertices, int start = 0, int instanceCount = 1);

#ifdef AF_GLES31
#define afBindVAO(vao) afHandleGLError(glBindVertexArray(vao))
#endif

void afDumpCaps();
void afDumpIsEnabled();

class AFRenderTarget
{
	IVec2 texSize;
	SRVID texColor;
	SRVID texDepth;
	GLuint framebufferObject = 0;
	GLuint renderbufferObject = 0;
public:
	void InitForDefaultRenderTarget();
	void Init(IVec2 size, AFFormat colorFormat, AFFormat depthStencilFormat = AFF_INVALID);
	void Destroy();
	void BeginRenderToThis();
	SRVID GetTexture() { return texColor; }
};

#ifdef AF_GLES31
typedef TBufName<GL_SHADER_STORAGE_BUFFER> SSBOID;
typedef TBufName<GL_UNIFORM_BUFFER> UBOID;
typedef AFGLName VAOID;
SSBOID afCreateSSBO(int size);
UBOID afCreateUBO(int size, const void* buf = nullptr);
void afBindBuffer(SSBOID ssbo, GLuint storageBlockBinding);
void afBindBuffer(UBOID ubo, GLuint uniformBlockBinding);
void afBindBuffer(int size, const void* buffer, GLuint uniformBlockBinding);
VAOID afCreateVAO(const InputElement elements[], int numElements, int numBuffers, VBOID const *vertexBufferIds, const int* strides, IBOID ibo);
inline void afSafeDeleteVAO(VAOID& vao)
{
	if (vao != 0) {
		glDeleteVertexArrays(1, &vao.x);
		vao.x = 0;
	}
}
IVec2 afGetTextureSize(SRVID tex);
IVec2 afGetRenderbufferSize(GLuint renderbuffer);
inline void afSetTextureName(SRVID, const char*) {}

void afClear();

#endif

void afCullMode(uint32_t flags);
void afBlendMode(uint32_t flags);
void afDepthStencilMode(uint32_t flags);
SAMPLERID afCreateSampler(SamplerType type);
void afSetSampler(SamplerType type, int slot);

class AFRenderStates {
	uint32_t flags = AFRS_NONE;
	int numSamplerTypes = 0;
	const SamplerType* samplerTypes = nullptr;
	ShaderMan::SMID shaderId = ShaderMan::INVALID_SMID;
	const InputElement* elements;
	int numElements;
public:
	ShaderMan::SMID GetShaderId() { return shaderId; }
	bool IsReady() { return shaderId != ShaderMan::INVALID_SMID; }
	void Create(const char* shaderName, int numInputElements = 0, const InputElement* inputElements = nullptr, uint32_t flags = AFRS_NONE, int numSamplerTypes_ = 0, const SamplerType samplerTypes_[] = nullptr);
	void Apply() const;
	void Destroy() { shaderId = ShaderMan::INVALID_SMID; }
};

#include "AFGraphicsFunctions.inl"

class AFDynamicQuadListVertexBuffer
{
	IBOID ibo;
	VBOID vbo;
	int nQuad;
	int vertexSize;
	int vertexBufferSize;
public:
	~AFDynamicQuadListVertexBuffer() { Destroy(); }
	void Create(int vertexSize_, int nQuad_)
	{
		Destroy();
		nQuad = nQuad_;
		vertexSize = vertexSize_;
		vertexBufferSize = nQuad * vertexSize * 4;
		ibo = afCreateQuadListIndexBuffer(nQuad);
		vbo = afCreateDynamicVertexBuffer(vertexBufferSize);
	}
	void Apply()
	{
		afSetVertexBuffer(vbo, vertexSize);
		afSetIndexBuffer(ibo);
	}
	void Write(const void* buf, int size)
	{
		assert(size <= vertexBufferSize);
		afWriteBuffer(vbo, size, buf);
	}
	void Destroy()
	{
		afSafeDeleteBuffer(ibo);
		afSafeDeleteBuffer(vbo);
	}
};
