#pragma once

// GDI-plus header files
#pragma comment(lib, "gdiplus")
#include <ole2.h>
#include <gdiplus.h>

#define MY_DLL_API extern "C"

extern HHOOK g_hHook;

MY_DLL_API int CallbackStretchBlt(LPFSP_EXTENSION_PARAM lpParam);

DWORD_PTR GetProcedure();

bool InitializeModule();
bool CleanupModule();
