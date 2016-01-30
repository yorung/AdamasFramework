#include "stdafx.h"

TexMan texMan;

TexMan::TMID TexMan::Create(const char *name)
{
	auto it = nameToId.find(name);
	if (it != nameToId.end())
	{
		return it->second;
	}
	TexDesc desc;
	TMID id = nameToId[name] = afLoadTexture(name, desc);
	StoreTexState(id, desc);
	return id;
}

TexMan::TMID TexMan::CreateWhiteTexture()
{
	const std::string name = "$WHITE";
	auto it = nameToId.find(name);
	if (it != nameToId.end())
	{
		return it->second;
	}
	TMID id = nameToId[name] = afCreateWhiteTexture();
	TexDesc desc;
	desc.size.x = desc.size.y = 1;
	StoreTexState(id, desc);
	return id;
}

void TexMan::Destroy()
{
	for (NameToId::iterator it = nameToId.begin(); it != nameToId.end(); ++it)
	{
		GLuint id[1] = { it->second };
		glDeleteTextures(1, id);
	}
	nameToId.clear();
}

void TexMan::StoreTexState(TMID id, const TexDesc& v)
{
	if (id >= texDescs.size() ) {
		texDescs.resize(id + 1);
	}
	texDescs[id] = v;
}

const TexDesc* TexMan::GetTexDesc(TMID id)
{
	if (id >= texDescs.size()) {
		return nullptr;
	}
	return &texDescs[id];
}
