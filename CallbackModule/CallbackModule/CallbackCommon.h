#pragma once

// GDI-plus header files
#pragma comment(lib, "gdiplus")
#include <ole2.h>
#include <gdiplus.h>

#define MY_DLL_API extern "C"

/*
* StretchBlt �ݹ� ó�� �Լ�.
*/
MY_DLL_API int CallbackStretchBlt(LPFSP_EXTENSION_PARAM lpParam);
/*
* BitBlt �ݹ� ó�� �Լ�.
*/
MY_DLL_API int CallbackBitBlt(LPFSP_EXTENSION_PARAM lpParam);

/*
* �ݹ� ó�� �Լ� ��ȯ.
*/
DWORD_PTR GetProcedure(const DWORD PROCEDURE_TYPE);


bool InitializeModule(HMODULE hModule);
bool CleanupModule();
