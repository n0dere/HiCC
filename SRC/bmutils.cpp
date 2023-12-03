/*
    This file is part of Hilight Color Changer (HiCC).

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with HiCC.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "bmutils.h"

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
    WCHAR szPathWide[MAX_PATH] = {0};
    MultiByteToWideChar(CP_UTF8, 0, pszPath, -1, szPathWide, MAX_PATH);
    gdiBitmap = Gdiplus::Bitmap::FromFile(szPathWide, FALSE);
#endif

    if (gdiBitmap != NULL) {
        gdiBitmap->GetHBITMAP(Gdiplus::Color(255, 255, 255), &hBitmap);
        delete gdiBitmap;
    }

    Gdiplus::GdiplusShutdown(gdiToken);
    return hBitmap;
}

HBITMAP HBITMAP_FromWindow(HWND hWnd)
{
    INT iWidth, iHeight;
    HDC hWindowDC, hMemDC;
    HBITMAP hBitmap = NULL;
    RECT rc;

    hWindowDC = GetDC(hWnd);
    hMemDC = CreateCompatibleDC(hWindowDC);
    
    GetClientRect(hWnd, &rc);
    
    iWidth = rc.right - rc.left;
    iHeight = rc.bottom - rc.top;
    
    hBitmap = CreateCompatibleBitmap(hWindowDC, iWidth, iHeight);

    SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, iWidth, iHeight, hWindowDC, 0, 0, SRCCOPY);
    ReleaseDC(hWnd, hWindowDC);
    DeleteDC(hMemDC);

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