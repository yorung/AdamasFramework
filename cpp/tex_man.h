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
	ivec2 GetSize(TMID id);
	void StoreTexState(TMID id, const ivec2& v2);
private:
	typedef std::map<std::string, TMID> NameToId;
	NameToId nameToId;
	typedef std::vector<ivec2> TexStates;
	TexStates texStates;
};

extern TexMan texMan;
