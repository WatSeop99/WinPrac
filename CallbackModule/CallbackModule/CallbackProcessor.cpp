#include <locale.h>
#include "pch.h"
#include "CallbackProcessor.h"

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

// Ÿ�� ������ �̸���
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


	// ���͸�ũ�� �ڿ� ���� �� �ʱ�ȭ.

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

	// ���͸�ũ ���� ���� ��� ����.
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


	// ���� �ð��� �⺻ �ؽ�Ʈ�� ����.
	SYSTEMTIME curSystemTime = {};
	GetLocalTime(&curSystemTime);
	
	WCHAR szSystemTimeAndUser[MAX_PATH] = { 0, };
	swprintf_s(szSystemTimeAndUser, MAX_PATH, L"%d-%d-%d %d:%d:%d TEST_USER",
			   curSystemTime.wYear, curSystemTime.wMonth, curSystemTime.wDay, curSystemTime.wHour, curSystemTime.wMinute, curSystemTime.wSecond);
	SIZE_T systemTimeAndUserLen = wcslen(szSystemTimeAndUser);

	// ���͸�ũ ����.
	{
		const WCHAR* CATEGORY_STRING = L"Watermark-String";
		const WCHAR* CATEGORY_IMAGE = L"Watermark-Image";
		const WCHAR* CATEGORY_WATERMARK = L"Watermark";
		WCHAR szBuffer[MAX_PATH];
		bool bUpdate = false;

		// .ini ���Ϸκ��� ���������� ������.
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

		// ������Ʈ �Ȱ� ������ �ѱ�.
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

		// ȸ�� ����. ������ ������ �־� �������� ����.
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

		// ���� Draw.
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

		// �̹��� ��ΰ� ������ ������ ó��.
		if (wcslen(m_WatermarkState.szImagePath) == 0)
		{
			goto LB_DRAW;
		}

		// �̹��� ������.
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

		// �̹��� Draw.
		m_pWatermarkImageGraphics->Clear(Gdiplus::Color(0, 0, 0, 0));
		m_pWatermarkImageGraphics->DrawImage(pImage, imageRect);
		delete pImage;
		pImage = nullptr;

		// ����, �̹����� ���� ���� ����.
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
	// Win32�� StretchBlt�� �ʿ���.
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

	// ���� StretchBlt ó��.
	if (!m_pfnOriginStretchBltFunc(hTempDC, 0, 0, wSrc, hSrc, hdcSrc, xSrc, ySrc, wSrc, hSrc, rop))
	{
		bRet = false;
		goto LB_CLEANUP;
	}

	// ���͸�ũ �����.
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

		// ��ȸ�ϸ鼭 ������ �ڵ� ����.
		WindowTable.clear();
		EnumWindows(EnumWindowsProc, (LPARAM)this);

		// ���̺� �ִ� �ڵ鰪�� �̿��� �����ŭ ���͸�ũ �����.
		for (auto iter = WindowTable.begin(), endIter = WindowTable.end(); iter != endIter; ++iter)
		{
			// ������ ũ�� ����.
			RECT rect = { 0, };
			GetWindowRect(*iter, &rect);

			// ��ġ�� ���� �Ǻ�.
			RECT requireRect = { xSrc, ySrc, xSrc + wSrc, ySrc + hSrc };
			RECT overrapedRect = { 0, };
			if (!IntersectRect(&overrapedRect, &rect, &requireRect))
			{
				continue;
			}

			// ũ�� ��Ƴ��� �� �׸�.
			Gdiplus::RectF windowRect(overrapedRect.left - xSrc, overrapedRect.top - ySrc, overrapedRect.right - overrapedRect.left, overrapedRect.bottom - overrapedRect.top);
			pTempDCGraphics->DrawImage(m_pWatermark, windowRect, 0, 0, m_pWatermark->GetWidth(), m_pWatermark->GetHeight(), Gdiplus::UnitPixel, &imageAtt);
		}
	}
	// �޸� �� �̹����� ��� DC�� ����.
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
	// Win32 BitBlt�� �ʿ�.
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

	// ���� BitBlt ó��.
	if (!m_pfnOriginBitBlt(hTempDC, 0, 0, cx, cy, hdcSrc, x1, y1, rop))
	{
		bRet = false;
		goto LB_CLEANUP;
	}

	// ���͸�ũ �����.
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
		
		// ��ȸ�ϸ鼭 ������ �ڵ� ����.
		WindowTable.clear();
		EnumWindows(EnumWindowsProc, (LPARAM)this);

		// ���̺� �ִ� �ڵ鰪�� �̿��� �����ŭ ���͸�ũ �����.
		for (auto iter = WindowTable.begin(), endIter = WindowTable.end(); iter != endIter; ++iter)
		{
			// ������ ũ�� ����.
			RECT rect = { 0, };
			GetWindowRect(*iter, &rect);

			// ��ġ�� ���� �Ǻ�.
			RECT requireRect = { x, y, x + cx, y + cy };
			RECT overrapedRect = { 0, };
			if (!IntersectRect(&overrapedRect, &rect, &requireRect))
			{
				continue;
			}

			// ũ�� ��Ƴ��� �� �׸�.
			Gdiplus::RectF windowRect(overrapedRect.left - x, overrapedRect.top - y, overrapedRect.right - overrapedRect.left, overrapedRect.bottom - overrapedRect.top);
			pTempDCGraphics->DrawImage(m_pWatermark, windowRect, 0, 0, m_pWatermark->GetWidth(), m_pWatermark->GetHeight(), Gdiplus::UnitPixel, &imageAtt);
		}
	}
	// �޸𸮻� DC �̹����� ��� DC�� ����.
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
	// ��ȿ���� ���� �������� ���.
	if (!hwnd || !IsWindow(hwnd))
	{
		return FALSE;
	}
	// ������ ���üӼ��� ���� ���.
	if (!IsWindowVisible(hwnd))
	{
		return TRUE;
	}

	CallbackProcessor* pProcessor = (CallbackProcessor*)lParam;

	// ���� �������� Ŭ���� �̸� ������.
	WCHAR szClassName[MAX_PATH];
	GetClassName(hwnd, szClassName, MAX_PATH);

	// ���ϴ� Ŭ���� �̸����� Ȯ��.
	// �´ٸ� ������ ���̺� �߰�.
	for (SIZE_T i = 0, size = _countof(g_pszCLASS_NAMES); i < size; ++i)
	{
		if (wcsncmp(szClassName, g_pszCLASS_NAMES[i], wcslen(szClassName)) == 0)
		{
			// ������ ũ�� ����.
			RECT rect = { 0, };
			GetWindowRect(hwnd, &rect);

			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			// â ����� 0��.
			if (width == 0 || height == 0)
			{
				break;
			}

			// �ֻ��� â�� �ƴ�.
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
