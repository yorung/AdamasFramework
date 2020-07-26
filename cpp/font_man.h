#pragma once
#include "dib.h"

class FontMan
{
	struct CharCache
	{
		Vec2 srcPos;
		CharDesc desc;
	};
	struct CharSprite
	{
		Vec2 pos;
		CharSignature signature;
		uint32_t color;
	};
	typedef std::map<CharSignature, CharCache> Caches;
	Caches caches;
	int curX, curY, curLineMaxH;
	AFTexRef texture;
	DIB texSrc;

	static const int SPRITE_MAX = 4096;
	static const int SPRITE_VERTS = SPRITE_MAX * 4;
	CharSprite charSprites[SPRITE_MAX];
	int numSprites;
	AFRenderStates renderStates;
	AFDynamicQuadListVertexBuffer quadListVertexBuffer;
	bool dirty = false;
	bool Build(const CharSignature& signature);
	bool Cache(const CharSignature& code);
	void DrawChar(Vec2& pos, const CharSignature& sig, uint32_t color);
	void ClearCache();
public:
	FontMan();
	~FontMan();
	void Create();
	void Destroy();
	void FlushToTexture();
	void DrawString(Vec2 pos, int fontSize, const wchar_t *text, uint32_t color);
	void DrawString(Vec2 pos, int fontSize, const char *text, uint32_t color);
	void Draw(AFCommandList& cmd, const IVec2& screenSize);
	Vec2 MeasureString(int fontSize, const char *text);
};

extern FontMan fontMan;
