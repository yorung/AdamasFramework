class ShaderMan11
{
	struct Effect
	{
		ID3D11InputLayout* pInputLayout;
		ID3D11VertexShader* pVertexShader;
		ID3D11PixelShader* pPixelShader;
		const D3D11_INPUT_ELEMENT_DESC *elements;
		int numElements;
	};
	std::map<std::string, SMID> m_nameToId;
	std::vector<Effect> m_effects;
public:
	ShaderMan11();
	~ShaderMan11();
	SMID Create(const char *name, const D3D11_INPUT_ELEMENT_DESC elements[], int numElements);
	void Destroy();
	void Reload();
	void Apply(SMID id);
};

extern ShaderMan11 shaderMan;
typedef ShaderMan11 ShaderMan;
