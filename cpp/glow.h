#ifndef AF_VULKAN
class Glow
{
	AFRenderStates renderStateGlowExtraction;
	AFRenderStates renderStateGlowCopy;
	AFRenderStates renderStateGlowLastPass;
	void LazyInit();
public:
	void Destroy();
	void MakeGlow(AFRenderTarget& target, SRVID srcTex);
};

extern Glow glow;
#endif
