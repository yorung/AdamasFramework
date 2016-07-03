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

SRVID LoadTextureViaOS(const char* name, IVec2& size)
{
	jclass myview = jniEnv->FindClass(boundJavaClass);
	jmethodID method = jniEnv->GetStaticMethodID(myview, "loadTexture", "(Ljava/lang/String;)I");
	if (method == 0) {
		return SRVID();
	}
	SRVID id;
	id.x = jniEnv->CallStaticIntMethod(myview, method, jniEnv->NewStringUTF(name));
	size = afGetTextureSize(id);
	return id;
}

void MakeFontBitmap(const char* fontName, const CharSignature& sig, DIB& dib, CharDesc& cache)
{
	int code = sig.code;

	jclass myview = jniEnv->FindClass(boundJavaClass);
	jmethodID method = jniEnv->GetStaticMethodID(myview, "makeFontBitmap", "(Ljava/lang/String;Ljava/lang/String;I[F)[B");
	//	jmethodID method = jniEnv->GetStaticMethodID(myview, "makeFontBitmap", "(Ljava/lang/String;II)[B");
	if (method == 0) {
		aflog("Java method not found!");
		afVerify(false);
	}

	jchar codeInUnicode[2] = { (jchar)code, 0 };

	jfloatArray floatArray = jniEnv->NewFloatArray(5);
	jobject arrayAsJObject = jniEnv->CallStaticObjectMethod(myview, method, jniEnv->NewStringUTF(fontName), jniEnv->NewString(codeInUnicode, 1), (jint)sig.fontSize, floatArray);
	//	aflog("Java method called and returned");

	{
		jfloat* pFloatArray = jniEnv->GetFloatArrayElements(floatArray, NULL);
		cache.distDelta = Vec2(pFloatArray[0], pFloatArray[1]);
		cache.srcWidth = Vec2(pFloatArray[2], pFloatArray[3]);
		cache.step = pFloatArray[4];
		jniEnv->ReleaseFloatArrayElements(floatArray, pFloatArray, 0);
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
