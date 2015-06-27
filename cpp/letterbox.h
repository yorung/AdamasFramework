class LetterBox
{
	GLuint shader = 0;
	void LazyInit();
public:
	void Draw(AFRenderTarget& target, GLuint srcTex);
	void Destroy();
};

extern LetterBox letterBox;
