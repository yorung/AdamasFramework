#include "stdafx.h"

void *LoadFile(const char *fileName, int* size)
{
	jclass myview = jniEnv->FindClass(boundJavaClass);
	jmethodID method = jniEnv->GetStaticMethodID(myview, "loadIntoBytes", "(Ljava/lang/String;)[B");
	if (method == 0) {
		return nullptr;
	}

	jobject arrayAsJObject = jniEnv->CallStaticObjectMethod(myview, method, jniEnv->NewStringUTF(fileName));
	if (!arrayAsJObject) {
		aflog("LoadFile: Loading %s failed!", fileName);
		return nullptr;
	}
	jbyteArray array = (jbyteArray)arrayAsJObject;

	jbyte* byteArray = jniEnv->GetByteArrayElements(array, NULL);
	jsize arrayLen = jniEnv->GetArrayLength(array);

	void* ptr = calloc(arrayLen + 1, 1);
	memcpy(ptr, byteArray, arrayLen);
	if (size) {
		*size = arrayLen;
	}

	jniEnv->ReleaseByteArrayElements(array, byteArray, 0);

	return ptr;
}

void Toast(const char *text)
{
	jclass helper = jniEnv->FindClass(boundJavaClass);
	jmethodID method = jniEnv->GetStaticMethodID(helper, "toast", "(Ljava/lang/String;)V");
	if (method == 0) {
		return;
	}
//	aflog("jclass=%d method=%d", helper, method);
	jniEnv->CallStaticVoidMethod(helper, method, jniEnv->NewStringUTF(text));
}

void PlayBgm(const char *fileName)
{
	jclass helper = jniEnv->FindClass(boundJavaClass);
	jmethodID method = jniEnv->GetStaticMethodID(helper, "playBgm", "(Ljava/lang/String;)V");
	if (method == 0) {
		return;
	}
	jniEnv->CallStaticVoidMethod(helper, method, jniEnv->NewStringUTF(fileName));
}

const char* StrMessageBox(const char* txt, const char* type)
{

}

void ClearMenu()
{

}
void AddMenu(const char *name, const char *cmd)
{

}
void PostCommand(const char* cmdString)
{

}
