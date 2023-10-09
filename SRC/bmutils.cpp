#include "bmutils.h"

#include <tchar.h>
#include <math.h>
#include <gdiplus.h>

HBITMAP HBITMAP_FromFile(LPCTSTR pszPath)
{
    Gdiplus::GdiplusStartupInput gdiInput;
    ULONG_PTR gdiToken;
    HBITMAP hBitmap = NULL;

    if (Gdiplus::GdiplusStartup(&gdiToken, &gdiInput, NULL) != Gdiplus::Ok)
        return NULL;

    if (pszPath == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    Gdiplus::Bitmap* gdiBitmap;

#if _UNICODE
    gdiBitmap = Gdiplus::Bitmap::FromFile(pszPath, FALSE);
#else
    WCHAR szPathWide[MAX_PATH] = { 0 };
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszPath, -1, szPathWide, 0);
    gdiBitmap = Gdiplus::Bitmap::FromFile(szPathWide, FALSE);
#endif

    if (gdiBitmap != NULL) {
        gdiBitmap->GetHBITMAP(Gdiplus::Color(255, 255, 255), &hBitmap);
        delete gdiBitmap;
    }

    Gdiplus::GdiplusShutdown(gdiToken);
    return hBitmap;
}

LPRGBQUAD HBITMAP_GetPixels(HBITMAP hBitmap, LPDWORD lpcPixelCount)
{
    BITMAPINFO bmInfo;
    HDC hDC = NULL;
    LPRGBQUAD aPixels = NULL;
    INT iResult;

    if (lpcPixelCount == NULL || hBitmap == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    hDC = CreateCompatibleDC(NULL);

    if (hDC == NULL)
        return NULL;

    ZeroMemory(&bmInfo, sizeof(bmInfo));

    bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    iResult = GetDIBits(hDC, hBitmap, 0, 0, NULL, &bmInfo, DIB_RGB_COLORS);

    if (iResult == 0) {
        DeleteDC(hDC);
        return NULL;
    }

    bmInfo.bmiHeader.biCompression = BI_RGB;

    aPixels = (LPRGBQUAD)HeapAlloc(GetProcessHeap(), 0,
                                   bmInfo.bmiHeader.biSizeImage);

    if (aPixels == NULL) {
        DeleteDC(hDC);
        return NULL;
    }

    iResult = GetDIBits(hDC, hBitmap, 0, bmInfo.bmiHeader.biHeight, aPixels,
                        &bmInfo, DIB_RGB_COLORS);

    if (iResult == 0) {
        HeapFree(GetProcessHeap(), 0, aPixels);
        DeleteDC(hDC);
        return NULL;
    }

    DeleteDC(hDC);

    *lpcPixelCount = bmInfo.bmiHeader.biSizeImage / (sizeof *aPixels);

    return aPixels;
}

HBITMAP HBITMAP_Resize(HBITMAP hBitmap, UINT uNewWidth, UINT uNewHeight)
{
    Gdiplus::GdiplusStartupInput gdiInput;
    ULONG_PTR gdiToken;
    HBITMAP hResult = NULL;

    if (hBitmap == NULL || uNewWidth <= 0 || uNewHeight <= 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if (Gdiplus::GdiplusStartup(&gdiToken, &gdiInput, NULL) != Gdiplus::Ok)
        return NULL;

    Gdiplus::Bitmap* gdiBitmap = Gdiplus::Bitmap::FromHBITMAP(hBitmap, NULL);

    if (gdiBitmap != NULL) {
        Gdiplus::Bitmap* gdiTmpBitmap;

        gdiTmpBitmap = new Gdiplus::Bitmap(uNewWidth, uNewHeight);

        Gdiplus::Graphics graphics(gdiTmpBitmap);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeBilinear);
        graphics.DrawImage(gdiBitmap, 0, 0, uNewWidth, uNewHeight);

        gdiTmpBitmap->GetHBITMAP(Gdiplus::Color(255, 255, 255), &hResult);

        delete gdiTmpBitmap;
        delete gdiBitmap;
    }

    Gdiplus::GdiplusShutdown(gdiToken);
    return hResult;
}