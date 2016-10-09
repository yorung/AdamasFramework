typedef D3D12_INPUT_ELEMENT_DESC InputElement;

class CInputElement : public InputElement {
public:
	CInputElement(const char* name, DXGI_FORMAT format, int offset, int inputSlot = 0) {
		SemanticName = name;
		SemanticIndex = 0;
		Format = format;
		InputSlot = inputSlot;
		AlignedByteOffset = offset;
		InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		InstanceDataStepRate = 0;
	}
};

typedef ComPtr<ID3D12Resource> IBOID;
typedef ComPtr<ID3D12Resource> VBOID;
typedef ComPtr<ID3D12Resource> UBOID;
typedef ComPtr<ID3D12Resource> SRVID;
inline void afSafeDeleteBuffer(ComPtr<ID3D12Resource>& p) { p.Reset(); }
inline void afSafeDeleteTexture(SRVID& p) { p.Reset(); }

void afSetDescriptorHeap(ComPtr<ID3D12DescriptorHeap> heap);
void afSetVertexBuffer(VBOID id, int stride);
void afSetVertexBuffers(int numIds, VBOID ids[], int strides[]);
void afSetIndexBuffer(IBOID id);
void afWriteBuffer(const IBOID id, const void* buf, int size);
ComPtr<ID3D12Resource> afCreateBuffer(int size, const void* buf = nullptr);
VBOID afCreateVertexBuffer(int size, const void* buf = nullptr);
IBOID afCreateIndexBuffer(int numIndi, const AFIndex* indi);
ComPtr<ID3D12Resource> afCreateDynamicVertexBuffer(int size, const void* buf = nullptr);
UBOID afCreateUBO(int size);

ComPtr<ID3D12PipelineState> afCreatePSO(const char *shaderName, const InputElement elements[], int numElements, uint32_t flags, ComPtr<ID3D12RootSignature>& rootSignature);

void afDrawIndexed(int numIndices, int start = 0, int instanceCount = 1);
void afDraw(int numVertices, int start = 0, int instanceCount = 1);

struct AFTexSubresourceData
{
	const void* ptr;
	uint32_t pitch;
	uint32_t pitchSlice;
};

SRVID afCreateTexture2D(AFFormat format, const IVec2& size, void *image = nullptr, bool isRenderTargetOrDepthStencil = false);
SRVID afCreateTexture2D(AFFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
void afSetTextureName(SRVID tex, const char* name);

void afWriteTexture(SRVID srv, const TexDesc& desc, const void* buf);
void afWriteTexture(SRVID id, const TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
#define afCreateDynamicTexture afCreateTexture2D

SRVID LoadTextureViaOS(const char* name, IVec2& size);
IBOID afCreateTiledPlaneIBO(int numTiles, int* numIndies = nullptr);
SRVID afLoadTexture(const char* name, TexDesc& desc);
VBOID afCreateTiledPlaneVBO(int numTiles);
IBOID afCreateQuadListIndexBuffer(int numQuads);

ComPtr<ID3D12DescriptorHeap> afCreateDescriptorHeap(int numSrvs, SRVID srvs[]);
void afWaitFenceValue(ComPtr<ID3D12Fence> fence, UINT64 value);
IVec2 afGetTextureSize(SRVID tex);

void afSetVertexBufferFromSystemMemory(const void* buf, int size, int stride);

class AFRenderStates {
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pipelineState;
	PrimitiveTopology primitiveTopology = PT_TRIANGLESTRIP;
public:
	bool IsReady() { return !!pipelineState; }
	void Create(const char* shaderName, int numInputElements, const InputElement* inputElements, uint32_t flags, int numSamplerTypes_ = 0, const SamplerType samplerTypes_[] = nullptr, PrimitiveTopology primitiveTopology_ = PT_TRIANGLESTRIP)
	{
		primitiveTopology = RenderFlagsToPrimitiveTopology(flags);
		pipelineState = afCreatePSO(shaderName, inputElements, numInputElements, flags, rootSignature);
	}
	void Apply() const
	{
		ID3D12GraphicsCommandList* list = deviceMan.GetCommandList();
		list->SetPipelineState(pipelineState.Get());
		list->SetGraphicsRootSignature(rootSignature.Get());
		list->IASetPrimitiveTopology(primitiveTopology);
	}
	void Destroy()
	{
		pipelineState.Reset();
		rootSignature.Reset();
	}
};

class AFDynamicQuadListVertexBuffer {
	IBOID ibo;
	UINT stride;
	int vertexBufferSize;
public:
	~AFDynamicQuadListVertexBuffer() { Destroy(); }
	void Create(int vertexSize_, int nQuad);
	void Apply()
	{
		afSetIndexBuffer(ibo);
	}
	void Write(const void* buf, int size);
	void Destroy()
	{
		afSafeDeleteBuffer(ibo);
	}
};

class AFCbvBindToken {
public:
	UBOID ubo;
	int top = -1;
	int size = 0;
	void Create(UBOID ubo_)
	{
		ubo = ubo_;
	}
	void Create(const void* buf, int size_)
	{
		top = deviceMan.AssignConstantBuffer(buf, size_);
		size = size_;
	}
};

void afBindCbvs(AFCbvBindToken cbvs[], int nCbvs, int startBindingPoint = 0);

class AFRenderTarget
{
	IVec2 texSize;
	ComPtr<ID3D12Resource> renderTarget;
	bool asDefault = false;
public:
	~AFRenderTarget() { Destroy(); }
	void InitForDefaultRenderTarget();
	void Init(IVec2 size, AFFormat colorFormat, AFFormat depthStencilFormat = AFF_INVALID);
	void Destroy();
	void BeginRenderToThis();
	ComPtr<ID3D12Resource> GetTexture() { return renderTarget; }
};

void afBindBufferToBindingPoint(const void* buf, int size, int rootParameterIndex);
void afBindBufferToBindingPoint(UBOID ubo, int rootParameterIndex);
void afBindTexture(SRVID srv, int rootParameterIndex);
#define afBindCubeMap afBindTexture
