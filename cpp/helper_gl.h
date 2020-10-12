#pragma once

#include "AFGraphicsDefinitions.inl"

typedef unsigned short AFIndex;
constexpr GLenum AFIndexTypeToDevice = GL_UNSIGNED_SHORT;

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
	AFF_D32_FLOAT,
	AFF_D24_UNORM_S8_UINT,
	AFF_D32_FLOAT_S8_UINT,
	AFF_R16G16B16A16_FLOAT,
	AFF_BC1_UNORM = 0x83F1,	// GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	AFF_BC2_UNORM = 0x83F2,	// GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
	AFF_BC3_UNORM = 0x83F3,	// GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
	AFF_B8G8R8A8_UNORM,
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

typedef GLuint SAMPLERID;

class TextureContext
{
public:
	GLuint name = 0;
	bool isCubemap = false;
	~TextureContext()
	{
		if (name != 0)
		{
			glDeleteTextures(1, &name);
			name = 0;
		}
	}
	operator GLuint() const { return name; }
};

typedef std::shared_ptr<TextureContext> SRVID;
typedef SRVID AFTexRef;
typedef GLuint AFBufferResource;

void afSetVertexAttributes(const InputElement elements[], int numElements, int numBuffers, AFBufferResource const vertexBufferIds[], const int strides[]);
void afSetVertexAttributes(const InputElement elements[], int numElements, int numBuffers, void const* vertexBuffers[], const int strides[]);
void afSetIndexBuffer(AFBufferResource indexBuffer);

inline void afSafeDeleteBuffer(GLuint& b)
{
	if (b != 0)
	{
		glDeleteBuffers(1, &b);
		b = 0;
	}
}

inline void afSafeDeleteTexture(AFTexRef& tex) { tex.reset(); }

struct AFTexSubresourceData
{
	const void* ptr;
	uint32_t pitch;
	uint32_t pitchSlice;
};

SRVID afCreateTexture2D(AFFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
SRVID afCreateDynamicTexture(AFFormat format, const IVec2& size);

void afWriteTexture(SRVID srv, const TexDesc& desc, const void* buf);

GLuint afCreateBuffer(int size, const void* buf, AFBufferType bufferType);

#ifdef AF_GLES31
inline void afBindSamplerToBindingPoint(SAMPLERID samp, int pnt)
{
	afHandleGLError(glBindSampler(pnt, samp));
}

inline void afSafeDeleteSampler(SAMPLERID& s)
{
	if (s != 0)
	{
		glDeleteSamplers(1, &s);
		s = 0;
	}
}
#endif

// without "binding" Layout Qualifier
void afLayoutSamplerBindingManually(GLuint program, const GLchar* name, GLuint samplerBinding);
#if 0
void afLayoutSSBOBindingManually(GLuint program, const GLchar* name, GLuint storageBlockBinding);
void afLayoutUBOBindingManually(GLuint program, const GLchar* name, GLuint uniformBlockBinding);
#endif
void afBindTexture(AFTexRef tex, GLuint textureBindingPoint);

enum PrimitiveTopology {
	PT_TRIANGLESTRIP = GL_TRIANGLE_STRIP,
	PT_TRIANGLELIST = GL_TRIANGLES,
	PT_LINELIST = GL_LINES,
};

void afDrawIndexed(PrimitiveTopology primitiveTopology, int numIndices, int start = 0, int instanceCount = 1);
void afDraw(PrimitiveTopology primitiveTopology, int numVertices, int start = 0, int instanceCount = 1);

void afDumpCaps();
void afDumpIsEnabled();

void afBeginRenderToSwapChain();
inline void afEndRenderToSwapChain() {}

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
	void EndRenderToThis() {}
	SRVID GetTexture() { return texColor; }
};

#ifdef AF_GLES31
typedef GLuint SSBOID;
typedef GLuint VAOID;
SSBOID afCreateSSBO(int size);
void afBindSSBO(SSBOID ssbo, GLuint storageBlockBinding);
void afBindConstantBuffer(AFBufferResource ubo, GLuint uniformBlockBinding);
VAOID afCreateVAO(const InputElement elements[], int numElements, int numBuffers, AFBufferResource const *vertexBufferIds, const int* strides, AFBufferResource ibo);
inline void afSafeDeleteVAO(VAOID& vao)
{
	if (vao != 0)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
}
IVec2 afGetRenderbufferSize(GLuint renderbuffer);
SAMPLERID afCreateSampler(SamplerType type);
void afSetSampler(SamplerType type, int slot);
IVec2 afGetTextureSize(SRVID tex);
#endif

void afUpdateUniformVariable(GLuint program, int size, const void* buffer, const char* name);
inline void afSetTextureName(SRVID, const char*) {}
void afClear();

void afCullMode(uint32_t flags);
void afBlendMode(uint32_t flags);
void afDepthStencilMode(uint32_t flags);

class AFRenderStates
{
	uint32_t flags = AFRS_NONE;
	int numSamplerTypes = 0;
	const SamplerType* samplerTypes = nullptr;
	GLuint shaderId = 0;
	const InputElement* elements;
	int numElements;
public:
	~AFRenderStates() { Destroy(); }
	GLuint GetShaderId() { return shaderId; }
	void GetInputElements(const InputElement*& elements_, int& numElements_) const
	{
		elements_ = elements;
		numElements_ = numElements;
	}
	PrimitiveTopology GetPrimitiveTopology() const;
	bool IsReady() { return shaderId != 0; }
	void Create(const char* shaderName, int numInputElements = 0, const InputElement* inputElements = nullptr, uint32_t flags = AFRS_NONE, int numSamplerTypes_ = 0, const SamplerType samplerTypes_[] = nullptr);
	void Apply() const;
	void Destroy();
};

class AFCommandList
{
	AFRenderStates* currentRS = nullptr;
public:
	void SetRenderStates(AFRenderStates& rs)
	{
		rs.Apply();
		currentRS = &rs;
	}
	void SetTexture(AFTexRef texId, int descritorSetIndex)
	{
		afBindTexture(texId, descritorSetIndex);
	}
	void SetBuffer(int size, const void* buf, int descritorSetIndex)
	{
		char name[] = { 'b', (char)('0' + descritorSetIndex), '\0' };
		afUpdateUniformVariable(currentRS->GetShaderId(), size, buf, name);
	}
#if defined(AF_GLES31)
	void SetBuffer(AFBufferResource uniformBuffer, int descriptorSetIndex)
	{
		afBindConstantBuffer(uniformBuffer, descriptorSetIndex);
	}
#endif
	// The driver doesn't know the size of buffer before draw calls.
	// This means driver only keeps the pointer of buf, so caller must care to keep whole buf until draw calls.
	void SetVertexBuffer(int /*size*/, const void* buf, int stride)
	{
		const InputElement* elements;
		int numElements;
		currentRS->GetInputElements(elements, numElements);
		afSetVertexAttributes(elements, numElements, 1, &buf, &stride);
	}
	void SetVertexBuffer(AFBufferResource vertexBuffer, int stride)
	{
		const InputElement* elements;
		int numElements;
		currentRS->GetInputElements(elements, numElements);
		afSetVertexAttributes(elements, numElements, 1, &vertexBuffer, &stride);
	}
	void SetIndexBuffer(AFBufferResource indexBuffer)
	{
		afSetIndexBuffer(indexBuffer);
	}
	void Draw(int numVertices, int start = 0, int instanceCount = 1)
	{
		afDraw(currentRS->GetPrimitiveTopology(), numVertices, start, instanceCount);
	}
	void DrawIndexed(int numVertices, int start = 0, int instanceCount = 1)
	{
		afDrawIndexed(currentRS->GetPrimitiveTopology(), numVertices, start, instanceCount);
	}
};

#include "AFGraphicsFunctions.inl"
#include "AFDynamicQuadListVertexBuffer.inl"
