class StockObjects {
	VBOID vboFullScr;
	IBOID iboFullScr;
#if defined(AF_GLES31) || defined(AF_DX11)
	SAMPLERID builtInSamplers[AFST_MAX];
#endif
	void CreateFullScreenVAO();
public:
	void Create();
	void Destroy();
	void ApplyFullScreenVertexBuffer() const;
	const InputElement* GetFullScreenInputElements(int& numElements) const;
#if defined(AF_GLES31) || defined(AF_DX11)
	SAMPLERID GetBuiltInSampler(SamplerType samplerType) { return builtInSamplers[samplerType]; }
#endif
};
extern StockObjects stockObjects;
