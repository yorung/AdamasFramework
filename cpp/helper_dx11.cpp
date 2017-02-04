#include "stdafx.h"

#ifdef __d3d11_h__

#include <D3DCommon.h>
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")
#pragma warning(disable:4238) // nonstandard extension used : class rvalue used as lvalue

DeviceMan11 deviceMan11;

DeviceMan11::~DeviceMan11()
{
	Destroy();
}

void DeviceMan11::Create(HWND hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = rc.right - rc.left;
	sd.BufferDesc.Height = rc.bottom - rc.top;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3 };
	D3D_FEATURE_LEVEL supportLevel;
#ifdef _DEBUG
	UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT flags = 0;
#endif
	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, dimof(featureLevels), D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, &supportLevel, &pImmediateContext);

	ID3D11Texture2D* pBackBuffer = nullptr;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
	SAFE_RELEASE(pBackBuffer);

	ID3D11Texture2D* pDepthStencil;
	pDevice->CreateTexture2D(&CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_D24_UNORM_S8_UINT, sd.BufferDesc.Width, sd.BufferDesc.Height, 1, 0, D3D11_BIND_DEPTH_STENCIL), nullptr, &pDepthStencil);
	pDevice->CreateDepthStencilView(pDepthStencil, &CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT), &pDepthStencilView);
	SAFE_RELEASE(pDepthStencil);

	pImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);
}

void DeviceMan11::Present()
{
	pSwapChain->Present(AF_WAIT_VBLANK, 0);
}

void DeviceMan11::Destroy()
{
	if (pImmediateContext)
	{
		pImmediateContext->ClearState();
	}
	SAFE_RELEASE(pImmediateContext);
	SAFE_RELEASE(pRenderTargetView);
	SAFE_RELEASE(pDepthStencilView);
	SAFE_RELEASE(pSwapChain);

	if (pDevice)
	{
		int cnt;
		cnt = pDevice->Release();
		if (cnt)
		{
			ComPtr<ID3D11Debug> dbg;
			pDevice->QueryInterface(IID_PPV_ARGS(&dbg));
			dbg->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
			dbg.Reset();
			MessageBoxA(GetActiveWindow(), SPrintf("%d DX11 interface leak detected.", cnt), "DX11 leaks", MB_OK);
		}
		pDevice = nullptr;
	}
}

void afSetVertexBuffer(VBOID vertexBuffer, int stride_)
{
	ID3D11Buffer* d11Bufs[] = { vertexBuffer.Get() };
	UINT stride = (UINT)stride_;
	UINT offset = 0;
	deviceMan11.GetContext()->IASetVertexBuffers(0, arrayparam(d11Bufs), &stride, &offset);
}

void afSetVertexBuffer(int size, const void* buf, int stride)
{
	VBOID vbo = afCreateDynamicVertexBuffer(size);
	afWriteBuffer(vbo, size, buf);
	afSetVertexBuffer(vbo, stride);
}

void afSetIndexBuffer(IBOID indexBuffer)
{
	deviceMan11.GetContext()->IASetIndexBuffer(indexBuffer.Get(), AFIndexTypeToDevice, 0);
}

IBOID afCreateIndexBuffer(int numIndi, const AFIndex* indi)
{
	IBOID indexBuffer;
	D3D11_SUBRESOURCE_DATA subresData = { indi, 0, 0 };
	deviceMan11.GetDevice()->CreateBuffer(&CD3D11_BUFFER_DESC(numIndi * sizeof(AFIndex), D3D11_BIND_INDEX_BUFFER), &subresData, &indexBuffer);
	return indexBuffer;
}

VBOID afCreateVertexBuffer(int size, const void* data)
{
	VBOID vbo;
	D3D11_SUBRESOURCE_DATA subresData = { data, 0, 0 };
	deviceMan11.GetDevice()->CreateBuffer(&CD3D11_BUFFER_DESC(size, D3D11_BIND_VERTEX_BUFFER), &subresData, &vbo);
	return vbo;
}

VBOID afCreateDynamicVertexBuffer(int size)
{
	VBOID vbo;
	deviceMan11.GetDevice()->CreateBuffer(&CD3D11_BUFFER_DESC(size, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), nullptr, &vbo);
	return vbo;
}

UBOID afCreateUBO(int size, const void* buf)
{
	UBOID ubo;
	D3D11_SUBRESOURCE_DATA subresData = { buf, 0, 0 };
	deviceMan11.GetDevice()->CreateBuffer(&CD3D11_BUFFER_DESC(size, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), buf ? &subresData : nullptr, &ubo);
	return ubo;
}

SRVID afCreateTexture2D(AFFormat format, const TexDesc& afDesc, int mipCount, const AFTexSubresourceData datas[])
{
	ComPtr<ID3D11Texture2D> tex;
	CD3D11_TEXTURE2D_DESC desc(format, afDesc.size.x, afDesc.size.y, afDesc.arraySize, mipCount, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, 1, 0, afDesc.isCubeMap ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0);
	deviceMan11.GetDevice()->CreateTexture2D(&desc, datas, &tex);
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(afDesc.isCubeMap ? D3D_SRV_DIMENSION_TEXTURECUBE : D3D_SRV_DIMENSION_TEXTURE2D, desc.Format);
	ComPtr<ID3D11ShaderResourceView> srv;
	deviceMan11.GetDevice()->CreateShaderResourceView(tex.Get(), &srvDesc, &srv);
	return srv;
}

SRVID afCreateDynamicTexture(AFFormat format, const IVec2& size)
{
	CD3D11_TEXTURE2D_DESC desc(format, size.x, size.y, 1, 1, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	ComPtr<ID3D11Texture2D> tex;
	ComPtr<ID3D11ShaderResourceView> srv;
	deviceMan11.GetDevice()->CreateTexture2D(&desc, nullptr, &tex);
	deviceMan11.GetDevice()->CreateShaderResourceView(tex.Get(), nullptr, &srv);
	return srv;
}

SAMPLERID afCreateSampler(SamplerType type)
{
	D3D11_TEXTURE_ADDRESS_MODE wrap = (type & 0x01) ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP;
	int filter = type >> 1;
	D3D11_SAMPLER_DESC desc = {};
	desc.AddressU = wrap;
	desc.AddressV = wrap;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MaxAnisotropy = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	switch (filter) {
	case 2:	// mipmap
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case 1:	// linear
		desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case 0:	// point
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	}
	ComPtr<ID3D11SamplerState> sampler;
	deviceMan11.GetDevice()->CreateSamplerState(&desc, &sampler);
	return sampler;
}

void afBindBuffer(UBOID ubo, UINT slot)
{
	deviceMan11.GetContext()->VSSetConstantBuffers(slot, 1, ubo.GetAddressOf());
	deviceMan11.GetContext()->PSSetConstantBuffers(slot, 1, ubo.GetAddressOf());
}

void afBindBuffer(int size, const void* buf, UINT slot)
{
	UBOID id = afCreateUBO(size, buf);
	afBindBuffer(id, slot);
}

void afBindTexture(SRVID srv, UINT slot)
{
	deviceMan11.GetContext()->PSSetShaderResources(slot, 1, srv.GetAddressOf());
}

void afBindSamplerToBindingPoint(SAMPLERID sampler, UINT slot)
{
	deviceMan11.GetContext()->PSSetSamplers(slot, 1, sampler.GetAddressOf());
}

void afWriteBuffer(const IBOID p, int size, const void* buf)
{
	assert(p);
#ifdef _DEBUG
	D3D11_BUFFER_DESC desc;
	p->GetDesc(&desc);
	if (size > (int)desc.ByteWidth) {
		return;
	}
#endif
	D3D11_MAPPED_SUBRESOURCE m;
	afHandleDXError(deviceMan11.GetContext()->Map(p.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m));
	memcpy(m.pData, buf, size);
	deviceMan11.GetContext()->Unmap(p.Get(), 0);
}

void afWriteTexture(SRVID srv, const TexDesc& desc, const void* buf)
{
	ComPtr<ID3D11Resource> res;
	srv->GetResource(&res);
	assert(res);
	ComPtr<ID3D11Texture2D> tx;
	res.As(&tx);
	assert(tx);

//	D3D11_TEXTURE2D_DESC desc;
//	tx->GetDesc(&desc);

	D3D11_MAPPED_SUBRESOURCE m;
	afHandleDXError(deviceMan11.GetContext()->Map(tx.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m));
	memcpy(m.pData, buf, desc.size.x * desc.size.y * 4);
	deviceMan11.GetContext()->Unmap(tx.Get(), 0);
}

void afDrawIndexed(int numIndices, int start, int instanceCount)
{
	deviceMan11.GetContext()->DrawIndexedInstanced(numIndices, instanceCount, start, 0, 0);
}

void afDraw(int numVertices, int start, int instanceCount)
{
	deviceMan11.GetContext()->DrawInstanced(numVertices, instanceCount, start, 0);
}

void afCullMode(uint32_t flags)
{
	ID3D11RasterizerState* rs;
	CD3D11_RASTERIZER_DESC rasterDesc(D3D11_DEFAULT);
	if (flags & AFRS_CULL_CCW)
	{
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.FrontCounterClockwise = TRUE;
	}
	else if (flags & AFRS_CULL_CW)
	{
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.FrontCounterClockwise = FALSE;
	}
	else
	{
		rasterDesc.CullMode = D3D11_CULL_NONE;
	}
	deviceMan11.GetDevice()->CreateRasterizerState(&rasterDesc, &rs);
	deviceMan11.GetContext()->RSSetState(rs);
	SAFE_RELEASE(rs);
}

void afBlendMode(uint32_t flags)
{
	if (!(flags & AFRS_ALPHA_BLEND)) {
		deviceMan11.GetContext()->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		return;
	}

	CD3D11_BLEND_DESC bdesc(D3D11_DEFAULT);
	bdesc.RenderTarget[0].BlendEnable = TRUE;
	bdesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bdesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bdesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bdesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bdesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bdesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bdesc.RenderTarget[0].RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
	ID3D11BlendState* bs;
	deviceMan11.GetDevice()->CreateBlendState(&bdesc, &bs);
	FLOAT factor[] = {0, 0, 0, 0};
	deviceMan11.GetContext()->OMSetBlendState(bs, factor, 0xffffffff);
	SAFE_RELEASE(bs);
}

void afDepthStencilMode(uint32_t flags)
{
	ComPtr<ID3D11DepthStencilState> ds;
	D3D11_DEPTH_STENCIL_DESC desc = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());
	if (flags & AFRS_DEPTH_ENABLE)
	{
		// same as default
	}
	else if (flags & AFRS_DEPTH_CLOSEREQUAL_READONLY)
	{
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	}
	else
	{
		desc.DepthEnable = FALSE;
	}
	deviceMan11.GetDevice()->CreateDepthStencilState(&desc, &ds);
	deviceMan11.GetContext()->OMSetDepthStencilState(ds.Get(), 1);
}

IVec2 afGetTextureSize(ComPtr<ID3D11View> view)
{
	ComPtr<ID3D11Resource> res;
	view->GetResource(&res);
	assert(res);
	ComPtr<ID3D11Texture2D> tx;
	res.As(&tx);
	assert(tx);

	D3D11_TEXTURE2D_DESC desc;
	tx->GetDesc(&desc);
	return IVec2((int)desc.Width, (int)desc.Height);
}

void afSetTextureName(SRVID tex, const char* name)
{
	ComPtr<ID3D11Resource> res;
	tex->GetResource(&res);
	assert(res);
	if (res)
	{
		res->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
	}
}

void AFRenderTarget::InitForDefaultRenderTarget()
{
	Destroy();
	renderTargetView = deviceMan11.GetDefaultRenderTarget();
	depthStencilView = deviceMan11.GetDefaultDepthStencil();
	texSize = afGetTextureSize(renderTargetView);
}

void AFRenderTarget::Init(IVec2 size, DXGI_FORMAT colorFormat, DXGI_FORMAT depthStencilFormat)
{
	(void)depthStencilFormat;

	Destroy();
	texSize = size;
	CD3D11_TEXTURE2D_DESC tDesc(colorFormat, size.x, size.y, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE/* | D3D11_BIND_UNORDERED_ACCESS*/);
	ID3D11Texture2D* tex;
	HRESULT hr = deviceMan11.GetDevice()->CreateTexture2D(&tDesc, NULL, &tex);
	CD3D11_RENDER_TARGET_VIEW_DESC rDesc(D3D11_RTV_DIMENSION_TEXTURE2D, tDesc.Format);
	hr = deviceMan11.GetDevice()->CreateRenderTargetView(tex, &rDesc, &renderTargetView);
	CD3D11_SHADER_RESOURCE_VIEW_DESC sDesc(D3D11_SRV_DIMENSION_TEXTURE2D, tDesc.Format);
	hr = deviceMan11.GetDevice()->CreateShaderResourceView(tex, &sDesc, &shaderResourceView);
//	CD3D11_UNORDERED_ACCESS_VIEW_DESC uDesc(D3D11_UAV_DIMENSION_TEXTURE2D, tDesc.Format);
//	hr = deviceMan11.GetDevice()->CreateUnorderedAccessView(tex, &uDesc, &unorderedAccessView);
	SAFE_RELEASE(tex);

	switch (depthStencilFormat)
	{
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		depthStencilView = deviceMan11.GetDefaultDepthStencil();
		return;
	case DXGI_FORMAT_UNKNOWN:
		return;
	}
	assert(0);
}

void AFRenderTarget::Destroy()
{
	renderTargetView.Reset();
	shaderResourceView.Reset();
	depthStencilView.Reset();
}

void AFRenderTarget::BeginRenderToThis()
{
	ID3D11RenderTargetView* rtv = renderTargetView.Get();
	ID3D11DeviceContext* context = deviceMan11.GetContext();
	context->OMSetRenderTargets(1, &rtv, depthStencilView.Get());
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView(rtv, clearColor);
	if (depthStencilView)
	{
		context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	D3D11_VIEWPORT vp = { 0, 0, (float)texSize.x, (float)texSize.y, 0.0f, 1.0f };
	context->RSSetViewports(1, &vp);
	D3D11_RECT rc = { 0, 0, texSize.x, texSize.y };
	context->RSSetScissorRects(1, &rc);
}

void AFRenderStates::Create(const char* shaderName, int numInputElements, const InputElement* inputElements, uint32_t flags_, int numSamplerTypes_, const SamplerType samplerTypes_[])
{
	ComPtr<ID3DBlob> vs = afCompileHLSL(shaderName, "VSMain", "vs_5_0");
	ComPtr<ID3DBlob> ps = afCompileHLSL(shaderName, "PSMain", "ps_5_0");
	HRESULT hr = S_OK;
	if (ps)
	{
		hr = deviceMan11.GetDevice()->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &pixelShader);
		assert(!hr);
	}
	if (vs)
	{
		hr = deviceMan11.GetDevice()->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &vertexShader);
		assert(!hr);
		if (inputElements && numInputElements > 0)
		{
			hr = deviceMan11.GetDevice()->CreateInputLayout(inputElements, numInputElements, vs->GetBufferPointer(), vs->GetBufferSize(), &inputLayout);
			assert(!hr);
		}
	}
	numSamplerTypes = numSamplerTypes_;
	samplerTypes = samplerTypes_;
	flags = flags_;
}

void AFRenderStates::Apply() const
{
	deviceMan11.GetContext()->IASetInputLayout(inputLayout.Get());
	deviceMan11.GetContext()->VSSetShader(vertexShader.Get(), nullptr, 0);
	deviceMan11.GetContext()->PSSetShader(pixelShader.Get(), nullptr, 0);
	deviceMan11.GetContext()->IASetPrimitiveTopology(RenderFlagsToPrimitiveTopology(flags));
	afBlendMode(flags);
	afDepthStencilMode(flags);
	afCullMode(flags);
	for (int i = 0; i < numSamplerTypes; i++)
	{
		afSetSampler(samplerTypes[i], i);
	}
}

void AFRenderStates::Destroy()
{
	inputLayout.Reset();
	vertexShader.Reset();
	pixelShader.Reset();
}

void afSetSampler(SamplerType type, int slot)
{
	afBindSamplerToBindingPoint(stockObjects.GetBuiltInSampler(type), slot);
}
#endif
