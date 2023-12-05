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

#include <Windows.h>
#include <tchar.h>

#include "registry.h"
#include "wallpaper.h"
#include "clrkmeans.h"
#include "resources.h"
#include "controls.h"
#include "syscolors.h"
#include "bmutils.h"
#include "clrpicker.h"
#include "wndcenter.h"

#define DW_PALETTE_K                7

#define WINDOW_WIDTH                415
#define WINDOW_HEIGHT               480

#define CHANGE_HILIGHT              0
#define CHANGE_HTC                  1

#define WINDOW_TITLE_BUFFER_SZ      256
#define COLOR_TEXT_BUFFER_SZ        32

static const TCHAR *g_pszWindowClassName = TEXT("HiCC MainWindow");

static TCHAR g_szWindowTitle[WINDOW_TITLE_BUFFER_SZ];

static HWND g_hMainWindow = NULL;

typedef struct tagMAINWINDOW
{
    HWND                hWnd;
    HINSTANCE           hInstance;
    HWND                hWndPreview;
    HBITMAP             hbmWallpaper;
    PKM_PALETTE         pkmPalette;
    COLORREF            crHilight;
    COLORREF            crHotTrackingColor;
    COLORREF            acrCustom[16];
} MAINWINDOW, *PMAINWINDOW;

extern VOID AboutBox_Show(HINSTANCE hInstance, HWND hWnd);

VOID ColorToSpecialText(COLORREF crColor, DWORD cbSize, LPTSTR pszOut)
{
    UINT uRed, uGreen, uBlue;

    if (pszOut == NULL || cbSize <= 0)
        return;
    
    uRed = GetRValue(crColor);
    uGreen = GetGValue(crColor);
    uBlue = GetBValue(crColor);

    _sntprintf(pszOut, cbSize, TEXT("#%02X%02X%02X [%u %u %u]"),
               uRed, uGreen, uBlue, uRed, uGreen, uBlue);
}

static BOOL CALLBACK SetFontCallback(HWND child, LPARAM lParam)
{
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(child, WM_SETFONT, (WPARAM)hFont, TRUE);
    return TRUE;
}

static UINT_PTR CALLBACK ColorSelect_Hook(HWND hDlg, UINT uMsg,
                                          WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_INITDIALOG:
            MoveWindowToParentCenter(GetParent(hDlg), hDlg);
            break;
    }

    return (UINT_PTR)FALSE;
}

static BOOL MainWindow_ColorSelect(PMAINWINDOW pMainWnd, LPCOLORREF lpcrSel)
{
    CHOOSECOLOR cc;

    if (pMainWnd == NULL || lpcrSel == NULL)
        return FALSE;

    ZeroMemory(&cc, sizeof cc);

    cc.lStructSize = sizeof cc;
    cc.hwndOwner = pMainWnd->hWnd;
    cc.lpCustColors = (LPDWORD)pMainWnd->acrCustom;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ENABLEHOOK;
    cc.rgbResult = *lpcrSel;
    cc.lpfnHook = ColorSelect_Hook;

    if (ChooseColor(&cc) == TRUE) {
        *lpcrSel = cc.rgbResult;
        return TRUE;
    }

    return FALSE;
}

static BOOL MainWindow_UpdateColors(PMAINWINDOW pMainWnd, BOOL bEnableApply)
{
    COLORREF crHi, crHTC;
    TCHAR szHiText[COLOR_TEXT_BUFFER_SZ];
    TCHAR szHTCText[COLOR_TEXT_BUFFER_SZ];

    if (pMainWnd == NULL)
        return FALSE;

    crHi = pMainWnd->crHilight;
    crHTC = pMainWnd->crHotTrackingColor;

    ColorToSpecialText(crHi, COLOR_TEXT_BUFFER_SZ, szHiText);
    ColorToSpecialText(crHTC, COLOR_TEXT_BUFFER_SZ, szHTCText);

    Preview_UpdateColors(pMainWnd->hWndPreview, crHi, crHTC);
    ColorBox_ChangeColor(GetDlgItem(pMainWnd->hWnd, IDC_CLRBOX_HI), crHi);
    ColorBox_ChangeColor(GetDlgItem(pMainWnd->hWnd, IDC_CLRBOX_HTC), crHTC);
    Edit_SetText(GetDlgItem(pMainWnd->hWnd, IDC_EDIT_HI), szHiText);
    Edit_SetText(GetDlgItem(pMainWnd->hWnd, IDC_EDIT_HTC), szHTCText);

    EnableWindow(GetDlgItem(pMainWnd->hWnd, IDC_BUTTON_APPLY), bEnableApply);

    return TRUE;
}

static BOOL UpdateAllSystemColors(VOID)
{
    INT aElements[SYSTEM_COLORS_COUNT];
    COLORREF aColors[SYSTEM_COLORS_COUNT];
    SIZE_T i;

    for (i = 0; i < SYSTEM_COLORS_COUNT; i++) {
        aElements[i] = g_v_SystemColors[i].nId;
        aColors[i] = ColorsRegistryGet(g_v_SystemColors[i].pszValueName);
    }

    return SetSysColors(SYSTEM_COLORS_COUNT, aElements, aColors);
}

static BOOL ResetButton_OnClick(PMAINWINDOW pMainWnd)
{
    if (pMainWnd == NULL)
        return FALSE;
    
    if (HiCCRegistryGetResetAll() == TRUE) {
        if (ColorsRegistryResetToDefaultAll() != ERROR_SUCCESS)
            return FALSE;
    }
    else {
        if (ColorsRegistryResetToDefault(COLOR_HIGHLIGHT) != ERROR_SUCCESS)
            return FALSE;
        
        if (ColorsRegistryResetToDefault(COLOR_HOTLIGHT) != ERROR_SUCCESS)
            return FALSE;
    }

    if (UpdateAllSystemColors() != TRUE)
        return FALSE;
    
    pMainWnd->crHilight = ColorsRegistryGetHilight();
    pMainWnd->crHotTrackingColor = ColorsRegistryGetHTC();

    return MainWindow_UpdateColors(pMainWnd, FALSE);
}

static BOOL ApplyButton_OnClick(PMAINWINDOW pMainWnd)
{
    if (ColorsRegistrySetHTC(pMainWnd->crHotTrackingColor) != ERROR_SUCCESS)
        return FALSE;

    if (ColorsRegistrySetHilight(pMainWnd->crHilight) != ERROR_SUCCESS)
        return FALSE;

    if (UpdateAllSystemColors() != TRUE)
        return FALSE;

    return MainWindow_UpdateColors(pMainWnd, FALSE);
}

static BOOL PickColor_OnClick(PMAINWINDOW pMainWnd, UINT uFlag)
{
    COLORREF crNew;

    if (ColorPicker_PickColorOnScreen(pMainWnd->hWnd, &crNew)) {
        if (uFlag == CHANGE_HILIGHT)
            pMainWnd->crHilight = crNew;
        else
            pMainWnd->crHotTrackingColor = crNew;

        return MainWindow_UpdateColors(pMainWnd, TRUE);
    }
    
    return TRUE;
}

static BOOL Change_OnClick(PMAINWINDOW pMainWnd, UINT uFlag)
{
    LPCOLORREF lpcrToChange;

    if (uFlag == CHANGE_HILIGHT)
        lpcrToChange = &pMainWnd->crHilight;
    else
        lpcrToChange = &pMainWnd->crHotTrackingColor;

    if (MainWindow_ColorSelect(pMainWnd, lpcrToChange) == FALSE)
        return TRUE;

    return MainWindow_UpdateColors(pMainWnd, TRUE);
}

static VOID MainWindow_ShowErrorDialog(HWND hWnd)
{
    LPTSTR lpszErrorMessage = NULL;

    static const DWORD dwFmtFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                    FORMAT_MESSAGE_FROM_SYSTEM     |
                                    FORMAT_MESSAGE_IGNORE_INSERTS;

    FormatMessage(dwFmtFlags, NULL, GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&lpszErrorMessage, 0, NULL);

    if (lpszErrorMessage != NULL) {
        MessageBox(hWnd, lpszErrorMessage, g_szWindowTitle,
                   MB_OK | MB_ICONERROR);

        HeapFree(GetProcessHeap(), 0, lpszErrorMessage);
    }
}

static BOOL MainWindow_OnCommand(PMAINWINDOW pMainWnd, WORD wId)
{
    switch (wId) {
        case IDC_BUTTON_CHANGE_HI:
            return Change_OnClick(pMainWnd, CHANGE_HILIGHT);

        case IDC_BUTTON_CHANGE_HTC:
            return Change_OnClick(pMainWnd, CHANGE_HTC);
        
        case IDC_BUTTON_PCLR_HI:
            return PickColor_OnClick(pMainWnd, CHANGE_HILIGHT);
        
        case IDC_BUTTON_PCLR_HTC:
            return PickColor_OnClick(pMainWnd, CHANGE_HTC);

        case IDC_BUTTON_RESET:
            return ResetButton_OnClick(pMainWnd);

        case IDC_BUTTON_APPLY:
            return ApplyButton_OnClick(pMainWnd);

        case IDC_BUTTON_CANCEL:
            return DestroyWindow(pMainWnd->hWnd) != FALSE;
        
        case IDOK:
            return ApplyButton_OnClick(pMainWnd) &&
                   DestroyWindow(pMainWnd->hWnd) != FALSE;
    }
    
    return TRUE;
}

static BOOL MainWindow_OnCreate(PMAINWINDOW pMainWnd, HWND hWnd)
{
    InitCommonControls();

    pMainWnd->hWnd = hWnd;
    pMainWnd->hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
    pMainWnd->hbmWallpaper = DesktopWallpaperGetHBITMAP();
    pMainWnd->crHilight = ColorsRegistryGetHilight();
    pMainWnd->crHotTrackingColor = ColorsRegistryGetHTC();

    if (HiCCRegistryRestoreWindowPosition(pMainWnd->hWnd) != ERROR_SUCCESS)
        MoveWindowToScreenCenter(pMainWnd->hWnd);

    if (Controls_RegisterAllClasses(pMainWnd->hInstance) == FALSE)
        return FALSE;

    if (pMainWnd->hbmWallpaper != NULL) {
        pMainWnd->pkmPalette = KM_GeneratePaletteFromHBITMAP(DW_PALETTE_K,
            pMainWnd->hbmWallpaper);

        KM_SortPaletteByBrightness(pMainWnd->pkmPalette);
    }

    GroupBox_Create(hWnd, 7, 7, 385, 243, IDS_PREVIEW);

    pMainWnd->hWndPreview = Preview_Create(hWnd, 0, 15, 26, 369, 191);

    Preview_UpdateBG(pMainWnd->hWndPreview, pMainWnd->hbmWallpaper);

    Palette_Create(hWnd, IDC_PALETTE, 15, 216, 369, 25, pMainWnd->pkmPalette);

    if (pMainWnd->hbmWallpaper == NULL)
        Palette_SetError(GetDlgItem(hWnd, IDC_PALETTE), IDS_NO_WALLPAPER);

    GroupBox_Create(hWnd, 7, 252, 385, 74, IDS_HILIGHT);
    ColorBox_Create(hWnd, IDC_CLRBOX_HI, 15, 271, 23, 23);
    Edit_Create(hWnd, IDC_EDIT_HI, 48, 271, 160, 23);
    Edit_SetReadOnly(GetDlgItem(hWnd, IDC_EDIT_HI), TRUE);
    Button_Create(hWnd, IDC_BUTTON_CHANGE_HI, 305, 271, 80, 23, IDS_CHANGE);
    BitmapButton_Create(hWnd, IDC_BUTTON_PCLR_HI, 276, 271, 23, 23, NULL);
    Button_SetIcon(GetDlgItem(hWnd, IDC_BUTTON_PCLR_HI),
        LoadIcon(pMainWnd->hInstance, MAKEINTRESOURCE(IDI_EYEDROPPER)));
    Static_Create(hWnd, 15, 304, 270, 17, IDS_DESC_HI);

    GroupBox_Create(hWnd, 7, 328, 385, 74, IDS_HTC);
    ColorBox_Create(hWnd, IDC_CLRBOX_HTC, 15, 347, 23, 23);
    Edit_Create(hWnd, IDC_EDIT_HTC, 48, 347, 160, 23);
    Edit_SetReadOnly(GetDlgItem(hWnd, IDC_EDIT_HTC), TRUE);
    Button_Create(hWnd, IDC_BUTTON_CHANGE_HTC, 305, 347, 80, 23, IDS_CHANGE);
    BitmapButton_Create(hWnd, IDC_BUTTON_PCLR_HTC, 276, 347, 23, 23, NULL);
    Button_SetIcon(GetDlgItem(hWnd, IDC_BUTTON_PCLR_HTC),
        LoadIcon(pMainWnd->hInstance, MAKEINTRESOURCE(IDI_EYEDROPPER)));
    Static_Create(hWnd, 15, 380, 270, 17, IDS_DESC_HTC);

    Button_Create(hWnd, IDC_BUTTON_RESET, 6, 411, 120, 23, IDS_RESET);
    SetFocus(Button_Create(hWnd, IDOK, 162, 411, 73, 23, IDS_OK));
    Button_Create(hWnd, IDC_BUTTON_CANCEL, 241, 411, 73, 23, IDS_CANCEL);
    Button_Create(hWnd, IDC_BUTTON_APPLY, 320, 411, 73, 23, IDS_APPLY);
    Button_SetDefault(GetDlgItem(hWnd, IDOK));

    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_APPLY), FALSE);

    EnumChildWindows(hWnd, (WNDENUMPROC)SetFontCallback, 0);
    
    HiCCRegistryGetColors(pMainWnd->acrCustom);
    
    return MainWindow_UpdateColors(pMainWnd, FALSE);
}

static VOID MainWindow_OnDestroy(PMAINWINDOW pMainWnd, HWND hWnd)
{
    HiCCRegistrySaveWindowPosition(pMainWnd->hWnd);
    HiCCRegistrySaveColors(pMainWnd->acrCustom);
    KM_FreePalette(pMainWnd->pkmPalette);
    DeleteObject(pMainWnd->hbmWallpaper);
    Controls_UnregisterAllClasses(pMainWnd->hInstance);
}

/* bPower in percent */
static inline COLORREF MakeColorDarker(COLORREF crColor, BYTE bPower)
{
    FLOAT fPower = max((1.0f - (((FLOAT)bPower)) / 100.0f), 0);

    return RGB(GetRValue(crColor) * fPower,
               GetGValue(crColor) * fPower,
               GetBValue(crColor) * fPower);
}

static LRESULT CALLBACK MainWindow_Proc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                        LPARAM lParam)
{
    PMAINWINDOW pMainWnd;
    COLORREF crColor;

    pMainWnd = (PMAINWINDOW)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    
    switch (uMsg) {
        case WM_NCCREATE:
            pMainWnd = HeapAlloc(GetProcessHeap(), 0, sizeof * pMainWnd);

            if (pMainWnd == NULL) {
                MainWindow_ShowErrorDialog(hWnd);
                ExitProcess(GetLastError());
            }

            ZeroMemory(pMainWnd, sizeof * pMainWnd);

            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pMainWnd);
            break;

        case WM_CREATE:
            if (MainWindow_OnCreate(pMainWnd, hWnd) == FALSE) {
                MainWindow_ShowErrorDialog(hWnd);
                ExitProcess(GetLastError());
            }
            break;

        case WM_DESTROY:
            MainWindow_OnDestroy(pMainWnd, hWnd);
            PostQuitMessage(0);
            break;

        case XXM_PALETTE_CLRSEL:
            pMainWnd->crHilight = (COLORREF)lParam;
            crColor = MakeColorDarker((COLORREF)lParam, 10);
            pMainWnd->crHotTrackingColor = crColor;
            MainWindow_UpdateColors(pMainWnd, TRUE);
            break;

        case WM_COMMAND:
            if (MainWindow_OnCommand(pMainWnd, LOWORD(wParam)) == FALSE)
                MainWindow_ShowErrorDialog(hWnd);
            break;
        
        case WM_SYSCOMMAND:
            if (wParam == SC_CONTEXTHELP) {
                AboutBox_Show(pMainWnd->hInstance, pMainWnd->hWnd);
                return 0;
            }
            break;

        case WM_NCDESTROY:
            if (pMainWnd != NULL)
                HeapFree(GetProcessHeap(), 0, (LPVOID)pMainWnd);
            break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static HWND MainWindow_Create(HINSTANCE hInstance)
{
    WNDCLASS wndClass;
    DWORD dwStyle = WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME;
    DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_CONTEXTHELP;
    HWND hWnd = NULL;

    ZeroMemory(&wndClass, sizeof wndClass);

    wndClass.lpfnWndProc = MainWindow_Proc;
    wndClass.hInstance = hInstance;
    wndClass.lpszClassName = g_pszWindowClassName;
    wndClass.cbWndExtra = sizeof(MAINWINDOW);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));

    if (RegisterClass(&wndClass) == 0)
        return NULL;

    return CreateWindowEx(dwExStyle, g_pszWindowClassName, g_szWindowTitle,
                          dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH,
                          WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
}

INT WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     PTSTR pCmdLine, INT nCmdShow)
{
    MSG msg;

    LoadStringUTF8(IDS_WINDOW_TITLE, g_szWindowTitle, WINDOW_TITLE_BUFFER_SZ);

    if ((g_hMainWindow = MainWindow_Create(hInstance)) == NULL) {
        MainWindow_ShowErrorDialog(NULL);
        return -1;
    }
    
    ShowWindow(g_hMainWindow, nCmdShow);

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}