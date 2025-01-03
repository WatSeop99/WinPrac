#pragma once

#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "detours.lib")

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <Windows.h>
#include <libloaderapi.h>
#include <TlHelp32.h>
#include <DbgHelp.h>

#include <crtdbg.h>

#include <detours.h>
