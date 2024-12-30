#include "pch.h"
#include "HookingLauncher.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			/*long error = 0;
			HKEY hKey = nullptr;
			DWORD dwDesc;
			WCHAR buffer[100] = { 0, };
			const WCHAR* MYDLL_KEY_PATH = L"SOFTWARE\\HookingDLL";
			const WCHAR* DATA = L"D:\\workspace\\test\\MyDLL\\Debug\\Setting.ini";

			error = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
								   MYDLL_KEY_PATH,
								   0,
								   buffer,
								   REG_OPTION_NON_VOLATILE,
								   KEY_ALL_ACCESS,
								   nullptr,
								   &hKey,
								   &dwDesc);
			if (error == ERROR_SUCCESS)
			{
				RegSetValueEx(hKey, L"Setting_Dir", 0, REG_SZ, (BYTE*)DATA, (wcslen(DATA) + 1) * sizeof(WCHAR));
			}
			else
			{
				return FALSE;
			}

			RegCloseKey(hKey);*/

			OutputDebugString(L"dllmain called!(construct)\n");

			break;
		}

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;

		case DLL_PROCESS_DETACH:
		{
			/*long error = 0;
			HKEY hKey = nullptr;
			WCHAR buffer[100] = { 0, };
			const WCHAR* MYDLL_KEY_PATH = L"SOFTWARE\\HookingDLL";

			error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
								 MYDLL_KEY_PATH,
								 0,
								 KEY_ALL_ACCESS,
								 &hKey);
			if (error != ERROR_SUCCESS)
			{
				return FALSE;
			}

			error = RegDeleteValue(hKey, L"Setting_Dir");
			if (error != ERROR_SUCCESS)
			{
				return FALSE;
			}

			RegCloseKey(hKey);*/

			OutputDebugString(L"dllmain called!(destruct)\n");

			break;
		}
	}
	return TRUE;
}

