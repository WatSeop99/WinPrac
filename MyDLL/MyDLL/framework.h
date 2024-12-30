#pragma once

#pragma comment(lib,"msimg32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "dwmapi.lib")

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <Windows.h>
#include <process.h>
#include <TlHelp32.h>
#include <ole2.h>
#include <gdiplus.h>
#include <dwmapi.h>
#include <psapi.h>

#include <crtdbg.h>
#include <stdio.h>
#include <math.h>

#include <vector>
#include <unordered_set>
#include <unordered_map>
