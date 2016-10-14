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

void afSetVertexBuffer(VBOID vertexBuffer, int stride);
void afSetVertexBuffer(int size, const void* buf, int stride);
void afSetIndexBuffer(IBOID indexBuffer);

void afWriteBuffer(const IBOID p, int size, const void* buf);
void afWriteTexture(SRVID srv, const struct TexDesc& desc, const void* buf);

IBOID afCreateIndexBuffer(int numIndi, const AFIndex* indi);
VBOID afCreateVertexBuffer(int size, const void* buf);
VBOID afCreateDynamicVertexBuffer(int size);
UBOID afCreateUBO(int size, const void* buf = nullptr);

void afBindBuffer(UBOID ubo, UINT slot);
void afBindBuffer(int size, const void* buf, UINT slot);
void afBindTexture(SRVID srv, UINT slot);
#define afBindCubeMap afBindTexture
void afBindSamplerToBindingPoint(SAMPLERID sampler, UINT slot);

void afDrawIndexed(int numIndices, int start = 0, int instanceCount = 1);
void afDraw(int numVertices, int start = 0, int instanceCount = 1);

typedef D3D11_SUBRESOURCE_DATA AFTexSubresourceData;

SRVID afCreateTexture2D(AFFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
SRVID afCreateDynamicTexture(AFFormat format, const IVec2& size);
IVec2 afGetTextureSize(SRVID tex);
void afSetTextureName(SRVID tex, const char* name);

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
	void Init(IVec2 size, AFFormat colorFormat, AFFormat depthStencilFormat = AFF_INVALID);
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
public:
	ShaderMan::SMID GetShaderId() { return shaderId; }
	bool IsReady() { return shaderId != ShaderMan::INVALID_SMID; }
	void Create(const char* shaderName, int numInputElements = 0, const InputElement* inputElements = nullptr, uint32_t flags = AFRS_NONE, int numSamplerTypes_ = 0, const SamplerType samplerTypes_[] = nullptr);
	void Apply() const;
	void Destroy() { shaderId = ShaderMan::INVALID_SMID; }
};

class AFDynamicQuadListVertexBuffer {
	IBOID ibo;
	UINT stride;
	int vertexBufferSize;
public:
	~AFDynamicQuadListVertexBuffer() { Destroy(); }
	void Create(int vertexSize_, int nQuad)
	{
		Destroy();
		stride = vertexSize_;
		vertexBufferSize = nQuad * vertexSize_ * 4;
		ibo = afCreateQuadListIndexBuffer(nQuad);
	}
	void Apply()
	{
		afSetIndexBuffer(ibo);
	}
	void Write(const void* buf, int size)
	{
		assert(size <= vertexBufferSize);
		afSetVertexBuffer(size, buf, stride);
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
		afWriteBuffer(ubo, size, buf);
	}
	UBOID Get() { return ubo; }
};

inline void afBindCbvs(AFCbvBindToken cbvs[], int nCbvs, int startBindingPoint = 0)
{
	for (int i = 0; i < nCbvs; i++)
	{
		afBindBuffer(cbvs[i].Get(), startBindingPoint + i);
	}
}
