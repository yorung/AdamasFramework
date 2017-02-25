inline IBOID afCreateTiledPlaneIBO(int numTiles, int* numIndies)
{
	const int numVert = numTiles + 1;

	std::vector<AFIndex> indi;

	for (int y = 0; y < numTiles; y++)
	{
		if (y != 0)
		{
			indi.push_back(AFIndex(y * numVert));
		}
		indi.push_back(AFIndex(y * numVert));
		for (int x = 0; x < numTiles; x++)
		{
			indi.push_back(AFIndex((y + 1) * numVert + x));
			indi.push_back(AFIndex(y * numVert + x + 1));
		}
		indi.push_back(AFIndex((y + 1) * numVert + numVert - 1));
		if (y != numTiles - 1)
		{
			indi.push_back(AFIndex((y + 1) * numVert + numVert - 1));
		}
	}

	if (numIndies)
	{
		*numIndies = (int)indi.size();
	}

	return afCreateIndexBuffer((int)indi.size(), &indi[0]);
}

inline VBOID afCreateTiledPlaneVBO(int numTiles)
{
	std::vector<Vec2> v;
	for (int y = 0; y <= numTiles; y++) {
		for (int x = 0; x <= numTiles; x++) {
			v.push_back((Vec2)IVec2(x, y) / (float)numTiles * 2 - Vec2(1, 1));
		}
	}
	return afCreateVertexBuffer((int)v.size() * sizeof(v[0]), &v[0]);
}

inline IBOID afCreateQuadListIndexBuffer(int numQuads)
{
	std::vector<AFIndex> indi;
	int numIndi = numQuads * 6;
	indi.resize(numIndi);
	for (int i = 0; i < numIndi; i++)
	{
		static int tbl[] = { 0, 1, 2, 1, 3, 2 };
		int rectIdx = i / 6;
		int vertIdx = i % 6;
		indi[i] = AFIndex(rectIdx * 4 + tbl[vertIdx]);
	}
	return afCreateIndexBuffer(numIndi, &indi[0]);
}

struct DDSHeader
{
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

inline void bitScanForward(uint32_t* result, uint32_t mask)
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

inline AFFormat ArrangeRawDDS(void* img, int size)
{
	const DDSHeader* hdr = (DDSHeader*)img;
	assert(hdr->bitsPerPixel == 32);
#ifdef _MSC_VER	// GL_EXT_texture_format_BGRA8888 is an extension for OpenGL ES, so should use this only on Windows.
	if (hdr->rMask == 0xff0000 && hdr->gMask == 0x00ff00 && hdr->bMask == 0x0000ff)
	{
		return AFF_B8G8R8A8_UNORM;
	}
#endif
	if (hdr->rMask == 0x0000ff && hdr->gMask == 0x00ff00 && hdr->bMask == 0xff0000)
	{
		return AFF_R8G8B8A8_UNORM;
	}
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
	return AFF_R8G8B8A8_UNORM;
}

inline void Convert24To32(void*& img, int& size)
{
	const DDSHeader* hdr = (DDSHeader*)img;
	if (hdr->bitsPerPixel != 24)
	{
		return;
	}
	int numPixels = (size - 128) / 3;
	int newSize = 128 + numPixels * 4;
	void* newImg = malloc(newSize);
	*(DDSHeader*)newImg = *hdr;
	((DDSHeader*)newImg)->bitsPerPixel = 32;
	uint8_t* dst = (uint8_t*)newImg + sizeof(DDSHeader);
	uint8_t* src = (uint8_t*)img + sizeof(DDSHeader);
	for (int i = 0; i < numPixels; i++)
	{
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = *src++;
		*dst++ = 255;
	}
	free(img);
	img = newImg;
	size = newSize;
}

inline AFTexRef afLoadDDSTexture(const char* name, TexDesc& texDesc)
{
	int fileSize;
	void* img = LoadFile(name, &fileSize);
	if (!img) {
		aflog("afLoadDDSTexture failed! %s", name);
		return AFTexRef();
	}
	Convert24To32(img, fileSize);
	const DDSHeader* hdr = (DDSHeader*)img;

	AFFormat format = AFF_INVALID;
	int(*pitchCalcurator)(int, int) = nullptr;
	switch (hdr->fourcc) {
	case 0x31545844: //'1TXD':
		format = AFF_BC1_UNORM;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 8; };
		break;
	case 0x33545844: //'3TXD':
		format = AFF_BC2_UNORM;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 16; };
		break;
	case 0x35545844: //'5TXD':
		format = AFF_BC3_UNORM;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 16; };
		break;
	default:
		format = ArrangeRawDDS(img, fileSize);
		pitchCalcurator = [](int w, int h) { return w * h * 4; };
		break;
	}
	texDesc.size.x = hdr->w;
	texDesc.size.y = hdr->h;
	texDesc.arraySize = hdr->GetArraySize();
	texDesc.isCubeMap = hdr->IsCubeMap();

	int arraySize = hdr->GetArraySize();
	int mipCnt = hdr->GetMipCnt();
	std::vector<AFTexSubresourceData> r;
	int offset = 128;
	for (int a = 0; a < arraySize; a++) {
		for (int m = 0; m < mipCnt; m++) {
			int w = std::max(1, hdr->w >> m);
			int h = std::max(1, hdr->h >> m);
			int pitch = pitchCalcurator(w, h);
			r.push_back({ (char*)img + offset, (uint32_t)pitchCalcurator(w, 1), (uint32_t)pitch });
			offset += pitch;
		}
	}
	assert(offset <= fileSize);

	AFTexRef tex = afCreateTexture2D(format, texDesc, mipCnt, &r[0]);
	assert(tex);
	free(img);
	return tex;
}

AFTexRef LoadTextureViaOS(const char* name, IVec2& size);

inline AFTexRef afLoadTexture(const char* name, TexDesc& desc)
{
	desc = TexDesc();
	size_t len = strlen(name);
	AFTexRef tex;
	if (len > 4 && !stricmp(name + len - 4, ".dds"))
	{
		tex = afLoadDDSTexture(name, desc);
	}
	else
	{
		tex = LoadTextureViaOS(name, desc.size);
	}
	afSetTextureName(tex, name);
	return tex;
}

inline AFTexRef afLoadTexture(const char* name)
{
	TexDesc dummy;
	return afLoadTexture(name, dummy);
}
