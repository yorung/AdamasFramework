class DeviceManWgl
{
	HGLRC hglrc;
	std::function<void(HDC)> dcDeleter;
	void CreateWGLInternal(HDC hdc);
public:
	void Create(HWND hWnd);
	void Destroy();
	void Present();
};

extern DeviceManWgl deviceManWgl;
#define deviceMan deviceManWgl
