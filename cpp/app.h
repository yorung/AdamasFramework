class App {
	MeshX* mesh;
public:
	App();
	~App();
	void Init();
	void Update();
	void Draw();
	void OnTap(float x, float y);
	void Destroy();
};

extern App app;

extern std::string g_type;
