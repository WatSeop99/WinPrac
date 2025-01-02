#include "pch.h"
#include "CallbackProcessor.h"
#include "TypeDef.h"
#include "MyDll.h"

CallbackProcessor* g_pCallbackProcessor = nullptr;

int CallbackStretchBlt(LPFSP_EXTENSION_PARAM lpParam)
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
	DWORD dwRop = (DWORD)lpParam->dwParam13;

	lpParam->dwParam1 = FSP_EXTRET_DEFAULT;


	// DC로부터 popup window인지 판별.
	HWND hwndDest = WindowFromDC(hdcDest);
	// 시스템 전체(?) DC일 경우 그냥 처리.
	/*if (hwndDest == nullptr)
	{
		pfnStretchBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, dwRop);
		retVal = TRUE;
		goto LB_RET;
	}*/
	// 팝업 윈도우일 경우에도 그냥 처리.
	DWORD windowStyle = GetWindowLong(hwndDest, GWL_STYLE);
	//if (!(windowStyle & WS_POPUP) || (windowStyle & WS_CHILD))
	if ((windowStyle & WS_POPUP) && !(windowStyle & WS_CHILD))
	{
		pfnStretchBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, dwRop);
		retVal = TRUE;                       
		goto LB_RET;
	}


	g_pCallbackProcessor->SetOriginStretchBltFunc(pfnStretchBlt);

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


	//// DC로부터 popup window인지 판별.
	//HWND hwndDest = WindowFromDC(hdc);
	//// 시스템 전체(?) DC일 경우 그냥 처리.
	//if (!hwndDest)
	//{
	//	pfnBitBlt(hdc, x, y, cx, y, hdcSrc, x1, y1, dwRop);
	//	retVal = TRUE;
	//	goto LB_RET;
	//}
	//// 팝업 윈도우일 경우에도 그냥 처리.
	//DWORD windowStyle = GetWindowLong(hwndDest, GWL_STYLE);
	//if ((windowStyle & WS_POPUP) && !(windowStyle & WS_CHILD))
	//{
	//	pfnBitBlt(hdc, x, y, cx, y, hdcSrc, x1, y1, dwRop);
	//	retVal = TRUE;
	//	goto LB_RET;
	//}


	g_pCallbackProcessor->SetOriginBitBltFunc(pfnBitBlt);

	// Draw watermark..
	retVal = g_pCallbackProcessor->Update(cx, cy);
	if (!retVal)
	{
		goto LB_RET;
	}
	retVal = g_pCallbackProcessor->Render(hdc, x, y, cx, y, hdcSrc, x1, y1, dwRop);
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
	PFnCallbackFunc pfnNewFunc = nullptr;
	switch (PROCEDURE_TYPE)
	{
		case DLL_PROCEDURE_STRETCHBLT:
			pfnNewFunc = CallbackStretchBlt;
			break;

		case DLL_PROCEDURE_BITBLT:
			pfnNewFunc = CallbackBitBlt;
			break;

		default:
			break;
	}

	return (DWORD_PTR)pfnNewFunc;
}

bool InitializeModule(HMODULE hModule)
{
	if (g_pCallbackProcessor == nullptr)
	{
		g_pCallbackProcessor = new CallbackProcessor;
		return g_pCallbackProcessor->Initialize(hModule);
	}
	
	return true;
}

bool CleanupModule()
{
	if (g_pCallbackProcessor != nullptr)
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
