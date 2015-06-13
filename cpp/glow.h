class Glow
{
	ShaderMan::SMID shaderGlowExtraction = 0;
	ShaderMan::SMID shaderGlowCopy = 0;
	ShaderMan::SMID shaderGlowLastPass = 0;
public:
	void Init();
	void Destroy();
	void MakeGlow(AFRenderTarget& target, GLuint srcTex);
};

extern Glow glow;
