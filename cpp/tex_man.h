struct TexDesc {
	ivec2 size;
	int arraySize = 1;
};

class TexMan
{
public:
	typedef GLuint TMID;
	static const TMID INVALID_TMID = 0;
	TMID Create(const char *name);
	TMID CreateWhiteTexture();
	TMID CreateDynamicTexture(const char* name, int w, int h);
	void Destroy();
	void Write(TMID id, const void* buf);
	const TexDesc* GetTexDesc(TMID id);
private:
	void StoreTexState(TMID id, const TexDesc& v);
	typedef std::map<std::string, TMID> NameToId;
	NameToId nameToId;
	typedef std::vector<TexDesc> TexDescs;
	TexDescs texDescs;
};

extern TexMan texMan;
