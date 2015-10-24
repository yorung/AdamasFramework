class SkyMan
{
public:
	enum MappingType {
		CUBEMAP,
		PHOTOSPHERE,
	};
private:
	TexMan::TMID texId;
	ShaderMan::SMID shaderId;
	MappingType mappingType;
	UBOID uboId;
public:
	SkyMan();
	~SkyMan();
	void Create(const char *strCubeMapFile, MappingType type);
	void Draw();
	void Destroy();
};

extern SkyMan skyMan;
