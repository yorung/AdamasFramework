class ShaderMan
{
public:
	typedef GLuint SMID;
	static const SMID INVALID_SMID = 0;
private:
	typedef std::map<std::string, SMID> NameToId;
	NameToId nameToId;
public:
	SMID Create(const char *name, const std::vector<std::string>* attributeIndex = nullptr);
	void Destroy();
	void Reload();
	void Apply(SMID id);
};

extern ShaderMan shaderMan;

