class StockObjects {
	VBOID vboFullScr;
	IBOID iboFullScr;
	VAOID vaoFullScr;
	SAMPLERID samplerClamp;
	SAMPLERID samplerRepeat;
	SAMPLERID samplerNoMipmap;
	void CreateFullScreenVAO();
	void CreateSamplers();
public:
	void Init();
	void Destroy();
	void ApplyFullScreenVAO() const;
	const InputElement* GetFullScreenInputElements(int& numElements) const;
	SAMPLERID GetClampSampler() { return samplerClamp; }
	SAMPLERID GetRepeatSampler() { return samplerRepeat; }
	SAMPLERID GetNoMipmapSampler() { return samplerNoMipmap; }
};
extern StockObjects stockObjects;
