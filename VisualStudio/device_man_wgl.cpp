#include "stdafx.h"
#include "device_man_wgl.h"

#pragma comment(lib, "opengl32.lib")

// Tell the NVIDIA driver to choose NVIDIA GPU
extern "C" _declspec(dllexport) DWORD NvOptimusEnablement = 1;

DeviceManWgl deviceManWgl;

static void err(char *msg)
{
	puts(msg);
}

#ifdef _DEBUG
static void APIENTRY debugMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	switch (type) {
	case GL_DEBUG_TYPE_OTHER_ARB:
		return;
	}
	puts(message);
}
#endif

void DeviceManWgl::CreateWGLInternal(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int pixelFormat;
	if (!(pixelFormat = ChoosePixelFormat(hdc, &pfd))) {
		err("ChoosePixelFormat failed.");
		goto END;
	}
	if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
		err("SetPixelFormat failed.");
		goto END;
	}
	if (!(hglrc = wglCreateContext(hdc))) {
		err("wglCreateContext failed.");
		goto END;
	}
	if (!wglMakeCurrent(hdc, hglrc)) {
		err("wglMakeCurrent failed.");
		goto END;
	}
#if 0 // for test
	void* glpu = wglGetProcAddress("glProgramUniform1f");
	void* gldei = wglGetProcAddress("glDrawElementsIndirect");
	void* glen1 = glEnable;
	void* glen2 = wglGetProcAddress("glEnable");
	GLuint(APIENTRY*glCreateProgram)(void);
	glCreateProgram = (GLuint(APIENTRY*)(void))wglGetProcAddress("glCreateProgram");
	GLuint(APIENTRY*glCreateShader)(GLenum type);
	glCreateShader = (GLuint(APIENTRY*)(GLenum type))wglGetProcAddress("glCreateShader");
	GLuint test = glCreateShader(GL_VERTEX_SHADER);
	void(APIENTRY*glDeleteShader)(GLuint name);
	glDeleteShader = (void(APIENTRY*)(GLuint name))wglGetProcAddress("glDeleteShader");
	glDeleteShader(test);
#endif
	WGLGrabberInit();
	int flags = 0;
#ifdef _DEBUG
	flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif
	static const int attribList[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 1,
		WGL_CONTEXT_FLAGS_ARB, flags,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_ES2_PROFILE_BIT_EXT,
		0,
	};
	HGLRC hglrcNew = wglCreateContextAttribsARB(hdc, 0, attribList);

	wglMakeCurrent(nullptr, nullptr);
	if (hglrcNew) {
		wglDeleteContext(hglrc);
		hglrc = hglrcNew;
	}
	if (!wglMakeCurrent(hdc, hglrc)) {
		err("wglMakeCurrent failed.");
		goto END;
	}
#ifdef _DEBUG
	afDumpCaps();
	afDumpIsEnabled();
	glDebugMessageCallbackARB(debugMessageHandler, nullptr);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
#endif
END:
	//	ReleaseDC(hWnd, hdc);	// do not release dc; WGL using it
	return ;	// do nothing
}

void DeviceManWgl::Create(HWND hWnd)
{
	HDC hdc = GetDC(hWnd);
	dcDeleter = [hWnd](HDC hdc) {
		int r = ReleaseDC(hWnd, hdc);
		assert(r == 1);
	};
	CreateWGLInternal(hdc);
}

void DeviceManWgl::Destroy()
{
	HDC hdc = wglGetCurrentDC();
	if (hdc) {
		dcDeleter(hdc);
	}
	wglMakeCurrent(nullptr, nullptr);
	if (hglrc) {
		wglDeleteContext(hglrc);
		hglrc = nullptr;
	}
}

void DeviceManWgl::Present()
{
	SwapBuffers(wglGetCurrentDC());
}
