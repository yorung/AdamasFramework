#include "stdafx.h"
#ifdef __ANDROID__
#include <jni.h>

JNIEnv* jniEnv;
const char* boundJavaClass = "common/pinotnoir/Helper";
#define FUNC(name) Java_common_pinotnoir_Native_##name

struct LastInputState {
    float x, y;
    bool pressed;
} lastInputState;

extern "C" {
JNIEXPORT void JNICALL FUNC(init)(JNIEnv* env, jobject obj, jint screenW, jint screenH)
{
	jniEnv = env;
	systemMisc.SetScreenSize(IVec2(screenW, screenH));
	app.Create();
	memset(&lastInputState, 0, sizeof(lastInputState));
	jniEnv = nullptr;
}

JNIEXPORT void JNICALL FUNC(destroy)(JNIEnv* env, jobject obj)
{
	jniEnv = env;
	app.Destroy();
	jniEnv = nullptr;
}

JNIEXPORT void JNICALL FUNC(update)(JNIEnv* env, jobject obj, jfloat inputX, jfloat inputY, jboolean pressed)
{
	jniEnv = env;

    IVec2 scrSize = systemMisc.GetScreenSize();
    float x = inputX / scrSize.x;
    float y = inputY / scrSize.y;

    if (pressed && !lastInputState.pressed) {
        devCamera.LButtonDown(x, y);
		inputMan.HandleTouch(true);
	}

    if (pressed && lastInputState.pressed) {
        devCamera.MouseMove(x, y);
    }

    if (!pressed && lastInputState.pressed) {
        aflog("tap: %f, %f", inputX, inputY);
        devCamera.LButtonUp(x, y);
		inputMan.HandleTouch(false);
	}
    lastInputState.x = inputX;
    lastInputState.y = inputY;
    lastInputState.pressed = pressed;

	systemMisc.SetMousePos(IVec2(inputX, inputY));
	systemMisc.mouseDown = pressed;

	app.Update();
	jniEnv = nullptr;
}
}
#endif	// __ANDROID__
