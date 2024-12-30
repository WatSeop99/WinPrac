#include "pch.h"
#include "TypeDef.h"

typedef BOOL(APIENTRY *PFnDLLMain)(HMODULE, DWORD, LPVOID);
typedef BOOL(WINAPI *PFnStrecthFunc)(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
typedef int (*PFnCallbackFunc)(FSP_EXTENSION_PARAM*);

HMODULE g_HookingModule = nullptr;
DLL_DIRECTORY_COOKIE g_pfnDLLCookie = nullptr;
PFnStrecthFunc g_pfnOriginStretchBlt = nullptr;
PFnCallbackFunc g_pfnCallbackStretchBlt = nullptr;

HMODULE ModuleFromAddress(void* pV);
BOOL WINAPI MyStretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop);
void ReplaceIATEntryInOneMod(PCSTR pszCalleeModName, PFnStrecthFunc pfnCurrent, PFnStrecthFunc pfnNew, HMODULE hmodCaller);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
//#ifndef _WIN64
//			const WCHAR* pszHOOKING_MODULE_PATH = L"D:\\workspace\\test\\MyDLL\\Debug\\MyDLL.dll";
//#else
//			const WCHAR* pszHOOKING_MODULE_PATH = L"D:\\workspace\\test\\MyDLL\\x64\\Debug\\MyDLL.dll";
//#endif
			//if (!CopyFile(pszHOOKING_MODULE_PATH, L"./Hooking.dll", FALSE))
			//{
			//	return FALSE;
			//}

			const WCHAR* pszHOOKING_MODULE_PATH = L"./MyDLL.dll";

			_ASSERT(!g_HookingModule);
			g_HookingModule = LoadLibrary(pszHOOKING_MODULE_PATH);
			if (!g_HookingModule)
			{
				return FALSE;
			}


			PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_HookingModule;
			PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pDosHeader + (DWORD)pDosHeader->e_lfanew);

			PFnDLLMain pfnDllMain = (PFnDLLMain)((LPBYTE)g_HookingModule + pNTHeader->OptionalHeader.AddressOfEntryPoint);

			DWORD_PTR dwAddr = 0;
			if (pfnDllMain(g_HookingModule, DLL_PROCESS_GET_PROCEDURE, &dwAddr) && dwAddr)
			{
				g_pfnCallbackStretchBlt = (PFnCallbackFunc)dwAddr;
			}
			else
			{
				return FALSE;
			}


			const WCHAR* pszTARGET_MODULE_NAME = L"Gdi32.dll";
			g_pfnOriginStretchBlt = (PFnStrecthFunc)GetProcAddress(GetModuleHandle(pszTARGET_MODULE_NAME), "StretchBlt");
			if (!g_pfnOriginStretchBlt)
			{
				return FALSE;
			}

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
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
				DetourDetach(&(PVOID&)g_pfnOriginStretchBlt, MyStretchBlt);
				DetourTransactionCommit();


				PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)g_HookingModule;
				PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pDosHeader + (DWORD)pDosHeader->e_lfanew);

				PFnDLLMain pfnDllMain = (PFnDLLMain)((LPBYTE)g_HookingModule + pNTHeader->OptionalHeader.AddressOfEntryPoint);

				if (!pfnDllMain(g_HookingModule, DLL_PROCESS_RELEASE_PROCEDURE, 0))
				{
					return FALSE;
				}

				if (!FreeLibrary(g_HookingModule))
				{
					return FALSE;
				}
				g_HookingModule = nullptr;

				DeleteFile(L"./Hooking.dll");
			}

			break;
		}
	}

	return TRUE;
}

void ReplaceIATEntryInOneMod(PCSTR pszCalleeModName, PFnStrecthFunc pfnCurrent, PFnStrecthFunc pfnNew, HMODULE hmodCaller)
{
	_ASSERT(pszCalleeModName);
	_ASSERT(pfnCurrent);
	_ASSERT(pfnNew);
	_ASSERT(hmodCaller);

	BYTE* pImageBase = (BYTE*)hmodCaller;
	if (!pImageBase)
	{
		__debugbreak();
	}

	PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER)pImageBase;
	if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
	{
		__debugbreak();
	}

	PIMAGE_NT_HEADERS pINH = (PIMAGE_NT_HEADERS)(pImageBase + pIDH->e_lfanew);
	if (pINH->Signature != IMAGE_NT_SIGNATURE)
	{
		__debugbreak();
	}

	PIMAGE_DATA_DIRECTORY pIDD = &pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR pIID = (PIMAGE_IMPORT_DESCRIPTOR)(pImageBase + pIDD->VirtualAddress);
	for (; pIID->Name; ++pIID)
	{
		const char* pszMOD_NAME = (const char*)(pImageBase + pIID->Name);
		if (_strcmpi(pszMOD_NAME, pszCalleeModName) == 0)
		{
			break;
		}
	}
	if (!pIID->Name)
	{
		__debugbreak();
	}

	bool bFinished = false;
	PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)(pImageBase + pIID->FirstThunk);
	for (; pThunk->u1.Function; ++pThunk)
	{
		PFnStrecthFunc* ppfn = (PFnStrecthFunc*)&pThunk->u1.Function;
		if (*ppfn == pfnCurrent)
		{
			if (!WriteProcessMemory(GetCurrentProcess(), ppfn, &pfnNew, sizeof(pfnNew), nullptr))
			{
				DWORD dwOldProtect;
				if (VirtualProtect(ppfn, sizeof(pfnNew), PAGE_WRITECOPY, &dwOldProtect))
				{
					WriteProcessMemory(GetCurrentProcess(), ppfn, &pfnNew, sizeof(pfnNew), nullptr);
					VirtualProtect(ppfn, sizeof(pfnNew), dwOldProtect, &dwOldProtect);
					bFinished = true;
				}
				else
				{
					__debugbreak();
				}
			}
			else
			{
				bFinished = true;
			}

			break;
		}
	}

	if (!bFinished)
	{
		__debugbreak();
	}
}

HMODULE ModuleFromAddress(void* pV)
{
	MEMORY_BASIC_INFORMATION mbi = {};
	return (VirtualQuery(pV, &mbi, sizeof(mbi)) != 0 ? (HMODULE)mbi.AllocationBase : nullptr);
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
