class SkyMan
{
	AFTexRef texRef;
	AFRenderStates renderStates;
public:
	~SkyMan();
	void Create(const char *texFileName, const char* shader);
	void Draw(AFCommandList& cmd);
	void Destroy();
};

extern SkyMan skyMan;
