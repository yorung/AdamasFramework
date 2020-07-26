#include "stdafx.h"
#include "mesh_man.h"
#include "mesh_x.h"

MeshMan meshMan;

MeshMan::MeshMan()
{
	Destroy();
}

MeshMan::MMID MeshMan::Create(const char *name)
{
	auto it = m_nameToId.find(name);
	if (it != m_nameToId.end())
	{
		return it->second;
	}

	Mesh *mesh = nullptr;

	mesh = new MeshX(name);
	m_meshes.push_back(mesh);
	return m_nameToId[name] = (int)m_meshes.size() - 1;
}

void MeshMan::Destroy()
{
	std::for_each(m_meshes.begin(), m_meshes.end(), [] (Mesh* p) { delete p; });
	m_meshes.clear();
	m_meshes.push_back(nullptr);	// make 0 invalid ID
	m_nameToId.clear();
}

Mesh* MeshMan::Get(MMID id)
{
	if (id >= 0 && id < (MMID)m_meshes.size())
	{
		return m_meshes[id];
	}
	return nullptr;
}
