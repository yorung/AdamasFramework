class ShaderMan
{
public:
	typedef GLuint SMID;
	static const SMID INVALID_SMID = 0;
private:
	struct Effect
	{
		BlendMode blendMode;
		DepthStencilMode depthStencilMode;
		Effect() { memset(this, 0, sizeof(*this)); }
	};
	typedef std::map<std::string, SMID> NameToId;
	NameToId nameToId;
	std::map<SMID, Effect> effects;
public:
	SMID Create(const char *name, const InputElement elements[], int numElements, BlendMode blendMode, DepthStencilMode depthStencilMode);
	void Destroy();
	void Apply(SMID id);
};

extern ShaderMan shaderMan;

