class App {
	MeshMan::MMID meshId;
public:
	App();
	void Create();
	void LoadMesh(const char* fileName);
	void Update();
	void Draw();
	void Destroy();
};

extern App app;

extern std::string g_type;
