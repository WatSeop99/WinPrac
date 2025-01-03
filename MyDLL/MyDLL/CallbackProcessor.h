#pragma once

#include "TypeDef.h"

class CallbackProcessor
{
private:
	static const int MAX_CORE_COUNT = 64;

public:
	CallbackProcessor() = default;
	~CallbackProcessor() { Cleanup(); }

	bool Initialize(HMODULE hModule);

	bool Update(int targetX, int targetY, int targetWidth, int targetHeight);

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
	bool Render(HDC   hdc,
				int   x,
				int   y,
				int   cx,
				int   cy,
				HDC   hdcSrc,
				int   x1,
				int   y1,
				DWORD rop);

	bool Cleanup();

	inline Gdiplus::Bitmap* GetWatermark() { return m_pWatermark; }
	inline Gdiplus::Bitmap* GetWatermarkString() { return m_pWatermarkString; }
	inline Gdiplus::Bitmap* GetWatermarkImage() { return m_pWatermarkImage; }
	inline Gdiplus::Graphics* GetWatermarkGraphics() { return m_pWatermarkGraphics; }

	inline void SetOriginStretchBltFunc(PFnStretchBlt pfnOrigin) { m_pfnOriginStretchBltFunc = pfnOrigin; }
	inline void SetOriginBitBltFunc(PFnBitBlt pfnOrigin) { m_pfnOriginBitBlt = pfnOrigin; }
	inline void SetNullOriginStretchBltFunc() { m_pfnOriginStretchBltFunc = nullptr; }
	inline void SetNullOriginBitBltFunc() { m_pfnOriginBitBlt = nullptr; }

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
	HMODULE m_hModule = nullptr;
	PFnStretchBlt m_pfnOriginStretchBltFunc = nullptr;
	PFnBitBlt m_pfnOriginBitBlt = nullptr;
};
