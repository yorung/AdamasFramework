class StockObjects {
	VBOID vboFullScr;
	IBOID iboFullScr;
	VAOID vaoFullScr;
public:
	StockObjects();
	void Init();
	void Destroy();
	void ApplyFullScreenVAO() const;
	const std::vector<std::string>* GetFullScreenVertexAttributeLayout() const;
};
extern StockObjects stockObjects;
