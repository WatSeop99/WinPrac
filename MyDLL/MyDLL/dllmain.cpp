#include "pch.h"
#include "TypeDef.h"
#include "MyDll.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_GET_PROCEDURE:
		{
			if (!lpReserved)
			{
				return FALSE;
			}

#ifdef _DEBUG
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

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
			*((DWORD_PTR*)pRetAddr) = GetProcedure();
			if (!pRetAddr)
			{
				return FALSE;
			}

			if (!InitializeModule())
			{
				return FALSE;
			}

			break;
		}

		case DLL_PROCESS_RELEASE_PROCEDURE:
		{
			if (!CleanupModule())
			{
				return FALSE;
			}

#ifdef _DEBUG
			_CrtCheckMemory();
#endif

			break;
		}

		default:
			break;
	}

	return TRUE;
}
