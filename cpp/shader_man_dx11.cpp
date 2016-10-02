#include "stdafx.h"

#ifdef __d3d11_h__

ShaderMan11 shaderMan;

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

	ComPtr<ID3DBlob> vs = afCompileHLSL(name, "VSMain", "vs_5_0");
	ComPtr<ID3DBlob> ps = afCompileHLSL(name, "PSMain", "ps_5_0");
	Effect effect = {};
	HRESULT hr = S_OK;
	if (ps) {
		hr = deviceMan11.GetDevice()->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &effect.pPixelShader);
		assert(!hr);
	}
	if (vs) {
		hr = deviceMan11.GetDevice()->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &effect.pVertexShader);
		assert(!hr);
		if (elements) {
			hr = deviceMan11.GetDevice()->CreateInputLayout(elements, numElements, vs->GetBufferPointer(), vs->GetBufferSize(), &effect.pInputLayout);
			assert(!hr);
		}
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
		Create(names[i].c_str(), ef.elements, ef.numElements);
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

#endif
