#include "stdafx.h"

struct WaveContext
{
	HWAVEOUT hWaveOut;
	WAVEHDR wavehdr;
	void *fileImg;
};

static bool FindAndFillWavehdr(WAVEHDR *wh, const void *fileImg)
{
	memset(wh, 0, sizeof(*wh));
	int size;
	wh->lpData = (char*)RiffFindChunk(fileImg, "data", &size);
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

void Voice::Create(const char *fileName)
{
	Destroy();
	context = new WaveContext;
	bool result = false;
	result = !!(context->fileImg = LoadFile(fileName));
	assert(result);
	const WAVEFORMATEX* wfx = (WAVEFORMATEX*)RiffFindChunk(context->fileImg, "fmt ");
	assert(wfx);
	result = FindAndFillWavehdr(&context->wavehdr, context->fileImg);
	assert(result);
	mmresultVerify(waveOutOpen(&context->hWaveOut, WAVE_MAPPER, wfx, 0, 0, CALLBACK_NULL));
	mmresultVerify(waveOutPrepareHeader(context->hWaveOut, &context->wavehdr, sizeof(WAVEHDR)));
}

void Voice::Play(bool loop)
{
	if (!IsReady()) {
		return;
	}
	context->wavehdr.dwFlags |= WHDR_BEGINLOOP | WHDR_ENDLOOP;
	context->wavehdr.dwLoops = loop ? 0xffffffff : 1;
	mmresultVerify(waveOutWrite(context->hWaveOut, &context->wavehdr, sizeof(WAVEHDR)));
}

void Voice::Stop()
{
	if (!IsReady()) {
		return;
	}
	waveOutReset(context->hWaveOut);
}

void Voice::Destroy()
{
	if (!IsReady()) {
		return;
	}
	Stop();
	waveOutUnprepareHeader(context->hWaveOut, &context->wavehdr, sizeof(WAVEHDR));
	if (context->hWaveOut) {
		waveOutClose(context->hWaveOut);
	}
	if (context->fileImg) {
		free(context->fileImg);
	}
	SAFE_DELETE(context);
}
