#include "stdafx.h"

TexMan texMan;

#ifndef _MSC_VER
static GLuint LoadTextureViaOS(const char* name, ivec2& size)
{
	jclass myview = jniEnv->FindClass(boundJavaClass);
	jmethodID method = method = jniEnv->GetStaticMethodID(myview, "loadTexture", "(Ljava/lang/String;)I");
	if (method == 0) {
		return 0;
	}
	GLuint id = jniEnv->CallStaticIntMethod(myview, method, jniEnv->NewStringUTF(name));

	glBindTexture(GL_TEXTURE_2D, id);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &size.x);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &size.y);
	glBindTexture(GL_TEXTURE_2D, 0);
}
#endif

#ifdef _MSC_VER
namespace Gdiplus {
	using std::min;
	using std::max;
}
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

static GLuint LoadTextureViaOS(const char* name, ivec2& size)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
	WCHAR wc[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, name, -1, wc, dimof(wc));
	Gdiplus::Bitmap* image = new Gdiplus::Bitmap(wc);

	int w = (int)image->GetWidth();
	int h = (int)image->GetHeight();
	size.x = w;
	size.y = h;
	Gdiplus::Rect rc(0, 0, w, h);

	Gdiplus::BitmapData* bitmapData = new Gdiplus::BitmapData;
	image->LockBits(&rc, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bitmapData);

	std::vector<uint32_t> col;
	col.resize(w * h);
	for (int y = 0; y < h; y++) {
		memcpy(&col[y * w], (char*)bitmapData->Scan0 + bitmapData->Stride * y, w * 4);
		for (int x = 0; x < w; x++) {
			uint32_t& c = col[y * w + x];
			c = (c & 0xff00ff00) | ((c & 0xff) << 16) | ((c & 0xff0000) >> 16);
		}
	}
	image->UnlockBits(bitmapData);
	delete bitmapData;
	delete image;
	Gdiplus::GdiplusShutdown(gdiplusToken);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &col[0]);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}
#endif

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
	});
}

static GLuint LoadDDSTexture(const char* name, ivec2& texSize)
{
	int size;
	GLuint texture = 0;
	void* img = LoadFile(name, &size);
	if (!img) {
		aflog("LoadDDSTexture failed! %s", name);
		return 0;
	}
	const DDSHeader* hdr = (DDSHeader*)img;

	GLenum format;
	int(*pitchCalcurator)(int, int) = nullptr;
	switch (hdr->fourcc) {
	case 0x31545844: //'1TXD':
		format = 0x83F1;	// GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 8; };
		break;
	case 0x33545844: //'3TXD':
		format = 0x83F2;	// GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 16; };
		break;
	case 0x35545844: //'5TXD':
		format = 0x83F3;	// GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		pitchCalcurator = [](int w, int h) { return ((w + 3) / 4) * ((h + 3) / 4) * 16; };
		break;
	default:
		ArrangeRawDDS(img, size);
		format = GL_RGBA;
		pitchCalcurator = [](int w, int h) { return w * h * 4; };
		break;
	}
	texSize.x = hdr->w;
	texSize.y = hdr->h;

	GLenum target = hdr->IsCubeMap() ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
	GLenum targetFace = hdr->IsCubeMap() ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D;

	glGenTextures(1, &texture);
	glBindTexture(target, texture);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int arraySize = hdr->GetArraySize();
	int mipCnt = hdr->GetMipCnt();
	int offset = 128;
	for (int a = 0; a < arraySize; a++) {
		for (int m = 0; m < mipCnt; m++) {
			int w = std::max(1, hdr->w >> m);
			int h = std::max(1, hdr->h >> m);
			if (format == GL_RGBA) {
				afHandleGLError(glTexImage2D(targetFace + a, m, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (char*)img + offset));
			} else {
				int texSize = pitchCalcurator(w, h);
				afHandleGLError(glCompressedTexImage2D(targetFace + a, m, format, w, h, 0, texSize, (char*)img + offset));
			}
			offset += pitchCalcurator(w, h);
		}
	}
	if (mipCnt == 1) {
		if (format == GL_RGBA) {
			afHandleGLError(glGenerateMipmap(target));
		} else {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// temporary disable mipmap
		}
	}

	glBindTexture(target, 0);
	free(img);
	return texture;
}

static GLuint LoadTexture(const char* name, ivec2& size)
{
	size = ivec2();
	int len = strlen(name);
	if (len > 4 && !stricmp(name + len - 4, ".dds")) {
		return LoadDDSTexture(name, size);
	} else {
		return LoadTextureViaOS(name, size);
	}
}

TexMan::TMID TexMan::CreateDynamicTexture(const char* name, int w, int h)
{
	auto it = nameToId.find(name);
	if (it != nameToId.end())
	{
		return it->second;
	}
	TMID id = nameToId[name] = afCreateDynamicTexture(w, h, AFDT_R8G8B8A8_UINT);
	ivec2 size(w, h);
	StoreTexState(id, size);
	return id;
}

TexMan::TMID TexMan::Create(const char *name)
{
	NameToId::iterator it = nameToId.find(name);
	if (it != nameToId.end())
	{
		return it->second;
	}
	ivec2 size;
	TMID id = nameToId[name] = LoadTexture(name, size);
	StoreTexState(id, size);
	return id;
}

TexMan::TMID TexMan::CreateWhiteTexture()
{
	const std::string name = "$WHITE";
	NameToId::iterator it = nameToId.find(name);
	if (it != nameToId.end())
	{
		return it->second;
	}
	TMID id = nameToId[name] = afCreateWhiteTexture();
	ivec2 size(1, 1);
	StoreTexState(id, size);
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

void TexMan::StoreTexState(TMID id, const ivec2& v2)
{
	if (id >= texStates.size() ) {
		texStates.resize(id + 1);
	}
	texStates[id] = v2;
}

ivec2 TexMan::GetSize(TMID id)
{
	if (id >= texStates.size()) {
		return ivec2();
	}
	return texStates[id];
}

void TexMan::Write(TMID id, const void* buf)
{
	ivec2 v = GetSize(id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, v.x, v.y, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	glBindTexture(GL_TEXTURE_2D, 0);
}
