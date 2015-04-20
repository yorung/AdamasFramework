class SystemMetrics
{
public:
	ivec2 screenSize;
	ivec2 mousePos;
	bool mouseDown;
	void SetScreenSize(ivec2 size) { screenSize = size; }
	const ivec2& GetScreenSize() { return screenSize; }
	void SetMousePos(ivec2 pos) { mousePos = pos; }
	const ivec2& GetMousePos() { return mousePos; }
};
extern SystemMetrics systemMetrics;
