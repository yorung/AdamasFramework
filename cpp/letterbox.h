#ifndef AF_VULKAN
class LetterBox
{
	AFRenderStates renderStates;
	void LazyInit();
public:
	void Draw(AFCommandList& cmd, AFRenderTarget& target, SRVID srcTex);
	void Destroy();
};

extern LetterBox letterBox;
#endif
