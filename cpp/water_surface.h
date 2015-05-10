class WaterSurface
{
	Mat matProj, matView;
	ShaderMan::SMID shaderId;
	ShaderMan::SMID shaderIdFullScr;
	ShaderMan::SMID heightMapGenShaderId;
	double elapsedTime;
	double lastTime;
	double nextTime;
	VAOID vaoEmpty;
	GLuint samplerClamp;
	GLuint samplerRepeat;
	GLuint samplerNoMipmap;
	VBOID vboTiledPlane;
	IBOID iboTiledPlane;
	VAOID vaoWater;
	int numIndi;
	void InitBuffers();
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
