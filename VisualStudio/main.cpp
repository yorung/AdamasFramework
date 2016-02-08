// WGLTest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

#pragma comment(lib, "opengl32.lib")

// Tell the NVIDIA driver to choose NVIDIA GPU
extern "C" _declspec(dllexport) DWORD NvOptimusEnablement = 1;

HGLRC hglrc;
std::function<void(HDC)> dcDeleter;

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

static void CreateWGLInternal(HDC hdc)
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
	if (!wglMakeCurrent(hdc, hglrc)){
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
	return;	// do nothing
}

static void DestroyWGL()
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

static void CreateWGLFromWindowDC(HWND hWnd)
{
	HDC hdc = GetDC(hWnd);
	dcDeleter = [hWnd](HDC hdc) {
		int r = ReleaseDC(hWnd, hdc);
		assert(r == 1);
	};
	CreateWGLInternal(hdc);
}

static void CreateNVWGL()
{
	HGPUNV hGpu[2] = { nullptr, nullptr };
	for (int i = 0; wglEnumGpusNV(i, &hGpu[0]); i++) {
		_GPU_DEVICE d = {sizeof(_GPU_DEVICE)};
		wglEnumGpuDevicesNV(hGpu[0], 0, &d);
//		wglCreateAffinityDCNV()
	}
}

#define MAX_LOADSTRING 100

// Global Variables:
HWND hWnd;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HACCEL hAccelTable;

static UINT g_itemId = 1000;
static std::map<int, std::string> g_menuTbl;

void PostCommand(const char* cmdString)
{
	if (!strcmp(cmdString, "exit")) {
		PostMessage(hWnd, WM_COMMAND, IDM_EXIT, 0);
	}
}

void AddMenu(const char *name, const char *cmd)
{
	HMENU hMenu = GetMenu(hWnd);
	MENUITEMINFOA mii;
	mii.cbSize = sizeof(MENUITEMINFOA);
	mii.fMask = MIIM_TYPE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.hSubMenu = NULL;
	mii.wID = g_itemId++;
	mii.dwTypeData = (char*)name;
	InsertMenuItemA(hMenu, g_menuTbl.size(), TRUE, &mii);
	g_menuTbl[mii.wID] = cmd;
	SetMenu(hWnd, hMenu);
	DrawMenuBar(hWnd);
}


static void ShowLastError()
{
	wchar_t* msg;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0, nullptr);
	MessageBox(hWnd, msg, L"", MB_OK);
	LocalFree(msg);
}

void ClearMenu()
{
	HMENU hMenu = GetMenu(hWnd);
	std::for_each(g_menuTbl.begin(), g_menuTbl.end(), [hMenu](std::pair<int, std::string> m)
	{
		BOOL r = RemoveMenu(hMenu, m.first, MF_BYCOMMAND);
		if (!r) {
			ShowLastError();
		}
	}
	);

	SetMenu(hWnd, hMenu);
	DrawMenuBar(hWnd);
	g_menuTbl.clear();
}

static void ProcessLuaCommandTbl(int cmd)
{
	auto it = g_menuTbl.find(cmd);
	if (it == g_menuTbl.end()) {
		return;
	}
	lua_State *L = luaMan.GetState();
	if (!L) {
		return;
	}
	if (luaL_dostring(L, it->second.c_str())) {
		const char *p = lua_tostring(L, -1);
		MessageBoxA(GetActiveWindow(), p, "luaL_dostring error", MB_OK);
	}
}

// WindowMessage
static BOOL ProcessWindowMessage(){
	MSG msg;
	for (;;){
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			if (msg.message == WM_QUIT){
				return FALSE;
			}
			if (!TranslateAccelerator(hWnd, hAccelTable, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		BOOL active = !IsIconic(hWnd) && GetForegroundWindow() == hWnd;

		if (!active){
			WaitMessage();
			continue;
		}

		return TRUE;
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

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ADAMAS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ADAMAS));

//	CreateNVWGL();
	CreateWGLFromWindowDC(hWnd);
//	app.Create();

	int lastW = 0;
	int lastH = 0;

	// Main message loop:
	for (;;) {
		if (!ProcessWindowMessage()) {
			break;
		}

		RECT rc;
		GetClientRect(hWnd, &rc);
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;
		if (w != lastW || h != lastH) {
			lastW = w;
			lastH = h;
			hub.Destroy();
			hub.Init();
		}
		float aspect = (float)w / (float)h;
		hub.Update();
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ADAMAS);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON));

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

   DragAcceptFiles(hWnd, TRUE);
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
	inputMan.HandleWindowMessage(hWnd, message, wParam, lParam);

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	ivec2 screenSize = systemMisc.GetScreenSize();
	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		ProcessLuaCommandTbl(wmId);

		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case IDM_RELOAD:
			hub.Destroy();
			hub.Init();
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
		DestroyWGL();
		DestroyWindow(hWnd);
		return 0;
	case WM_DROPFILES:
	{
		HDROP hDrop = (HDROP)wParam;
		char fileName[MAX_PATH];
		DragQueryFileA(hDrop, 0, fileName, MAX_PATH);
		DragFinish(hDrop);
		app.LoadMesh(fileName);
		break;
	}
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		devCamera.LButtonDown(LOWORD(lParam) / (float)screenSize.x, HIWORD(lParam) / (float)screenSize.y);
		systemMisc.mouseDown = true;
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		devCamera.LButtonUp(LOWORD(lParam) / (float)screenSize.x, HIWORD(lParam) / (float)screenSize.y);
		systemMisc.mouseDown = false;
		break;
	case WM_MOUSEMOVE:
		systemMisc.SetMousePos(ivec2(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y));
		devCamera.MouseMove(MAKEPOINTS(lParam).x / (float)screenSize.x, MAKEPOINTS(lParam).y / (float)screenSize.y);
		break;
	case WM_MOUSEWHEEL:
		devCamera.MouseWheel((short)HIWORD(wParam) / (float)WHEEL_DELTA);
		break;
	case WM_SIZE:
		systemMisc.SetScreenSize(ivec2(LOWORD(lParam), HIWORD(lParam)));
		break;
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
