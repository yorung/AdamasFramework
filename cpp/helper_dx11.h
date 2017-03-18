#include "AFGraphicsDefinitions.inl"

class DeviceMan11
{
	ID3D11Device* pDevice = nullptr;
	ComPtr<IDXGISwapChain> pSwapChain;
	ComPtr<ID3D11DeviceContext> pImmediateContext;
	ComPtr<ID3D11Texture2D> renderTarget;
	ComPtr<ID3D11Texture2D> depthStencil;
public:
	~DeviceMan11();

	void Create(HWND hWnd);
	void Destroy();

	void Present();

	ID3D11Device* GetDevice() { return pDevice; }
	ID3D11DeviceContext* GetContext() { return pImmediateContext.Get(); }
	ComPtr<ID3D11Texture2D>	GetDefaultRenderTarget() { return renderTarget.Get(); }
	ComPtr<ID3D11Texture2D> GetDefaultDepthStencil() { return depthStencil.Get(); }
};

extern DeviceMan11 deviceMan11;
#define deviceMan deviceMan11

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
typedef ComPtr<ID3D11Resource> AFTexRef;
inline void afSafeDeleteBuffer(ComPtr<ID3D11Buffer>& p) { p.Reset(); }
inline void afSafeDeleteSampler(SAMPLERID& p) { p.Reset(); }
inline void afSafeDeleteTexture(SRVID& p) { p.Reset(); }
inline void afSafeDeleteTexture(AFTexRef& p) { p.Reset(); }

void afSetVertexBuffer(VBOID vertexBuffer, int stride);
void afSetVertexBuffer(int size, const void* buf, int stride);
void afSetIndexBuffer(IBOID indexBuffer);

void afWriteBuffer(const IBOID p, int size, const void* buf);
void afWriteTexture(AFTexRef srv, const struct TexDesc& desc, const void* buf);

IBOID afCreateIndexBuffer(int numIndi, const AFIndex* indi);
VBOID afCreateVertexBuffer(int size, const void* buf);
VBOID afCreateDynamicVertexBuffer(int size);
UBOID afCreateUBO(int size, const void* buf = nullptr);

SRVID afCreateSRVFromTexture(AFTexRef tex);
ComPtr<ID3D11DepthStencilView> afCreateDSVFromTexture(ComPtr<ID3D11Resource> tex);
ComPtr<ID3D11RenderTargetView> afCreateRTVFromTexture(AFTexRef tex, AFFormat formatAs = AFF_INVALID);

void afBindBuffer(UBOID ubo, UINT slot);
void afBindBuffer(int size, const void* buf, UINT slot);
void afBindTexture(SRVID srv, uint32_t slot);
void afBindTexture(ComPtr<ID3D11Resource> tex, uint32_t slot);
void afBindSamplerToBindingPoint(SAMPLERID sampler, UINT slot);

void afDrawIndexed(int numIndices, int start = 0, int instanceCount = 1);
void afDraw(int numVertices, int start = 0, int instanceCount = 1);

typedef D3D11_SUBRESOURCE_DATA AFTexSubresourceData;

ComPtr<ID3D11Texture2D> afCreateTexture2D(AFFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
ComPtr<ID3D11Texture2D> afCreateDynamicTexture(AFFormat format, const IVec2& size, uint32_t flags = AFTF_CPU_WRITE | AFTF_SRV);
IVec2 afGetTextureSize(ComPtr<ID3D11View> view);
IVec2 afGetTextureSize(ComPtr<ID3D11Texture2D> tex);
void afSetTextureName(AFTexRef tex, const char* name);
void afSetRenderTarget(ComPtr<ID3D11Resource> color, ComPtr<ID3D11Resource> depthStencil);
void afClearRenderTarget(ComPtr<ID3D11Resource> color);
void afClearDepthStencil(ComPtr<ID3D11Resource> depthStencil);

class AFRenderTarget
{
	IVec2 texSize;
	ComPtr<ID3D11Texture2D> renderTarget, depthStencil;
//	ComPtr<ID3D11RenderTargetView> renderTargetView;
//	ComPtr<ID3D11DepthStencilView> depthStencilView;
public:
	~AFRenderTarget() { Destroy(); }
	void InitForDefaultRenderTarget();
	void Init(IVec2 size, AFFormat colorFormat, AFFormat depthStencilFormat = AFF_INVALID);
	void Destroy();
	void BeginRenderToThis();
	AFTexRef GetTexture() { return renderTarget; }
};

void afCullMode(uint32_t flags);
void afBlendMode(uint32_t flags);
void afDepthStencilMode(uint32_t flags);
SAMPLERID afCreateSampler(SamplerType type);
void afSetSampler(SamplerType type, int slot);

class AFRenderStates {
	uint32_t flags = AFRS_NONE;
	int numSamplerTypes = 0;
	const SamplerType* samplerTypes = nullptr;
	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
public:
	bool IsReady() { return !!pixelShader; }
	void Create(const char* shaderName, int numInputElements = 0, const InputElement* inputElements = nullptr, uint32_t flags = AFRS_NONE, int numSamplerTypes_ = 0, const SamplerType samplerTypes_[] = nullptr);
	void Apply() const;
	void Destroy();
};

class AFCommandList
{
public:
	void SetRenderStates(AFRenderStates& rs)
	{
		rs.Apply();
	}
	void SetTexture(SRVID texId, uint32_t slot)
	{
		afBindTexture(texId, slot);
	}
	void SetTexture(AFTexRef tex, uint32_t slot)
	{
		afBindTexture(tex, slot);
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
