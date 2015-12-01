#include "stdafx.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

template <class T> void SAFE_DESTROY(T& p) {
	if (p) {
		(*p)->Destroy(p);
		p = nullptr;
	}
}

class SL {
	SLObjectItf engineObject = nullptr;
public:
	SL() {
		SLresult r = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
		assert(r == SL_RESULT_SUCCESS);
		aflog("slCreateEngine");
		r = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
		assert(r == SL_RESULT_SUCCESS);
		aflog("engineObject->Realize");
	}
	~SL() {
		SAFE_DESTROY(engineObject);
		aflog("engineObject destroyed");
	}
};

static SL sl;

void Voice::Create(const char*)
{
}

void Voice::Play(bool)
{
}

void Voice::Stop()
{
}

void Voice::Destroy()
{
}
