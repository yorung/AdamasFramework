typedef D3D11_INPUT_ELEMENT_DESC InputElement;

class CInputElement : public InputElement {
public:
	CInputElement(const char* name, DXGI_FORMAT format, int offset, int inputSlot = 0) {
		SemanticName = name;
		SemanticIndex = 0;
		Format = format;
		InputSlot = inputSlot;
		AlignedByteOffset = offset;
		InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		InstanceDataStepRate = 0;
	}
};

typedef ComPtr<ID3D11Buffer> IBOID;
typedef ComPtr<ID3D11Buffer> VBOID;
typedef ComPtr<ID3D11Buffer> UBOID;
typedef ComPtr<ID3D11SamplerState> SAMPLERID;
typedef ComPtr<ID3D11ShaderResourceView> SRVID;
inline void afSafeDeleteBuffer(ComPtr<ID3D11Buffer>& p) { p.Reset(); }
inline void afSafeDeleteSampler(SAMPLERID& p) { p.Reset(); }
inline void afSafeDeleteTexture(SRVID& p) { p.Reset(); }

void afWriteBuffer(const IBOID p, const void* buf, int size);
void afWriteTexture(SRVID srv, const struct TexDesc& desc, const void* buf);

IBOID afCreateIndexBuffer(const AFIndex* indi, int numIndi);
VBOID afCreateVertexBuffer(int size, const void* buf);
VBOID afCreateDynamicVertexBuffer(int size);
UBOID afCreateUBO(int size);

void afBindBufferToBindingPoint(UBOID ubo, UINT uniformBlockBinding);
void afBindTextureToBindingPoint(SRVID srv, UINT textureBindingPoint);
#define afBindCubeMapToBindingPoint afBindTextureToBindingPoint
void afBindSamplerToBindingPoint(SAMPLERID sampler, UINT textureBindingPoint);

void afDrawIndexed(PrimitiveTopology pt, int numIndices, int start = 0, int instanceCount = 1);
void afDraw(PrimitiveTopology pt, int numVertices, int start = 0, int instanceCount = 1);

typedef D3D11_SUBRESOURCE_DATA AFTexSubresourceData;

SRVID afCreateTexture2D(AFDTFormat format, const IVec2& size, void *image);
SRVID afCreateTexture2D(AFDTFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
SRVID afCreateDynamicTexture(AFDTFormat format, const IVec2& size);
IVec2 afGetTextureSize(SRVID tex);
void afSetTextureName(SRVID tex, const char* name);

class FakeVAO
{
	std::vector<VBOID> vbos;
	std::vector<ID3D11Buffer*> d3dBuffers;
	std::vector<UINT> offsets;
	std::vector<UINT> strides;
	ComPtr<ID3D11Buffer> ibo;
public:
	FakeVAO(int numBuffers, const VBOID buffers[], const int strides[], const UINT offsets[], IBOID ibo);
	void Apply();
};

typedef std::unique_ptr<FakeVAO> VAOID;
VAOID afCreateVAO(const InputElement elements[], int numElements, int numBuffers, VBOID const vertexBufferIds[], const int strides[], IBOID ibo);
void afBindVAO(const VAOID& vao);
inline void afSafeDeleteVAO(VAOID& p) { p.reset(); }

class AFRenderTarget
{
	IVec2 texSize;
	ID3D11RenderTargetView* renderTargetView = nullptr;
	ID3D11ShaderResourceView* shaderResourceView = nullptr;
//	ID3D11UnorderedAccessView* unorderedAccessView = nullptr;
	ID3D11DepthStencilView* depthStencilView = nullptr;
public:
	~AFRenderTarget() { Destroy(); }
	void InitForDefaultRenderTarget();
	void Init(IVec2 size, AFDTFormat colorFormat, AFDTFormat depthStencilFormat = AFDT_INVALID);
	void Destroy();
	void BeginRenderToThis();
	ID3D11ShaderResourceView* GetTexture() { return shaderResourceView; }
//	ID3D11UnorderedAccessView* GetUnorderedAccessView() { return unorderedAccessView; }
};

SRVID LoadTextureViaOS(const char* name, IVec2& size);
IBOID afCreateTiledPlaneIBO(int numTiles, int* numIndies = nullptr);
VBOID afCreateTiledPlaneVBO(int numTiles);
IBOID afCreateQuadListIndexBuffer(int numQuads);
SRVID afLoadTexture(const char* name, TexDesc& desc);
void afCullMode(CullMode cullMode);
void afBlendMode(BlendMode mode);
void afDepthStencilMode(DepthStencilMode mode);
SAMPLERID afCreateSampler(SamplerType type);
void afSetSampler(SamplerType type, int slot);

class AFRenderStates {
	BlendMode blendMode = BM_NONE;
	DepthStencilMode depthStencilMode = DSM_DISABLE;
	CullMode cullMode = CM_DISABLE;
	int numSamplerTypes = 0;
	const SamplerType* samplerTypes = nullptr;
	ShaderMan::SMID shaderId = ShaderMan::INVALID_SMID;
public:
	ShaderMan::SMID GetShaderId() { return shaderId; }
	bool IsReady() { return shaderId != ShaderMan::INVALID_SMID; }
	void Create(const char* shaderName, int numInputElements, const InputElement* inputElements, BlendMode blendMode_, DepthStencilMode depthStencilMode_, CullMode cullMode_, int numSamplerTypes_ = 0, const SamplerType samplerTypes_[] = nullptr);
	void Apply() const;
	void Destroy() { shaderId = ShaderMan::INVALID_SMID; }
};

class AFDynamicQuadListVertexBuffer {
	IBOID ibo;
	UINT stride;
	int vertexBufferSize;
public:
	~AFDynamicQuadListVertexBuffer() { Destroy(); }
	void Create(const InputElement*, int, int vertexSize_, int nQuad)
	{
		Destroy();
		stride = vertexSize_;
		vertexBufferSize = nQuad * vertexSize_ * 4;
		ibo = afCreateQuadListIndexBuffer(nQuad);
	}
	void Apply()
	{
		deviceMan.GetContext()->IASetIndexBuffer(ibo.Get(), AFIndexTypeToDevice, 0);
	}
	void Write(const void* buf, int size)
	{
		assert(size <= vertexBufferSize);
		VBOID vbo = afCreateDynamicVertexBuffer(size);
		afWriteBuffer(vbo, buf, size);
		ID3D11Buffer* d11Bufs[] = { vbo.Get() };
		UINT offsets[] = { 0 };
		deviceMan11.GetContext()->IASetVertexBuffers(0, dimof(d11Bufs), d11Bufs, &stride, offsets);
	}
	void Destroy()
	{
		afSafeDeleteBuffer(ibo);
	}
};

class AFCbvBindToken {
	UBOID ubo;
public:
	void Create(UBOID ubo_)
	{
		ubo = ubo_;
	}
	void Create(const void* buf, int size)
	{
		ubo = afCreateUBO(size);
		afWriteBuffer(ubo, buf, size);
	}
	UBOID Get() { return ubo; }
};

inline void afBindCbvs(AFCbvBindToken cbvs[], int nCbvs, int startBindingPoint = 0)
{
	for (int i = 0; i < nCbvs; i++)
	{
		afBindBufferToBindingPoint(cbvs[i].Get(), startBindingPoint + i);
	}
}
