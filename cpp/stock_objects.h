class StockObjects {
	VBOID vboFullScr;
	IBOID iboFullScr;
	VAOID vaoFullScr;
public:
	StockObjects();
	void Init();
	void Destroy();
	void ApplyFullScreenVAO();
};
extern StockObjects stockObjects;
