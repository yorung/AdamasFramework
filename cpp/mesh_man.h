#pragma once

class Mesh;

class MeshMan
{
public:
	typedef int MMID;
	static const MMID INVALID_MMID = 0;
	std::vector<Mesh*> m_meshes;
	MeshMan();
	MMID Create(const char *name);
	void Destroy();
	Mesh* Get(MMID id);
private:
	std::map<std::string, MMID> m_nameToId;
};

extern MeshMan meshMan;
