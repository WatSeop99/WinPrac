#include "framework.h"
#include "SimpleScreenCapture.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					  _In_opt_ HINSTANCE hPrevInstance,
					  _In_ LPWSTR    lpCmdLine,
					  _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Program* pProgram = new Program;
	if (!pProgram->Initialize(hInstance, nCmdShow))
	{
		__debugbreak();
	}

	pProgram->Run();

	if (pProgram)
	{
		delete pProgram;
		pProgram = nullptr;
	}

#ifdef _DEBUG
	_CrtCheckMemory();
#endif

	return 0;
}
