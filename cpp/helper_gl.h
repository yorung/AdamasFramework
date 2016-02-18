typedef unsigned short AFIndex;
#define AFIndexTypeToDevice GL_UNSIGNED_SHORT

GLenum _afHandleGLError(const char* file, const char* func, int line, const char* command);
#define afHandleGLError(command) do{ command; _afHandleGLError(__FILE__, __FUNCTION__, __LINE__, #command); } while(0)

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
};

class CInputElement : public InputElement {
public:
	CInputElement(const char* name, ShaderFormat format, int offset, int inputSlot = 0, bool perInstance = false) {
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

void afSetVertexAttributes(const InputElement elements[], int numElements, int numBuffers, VBOID const *vertexBufferIds, const GLsizei* strides);


template <class BufName>
void afWriteBuffer(BufName bufName, const void* buf, int size)
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

enum AFDTFormat
{
	AFDT_INVALID,
	AFDT_R8G8B8A8_UNORM,
	AFDT_R5G6B5_UINT,
	AFDT_R32G32B32A32_FLOAT,
	AFDT_R16G16B16A16_FLOAT,
	AFDT_DEPTH,
	AFDT_DEPTH_STENCIL,
	AFDT_BC1_UNORM = 0x83F1,	// GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	AFDT_BC2_UNORM = 0x83F2,	// GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
	AFDT_BC3_UNORM = 0x83F3,	// GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
};
SRVID afCreateTexture2D(AFDTFormat format, const IVec2& size, void *image);
SRVID afCreateTexture2D(AFDTFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
SRVID afCreateDynamicTexture(AFDTFormat format, const IVec2& size);

void afWriteTexture(SRVID srv, const TexDesc& desc, const void* buf);

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

void afDrawIndexedTriangleStrip(int numIndices, int start = 0);
void afDrawIndexedTriangleList(int numIndices, int start = 0);
void afDrawTriangleStrip(int numVertices, int start = 0);
void afDrawLineList(int numVertices, int start = 0);
#ifdef AF_GLES31
void afDrawIndexedInstancedTriangleList(int instanceCount, int numIndices, int start = 0);
#endif

enum CullMode {
	CM_DISABLE,
	CM_CW,
	CM_CCW,
};
void afCullMode(CullMode cullMode);
enum BlendMode {
	BM_NONE,
	BM_ALPHA,
};
void afBlendMode(BlendMode mode);
enum DepthStencilMode {
	DSM_DISABLE,
	DSM_DEPTH_ENABLE,
	DSM_DEPTH_CLOSEREQUAL_READONLY,
};
void afDepthStencilMode(DepthStencilMode mode);
#define afBindVAO(vao) afHandleGLError(glBindVertexArray(vao))
#define afBindSamplerToBindingPoint(samp,pnt) afHandleGLError(glBindSampler(pnt, samp))

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
	void Init(IVec2 size, AFDTFormat colorFormat, AFDTFormat depthStencilFormat = AFDT_INVALID);
	void Destroy();
	void BeginRenderToThis();
	GLuint GetTexture() { return texColor; }
};

#ifdef AF_GLES31
typedef TBufName<GL_SHADER_STORAGE_BUFFER> SSBOID;
typedef TBufName<GL_UNIFORM_BUFFER> UBOID;
typedef AFGLName VAOID;
SSBOID afCreateSSBO(int size);
UBOID afCreateUBO(int size);
void afBindBufferToBindingPoint(SSBOID ssbo, GLuint storageBlockBinding);
void afBindBufferToBindingPoint(UBOID ubo, GLuint uniformBlockBinding);
VAOID afCreateVAO(const InputElement elements[], int numElements, int numBuffers, VBOID const *vertexBufferIds, const GLsizei* strides, IBOID ibo);
inline void afSafeDeleteVAO(VAOID& vao)
{
	if (vao != 0) {
		glDeleteVertexArrays(1, &vao.x);
		vao.x = 0;
	}
}
inline void afSafeDeleteSampler(SAMPLERID& s)
{
	if (s != 0) {
		glDeleteSamplers(1, &s.x);
		s.x = 0;
	}
}
IVec2 afGetTextureSize(GLuint tex);
IVec2 afGetRenderbufferSize(GLuint renderbuffer);

#endif
