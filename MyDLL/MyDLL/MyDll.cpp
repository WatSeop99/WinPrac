#include "pch.h"
#include "CallbackProcessor.h"
#include "MyDll.h"

typedef BOOL(WINAPI *PFnStretchBlt)(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
typedef int (*PFnCallbackFunc)(FSP_EXTENSION_PARAM*);

CallbackProcessor* g_pCallbackProcessor = nullptr;

HMODULE ModuleFromAddress(void* pv)
{
	MEMORY_BASIC_INFORMATION mbi = {};
	return((VirtualQuery(pv, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : nullptr);
}

int CallbackStretchBlt(LPFSP_EXTENSION_PARAM lpParam)
{
	BOOL retVal = FALSE;

	if (lpParam == NULL)
	{
		return TRUE;
	}
	if (!g_pCallbackProcessor)
	{
		goto LB_RET;
	}

	// Get Param
	int nCaller = (int)lpParam->dwParam1;
	PFnStretchBlt pfnStretchBlt = (PFnStretchBlt)lpParam->dwParam2;
	HDC hdcDest = (HDC)lpParam->dwParam3;
	int nXOriginDest = (int)lpParam->dwParam4;
	int nYOriginDest = (int)lpParam->dwParam5;
	int nWidthDest = (int)lpParam->dwParam6;
	int nHeightDest = (int)lpParam->dwParam7;
	HDC hdcSrc = (HDC)lpParam->dwParam8;
	int nXOriginSrc = (int)lpParam->dwParam9;
	int nYOriginSrc = (int)lpParam->dwParam10;
	int nWidthSrc = (int)lpParam->dwParam11;
	int nHeightSrc = (int)lpParam->dwParam12;
	DWORD dwRop = (int)lpParam->dwParam13;

	lpParam->dwParam1 = FSP_EXTRET_DEFAULT;

	g_pCallbackProcessor->SetOriginFunc(pfnStretchBlt);

	// Draw watermark..
	retVal = g_pCallbackProcessor->Update(nWidthDest, nHeightDest);
	if (!retVal)
	{
		goto LB_RET;
	}
	retVal = g_pCallbackProcessor->Render(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, dwRop);
	if (!retVal)
	{
		goto LB_RET;
	}

	g_pCallbackProcessor->SetNullOriginFunc();

LB_RET:
	lpParam->dwParam1 = FSP_EXTRET_VALUE;
	lpParam->dwParam2 = (DWORD_PTR)retVal;

	return TRUE;
}

DWORD_PTR GetProcedure()
{
	PFnCallbackFunc pfnNewFunc = CallbackStretchBlt;
	if (!pfnNewFunc)
	{
		return 0;
	}

	return (DWORD_PTR)pfnNewFunc;
}

bool InitializeModule()
{
	if (!g_pCallbackProcessor)
	{
		g_pCallbackProcessor = new CallbackProcessor;
		return g_pCallbackProcessor->Initialize();
	}

	return false;
}

bool CleanupModule()
{
	if (g_pCallbackProcessor)
	{
		if (!g_pCallbackProcessor->Cleanup())
		{
			return false;
		}
		delete g_pCallbackProcessor;
		g_pCallbackProcessor = nullptr;
	}

	return true;
}
