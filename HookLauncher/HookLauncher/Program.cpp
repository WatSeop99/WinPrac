#include "framework.h"
#include "resource.h"
#include "Program.h"

typedef DLL_DIRECTORY_COOKIE(*PFN_ADD_DLL_DIREFTORY)(PCWSTR);

struct DLLPathParameter
{
	WCHAR szDLLName[MAX_PATH];
	HANDLE hFile;
	DWORD dwFlags;
};

Program* g_pProgram = nullptr;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	_ASSERT(g_pProgram);
	return g_pProgram->ProgWndProc(hWnd, msg, wParam, lParam);
}
static INT_PTR CALLBACK About(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	_ASSERT(g_pProgram);
	return g_pProgram->ProgAbout(hWnd, msg, wParam, lParam);
}

Program::Program()
{
	g_pProgram = this;
}

bool Program::Initialize(HINSTANCE hInstance, int nCmdShow)
{
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, 100);
	LoadStringW(hInstance, IDC_HOOKLAUNCHER, szWindowClass, 100);
	registerClass(hInstance);

	if (!initInstance(hInstance, nCmdShow))
	{
		return false;
	}

	/*{
		long error = 0;
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
			return false;
		}

		RegCloseKey(hKey);
	}*/

	return true;
}

int Program::Run()
{
	HACCEL hAccelTable = LoadAccelerators(m_hInst, MAKEINTRESOURCE(IDC_HOOKLAUNCHER));

	// hwnd Å½»ö ºÎºÐ.
	/*{
		HWND hWnd = nullptr;
		do
		{
			hWnd = FindWindowEx(nullptr, hWnd, nullptr, nullptr);
			DWORD dwProcID = 0;
			GetWindowThreadProcessId(hWnd, &dwProcID);
			
			HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (!hSnapShot || hSnapShot == INVALID_HANDLE_VALUE)
			{
				__debugbreak();
			}
			
			PROCESSENTRY32 pe = { 0, };
			pe.dwSize = sizeof(PROCESSENTRY32);

			for (BOOL bOK = Process32First(hSnapShot, &pe); bOK; Process32Next(hSnapShot, &pe))
			{
				if (pe.th32ProcessID == dwProcID)
				{
					break;
				}
			}
			CloseHandle(hSnapShot);

		} while (hWnd);
	}*/

	MSG msg = { 0, };

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

void Program::Cleanup()
{
	DestroyWindow(m_hMainWindow);
	m_hMainWindow = nullptr;
	m_hInst = {};
}

LRESULT Program::ProgWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			RECT rect = {};
			GetWindowRect(hWnd, &rect);

			long screenWidth = rect.right - rect.left;
			long screenHeight = rect.bottom - rect.top;

			long startButtonPosX = screenWidth / 10 * 2;
			long startButtonPosY = screenHeight / 10 * 2;

			long baseSize = GetDialogBaseUnits();
			int cxChar = LOWORD(baseSize);
			int cyChar = HIWORD(baseSize);
			int buttonWidth = 20 * cxChar;
			int buttonHeight = 7 * cyChar / 4;

			m_hStartButton = CreateWindow(TEXT("Button"),
										  TEXT("StartHook"),
										  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
										  startButtonPosX, startButtonPosY,
										  buttonWidth, buttonHeight,
										  hWnd, (HMENU)0,
										  m_hInst,
										  nullptr);
			if (!m_hStartButton || m_hStartButton == INVALID_HANDLE_VALUE)
			{
				__debugbreak();
			}

			m_hStopButton = CreateWindow(TEXT("Button"),
										 TEXT("StopHook"),
										 WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
										 startButtonPosX, startButtonPosY + 40,
										 buttonWidth, buttonHeight,
										 hWnd, (HMENU)1,
										 m_hInst,
										 nullptr);
			if (!m_hStopButton || m_hStopButton == INVALID_HANDLE_VALUE)
			{
				__debugbreak();
			}


			break;
		}

		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case 0: // start button
					startHook();
					break;

				case 1: // stop button
					stopHook();
					break;

				case IDM_ABOUT:
					DialogBox(m_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;

				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;

				default:
					return DefWindowProc(hWnd, msg, wParam, lParam);
			}

			break;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps = {};
			HDC hdc = BeginPaint(hWnd, &ps);

			EndPaint(hWnd, &ps);

			break;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

INT_PTR Program::ProgAbout(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (msg)
	{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hWnd, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}

	return (INT_PTR)FALSE;
}

ATOM Program::registerClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOOKLAUNCHER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_HOOKLAUNCHER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

bool Program::initInstance(HINSTANCE hInstance, int nCmdShow)
{
	m_hInst = hInstance;

	m_hMainWindow = CreateWindowW(szWindowClass,
								  szTitle,
								  WS_OVERLAPPEDWINDOW,
								  CW_USEDEFAULT, CW_USEDEFAULT,
								  300, 300,
								  nullptr,
								  nullptr,
								  hInstance,
								  nullptr);
	if (!m_hMainWindow || m_hMainWindow == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	ShowWindow(m_hMainWindow, nCmdShow);
	UpdateWindow(m_hMainWindow);

	return true;
}

void Program::startHook()
{
	DWORD targetProcessID = searchTargetProcess(L"SimpleScreenCapture.exe");
	if (targetProcessID == -1)
	{
		__debugbreak();
	}

	WCHAR szForceInjectionLibPath[MAX_PATH] = L"D:\\workspace\\test\\ForceInjection\\Debug\\";
	WCHAR szForceInjectionLib[MAX_PATH] = L"ForceInjection.dll";
	HANDLE hTargetProcess = nullptr;
	HANDLE hRemoteThread = nullptr;

	hTargetProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
								 FALSE,
								 targetProcessID);
	if (!hTargetProcess || hTargetProcess == INVALID_HANDLE_VALUE)
	{
		__debugbreak();
	}
	if (!CopyFile(L"D:\\workspace\\test\\ForceInjection\\Debug\\ForceInjection.dll", L"D:\\workspace\\test\\SimpleScreenCapture\\Debug\\ForceInjection.dll", FALSE))
	{
		__debugbreak();
	}

	SIZE_T cb = (1 + wcslen(szForceInjectionLib)) * sizeof(WCHAR);
	//SIZE_T cb = (1 + wcslen(szForceInjectionLibPath)) * sizeof(WCHAR);
	WCHAR* pszLibFileRemote = (WCHAR*)VirtualAllocEx(hTargetProcess, nullptr, cb, MEM_COMMIT, PAGE_READWRITE);
	if (!pszLibFileRemote)
	{
		__debugbreak();
	}
	if (!WriteProcessMemory(hTargetProcess, pszLibFileRemote, (void*)szForceInjectionLib, cb, nullptr))
	{
		__debugbreak();
	}
	/*if (!WriteProcessMemory(hTargetProcess, pszLibFileRemote, (void*)szForceInjectionLibPath, cb, nullptr))
	{
		__debugbreak();
	}*/


	/*PTHREAD_START_ROUTINE pfnAddDllDirectory = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "AddDllDirectory");
	if (!pfnAddDllDirectory)
	{
		__debugbreak();
	}
	hRemoteThread = CreateRemoteThread(hTargetProcess, nullptr, 0, pfnAddDllDirectory, pszLibFileRemote, 0, 0);
	if (!hRemoteThread || hRemoteThread == INVALID_HANDLE_VALUE)
	{
		__debugbreak();
	}

	WaitForSingleObject(hRemoteThread, INFINITE);
	GetExitCodeThread(hRemoteThread, (LPDWORD)&m_pInjectionCookie);
	if (!m_pInjectionCookie)
	{
		__debugbreak();
	}

	VirtualFreeEx(hTargetProcess, pszLibFileRemote, 0, MEM_RELEASE);
	CloseHandle(hRemoteThread);*/


	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
	if (!pfnThreadRtn)
	{
		__debugbreak();
	}
	/*PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryExW");
	if (!pfnThreadRtn)
	{
		__debugbreak();
	}

	DLLPathParameter dllParameter = { L"ForceInjection.dll", nullptr, LOAD_LIBRARY_SEARCH_USER_DIRS };
	cb = sizeof(DLLPathParameter);

	DLLPathParameter* pLoadedParameter = (DLLPathParameter*)VirtualAllocEx(hTargetProcess, nullptr, cb, MEM_COMMIT, PAGE_READWRITE);
	if (!pLoadedParameter)
	{
		__debugbreak();
	}
	if (!WriteProcessMemory(hTargetProcess, pLoadedParameter, (void*)&dllParameter, cb, nullptr))
	{
		__debugbreak();
	}*/


	// Create a remote thread that calls LoadLibraryW(DLLPathname)
	hRemoteThread = CreateRemoteThread(hTargetProcess, nullptr, 0, pfnThreadRtn, pszLibFileRemote, 0, 0);
	if (!hRemoteThread || hRemoteThread == INVALID_HANDLE_VALUE)
	{
		__debugbreak();
	}
	/*hRemoteThread = CreateRemoteThread(hTargetProcess, nullptr, 0, pfnThreadRtn, pLoadedParameter, 0, 0);
	if (!hRemoteThread || hRemoteThread == INVALID_HANDLE_VALUE)
	{
		__debugbreak();
	}*/

	WaitForSingleObject(hRemoteThread, INFINITE);
	/*DWORD retCode = 0;
	GetExitCodeThread(hRemoteThread, &retCode);
	if (retCode == 0)
	{
		__debugbreak();
	}*/

	VirtualFreeEx(hTargetProcess, pszLibFileRemote, 0, MEM_RELEASE);
	//VirtualFreeEx(hTargetProcess, pLoadedParameter, 0, MEM_RELEASE);

	CloseHandle(hRemoteThread);
	CloseHandle(hTargetProcess);
}

void Program::stopHook()
{
	DWORD targetProcessID = searchTargetProcess(L"SimpleScreenCapture.exe");
	if (targetProcessID == -1)
	{
		__debugbreak();
	}

	MODULEENTRY32 me = { 0, };
	me.dwSize = sizeof(MODULEENTRY32);
	if (!searchInjectionModule(targetProcessID, L"ForceInjection.dll", &me))
	{
		__debugbreak();
	}


	WCHAR szForceInjectionLib[MAX_PATH] = L"ForceInjection.dll";
	HANDLE hTargetProcess = nullptr;
	HANDLE hRemoteThread = nullptr;

	hTargetProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION,
								 FALSE,
								 targetProcessID);
	if (!hTargetProcess || hTargetProcess == INVALID_HANDLE_VALUE)
	{
		__debugbreak();
	}

	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "FreeLibrary");
	if (!pfnThreadRtn)
	{
		__debugbreak();
	}

	hRemoteThread = CreateRemoteThread(hTargetProcess, nullptr, 0, pfnThreadRtn, me.modBaseAddr, 0, nullptr);
	if (!hRemoteThread || hRemoteThread == INVALID_HANDLE_VALUE)
	{
		__debugbreak();
	}

	WaitForSingleObject(hRemoteThread, INFINITE);
	
	CloseHandle(hRemoteThread);
	
	DeleteFile(L"D:\\workspace\\test\\SimpleScreenCapture\\Debug\\ForceInjection.dll");

	/*if (m_pInjectionCookie)
	{
		DLLPathParameter dllParameter = { L"ForceInjection.dll", nullptr, LOAD_LIBRARY_SEARCH_USER_DIRS };
		SIZE_T cb = sizeof(DLLPathParameter);

		DLLPathParameter* pLoadedParameter = (DLLPathParameter*)VirtualAllocEx(hTargetProcess, nullptr, cb, MEM_COMMIT, PAGE_READWRITE);
		if (!pLoadedParameter)
		{
			__debugbreak();
		}
		if (!WriteProcessMemory(hTargetProcess, pLoadedParameter, (void*)&dllParameter, cb, nullptr))
		{
			__debugbreak();
		}

		pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "RemoveDllDirectory");
		if (!pfnThreadRtn)
		{
			__debugbreak();
		}

		hRemoteThread = CreateRemoteThread(hTargetProcess, nullptr, 0, pfnThreadRtn, pLoadedParameter, 0, nullptr);
		if (!hRemoteThread || hRemoteThread == INVALID_HANDLE_VALUE)
		{
			__debugbreak();
		}

		VirtualFreeEx(hTargetProcess, pLoadedParameter, 0, MEM_RELEASE);
		CloseHandle(hRemoteThread);
	}*/

	CloseHandle(hTargetProcess);
}

DWORD Program::searchTargetProcess(const WCHAR* pszProcessName)
{
	_ASSERT(pszProcessName);

	DWORD retProcessID = -1;

	if (wcslen(pszProcessName) == 0)
	{
		goto LB_RET;
	}


	PROCESSENTRY32 pe = { 0, };
	pe.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (!hSnapShot || hSnapShot == INVALID_HANDLE_VALUE)
	{
		goto LB_RET;
	}

	for (BOOL bOk = Process32First(hSnapShot, &pe); bOk; bOk = Process32Next(hSnapShot, &pe))
	{
		if (wcscmp(pe.szExeFile, pszProcessName) == 0)
		{
			retProcessID = pe.th32ProcessID;
			break;
		}
	}

	CloseHandle(hSnapShot);

LB_RET:
	return retProcessID;
}

bool Program::searchInjectionModule(DWORD processID, const WCHAR* pszInjectionModule, MODULEENTRY32* pOutEntry)
{
	_ASSERT(processID != -1);
	_ASSERT(pszInjectionModule);
	_ASSERT(pOutEntry);

	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID);
	if (!hSnapShot || hSnapShot == INVALID_HANDLE_VALUE)
	{
		__debugbreak();
		return false;
	}


	bool bFound = false;
	MODULEENTRY32 me = { 0, };
	me.dwSize = sizeof(MODULEENTRY32);

	for (BOOL bOk = Module32First(hSnapShot, &me); bOk; bOk = Module32Next(hSnapShot, &me))
	{
		if (wcscmp(me.szModule, pszInjectionModule) == 0)
		{
			bFound = true;
			break;
		}
	}
	CloseHandle(hSnapShot);

	if (!bFound)
	{
		return false;
	}

	memcpy(pOutEntry, &me, me.dwSize);
	return true;
}
