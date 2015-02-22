class Hub
{
public:
	Hub();
	void Init(int screenW, int screenH);
	void Update(int screenW, int screenH, float offset);
	void Destroy();
	void CreateRipple(float x, float y);
};

extern Hub hub;