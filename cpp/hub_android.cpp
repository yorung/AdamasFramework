#include "stdafx.h"

JNIEnv* jniEnv;
const char* boundJavaClass = "common/pinotnoir/Helper";
#define FUNC(name) Java_common_pinotnoir_Native_##name

static float width, height;

extern "C" {
JNIEXPORT void JNICALL FUNC(init)(JNIEnv* env, jobject obj, jint screenW, jint screenH)
{
	jniEnv = env;
	width = screenW;
	height = screenH;
	hub.Init(screenW, screenH);
	jniEnv = nullptr;
}

JNIEXPORT void JNICALL FUNC(destroy)(JNIEnv* env, jobject obj)
{
	jniEnv = env;
	hub.Destroy();
	jniEnv = nullptr;
}

JNIEXPORT void JNICALL FUNC(update)(JNIEnv* env, jobject obj, jint screenW, jint screenH, jfloat offset)
{
	jniEnv = env;
	hub.Update(screenW, screenH, offset);
	jniEnv = nullptr;
}

JNIEXPORT void JNICALL FUNC(onTap)(JNIEnv* env, jobject obj, jfloat x, jfloat y)
{
	jniEnv = env;

    aflog("%f, %f", x, y);
	hub.OnTap((float)x / width * 2 - 1, (float)y / height * -2 + 1);

//	hub.OnTap(x, y);
	jniEnv = nullptr;
}
}
