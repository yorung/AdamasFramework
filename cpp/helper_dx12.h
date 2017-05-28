#include "AFGraphicsDefinitions.inl"

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
typedef SRVID AFTexRef;
inline void afSafeDeleteBuffer(ComPtr<ID3D12Resource>& p) { p.Reset(); }
inline void afSafeDeleteTexture(SRVID& p) { p.Reset(); }

void afSetDescriptorHeap(ComPtr<ID3D12DescriptorHeap> heap);
void afSetVertexBuffer(VBOID id, int stride);
void afSetVertexBuffer(int size, const void* buf, int stride);
void afSetVertexBuffers(int numIds, VBOID ids[], int strides[]);
void afSetIndexBuffer(IBOID id);
void afWriteBuffer(const IBOID id, int size, const void* buf);
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

ComPtr<ID3D12Resource> afCreateDynamicTexture(AFFormat format, const IVec2& size, uint32_t flags = AFTF_CPU_WRITE | AFTF_SRV);
SRVID afCreateTexture2D(AFFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
void afSetTextureName(SRVID tex, const char* name);

void afWriteTexture(SRVID srv, const TexDesc& desc, const void* buf);
void afWriteTexture(SRVID id, const TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);

void afWaitFenceValue(ComPtr<ID3D12Fence> fence, UINT64 value);
IVec2 afGetTextureSize(SRVID tex);

void afSetVertexBufferFromSystemMemory(const void* buf, int size, int stride);

class AFRenderStates
{
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pipelineState;
	D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
public:
	bool IsReady() { return !!pipelineState; }
	void Create(const char* shaderName, int numInputElements, const InputElement* inputElements, uint32_t flags, int numSamplerTypes_ = 0, const SamplerType samplerTypes_[] = nullptr)
	{
		(void)samplerTypes_;
		(void)numSamplerTypes_;
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

class AFRenderTarget
{
	IVec2 texSize;
	ComPtr<ID3D12Resource> renderTarget;
	bool asDefault = false;
	D3D12_RESOURCE_STATES currentState;
public:
	~AFRenderTarget() { Destroy(); }
	void InitForDefaultRenderTarget();
	void Init(IVec2 size, AFFormat colorFormat, AFFormat depthStencilFormat = AFF_INVALID);
	void Destroy();
	void BeginRenderToThis();
	void EndRenderToThis() {}
	ComPtr<ID3D12Resource> GetTexture();
};

void afBindBuffer(int size, const void* buf, int rootParameterIndex);
void afBindBuffer(UBOID ubo, int rootParameterIndex);
void afBindTexture(SRVID srv, int rootParameterIndex);

class AFCommandList
{
public:
	void SetRenderStates(AFRenderStates& rs)
	{
		rs.Apply();
	}
	void SetTexture(SRVID texId, int descritorSetIndex)
	{
		afBindTexture(texId, descritorSetIndex);
	}
	void SetBuffer(int size, const void* buf, int descritorSetIndex)
	{
		afBindBuffer(size, buf, descritorSetIndex);
	}
	void SetBuffer(UBOID uniformBuffer, int descriptorSetIndex)
	{
		afBindBuffer(uniformBuffer, descriptorSetIndex);
	}
	void SetVertexBuffer(int size, const void* buf, int stride)
	{
		afSetVertexBuffer(size, buf, stride);
	}
	void SetVertexBuffer(VBOID vertexBuffer, int stride)
	{
		afSetVertexBuffer(vertexBuffer, stride);
	}
	void SetIndexBuffer(IBOID indexBuffer)
	{
		afSetIndexBuffer(indexBuffer);
	}
	void Draw(int numVertices, int start = 0, int instanceCount = 1)
	{
		afDraw(numVertices, start, instanceCount);
	}
	void DrawIndexed(int numVertices, int start = 0, int instanceCount = 1)
	{
		afDrawIndexed(numVertices, start, instanceCount);
	}
};

#include "AFGraphicsFunctions.inl"
#include "AFDynamicQuadListVertexBuffer.inl"
