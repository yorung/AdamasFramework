#define dimof(x) (sizeof(x) / sizeof(x[0]))

void *LoadFile(const char *fileName, int* size = nullptr);
bool SaveFile(const char *fileName, const uint8_t* buf, int size);
double GetTime();
float Random();
void GoMyDir();
void Toast(const char *text);
void PlayBgm(const char *fileName);
const char* StrMessageBox(const char* txt, const char* type);
void ClearMenu();
void AddMenu(const char *name, const char *cmd);
void PostCommand(const char* cmdString);
bool LoadImageViaGdiPlus(const char* name, IVec2& size, std::vector<uint32_t>& col);
SRVID LoadTextureViaOS(const char* name, IVec2& size);

template <class T> inline void SAFE_DELETE(T& p)
{
	delete p;
	p = nullptr;
}

template <class T> inline void SAFE_RELEASE(T& p)
{
	if (p) {
		p->Release();
		p = nullptr;
	}
}

IBOID afCreateTiledPlaneIBO(int numTiles, int* numIndies = nullptr);
VBOID afCreateTiledPlaneVBO(int numTiles);
IBOID afCreateQuadListIndexBuffer(int numQuads);

struct TexDesc {
	IVec2 size;
	int arraySize = 1;
	bool isCubeMap = false;
};

SRVID afLoadTexture(const char* name, TexDesc& desc);

enum CullMode {
	CM_DISABLE,
	CM_CW,
	CM_CCW,
};
void afCullMode(CullMode cullMode);
enum BlendMode {
	BM_NONE,
	BM_ALPHA,
};
void afBlendMode(BlendMode mode);
enum DepthStencilMode {
	DSM_DISABLE,
	DSM_DEPTH_ENABLE,
	DSM_DEPTH_CLOSEREQUAL_READONLY,
};
void afDepthStencilMode(DepthStencilMode mode);

class AFRenderStates {
	BlendMode blendMode = BM_NONE;
	DepthStencilMode depthStencilMode = DSM_DISABLE;
	CullMode cullMode = CM_DISABLE;
public:
	void Init(BlendMode blendMode_, DepthStencilMode depthStencilMode_, CullMode cullMode_) {
		blendMode = blendMode_;
		depthStencilMode = depthStencilMode_;
		cullMode = cullMode_;
	}
	void Apply() const {
		afBlendMode(blendMode);
		afDepthStencilMode(depthStencilMode);
		afCullMode(cullMode);
	}
};

struct CharSignature {
	wchar_t code;
	int fontSize;
	inline int GetOrder() const { return (code << 8) | fontSize; }
	bool operator < (const CharSignature& r) const { return GetOrder() < r.GetOrder(); }
	bool operator == (const CharSignature& r) const { return GetOrder() == r.GetOrder(); }
};
struct CharDesc {
	Vec2 srcWidth;
	Vec2 distDelta;
	float step;
};
void MakeFontBitmap(const char* fontName, const CharSignature& code, class DIB& dib, CharDesc& desc);

enum SamplerType {
	AFST_POINT_WRAP,
	AFST_POINT_CLAMP,
	AFST_LINEAR_WRAP,
	AFST_LINEAR_CLAMP,
	AFST_MIPMAP_WRAP,
	AFST_MIPMAP_CLAMP,
};

SAMPLERID afCreateSampler(SamplerType type);

void afVerify(bool ok);
