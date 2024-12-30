#pragma once

class Program
{
public:
	Program() = default;
	~Program() { Cleanup(); }

	bool Initialize(HINSTANCE hInstance, int nCmdShow);

	int Run();

	void Cleanup();

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK About(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static void DoCapture(HWND hWnd);

private:
	ATOM registerClass(HINSTANCE hInstance);
	bool initInstance(HINSTANCE hInstance, int nCmdShow);

private:
	static HINSTANCE m_hInst;

	HWND m_hMainWindow = nullptr;

	WCHAR szTitle[100] = { 0, };
	WCHAR szWindowClass[100] = { 0, };
};
