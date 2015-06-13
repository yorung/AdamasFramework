class StockObjects {
	VBOID vboFullScr;
	IBOID iboFullScr;
	VAOID vaoFullScr;
	GLuint samplerClamp = 0;
	GLuint samplerRepeat = 0;
	GLuint samplerNoMipmap = 0;
	void CreateFullScreenVAO();
	void CreateSamplers();
public:
	StockObjects();
	void Init();
	void Destroy();
	void ApplyFullScreenVAO() const;
	const std::vector<std::string>* GetFullScreenVertexAttributeLayout() const;
	GLuint GetClampSampler() { return samplerClamp; }
	GLuint GetRepeatSampler() { return samplerRepeat; }
	GLuint GetNoMipmapSampler() { return samplerNoMipmap; }
};
extern StockObjects stockObjects;
