#pragma once

#ifndef HOOKING_API
#define HOOKING_API extern "C" __declspec(dllimport)
#endif

// 외부 노출 함수
HOOKING_API bool StartHook();
HOOKING_API bool StopHook();
