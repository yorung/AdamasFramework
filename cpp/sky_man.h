class SkyMan
{
private:
	TexMan::TMID texId;
	ShaderMan::SMID shaderId;
	UBOID uboId;
public:
	SkyMan();
	~SkyMan();
	void Create(const char *strCubeMapFile, const char* shader);
	void Draw();
	void Destroy();
};

extern SkyMan skyMan;
