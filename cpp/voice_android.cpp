#include "stdafx.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

struct WaveFormatEx {
	uint16_t tag, channels;
	uint32_t samplesPerSecond, averageBytesPerSecond;
	uint16_t blockAlign, bitsPerSample;
};

template <class T> void SafeDestroy(T& p)
{
	if (p) {
		(*p)->Destroy(p);
		p = nullptr;
	}
}

SLresult _slHandleError(const char* func, int line, const char* command, SLresult r)
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
		E(SL_RESULT_CONTENT_CORRUPTED);
		E(SL_RESULT_CONTENT_UNSUPPORTED);
#undef E
		default:
			aflog("%s(%d): err=%d %s\n", func, line, r, command);
			return r;
		}
		aflog("%s(%d): %s %s\n", func, line, err, command);
	}
	return r;
}

#define SLHandleError(command) _slHandleError(__FUNCTION__, __LINE__, #command, command)
#define SLCall(obj,func,...) SLHandleError((*obj)->func(obj, __VA_ARGS__))

class SL {
	SLObjectItf engineObject = nullptr;
	SLEngineItf engineEngine = nullptr;
	SLObjectItf outputMixObject = nullptr;
public:
	SL() {
		SLHandleError(slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr));
		SLCall(engineObject, Realize, SL_BOOLEAN_FALSE);
		SLCall(engineObject, GetInterface, SL_IID_ENGINE, &engineEngine);
		SLInterfaceID ids = SL_IID_ENVIRONMENTALREVERB;
		SLboolean req = SL_BOOLEAN_FALSE;
		SLCall(engineEngine, CreateOutputMix, &outputMixObject, 1, &ids, &req);
		SLCall(outputMixObject, Realize, SL_BOOLEAN_FALSE);
	}
	~SL() {
		SafeDestroy(outputMixObject);
		SafeDestroy(engineObject);
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
	int enqueuedSize;
	bool loop;
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
	SLDataFormat_PCM f = {SL_DATAFORMAT_PCM, wfx->channels, wfx->samplesPerSecond * 1000, wfx->bitsPerSample, wfx->bitsPerSample, wfx->channels == 2 ? SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT : SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
	SLDataSource src = {&q, &f};
	SLDataLocator_OutputMix m = {SL_DATALOCATOR_OUTPUTMIX, sl.GetOutputMixObject()};
	SLDataSink sink = {&m, nullptr};
	SLInterfaceID ids = SL_IID_ANDROIDSIMPLEBUFFERQUEUE;
	SLboolean req = SL_BOOLEAN_TRUE;
	SLCall(sl.GetEngine(), CreateAudioPlayer, &context->playerObject, &src, &sink, 1, &ids, &req);
	SLCall(context->playerObject, Realize, SL_BOOLEAN_FALSE);
	SLCall(context->playerObject, GetInterface, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &context->playerBufferQueue);
	SLCall(context->playerObject, GetInterface, SL_IID_PLAY, &context->playerPlay);
}

void Voice::Play(bool loop)
{
	if (!IsReady()) {
		return;
	}
	auto playback = [](SLAndroidSimpleBufferQueueItf q, void* context_) {
		double now = GetTime();
		if (now - systemMetrics.GetLastUpdateTime() >= 0.5) {
			return;
		}

		WaveContext* context = (WaveContext*)context_;
		int totalSize;
		const void* buf = RiffFindChunk(context->fileImg, "data", &totalSize);
		int enqueued = context->enqueuedSize;
		if (enqueued >= totalSize) {
			if (!context->loop) {
			//	aflog("enqueue: finished");
				return;
			}
			enqueued = 0;
		}
		int toEnqueue = std::min(totalSize - enqueued, 32768);
		enqueued += toEnqueue;
		context->enqueuedSize = enqueued;
		SLCall(q, Enqueue, (char*)buf + enqueued - toEnqueue, toEnqueue);
		//aflog("enqueue: from=%d size=%d", context->enqueuedSize, toEnqueue);
	};
	SLCall(context->playerPlay, SetPlayState, SL_PLAYSTATE_STOPPED);
	SLCall(context->playerBufferQueue, RegisterCallback, playback, context);
	context->enqueuedSize = 0;
	context->loop = loop;
	SLCall(context->playerPlay, SetPlayState, SL_PLAYSTATE_PLAYING);
	playback(context->playerBufferQueue, context);
}

void Voice::Stop()
{
	if (!IsReady()) {
		return;
	}
	SLCall(context->playerPlay, SetPlayState, SL_PLAYSTATE_STOPPED);
}

void Voice::Destroy()
{
	if (!IsReady()) {
		return;
	}
	SafeDestroy(context->playerObject);
	if (context->fileImg) {
		free(context->fileImg);
	}
	SAFE_DELETE(context);
}
