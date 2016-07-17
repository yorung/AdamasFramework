class ShaderMan
{
	typedef std::map<std::string, SMID> NameToId;
	NameToId nameToId;
public:
	SMID Create(const char *name, const InputElement elements[], int numElements);
	void Destroy();
	void Apply(SMID id);
};

extern ShaderMan shaderMan;

