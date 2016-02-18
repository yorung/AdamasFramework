class LetterBox
{
	ShaderMan::SMID shader = 0;
	void LazyInit();
public:
	void Draw(AFRenderTarget& target, SRVID srcTex);
	void Destroy();
};

extern LetterBox letterBox;
