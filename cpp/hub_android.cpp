#include "stdafx.h"

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
	systemMetrics.SetScreenSize(ivec2(screenW, screenH));
	hub.Init();
	memset(&lastInputState, 0, sizeof(lastInputState));
	jniEnv = nullptr;
}

JNIEXPORT void JNICALL FUNC(destroy)(JNIEnv* env, jobject obj)
{
	jniEnv = env;
	hub.Destroy();
	jniEnv = nullptr;
}

JNIEXPORT void JNICALL FUNC(update)(JNIEnv* env, jobject obj, jint screenW, jint screenH, jfloat inputX, jfloat inputY, jboolean pressed)
{
	jniEnv = env;

    if (!pressed && lastInputState.pressed) {
        aflog("tap: %f, %f", inputX, inputY);
        ivec2 scrSize = systemMetrics.GetScreenSize();
        hub.OnTap(inputX / scrSize.x * 2 - 1, inputY / scrSize.y * -2 + 1);
    }
    lastInputState.x = inputX;
    lastInputState.y = inputY;
    lastInputState.pressed = pressed;

	hub.Update(screenW, screenH, 0.5f);
	jniEnv = nullptr;
}
}
