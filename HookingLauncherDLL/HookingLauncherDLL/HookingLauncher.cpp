#define HOOKING_API extern "C" __declspec(dllexport)

#include "pch.h"
#include "HookingLauncher.h"

HANDLE SearchTargetProcess(const WCHAR* pszPROCESS_NAME, DWORD* pOutProcessID, WCHAR* pOutPath);
HMODULE SearchInjectionModule(HANDLE hProcess, const WCHAR* pszTARGET_MODULE_PATH);

const WCHAR* pszTARGET_PROCESS = L"picpick.exe";
HMODULE g_hModule = nullptr;
WCHAR g_pszInjectorModulePath[MAX_PATH];

bool StartHook()
{
	bool bRet = true;

	DWORD targetProcessID;
	WCHAR pszTargetPath[MAX_PATH];

	// Ÿ�� ���μ��� �ڵ� �� ������.
	HANDLE hTargetProcess = SearchTargetProcess(pszTARGET_PROCESS, &targetProcessID, pszTargetPath);
	if (!hTargetProcess || hTargetProcess == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Target process not exists.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_RET;
	}
	
	if (!g_hModule)
	{
		bRet = false;
		goto LB_RET;
	}

	// Ÿ�� ���μ����� Injector DLL ��ΰ� �ۼ�.
	WCHAR* pszFilePart = nullptr;
	GetModuleFileNameW(g_hModule, g_pszInjectorModulePath, MAX_PATH);
	pszFilePart = wcsrchr(g_pszInjectorModulePath, L'\\');
	ZeroMemory(pszFilePart, wcslen(pszFilePart));
	wcsncat_s(g_pszInjectorModulePath, MAX_PATH, L"\\InjectorModule.dll", wcslen(L"\\InjectorModule.dll"));

	// ��ΰ� Ÿ�� ���μ����� ����.
	SIZE_T cb = (1 + wcslen(g_pszInjectorModulePath)) * sizeof(WCHAR);
	WCHAR* pszLibFileRemote = (WCHAR*)VirtualAllocEx(hTargetProcess, nullptr, cb, MEM_COMMIT, PAGE_READWRITE);
	if (!pszLibFileRemote)
	{
		MessageBox(nullptr, L"Can't alloc remote memory.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_PROCESS;
	}
	if (!WriteProcessMemory(hTargetProcess, pszLibFileRemote, (void*)g_pszInjectorModulePath, cb, nullptr))
	{
		MessageBox(nullptr, L"Can't write remote memory.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_MEMORY;
	}

	// LoadLibrary �ּ� ������.
	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"Kernel32"), "LoadLibraryW");
	if (!pfnThreadRtn)
	{
		MessageBox(nullptr, L"LoadLibrary is null.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_MEMORY;
	}

	// ���� ������ ����.
	HANDLE hRemoteThread = CreateRemoteThread(hTargetProcess, nullptr, 0, pfnThreadRtn, pszLibFileRemote, 0, 0);
	if (!hRemoteThread || hRemoteThread == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Can't create remote thread.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_MEMORY;
	}

	WaitForSingleObject(hRemoteThread, INFINITE);

	// ���� ������ ��ȯ�� üũ.
	DWORD retVal = 0;
	GetExitCodeThread(hRemoteThread, &retVal);
	if (retVal == 0)
	{
		MessageBox(nullptr, L"Remote thread return value '0'.", L"ERROR", MB_OK);
		bRet = false;
	}

	CloseHandle(hRemoteThread);

LB_CLEAN_MEMORY:
	VirtualFreeEx(hTargetProcess, pszLibFileRemote, 0, MEM_RELEASE);

LB_CLEAN_PROCESS:
	CloseHandle(hTargetProcess);

LB_RET:
	return bRet;
}

bool StopHook()
{
	bool bRet = true;

	DWORD targetProcessID;
	WCHAR pszTargetPath[MAX_PATH];

	// Ÿ�� ���μ��� �ڵ� �� ������.
	HANDLE hTargetProcess = SearchTargetProcess(pszTARGET_PROCESS, &targetProcessID, pszTargetPath);
	if (!hTargetProcess || hTargetProcess == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Can't open target process.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_RET;
	}

	// Ÿ�� ���μ������� Injector DLL �ּ� ������.
	HMODULE hTargetModule = SearchInjectionModule(hTargetProcess, g_pszInjectorModulePath);
	if (!hTargetModule || hTargetModule == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Can't load target DLL.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_PROCESS;
	}

	// FreeLibrary �ּ� ������.
	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"Kernel32"), "FreeLibrary");
	if (!pfnThreadRtn)
	{
		MessageBox(nullptr, L"LoadLibrary is null.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_PROCESS;
	}

	// ���� ������ ����.
	HANDLE hRemoteThread = CreateRemoteThread(hTargetProcess, nullptr, 0, pfnThreadRtn, hTargetModule, 0, nullptr);
	if (!hRemoteThread || hRemoteThread == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Can't create remote thread", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_PROCESS;
	}

	WaitForSingleObject(hRemoteThread, INFINITE);
	CloseHandle(hRemoteThread);

LB_CLEAN_PROCESS:
	CloseHandle(hTargetProcess);

LB_RET:
	return bRet;
}

HANDLE SearchTargetProcess(const WCHAR* pszPROCESS_NAME, DWORD* pOutProcessID, WCHAR* pOutPath)
{
	_ASSERT(pszPROCESS_NAME);
	_ASSERT(pOutProcessID);
	_ASSERT(pOutPath);

	HANDLE hRetProcess = nullptr;
	*pOutProcessID = -1;

	if (wcslen(pszPROCESS_NAME) == 0)
	{
		goto LB_RET;
	}

	DWORD processIDs[1024] = { 0, };
	DWORD needed = 0;
	// �������� ���μ��� ��� ������.
	if (!EnumProcesses(processIDs, sizeof(processIDs), &needed))
	{
		goto LB_RET;
	}

	// ������ ���鼭 �̸��� ��ġ�ϴ� ���μ��� ID, ��� ������.
	for (DWORD i = 0, totalProcesses = needed / sizeof(DWORD); i < totalProcesses; ++i)
	{
		if (processIDs[i])
		{
			// �ش� ���μ��� �ڵ� ������.
			hRetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processIDs[i]);
			if (!hRetProcess || hRetProcess == INVALID_HANDLE_VALUE)
			{
				SetLastError(0);
				continue;
			}

			// ��ο��� ���μ����� ������.
			WCHAR szBuffer[MAX_PATH];
			WCHAR* pFilePart = nullptr;
			GetModuleFileNameExW(hRetProcess, nullptr, szBuffer, MAX_PATH);
			pFilePart = wcsrchr(szBuffer, '\\') + 1;

			// ���ڷ� ���� ���μ��� �̸��� ��ġ�� ���.
			if (wcslen(pszPROCESS_NAME) == wcslen(pFilePart) && wcsncmp(pszPROCESS_NAME, pFilePart, wcslen(pszPROCESS_NAME)) == 0)
			{
				SIZE_T pathLen = pFilePart - szBuffer;
				wcsncpy_s(pOutPath, MAX_PATH, szBuffer, pathLen);

				*pOutProcessID = processIDs[i];

				break;
			}

			CloseHandle(hRetProcess);
			hRetProcess = nullptr;
		}
	}


LB_RET:
	return hRetProcess;
}

HMODULE SearchInjectionModule(HANDLE hProcess, const WCHAR* pszTARGET_MODULE_PATH)
{
	_ASSERT(hProcess);
	_ASSERT(pszTARGET_MODULE_PATH);

	HMODULE hRetModule = nullptr;

	if (hProcess == INVALID_HANDLE_VALUE)
	{
		goto LB_RET;
	}
	if (wcslen(pszTARGET_MODULE_PATH) == 0)
	{
		goto LB_RET;
	}

	HMODULE hModules[1024] = { 0, };
	DWORD needed;
	// �ε�� ��� ��� ������.
	if (!EnumProcessModules(hProcess, hModules, sizeof(hModules), &needed))
	{
		goto LB_RET;
	}

	// ������ ���鼭 ��ΰ� ��ġ�ϴ� ����� �ּҸ� ��ȯ.
	for (SIZE_T i = 0, size = needed / sizeof(HMODULE); i < size; ++i)
	{
		// ��� ��� ������.
		WCHAR szModulePath[MAX_PATH];
		GetModuleFileNameExW(hProcess, hModules[i], szModulePath, MAX_PATH);

		// ���ڷ� ���� ��� ��ο� ��ġ�� ���.
		if (wcslen(pszTARGET_MODULE_PATH) == wcslen(szModulePath) && wcsncmp(pszTARGET_MODULE_PATH, szModulePath, wcslen(szModulePath)) == 0)
		{
			hRetModule = hModules[i];
			break;
		}
	}

LB_RET:
	return hRetModule;
}
