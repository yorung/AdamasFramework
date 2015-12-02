#include "stdafx.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

template <class T> void SAFE_DESTROY(T& p)
{
	if (p) {
		(*p)->Destroy(p);
		p = nullptr;
	}
}

static void Log(SLresult r, const char* msg)
{
	aflog("%s %s", msg, (r == SL_RESULT_SUCCESS ? "success" : "failed"));
	assert(r == SL_RESULT_SUCCESS);
}

class SL {
	SLObjectItf engineObject = nullptr;
	SLEngineItf engineEngine = nullptr;
	SLObjectItf outputMixObject = nullptr;
public:
	SL() {
		SLresult r = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
		Log(r, "slCreateEngine");
		r = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
		Log(r, "engineObject->Realize");
		r = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
		Log(r, "engineObject->GetInterface");
		SLInterfaceID ids = SL_IID_ENVIRONMENTALREVERB;
		SLboolean req = SL_BOOLEAN_FALSE;
		r = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, &ids, &req);
		Log(r, "CreateOutputMix");
		r = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
		Log(r, "outputMixObject->Realize");
	}
	~SL() {
		SAFE_DESTROY(outputMixObject);
		aflog("outputMixObject destroyed");
		SAFE_DESTROY(engineObject);
		engineEngine = nullptr;
		aflog("engineObject destroyed");
	}
};

static SL sl;

struct WaveContext
{
	SLObjectItf playerObject;
	SLPlayItf playerPlay;
	SLAndroidSimpleBufferQueueItf playerBufferQueue;
	SLEffectSendItf playerEffectSend;
	SLMuteSoloItf playerMuteSolo;
	SLVolumeItf playerVolume;
};

void Voice::Create(const char*)
{
	context = new WaveContext;
	memset(context, 0, sizeof(*context));

	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
	SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
	SLDataSource audioSrc = {&loc_bufq, &format_pcm};
}

void Voice::Play(bool)
{
	if (!IsReady()) {
		return;
	}
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
	SAFE_DELETE(context);
}
