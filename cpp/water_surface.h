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
	int lines;
	void UpdateVert(std::vector<WaterVert>& vert);
	void UpdateRipple();
	WaterRipple ripples[2];
	int ripplesNext;
	double elapsedTime;
	double lastTime;
	double nextTime;
	VBOID vbo, vboFullScr;
	IBOID ibo;
	VAOID vao, vaoFullScr;
	int nIndi;
	GLuint samplerClamp;
	GLuint samplerRepeat;
	GLuint samplerNoMipmap;
	int storedW, storedH;
public:
	WaterSurface();
	~WaterSurface();
	void Destroy();
	void Init();
	void Update();
	void Draw();
	void CreateRipple(Vec2 pos);
};

extern WaterSurface waterSurface;
