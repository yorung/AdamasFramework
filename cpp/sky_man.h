class SkyMan
{
	SRVID texId;
	TexDesc texDesc;
	ShaderMan::SMID shaderId = ShaderMan::INVALID_SMID;
	AFRenderStates renderStates;
	SAMPLERID sampler;
public:
	~SkyMan();
	void Create(const char *texFileName, const char* shader);
	void Draw();
	void Destroy();
};

extern SkyMan skyMan;
