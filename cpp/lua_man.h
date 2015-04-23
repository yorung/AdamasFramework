class LuaMan
{
	lua_State* L;
	void CallGlobal(const char* func);
public:
	LuaMan();
	~LuaMan();
	void Create();
	void Destroy();
	void Update();
	void DrawSprites();
	lua_State* GetState() { return L; }
};

typedef std::vector<void(*)(lua_State* L)> LuaBindFuncContainer;
LuaBindFuncContainer& GetLuaBindFuncContainer();

extern LuaMan luaMan;
