class StockObjects {
	VBOID vboFullScr;
	IBOID iboFullScr;
	VAOID vaoFullScr;
#ifndef AF_DX12
	SAMPLERID builtInSamplers[AFST_MAX];
#endif
	void CreateFullScreenVAO();
public:
	void Init();
	void Destroy();
	void ApplyFullScreenVAO() const;
	const InputElement* GetFullScreenInputElements(int& numElements) const;
#ifndef AF_DX12
	SAMPLERID GetBuiltInSampler(SamplerType samplerType) { return builtInSamplers[samplerType]; }
#endif
};
extern StockObjects stockObjects;
