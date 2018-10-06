class AFApp
{
public:
	virtual void Create() = 0;
	virtual void Update() = 0;
	virtual void Destroy() = 0;
	static AFApp* (*Generator)();
};

extern std::string g_type;
