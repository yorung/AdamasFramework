class LetterBox
{
	ShaderMan::SMID shader = 0;
	AFRenderStates renderStates;
	void LazyInit();
public:
	void Draw(AFRenderTarget& target, SRVID srcTex);
	void Destroy();
};

extern LetterBox letterBox;
