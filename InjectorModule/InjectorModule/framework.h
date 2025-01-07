#pragma once

#pragma comment(lib, "detours.lib")

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <Windows.h>
#include <libloaderapi.h>
#include <DbgHelp.h>

#include <stdio.h>
#include <crtdbg.h>
#include <math.h>
#include <wchar.h>

#include <detours.h>
