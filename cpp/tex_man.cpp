#include "stdafx.h"

TexMan texMan;

struct DDSHeader {
	uint32_t h3[3];
	int h, w;
	uint32_t h2[2];
	int mipCnt;
	uint32_t h13[13];
	uint32_t fourcc, bitsPerPixel, rMask, gMask, bMask, aMask, caps1, caps2;
	bool IsCubeMap() const { return caps2 == 0xFE00; }
	int GetArraySize() const { return IsCubeMap() ? 6 : 1; }
	int GetMipCnt() const { return std::max(mipCnt, 1); }
};

static void bitScanForward(uint32_t* result, uint32_t mask)
{
	//	DWORD dwd;
	//	_BitScanForward(&dwd, mask);
	//	*result = dwd;

	for (int i = 0; i < 32; i++) {
		if (mask & (1 << i)) {
			*result = i;
			return;
		}
	}
	*result = 0;
}

static void ArrangeRawDDS(void* img, int size)
{
	const DDSHeader* hdr = (DDSHeader*)img;
	uint32_t rShift, gShift, bShift, aShift;
	bitScanForward(&rShift, hdr->rMask);
	bitScanForward(&gShift, hdr->gMask);
	bitScanForward(&bShift, hdr->bMask);
	bitScanForward(&aShift, hdr->aMask);
	std::for_each((uint32_t*)img + 128 / 4, (uint32_t*)img + size / 4, [&](uint32_t& im) {
		im = ((hdr->aMask & im) >> aShift << 24) + ((hdr->bMask & im) >> bShift << 16) + ((hdr->gMask & im) >> gShift << 8) + ((hdr->rMask & im) >> rShift);
		if (hdr->aMask == 0) {
			im |= 0xff000000;
		}
	});
}

static SRVID LoadDDSTexture(const char* name, TexDesc& texSize)
{
	int size;
	void* img = LoadFile(name, &size);
	if (!img) {
		aflog("LoadDDSTexture failed! %s", name);
		return 0;
	}
	const DDSHeader* hdr = (DDSHeader*)img;

	AFDTFormat format = AFDT_INVALID;
	int (*pitchCalcurator)(int, int) = nullptr;
	switch (hdr->fourcc) {
	case 0x31545844: //'1TXD':
		format = AFDT_BC1_UNORM;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 8; };
		break;
	case 0x33545844: //'3TXD':
		format = AFDT_BC2_UNORM;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 16; };
		break;
	case 0x35545844: //'5TXD':
		format = AFDT_BC3_UNORM;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 16; };
		break;
	default:
		ArrangeRawDDS(img, size);
		format = AFDT_R8G8B8A8_UNORM;
		pitchCalcurator = [](int w, int h) { return w * h * 4; };
		break;
	}
	texSize.size.x = hdr->w;
	texSize.size.y = hdr->h;
	texSize.arraySize = hdr->GetArraySize();

	int arraySize = hdr->GetArraySize();
	int mipCnt = hdr->GetMipCnt();

	std::vector<AFTexSubresourceData> r;
	int offset = 128;
	for (int a = 0; a < arraySize; a++) {
		for (int m = 0; m < mipCnt; m++) {
			int w = std::max(1, hdr->w >> m);
			int h = std::max(1, hdr->h >> m);
			int size = pitchCalcurator(w, h);
			r.push_back({ (char*)img + offset, (uint32_t)pitchCalcurator(w, 1), (uint32_t)size });
			offset += size;
		}
	}

	SRVID srv = afCreateTexture2D(format, texSize.size, arraySize, mipCnt, &r[0]);
	free(img);
	return srv;
}

static SRVID LoadTexture(const char* name, TexDesc& desc)
{
	int len = strlen(name);
	if (len > 4 && !stricmp(name + len - 4, ".dds")) {
		return LoadDDSTexture(name, desc);
	} else {
		desc.arraySize = 1;
		return LoadTextureViaOS(name, desc.size);
	}
}

TexMan::TMID TexMan::CreateDynamicTexture(const char* name, const ivec2& size)
{
	auto it = nameToId.find(name);
	if (it != nameToId.end())
	{
		return it->second;
	}
	TMID id = nameToId[name] = afCreateDynamicTexture(AFDT_R8G8B8A8_UNORM, size);
	TexDesc desc;
	desc.size = size;
	StoreTexState(id, desc);
	return id;
}

TexMan::TMID TexMan::Create(const char *name)
{
	auto it = nameToId.find(name);
	if (it != nameToId.end())
	{
		return it->second;
	}
	TexDesc desc;
	TMID id = nameToId[name] = LoadTexture(name, desc);
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

void TexMan::Write(TMID id, const void* buf)
{
	const TexDesc* d = GetTexDesc(id);
	if (!d) {
		return;
	}
	glBindTexture(GL_TEXTURE_2D, id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, d->size.x, d->size.y, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	glBindTexture(GL_TEXTURE_2D, 0);
}
