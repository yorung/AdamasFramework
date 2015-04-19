class MeshMan
{
public:
	typedef int MMID;
	static const MMID INVALID_MMID = 0;
	std::map<std::string, MMID> m_nameToId;
	std::vector<Mesh*> m_meshes;
public:
	MeshMan();
	MMID Create(const char *name);
	void Destroy();
	Mesh* Get(MMID id);
};

extern MeshMan meshMan;
