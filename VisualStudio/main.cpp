// WGLTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

#pragma comment(lib, "opengl32.lib")

HGLRC hglrc;

static void err(char *msg)
{
	puts(msg);
}

#ifdef _DEBUG
static void DumpInfo()
{
	puts((char*)glGetString(GL_VERSION));
	puts((char*)glGetString(GL_RENDERER));
	puts((char*)glGetString(GL_VENDOR));
	puts((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	const GLubyte* ext = glGetString(GL_EXTENSIONS);
	do {
		printf("%c", *ext == ' ' ? '\n' : *ext);
	}while(*ext++);
}

static void APIENTRY debugMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	puts(message);
}
#endif

void CreateWGL(HWND hWnd)
{
	HDC hdc = GetDC(hWnd);

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
	if (!(pixelFormat = ChoosePixelFormat(hdc, &pfd))){
		err("ChoosePixelFormat failed.");
		goto END;
	}
	if (!SetPixelFormat(hdc, pixelFormat, &pfd)){
		err("SetPixelFormat failed.");
		goto END;
	}
	if (!(hglrc = wglCreateContext(hdc))){
		err("wglCreateContext failed.");
		goto END;
	}
	if (!wglMakeCurrent(hdc, hglrc)){
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
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
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
	if (!wglMakeCurrent(hdc, hglrc)){
		err("wglMakeCurrent failed.");
		goto END;
	}
#ifdef _DEBUG
	DumpInfo();
	glDebugMessageCallbackARB(debugMessageHandler, nullptr);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
#endif
END:
//	ReleaseDC(hWnd, hdc);	// do not release dc; WGL using it
	return;	// do nothing
}

void DestroyWGL(HWND hWnd)
{
	HDC hdc = wglGetCurrentDC();
	if (hdc) {
		ReleaseDC(hWnd, hdc);
	}
	wglMakeCurrent(nullptr, nullptr);
	if (hglrc) {
		wglDeleteContext(hglrc);
		hglrc = nullptr;
	}
}

#define MAX_LOADSTRING 100

// Global Variables:
HWND hWnd;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// WindowMessage
static BOOL ProcessWindowMessage(){
	MSG msg;
	for (;;){
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			if (msg.message == WM_QUIT){
				return FALSE;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		BOOL active = !IsIconic(hWnd) && GetForegroundWindow() == hWnd;

		if (!active){
			WaitMessage();
			continue;
		}

		return TRUE;
	}
}

static void Input()
{
	static bool last;
	bool current = !!(GetKeyState(VK_LBUTTON) & 0x80);
	bool edge = current && !last;
	last = current;

	if (edge) {
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(GetForegroundWindow(), &pt);

		RECT rc;
		GetClientRect(hWnd, &rc);
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;

		app.CreateRipple((float)pt.x / w * 2 - 1, (float)pt.y / h * -2 + 1);
	}
}

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


#ifdef _DEBUG
int main(int, char**)
#else
int APIENTRY wWinMain(_In_ HINSTANCE,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int)
#endif

{
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	
	// TODO: Place code here.
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WGLTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WGLTEST));

	GoMyDir();
	SetCurrentDirectoryA("../assets");
	CreateWGL(hWnd);
//	app.Create();

	int lastW = 0;
	int lastH = 0;

	// Main message loop:
	for (;;) {
		if (!ProcessWindowMessage()) {
			break;
		}
		Input();

		RECT rc;
		GetClientRect(hWnd, &rc);
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;
		if (w != lastW || h != lastH) {
			lastW = w;
			lastH = h;
			hub.Destroy();
			hub.Init(lastW, lastH);
		}
		float aspect = (float)w / (float)h;
		hub.Update(w, h, 0.5);
		SwapBuffers(wglGetCurrentDC());
		Sleep(1);
	}
	return 0;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WGLTEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WGLTEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_SHOWNORMAL);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_CLOSE:
		hub.Destroy();
		DestroyWGL(hWnd);
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
