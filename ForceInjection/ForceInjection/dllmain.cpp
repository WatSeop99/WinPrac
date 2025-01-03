#include "pch.h"
#include "TypeDef.h"

HMODULE g_HookingModule = nullptr;

// StretchBlt pointer.
PFnStrecthFunc g_pfnOriginStretchBlt = nullptr;
PFnCallbackFunc g_pfnCallbackStretchBlt = nullptr;
// Bitblt pointer.
PFnBitBltFunc g_pfnOriginBitBlt = nullptr;
PFnCallbackFunc g_pfnCallbackBitBlt = nullptr;

bool GetCallbackModulePath(HMODULE hModule, WCHAR* pOutPath);
BOOL WINAPI MyStretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop);
BOOL WINAPI MyBitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			// 콜백 모듈 경로 가져옴.
			WCHAR szCallbackModulePath[MAX_PATH];
			if (!GetCallbackModulePath(hModule, szCallbackModulePath))
			{
				//DEBUG_BREAK;
				MessageBox(nullptr, L"Failed in GetCallbackModulePath", L"Error", MB_OK);
				return FALSE;
			}
			
			// 콜백 모듈 로드.
			_ASSERT(!g_HookingModule);
			g_HookingModule = LoadLibraryW(szCallbackModulePath);
			if (!g_HookingModule)
			{
				//DEBUG_BREAK;
				MessageBox(nullptr, L"Failed in LoadLibraryW-Injector", L"Error", MB_OK);
				return FALSE;
			}


			// 모듈 헤더 처리.
			PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_HookingModule;
			PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pDosHeader + (DWORD)pDosHeader->e_lfanew);

			// 실행 모듈로부터 entry point 주소 가져옴.
			PFnDLLMain pfnDllMain = (PFnDLLMain)((LPBYTE)g_HookingModule + pNTHeader->OptionalHeader.AddressOfEntryPoint);

			// 콜백함수 가져옴.
			DWORD_PTR dwAddr = 0;
			BOOL bResult = FALSE;

			bResult = pfnDllMain(g_HookingModule, (DLL_PROCESS_GET_PROCEDURE | DLL_PROCEDURE_STRETCHBLT), &dwAddr);
			if (!bResult || dwAddr == 0)
			{
				DEBUG_BREAK;
				return FALSE;
			}
			g_pfnCallbackStretchBlt = (PFnCallbackFunc)dwAddr;

			dwAddr = 0;
			bResult = pfnDllMain(g_HookingModule, (DLL_PROCESS_GET_PROCEDURE | DLL_PROCEDURE_BITBLT), &dwAddr);
			if (!bResult || dwAddr == 0)
			{
				//DEBUG_BREAK;
				MessageBox(nullptr, L"Failed in DllMain call-Injector", L"Error", MB_OK);
				return FALSE;
			}
			g_pfnCallbackBitBlt = (PFnCallbackFunc)dwAddr;

			// StretchBlt, BitBlt의 원형 주소 가져옴.
			/*_ASSERT(g_pfnOriginStretchBlt == nullptr);
			_ASSERT(g_pfnOriginBitBlt == nullptr);*/

			const WCHAR* pszTARGET_MODULE_NAME = L"Gdi32.dll";
			g_pfnOriginStretchBlt = (PFnStrecthFunc)GetProcAddress(GetModuleHandleW(pszTARGET_MODULE_NAME), "StretchBlt");
			if (g_pfnOriginStretchBlt == nullptr)
			{
				DEBUG_BREAK;
				return FALSE;
			}
			g_pfnOriginBitBlt = (PFnBitBltFunc)GetProcAddress(GetModuleHandleW(pszTARGET_MODULE_NAME), "BitBlt");
			if (g_pfnOriginBitBlt == nullptr)
			{
				MessageBox(nullptr, L"Failed in GetProcAddress-Injector", L"Error", MB_OK);
				return FALSE;
			}

			// Detour를 통해 콜백 등록.
			DetourRestoreAfterWith();
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)g_pfnOriginStretchBlt, MyStretchBlt);
			DetourAttach(&(PVOID&)g_pfnOriginBitBlt, MyBitBlt);
			DetourTransactionCommit();

			break;
		}

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
		{
			if (g_HookingModule)
			{
				// Detour 콜백 해제.
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
				DetourDetach(&(PVOID&)g_pfnOriginStretchBlt, MyStretchBlt);
				DetourDetach(&(PVOID&)g_pfnOriginBitBlt, MyBitBlt);
				DetourTransactionCommit();

				// 콜백 모듈 해제.
				if (!FreeLibrary(g_HookingModule))
				{
					return FALSE;
				}
				g_HookingModule = nullptr;

				// 콜백 관련 함수 해제.
				g_pfnOriginStretchBlt = nullptr;
				g_pfnCallbackStretchBlt = nullptr;
				g_pfnOriginBitBlt = nullptr;
				g_pfnCallbackBitBlt = nullptr;
			}

			break;
		}
	}

	return TRUE;
}

bool GetCallbackModulePath(HMODULE hModule, WCHAR* pOutPath)
{
	_ASSERT(hModule);
	_ASSERT(pOutPath);

	if (hModule == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	// 현재 모듈의 원 파일 경로를 가져옴.
	WCHAR szCurrentFilePath[MAX_PATH];
	WCHAR* pszFilePart = nullptr;
	GetModuleFileNameW(hModule, szCurrentFilePath, MAX_PATH);
	pszFilePart = wcsrchr(szCurrentFilePath, '\\');

	// 경로만 복사.
	wcsncpy_s(pOutPath, MAX_PATH, szCurrentFilePath, pszFilePart - szCurrentFilePath);

	// 콜백 모듈 경로 붙임.
	wcsncat_s(pOutPath, MAX_PATH, L"\\MyDLL.dll", wcslen(L"\\MyDLL.dll"));

	return true;
}


BOOL WINAPI MyStretchBlt(HDC hdcDest,
						 int xDest, int yDest, 
						 int wDest, int hDest,
						 HDC hdcSrc, 
						 int xSrc, int ySrc, 
						 int wSrc, int hSrc,
						 DWORD rop)
{
	_ASSERT(g_pfnOriginStretchBlt);
	_ASSERT(g_pfnCallbackStretchBlt);

	FSP_EXTENSION_PARAM param = { 0, };
	param.dwParam1 = (DWORD_PTR)MyStretchBlt;
	param.dwParam2 = (DWORD_PTR)g_pfnOriginStretchBlt;
	param.dwParam3 = (DWORD_PTR)hdcDest;
	param.dwParam4 = (DWORD_PTR)xDest;
	param.dwParam5 = (DWORD_PTR)yDest;
	param.dwParam6 = (DWORD_PTR)wDest;
	param.dwParam7 = (DWORD_PTR)hDest;
	param.dwParam8 = (DWORD_PTR)hdcSrc;
	param.dwParam9 = (DWORD_PTR)xSrc;
	param.dwParam10 = (DWORD_PTR)ySrc;
	param.dwParam11 = (DWORD_PTR)wSrc;
	param.dwParam12 = (DWORD_PTR)hSrc;
	param.dwParam13 = (DWORD_PTR)rop;

	if (!g_pfnCallbackStretchBlt(&param))
	{
		return FALSE;
	}

	if ((int)param.dwParam1 == FSP_EXTRET_VALUE)
	{
		return (BOOL)param.dwParam2;
	}

	return TRUE;
}
BOOL WINAPI MyBitBlt(HDC hdc, 
					 int x, int y, 
					 int cx, int cy, 
					 HDC hdcSrc, 
					 int x1, int y1, 
					 DWORD rop)
{
	_ASSERT(g_pfnOriginBitBlt);
	_ASSERT(g_pfnCallbackBitBlt);

	FSP_EXTENSION_PARAM param = { 0, };
	param.dwParam1 = (DWORD_PTR)MyBitBlt;
	param.dwParam2 = (DWORD_PTR)g_pfnOriginBitBlt;
	param.dwParam3 = (DWORD_PTR)hdc;
	param.dwParam4 = (DWORD_PTR)x;
	param.dwParam5 = (DWORD_PTR)y;
	param.dwParam6 = (DWORD_PTR)cx;
	param.dwParam7 = (DWORD_PTR)cy;
	param.dwParam8 = (DWORD_PTR)hdcSrc;
	param.dwParam9 = (DWORD_PTR)x1;
	param.dwParam10 = (DWORD_PTR)y1;
	param.dwParam11 = (DWORD_PTR)rop;

	if (!g_pfnCallbackBitBlt(&param))
	{
		return FALSE;
	}

	if ((int)param.dwParam1 == FSP_EXTRET_VALUE)
	{
		return (BOOL)param.dwParam2;
	}

	return TRUE;
}
