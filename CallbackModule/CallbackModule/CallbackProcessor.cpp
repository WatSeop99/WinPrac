#include <locale.h>
#include "pch.h"
#include "CallbackProcessor.h"

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

// 타깃 윈도우 이름들
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

	// Initialize gdiplus environment.
	if (Gdiplus::GdiplusStartup(&m_Token, &m_Input, nullptr) != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_RET;
	}


	// 워터마크용 자원 생성 및 초기화.

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

	// 워터마크 세팅 파일 경로 설정.
	GetModuleFileNameW(m_hModule, m_WatermarkState.szFilePath, MAX_PATH);

	WCHAR* pszFilePart = wcsrchr(m_WatermarkState.szFilePath, '\\');
	if (!pszFilePart)
	{
		bRet = false;
		goto LB_RET;
	}
	ZeroMemory(pszFilePart, wcslen(pszFilePart) * sizeof(WCHAR));
	wcsncat_s(m_WatermarkState.szFilePath, MAX_PATH, L"\\..\\Settings\\setting.ini", wcslen(L"\\..\\Settings\\setting.ini"));

LB_RET:
	return bRet;
}

bool CallbackProcessor::Update(int targetX, int targetY, int targetWidth, int targetHeight)
{
	_ASSERT(m_pWatermark);
	_ASSERT(m_pWatermarkGraphics);

	static int s_UpdateCount = 0;
	bool bRet = true;


	// 현재 시간을 기본 텍스트로 설정.
	SYSTEMTIME curSystemTime = {};
	GetLocalTime(&curSystemTime);
	
	WCHAR szSystemTimeAndUser[MAX_PATH] = { 0, };
	swprintf_s(szSystemTimeAndUser, MAX_PATH, L"%d-%d-%d %d:%d:%d TEST_USER",
			   curSystemTime.wYear, curSystemTime.wMonth, curSystemTime.wDay, curSystemTime.wHour, curSystemTime.wMinute, curSystemTime.wSecond);
	SIZE_T systemTimeAndUserLen = wcslen(szSystemTimeAndUser);

	// 워터마크 세팅.
	{
		const WCHAR* CATEGORY_STRING = L"Watermark-String";
		const WCHAR* CATEGORY_IMAGE = L"Watermark-Image";
		const WCHAR* CATEGORY_WATERMARK = L"Watermark";
		WCHAR szBuffer[MAX_PATH];
		bool bUpdate = false;

		// .ini 파일로부터 설정값들을 가져옴.
		GetPrivateProfileStringW(CATEGORY_STRING, L"String", szSystemTimeAndUser, szBuffer, MAX_PATH, m_WatermarkState.szFilePath);
		if (wcslen(m_WatermarkState.szString) != wcslen(szBuffer) || wcsncmp(m_WatermarkState.szString, szBuffer, wcslen(szBuffer)))
		{
			bUpdate = true;
			wcsncpy_s(m_WatermarkState.szString, MAX_PATH, szBuffer, wcslen(szBuffer));
		}
		GetPrivateProfileStringW(CATEGORY_STRING, L"Faily", L"Arial", szBuffer, MAX_PATH, m_WatermarkState.szFilePath);
		if (wcslen(m_WatermarkState.szFamily) != wcslen(szBuffer) || wcsncmp(m_WatermarkState.szFamily, szBuffer, wcslen(szBuffer)) != 0)
		{
			bUpdate = true;
			wcsncpy_s(m_WatermarkState.szFamily, MAX_PATH, szBuffer, wcslen(szBuffer));
		}
		GetPrivateProfileStringW(CATEGORY_STRING, L"Size", L"60", szBuffer, MAX_PATH, m_WatermarkState.szFilePath);
		if (wcslen(m_WatermarkState.szSize) != wcslen(szBuffer) || wcsncmp(m_WatermarkState.szSize, szBuffer, wcslen(szBuffer)) != 0)
		{
			bUpdate = true;
			wcsncpy_s(m_WatermarkState.szSize, 5, szBuffer, wcslen(szBuffer));
		}
		GetPrivateProfileStringW(CATEGORY_STRING, L"Style", L"0", szBuffer, MAX_PATH, m_WatermarkState.szFilePath);
		if (wcslen(m_WatermarkState.szStyle) != wcslen(szBuffer) || wcsncmp(m_WatermarkState.szStyle, szBuffer, wcslen(szBuffer)) != 0)
		{
			bUpdate = true;
			wcsncpy_s(m_WatermarkState.szStyle, 2, szBuffer, wcslen(szBuffer));
		}
		GetPrivateProfileStringW(CATEGORY_STRING, L"Unit", L"3", szBuffer, MAX_PATH, m_WatermarkState.szFilePath);
		if (wcslen(m_WatermarkState.szUnit) != wcslen(szBuffer) || wcsncmp(m_WatermarkState.szUnit, szBuffer, wcslen(szBuffer)) != 0)
		{
			bUpdate = true;
			wcsncpy_s(m_WatermarkState.szUnit, 3, szBuffer, wcslen(szBuffer));
		}
		GetPrivateProfileStringW(CATEGORY_STRING, L"Color", L"0", szBuffer, MAX_PATH, m_WatermarkState.szFilePath);
		if (wcslen(m_WatermarkState.szColor) != wcslen(szBuffer) || wcsncmp(m_WatermarkState.szColor, szBuffer, wcslen(szBuffer)) != 0)
		{
			bUpdate = true;
			wcsncpy_s(m_WatermarkState.szColor, MAX_PATH, szBuffer, wcslen(szBuffer));
		}
		GetPrivateProfileStringW(CATEGORY_IMAGE, L"Path", L"", szBuffer, MAX_PATH, m_WatermarkState.szFilePath);
		if (wcslen(m_WatermarkState.szImagePath) != wcslen(szBuffer) || wcsncmp(m_WatermarkState.szImagePath, szBuffer, wcslen(szBuffer)) != 0)
		{
			bUpdate = true;
			wcsncpy_s(m_WatermarkState.szImagePath, MAX_PATH, szBuffer, wcslen(szBuffer));
		}
		GetPrivateProfileStringW(CATEGORY_WATERMARK, L"Alpha", L"0.5", szBuffer, MAX_PATH, m_WatermarkState.szFilePath);
		if (wcslen(m_WatermarkState.szAlpha) != wcslen(szBuffer) || wcsncmp(m_WatermarkState.szAlpha, szBuffer, wcslen(szBuffer)) != 0)
		{
			bUpdate = true;
			wcsncpy_s(m_WatermarkState.szAlpha, 20, szBuffer, wcslen(szBuffer));
		}

		// 업데이트 된게 없으면 넘김.
		if (!bUpdate)
		{
			goto LB_RET;
		}

		// Initialize watermark backbuffer.
		m_pWatermarkGraphics->Clear(Gdiplus::Color(0, 0, 0, 0));

		std::wstring locale = _wsetlocale(LC_NUMERIC, nullptr);
		setlocale(LC_NUMERIC, "C");
		float alpha = _wtof(m_WatermarkState.szAlpha);
		_wsetlocale(LC_NUMERIC, locale.c_str());

		Gdiplus::ColorMatrix colorMatrix =
		{
			1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, alpha, 0.0f,
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


		Gdiplus::Font font(m_WatermarkState.szFamily, _wtoi(m_WatermarkState.szSize), (Gdiplus::FontStyle)_wtoi(m_WatermarkState.szStyle), (Gdiplus::Unit)_wtoi(m_WatermarkState.szUnit));
		Gdiplus::PointF point(0.0f, 0.0f);
		Gdiplus::HatchBrush hb(Gdiplus::HatchStyleSmallGrid, Gdiplus::Color(0xFF, 0xFF, 0, 0), Gdiplus::Color::Transparent);
		Gdiplus::SolidBrush brush(Gdiplus::Color(_wtoi(m_WatermarkState.szColor)));
		Gdiplus::RectF stringRect;
		SIZE_T stringLen = wcslen(m_WatermarkState.szString);
		m_pWatermarkStringGraphics->MeasureString(m_WatermarkState.szString, stringLen, &font, point, &stringRect);

		// 문구 Draw.
		m_pWatermarkStringGraphics->Clear(Gdiplus::Color(0, 0, 0, 0));
		for (int y = 0, endY = m_pWatermark->GetHeight(); y < endY; y += stringRect.Height)
		{
			for (int x = 0, endX = m_pWatermark->GetWidth(); x < endX; x += stringRect.Width)
			{
				point.X = x;
				point.Y = y;
				m_pWatermarkStringGraphics->DrawString(m_WatermarkState.szString, stringLen, &font, point, &brush);
				if (m_pWatermarkStringGraphics->GetLastStatus() != Gdiplus::Ok)
				{
					bRet = false;
					goto LB_RET;
				}
			}
		}

		// 이미지 경로가 없으면 문구만 처리.
		if (wcslen(m_WatermarkState.szImagePath) == 0)
		{
			goto LB_DRAW;
		}

		// 이미지 가져옴.
		Gdiplus::Bitmap* pImage = Gdiplus::Bitmap::FromFile(m_WatermarkState.szImagePath, 0);
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

		// 문구, 이미지를 같이 덮어 씌움.
	LB_DRAW:
		Gdiplus::RectF windowRect(0, 0, m_pWatermark->GetWidth(), m_pWatermark->GetHeight());
		m_pWatermarkGraphics->DrawImage(m_pWatermarkImage, windowRect, 0, 0, m_pWatermarkImage->GetWidth(), m_pWatermarkImage->GetHeight(), Gdiplus::UnitPixel, &imageAtt);
		m_pWatermarkGraphics->DrawImage(m_pWatermarkString, windowRect, 0, 0, m_pWatermarkString->GetWidth(), m_pWatermarkString->GetHeight(), Gdiplus::UnitPixel, &imageAtt);
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
	// Win32의 StretchBlt가 필요함.
	if (!m_pfnOriginStretchBltFunc)
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

	// Create gdiplus graphics object for compatible DC.
	pTempDCGraphics = Gdiplus::Graphics::FromHDC(hTempDC);
	if (!pTempDCGraphics || pTempDCGraphics->GetLastStatus() != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_CLEAN_DUMMY_WRITE;
	}

	// 원래 StretchBlt 처리.
	if (!m_pfnOriginStretchBltFunc(hTempDC, 0, 0, wSrc, hSrc, hdcSrc, xSrc, ySrc, wSrc, hSrc, rop))
	{
		bRet = false;
		goto LB_CLEANUP;
	}

	// 워터마크 덮어씌움.
	{
		Gdiplus::ColorMatrix colorMatrix =
		{
			1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f
		};
		Gdiplus::ImageAttributes imageAtt;
		imageAtt.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

		// 순회하면서 윈도우 핸들 얻어옴.
		WindowTable.clear();
		EnumWindows(EnumWindowsProc, (LPARAM)this);

		// 테이블에 있는 핸들값을 이용해 사이즈만큼 워터마크 덮어씌움.
		for (auto iter = WindowTable.begin(), endIter = WindowTable.end(); iter != endIter; ++iter)
		{
			// 윈도우 크기 얻어옴.
			RECT rect = { 0, };
			GetWindowRect(*iter, &rect);

			// 겹치는 영역 판별.
			RECT requireRect = { xSrc, ySrc, xSrc + wSrc, ySrc + hSrc };
			RECT overrapedRect = { 0, };
			if (!IntersectRect(&overrapedRect, &rect, &requireRect))
			{
				continue;
			}

			// 크기 잡아놓은 후 그림.
			Gdiplus::RectF windowRect(overrapedRect.left - xSrc, overrapedRect.top - ySrc, overrapedRect.right - overrapedRect.left, overrapedRect.bottom - overrapedRect.top);
			pTempDCGraphics->DrawImage(m_pWatermark, windowRect, 0, 0, m_pWatermark->GetWidth(), m_pWatermark->GetHeight(), Gdiplus::UnitPixel, &imageAtt);
		}
	}
	// 메모리 상 이미지를 대상 DC에 복사.
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
	// Win32 BitBlt가 필요.
	if (!m_pfnOriginBitBlt)
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
	BITMAPINFO bmi = { 0, };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = cx;
	bmi.bmiHeader.biHeight = cy;
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

	// Create gdiplus graphics object for compatible dc.
	pTempDCGraphics = Gdiplus::Graphics::FromHDC(hTempDC);
	if (!pTempDCGraphics || pTempDCGraphics->GetLastStatus() != Gdiplus::Ok)
	{
		bRet = false;
		goto LB_CLEAN_DUMMY_WRITE;
	}

	// 원래 BitBlt 처리.
	if (!m_pfnOriginBitBlt(hTempDC, 0, 0, cx, cy, hdcSrc, x1, y1, rop))
	{
		bRet = false;
		goto LB_CLEANUP;
	}

	// 워터마크 덮어씌움.
	{
		Gdiplus::ColorMatrix colorMatrix =
		{
			1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f
		};
		Gdiplus::ImageAttributes imageAtt;
		imageAtt.SetColorMatrix(&colorMatrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);
		
		// 순회하면서 윈도우 핸들 얻어옴.
		WindowTable.clear();
		EnumWindows(EnumWindowsProc, (LPARAM)this);

		// 테이블에 있는 핸들값을 이용해 사이즈만큼 워터마크 덮어씌움.
		for (auto iter = WindowTable.begin(), endIter = WindowTable.end(); iter != endIter; ++iter)
		{
			// 윈도우 크기 얻어옴.
			RECT rect = { 0, };
			GetWindowRect(*iter, &rect);

			// 겹치는 영역 판별.
			RECT requireRect = { x, y, x + cx, y + cy };
			RECT overrapedRect = { 0, };
			if (!IntersectRect(&overrapedRect, &rect, &requireRect))
			{
				continue;
			}

			// 크기 잡아놓은 후 그림.
			Gdiplus::RectF windowRect(overrapedRect.left - x, overrapedRect.top - y, overrapedRect.right - overrapedRect.left, overrapedRect.bottom - overrapedRect.top);
			pTempDCGraphics->DrawImage(m_pWatermark, windowRect, 0, 0, m_pWatermark->GetWidth(), m_pWatermark->GetHeight(), Gdiplus::UnitPixel, &imageAtt);
		}
	}
	// 메모리상 DC 이미지를 대상 DC에 복사.
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

	Gdiplus::GdiplusShutdown(m_Token);

	m_hModule = nullptr;

	return true;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	// 유효하지 않은 윈도우일 경우.
	if (!hwnd || !IsWindow(hwnd))
	{
		return FALSE;
	}
	// 윈도우 가시속성이 없을 경우.
	if (!IsWindowVisible(hwnd))
	{
		return TRUE;
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
			// 윈도우 크기 얻어옴.
			RECT rect = { 0, };
			GetWindowRect(hwnd, &rect);

			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			// 창 사이즈가 0임.
			if (width == 0 || height == 0)
			{
				break;
			}

			// 최상위 창이 아님.
			if (rect.left < 0 || rect.top < 0)
			{
				break;
			}

			pProcessor->WindowTable.insert(hwnd);
			break;
		}
	}

	return TRUE;
}
