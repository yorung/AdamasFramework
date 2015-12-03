#include "stdafx.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

struct WaveFormatEx {
	uint16_t tag, channels;
	uint32_t samplesPerSecond, averageBytesPerSecond;
	uint16_t blockAlign, bitsPerSample;
};

template <class T> void SAFE_DESTROY(T& p)
{
	if (p) {
		(*p)->Destroy(p);
		p = nullptr;
	}
}

void _afHandleSLError(const char* func, int line, const char* command, SLresult r)
{
	if (r != SL_RESULT_SUCCESS) {
		const char *err = nullptr;
		switch (r) {
#define E(er) case er: err = #er; break
		E(SL_RESULT_PRECONDITIONS_VIOLATED);
		E(SL_RESULT_PARAMETER_INVALID);
		E(SL_RESULT_MEMORY_FAILURE);
		E(SL_RESULT_RESOURCE_ERROR);
		E(SL_RESULT_RESOURCE_LOST);
		E(SL_RESULT_BUFFER_INSUFFICIENT);
#undef E
		default:
			aflog("%s(%d): err=%d %s\n", func, line, r, command);
			return;
		}
		aflog("%s(%d): %s %s\n", func, line, err, command);
	}
}

#define afHandleSLError(command) do{ SLresult r = command; _afHandleSLError(__FUNCTION__, __LINE__, #command, r); } while(0)

class SL {
	SLObjectItf engineObject = nullptr;
	SLEngineItf engineEngine = nullptr;
	SLObjectItf outputMixObject = nullptr;
public:
	SL() {
		afHandleSLError(slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr));
		afHandleSLError((*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE));
		afHandleSLError((*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine));
		SLInterfaceID ids = SL_IID_ENVIRONMENTALREVERB;
		SLboolean req = SL_BOOLEAN_FALSE;
		afHandleSLError((*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, &ids, &req));
		afHandleSLError((*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE));
	}
	~SL() {
		SAFE_DESTROY(outputMixObject);
		SAFE_DESTROY(engineObject);
		engineEngine = nullptr;
		aflog("engineObject destroyed");
	}
	SLEngineItf GetEngine(){ return engineEngine; }
	SLObjectItf GetOutputMixObject() { return outputMixObject; }
};

static SL sl;

struct WaveContext
{
	SLObjectItf playerObject;
	SLPlayItf playerPlay;
	SLAndroidSimpleBufferQueueItf playerBufferQueue;
	void *fileImg;
};

void Voice::Create(const char* fileName)
{
	context = new WaveContext;
	memset(context, 0, sizeof(*context));
	bool result = false;
	result = !!(context->fileImg = LoadFile(fileName));
	assert(result);
	const WaveFormatEx* wfx = (WaveFormatEx*)RiffFindChunk(context->fileImg, "fmt ");
	assert(wfx);
	SLDataLocator_AndroidSimpleBufferQueue q = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
	SLDataFormat_PCM f = {SL_DATAFORMAT_PCM, wfx->channels, wfx->samplesPerSecond * 1000, wfx->bitsPerSample, wfx->bitsPerSample, SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
	SLDataSource src = {&q, &f};
	SLDataLocator_OutputMix m = {SL_DATALOCATOR_OUTPUTMIX, sl.GetOutputMixObject()};
	SLDataSink sink = {&m, nullptr};
	SLInterfaceID ids = SL_IID_ANDROIDSIMPLEBUFFERQUEUE;
	SLboolean req = SL_BOOLEAN_TRUE;
	afHandleSLError((*sl.GetEngine())->CreateAudioPlayer(sl.GetEngine(), &context->playerObject, &src, &sink, 1, &ids, &req));
	afHandleSLError((*context->playerObject)->Realize(context->playerObject, SL_BOOLEAN_FALSE));
	afHandleSLError((*context->playerObject)->GetInterface(context->playerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &context->playerBufferQueue));
	afHandleSLError((*context->playerBufferQueue)->RegisterCallback(context->playerBufferQueue, [](SLAndroidSimpleBufferQueueItf q, void*){}, nullptr));
	afHandleSLError((*context->playerObject)->GetInterface(context->playerObject, SL_IID_PLAY, &context->playerPlay));
	afHandleSLError((*context->playerPlay)->SetPlayState(context->playerPlay, SL_PLAYSTATE_PLAYING));
}

void Voice::Play(bool)
{
	if (!IsReady()) {
		return;
	}
	int size;
	const void* buf = RiffFindChunk(context->fileImg, "data", &size);
	afHandleSLError((*context->playerBufferQueue)->Enqueue(context->playerBufferQueue, buf, size));
}

void Voice::Stop()
{
	if (!IsReady()) {
		return;
	}
}

void Voice::Destroy()
{
	if (!IsReady()) {
		return;
	}
	SAFE_DESTROY(context->playerObject);
	if (context->fileImg) {
		free(context->fileImg);
	}
	SAFE_DELETE(context);
}
