#include "stdafx.h"

#ifdef __d3d11_h__

#include <D3DCommon.h>
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")

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

	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&renderTarget);
}

void DeviceMan11::Present()
{
	AF_PROFILE_RANGE(Present);
	pSwapChain->Present(AF_WAIT_VBLANK, 0);
}

void DeviceMan11::Destroy()
{
	if (pImmediateContext)
	{
		pImmediateContext->ClearState();
	}
	pImmediateContext.Reset();
	renderTarget.Reset();
	pSwapChain.Reset();

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

void afBeginRenderToSwapChain()
{
	afSetRenderTarget(deviceMan.GetDefaultRenderTarget(), nullptr, AFSRTF_CLEAR_COLOR);
}

void afEndRenderToSwapChain()
{
	afSetRenderTarget(nullptr, nullptr);
}

void afSetVertexBuffer(AFBufferResource vertexBuffer, int stride_)
{
	ID3D11Buffer* d11Bufs[] = { vertexBuffer.Get() };
	UINT stride = (UINT)stride_;
	UINT offset = 0;
	deviceMan11.GetContext()->IASetVertexBuffers(0, arrayparam(d11Bufs), &stride, &offset);
}

void afSetVertexBuffer(int size, const void* buf, int stride)
{
	AFBufferResource vbo = afCreateDynamicVertexBuffer(size);
	afWriteBuffer(vbo, size, buf);
	afSetVertexBuffer(vbo, stride);
}

void afSetIndexBuffer(AFBufferResource indexBuffer)
{
	deviceMan11.GetContext()->IASetIndexBuffer(indexBuffer.Get(), AFIndexTypeToDevice, 0);
}

static D3D11_BIND_FLAG BufferTypeToBindFlag(AFBufferType bufferType)
{
	switch (bufferType)
	{
	case AFBT_VERTEX:
	case AFBT_VERTEX_CPUWRITE:
		return D3D11_BIND_VERTEX_BUFFER;
	case AFBT_INDEX:
		return D3D11_BIND_INDEX_BUFFER;
	case AFBT_CONSTANT:
	case AFBT_CONSTANT_CPUWRITE:
		return D3D11_BIND_CONSTANT_BUFFER;
	}
	assert(0);
	return D3D11_BIND_VERTEX_BUFFER;
}

ComPtr<ID3D11Buffer> afCreateBuffer(int size, const void* data, AFBufferType bufferType)
{
	ComPtr<ID3D11Buffer> buffer;
	switch (bufferType)
	{
	case AFBT_VERTEX:
	case AFBT_INDEX:
	case AFBT_CONSTANT:
		deviceMan11.GetDevice()->CreateBuffer(ToPtr<D3D11_BUFFER_DESC>({ (UINT)size, D3D11_USAGE_DEFAULT, (UINT)BufferTypeToBindFlag(bufferType) }), ToPtr<D3D11_SUBRESOURCE_DATA>({ data }), &buffer);
		break;
	case AFBT_VERTEX_CPUWRITE:
	case AFBT_CONSTANT_CPUWRITE:
		{
			D3D11_SUBRESOURCE_DATA subResource{ data };
			deviceMan11.GetDevice()->CreateBuffer(ToPtr<D3D11_BUFFER_DESC>({ (UINT)size, D3D11_USAGE_DYNAMIC, (UINT)BufferTypeToBindFlag(bufferType), D3D11_CPU_ACCESS_WRITE }), data ? &subResource : nullptr, &buffer);
			break;
		}
	}
	return buffer;
}

ComPtr<ID3D11Buffer> afCreateIndexBuffer(int numIndi, const AFIndex* indi)
{
	assert(indi);
	int size = numIndi * sizeof(AFIndex);
	return afCreateBuffer(size, indi, AFBT_INDEX);
}

ComPtr<ID3D11Buffer> afCreateVertexBuffer(int size, const void* data)
{
	return afCreateBuffer(size, data, AFBT_VERTEX);
}

AFBufferResource afCreateDynamicVertexBuffer(int size)
{
	return afCreateBuffer(size, nullptr, AFBT_VERTEX_CPUWRITE);
}

AFBufferResource afCreateUBO(int size, const void* buf)
{
	return afCreateBuffer(size, buf, AFBT_CONSTANT_CPUWRITE);
}

ComPtr<ID3D11Texture2D> afCreateTexture2D(AFFormat format, const TexDesc& afDesc, int mipCount, const AFTexSubresourceData datas[])
{
	ComPtr<ID3D11Texture2D> tex;
	afHandleDXError(deviceMan11.GetDevice()->CreateTexture2D(ToPtr<D3D11_TEXTURE2D_DESC>({(UINT)afDesc.size.x, (UINT)afDesc.size.y, (UINT)mipCount, (UINT)afDesc.arraySize, format, {1, 0}, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, afDesc.isCubeMap ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0u}), datas, &tex));
	return tex;
}

ComPtr<ID3D11Texture2D> afCreateDynamicTexture(AFFormat format, const IVec2& size, uint32_t flags)
{
	ComPtr<ID3D11Texture2D> tex;
	CD3D11_TEXTURE2D_DESC desc(format, size.x, size.y, 1, 1, 0);
	if (flags & AFTF_CPU_WRITE)
	{
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	if (flags & AFTF_SRV)
	{
		desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	}
	if (flags & AFTF_DSV)
	{
		desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	}
	if (flags & AFTF_RTV)
	{
		desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	afHandleDXError(deviceMan11.GetDevice()->CreateTexture2D(&desc, nullptr, &tex));
	return tex;
}

SAMPLERID afCreateSampler(SamplerType type)
{
	if (type == AFST_DEPTH_ANISOTROPIC)
	{
		static D3D11_SAMPLER_DESC depthDesc = { D3D11_FILTER_COMPARISON_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_CLAMP, 0.f, 16, D3D11_COMPARISON_LESS, { 1.f, 1.f, 1.f, 1.f }, 1, D3D11_FLOAT32_MAX };
		ComPtr<ID3D11SamplerState> sampler;
		deviceMan11.GetDevice()->CreateSamplerState(&depthDesc, &sampler);
		return sampler;
	}

	D3D11_TEXTURE_ADDRESS_MODE wrap = (type & 0x01) ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP;
	int filter = type >> 1;
	D3D11_SAMPLER_DESC desc = {};
	desc.AddressU = wrap;
	desc.AddressV = wrap;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	desc.MaxAnisotropy = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	switch (filter)
	{
	case 3:	// anisotropic
		desc.Filter = D3D11_FILTER_ANISOTROPIC;
		break;
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

void afBindBuffer(AFBufferResource ubo, UINT slot)
{
	deviceMan11.GetContext()->VSSetConstantBuffers(slot, 1, ubo.GetAddressOf());
	deviceMan11.GetContext()->PSSetConstantBuffers(slot, 1, ubo.GetAddressOf());
	deviceMan11.GetContext()->HSSetConstantBuffers(slot, 1, ubo.GetAddressOf());
	deviceMan11.GetContext()->DSSetConstantBuffers(slot, 1, ubo.GetAddressOf());
}

void afBindBuffer(int size, const void* buf, UINT slot)
{
	AFBufferResource id = afCreateUBO(size, buf);
	afBindBuffer(id, slot);
}

void afBindTexture(SRVID srv, uint32_t slot)
{
	deviceMan11.GetContext()->VSSetShaderResources(slot, 1, srv.GetAddressOf());
	deviceMan11.GetContext()->PSSetShaderResources(slot, 1, srv.GetAddressOf());
	deviceMan11.GetContext()->HSSetShaderResources(slot, 1, srv.GetAddressOf());
	deviceMan11.GetContext()->DSSetShaderResources(slot, 1, srv.GetAddressOf());
}

static D3D11_TEXTURE2D_DESC afGetTexture2DDesc(AFTexRef tex)
{
	if (!tex)
	{
		return D3D11_TEXTURE2D_DESC();
	}
	ComPtr<ID3D11Texture2D> tx;
	tex.As(&tx);
	assert(tx);

	D3D11_TEXTURE2D_DESC desc;
	tx->GetDesc(&desc);
	return desc;
}

SRVID afCreateSRVFromTexture(AFTexRef tex)
{
	if (!tex)
	{
		return ComPtr<ID3D11ShaderResourceView>();
	}
	D3D11_TEXTURE2D_DESC desc = afGetTexture2DDesc(tex);
	ComPtr<ID3D11ShaderResourceView> srv;
	afHandleDXError(deviceMan11.GetDevice()->CreateShaderResourceView(tex.Get(), ToPtr(CD3D11_SHADER_RESOURCE_VIEW_DESC(desc.ArraySize == 6 ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D, afTypelessToSRVFormat(desc.Format), 0, desc.MipLevels)), &srv));
	return srv;
}

ComPtr<ID3D11DepthStencilView> afCreateDSVFromTexture(ComPtr<ID3D11Resource> tex)
{
	ComPtr<ID3D11DepthStencilView> dsv;
	afHandleDXError(deviceMan11.GetDevice()->CreateDepthStencilView(tex.Get(), ToPtr(CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D, afTypelessToDSVFormat(afGetTexture2DDesc(tex).Format))), &dsv));
	return dsv;
}

ComPtr<ID3D11RenderTargetView> afCreateRTVFromTexture(AFTexRef tex, AFFormat formatAs)
{
	formatAs = (formatAs != AFF_INVALID) ? formatAs : afGetTexture2DDesc(tex).Format;
	ComPtr<ID3D11RenderTargetView> rtv;
	afHandleDXError(deviceMan11.GetDevice()->CreateRenderTargetView(tex.Get(), ToPtr(CD3D11_RENDER_TARGET_VIEW_DESC(D3D11_RTV_DIMENSION_TEXTURE2D, formatAs)), &rtv));
	return rtv;
}

void afBindTexture(ComPtr<ID3D11Resource> tex, uint32_t slot)
{
	afBindTexture(afCreateSRVFromTexture(tex), slot);
}

void afBindSamplerToBindingPoint(SAMPLERID sampler, UINT slot)
{
	deviceMan11.GetContext()->VSSetSamplers(slot, 1, sampler.GetAddressOf());
	deviceMan11.GetContext()->PSSetSamplers(slot, 1, sampler.GetAddressOf());
	deviceMan11.GetContext()->DSSetSamplers(slot, 1, sampler.GetAddressOf());
	deviceMan11.GetContext()->HSSetSamplers(slot, 1, sampler.GetAddressOf());
}

void afWriteBuffer(const AFBufferResource p, int size, const void* buf)
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

void afWriteTexture(AFTexRef tex, const TexDesc& desc, const void* buf)
{
	D3D11_MAPPED_SUBRESOURCE m;
	afHandleDXError(deviceMan11.GetContext()->Map(tex.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &m));
	memcpy(m.pData, buf, desc.size.x * desc.size.y * 4);
	deviceMan11.GetContext()->Unmap(tex.Get(), 0);
}

void afDrawIndexed(int numIndices, int start, int instanceCount)
{
	deviceMan11.GetContext()->DrawIndexedInstanced(numIndices, instanceCount, start, 0, 0);
}

void afDraw(int numVertices, int start, int instanceCount)
{
	deviceMan11.GetContext()->DrawInstanced(numVertices, instanceCount, start, 0);
}

void afRasterizerState(uint32_t flags)
{
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
	if (flags & AFRS_WIREFRAME)
	{
		rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	}

	ComPtr<ID3D11RasterizerState> rs;
	deviceMan11.GetDevice()->CreateRasterizerState(&rasterDesc, &rs);
	deviceMan11.GetContext()->RSSetState(rs.Get());
}

void afBlendMode(uint32_t flags)
{
	if (!(flags & AFRS_ALPHA_BLEND))
	{
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
	ComPtr<ID3D11BlendState> bs;
	deviceMan11.GetDevice()->CreateBlendState(&bdesc, &bs);
	FLOAT factor[] = {0, 0, 0, 0};
	deviceMan11.GetContext()->OMSetBlendState(bs.Get(), factor, 0xffffffff);
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

ComPtr<ID3D11Texture2D> afGetTexture2DFromView(ComPtr<ID3D11View> view)
{
	ComPtr<ID3D11Resource> res;
	view->GetResource(&res);
	assert(res);
	ComPtr<ID3D11Texture2D> tex;
	res.As(&tex);
	assert(tex);
	return tex;
}

IVec2 afGetTextureSize(ComPtr<ID3D11Texture2D> tex)
{
	D3D11_TEXTURE2D_DESC desc;
	tex->GetDesc(&desc);
	return IVec2((int)desc.Width, (int)desc.Height);
}

IVec2 afGetTextureSize(ComPtr<ID3D11View> view)
{
	return afGetTextureSize(afGetTexture2DFromView(view));
}

void afSetTextureName(AFTexRef tex, const char* name)
{
	tex->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
}

void afSetRenderTarget(ComPtr<ID3D11Resource> color, ComPtr<ID3D11Resource> depthStencil, uint32_t flags)
{
	D3D11_TEXTURE2D_DESC desc = afGetTexture2DDesc(color ? color : depthStencil);
	ComPtr<ID3D11RenderTargetView> rtv = color ? afCreateRTVFromTexture(color) : nullptr;
	ComPtr<ID3D11DepthStencilView> dsv = depthStencil ? afCreateDSVFromTexture(depthStencil) : nullptr;
	if ((flags & AFSRTF_CLEAR_COLOR) && rtv)
	{
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		deviceMan11.GetContext()->ClearRenderTargetView(rtv.Get(), clearColor);
	}
	if ((flags & AFSRTF_CLEAR_DEPTH_STENCIL) && dsv)
	{
		deviceMan11.GetContext()->ClearDepthStencilView(dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
	ID3D11DeviceContext* context = deviceMan11.GetContext();
	context->OMSetRenderTargets(1, rtv.GetAddressOf(), dsv.Get());
	context->RSSetViewports(1, ToPtr<D3D11_VIEWPORT>({ 0, 0, (float)desc.Width, (float)desc.Height, 0.0f, 1.0f }));
	context->RSSetScissorRects(1, ToPtr<D3D11_RECT>({ 0, 0, (LONG)desc.Width, (LONG)desc.Height }));
}

void AFRenderTarget::Init(IVec2 size, DXGI_FORMAT colorFormat, DXGI_FORMAT depthStencilFormat)
{
	Destroy();
	renderTarget = afCreateDynamicTexture(colorFormat, size, AFTF_RTV | AFTF_SRV);
	switch (depthStencilFormat)
	{
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		depthStencil = afCreateDynamicTexture(depthStencilFormat, size, AFTF_DSV);
		return;
	case DXGI_FORMAT_UNKNOWN:
		return;
	}
	assert(0);
}

void AFRenderTarget::Destroy()
{
	renderTarget.Reset();
	depthStencil.Reset();
}

void AFRenderTarget::BeginRenderToThis()
{
	afSetRenderTarget(renderTarget, depthStencil, AFSRTF_CLEAR_COLOR | AFSRTF_CLEAR_DEPTH_STENCIL);
}

void AFRenderTarget::EndRenderToThis()
{
	afSetRenderTarget(nullptr, nullptr);
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
	if (flags_ & AFRS_PRIMITIVE_PATCHLIST)
	{
		ComPtr<ID3DBlob> hs = afCompileHLSL(shaderName, "HSMain", "hs_5_0");
		ComPtr<ID3DBlob> ds = afCompileHLSL(shaderName, "DSMain", "ds_5_0");
		if (hs)
		{
			afHandleDXError(deviceMan11.GetDevice()->CreateHullShader(hs->GetBufferPointer(), hs->GetBufferSize(), nullptr, &hullShader));
		}
		if (ds)
		{
			afHandleDXError(deviceMan11.GetDevice()->CreateDomainShader(ds->GetBufferPointer(), ds->GetBufferSize(), nullptr, &domainShader));
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
	deviceMan11.GetContext()->HSSetShader(hullShader.Get(), nullptr, 0);
	deviceMan11.GetContext()->DSSetShader(domainShader.Get(), nullptr, 0);
	deviceMan11.GetContext()->IASetPrimitiveTopology(RenderFlagsToPrimitiveTopology(flags));
	afBlendMode(flags);
	afDepthStencilMode(flags);
	afRasterizerState(flags);
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
	hullShader.Reset();
	domainShader.Reset();
}

void afSetSampler(SamplerType type, int slot)
{
	afBindSamplerToBindingPoint(stockObjects.GetBuiltInSampler(type), slot);
}
#endif
