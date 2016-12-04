#include "stdafx.h"
#include <jni.h>

extern JNIEnv* jniEnv;
extern const char* boundJavaClass;

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
	return "ok";
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

SRVID LoadTextureViaOS_old(const char* name, IVec2& size)
{
	jclass myview = jniEnv->FindClass(boundJavaClass);
	jmethodID method = jniEnv->GetStaticMethodID(myview, "loadTexture", "(Ljava/lang/String;)I");
	if (method == 0) {
		return SRVID();
	}
	SRVID id;
	id.x = jniEnv->CallStaticIntMethod(myview, method, jniEnv->NewStringUTF(name));
#ifdef AF_GLES31
	size = afGetTextureSize(id);
#else
	size.x = 0;
	size.y = 0;
#endif
	return id;
}

SRVID LoadTextureViaOS(const char* name, IVec2& size)
{
	jclass myview = jniEnv->FindClass(boundJavaClass);
	jmethodID method = jniEnv->GetStaticMethodID(myview, "loadImage", "(Ljava/lang/String;[I)[B");
	if (method == 0)
	{
		return SRVID();
	}

	jintArray descArray = jniEnv->NewIntArray(2);
	jobject arrayAsJObject = jniEnv->CallStaticObjectMethod(myview, method, jniEnv->NewStringUTF(name), descArray);
	{
		jint* pDescArray = jniEnv->GetIntArrayElements(descArray, NULL);
		size.x = pDescArray[0];
		size.y = pDescArray[0];
		jniEnv->ReleaseIntArrayElements(descArray, pDescArray, 0);
	}
	if (!arrayAsJObject)
	{
		aflog("Java method returned null");
		return SRVID();
	}

	jbyteArray array = (jbyteArray)arrayAsJObject;
	jbyte* byteArray = jniEnv->GetByteArrayElements(array, NULL);
	jsize arrayLen = jniEnv->GetArrayLength(array);
	//	aflog("arrayLen=%d", arrayLen);

	SRVID tex;
	int expectedLen = size.x * size.y * 4;
	if (arrayLen != expectedLen)
	{
		aflog("wrong size! returned=%d expected=%d", arrayLen, expectedLen);
		afVerify(false);
	}
	else
	{
		TexDesc desc;
		desc.size = size;
		AFTexSubresourceData subresource = { byteArray, (uint32_t)size.x * 4, (uint32_t)size.x * 4 * size.y };
		tex = afCreateTexture2D(AFF_B8G8R8A8_UNORM, desc, 1, &subresource);
	}
	jniEnv->ReleaseByteArrayElements(array, byteArray, 0);
	return tex;
}

void MakeFontBitmap(const char* fontName, const CharSignature& sig, DIB& dib, CharDesc& cache)
{
	int code = sig.code;

	jclass myview = jniEnv->FindClass(boundJavaClass);
	jmethodID method = jniEnv->GetStaticMethodID(myview, "makeFontBitmap", "(Ljava/lang/String;Ljava/lang/String;I[I)[B");
	//	jmethodID method = jniEnv->GetStaticMethodID(myview, "makeFontBitmap", "(Ljava/lang/String;II)[B");
	if (method == 0) {
		aflog("Java method not found!");
		afVerify(false);
	}

	jchar codeInUnicode[2] = { (jchar)code, 0 };

	jintArray descArray = jniEnv->NewIntArray(5);
	jobject arrayAsJObject = jniEnv->CallStaticObjectMethod(myview, method, jniEnv->NewStringUTF(fontName), jniEnv->NewString(codeInUnicode, 1), (jint)sig.fontSize, descArray);
	//	aflog("Java method called and returned");

	{
		jint* pDescArray = jniEnv->GetIntArrayElements(descArray, NULL);
		cache.distDelta = Vec2(pDescArray[0], pDescArray[1]);
		cache.srcWidth = Vec2(pDescArray[2], pDescArray[3]);
		cache.step = pDescArray[4];
		jniEnv->ReleaseIntArrayElements(descArray, pDescArray, 0);
	}
	if (!arrayAsJObject) {
		//		aflog("Java method returned null; it's white space");
		return;
	}

	jbyteArray array = (jbyteArray)arrayAsJObject;
	jbyte* byteArray = jniEnv->GetByteArrayElements(array, NULL);
	jsize arrayLen = jniEnv->GetArrayLength(array);
	//	aflog("arrayLen=%d", arrayLen);

	int expectedLen = cache.srcWidth.x * cache.srcWidth.y * 4;
	if (arrayLen != expectedLen) {
		//		aflog("wrong size! returned=%d expected=%d", arrayLen, expectedLen);
		afVerify(false);
	} else {
		dib.Create(cache.srcWidth.x, cache.srcWidth.y, 32);
		memcpy(dib.ReferPixels(), byteArray, arrayLen);
	}
	jniEnv->ReleaseByteArrayElements(array, byteArray, 0);
	dib.DibToDXFont();
}
