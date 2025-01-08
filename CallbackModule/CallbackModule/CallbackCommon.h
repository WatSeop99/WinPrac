#pragma once

// GDI-plus header files
#pragma comment(lib, "gdiplus")
#include <ole2.h>
#include <gdiplus.h>

#define MY_DLL_API extern "C"

/*
* StretchBlt 콜백 처리 함수.
*/
MY_DLL_API int CallbackStretchBlt(LPFSP_EXTENSION_PARAM lpParam);
/*
* BitBlt 콜백 처리 함수.
*/
MY_DLL_API int CallbackBitBlt(LPFSP_EXTENSION_PARAM lpParam);

/*
* 콜백 처리 함수 반환.
*/
DWORD_PTR GetProcedure(const DWORD PROCEDURE_TYPE);


bool InitializeModule(HMODULE hModule);
bool CleanupModule();
