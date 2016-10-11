#include "stdafx.h"

TexMan texMan;

TexMan::TexMan()
{
	Destroy();
}

TexMan::~TexMan()
{
	assert(nameToIndex.empty());
}

int TexMan::Create(const char *name)
{
	auto it = nameToIndex.find(name);
	if (it != nameToIndex.end())
	{
		return it->second;
	}
	srvids.push_back(afLoadTexture(name, TexDesc()));
	return nameToIndex[name] = srvids.size() - 1;
}

int TexMan::CreateWhiteTexture()
{
	const std::string name = "$WHITE";
	uint32_t white = 0xffffffff;
	TexDesc desc;
	desc.size = IVec2(1, 1);
	AFTexSubresourceData data = { &white, 4, 4 };

	auto it = nameToIndex.find(name);
	if (it != nameToIndex.end())
	{
		return it->second;
	}
	SRVID tex = afCreateTexture2D(AFF_R8G8B8A8_UNORM, desc, 1, &data);
	afSetTextureName(tex, __FUNCTION__);
	srvids.push_back(tex);
	return nameToIndex[name] = srvids.size() - 1;
}

SRVID TexMan::IndexToTexture(int index)
{
	if (index < 0 || index >= (int)srvids.size())
	{
		return SRVID();
	}
	return srvids[index];
}

void TexMan::Destroy()
{
	for (auto& it : srvids)
	{
		afSafeDeleteTexture(it);
	}
	srvids.clear();
	srvids.push_back(SRVID());	// invalidate index 0
	nameToIndex.clear();
}
