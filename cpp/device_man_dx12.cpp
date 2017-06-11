#include "stdafx.h"

#ifdef __d3d12_h__

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

DeviceManDX12 deviceMan;

DeviceManDX12::FrameResources::~FrameResources()
{
	assert(!renderTarget);
	assert(!commandAllocator);
	assert(!constantBuffer);
	assert(!mappedConstantBuffer);
}

DeviceManDX12::~DeviceManDX12()
{
	assert(!device);
	assert(!commandQueue);
	assert(!commandList);
	assert(!fence);
	assert(!swapChain);
	assert(!rtvHeap);
	assert(!depthStencil);
	assert(!dsvHeap);
}

void DeviceManDX12::Destroy()
{
	Flush();
	commandList.Reset();
	commandQueue.Reset();
	swapChain.Reset();
	for (FrameResources& res : frameResources)
	{
		res.renderTarget.Reset();
		res.commandAllocator.Reset();
		res.srvHeap.Destroy();
		res.mappedConstantBuffer = nullptr;
		if (res.constantBuffer)
		{
			res.constantBuffer->Unmap(0, nullptr);
		}
		res.constantBuffer.Reset();
		res.fenceValueToGuard = 0;
	}
	rtvHeap.Reset();
	depthStencil.Reset();
	dsvHeap.Reset();
	fence.Reset();
	fenceValue = 1;
	frameIndex = 0;
	int cnt = device.Reset();
	if (cnt)
	{
		MessageBoxA(GetActiveWindow(), SPrintf("%d leaks detected.", cnt), "DX12 leaks", MB_OK);
	}
}

ComPtr<ID3D12Resource> DeviceManDX12::GetDefaultRenderTarget()
{
	return frameResources[frameIndex].renderTarget;
}

ComPtr<ID3D12Resource> DeviceManDX12::GetDefaultDepthStencil()
{
	return depthStencil;
}

void DeviceManDX12::BeginScene()
{
	numAssignedConstantBufferBlocks = 0;
	frameIndex = swapChain->GetCurrentBackBufferIndex();
	FrameResources& res = frameResources[frameIndex];
	res.srvHeap.ResetAllocation();
	afWaitFenceValue(fence, res.fenceValueToGuard);

	res.intermediateCommandlistDependentResources.clear();
	res.commandAllocator->Reset();
	ResetCommandListAndSetDescriptorHeap();
	commandList->ResourceBarrier(1, ToPtr<D3D12_RESOURCE_BARRIER>({ D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,{ res.renderTarget.Get(), 0, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET } }));
}

void DeviceManDX12::EndScene()
{
	FrameResources& res = frameResources[frameIndex];
	commandList->ResourceBarrier(1, ToPtr<D3D12_RESOURCE_BARRIER>({ D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,{ res.renderTarget.Get(), 0, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT } }));

	commandList->Close();
	ID3D12CommandList* lists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(arrayparam(lists));

	commandQueue->Signal(fence.Get(), res.fenceValueToGuard = fenceValue++);
}

void DeviceManDX12::Flush()
{
	if (!commandList)
	{
		return;
	}
	commandList->Close();
	ID3D12CommandList* lists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(arrayparam(lists));
	commandQueue->Signal(fence.Get(), fenceValue);
	afWaitFenceValue(fence, fenceValue++);

	for (FrameResources& res : frameResources)
	{
		res.commandAllocator->Reset();
		res.intermediateCommandlistDependentResources.clear();
	}
	ResetCommandListAndSetDescriptorHeap();
}

void DeviceManDX12::ResetCommandListAndSetDescriptorHeap()
{
	FrameResources& res = frameResources[frameIndex];
	commandList->Reset(res.commandAllocator.Get(), nullptr);
	commandList->SetDescriptorHeaps(1, ToPtr<ID3D12DescriptorHeap*>(res.srvHeap.GetHeap().Get()));
}

int DeviceManDX12::AssignConstantBuffer(const void* buf, int size)
{
	int sizeAligned = (size + 0xff) & ~0xff;
	int numRequired = sizeAligned / 0x100;

	if (numAssignedConstantBufferBlocks + numRequired > maxConstantBufferBlocks)
	{
		assert(0);
		return -1;
	}
	int top = numAssignedConstantBufferBlocks;
	numAssignedConstantBufferBlocks += numRequired;

	FrameResources& res = frameResources[frameIndex];
	memcpy(res.mappedConstantBuffer + top, buf, size);
	return top;
}

D3D12_GPU_VIRTUAL_ADDRESS DeviceManDX12::GetConstantBufferGPUAddress(int constantBufferTop)
{
	FrameResources& res = frameResources[frameIndex];
	return res.constantBuffer->GetGPUVirtualAddress() + constantBufferTop * 0x100;
}

void DeviceManDX12::AddIntermediateCommandlistDependentResource(ComPtr<ID3D12Resource> intermediateResource)
{
	FrameResources& res = frameResources[frameIndex];
	res.intermediateCommandlistDependentResources.push_back(intermediateResource);
}

void DeviceManDX12::Present()
{
	if (!swapChain)
	{
		return;
	}
	EndScene();
	swapChain->Present(AF_WAIT_VBLANK, 0);
	BeginScene();
}

void DeviceManDX12::Create(HWND hWnd)
{
	Destroy();
#ifndef NDEBUG
	ComPtr<ID3D12Debug> debug;
	if (S_OK == D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))
	{
		debug->EnableDebugLayer();
	}
#endif
	ComPtr<IDXGIFactory4> factory;
	if (S_OK != CreateDXGIFactory1(IID_PPV_ARGS(&factory)))
	{
		Destroy();
		return;
	}
	ComPtr<IDXGIAdapter1> adapter;
	for (int i = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &adapter); i++)
	{
		if (S_OK == D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))
		{
			break;
		}
	}
	if (!device)
	{
		Destroy();
		return;
	}

	if (S_OK != device->CreateCommandQueue(ToPtr<D3D12_COMMAND_QUEUE_DESC>({}), IID_PPV_ARGS(&commandQueue)))
	{
		Destroy();
		return;
	}

	RECT rc;
	GetClientRect(hWnd, &rc);

	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = numFrameBuffers;
	sd.BufferDesc.Width = rc.right - rc.left;
	sd.BufferDesc.Height = rc.bottom - rc.top;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	ComPtr<IDXGISwapChain> sc;
	if (S_OK != factory->CreateSwapChain(commandQueue.Get(), &sd, &sc))
	{
		Destroy();
		return;
	}
	if (S_OK != sc.As(&swapChain))
	{
		Destroy();
		return;
	}

	device->CreateDescriptorHeap(ToPtr<D3D12_DESCRIPTOR_HEAP_DESC>({ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1 }), IID_PPV_ARGS(&rtvHeap));
	for (int i = 0; i < numFrameBuffers; i++)
	{
		FrameResources& res = frameResources[i];
		if (S_OK != swapChain->GetBuffer(i, IID_PPV_ARGS(&res.renderTarget)))
		{
			Destroy();
			return;
		}
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&res.commandAllocator));
		res.srvHeap.Create(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, maxSrvs);
		res.constantBuffer = afCreateUBO(maxConstantBufferBlocks * 0x100);
		afHandleDXError(res.constantBuffer->Map(0, ToPtr<D3D12_RANGE>({}), (void**)&res.mappedConstantBuffer));
		afVerify(res.mappedConstantBuffer);
	}

	depthStencil = afCreateDynamicTexture(AFF_D24_UNORM_S8_UINT, IVec2((int)sd.BufferDesc.Width, (int)sd.BufferDesc.Height), AFTF_DSV);
	device->CreateDescriptorHeap(ToPtr<D3D12_DESCRIPTOR_HEAP_DESC>({ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1 }), IID_PPV_ARGS(&dsvHeap));
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	device->CreateDepthStencilView(depthStencil.Get(), nullptr, dsvHandle);

	factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frameResources[0].commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	commandList->Close();

	if (S_OK != device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))
	{
		Destroy();
		return;
	}
	BeginScene();
}

AFHeapStackAllocator& DeviceManDX12::GetFrameSRVHeap()
{
	FrameResources& res = frameResources[frameIndex];
	return res.srvHeap;
}

#endif
