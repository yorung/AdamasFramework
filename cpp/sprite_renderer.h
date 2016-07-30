class SpriteCommand
{
public:
	Mat matW;
	Vec4 quad;
	uint32_t color = 0;
	SRVID tex;
	SpriteCommand() {}
	SpriteCommand(const SpriteCommand& r) { *this = r; }
	const SpriteCommand& operator=(const SpriteCommand& r) {
		matW = r.matW;
		quad = r.quad;
		color = r.color;
		tex = r.tex;
		return *this;
	}
};

typedef std::vector<SpriteCommand> SpriteCommands;

class SpriteRenderer
{
	AFRenderStates renderStates;
	AFDynamicQuadListVertexBuffer quadListVertexBuffer;
public:
	~SpriteRenderer();
	void Destroy();
	void Init();
	void Draw(const SpriteCommands& sprites);
};

extern SpriteRenderer spriteRenderer;
