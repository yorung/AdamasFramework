class ShaderMan
{
public:
	typedef unsigned int SMID;
	static const SMID INVALID_SMID = 0;
	SMID Create(const char *name, const struct InputElement elements[], int numElements);
	void Destroy();
	void Apply(SMID id);
private:
	typedef std::map<std::string, SMID> NameToId;
	NameToId nameToId;
};

extern ShaderMan shaderMan;
