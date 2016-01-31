#include "stdafx.h"

TexMan texMan;

TexMan::~TexMan()
{
	assert(nameToId.empty());
}

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
	uint32_t white = 0xffffffff;
	TMID id = nameToId[name] = afCreateTexture2D(AFDT_R8G8B8A8_UNORM, ivec2(1, 1), &white);
	TexDesc desc;
	desc.size.x = desc.size.y = 1;
	StoreTexState(id, desc);
	return id;
}

void TexMan::Destroy()
{
	for (NameToId::iterator it = nameToId.begin(); it != nameToId.end(); ++it)
	{
		afSafeDeleteTexture(it->second);
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
