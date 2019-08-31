#include "stdafx.h"
#include "resource.h"

#define MAX_LOADSTRING 100

// Global Variables:
static HWND s_hWnd;
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HACCEL hAccelTable;

static UINT g_itemId = 1000;
static std::map<int, std::string> g_menuTbl;

AFApp *app;

void PostCommand(const char* cmdString)
{
	if (!strcmp(cmdString, "exit")) {
		PostMessage(s_hWnd, WM_COMMAND, IDM_EXIT, 0);
	}
}

void AddMenu(const char *name, const char *cmd)
{
	HMENU hMenu = GetMenu(s_hWnd);
	MENUITEMINFOA mii;
	mii.cbSize = sizeof(MENUITEMINFOA);
	mii.fMask = MIIM_TYPE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.hSubMenu = NULL;
	mii.wID = g_itemId++;
	mii.dwTypeData = (char*)name;
	InsertMenuItemA(hMenu, (int)g_menuTbl.size(), TRUE, &mii);
	g_menuTbl[mii.wID] = cmd;
	SetMenu(s_hWnd, hMenu);
	DrawMenuBar(s_hWnd);
}

void ClearMenu()
{
	HMENU hMenu = GetMenu(s_hWnd);
	std::for_each(g_menuTbl.begin(), g_menuTbl.end(), [hMenu](std::pair<int, std::string> m)
	{
		BOOL r = RemoveMenu(hMenu, m.first, MF_BYCOMMAND);
		if (!r) {
			ShowLastWinAPIError();
		}
	}
	);

	SetMenu(s_hWnd, hMenu);
	DrawMenuBar(s_hWnd);
	g_menuTbl.clear();
}

static void ProcessLuaCommandTbl(int cmd)
{
	auto it = g_menuTbl.find(cmd);
	if (it == g_menuTbl.end()) {
		return;
	}
	lua_State *L = luaMan.GetState();
	if (!L)
	{
		return;
	}
	if (luaL_dostring(L, it->second.c_str()))
	{
		const char *p = lua_tostring(L, -1);
		MessageBoxA(GetActiveWindow(), p, "luaL_dostring error", MB_OK);
	}
	lua_gc(L, LUA_GCCOLLECT, 0);
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
	_In_opt_ HINSTANCE,
	_In_ LPTSTR,
	_In_ int)
#endif

{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ADAMAS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	app = AFApp::Generator();
	if (!InitInstance (hInstance))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ADAMAS));

	int lastW = 0;
	int lastH = 0;

	while (ProcessWindowMessage(s_hWnd, hAccelTable))
	{
		RECT rc;
		GetClientRect(s_hWnd, &rc);
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;
		if (w != lastW || h != lastH)
		{
			lastW = w;
			lastH = h;
			app->Destroy();
			deviceMan.Destroy();
			deviceMan.Create(s_hWnd);
			app->Create();
		}
		app->Update();
		deviceMan.Present();
		Sleep(1);
	}
	afSafeDelete(app);

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

   s_hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!s_hWnd)
   {
      return FALSE;
   }

   ShowWindow(s_hWnd, SW_SHOWNORMAL);
   UpdateWindow(s_hWnd);

   DragAcceptFiles(s_hWnd, TRUE);
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

	IVec2 screenSize = systemMisc.GetScreenSize();
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
			app->Destroy();
			app->Create();
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
		app->Destroy();
		deviceMan.Destroy();
		DestroyWindow(hWnd);
		return 0;
	case WM_DROPFILES:
	{
		HDROP hDrop = (HDROP)wParam;
		char fileName[MAX_PATH];
		DragQueryFileA(hDrop, 0, fileName, MAX_PATH);
		DragFinish(hDrop);
	//	app.LoadMesh(fileName);
		break;
	}
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		app->LButtonDown(LOWORD(lParam), HIWORD(lParam));
		systemMisc.mouseDown = true;
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		app->LButtonUp(LOWORD(lParam), HIWORD(lParam));
		systemMisc.mouseDown = false;
		break;
	case WM_RBUTTONDOWN:
		SetCapture(hWnd);
		app->RButtonDown(LOWORD(lParam), HIWORD(lParam));
		systemMisc.mouseDown = true;
		break;
	case WM_RBUTTONUP:
		ReleaseCapture();
		app->RButtonUp(LOWORD(lParam), HIWORD(lParam));
		systemMisc.mouseDown = false;
		break;
	case WM_MOUSEMOVE:
		systemMisc.SetMousePos(IVec2(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y));
		app->MouseMove(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_MOUSEWHEEL:
		app->MouseWheel((short)HIWORD(wParam) / (float)WHEEL_DELTA);
		break;
	case WM_SIZE:
		systemMisc.SetScreenSize(IVec2(LOWORD(lParam), HIWORD(lParam)));
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
