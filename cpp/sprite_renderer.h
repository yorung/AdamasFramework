struct SpriteCommand
{
	Mat matW;
	Vec4 quad;
	uint32_t color;
	SRVID tex;
};

typedef std::vector<SpriteCommand> SpriteCommands;

class SpriteRenderer
{
	ShaderMan::SMID shaderId;
	SAMPLERID sampler;
	VBOID vbo;
	IBOID ibo;
	VAOID vao;
	UBOID ubo;
public:
	SpriteRenderer();
	~SpriteRenderer();
	void Destroy();
	void Init();
	void Draw(const SpriteCommands& sprites);
};

extern SpriteRenderer spriteRenderer;
