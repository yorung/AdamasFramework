class AFApp
{
public:
	virtual void Create() {}
	virtual void Update() {}
	virtual void Destroy() {}
	virtual void MouseWheel(float /*rate*/) {}
	virtual void LButtonDown(int /*x*/, int /*y*/) {}
	virtual void LButtonUp(int /*x*/, int /*y*/) {}
	virtual void RButtonDown(int /*x*/, int /*y*/) {}
	virtual void RButtonUp(int /*x*/, int /*y*/) {}
	virtual void MouseMove(int /*x*/, int /*y*/) {}
	static AFApp* (*Generator)();
};

extern std::string g_type;
