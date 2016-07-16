class StockObjects {
	VBOID vboFullScr;
	IBOID iboFullScr;
	VAOID vaoFullScr;
	SAMPLERID builtInSamplers[AFST_MAX];
	void CreateFullScreenVAO();
public:
	void Init();
	void Destroy();
	void ApplyFullScreenVAO() const;
	const InputElement* GetFullScreenInputElements(int& numElements) const;
	SAMPLERID GetBuiltInSampler(SamplerType samplerType) { return builtInSamplers[samplerType]; }
};
extern StockObjects stockObjects;
