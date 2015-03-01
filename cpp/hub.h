class Hub
{
public:
	Hub();
	void Init();
	void Update(int screenW, int screenH, float offset);
	void Destroy();
	void OnTap(float x, float y);
};

extern Hub hub;