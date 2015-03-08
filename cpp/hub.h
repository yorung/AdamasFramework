class Hub
{
public:
	Hub();
	void Init();
	void Update();
	void Destroy();
	void OnTap(float x, float y);
};

extern Hub hub;