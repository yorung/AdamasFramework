class TexMan
{
public:
	TexMan();
	~TexMan();
	int Create(const char *name);
	int CreateWhiteTexture();
	AFTexRef IndexToTexture(int index);
	void Destroy();
private:
	std::map<std::string, int> nameToIndex;
	std::vector<AFTexRef> srvids;
};

extern TexMan texMan;
