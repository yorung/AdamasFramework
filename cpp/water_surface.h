struct WaterVert
{
	Vec3 pos;
	Vec3 normal;
};

struct WaterRipple
{
	WaterRipple()
	{
		generatedTime = -10000;
	}
	double generatedTime;
	Vec2 centerPos;
};

class WaterSurface
{
	Mat matProj, matView;
	ShaderMan::SMID shaderId;
	ShaderMan::SMID shaderIdFullScr;
	ShaderMan::SMID heightMapGenShaderId;
	int lines;
	double elapsedTime;
	double lastTime;
	double nextTime;
	VAOID vaoEmpty;
	int nIndi;
	GLuint samplerClamp;
	GLuint samplerRepeat;
	GLuint samplerNoMipmap;
	int storedW, storedH;
	void UpdateTime();
public:
	WaterSurface();
	~WaterSurface();
	void Destroy();
	void Init();
	void Update();
	void Draw();
};

extern WaterSurface waterSurface;
