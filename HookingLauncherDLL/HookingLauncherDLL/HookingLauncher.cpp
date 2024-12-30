#define HOOKING_API extern "C" __declspec(dllexport)

#include "pch.h"
#include "HookingLauncher.h"

HANDLE SearchTargetProcess(const WCHAR* pszPROCESS_NAME, DWORD* pOutProcessID, WCHAR* pOutPath);
HMODULE SearchInjectionModule(HANDLE hProcess, const WCHAR* pszTARGET_MODULE_PATH);

WCHAR g_pszFORCE_INJECTION_PATH[MAX_PATH] = L"./ForceInjection.dll";

bool EnableDebugPrivilege()
{
	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		std::cerr << "OpenProcessToken failed. Error: " << GetLastError() << std::endl;
		return false;
	}

	LUID luid;
	if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &luid))
	{
		std::cerr << "LookupPrivilegeValue failed. Error: " << GetLastError() << std::endl;
		CloseHandle(hToken);
		return false;
	}

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr))
	{
		std::cerr << "AdjustTokenPrivileges failed. Error: " << GetLastError() << std::endl;
		CloseHandle(hToken);
		return false;
	}

	CloseHandle(hToken);
	return GetLastError() == ERROR_SUCCESS;
}

bool StartHook()
{
	bool bRet = true;

	if (!EnableDebugPrivilege())
	{
		__debugbreak();
	}

	DWORD targetProcessID;
	WCHAR pszTargetPath[MAX_PATH];

	// 타깃 프로세스 핸들 값 가져옴.
	//HANDLE hTargetProcess = SearchTargetProcess(L"picpick.exe", &targetProcessID, pszTargetPath);
	HANDLE hTargetProcess = SearchTargetProcess(L"SimpleScreenCapture.exe", &targetProcessID, pszTargetPath);
	if (!hTargetProcess || hTargetProcess == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Target process not exists.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_RET;
	}

	WCHAR szForceInjectionLib[MAX_PATH] = L"ForceInjection.dll";
	wcscat_s(pszTargetPath, MAX_PATH, szForceInjectionLib);

	// 타깃 프로세스에 DLL 경로값 작성.
	SIZE_T cb = (1 + wcslen(g_pszFORCE_INJECTION_PATH)) * sizeof(WCHAR);
	WCHAR* pszLibFileRemote = (WCHAR*)VirtualAllocEx(hTargetProcess, nullptr, cb, MEM_COMMIT, PAGE_READWRITE);
	if (!pszLibFileRemote)
	{
		ZeroMemory(pszTargetPath, sizeof(pszTargetPath));

		MessageBox(nullptr, L"Can't alloc remote memory.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_PROCESS;
	}
	if (!WriteProcessMemory(hTargetProcess, pszLibFileRemote, (void*)g_pszFORCE_INJECTION_PATH, cb, nullptr))
	{
		ZeroMemory(pszTargetPath, sizeof(pszTargetPath));

		MessageBox(nullptr, L"Can't write remote memory.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_MEMORY;
	}

	// LoadLibrary 주소 가져옴.
	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
	if (!pfnThreadRtn)
	{
		ZeroMemory(pszTargetPath, sizeof(pszTargetPath));

		MessageBox(nullptr, L"LoadLibrary is null.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_MEMORY;
	}

	// Create a remote thread that calls LoadLibraryW(DLLPathname)
	HANDLE hRemoteThread = CreateRemoteThread(hTargetProcess, nullptr, 0, pfnThreadRtn, pszLibFileRemote, 0, 0);
	if (!hRemoteThread || hRemoteThread == INVALID_HANDLE_VALUE)
	{
		ZeroMemory(pszTargetPath, sizeof(pszTargetPath));

		MessageBox(nullptr, L"Can't create remote thread.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_MEMORY;
	}

	WaitForSingleObject(hRemoteThread, INFINITE);

	// 원격 스레드 반환값 체크.
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
	HANDLE hTargetProcess = SearchTargetProcess(L"picpick.exe", &targetProcessID, pszTargetPath);
	if (!hTargetProcess || hTargetProcess == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Can't open target process.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_RET;
	}

	wcscat_s(pszTargetPath, MAX_PATH, L"ForceInjection.dll");


	HMODULE hTargetModule = SearchInjectionModule(hTargetProcess, pszTargetPath);
	if (!hTargetModule || hTargetModule == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Can't load target DLL.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_PROCESS;
	}


	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "FreeLibrary");
	if (!pfnThreadRtn)
	{
		MessageBox(nullptr, L"LoadLibrary is null.", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_PROCESS;
	}

	HANDLE hRemoteThread = CreateRemoteThread(hTargetProcess, nullptr, 0, pfnThreadRtn, hTargetModule, 0, nullptr);
	if (!hRemoteThread || hRemoteThread == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Can't create remote thread", L"ERROR", MB_OK);
		bRet = false;
		goto LB_CLEAN_PROCESS;
	}

	WaitForSingleObject(hRemoteThread, INFINITE);


	CloseHandle(hRemoteThread);

	DeleteFile(pszTargetPath);

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
	if (!EnumProcesses(processIDs, sizeof(processIDs), &needed))
	{
		goto LB_RET;
	}

	for (DWORD i = 0, totalProcesses = needed / sizeof(DWORD); i < totalProcesses; ++i)
	{
		if (processIDs[i])
		{
			DWORD lastErrorCode;

			hRetProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
									  FALSE,
									  processIDs[i]);
			lastErrorCode = GetLastError();
			if (!hRetProcess || hRetProcess == INVALID_HANDLE_VALUE)
			{
				SetLastError(0);
				continue;
			}
			

			WCHAR buffer[MAX_PATH];
			WCHAR* pFilePart = nullptr;
			GetModuleFileNameEx(hRetProcess, nullptr, buffer, MAX_PATH);
			pFilePart = wcsrchr(buffer, '\\') + 1;

			if (wcsncmp(pszPROCESS_NAME, pFilePart, wcslen(pszPROCESS_NAME)) == 0)
			{
				SIZE_T pathLen = pFilePart - buffer;
				wcsncpy_s(pOutPath, MAX_PATH, buffer, pathLen);

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
	if (!EnumProcessModules(hProcess, hModules, sizeof(hModules), &needed))
	{
		goto LB_RET;
	}

	for (SIZE_T i = 0, size = needed / sizeof(HMODULE); i < size; ++i)
	{
		WCHAR szModulePath[MAX_PATH];
		GetModuleFileNameEx(hProcess, hModules[i], szModulePath, MAX_PATH);

		if (wcsncmp(pszTARGET_MODULE_PATH, szModulePath, wcslen(pszTARGET_MODULE_PATH)) == 0)
		{
			hRetModule = hModules[i];
			break;
		}
	}

LB_RET:
	return hRetModule;
}
