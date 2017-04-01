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

template <class T> inline void afSafeDelete(T& p)
{
	delete p;
	p = nullptr;
}

template <class T> class TToPtr
{
	T Ref;
public:
	TToPtr(T InRef) : Ref(InRef) {}
	operator T*() { return &Ref; }
};

template <class T> TToPtr<T> ToPtr(T InRef)
{
	return TToPtr<T>(InRef);
}

struct CharSignature
{
	wchar_t code;
	int fontSize;
	inline int GetOrder() const { return (code << 8) | fontSize; }
	bool operator < (const CharSignature& r) const { return GetOrder() < r.GetOrder(); }
	bool operator == (const CharSignature& r) const { return GetOrder() == r.GetOrder(); }
};

struct CharDesc
{
	Vec2 srcWidth;
	Vec2 distDelta;
	float step;
};

void MakeFontBitmap(const char* fontName, const CharSignature& code, class DIB& dib, CharDesc& desc);

void _afVerify(const char* file, const char* func, int line, const char* command, bool result);
#define afVerify(command) do{ _afVerify(__FILE__, __FUNCTION__, __LINE__, #command, !!(command)); } while(0)
