class LuaMan
{
	lua_State* L;
public:
	LuaMan();
	~LuaMan();
	void Create();
	void Destroy();
	void Update();
	lua_State* GetState() { return L; }
};

extern LuaMan luaMan;
