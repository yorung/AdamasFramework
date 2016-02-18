class Glow
{
	ShaderMan::SMID shaderGlowExtraction = 0;
	ShaderMan::SMID shaderGlowCopy = 0;
	ShaderMan::SMID shaderGlowLastPass = 0;
	void LazyInit();
public:
	void Destroy();
	void MakeGlow(AFRenderTarget& target, SRVID srcTex);
};

extern Glow glow;
