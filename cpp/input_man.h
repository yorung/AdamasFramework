class InputMan {
	int keyDownCount[256];
	void HandleEdge(int virtualKey, bool up);
public:
	InputMan();
	void Update();
	void Reset();
	int GetInputCount(int virtualKey);
#ifdef _MSC_VER
	BOOL HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif
};
extern InputMan inputMan;
