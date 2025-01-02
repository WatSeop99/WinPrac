#include "pch.h"
#include "CallbackProcessor.h"

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);


const WCHAR* g_pszCLASS_NAMES[] =
{
	L"OpusApp",
	L"PPTFrameClass",
	L"XLMAIN"
};

bool CallbackProcessor::Initialize(HMODULE hModule)
{
	bool bRet = true;

	m_hModule = hModule;

	if (Gdiplus::GdiplusStartup(&m_Token, &m_Input, nullptr) != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_RET;
	}

	m_pWatermark = new Gdiplus::Bitmap(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), PixelFormat32bppARGB);
	if (!m_pWatermark || m_pWatermark->GetLastStatus() != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_RET;
	}

	m_pWatermarkString = new Gdiplus::Bitmap(m_pWatermark->GetWidth(), m_pWatermark->GetHeight(), PixelFormat32bppARGB);
	if (!m_pWatermarkString || m_pWatermarkString->GetLastStatus() != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_RET;
	}

	m_pWatermarkImage = new Gdiplus::Bitmap(m_pWatermark->GetWidth(), m_pWatermark->GetHeight(), PixelFormat32bppARGB);
	if (!m_pWatermarkImage || m_pWatermarkImage->GetLastStatus() != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_RET;
	}

	m_pWatermarkGraphics = Gdiplus::Graphics::FromImage(m_pWatermark);
	if (!m_pWatermarkGraphics || m_pWatermarkGraphics->GetLastStatus() != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_RET;
	}

	m_pWatermarkStringGraphics = Gdiplus::Graphics::FromImage(m_pWatermarkString);
	if (!m_pWatermarkStringGraphics || m_pWatermarkStringGraphics->GetLastStatus() != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_RET;
	}

	m_pWatermarkImageGraphics = Gdiplus::Graphics::FromImage(m_pWatermarkImage);
	if (!m_pWatermarkImageGraphics || m_pWatermarkImageGraphics->GetLastStatus() != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_RET;
	}

LB_RET:
	return bRet;
}

bool CallbackProcessor::Update(int targetWidth, int targetHeight)
{
	_ASSERT(m_pWatermark);
	_ASSERT(m_pWatermarkGraphics);

	static int s_UpdateCount = 0;
	bool bRet = true;

	SYSTEMTIME curSystemTime = {};
	GetLocalTime(&curSystemTime);

	// 텍스트 설정.
	WCHAR szSystemTimeAndUser[MAX_PATH] = { 0, };
	swprintf_s(szSystemTimeAndUser, MAX_PATH, L"%d-%d-%d %d:%d:%d TEST_USER",
			   curSystemTime.wYear, curSystemTime.wMonth, curSystemTime.wDay, curSystemTime.wHour, curSystemTime.wMinute, curSystemTime.wSecond);
	SIZE_T systemTimeAndUserLen = wcslen(szSystemTimeAndUser);

	{
		// Initialize watermark backbuffer.
		m_pWatermarkGraphics->Clear(Gdiplus::Color(0, 0, 0, 0));

		Gdiplus::ColorMatrix colorMatrix = 
		{ 
			1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f 
		};
		Gdiplus::ImageAttributes imageAtt;
		imageAtt.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

		// 회전 적용. 지금은 문제가 있어 적용하지 않음.
		/*if (s_UpdateCount == 0)
		{
			int halfWidth = targetWidth / 2;
			int halfHeight = targetHeight / 2;
			int halfTextWidth = m_pWatermark->GetWidth() / 2;
			int halfTextHeight = m_pWatermark->GetHeight() / 2;
			m_pWatermarkGraphics->RotateTransform(45.0f);
			m_pWatermarkGraphics->TranslateTransform(-halfWidth * 3, -halfHeight * 3);
			++s_UpdateCount;
		}*/

		Gdiplus::RectF commonRect(0, 0, m_pWatermark->GetWidth(), m_pWatermark->GetHeight());

		WCHAR szFilePath[MAX_PATH];
		WCHAR szImageFilePath[MAX_PATH];
		WCHAR* pszFilePart = nullptr;
		GetModuleFileName(m_hModule, szFilePath, MAX_PATH);
		
		pszFilePart = wcsrchr(szFilePath, '\\');
		ZeroMemory(pszFilePart, wcslen(pszFilePart));
		wcsncpy_s(szImageFilePath, MAX_PATH, szFilePath, wcslen(szFilePath));

		wcsncat_s(szFilePath, MAX_PATH, L"\\..\\Settings\\setting.ini", wcslen(L"\\..\\Settings\\setting.ini"));
		wcsncat_s(szImageFilePath, MAX_PATH, L"\\..\\Settings\\sample.bmp", wcslen(L"\\..\\Settings\\sample.bmp"));

		const WCHAR* CATEGORY_STRING = L"Watermark-String";
		const WCHAR* CATEGORY_IMAGE = L"Watermark-Image";
		WCHAR szString[MAX_PATH];
		WCHAR szFamily[MAX_PATH];
		WCHAR szSize[5];
		WCHAR szStyle[2];
		WCHAR szUnit[3];
		WCHAR szColor[MAX_PATH];
		WCHAR szImagePath[MAX_PATH];

		// .ini 파일로부터 설정값들을 가져옴.
		GetPrivateProfileString(CATEGORY_STRING, L"String", szSystemTimeAndUser, szString, MAX_PATH * sizeof(WCHAR), szFilePath);
		GetPrivateProfileString(CATEGORY_STRING, L"Faily", L"Arial", szFamily, MAX_PATH * sizeof(WCHAR), szFilePath);
		GetPrivateProfileString(CATEGORY_STRING, L"Size", L"60", szSize, 5 * sizeof(WCHAR), szFilePath);
		GetPrivateProfileString(CATEGORY_STRING, L"Style", L"0", szStyle, 2 * sizeof(WCHAR), szFilePath);
		GetPrivateProfileString(CATEGORY_STRING, L"Unit", L"3", szUnit, 3 * sizeof(WCHAR), szFilePath);
		GetPrivateProfileString(CATEGORY_STRING, L"Color", L"0", szColor, MAX_PATH * sizeof(WCHAR), szFilePath);
		GetPrivateProfileString(CATEGORY_IMAGE, L"Path", szImageFilePath, szImagePath, MAX_PATH * sizeof(WCHAR), szFilePath);


		Gdiplus::Font font(szFamily, _wtoi(szSize), (Gdiplus::FontStyle)_wtoi(szStyle), (Gdiplus::Unit)_wtoi(szUnit));
		Gdiplus::PointF point(0.0f, 0.0f);
		Gdiplus::HatchBrush hb(Gdiplus::HatchStyleSmallGrid, Gdiplus::Color(0xFF, 0xFF, 0, 0), Gdiplus::Color::Transparent);
		Gdiplus::SolidBrush brush(Gdiplus::Color(_wtoi(szColor)));
		Gdiplus::RectF stringRect;
		SIZE_T stringLen = wcslen(szString);
		m_pWatermarkStringGraphics->MeasureString(szString, stringLen, &font, point, &stringRect);

		// 문구 Draw.
		m_pWatermarkStringGraphics->Clear(Gdiplus::Color(0, 0, 0, 0));
		for (int y = 0, endY = m_pWatermark->GetHeight(); y < endY; y += stringRect.Height)
		{
			for (int x = 0, endX = m_pWatermark->GetWidth(); x < endX; x += stringRect.Width)
			{
				point.X = x;
				point.Y = y;
				m_pWatermarkStringGraphics->DrawString(szString, stringLen, &font, point, &brush);
				if (m_pWatermarkStringGraphics->GetLastStatus() != Gdiplus::Ok)
				{
					bRet = false;
					goto LB_RET;
				}
			}
		}


		// 이미지 가져옴.
		Gdiplus::Bitmap* pImage = Gdiplus::Bitmap::FromFile(szImagePath, 0);
		if (!pImage || pImage->GetLastStatus() != Gdiplus::Ok)
		{
			bRet = false;
			goto LB_RET;
		}

		int halfHeight = m_pWatermark->GetHeight() / 2;
		int posX = m_pWatermark->GetWidth() / 2 - halfHeight / 2;
		int posY = m_pWatermark->GetHeight() / 2 - halfHeight / 2;
		Gdiplus::RectF imageRect(posX, posY, halfHeight, halfHeight);

		// 이미지 Draw.
		m_pWatermarkImageGraphics->Clear(Gdiplus::Color(0, 0, 0, 0));
		m_pWatermarkImageGraphics->DrawImage(pImage, imageRect);
		delete pImage;
		pImage = nullptr;

		{
			// 순회하면서 윈도우 핸들 얻어옴.
			WindowTable.clear();
			EnumWindows(EnumWindowsProc, (LPARAM)this);

			// 테이블에 있는 핸들값을 이용해 사이즈만큼 워터마크 덮어씌움.
			for (auto iter = WindowTable.begin(), endIter = WindowTable.end(); iter != endIter; ++iter)
			{
				// 윈도우 크기 얻어옴.
				RECT rect = { 0, };
				GetWindowRect(*iter, &rect);

				int width = rect.right - rect.left;
				int height = rect.bottom - rect.top;
				if (width == 0 || height == 0) // 사이즈 0
				{
					__debugbreak();
				}

				// 가려져 있거나 최상위 창이 아님.
				if (rect.left < 0 || rect.top < 0)
				{
					continue;
				}

				// 크기 잡아놓은 후 그림.
				Gdiplus::RectF windowRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
				m_pWatermarkGraphics->DrawImage(m_pWatermarkImage, windowRect, 0, 0, m_pWatermarkImage->GetWidth(), m_pWatermarkImage->GetHeight(), Gdiplus::UnitPixel, &imageAtt);
				m_pWatermarkGraphics->DrawImage(m_pWatermarkString, windowRect, 0, 0, m_pWatermarkString->GetWidth(), m_pWatermarkString->GetHeight(), Gdiplus::UnitPixel, &imageAtt);
			}
		}
	}

LB_RET:
	return bRet;
}

bool CallbackProcessor::Render(HDC   hdcDest,
							   int   xDest,
							   int   yDest,
							   int   wDest,
							   int   hDest,
							   HDC   hdcSrc,
							   int   xSrc,
							   int   ySrc,
							   int   wSrc,
							   int   hSrc,
							   DWORD rop)
{
	if (m_pfnOriginStretchBltFunc == nullptr)
	{
		return false;
	}


	bool bRet = true;

	HDC hWatermarkDC = nullptr;
	HDC hTempDC = nullptr;
	HBITMAP hDummyBitmap = nullptr;
	HBITMAP hDummyWrite = nullptr;
	BYTE* pBitmapDataForDummy = nullptr;
	Gdiplus::Graphics* pTempDCGraphics = nullptr;

	// Create compatible dc.
	hTempDC = CreateCompatibleDC(hdcDest);
	if (!hTempDC || hTempDC == INVALID_HANDLE_VALUE)
	{
		bRet = false;
		goto LB_RET;
	}

	// Create bitmap for compatible dc.
	BITMAPINFO bmi = { 0, };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = wSrc;
	bmi.bmiHeader.biHeight = hSrc;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	hDummyBitmap = CreateDIBSection(hTempDC, &bmi, DIB_RGB_COLORS, (void**)&pBitmapDataForDummy, nullptr, 0);
	if (!hDummyBitmap || hDummyBitmap == INVALID_HANDLE_VALUE)
	{
		bRet = false;
		goto LB_CLEAN_TEMP_DC;
	}

	hDummyWrite = (HBITMAP)SelectObject(hTempDC, hDummyBitmap);
	if (!hDummyWrite || hDummyWrite == INVALID_HANDLE_VALUE)
	{
		bRet = false;
		goto LB_CLEAN_DUMMY_BITMAP;
	}

	// Create gdi+ graphics object for compatible dc.
	pTempDCGraphics = Gdiplus::Graphics::FromHDC(hTempDC);
	if (!pTempDCGraphics || pTempDCGraphics->GetLastStatus() != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_CLEAN_DUMMY_WRITE;
	}

	// Capture whole screen.
	if (!m_pfnOriginStretchBltFunc(hTempDC, 0, 0, wSrc, hSrc, hdcSrc, xSrc, ySrc, wSrc, hSrc, rop))
	{
		bRet = false;
		goto LB_CLEANUP;
	}

	// Draw watermark to captured image.
	{
		Gdiplus::ColorMatrix colorMatrix =
		{
			1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f
		};
		Gdiplus::ImageAttributes imageAtt;
		imageAtt.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

		Gdiplus::RectF screenSize(0, 0, wSrc, hSrc);
		pTempDCGraphics->DrawImage(m_pWatermark, screenSize, 0, 0, m_pWatermark->GetWidth(), m_pWatermark->GetHeight(), Gdiplus::UnitPixel, &imageAtt);
	}
	if (!m_pfnOriginStretchBltFunc(hdcDest, xDest, yDest, wDest, hDest, hTempDC, 0, 0, wSrc, hSrc, rop))
	{
		bRet = false;
	}

	
LB_CLEANUP:
	if (pTempDCGraphics)
	{
		delete pTempDCGraphics;
		pTempDCGraphics = nullptr;
	}

LB_CLEAN_DUMMY_WRITE:
	DeleteObject(hDummyWrite);

LB_CLEAN_DUMMY_BITMAP:
	DeleteObject(hDummyBitmap);

LB_CLEAN_TEMP_DC:
	DeleteDC(hTempDC);

LB_RET:
	return bRet;
}

bool CallbackProcessor::Render(HDC   hdc, 
							   int   x, 
							   int   y, 
							   int   cx, 
							   int   cy, 
							   HDC   hdcSrc, 
							   int   x1, 
							   int   y1, 
							   DWORD rop)
{
	if (m_pfnOriginBitBlt == nullptr)
	{
		return false;
	}


	bool bRet = true;

	HDC hWatermarkDC = nullptr;
	HDC hTempDC = nullptr;
	HBITMAP hDummyBitmap = nullptr;
	HBITMAP hDummyWrite = nullptr;
	BYTE* pBitmapDataForDummy = nullptr;
	Gdiplus::Graphics* pTempDCGraphics = nullptr;

	// Create compatible dc.
	hTempDC = CreateCompatibleDC(hdc);
	if (!hTempDC || hTempDC == INVALID_HANDLE_VALUE)
	{
		bRet = false;
		goto LB_RET;
	}

	// Create bitmap for compatible dc.
	BITMAP bitmapHeader = { 0, };
	HGDIOBJ hBitmap = GetCurrentObject(hdcSrc, OBJ_BITMAP);
	GetObject(hBitmap, sizeof(HGDIOBJ), &bitmapHeader);

	BITMAPINFO bmi = { 0, };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = bitmapHeader.bmWidth;
	bmi.bmiHeader.biHeight = bitmapHeader.bmHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	hDummyBitmap = CreateDIBSection(hTempDC, &bmi, DIB_RGB_COLORS, (void**)&pBitmapDataForDummy, nullptr, 0);
	if (!hDummyBitmap || hDummyBitmap == INVALID_HANDLE_VALUE)
	{
		bRet = false;
		goto LB_CLEAN_TEMP_DC;
	}

	hDummyWrite = (HBITMAP)SelectObject(hTempDC, hDummyBitmap);
	if (!hDummyWrite || hDummyWrite == INVALID_HANDLE_VALUE)
	{
		bRet = false;
		goto LB_CLEAN_DUMMY_BITMAP;
	}

	// Create gdi+ graphics object for compatible dc.
	pTempDCGraphics = Gdiplus::Graphics::FromHDC(hTempDC);
	if (!pTempDCGraphics || pTempDCGraphics->GetLastStatus() != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_CLEAN_DUMMY_WRITE;
	}

	// Capture whole screen.
	if (!m_pfnOriginBitBlt(hTempDC, 0, 0, bitmapHeader.bmWidth, bitmapHeader.bmHeight, hdcSrc, x1, y1, rop))
	{
		bRet = false;
		goto LB_CLEANUP;
	}

	// Draw watermark to captured image.
	{
		Gdiplus::ColorMatrix colorMatrix =
		{
			1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f
		};
		Gdiplus::ImageAttributes imageAtt;
		imageAtt.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
		Gdiplus::RectF screenSize(0, 0, bitmapHeader.bmWidth, bitmapHeader.bmHeight);
		pTempDCGraphics->DrawImage(m_pWatermark, screenSize, 0, 0, m_pWatermark->GetWidth(), m_pWatermark->GetHeight(), Gdiplus::UnitPixel, &imageAtt);
	}
	if (!m_pfnOriginBitBlt(hdc, x, y, cx, cy, hTempDC, 0, 0, rop))
	{
		bRet = false;
	}


LB_CLEANUP:
	if (pTempDCGraphics)
	{
		delete pTempDCGraphics;
		pTempDCGraphics = nullptr;
	}
	DeleteObject(hBitmap);

LB_CLEAN_DUMMY_WRITE:
	DeleteObject(hDummyWrite);

LB_CLEAN_DUMMY_BITMAP:
	DeleteObject(hDummyBitmap);

LB_CLEAN_TEMP_DC:
	DeleteDC(hTempDC);

LB_RET:
	return bRet;
}

bool CallbackProcessor::Cleanup()
{
	if (m_pWatermarkImageGraphics)
	{
		delete m_pWatermarkImageGraphics;
		m_pWatermarkImageGraphics = nullptr;
	}
	if (m_pWatermarkStringGraphics)
	{
		delete m_pWatermarkStringGraphics;
		m_pWatermarkStringGraphics = nullptr;
	}
	if (m_pWatermarkGraphics)
	{
		delete m_pWatermarkGraphics;
		m_pWatermarkGraphics = nullptr;
	}
	if (m_pWatermarkString)
	{
		delete m_pWatermarkString;
		m_pWatermarkString = nullptr;
	}
	if (m_pWatermarkImage)
	{
		delete m_pWatermarkImage;
		m_pWatermarkImage = nullptr;
	}
	if (m_pWatermark)
	{
		delete m_pWatermark;
		m_pWatermark = nullptr;
	}

	m_hModule = nullptr;

	return true;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	if (!hwnd)
	{
		return FALSE;
	}

	CallbackProcessor* pProcessor = (CallbackProcessor*)lParam;

	// 현재 윈도우의 클래스 이름 가져옴.
	WCHAR szClassName[MAX_PATH];
	GetClassName(hwnd, szClassName, MAX_PATH);

	// 원하는 클래스 이름인지 확인.
	// 맞다면 윈도우 테이블에 추가.
	for (SIZE_T i = 0, size = _countof(g_pszCLASS_NAMES); i < size; ++i)
	{
		if (wcsncmp(szClassName, g_pszCLASS_NAMES[i], wcslen(szClassName)) == 0)
		{
			pProcessor->WindowTable.insert(hwnd);
			break;
		}
	}

	return TRUE;
}
