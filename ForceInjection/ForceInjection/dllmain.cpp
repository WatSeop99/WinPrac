#include "pch.h"
#include "TypeDef.h"

HMODULE g_HookingModule = nullptr;
DLL_DIRECTORY_COOKIE g_pfnDLLCookie = nullptr;
PFnStrecthFunc g_pfnOriginStretchBlt = nullptr;
PFnCallbackFunc g_pfnCallbackStretchBlt = nullptr;

bool GetCallbackModulePath(HMODULE hModule, WCHAR* pOutPath);
BOOL WINAPI MyStretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			// �ݹ� ��� ��� ������.
			WCHAR szCallbackModulePath[MAX_PATH];
			if (!GetCallbackModulePath(hModule, szCallbackModulePath))
			{
				__debugbreak();
				return FALSE;
			}

			// �ݹ� ��� �ε�.
			_ASSERT(!g_HookingModule);
			g_HookingModule = LoadLibrary(szCallbackModulePath);
			if (!g_HookingModule)
			{
				__debugbreak();
				return FALSE;
			}


			PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_HookingModule;
			PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pDosHeader + (DWORD)pDosHeader->e_lfanew);

			// ���� ���κ��� entry point �ּ� ������.
			PFnDLLMain pfnDllMain = (PFnDLLMain)((LPBYTE)g_HookingModule + pNTHeader->OptionalHeader.AddressOfEntryPoint);

			// �ݹ��Լ� ������.
			DWORD_PTR dwAddr = 0;
			if (pfnDllMain(g_HookingModule, DLL_PROCESS_GET_PROCEDURE, &dwAddr) && dwAddr)
			{
				g_pfnCallbackStretchBlt = (PFnCallbackFunc)dwAddr;
			}
			else
			{
				__debugbreak();
				return FALSE;
			}

			// StretchBlt�� ���� �ּ� ������.
			const WCHAR* pszTARGET_MODULE_NAME = L"Gdi32.dll";
			g_pfnOriginStretchBlt = (PFnStrecthFunc)GetProcAddress(GetModuleHandle(pszTARGET_MODULE_NAME), "StretchBlt");
			if (!g_pfnOriginStretchBlt)
			{
				__debugbreak();
				return FALSE;
			}

			// Detour�� ���� �ݹ� ���.
			DetourRestoreAfterWith();
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)g_pfnOriginStretchBlt, MyStretchBlt);
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
				// Detour �ݹ� ����.
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
				DetourDetach(&(PVOID&)g_pfnOriginStretchBlt, MyStretchBlt);
				DetourTransactionCommit();


				PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_HookingModule;
				PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pDosHeader + (DWORD)pDosHeader->e_lfanew);

				// �ݹ� ��� entrypoint �ּ� ������.
				PFnDLLMain pfnDllMain = (PFnDLLMain)((LPBYTE)g_HookingModule + pNTHeader->OptionalHeader.AddressOfEntryPoint);

				// �ݹ� ��� ����.
				if (!pfnDllMain(g_HookingModule, DLL_PROCESS_RELEASE_PROCEDURE, 0))
				{
					return FALSE;
				}

				if (!FreeLibrary(g_HookingModule))
				{
					return FALSE;
				}
				g_HookingModule = nullptr;
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

	// ���� ����� �� ���� ��θ� ������.
	WCHAR szCurrentFilePath[MAX_PATH];
	WCHAR* pszFilePart = nullptr;
	GetModuleFileNameW(hModule, szCurrentFilePath, MAX_PATH);
	pszFilePart = wcsrchr(szCurrentFilePath, '\\');

	// ��θ� ����.
	wcsncpy_s(pOutPath, MAX_PATH, szCurrentFilePath, pszFilePart - szCurrentFilePath);

	// �ݹ� ��� ��� ����.
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
