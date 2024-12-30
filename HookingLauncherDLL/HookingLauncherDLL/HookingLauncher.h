#pragma once

#ifndef HOOKING_API
#define HOOKING_API extern "C" __declspec(dllimport)
#endif

HOOKING_API bool StartHook();
HOOKING_API bool StopHook();
