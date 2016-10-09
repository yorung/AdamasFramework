#include "stdafx.h"

TexMan texMan;

TexMan::~TexMan()
{
	assert(nameToId.empty());
}

SRVID TexMan::Create(const char *name)
{
	auto it = nameToId.find(name);
	if (it != nameToId.end()) {
		return it->second;
	}
	TexDesc dummy;
	return nameToId[name] = afLoadTexture(name, dummy);
}

SRVID TexMan::CreateWhiteTexture()
{
	const std::string name = "$WHITE";
	auto it = nameToId.find(name);
	if (it != nameToId.end())
	{
		return it->second;
	}
	uint32_t white = 0xffffffff;
	TexDesc desc;
	desc.size = IVec2(1, 1);
	AFTexSubresourceData data = { &white, 4, 4 };
	SRVID tex = afCreateTexture2D(AFF_R8G8B8A8_UNORM, desc, 1, &data);
	afSetTextureName(tex, __FUNCTION__);
	return nameToId[name] = tex;
}

void TexMan::Destroy()
{
	for (auto& it : nameToId) {
		afSafeDeleteTexture(it.second);
	}
	nameToId.clear();
}
