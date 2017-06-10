#include "stdafx.h"

#ifdef __d3d12_h__

static const D3D12_HEAP_PROPERTIES defaultHeapProperties = { D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };
static const D3D12_HEAP_PROPERTIES uploadHeapProperties = { D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };
static const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

void afSetVertexBuffer(VBOID id, int stride)
{
	ID3D12GraphicsCommandList* list = deviceMan.GetCommandList();
	D3D12_RESOURCE_DESC desc = id->GetDesc();
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = { id->GetGPUVirtualAddress(), (UINT)desc.Width, (UINT)stride };
	list->IASetVertexBuffers(0, 1, &vertexBufferView);
}

void afSetVertexBuffer(int size, const void* buf, int stride)
{
	VBOID vbo = afCreateDynamicVertexBuffer(size, buf);
	afSetVertexBuffer(vbo, stride);
	deviceMan.AddIntermediateCommandlistDependentResource(vbo);
}

void afSetVertexBuffers(int numIds, VBOID ids[], int strides[])
{
	ID3D12GraphicsCommandList* list = deviceMan.GetCommandList();
	D3D12_VERTEX_BUFFER_VIEW views[10];
	assert(numIds < _countof(views));
	for (int i = 0; i < numIds; i++) {
		D3D12_RESOURCE_DESC desc = ids[i]->GetDesc();
		views[i] = { ids[i]->GetGPUVirtualAddress(), (UINT)desc.Width, (UINT)strides[i] };
	}

	list->IASetVertexBuffers(0, numIds, views);
}

void afSetIndexBuffer(IBOID id)
{
	ID3D12GraphicsCommandList* list = deviceMan.GetCommandList();
	D3D12_RESOURCE_DESC desc = id->GetDesc();
	D3D12_INDEX_BUFFER_VIEW indexBufferView = { id->GetGPUVirtualAddress(), (UINT)desc.Width, AFIndexTypeToDevice };
	list->IASetIndexBuffer(&indexBufferView);
}

void afWriteBuffer(const IBOID id, int size, const void* buf)
{
#ifdef _DEBUG
	D3D12_RESOURCE_DESC desc = id->GetDesc();
	if (size > (int)desc.Width) {
		return;
	}
#endif
	void* p;
	D3D12_RANGE readRange = {};
	afHandleDXError(id->Map(0, &readRange, &p));
	assert(p);
	memcpy(p, buf, size);
	D3D12_RANGE wroteRange = {0, (SIZE_T)size};
	id->Unmap(0, &wroteRange);
}

static ComPtr<ID3D12Resource> afCreateUploadHeap(int size, const void* buf = nullptr)
{
	D3D12_RESOURCE_DESC desc = { D3D12_RESOURCE_DIMENSION_BUFFER, 0, (UINT64)size, 1, 1, 1, DXGI_FORMAT_UNKNOWN, { 1, 0 }, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };
	UBOID o;
	afHandleDXError(deviceMan.GetDevice()->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&o)));
	if (buf)
	{
		afWriteBuffer(o, size, buf);
	}
	return o;
}

VBOID afCreateDynamicVertexBuffer(int size, const void* buf)
{
	return afCreateUploadHeap(size, buf);
}

static ComPtr<ID3D12Resource> afCreateBufferAs(int size, const void* buf, D3D12_RESOURCE_STATES as)
{
	D3D12_RESOURCE_DESC desc = { D3D12_RESOURCE_DIMENSION_BUFFER, 0, (UINT64)size, 1, 1, 1, DXGI_FORMAT_UNKNOWN,{ 1, 0 }, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE };
	VBOID o;
	deviceMan.GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&o));
	if (o) {
		ComPtr<ID3D12Resource> intermediateBuffer = afCreateUploadHeap(size, buf);
		ID3D12GraphicsCommandList* list = deviceMan.GetCommandList();
		list->CopyBufferRegion(o.Get(), 0, intermediateBuffer.Get(), 0, size);
		list->ResourceBarrier(1, ToPtr<D3D12_RESOURCE_BARRIER>({ D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,{ o.Get(), 0, D3D12_RESOURCE_STATE_COPY_DEST, as } }));
		deviceMan.AddIntermediateCommandlistDependentResource(intermediateBuffer);
		deviceMan.AddIntermediateCommandlistDependentResource(o);
	}
	return o;
}

VBOID afCreateVertexBuffer(int size, const void* buf)
{
	return afCreateBufferAs(size, buf, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
}

IBOID afCreateIndexBuffer(int numIndi, const AFIndex* indi)
{
	assert(indi);
	int size = numIndi * sizeof(AFIndex);
	return afCreateBufferAs(size, indi, D3D12_RESOURCE_STATE_INDEX_BUFFER);
}

UBOID afCreateUBO(int size)
{
	return afCreateUploadHeap((size + 0xff) & ~0xff);
}

void afWriteTexture(SRVID tex, const TexDesc& desc, int mipCount, const AFTexSubresourceData datas[])
{
	const int maxSubresources = 100;
	const UINT subResources = mipCount * desc.arraySize;
	assert(subResources <= maxSubresources);
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprints[maxSubresources];
	UINT64 rowSizeInBytes[maxSubresources], uploadSize;
	UINT numRows[maxSubresources];
	D3D12_RESOURCE_BARRIER transitions1[maxSubresources], transitions2[maxSubresources];
	D3D12_RESOURCE_DESC texDesc = tex->GetDesc();
	deviceMan.GetDevice()->GetCopyableFootprints(&texDesc, 0, subResources, 0, footprints, numRows, rowSizeInBytes, &uploadSize);
	ComPtr<ID3D12Resource> uploadBuf = afCreateUploadHeap((int)uploadSize);
	assert(uploadBuf);
	uploadBuf->SetName(__FUNCTIONW__ L" intermediate buffer");
	D3D12_RANGE readRange = {};
	BYTE* ptr;
	afHandleDXError(uploadBuf->Map(0, &readRange, (void**)&ptr));
	assert(ptr);
	for (UINT i = 0; i < subResources; i++)
	{
		transitions1[i] = { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,{ tex.Get(), i, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST } };
		transitions2[i] = { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,{ tex.Get(), i, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE } };
	}
	ID3D12GraphicsCommandList* list = deviceMan.GetCommandList();
	list->ResourceBarrier(subResources, transitions1);
	for (UINT i = 0; i < subResources; i++)
	{
		assert(datas[i].pitch == rowSizeInBytes[i]);
		assert(datas[i].pitch <= footprints[i].Footprint.RowPitch);
		for (UINT row = 0; row < numRows[i]; row++) {
			memcpy(ptr + footprints[i].Offset + footprints[i].Footprint.RowPitch * row, (BYTE*)datas[i].ptr + datas[i].pitch * row, datas[i].pitch);
		}
		D3D12_TEXTURE_COPY_LOCATION uploadBufLocation = { uploadBuf.Get(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, footprints[i] };
		D3D12_TEXTURE_COPY_LOCATION nativeBufLocation = { tex.Get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, i };
		list->CopyTextureRegion(&nativeBufLocation, 0, 0, 0, &uploadBufLocation, nullptr);
	}
	list->ResourceBarrier(subResources, transitions2);
	uploadBuf->Unmap(0, nullptr);
	deviceMan.AddIntermediateCommandlistDependentResource(uploadBuf);
	deviceMan.AddIntermediateCommandlistDependentResource(tex);
}

void afWriteTexture(SRVID id, const TexDesc& desc, const void* buf)
{
	assert(desc.arraySize == 1);
	assert(!desc.isCubeMap);

	const D3D12_RESOURCE_DESC destDesc = id->GetDesc();
	assert(destDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM);
	AFTexSubresourceData data = { buf, (uint32_t)desc.size.x * 4, 0 };
	afWriteTexture(id, desc, 1, &data);
}

void afSetTextureName(SRVID tex, const char* name)
{
	if (tex)
	{
		tex->SetName(SWPrintf(L"%S", name));
	}
}

ComPtr<ID3D12Resource> afCreateDynamicTexture(AFFormat format, const IVec2& size, uint32_t flags)
{
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Format = format;
	textureDesc.Width = size.x;
	textureDesc.Height = size.y;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	bool useClearValue = !!(flags & (AFTF_DSV | AFTF_RTV));
	D3D12_CLEAR_VALUE clearValue = { format };
	D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;
	if (flags & AFTF_SRV)
	{
		resourceState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	}
	if (flags & AFTF_DSV)
	{
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		clearValue.DepthStencil.Depth = 1.0f;
		resourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	}
	if (flags & AFTF_RTV)
	{
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		std::copy_n(clearColor, 4, clearValue.Color);
		resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;	// not |=
	}

	ComPtr<ID3D12Resource> res;
	afHandleDXError(deviceMan.GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc, resourceState, useClearValue ? &clearValue : nullptr, IID_PPV_ARGS(&res)));
	return res;
}

SRVID afCreateTexture2D(AFFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[])
{
	D3D12_RESOURCE_DESC resourceDesc = { D3D12_RESOURCE_DIMENSION_TEXTURE2D, 0, (UINT64)desc.size.x, (UINT)desc.size.y, (UINT16)desc.arraySize, (UINT16)mipCount, format, {1, 0} };
	SRVID tex;
	afHandleDXError(deviceMan.GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&tex)));
	afWriteTexture(tex, desc, mipCount, datas);
	return tex;
}

void afDrawIndexed(int numIndices, int start, int instanceCount)
{
	ID3D12GraphicsCommandList* list = deviceMan.GetCommandList();
	list->DrawIndexedInstanced(numIndices, instanceCount, start, 0, 0);
}

void afDraw(int numVertices, int start, int instanceCount)
{
	ID3D12GraphicsCommandList* list = deviceMan.GetCommandList();
	list->DrawInstanced(numVertices, instanceCount, start, 0);
}

static D3D12_PRIMITIVE_TOPOLOGY_TYPE ToD3D12PrimitiveTopologyType(D3D_PRIMITIVE_TOPOLOGY topology)
{
	switch(topology)
	{
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	case D3D_PRIMITIVE_TOPOLOGY_LINELIST:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}
	assert(0);
	return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}

#include <D3Dcompiler.h>
ComPtr<ID3D12PipelineState> afCreatePSO(const char *shaderName, const InputElement elements[], int numElements, uint32_t flags, ComPtr<ID3D12RootSignature>& rootSignature)
{
	ComPtr<ID3DBlob> vertexShader = afCompileHLSL(shaderName, "VSMain", "vs_5_0");
	ComPtr<ID3DBlob> pixelShader = afCompileHLSL(shaderName, "PSMain", "ps_5_0");
	ComPtr<ID3DBlob> rootSignatureBlob;
	if (S_OK == D3DGetBlobPart(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0, &rootSignatureBlob))
	{
		if (rootSignatureBlob)
		{
			afHandleDXError(deviceMan.GetDevice()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
		}
	}

	static D3D12_RENDER_TARGET_BLEND_DESC solid = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
	};
	static D3D12_RENDER_TARGET_BLEND_DESC alphaBlend = {
		TRUE, FALSE,
		D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.BlendState.RenderTarget[0] = (flags & AFRS_ALPHA_BLEND) ? alphaBlend : solid;
	psoDesc.InputLayout = { elements, (UINT)numElements };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
	psoDesc.RasterizerState = { (flags & AFRS_WIREFRAME) ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID, (flags & AFRS_CULL_CCW) ? D3D12_CULL_MODE_FRONT : (flags & AFRS_CULL_CW) ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE };
	psoDesc.DepthStencilState = { !!(flags & (AFRS_DEPTH_ENABLE | AFRS_DEPTH_CLOSEREQUAL_READONLY)), D3D12_DEPTH_WRITE_MASK_ALL, (flags & AFRS_DEPTH_CLOSEREQUAL_READONLY) ? D3D12_COMPARISON_FUNC_LESS_EQUAL : D3D12_COMPARISON_FUNC_LESS };
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = ToD3D12PrimitiveTopologyType(RenderFlagsToPrimitiveTopology(flags));
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = (flags & AFRS_OFFSCREEN_RENDER_TARGET_R16G16B16A16_FLOAT) ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = AFF_DEPTH_STENCIL;
	psoDesc.SampleDesc.Count = 1;
	ComPtr<ID3D12PipelineState> pso;
	assert(psoDesc.pRootSignature);
	HRESULT hr = deviceMan.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
	if (hr != S_OK)
	{
		aflog("Failed to create PSO with %s", shaderName);
	}
	assert(hr == S_OK);
	return pso;
}

void afWaitFenceValue(ComPtr<ID3D12Fence> fence, UINT64 value)
{
	if (fence->GetCompletedValue() >= value) {
		return;
	}
	HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent);
	fence->SetEventOnCompletion(value, fenceEvent);
	WaitForSingleObject(fenceEvent, INFINITE);
	CloseHandle(fenceEvent);
}

IVec2 afGetTextureSize(SRVID tex)
{
	D3D12_RESOURCE_DESC desc = tex->GetDesc();
	return IVec2((int)desc.Width, (int)desc.Height);
}

static void AssignSRV(D3D12_CPU_DESCRIPTOR_HANDLE ptr , ComPtr<ID3D12Resource> res)
{
	D3D12_RESOURCE_DESC resDesc = res->GetDesc();
	if (resDesc.DepthOrArraySize == 6)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = resDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.TextureCube.MipLevels = resDesc.MipLevels;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0;
		deviceMan.GetDevice()->CreateShaderResourceView(res.Get(), &srvDesc, ptr);
	}
	else
	{
		deviceMan.GetDevice()->CreateShaderResourceView(res.Get(), nullptr, ptr);
	}
}

void afBindTexture(SRVID srv, int rootParameterIndex)
{
	assert(srv);
	AFHeapStackAllocator& heap = deviceMan.GetFrameSRVHeap();
	int descriptorHeapIndex = heap.AssignDescriptorHeap(1);
	AssignSRV(heap.GetCPUAddress(descriptorHeapIndex), srv);
	ID3D12GraphicsCommandList* commandList = deviceMan.GetCommandList();
	commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, heap.GetGPUAddress(descriptorHeapIndex));
}

void afSetVertexBufferFromSystemMemory(const void* buf, int size, int stride)
{
	VBOID vbo = afCreateDynamicVertexBuffer(size, buf);
	afSetVertexBuffer(vbo, stride);
	deviceMan.AddIntermediateCommandlistDependentResource(vbo);
}

AFHeapStackAllocator::~AFHeapStackAllocator()
{
	assert(!heap);
}

void AFHeapStackAllocator::Create(D3D12_DESCRIPTOR_HEAP_TYPE inHeapType, int inMaxDescriptors)
{
	heapType = inHeapType;
	maxDescriptors = inMaxDescriptors;
	deviceMan.GetDevice()->CreateDescriptorHeap(ToPtr<D3D12_DESCRIPTOR_HEAP_DESC>({ heapType, (UINT)maxDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE }), IID_PPV_ARGS(&heap));
}

void AFHeapStackAllocator::Destroy()
{
	heap.Reset();
}

int AFHeapStackAllocator::AssignDescriptorHeap(int numRequired)
{
	if (numAssigned + numRequired > maxDescriptors)
	{
		assert(0);
		return -1;
	}
	int head = numAssigned;
	numAssigned += numRequired;
	return head;
}

D3D12_GPU_DESCRIPTOR_HANDLE AFHeapStackAllocator::GetGPUAddress(int topIndex)
{
	D3D12_GPU_DESCRIPTOR_HANDLE addr = heap->GetGPUDescriptorHandleForHeapStart();
	addr.ptr += topIndex * deviceMan.GetDevice()->GetDescriptorHandleIncrementSize(heapType);
	return addr;
}

D3D12_CPU_DESCRIPTOR_HANDLE AFHeapStackAllocator::GetCPUAddress(int topIndex)
{
	D3D12_CPU_DESCRIPTOR_HANDLE addr = heap->GetCPUDescriptorHandleForHeapStart();
	addr.ptr += topIndex * deviceMan.GetDevice()->GetDescriptorHandleIncrementSize(heapType);
	return addr;
}

void AFRenderTarget::InitForDefaultRenderTarget()
{
	asDefault = true;
}

void AFRenderTarget::Init(IVec2 size, AFFormat colorFormat, AFFormat depthStencilFormat)
{
	texSize = size;
	renderTarget = afCreateDynamicTexture(colorFormat, size, AFTF_RTV | AFTF_SRV);
	currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	if (depthStencilFormat != AFF_INVALID)
	{
		depthStencil = afCreateDynamicTexture(depthStencilFormat, size, AFTF_DSV);
	}
	deviceMan.AddIntermediateCommandlistDependentResource(renderTarget);
	afSetTextureName(renderTarget, __FUNCTION__);
}

void AFRenderTarget::Destroy()
{
	renderTarget.Reset();
}

void afSetRenderTarget(ComPtr<ID3D12Resource> color, ComPtr<ID3D12Resource> depthStencil, uint32_t flags)
{
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = deviceMan.GetDepthStencilView();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = deviceMan.GetRenderTargetView();
	ID3D12GraphicsCommandList* commandList = deviceMan.GetCommandList();
	if (color)
	{
		deviceMan.GetDevice()->CreateRenderTargetView(color.Get(), nullptr, rtvHandle);
		if (flags & AFSRTF_CLEAR_COLOR)
		{
			commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		}
	}
	if (depthStencil)
	{
		deviceMan.GetDevice()->CreateDepthStencilView(depthStencil.Get(), nullptr, dsvHandle);
		if (flags & AFSRTF_CLEAR_DEPTH_STENCIL)
		{
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		}
	}
	IVec2 size = afGetTextureSize(color ? color : depthStencil);
	commandList->OMSetRenderTargets(1, color ? &rtvHandle : nullptr, FALSE, depthStencil ? &dsvHandle : nullptr);
	commandList->RSSetViewports(1, ToPtr<D3D12_VIEWPORT>({ 0.f, 0.f, (float)size.x, (float)size.y, 0.f, 1.f }));
	commandList->RSSetScissorRects(1, ToPtr<D3D12_RECT>({ 0, 0, (LONG)size.x, (LONG)size.y }));
}

void AFRenderTarget::BeginRenderToThis()
{
	if (asDefault)
	{
		ComPtr<ID3D12Resource> rt = deviceMan.GetDefaultRenderTarget();
		afSetRenderTarget(renderTarget, depthStencil, AFSRTF_CLEAR_ALL);
		return;
	}

	ID3D12GraphicsCommandList* commandList = deviceMan.GetCommandList();
	if (currentState != D3D12_RESOURCE_STATE_RENDER_TARGET)
	{
		commandList->ResourceBarrier(1, ToPtr<D3D12_RESOURCE_BARRIER>({ D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,{ renderTarget.Get(), 0, currentState, D3D12_RESOURCE_STATE_RENDER_TARGET }}));
		currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	}

	afSetRenderTarget(renderTarget, depthStencil, AFSRTF_CLEAR_ALL);
}

ComPtr<ID3D12Resource> AFRenderTarget::GetTexture()
{
	if (currentState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
	{
		ID3D12GraphicsCommandList* commandList = deviceMan.GetCommandList();
		commandList->ResourceBarrier(1, ToPtr<D3D12_RESOURCE_BARRIER>({ D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,{ renderTarget.Get(), 0, currentState, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE }}));
		currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	}

	return renderTarget;
}

void afBindBuffer(int size, const void* buf, int rootParameterIndex)
{
	int cbTop = deviceMan.AssignConstantBuffer(buf, size);
	deviceMan.GetCommandList()->SetGraphicsRootConstantBufferView(rootParameterIndex, deviceMan.GetConstantBufferGPUAddress(cbTop));
}

void afBindBuffer(UBOID ubo, int rootParameterIndex)
{
	deviceMan.GetCommandList()->SetGraphicsRootConstantBufferView(rootParameterIndex, ubo->GetGPUVirtualAddress());
}

#endif

void AFRenderStates::Apply() const
{
	ID3D12GraphicsCommandList* list = deviceMan.GetCommandList();
	list->SetPipelineState(pipelineState.Get());
	list->SetGraphicsRootSignature(rootSignature.Get());
	list->IASetPrimitiveTopology(primitiveTopology);
}
