#pragma once

#include "TypeDef.h"

typedef BOOL(WINAPI* PFnStretchBlt)(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);

class CallbackProcessor
{
private:
	static const int MAX_CORE_COUNT = 64;

public:
	CallbackProcessor() = default;
	~CallbackProcessor() { Cleanup(); }

	bool Initialize();

	bool Update(int targetWidth, int targetHeight);

	bool Render(HDC   hdcDest,
				int   xDest,
				int   yDest,
				int   wDest,
				int   hDest,
				HDC   hdcSrc,
				int   xSrc,
				int   ySrc,
				int   wSrc,
				int   hSrc,
				DWORD rop);

	bool Cleanup();

	inline Gdiplus::Bitmap* GetWatermark() { return m_pWatermark; }
	inline Gdiplus::Bitmap* GetWatermarkString() { return m_pWatermarkString; }
	inline Gdiplus::Bitmap* GetWatermarkImage() { return m_pWatermarkImage; }
	inline Gdiplus::Graphics* GetWatermarkGraphics() { return m_pWatermarkGraphics; }

	inline void SetOriginFunc(PFnStretchBlt pfnOrigin) { m_pfnOriginFunc = pfnOrigin; }
	inline void SetNullOriginFunc() { m_pfnOriginFunc = nullptr; }

public:
	std::unordered_set<HWND> WindowTable;

private:
	Gdiplus::Bitmap* m_pWatermark = nullptr;
	Gdiplus::Bitmap* m_pWatermarkString = nullptr;
	Gdiplus::Bitmap* m_pWatermarkImage = nullptr;
	Gdiplus::Graphics* m_pWatermarkGraphics = nullptr;
	Gdiplus::Graphics* m_pWatermarkStringGraphics = nullptr;
	Gdiplus::Graphics* m_pWatermarkImageGraphics = nullptr;

	ULONG_PTR m_Token = 0;
	Gdiplus::GdiplusStartupInput m_Input;

	// DO NOT Release directly.
	PFnStretchBlt m_pfnOriginFunc = nullptr;
};
