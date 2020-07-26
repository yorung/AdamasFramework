#pragma once

void _aflDumpStack(lua_State* L, const char* func, int line);
#ifndef NDEBUG
#define aflDumpStack() _aflDumpStack(L, __FUNCTION__, __LINE__)
#define aflDumpStackL(L) _aflDumpStack(L, __FUNCTION__, __LINE__)
#else
#define aflDumpStack()
#define aflDumpStackL(L)
#endif

void aflBindClass(lua_State* L, const char* className, luaL_Reg methods[], lua_CFunction creator);
void aflBindNamespace(lua_State* L, const char* nameSpace, luaL_Reg methods[]);
bool aflDoFile(lua_State* L, const char* fileName);
int aflDoFileForReplace(lua_State* L);
int aflPrintForReplace(lua_State* L);

void aflPushIVec2(lua_State* L, const IVec2& pt);
void aflPushVec2(lua_State* L, const Vec2& pt);
