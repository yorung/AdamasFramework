#include "stdafx.h"

#ifdef __d3d12_h__

static const D3D12_HEAP_PROPERTIES defaultHeapProperties = { D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };
static const D3D12_HEAP_PROPERTIES uploadHeapProperties = { D3D12_HEAP_TYPE_UPLOAD, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 1, 1 };
static const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static const D3D12_RESOURCE_STATES TEXTURE_STATE = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

void afSafeDeleteTexture(SRVID& p)
{
	deviceMan.AddIntermediateCommandlistDependentResource(p);	// prevent 'still in use on GPU' error
	p.Reset();
}

void afSafeDeleteBuffer(ComPtr<ID3D12Resource>& p)
{
	deviceMan.AddIntermediateCommandlistDependentResource(p);	// prevent 'still in use on GPU' error
	p.Reset();
}

void afTransition(ID3D12GraphicsCommandList* cmd, ComPtr<ID3D12Resource> res, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
{
	cmd->ResourceBarrier(1, ToPtr<D3D12_RESOURCE_BARRIER>({ D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,{ res.Get(), 0, from, to } }));
}

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
	if (o)
	{
		ComPtr<ID3D12Resource> intermediateBuffer = afCreateUploadHeap(size, buf);
		ID3D12GraphicsCommandList* list = deviceMan.GetCommandList();
		list->CopyBufferRegion(o.Get(), 0, intermediateBuffer.Get(), 0, size);
		afTransition(list, o, D3D12_RESOURCE_STATE_COPY_DEST, as);
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

UBOID afCreateUBO(int size, const void* buf)
{
	UBOID ubo = afCreateUploadHeap((size + 0xff) & ~0xff);
	if (buf)
	{
		afWriteBuffer(ubo, size, buf);
	}
	return ubo;
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
		transitions1[i] = { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,{ tex.Get(), i, TEXTURE_STATE, D3D12_RESOURCE_STATE_COPY_DEST } };
		transitions2[i] = { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE,{ tex.Get(), i, D3D12_RESOURCE_STATE_COPY_DEST, TEXTURE_STATE } };
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
	D3D12_CLEAR_VALUE clearValue = { afTypelessToDSVFormat(format) };
	D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;
	if (flags & AFTF_SRV)
	{
		resourceState = TEXTURE_STATE;
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
	afHandleDXError(deviceMan.GetDevice()->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, TEXTURE_STATE, nullptr, IID_PPV_ARGS(&tex)));
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
	ComPtr<ID3DBlob> vs, ps, hs, ds;
	vs = afCompileHLSL(shaderName, "VSMain", "vs_5_0");
	ps = afCompileHLSL(shaderName, "PSMain", "ps_5_0");
	if (flags & AFRS_PRIMITIVE_PATCHLIST)
	{
		hs = afCompileHLSL(shaderName, "HSMain", "hs_5_0");
		ds = afCompileHLSL(shaderName, "DSMain", "ds_5_0");
	}
	ComPtr<ID3DBlob> rootSignatureBlob;
	if (S_OK == D3DGetBlobPart(vs->GetBufferPointer(), vs->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0, &rootSignatureBlob))
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
	psoDesc.VS = { reinterpret_cast<UINT8*>(vs->GetBufferPointer()), vs->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(ps->GetBufferPointer()), ps->GetBufferSize() };
	psoDesc.HS = hs ? D3D12_SHADER_BYTECODE({ reinterpret_cast<UINT8*>(hs->GetBufferPointer()), hs->GetBufferSize() }) : D3D12_SHADER_BYTECODE({});
	psoDesc.DS = ds ? D3D12_SHADER_BYTECODE({ reinterpret_cast<UINT8*>(ds->GetBufferPointer()), ds->GetBufferSize() }) : D3D12_SHADER_BYTECODE({});
	psoDesc.RasterizerState = { (flags & AFRS_WIREFRAME) ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID, (flags & AFRS_CULL_CCW) ? D3D12_CULL_MODE_FRONT : (flags & AFRS_CULL_CW) ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE };
	psoDesc.DepthStencilState = { !!(flags & (AFRS_DEPTH_ENABLE | AFRS_DEPTH_CLOSEREQUAL_READONLY)), D3D12_DEPTH_WRITE_MASK_ALL, (flags & AFRS_DEPTH_CLOSEREQUAL_READONLY) ? D3D12_COMPARISON_FUNC_LESS_EQUAL : D3D12_COMPARISON_FUNC_LESS };
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = ToD3D12PrimitiveTopologyType(RenderFlagsToPrimitiveTopology(flags));
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = afRenderFlagsToRTVFormat(flags);
	psoDesc.DSVFormat = afRenderFlagsToDSVFormat(flags);
	psoDesc.SampleDesc.Count = 1;
	ComPtr<ID3D12PipelineState> pso;
	assert(psoDesc.pRootSignature);
	HRESULT hr = deviceMan.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
	if (hr != S_OK)
	{
		aflog("Failed to create PSO with %s", shaderName);
	}
	assert(hr == S_OK);
	pso->SetName(SWPrintf(L"%S", shaderName));
	return pso;
}

IVec2 afGetTextureSize(SRVID tex)
{
	D3D12_RESOURCE_DESC desc = tex->GetDesc();
	return IVec2((int)desc.Width, (int)desc.Height);
}

static void AssignSRV(D3D12_CPU_DESCRIPTOR_HANDLE ptr , ComPtr<ID3D12Resource> res)
{
	D3D12_RESOURCE_DESC resDesc = res->GetDesc();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	if (resDesc.DepthOrArraySize == 6)
	{
		srvDesc.Format = resDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.TextureCube.MipLevels = resDesc.MipLevels;
	}
	else
	{
		srvDesc.Format = afTypelessToSRVFormat(resDesc.Format);
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = resDesc.MipLevels;
	}
	deviceMan.GetDevice()->CreateShaderResourceView(res.Get(), &srvDesc, ptr);
}

void afBindTextures(int numResources, ComPtr<ID3D12Resource> resources[], int rootParameterIndex)
{
	AFHeapRingAllocator& heap = deviceMan.GetRingSRVHeap();
	int descriptorHeapIndex = heap.AssignDescriptorHeap(numResources);
	for (int i = 0; i < numResources; i++)
	{
		AssignSRV(heap.GetCPUAddress(descriptorHeapIndex + i), resources[i]);
	}
	ID3D12GraphicsCommandList* commandList = deviceMan.GetCommandList();
	commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, heap.GetGPUAddress(descriptorHeapIndex));
}

void afSetVertexBufferFromSystemMemory(const void* buf, int size, int stride)
{
	VBOID vbo = afCreateDynamicVertexBuffer(size, buf);
	afSetVertexBuffer(vbo, stride);
	deviceMan.AddIntermediateCommandlistDependentResource(vbo);
}

AFHeapAllocator::~AFHeapAllocator()
{
	assert(!heap);
}

void AFHeapAllocator::Destroy()
{
	heap.Reset();
}

D3D12_GPU_DESCRIPTOR_HANDLE AFHeapAllocator::GetGPUAddress(int index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE addr = heap->GetGPUDescriptorHandleForHeapStart();
	addr.ptr += (topIndex + index) * deviceMan.GetDevice()->GetDescriptorHandleIncrementSize(heapType);
	return addr;
}

D3D12_CPU_DESCRIPTOR_HANDLE AFHeapAllocator::GetCPUAddress(int index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE addr = heap->GetCPUDescriptorHandleForHeapStart();
	addr.ptr += (topIndex + index) * deviceMan.GetDevice()->GetDescriptorHandleIncrementSize(heapType);
	return addr;
}

void AFHeapAllocator::Create(ComPtr<ID3D12DescriptorHeap> inHeap, int inTopIndex, int inMaxDescriptors)
{
	heap = inHeap;
	topIndex = inTopIndex;
	maxDescriptors = inMaxDescriptors;
}

void AFHeapRingAllocator::Create(ComPtr<ID3D12DescriptorHeap> inHeap, int inTopIndex, int inMaxDescriptors)
{
	AFHeapAllocator::Create(inHeap, inTopIndex, inMaxDescriptors);
	curPos = 0;
	fenceToGuard.resize(maxDescriptors);
	std::fill(fenceToGuard.data(), fenceToGuard.data() + maxDescriptors, 0);
}

int AFHeapRingAllocator::AssignDescriptorHeap(int numRequired)
{
	assert(numRequired > 0);
	assert(numRequired <= maxDescriptors);
	if (curPos + numRequired > maxDescriptors)	// loop
	{
		curPos = 0;
	}

	AFCommandQueue& queue = deviceMan.GetCommandQueue();
	const UINT64 fenceValueToSignal = queue.GetFenceValue();
	const UINT64 fenceValueToWait = fenceToGuard[curPos + numRequired - 1];
	assert(fenceValueToWait < fenceValueToSignal);	// otherwise the fenceValueToWait never completed
	queue.WaitFenceValue(fenceValueToWait);
	std::fill(&fenceToGuard[curPos], &fenceToGuard[curPos] + numRequired, fenceValueToSignal);

	int head = curPos;
	curPos += numRequired;
	return head;
}

void AFHeapStackAllocator::Create(ComPtr<ID3D12DescriptorHeap> inHeap, int inTopIndex, int inMaxDescriptors)
{
	AFHeapAllocator::Create(inHeap, inTopIndex, inMaxDescriptors);
	curPos = 0;
}

int AFHeapStackAllocator::AssignDescriptorHeap(int numRequired)
{
	assert(numRequired > 0);
	assert(curPos + numRequired <= maxDescriptors);
	int head = curPos;
	curPos += numRequired;
	return head;
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
	afSafeDeleteTexture(renderTarget);
	afSafeDeleteTexture(depthStencil);
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
		const D3D12_RESOURCE_DESC resDesc = depthStencil->GetDesc();
		const D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = { afTypelessToDSVFormat(resDesc.Format), D3D12_DSV_DIMENSION_TEXTURE2D };
		deviceMan.GetDevice()->CreateDepthStencilView(depthStencil.Get(), &dsvDesc, dsvHandle);
		if (flags & AFSRTF_CLEAR_DEPTH_STENCIL)
		{
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		}
	}
	IVec2 size = afGetTextureSize(color ? color : depthStencil);
	commandList->OMSetRenderTargets(color ? 1 : 0, &rtvHandle, FALSE, depthStencil ? &dsvHandle : nullptr);
	commandList->RSSetViewports(1, ToPtr<D3D12_VIEWPORT>({ 0.f, 0.f, (float)size.x, (float)size.y, 0.f, 1.f }));
	commandList->RSSetScissorRects(1, ToPtr<D3D12_RECT>({ 0, 0, (LONG)size.x, (LONG)size.y }));
}

void AFRenderTarget::BeginRenderToThis()
{
	if (asDefault)
	{
		afSetRenderTarget(deviceMan.GetDefaultRenderTarget(), nullptr, AFSRTF_CLEAR_ALL);
		return;
	}

	ID3D12GraphicsCommandList* commandList = deviceMan.GetCommandList();
	if (currentState != D3D12_RESOURCE_STATE_RENDER_TARGET)
	{
		afTransition(commandList, renderTarget, currentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
		currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	}

	afSetRenderTarget(renderTarget, depthStencil, AFSRTF_CLEAR_ALL);
}

ComPtr<ID3D12Resource> AFRenderTarget::GetTexture()
{
	if (currentState != TEXTURE_STATE)
	{
		ID3D12GraphicsCommandList* commandList = deviceMan.GetCommandList();
		afTransition(commandList, renderTarget, currentState, TEXTURE_STATE);
		currentState = TEXTURE_STATE;
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

void AFRenderStates::Destroy()
{
	deviceMan.Flush();		// prevent OBJECT_DELETED_WHILE_STILL_IN_USE
	pipelineState.Reset();
	rootSignature.Reset();
}

void AFCommandQueue::Create()
{
	ComPtr<ID3D12Device> device = deviceMan.GetDevice();
	afHandleDXError(device->CreateCommandQueue(ToPtr<D3D12_COMMAND_QUEUE_DESC>({}), IID_PPV_ARGS(&commandQueue)));
	afHandleDXError(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
}

void AFCommandQueue::Destroy()
{
	commandQueue.Reset();
	fence.Reset();
	fenceValue = 1;
	lastCompletedValue = 0;
}

UINT64 AFCommandQueue::InsertFence()
{
	UINT64 lastValue = fenceValue++;
	commandQueue->Signal(fence.Get(), lastValue);
	return lastValue;
}

void AFCommandQueue::WaitFenceValue(UINT64 waitFor)
{
	if (lastCompletedValue >= waitFor)
	{
		return;
	}
	lastCompletedValue = fence->GetCompletedValue();
	if (lastCompletedValue >= waitFor)
	{
		return;
	}
	HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent);
	fence->SetEventOnCompletion(waitFor, fenceEvent);
	WaitForSingleObject(fenceEvent, INFINITE);
	CloseHandle(fenceEvent);
}
