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
	ShaderMan::SMID shaderHeightMap = 0;
	ShaderMan::SMID shaderNormalMap = 0;
	ShaderMan::SMID shaderWaterLastPass = 0;
	ShaderMan::SMID shaderFullScr = 0;
	double elapsedTime;
	double lastTime;
	double nextTime;
	VAOID vaoEmpty;
	GLuint samplerClamp;
	GLuint samplerRepeat;
	GLuint samplerNoMipmap;
	bool lastMouseDown;
	void UpdateTime();
	void UpdateHeightMap(const UniformBuffer&);
	void UpdateNormalMap(const UniformBuffer&);
	void RenderWater(const UniformBuffer&);
	void MakeGlow(const UniformBuffer&);
	void PostProcess();
public:
	WaterSurface();
	~WaterSurface();
	void Destroy();
	void Init();
	void Update();
	void Draw();
};

extern WaterSurface waterSurface;
