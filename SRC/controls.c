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

#include "controls.h"

#include "resources.h"

#define LOADSTRING_MAX_SZ           512

static void DrawTransparentRectangle(HDC hDC, LPRECT rect, COLORREF color,
                                     BYTE bAlpha)
{
    HDC hMemDC = CreateCompatibleDC(hDC);
    INT iWidth = rect->right - rect->left;
    INT iHeight = rect->bottom - rect->top;
    HBITMAP bitmap = CreateCompatibleBitmap(hDC, iWidth, iHeight);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(hMemDC, bitmap);
    HBRUSH brush = CreateSolidBrush(color);
    RECT fillRect = { 0, 0, iWidth, iHeight };
    BLENDFUNCTION blend;

    FillRect(hMemDC, &fillRect, brush);
    DeleteObject(brush);

    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = bAlpha;
    blend.AlphaFormat = 0;

    AlphaBlend(hDC, rect->left, rect->top, iWidth, iHeight, hMemDC, 0, 0,
               iWidth, iHeight, blend);

    SelectObject(hMemDC, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(hMemDC);
}

/* HACK:
 *
 * MINGW windres does not support resource files in UTF-16LE format, so
 * resource.rc had to be converted to UTF-8. And due to the fact that Windows
 * applications do not support UTF-8 strings from this resource file,
 * I had to write a custom LoadString :D
 * 
 */
BOOL LoadStringUTF8(UINT uId, LPTSTR pszBuffer, DWORD dwMaxSize)
{
    CHAR *szTmpBuffer = NULL;

    if (pszBuffer == NULL || dwMaxSize <= 0)
        return FALSE;

    szTmpBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                            dwMaxSize * (sizeof *szTmpBuffer));

    if (szTmpBuffer == NULL)
        return FALSE;

    if (LoadStringA(NULL, uId, szTmpBuffer, dwMaxSize) <= 0) {
        HeapFree(GetProcessHeap(), 0, szTmpBuffer);
        return FALSE;
    }

#if _UNICODE
    MultiByteToWideChar(CP_UTF8, 0, szTmpBuffer, -1, pszBuffer, dwMaxSize);
#else
    strncpy(pszBuffer, szTmpBuffer, (size_t)dwMaxSize);
#endif
    
    HeapFree(GetProcessHeap(), 0, szTmpBuffer);
    return TRUE;
}

typedef struct tagPREVIEW
{
    HBITMAP             hBitmapBG;
    COLORREF            crHilight;
    COLORREF            crHotTrackingColor;
} *PPREVIEW;

static void Preview_PaintBG(PPREVIEW pPreview, HDC hDC, PRECT pRect)
{
    HDC hDCBitmap;
    HBITMAP hOldBitmap;
    BITMAP bm;
    INT x, y;

    if (pPreview == NULL || hDC == NULL || pRect == NULL)
        return;

    if (pPreview->hBitmapBG == NULL) {
        FillRect(hDC, pRect, GetSysColorBrush(COLOR_BACKGROUND));
        return;
    }

    GetObject(pPreview->hBitmapBG, sizeof bm, (LPVOID)&bm);

    if (bm.bmWidth < pRect->right || bm.bmHeight < pRect->bottom)
        FillRect(hDC, pRect, GetSysColorBrush(COLOR_BACKGROUND));

    hDCBitmap = CreateCompatibleDC(hDC);

    if (hDCBitmap == NULL)
        return;

    hOldBitmap = SelectObject(hDCBitmap, pPreview->hBitmapBG);

    x = bm.bmWidth - (INT)((pRect->right + bm.bmWidth) / 2.0);
    y = bm.bmHeight - (INT)((pRect->bottom + bm.bmHeight) / 2.0);

    BitBlt(hDC, 0, 0, pRect->right, pRect->bottom, hDCBitmap, x, y, SRCCOPY);

    SelectObject(hDCBitmap, hOldBitmap);
    DeleteDC(hDCBitmap);
}

static void Preview_OnPaint(PPREVIEW pPreview, HDC hDC, PRECT pRect)
{
    HBRUSH hBrush;
    INT x, y;

    if (pPreview == NULL || hDC == NULL || pRect == NULL)
        return;

    Preview_PaintBG(pPreview, hDC, pRect);

    x = (INT)(pRect->left + pRect->right * 0.2); /* 20% */
    y = (INT)(pRect->top + pRect->bottom * 0.2); /* 20% */

    InflateRect(pRect, -x, -y);

    DrawTransparentRectangle(hDC, pRect, pPreview->crHotTrackingColor, 65);

    hBrush = CreateSolidBrush(pPreview->crHilight);
    FrameRect(hDC, pRect, hBrush);
    DeleteObject(hBrush);

    DrawIcon(hDC, pRect->right, pRect->bottom, LoadCursor(NULL, IDC_ARROW));

    InflateRect(pRect, x, y);

    FrameRect(hDC, pRect, GetSysColorBrush(COLOR_BTNSHADOW));
}

static LRESULT CALLBACK Preview_Proc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam)
{
    PPREVIEW pPreview = (PPREVIEW)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    PAINTSTRUCT ps;
    HDC hDC;
    RECT rect;

    switch (uMsg) {
        case WM_NCCREATE:
            pPreview = HeapAlloc(GetProcessHeap(), 0, sizeof * pPreview);

            if (pPreview == NULL)
                return FALSE;

            ZeroMemory(pPreview, sizeof * pPreview);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pPreview);
            break;

        case XXM_PREVIEW_UPDATE_BG:
            pPreview->hBitmapBG = (HBITMAP)lParam;
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case XXM_PREVIEW_UPDATE_HILIGHT:
            pPreview->crHilight = (COLORREF)lParam;
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case XXM_PREVIEW_UPDATE_HTC:
            pPreview->crHotTrackingColor = (COLORREF)lParam;
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case WM_PAINT:
            GetClientRect(hWnd, &rect);
            hDC = BeginPaint(hWnd, &ps);
            Preview_OnPaint(pPreview, hDC, &rect);
            EndPaint(hWnd, &ps);
            break;

        case WM_NCDESTROY:
            if (pPreview != NULL)
                HeapFree(GetProcessHeap(), 0, pPreview);
            break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND Preview_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
                    INT nHeight)
{
    return CreateWindowEx(WS_EX_COMPOSITED, PREVIEWCLASSNAME, NULL,
                          WS_CHILD | WS_VISIBLE, x, y, nWidth, nHeight,
                          hParent, (HMENU)(UINT_PTR)uId, NULL, NULL);
}

VOID Preview_UpdateColors(HWND hPreview, COLORREF crHi, COLORREF crHTC)
{
    if (hPreview == NULL)
        return;

    SendMessage(hPreview, XXM_PREVIEW_UPDATE_HILIGHT, 0, (LPARAM)crHi);
    SendMessage(hPreview, XXM_PREVIEW_UPDATE_HTC, 0, (LPARAM)crHTC);
}

VOID Preview_UpdateBG(HWND hPreview, HBITMAP hbmBG)
{
    if (hPreview == NULL || hbmBG == NULL)
        return;

    SendMessage(hPreview, XXM_PREVIEW_UPDATE_BG, 0, (LPARAM)hbmBG);
}

typedef struct tagPALETTE
{
    POINT                   pointCursor;
    PKM_PALETTE             pKmPalette;
    BOOL                    bLMB_Down;
    DOUBLE                  dbSegWidth;
    LPTSTR                  pszErrorMsg;
} *PPALETTE;

static void Palette_PaintEmpty(PPALETTE pPalette, HDC hDC, PRECT pRect)
{
    FillRect(hDC, pRect, GetSysColorBrush(COLOR_BACKGROUND));

    if (pPalette->pszErrorMsg != NULL) {
        SetTextColor(hDC, RGB(255, 255, 255));
        SetBkMode(hDC, TRANSPARENT);

        DrawText(hDC, pPalette->pszErrorMsg, -1, pRect,
                 DT_VCENTER | DT_SINGLELINE | DT_CENTER);
    }
    
    DrawEdge(hDC, pRect, EDGE_SUNKEN, BF_RECT);
}

static void Palette_OnPaint(PPALETTE pPalette, HDC hDC, PRECT pRect)
{
    HBRUSH hBrush = NULL;
    DWORD dwCount = 0;
    BOOL bInRect = FALSE;
    INT i;

    if (pPalette == NULL || hDC == NULL || pRect == NULL)
        return;

    if (pPalette->pKmPalette == NULL) {
        Palette_PaintEmpty(pPalette, hDC, pRect);
        return;
    }

    dwCount = pPalette->pKmPalette->dwColorsCount;

    for (i = 0; i < (INT)dwCount; i++) {
        hBrush = CreateSolidBrush(pPalette->pKmPalette->aColors[i]);

        pRect->left = (LONG)(pPalette->dbSegWidth * i);
        pRect->right = (LONG)(pPalette->dbSegWidth * (i + 1));

        FillRect(hDC, pRect, hBrush);

        bInRect = PtInRect(pRect, pPalette->pointCursor);

        if (pPalette->bLMB_Down && bInRect) {
            InflateRect(pRect, -1, -1);
            DrawFocusRect(hDC, pRect);
            InflateRect(pRect, 1, 1);
        }

        DeleteObject(hBrush);
    }

    pRect->left = 0;
    pRect->right = (LONG)(pPalette->dbSegWidth * dwCount);

    FrameRect(hDC, pRect, GetSysColorBrush(COLOR_BTNSHADOW));
}

static void Palette_OnMouseUp(PPALETTE pPalette, HWND hWnd)
{
    INT i;

    if (pPalette == NULL || hWnd == NULL || pPalette->pKmPalette == NULL)
        return;

    i = (INT)(pPalette->pointCursor.x / pPalette->dbSegWidth);

    SendMessage(GetParent(hWnd), XXM_PALETTE_CLRSEL, 0,
                (LPARAM)pPalette->pKmPalette->aColors[i]);
}

static LRESULT CALLBACK Palette_Proc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam)
{
    PPALETTE pPalette = (PPALETTE)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    TRACKMOUSEEVENT tme;
    PAINTSTRUCT ps;
    HDC hDC;
    RECT rect;
    DWORD dwColors;

    switch (uMsg) {
        case WM_NCCREATE:
            pPalette = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                 sizeof * pPalette);

            if (pPalette == NULL)
                return FALSE;

            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pPalette);
            break;

        case XXM_PALETTE_UPDATE_PALETTE:
            pPalette->pKmPalette = (PKM_PALETTE)lParam;
            
            if (pPalette->pKmPalette != NULL) {
                GetClientRect(hWnd, &rect);
                dwColors = pPalette->pKmPalette->dwColorsCount;
                pPalette->dbSegWidth = ((DOUBLE)rect.right) / dwColors;
            }

            break;
        
        case WM_SETCURSOR:
            if (LOWORD(lParam) == HTCLIENT) {
                if (pPalette->pKmPalette != NULL)
                    SetCursor(LoadCursor(NULL, IDC_HAND));
                else
                    SetCursor(LoadCursor(NULL, IDC_ARROW));

                return TRUE;
            }
            break;

        case XXM_PALETTE_ERROR_MSG:
            pPalette->pszErrorMsg = (LPTSTR)lParam;
            break;

        case WM_PAINT:
            GetClientRect(hWnd, &rect);
            hDC = BeginPaint(hWnd, &ps);
            Palette_OnPaint(pPalette, hDC, &rect);
            EndPaint(hWnd, &ps);
            break;

        case WM_MOUSEMOVE:
            pPalette->pointCursor.x = GET_X_LPARAM(lParam);
            pPalette->pointCursor.y = GET_Y_LPARAM(lParam);
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hWnd;
            tme.dwHoverTime = HOVER_DEFAULT;
            TrackMouseEvent(&tme);
            break;

        case WM_LBUTTONUP:
            Palette_OnMouseUp(pPalette, hWnd);

        case WM_MOUSELEAVE:
        case WM_LBUTTONDOWN:
            pPalette->bLMB_Down = (uMsg == WM_LBUTTONDOWN);
            InvalidateRect(hWnd, NULL, FALSE);
            break;

        case WM_NCDESTROY:
            if (pPalette != NULL) {
                if (pPalette->pszErrorMsg != NULL)
                    HeapFree(GetProcessHeap(), 0, pPalette->pszErrorMsg);

                HeapFree(GetProcessHeap(), 0, pPalette);
            }
            break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND Palette_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
                    INT nHeight, PKM_PALETTE pKmPalette)
{
    HWND hWnd = CreateWindow(PALETTECLASSNAME, NULL, WS_CHILD | WS_VISIBLE,
                             x, y, nWidth, nHeight, hParent,
                             (HMENU)(UINT_PTR)uId, NULL, NULL);

    SendMessage(hWnd, XXM_PALETTE_UPDATE_PALETTE, 0, (LPARAM)pKmPalette);

    return hWnd;
}

VOID Palette_SetError(HWND hPalette, UINT uMsgId)
{
    LPTSTR szName = NULL;

    if (hPalette != NULL && uMsgId != 0) {
        szName = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                           LOADSTRING_MAX_SZ * (sizeof * szName));

        if (szName != NULL)
            LoadStringUTF8(uMsgId, szName, LOADSTRING_MAX_SZ);
    }

    if (hPalette != NULL)
        SendMessage(hPalette, XXM_PALETTE_ERROR_MSG, 0, (LPARAM)szName);
}

static LRESULT CALLBACK ColorBox_Proc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                      LPARAM lParam)
{
    LPCOLORREF lpcrColor = (LPCOLORREF)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    PAINTSTRUCT ps;
    RECT rect;
    HBRUSH hBrush = NULL;

    switch (uMsg) {
    case WM_NCCREATE:
        lpcrColor = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                              sizeof * lpcrColor);

        if (lpcrColor == NULL)
            return FALSE;

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)lpcrColor);
        break;

    case XXM_COLORBOX_SET_COLOR:
        *lpcrColor = (COLORREF)lParam;
        InvalidateRect(hWnd, NULL, FALSE);
        break;

    case WM_PAINT:
        GetClientRect(hWnd, &rect);
        BeginPaint(hWnd, &ps);
        hBrush = CreateSolidBrush(*lpcrColor);
        FillRect(ps.hdc, &rect, hBrush);
        DeleteObject(hBrush);
        FrameRect(ps.hdc, &rect, GetSysColorBrush(COLOR_BTNSHADOW));
        EndPaint(hWnd, &ps);
        break;
    
    case WM_NCDESTROY:
        if (lpcrColor != NULL)
            HeapFree(GetProcessHeap(), 0, lpcrColor);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND ColorBox_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
                     INT nHeight)
{
    return CreateWindow(COLORBOXCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, x, y,
                        nWidth, nHeight, hParent, (HMENU)(UINT_PTR)uId, NULL,
                        NULL);
}

VOID ColorBox_ChangeColor(HWND hColorBox, COLORREF crColor)
{
    if (hColorBox != NULL)
        SendMessage(hColorBox, XXM_COLORBOX_SET_COLOR, 0, (LPARAM)crColor);
}

static HWND Window_Create(LPCTSTR pszClassName, DWORD dwStyleEx, DWORD dwStyle,
                          HWND hParent, UINT uId, INT x, INT y, INT nWidth,
                          INT nHeight, UINT uNameId)
{
    TCHAR szName[LOADSTRING_MAX_SZ] = {0};

    if (uNameId != 0)
        LoadStringUTF8(uNameId, szName, LOADSTRING_MAX_SZ);

    return CreateWindowEx(dwStyleEx, pszClassName, szName, dwStyle, x, y,
                          nWidth, nHeight, hParent, (HMENU)(UINT_PTR)uId,
                          NULL, NULL);
}

HWND Button_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
                   INT nHeight, UINT uNameId)
{
    return Window_Create(TEXT("Button"), 0, WS_VISIBLE | WS_CHILD, hParent,
                         uId, x, y, nWidth, nHeight, uNameId);
}

HWND Static_Create(HWND hParent, INT x, INT y, INT nWidth, INT nHeight,
                   UINT uTextId)
{
    return Window_Create(TEXT("Static"), 0, WS_VISIBLE | WS_CHILD, hParent, 0,
                         x, y, nWidth, nHeight, uTextId);
}

HWND GroupBox_Create(HWND hParent, INT x, INT y, INT nWidth, INT nHeight,
                     UINT uTextId)
{
    return Window_Create(TEXT("Button"), 0,
                         WS_VISIBLE | WS_CHILD | WS_GROUP | BS_GROUPBOX,
                         hParent, 0, x, y, nWidth, nHeight, uTextId);
}

HWND Edit_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
                 INT nHeight)
{
    return Window_Create(TEXT("Edit"), WS_EX_CLIENTEDGE,
                         WS_VISIBLE | WS_CHILD | ES_LEFT | ES_CENTER, hParent,
                         uId, x, y, nWidth, nHeight, 0);
}

BOOL Controls_RegisterAllClasses(VOID)
{
    WNDCLASS wndClass;

    ZeroMemory(&wndClass, sizeof wndClass);

    wndClass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = Preview_Proc;
    wndClass.lpszClassName = PREVIEWCLASSNAME;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (RegisterClass(&wndClass) == 0)
        return FALSE;

    wndClass.lpfnWndProc = Palette_Proc;
    wndClass.lpszClassName = PALETTECLASSNAME;
    wndClass.hCursor = LoadCursor(NULL, IDC_HAND);

    if (RegisterClass(&wndClass) == 0)
        return FALSE;

    wndClass.lpfnWndProc = ColorBox_Proc;
    wndClass.lpszClassName = COLORBOXCLASSNAME;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (RegisterClass(&wndClass) == 0)
        return FALSE;

    return TRUE;
}

VOID Controls_UnregisterAllClasses(VOID)
{
    UnregisterClass(COLORBOXCLASSNAME, NULL);
    UnregisterClass(PALETTECLASSNAME, NULL);
    UnregisterClass(PREVIEWCLASSNAME, NULL);
}