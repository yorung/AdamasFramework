class App {
public:
	void Init(int screenW, int screenH);
	void Update(int w, int h, float offset);
	void Draw();
	void OnTap(float x, float y);
	void Destroy();
};

extern App app;
