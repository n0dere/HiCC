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

#include "clrpicker.h"

#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <string.h>

#include "resources.h"

#define CLRPICKERCLASSNAME              TEXT("ColorPickerHiCC")

#define TOOLTIP_TEXT_SZ                 32

typedef struct tagCOLORPICKER
{
    HWND                    hWnd;
    HINSTANCE               hInstance;
    HWND                    hTrackingTT;
    TOOLINFO                toolInfo;
    COLORREF                crColor;
    POINT                   ptCursor;
    POINT                   ptCursorOld;
    TCHAR                   szTooltipText[TOOLTIP_TEXT_SZ];
    WNDPROC                 wpDialogOld;
} *PCOLORPICKER;

extern VOID ColorToSpecialText(COLORREF crColor, DWORD cbSize, LPTSTR pszOut);

static VOID ColorPicker_SetCursor(PCOLORPICKER pPicker, LPCTSTR lpszName)
{
    HCURSOR hCursor = NULL;
    hCursor = LoadCursor(pPicker->hInstance, lpszName);
    SetClassLongPtr(pPicker->hWnd, GCLP_HCURSOR, (LONG_PTR)hCursor);
}

static VOID ColorPicker_OnMouseMove(PCOLORPICKER pPicker, INT x, INT y)
{
    COLORREF crColor;

    if ((x == pPicker->ptCursorOld.x) && (y == pPicker->ptCursorOld.y))
        return;
    
    CopyMemory(&pPicker->ptCursorOld, &pPicker->ptCursor, sizeof(POINT));

    crColor = GetPixel(GetDC(NULL), x, y);

    ColorToSpecialText(crColor, TOOLTIP_TEXT_SZ, pPicker->szTooltipText);
    
    SendMessage(pPicker->hTrackingTT, TTM_SETTOOLINFO, 0,
                (LPARAM)&pPicker->toolInfo);
    
    SendMessage(pPicker->hTrackingTT, TTM_TRACKPOSITION, 0,
                (LPARAM)MAKELONG(x + 25, y - 15));
}

static VOID ColorPicker_OnMouseClick(PCOLORPICKER pPicker, INT x, INT y)
{
    pPicker->crColor = GetPixel(GetDC(NULL), x, y);
    ColorPicker_SetCursor(pPicker, IDC_ARROW);
    EndDialog(pPicker->hWnd, TRUE);
}

static BOOL ColorPicker_OnInit(PCOLORPICKER pPicker)
{
    ColorPicker_SetCursor(pPicker, MAKEINTRESOURCE(IDC_EYEDROPPER));
    
    pPicker->hTrackingTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS,
        NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, pPicker->hWnd, NULL,
        pPicker->hInstance, NULL);
    
    ColorToSpecialText(0, TOOLTIP_TEXT_SZ, pPicker->szTooltipText);
    
    pPicker->toolInfo.cbSize   = sizeof(TOOLINFO);
    pPicker->toolInfo.uFlags   = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
    pPicker->toolInfo.hwnd     = pPicker->hWnd;
    pPicker->toolInfo.hinst    = pPicker->hInstance;
    pPicker->toolInfo.lpszText = pPicker->szTooltipText;
    pPicker->toolInfo.uId      = (UINT_PTR)pPicker->hWnd;
    
    GetClientRect(pPicker->hWnd, &pPicker->toolInfo.rect);

    SendMessage(pPicker->hTrackingTT, TTM_ADDTOOL, 0,
                (LPARAM) (LPTOOLINFO) &pPicker->toolInfo);
    
    SendMessage(pPicker->hTrackingTT, TTM_TRACKACTIVATE, (WPARAM)TRUE,
                (LPARAM) &pPicker->toolInfo);

    return TRUE;
}

static LRESULT CALLBACK ColorPicker_WndProc(HWND hWnd, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam)
{
    PCOLORPICKER pPicker;
    INT x, y;

    pPicker = (PCOLORPICKER)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (uMsg) {
        case WM_MOUSEMOVE:
            x = pPicker->ptCursor.x = GET_X_LPARAM(lParam);
            y = pPicker->ptCursor.y = GET_Y_LPARAM(lParam);

            ColorPicker_OnMouseMove(pPicker, x, y);
            break;
        
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONUP:
            x = pPicker->ptCursor.x;
            y = pPicker->ptCursor.y;

            ColorPicker_OnMouseClick(pPicker, x, y);
            break;
    }

    return CallWindowProc(pPicker->wpDialogOld, hWnd, uMsg, wParam, lParam);
}

static INT_PTR CALLBACK ColorPicker_DlgProc(HWND hDlg, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam)
{
    PCOLORPICKER pPicker;

    if (uMsg != WM_INITDIALOG)
        return (INT_PTR)FALSE;
    
    pPicker = (PCOLORPICKER)lParam;

    pPicker->hWnd = hDlg;
    pPicker->wpDialogOld = (WNDPROC)GetWindowLongPtr(hDlg, GWLP_WNDPROC);

    SetWindowLongPtr(hDlg, GWLP_WNDPROC, (LONG_PTR)ColorPicker_WndProc);
    SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)lParam);

    return (INT_PTR)ColorPicker_OnInit(pPicker);
}

BOOL ColorPicker_PickColorOnScreen(HWND hParent, LPCOLORREF lpcrColorOut)
{
    DLGTEMPLATE dlg = {0};
    INT_PTR ipRes;
    PCOLORPICKER pColorPicker;

    if (lpcrColorOut == NULL)
        return FALSE;

    pColorPicker = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                             sizeof *pColorPicker);
    
    pColorPicker->hInstance = (HINSTANCE)GetWindowLongPtr(hParent,
        GWLP_HINSTANCE);
    
    if (pColorPicker == NULL)
        return FALSE;

    dlg.dwExtendedStyle = WS_EX_LAYERED | WS_EX_TOPMOST;
    dlg.style = WS_POPUP | DS_MODALFRAME;
    dlg.cx = GetSystemMetrics(SM_CXSCREEN);
    dlg.cy = GetSystemMetrics(SM_CYSCREEN);

    ipRes = DialogBoxIndirectParam(NULL, &dlg, hParent,
                                  (DLGPROC)ColorPicker_DlgProc,
                                  (LPARAM)pColorPicker);
    
    *lpcrColorOut = pColorPicker->crColor;

    HeapFree(GetProcessHeap(), 0, pColorPicker);
    return ipRes > 0;
}