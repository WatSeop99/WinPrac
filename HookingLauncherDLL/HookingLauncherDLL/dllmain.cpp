#include "pch.h"
#include "HookingLauncher.h"

extern HMODULE g_hModule;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			g_hModule = hModule;
			OutputDebugString(L"dllmain called!(construct)\n");
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
			g_hModule = nullptr;
			OutputDebugString(L"dllmain called!(destruct)\n");
			break;
	}
	return TRUE;
}

