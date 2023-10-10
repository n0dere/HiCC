#include <Windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <windowsx.h>

#include "clrregistry.h"
#include "wallpaper.h"
#include "clrkmeans.h"
#include "resource.h"
#include "controls.h"

#define DW_PALETTE_K                7

#define WINDOW_WIDTH                450
#define WINDOW_HEIGHT               600

static const TCHAR *g_pszWindowClassName = TEXT("HiCC MainWindow");
static const TCHAR *g_pszWindowName = TEXT("Hilight Color Changer");

static HWND g_hMainWindow = NULL;

typedef struct tagMAINWINDOW
{
    HWND                hWnd;
    HWND                hWndStatus;
    HWND                hWndPreview;
    HBITMAP             hbmWallpaper;
    PKM_PALETTE         pkmPalette;
    COLORREF            crHilight;
    COLORREF            crHotTrackingColor;
} MAINWINDOW, *PMAINWINDOW;

#if 0
static COLORREF ColorSelectionDialogShow(HWND hWnd)
{
    CHOOSECOLOR cc;
    COLORREF acrCustom[16];

    ZeroMemory(&cc, sizeof cc);

    cc.lStructSize = sizeof cc;
    cc.hwndOwner = hWnd;
    cc.lpCustColors = (LPDWORD)acrCustom;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    return ChooseColor(&cc) ? cc.rgbResult : RGB(0, 0, 0);
}
#endif

static BOOL CALLBACK SetFontCallback(HWND child, LPARAM font)
{
    SendMessage(child, WM_SETFONT, font, TRUE);
    return TRUE;
}

static VOID MainWindow_Update(PMAINWINDOW pMainWnd, BOOL bEnableApply)
{
    SendMessage(pMainWnd->hWndPreview, XXM_PREVIEW_UPDATE_HILIGHT, 0,
                (LPARAM)pMainWnd->crHilight);

    SendMessage(pMainWnd->hWndPreview, XXM_PREVIEW_UPDATE_HTC, 0,
                (LPARAM)pMainWnd->crHotTrackingColor);

    EnableWindow(GetDlgItem(pMainWnd->hWnd, IDC_BUTTON_APPLY), bEnableApply);
}

static BOOL ResetButton_OnClick(PMAINWINDOW pMainWnd)
{
    if (ColorsRegistryResetToDefaults() != ERROR_SUCCESS)
        return FALSE;

    if (ColorsRegistryUpdateSystem() != TRUE)
        return FALSE;

    pMainWnd->crHilight = ColorsRegistryGetHilight();
    pMainWnd->crHotTrackingColor = ColorsRegistryGetHTC();

    MainWindow_Update(pMainWnd, FALSE);
    return TRUE;
}

static BOOL ApplyButton_OnClick(PMAINWINDOW pMainWnd)
{
    if (ColorsRegistrySetHTC(pMainWnd->crHotTrackingColor) != ERROR_SUCCESS)
        return FALSE;

    if (ColorsRegistrySetHilight(pMainWnd->crHilight) != ERROR_SUCCESS)
        return FALSE;

    ColorsRegistryUpdateSystem();
    MainWindow_Update(pMainWnd, FALSE);
    return TRUE;
}

static VOID MainWindow_ShowErrorDialog(HWND hWnd)
{
    LPTSTR lpszErrorMessage = NULL;

    static const DWORD dwFmtFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                    FORMAT_MESSAGE_FROM_SYSTEM |
                                    FORMAT_MESSAGE_IGNORE_INSERTS;

    FormatMessage(dwFmtFlags, NULL, GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&lpszErrorMessage, 0, NULL);

    if (lpszErrorMessage != NULL) {
        MessageBox(hWnd, lpszErrorMessage, g_pszWindowName,
                   MB_OK | MB_ICONERROR);

        HeapFree(GetProcessHeap(), 0, lpszErrorMessage);
    }
}

static BOOL MainWindow_OnCommand(PMAINWINDOW pMainWnd, DWORD dwId)
{
    switch (dwId) {
        case IDC_BUTTON_RESET:
            return ResetButton_OnClick(pMainWnd);
        case IDC_BUTTON_APPLY:
            return ApplyButton_OnClick(pMainWnd);
        case IDC_BUTTON_CANCEL:
            DestroyWindow(pMainWnd->hWnd);
            return TRUE;
    }
    
    return TRUE;
}

static BOOL MainWindow_OnCreate(PMAINWINDOW pMainWnd, HWND hWnd)
{
    InitCommonControls();

    if (Controls_RegisterAllClasses() == FALSE)
        return FALSE;

    pMainWnd->hWnd = hWnd;
    pMainWnd->hbmWallpaper = DesktopWallpaperGetHBITMAP();
    pMainWnd->crHilight = ColorsRegistryGetHilight();
    pMainWnd->crHotTrackingColor = ColorsRegistryGetHTC();

    if (pMainWnd->hbmWallpaper != NULL) {
        pMainWnd->pkmPalette = KM_GeneratePaletteFromHBITMAP(DW_PALETTE_K,
            pMainWnd->hbmWallpaper);

        KM_SortPaletteByBrightness(pMainWnd->pkmPalette);
    }

    pMainWnd->hWndPreview = Preview_Create(hWnd, 0, 25, 25, 384, 250);

    Palette_Create(hWnd, IDC_PALETTE, 25, 280, 384, 25, pMainWnd->pkmPalette);

    if (pMainWnd->hbmWallpaper == NULL)
        Palette_SetError(GetDlgItem(hWnd, IDC_PALETTE), IDS_NO_WALLPAPER);

    Static_Create(hWnd, 25, 318, 385, 25, IDS_HILIGHT);
    Button_Create(hWnd, 0, 25, 338, 80, 25, IDS_CHANGE);

    Static_Create(hWnd, 25, 380, 385, 25, IDS_HTC);

    Button_Create(hWnd, IDC_BUTTON_RESET, 25, 500, 120, 25, IDS_RESET);
    Button_Create(hWnd, IDC_BUTTON_ABOUT, 224, 500, 25, 25, IDS_ABOUT);
    Button_Create(hWnd, IDC_BUTTON_CANCEL, 249, 500, 80, 25, IDS_CANCEL);
    Button_Create(hWnd, IDC_BUTTON_APPLY, 329, 500, 80, 25, IDS_APPLY);

    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON_APPLY), FALSE);

    SendMessage(pMainWnd->hWndPreview, XXM_PREVIEW_UPDATE_BG, 0,
                (LPARAM)pMainWnd->hbmWallpaper);

    EnumChildWindows(hWnd, (WNDENUMPROC)SetFontCallback,
                     (LPARAM)GetStockObject(DEFAULT_GUI_FONT));

    MainWindow_Update(pMainWnd, FALSE);
    return TRUE;
}

static VOID MainWindow_OnDestroy(PMAINWINDOW pMainWnd, HWND hWnd)
{
    KM_FreePalette(pMainWnd->pkmPalette);

    Controls_UnregisterAllClasses();
}

static LRESULT CALLBACK MainWindow_Proc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                        LPARAM lParam)
{
    PMAINWINDOW pMainWnd = (PMAINWINDOW)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    
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
            pMainWnd->crHotTrackingColor = pMainWnd->crHilight;
            MainWindow_Update(pMainWnd, TRUE);
            break;

        case WM_COMMAND:
            if (MainWindow_OnCommand(pMainWnd, LOWORD(wParam)) == FALSE)
                MainWindow_ShowErrorDialog(hWnd);
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
    HWND hWnd = NULL;

    ZeroMemory(&wndClass, sizeof wndClass);

    wndClass.lpfnWndProc = MainWindow_Proc;
    wndClass.hInstance = hInstance;
    wndClass.lpszClassName = g_pszWindowClassName;
    wndClass.cbWndExtra = sizeof(MAINWINDOW);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;

    if (RegisterClass(&wndClass) == 0)
        return NULL;

    return CreateWindowEx(0, g_pszWindowClassName, g_pszWindowName,
                          WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL,
                          hInstance, NULL);
}

INT WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     PTSTR pCmdLine, INT nCmdShow)
{
    MSG msg;

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