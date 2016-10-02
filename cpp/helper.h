#define dimof(x) (sizeof(x) / sizeof(x[0]))
#define arrayparam(x) dimof(x), x

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

struct TexDesc {
	IVec2 size;
	int arraySize = 1;
	bool isCubeMap = false;
};

enum RenderStateFlags : uint32_t
{
	AFRS_NONE = 0,
	AFRS_CULL_CCW = 0x1,
	AFRS_CULL_CW = 0x2,
	AFRS_ALPHA_BLEND = 0x4,
	AFRS_DEPTH_ENABLE = 0x8,
	AFRS_DEPTH_CLOSEREQUAL_READONLY = 0x10,
	AFRS_PRIMITIVE_TRIANGLESTRIP = 0x0,	// default
	AFRS_PRIMITIVE_TRIANGLELIST = 0x20,
	AFRS_PRIMITIVE_LINELIST = 0x40,
};

enum SamplerType {
	AFST_POINT_WRAP,
	AFST_POINT_CLAMP,
	AFST_LINEAR_WRAP,
	AFST_LINEAR_CLAMP,
	AFST_MIPMAP_WRAP,
	AFST_MIPMAP_CLAMP,
	AFST_MAX
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

void afVerify(bool ok);
