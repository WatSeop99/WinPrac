#include "framework.h"
#include "resource.h"
#include "Program.h"

HINSTANCE Program::m_hInst = {};

typedef BOOL(*PFNCUSTOM)(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
HMODULE g_hModule = nullptr;


bool Program::Initialize(HINSTANCE hInstance, int nCmdShow)
{
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, 100);
	LoadStringW(hInstance, IDC_SIMPLESCREENCAPTURE, szWindowClass, 100);
	registerClass(hInstance);

	if (!initInstance(hInstance, nCmdShow))
	{
		return false;
	}

	return true;
}

int Program::Run()
{
	HACCEL hAccelTable = LoadAccelerators(m_hInst, MAKEINTRESOURCE(IDC_SIMPLESCREENCAPTURE));

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
	m_hInst = nullptr;
}

LRESULT CALLBACK Program::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND s_StaticWindow = nullptr;
	static HWND s_CaptureButton = nullptr;

	switch (msg)
	{
		case WM_CREATE:
		{
			RECT rect = {};
			GetWindowRect(hWnd, &rect);

			long screenWidth = rect.right - rect.left;
			long screenHeight = rect.bottom - rect.top;

			long startStaticPos = screenWidth / 10 * 8;

			s_StaticWindow = CreateWindow(TEXT("static"),
										  nullptr,
										  WS_CHILD | WS_VISIBLE | SS_WHITERECT,
										  startStaticPos, 0,
										  screenWidth - startStaticPos, screenHeight,
										  hWnd,
										  (HMENU)9,
										  m_hInst,
										  nullptr);
			if (!s_StaticWindow || s_StaticWindow == INVALID_HANDLE_VALUE)
			{
				__debugbreak();
			}

			long baseSize = GetDialogBaseUnits();
			int cxChar = LOWORD(baseSize);
			int cyChar = HIWORD(baseSize);
			int buttonWidth = 20 * cxChar;
			int buttonHeight = 7 * cyChar / 4;

			s_CaptureButton = CreateWindow(TEXT("Button"),
										   TEXT("Capture"),
										   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
										   startStaticPos + (screenWidth - startStaticPos) / 2 - buttonWidth / 2, screenHeight - (cyChar * 4 + buttonHeight),
										   buttonWidth, buttonHeight,
										   hWnd, (HMENU)0,
										   m_hInst,
										   nullptr);
			if (!s_CaptureButton || s_CaptureButton == INVALID_HANDLE_VALUE)
			{
				__debugbreak();
			}

#ifndef _WIN32
			SetClassLong(hWnd, GCL_HBRBACKGROUND, (LONG)CreateSolidBrush(RGB(0, 0, 0)));
#else 
			SetClassLong(hWnd, GCLP_HBRBACKGROUND, (LONG)CreateSolidBrush(RGB(0, 0, 0)));
#endif

			break;
		}

		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case 0: // capture button.
					DoCapture(hWnd);
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

INT_PTR CALLBACK Program::About(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

void Program::DoCapture(HWND hWnd)
{
	HDC hdcScreen = nullptr;
	HDC hdcWindow = nullptr;

	hdcScreen = GetDC(nullptr);
	hdcWindow = GetDC(hWnd);

	// Get the client area for size calculation.
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	if (!StretchBlt(hdcWindow,
					0, 0,
					rcClient.right / 10 * 8 + 20, rcClient.bottom,
					hdcScreen,
					0, 0,
					GetSystemMetrics(SM_CXSCREEN),
					GetSystemMetrics(SM_CYSCREEN),
					SRCCOPY))
	{
		MessageBox(hWnd, L"StretchBlt has failed", L"Failed", MB_OK);
	}


	// Clean up.
	ReleaseDC(nullptr, hdcScreen); 
	ReleaseDC(hWnd, hdcWindow);
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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPLESCREENCAPTURE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	//wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hbrBackground = CreateSolidBrush(0);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SIMPLESCREENCAPTURE);
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
								  CW_USEDEFAULT, CW_USEDEFAULT,
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
