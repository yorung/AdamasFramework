class App {
	MeshX* mesh;
public:
	App();
	void Init();
	void LoadMesh(const char* fileName);
	void Update();
	void Draw();
	void OnTap(float x, float y);
	void Destroy();
};

extern App app;

extern std::string g_type;
