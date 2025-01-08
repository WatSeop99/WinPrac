#include "pch.h"
#include "CallbackProcessor.h"
#include "TypeDef.h"
#include "CallbackCommon.h"

// 콜백 처리 개체
CallbackProcessor* g_pCallbackProcessor = nullptr;

int CallbackStretchBlt(LPFSP_EXTENSION_PARAM lpParam)
{
	BOOL retVal = FALSE;

	if (!lpParam)
	{
		return TRUE;
	}
	if (!g_pCallbackProcessor)
	{
		goto LB_RET;
	}

	// Get parameters.
	int nCaller = (int)lpParam->dwParam1;
	PFnStretchBlt pfnStretchBlt = (PFnStretchBlt)lpParam->dwParam2;
	HDC hdcDest = (HDC)lpParam->dwParam3;
	int xDest = (int)lpParam->dwParam4;
	int yDest = (int)lpParam->dwParam5;
	int wDest = (int)lpParam->dwParam6;
	int hDest = (int)lpParam->dwParam7;
	HDC hdcSrc = (HDC)lpParam->dwParam8;
	int xSrc = (int)lpParam->dwParam9;
	int ySrc = (int)lpParam->dwParam10;
	int wSrc = (int)lpParam->dwParam11;
	int hSrc = (int)lpParam->dwParam12;
	DWORD dwRop = (DWORD)lpParam->dwParam13;

	lpParam->dwParam1 = FSP_EXTRET_DEFAULT;


	// DC로부터 popup window인지 판별.
	HWND hwndSrc= WindowFromDC(hdcSrc);
	// 팝업 윈도우일 경우, 그냥 처리.
	DWORD windowStyle = GetWindowLong(hwndSrc, GWL_STYLE);
	if (!(windowStyle & WS_POPUP) || (windowStyle & WS_CHILD))
	{
		pfnStretchBlt(hdcDest, xDest, yDest, wDest, hDest, hdcSrc, xSrc, ySrc, wSrc, hSrc, dwRop);
		retVal = TRUE;                       
		goto LB_RET;
	}


	g_pCallbackProcessor->SetOriginStretchBltFunc(pfnStretchBlt);

	// Draw watermark..
	retVal = g_pCallbackProcessor->Update(xSrc, ySrc, wSrc, hSrc);
	if (!retVal)
	{
		goto LB_RET;
	}
	retVal = g_pCallbackProcessor->Render(hdcDest, xDest, yDest, wDest, hDest, hdcSrc, xSrc, ySrc, wSrc, hSrc, dwRop);
	if (!retVal)
	{
		goto LB_RET;
	}

	g_pCallbackProcessor->SetNullOriginStretchBltFunc();

LB_RET:
	lpParam->dwParam1 = FSP_EXTRET_VALUE;
	lpParam->dwParam2 = (DWORD_PTR)retVal;

	return TRUE;
}
int CallbackBitBlt(LPFSP_EXTENSION_PARAM lpParam)
{
	BOOL retVal = FALSE;

	if (lpParam == nullptr)
	{
		return TRUE;
	}
	if (g_pCallbackProcessor == nullptr)
	{
		goto LB_RET;
	}

	// Get Param
	int nCaller = (int)lpParam->dwParam1;
	PFnBitBlt pfnBitBlt = (PFnBitBlt)lpParam->dwParam2;
	HDC hdc = (HDC)lpParam->dwParam3;
	int x = (int)lpParam->dwParam4;
	int y = (int)lpParam->dwParam5;
	int cx = (int)lpParam->dwParam6;
	int cy = (int)lpParam->dwParam7;
	HDC hdcSrc = (HDC)lpParam->dwParam8;
	int x1 = (int)lpParam->dwParam9;
	int y1 = (int)lpParam->dwParam10;
	DWORD dwRop = (DWORD)lpParam->dwParam11;

	lpParam->dwParam1 = FSP_EXTRET_DEFAULT;


	// DC로부터 popup window인지 판별.
	HWND hwndSrc = WindowFromDC(hdcSrc);
	// 팝업 윈도우일 경우, 그냥 처리.
	DWORD windowStyle = GetWindowLong(hwndSrc, GWL_STYLE);
	if (!(windowStyle & WS_POPUP) || (windowStyle & WS_CHILD))
	{
		pfnBitBlt(hdc, x, y, cx, cy, hdcSrc, x1, y1, dwRop);
		retVal = TRUE;
		goto LB_RET;
	}


	g_pCallbackProcessor->SetOriginBitBltFunc(pfnBitBlt);

	// Draw watermark..
	retVal = g_pCallbackProcessor->Update(x1, y1, cx, cy);
	if (!retVal)
	{
		goto LB_RET;
	}
	retVal = g_pCallbackProcessor->Render(hdc, x, y, cx, cy, hdcSrc, x1, y1, dwRop);
	if (!retVal)
	{
		goto LB_RET;
	}

	g_pCallbackProcessor->SetNullOriginBitBltFunc();

LB_RET:
	lpParam->dwParam1 = FSP_EXTRET_VALUE;
	lpParam->dwParam2 = (DWORD_PTR)retVal;

	return TRUE;
}

DWORD_PTR GetProcedure(const DWORD PROCEDURE_TYPE)
{
	PFnCallbackFunc pfnReturnFunc = nullptr;
	switch (PROCEDURE_TYPE)
	{
		case DLL_PROCEDURE_STRETCHBLT:
			pfnReturnFunc = CallbackStretchBlt;
			break;

		case DLL_PROCEDURE_BITBLT:
			pfnReturnFunc = CallbackBitBlt;
			break;

		default:
			break;
	}

	return (DWORD_PTR)pfnReturnFunc;
}

bool InitializeModule(HMODULE hModule)
{
	if (!g_pCallbackProcessor)
	{
		g_pCallbackProcessor = new CallbackProcessor;
		return g_pCallbackProcessor->Initialize(hModule);
	}
	
	return true;
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
