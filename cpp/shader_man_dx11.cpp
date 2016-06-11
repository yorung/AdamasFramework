#include "stdafx.h"

#ifdef __d3d11_h__

ShaderMan11 shaderMan;

static ComPtr<ID3DBlob> CompileShader(const char* name, const char* entryPoint, const char* target)
{
	char path[MAX_PATH];
	sprintf_s(path, sizeof(path), "hlsl/%s.hlsl", name);
#ifdef _DEBUG
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#endif
	ComPtr<ID3DBlob> blob, err;
	WCHAR wname[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, path, -1, wname, dimof(wname));
	D3DCompileFromFile(wname, nullptr, nullptr, entryPoint, target, flags, 0, &blob, &err);
	if (err) {
		MessageBoxA(nullptr, (const char*)err->GetBufferPointer(), name, MB_OK | MB_ICONERROR);
	}
	return blob;
}

ShaderMan11::ShaderMan11()
{
	m_effects.push_back(Effect());	// make ID 0 invalid
}

ShaderMan11::~ShaderMan11()
{
	Destroy();
}

ShaderMan11::SMID ShaderMan11::Create(const char *name, const D3D11_INPUT_ELEMENT_DESC elements[], int numElements)
{
	auto it = m_nameToId.find(name);
	if (it != m_nameToId.end())	{
		return it->second;
	}

	ComPtr<ID3DBlob> vs = CompileShader(name, "mainVS", "vs_5_0");
	ComPtr<ID3DBlob> ps = CompileShader(name, "mainPS", "ps_5_0");
	Effect effect = {};
	HRESULT hr = deviceMan11.GetDevice()->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &effect.pVertexShader);
	hr = deviceMan11.GetDevice()->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &effect.pPixelShader);

	if (elements) {
		hr = deviceMan11.GetDevice()->CreateInputLayout(elements, numElements, vs->GetBufferPointer(), vs->GetBufferSize(), &effect.pInputLayout);
	//	assert(!hr);
	}

	effect.elements = elements;
	effect.numElements = numElements;
	m_effects.push_back(effect);
	return m_nameToId[name] = m_effects.size() - 1;
}

void ShaderMan11::Destroy()
{
	for (auto it = m_effects.begin(); it != m_effects.end(); it++)
	{
		SAFE_RELEASE(it->pInputLayout);
		SAFE_RELEASE(it->pVertexShader);
		SAFE_RELEASE(it->pPixelShader);
	}
	m_effects.clear();
	m_nameToId.clear();
	m_effects.push_back(Effect());	// make ID 0 invalid
}

void ShaderMan11::Reload()
{
	std::vector<Effect> effs = m_effects;
	std::vector<std::string> names;

	for (SMID i = 0; i < (SMID)m_effects.size(); i++) {
		auto it = std::find_if(m_nameToId.begin(), m_nameToId.end(), [i](std::pair<std::string, SMID> v) { return v.second == i; } );
		assert(it != m_nameToId.end());
		names.push_back(it->first);
	}
	Destroy();
	for (int i = 0; i < (int)names.size(); i++) {
		auto& ef = effs[i];
		Create(names[i].c_str(), ef.elements, ef.numElements, ef.blendMode, ef.depthStencilMode, ef.cullMode);
	}
}

void ShaderMan11::Apply(SMID id)
{
	if (id >= 0 && id < (SMID)m_effects.size())
	{
		Effect& it = m_effects[id];
		deviceMan11.GetContext()->IASetInputLayout(it.pInputLayout);
		deviceMan11.GetContext()->VSSetShader(it.pVertexShader, nullptr, 0);
		deviceMan11.GetContext()->PSSetShader(it.pPixelShader, nullptr, 0);
	}
}

FakeVAO::FakeVAO(int numBuffers, const VBOID vbos_[], const int strides_[], const UINT offsets_[], IBOID ibo_)
{
	ibo = ibo_;
	vbos.resize(numBuffers);
	d3dBuffers.resize(numBuffers);
	strides.resize(numBuffers);
	offsets.resize(numBuffers);
	for (int i = 0; i < numBuffers; i++) {
		vbos[i] = vbos_[i];
		d3dBuffers[i] = vbos[i].Get();
		strides[i] = (UINT)strides_[i];
		offsets[i] = offsets_ ? offsets_[i] : 0;
	}
}

void FakeVAO::Apply()
{
	deviceMan11.GetContext()->IASetVertexBuffers(0, vbos.size(), &d3dBuffers[0], &strides[0], &offsets[0]);
	deviceMan11.GetContext()->IASetIndexBuffer(ibo.Get(), AFIndexTypeToDevice, 0);
}

#endif
