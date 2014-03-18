#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define BOX2SF 64.f
#define SF2BOX 1.f/64.f
#define TIMESTEP 1.f/60.f
#define PI 3.14159

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "LuaBridge.h"

#endif