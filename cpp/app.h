class App
{
	AFRenderTarget appRenderTarget;
	AFRenderStates copyPSO;
	MeshMan::MMID meshId;
	void Draw(ViewDesc& view);
public:
	App();
	void Create();
	void LoadMesh(const char* fileName);
	void Update();
	void Destroy();
};

extern App app;

extern std::string g_type;
