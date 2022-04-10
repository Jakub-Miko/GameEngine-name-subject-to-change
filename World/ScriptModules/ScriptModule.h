#pragma once
#include <LuaEngine.h>
#include <vector>

#define LUA_FUNCTION(name, func) {name, LuaEngine::Invoke<func>}