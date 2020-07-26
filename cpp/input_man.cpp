#include "stdafx.h"
#include "input_man.h"

InputMan inputMan;

InputMan::InputMan()
{
	Reset();
}

void InputMan::HandleEdge(int vk, bool up)
{
	assert(vk >= 0 && vk <= 0xff);
	keyDownCount[vk] = up ? -1 : 0;
}

void InputMan::Reset()
{
	for(int i = 0; i < (int)dimof(keyDownCount); i++) {
		keyDownCount[i] = -1;
	}
}

void InputMan::HandleTouch(bool touched)
{
	HandleEdge(0x01, !touched);	// VK_LBUTTON
}

#ifdef _MSC_VER
BOOL InputMan::HandleWindowMessage(HWND, UINT message, WPARAM wParam, LPARAM lParam)
{
	(void)lParam;
	switch (message){
	case WM_ACTIVATE:
		Reset();
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		HandleEdge(int(wParam & 0xff), message == WM_KEYUP);
		return TRUE;
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		switch ((int)wParam){
		case VK_F10:	// F10 is an system key
			HandleEdge(int(wParam & 0xff), message == WM_SYSKEYUP);
			return TRUE;
		case VK_MENU:	// prevent entering system menu loop when ALT key pressed
			return TRUE;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		HandleEdge(VK_LBUTTON, message == WM_LBUTTONUP);
		return TRUE;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		HandleEdge(VK_RBUTTON, message == WM_RBUTTONUP);
		return TRUE;
	}
	return FALSE;
}
#endif

void InputMan::Update()
{
	for (int i = 0; i < (int)dimof(keyDownCount); i++) {
		if (keyDownCount[i] >= 0) {
			keyDownCount[i]++;
		}
	}
}

int InputMan::GetInputCount(int vk)
{
	assert(vk >= 0 && vk <= 0xff);
	return keyDownCount[vk];
}
