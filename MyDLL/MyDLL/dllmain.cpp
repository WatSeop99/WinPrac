#include "pch.h"
#include "TypeDef.h"
#include "MyDll.h"

BOOL DefaultDllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call < DLL_PROCESS_GET_NONE)
	{
		return DefaultDllMain(hModule, ul_reason_for_call, lpReserved);
	}

	switch (ul_reason_for_call & DLL_PROCESS_TYPE_MASK)
	{
		case DLL_PROCESS_GET_PROCEDURE:
		{
			if (lpReserved == nullptr)
			{
				return FALSE;
			}

			PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
			if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			{
				return FALSE;
			}

			PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDosHeader + pDosHeader->e_lfanew);
			if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
			{
				return FALSE;
			}

			void* pRetAddr = lpReserved;
			*((DWORD_PTR*)pRetAddr) = GetProcedure(ul_reason_for_call & DLL_PROCEDURE_TYPE_MASK);
			if (*((DWORD_PTR*)pRetAddr) == 0)
			{
				MessageBox(nullptr, L"Failed in GetProcedure-MyDLL", L"Error", MB_OK);
				return FALSE;
			}

			break;
		}

		default:
			DEBUG_BREAK;
			return FALSE;
	}

	return TRUE;
}

BOOL DefaultDllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
			if (!InitializeModule(hModule))
			{
				MessageBox(nullptr, L"Failed in InitializeModule-MyDLL", L"Error", MB_OK);
				return FALSE;
			}
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			if (!CleanupModule())
			{
				MessageBox(nullptr, L"Failed in CleanupModule-MyDLL", L"Error", MB_OK);
				return FALSE;
			}
#ifdef _DEBUG
			_CrtCheckMemory();
#endif
			break;

		default:
			DEBUG_BREAK;
			return FALSE;
	}

	return TRUE;
}
