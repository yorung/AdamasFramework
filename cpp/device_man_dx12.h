class DeviceManDX12
{
	static const UINT numFrameBuffers = 2;
	static const UINT maxSrvs = 1024;
	static const UINT maxConstantBufferBlocks = 1000;
	int numAssignedConstantBufferBlocks = 0;
	class FrameResources
	{
	public:
		~FrameResources();
		std::vector<ComPtr<ID3D12Resource>> intermediateCommandlistDependentResources;
		ComPtr<ID3D12Resource> renderTarget;
		std::vector<ComPtr<ID3D12CommandAllocator>> commandAllocators;
		ComPtr<ID3D12Resource> constantBuffer;
		AFHeapStackAllocator srvHeap;
		struct { char buf[256]; } *mappedConstantBuffer = nullptr;
		UINT64 fenceValueToGuard = 0;
	} frameResources[numFrameBuffers];
	ComPtr<ID3D12Device> device;
	ComPtr<IDXGISwapChain3> swapChain;
	ComPtr<ID3D12DescriptorHeap> rtvHeap, dsvHeap;
	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue = 1;
	UINT frameIndex = 0;
	void BeginScene();
	void EndScene();
	void ResetCommandListAndSetDescriptorHeap();
public:
	~DeviceManDX12();
	void Create(HWND hWnd);
	void Destroy();
	void Present();
	void Flush(bool wait = true);
	ComPtr<ID3D12Resource> GetDefaultRenderTarget();
	D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() { return dsvHeap->GetCPUDescriptorHandleForHeapStart(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() { return rtvHeap->GetCPUDescriptorHandleForHeapStart(); }
	AFHeapStackAllocator& GetFrameSRVHeap();
	int AssignConstantBuffer(const void* buf, int size);
	D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferGPUAddress(int constantBufferTop);
	ComPtr<ID3D12Device> GetDevice() { return device; }
	ID3D12GraphicsCommandList* GetCommandList() { return commandList.Get(); }
	void AddIntermediateCommandlistDependentResource(ComPtr<ID3D12Resource> intermediateResource);
};

extern DeviceManDX12 deviceMan;
