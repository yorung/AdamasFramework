class TexMan
{
public:
	~TexMan();
	typedef GLuint TMID;
	static const TMID INVALID_TMID = 0;
	TMID Create(const char *name);
	TMID CreateWhiteTexture();
	void Destroy();
	const TexDesc* GetTexDesc(TMID id);
private:
	void StoreTexState(TMID id, const TexDesc& v);
	typedef std::map<std::string, TMID> NameToId;
	NameToId nameToId;
	typedef std::vector<TexDesc> TexDescs;
	TexDescs texDescs;
};

extern TexMan texMan;
