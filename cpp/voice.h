#pragma once

struct RiffHeader
{
	char type1[4];
	int size;
	char type2[4];
};

struct RiffChunk
{
	char type[4];
	int size;
};

const void *RiffFindChunk(const void *img, const char *requestChunkName, int *size = nullptr);

class Voice
{
	struct WaveContext* context = nullptr;
	bool IsReady() { return !!context; }
public:
	Voice(const char *fileName) { Create(fileName); }
	~Voice() { Destroy(); }
	void Create(const char *fileName);
	void Play(bool loop);
	void Stop();
	void Destroy();
};
