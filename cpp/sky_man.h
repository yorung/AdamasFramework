class SkyMan
{
	TexMan::TMID texId = TexMan::INVALID_TMID;
	ShaderMan::SMID shaderId = ShaderMan::INVALID_SMID;
	UBOID uboId;
public:
	~SkyMan();
	void Create(const char *texFileName, const char* shader);
	void Draw();
	void Destroy();
};

extern SkyMan skyMan;
