void _aflDumpStack(lua_State* L, const char* func, int line);
#ifdef _DEBUG
#define aflDumpStack() _aflDumpStack(L, __FUNCTION__, __LINE__)
#define aflDumpStackL(L) _aflDumpStack(L, __FUNCTION__, __LINE__)
#else
#define aflDumpStack()
#define aflDumpStackL(L)
#endif

void aflBindClass(lua_State* L, const char* className, luaL_Reg methods[], lua_CFunction creator);
bool aflDoFile(lua_State* L, const char* fileName);
