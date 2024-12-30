#pragma once

class Program
{
public:
	Program();
	Program(const Program&) = delete;
	~Program() { Cleanup(); }

	bool Initialize(HINSTANCE hInstance, int nCmdShow);

	int Run();

	void Cleanup();

public:
	LRESULT ProgWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	INT_PTR ProgAbout(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	ATOM registerClass(HINSTANCE hInstance);
	bool initInstance(HINSTANCE hInstance, int nCmdShow);

	void startHook();
	void stopHook();

	DWORD searchTargetProcess(const WCHAR* pszProcessName);
	bool searchInjectionModule(DWORD processID, const WCHAR* pszInjectionModule, MODULEENTRY32* pOutEntry);

private:
	HINSTANCE m_hInst;

	HWND m_hMainWindow = nullptr;
	HWND m_hStartButton = nullptr;
	HWND m_hStopButton = nullptr;
	DLL_DIRECTORY_COOKIE m_pInjectionCookie = nullptr;

	WCHAR szTitle[100] = { 0, };
	WCHAR szWindowClass[100] = { 0, };
};
