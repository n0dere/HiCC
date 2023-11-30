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
#include <windowsx.h>
#include <CommCtrl.h>

#include "resource.h"
#include "controls.h"
#include "registry.h"

#define GNU_GPL_V3_BUFFER_SZ            512
#define APPNAME_BUFFER_SZ               256
#define OK_BUFFER_SZ                    64
#define OLD_BEH_BUFFER_SZ               128

static VOID AboutBox_OnNotify(HWND hDlg, LPNMHDR lpNMHDR)
{
    TCHAR szUrl[MAX_PATH];

    if (lpNMHDR->idFrom == IDC_DLG_URL_GITHUB) {
        LoadStringUTF8(IDS_GITHUB_URL, szUrl, MAX_PATH);

        if (lpNMHDR->code == NM_CLICK)
            ShellExecute(hDlg, 0, szUrl, 0, 0, SW_SHOW);
    }
}

static BOOL AboutBox_OnInit(HWND hDlg)
{
    TCHAR szLicense[GNU_GPL_V3_BUFFER_SZ];
    TCHAR szName[APPNAME_BUFFER_SZ];
    TCHAR szOk[OK_BUFFER_SZ];
    TCHAR szOldBeh[OLD_BEH_BUFFER_SZ];
    RECT rcParent, rcDlg;
    INT x, y;

    LoadStringUTF8(IDS_GNU_GPL_V3, szLicense, GNU_GPL_V3_BUFFER_SZ);
    LoadStringUTF8(IDS_WINDOW_TITLE, szName, APPNAME_BUFFER_SZ);
    LoadStringUTF8(IDS_OK, szOk, OK_BUFFER_SZ);
    LoadStringUTF8(IDS_OLD_BEH, szOldBeh, OLD_BEH_BUFFER_SZ);

    Button_SetText(GetDlgItem(hDlg, IDC_DLG_OK), szOk);
    Edit_SetText(GetDlgItem(hDlg, IDC_DLG_EDIT_LICENSE), szLicense);
    Static_SetText(GetDlgItem(hDlg, IDC_DLG_APPNAME), szName);
    SetWindowText(GetDlgItem(hDlg, IDC_DLG_CHCKBOX_OLD_BEH), szOldBeh);

    if (HiCCRegistryGetResetAll() == TRUE)
        CheckDlgButton(hDlg, IDC_DLG_CHCKBOX_OLD_BEH, BST_CHECKED);

    GetWindowRect(GetParent(hDlg), &rcParent);
    GetWindowRect(hDlg, &rcDlg);

    x = rcParent.left + ((rcParent.right - rcDlg.right) / 2);
    y = rcParent.top + ((rcParent.bottom - rcDlg.bottom) / 2);

    SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOSIZE);

    return TRUE;
}

static BOOL OldBehaviour_OnCheck(HWND hDlg, BOOL bNewState)
{
    return HiCCRegistrySetResetAll(bNewState) == ERROR_SUCCESS;
}

static BOOL AboutBox_OnCommand(HWND hDlg, WORD wId)
{
    BOOL bIsChecked = FALSE;
    UINT uCheck = BST_UNCHECKED;
    
    switch (wId) {
        case IDC_DLG_OK:
        case IDCANCEL:
        case IDOK:
            return (INT_PTR)EndDialog(hDlg, wId);
        
        case IDC_DLG_CHCKBOX_OLD_BEH:
            bIsChecked = IsDlgButtonChecked(hDlg, wId);
            uCheck = (!bIsChecked) ? BST_CHECKED : BST_UNCHECKED;
            CheckDlgButton(hDlg, wId, uCheck);
            return (INT_PTR)OldBehaviour_OnCheck(hDlg, !bIsChecked);
    }

    return FALSE;
}

INT_PTR CALLBACK AboutBox_Proc(HWND hDlg, UINT uMsg, WPARAM wParam,
                               LPARAM lParam)
{
    switch (uMsg) {
        case WM_INITDIALOG:
            return (INT_PTR)AboutBox_OnInit(hDlg);

        case WM_COMMAND:
            return (INT_PTR)AboutBox_OnCommand(hDlg, LOWORD(wParam));

        case WM_NOTIFY:
            AboutBox_OnNotify(hDlg, (LPNMHDR)lParam);
            break;
    }

    return (INT_PTR)FALSE;
}

VOID AboutBox_Show(HINSTANCE hInstance, HWND hWnd)
{
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBox_Proc);
}