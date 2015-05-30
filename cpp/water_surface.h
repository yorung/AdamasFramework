class WaterSurface
{
	struct UniformBuffer {
		Vec2 mousePos;
		float mouseDown;
		float padding;
		float elapsedTime;
		float wrappedTime;
		Vec2 heightMapSize;
	};

	Mat matProj, matView;
	ShaderMan::SMID shaderHeightMap;
	ShaderMan::SMID shaderNormalMap;
	ShaderMan::SMID shaderWaterLastPass;
	ShaderMan::SMID shaderFullScr;
	double elapsedTime;
	double lastTime;
	double nextTime;
	VAOID vaoEmpty;
	GLuint samplerClamp;
	GLuint samplerRepeat;
	GLuint samplerNoMipmap;
	void UpdateTime();
	void UpdateHeightMap(const UniformBuffer&);
	void UpdateNormalMap(const UniformBuffer&);
public:
	WaterSurface();
	~WaterSurface();
	void Destroy();
	void Init();
	void Update();
	void Draw();
};

extern WaterSurface waterSurface;
