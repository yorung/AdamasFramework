#include "stdafx.h"

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

static const void *FindChunk(const void *img, const char *requestChunkName, int *size = nullptr)
{
	const RiffHeader *riff = (RiffHeader*)img;
	if (*(DWORD*)riff->type1 != *(DWORD*)"RIFF" || *(DWORD*)riff->type2 != *(DWORD*)"WAVE") {
		return nullptr;
	}

	const void *end = (BYTE*)img + riff->size + 8;
	img = (BYTE*)img + sizeof(RiffHeader);
	while (img < end) {
		const RiffChunk *chunk = (RiffChunk*)img; img = (BYTE*)img + sizeof(RiffChunk);
		if (*(DWORD*)chunk->type == *(DWORD*)requestChunkName) {
			if (size) {
				*size = chunk->size;
			}
			return img;
		}
		img = (BYTE*)img + chunk->size;
	}
	return nullptr;
}

static const WAVEFORMATEX *FindWaveformatex(const void *fileImg)
{
	return (WAVEFORMATEX*)FindChunk(fileImg, "fmt ");
}

static bool FindAndFillWavehdr(WAVEHDR *wh, const void *fileImg)
{
	memset(wh, 0, sizeof(*wh));
	int size;
	wh->lpData = (char*)FindChunk(fileImg, "data", &size);
	if (!wh->lpData) {
		return false;
	}
	wh->dwBufferLength = size;
	return true;
}

static void mmresultVerify(MMRESULT mr)
{
	switch (mr)
	{
	case MMSYSERR_INVALHANDLE: OutputDebugStringA("MMSYSERR_INVALHANDLE\n"); break;
	case MMSYSERR_NODRIVER: OutputDebugStringA("MMSYSERR_NODRIVER\n"); break;
	case MMSYSERR_NOMEM: OutputDebugStringA("MMSYSERR_NOMEM\n"); break;
	case WAVERR_BADFORMAT: OutputDebugStringA("WAVERR_BADFORMAT\n"); break;
	case WAVERR_STILLPLAYING: OutputDebugStringA("WAVERR_STILLPLAYING\n"); break;
	case WAVERR_UNPREPARED: OutputDebugStringA("WAVERR_UNPREPARED\n"); break;
	case WAVERR_SYNC: OutputDebugStringA("WAVERR_SYNC\n"); break;
	}
	assert(mr == MMSYSERR_NOERROR);
}

Voice::Voice()
{
	memset(&context, 0, sizeof(context));
}

Voice::Voice(const char *fileName) : Voice()
{
	Load(fileName);
}

Voice::~Voice()
{
	Release();
}

bool Voice::IsReady()
{
	return !!context.fileImg;
}

void Voice::Load(const char *fileName)
{
	Release();
	bool result = false;
	result = !!(context.fileImg = LoadFile(fileName));
	assert(result);
	result = !!FindWaveformatex(context.fileImg);
	assert(result);
	result = FindAndFillWavehdr(&context.wavehdr, context.fileImg);
	assert(result);
	mmresultVerify(waveOutOpen(&context.hWaveOut, WAVE_MAPPER, FindWaveformatex(context.fileImg), 0, 0, CALLBACK_NULL));
	mmresultVerify(waveOutPrepareHeader(context.hWaveOut, &context.wavehdr, sizeof(WAVEHDR)));
}

void Voice::Play(bool loop)
{
	if (!IsReady()) {
		return;
	}
	context.wavehdr.dwFlags |= WHDR_BEGINLOOP | WHDR_ENDLOOP;
	context.wavehdr.dwLoops = loop ? 0xffffffff : 1;
	mmresultVerify(waveOutWrite(context.hWaveOut, &context.wavehdr, sizeof(WAVEHDR)));
}

void Voice::Stop()
{
	if (!IsReady()) {
		return;
	}
	waveOutReset(context.hWaveOut);
}

void Voice::Release()
{
	if (!IsReady()) {
		return;
	}
	Stop();
	waveOutUnprepareHeader(context.hWaveOut, &context.wavehdr, sizeof(WAVEHDR));
	if (context.hWaveOut) {
		waveOutClose(context.hWaveOut);
	}
	if (context.fileImg) {
		free(context.fileImg);
	}
	memset(&context, 0, sizeof(context));
}
