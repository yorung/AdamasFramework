class TexMan
{
public:
	TexMan();
	~TexMan();
	int Create(const char *name);
	int CreateWhiteTexture();
	SRVID IndexToTexture(int index);
	void Destroy();
private:
	std::map<std::string, int> nameToIndex;
	std::vector<SRVID> srvids;
};

extern TexMan texMan;
