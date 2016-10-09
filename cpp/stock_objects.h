class StockObjects {
	VBOID vboFullScr;
	IBOID iboFullScr;
#ifndef AF_DX12
	SAMPLERID builtInSamplers[AFST_MAX];
#endif
	void CreateFullScreenVAO();
public:
	void Create();
	void Destroy();
	void ApplyFullScreenVertexBuffer() const;
	const InputElement* GetFullScreenInputElements(int& numElements) const;
#ifndef AF_DX12
	SAMPLERID GetBuiltInSampler(SamplerType samplerType) { return builtInSamplers[samplerType]; }
#endif
};
extern StockObjects stockObjects;
