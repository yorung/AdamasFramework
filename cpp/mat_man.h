struct Material
{
	Material() { memset(this, 0, sizeof(*this)); }
	Vec4 faceColor;
	Vec3 specular;
	float power;
	Vec3 emissive;
	TexMan::TMID tmid;
	bool operator==(const Material& r) const;
};

typedef int MMID;
static int INVALID_MMID = 0;

class MatMan
{
	std::vector<Material> m_mats;
public:
	MatMan();
	MMID Create(const Material& mat);
	void Destroy();
	const Material* Get(MMID id);
};

extern MatMan matMan;
