class TexMan
{
public:
	~TexMan();
	SRVID Create(const char *name);
	SRVID CreateWhiteTexture();
	void Destroy();
	const TexDesc* GetTexDesc(SRVID id);
private:
	void StoreTexState(SRVID id, const TexDesc& v);
	typedef std::map<std::string, SRVID> NameToId;
	NameToId nameToId;
	typedef std::vector<TexDesc> TexDescs;
	TexDescs texDescs;
};

extern TexMan texMan;
