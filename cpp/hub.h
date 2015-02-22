class Hub
{
public:
	Hub();
	void Init(int screenW, int screenH);
	void Update(float aspect, float offset);
	void Destroy();
	void CreateRipple(float x, float y);
};

extern Hub hub;