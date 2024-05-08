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

#include "wallpaper.h"

#include <Shlwapi.h>
#include <ShlObj.h>

#include "bmutils.h"

static LPTSTR DesktopWallpaperGetPathSPI(VOID)
{
    DWORD cbPathSize = MAX_PATH * sizeof(TCHAR);
    LPTSTR pszPathBuffer = NULL;
    BOOL bResult;

    SystemParametersInfo(SPI_GETDESKWALLPAPER, 0, NULL, SPIF_UPDATEINIFILE);

    pszPathBuffer = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                      cbPathSize);

    if (pszPathBuffer != NULL) {
        bResult = SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH - 1,
                                       (PVOID)pszPathBuffer, 0);

        if (bResult == FALSE) {
            HeapFree(GetProcessHeap(), 0, (LPVOID)pszPathBuffer);
            return NULL;
        }

        if (PathFileExists(pszPathBuffer) == FALSE) {
            HeapFree(GetProcessHeap(), 0, (LPVOID)pszPathBuffer);
            return NULL;
        }
    }

    return pszPathBuffer;
}

static LPTSTR DesktopWallpaperGetPathCached(VOID)
{
    DWORD cbPathSize = MAX_PATH * sizeof(TCHAR);
    LPTSTR pszPathBuffer = NULL;
    HRESULT hRes;

    pszPathBuffer = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                      cbPathSize);

    if (pszPathBuffer == NULL)
        return NULL;

    hRes = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, pszPathBuffer);

    if (FAILED(hRes)) {
        HeapFree(GetProcessHeap(), 0, (LPVOID)pszPathBuffer);
        return NULL;
    }

    /* %AppData%\Microsoft\Windows\Themes\TranscodedWallpaper */
    PathAppend(pszPathBuffer, TEXT("\\Microsoft\\Windows\\Themes"));
    PathAppend(pszPathBuffer, TEXT("\\TranscodedWallpaper"));

    if (PathFileExists(pszPathBuffer) == FALSE) {
        HeapFree(GetProcessHeap(), 0, (LPVOID)pszPathBuffer);
        return NULL;
    }

    return pszPathBuffer;
}

static BOOL CALLBACK FindWPELiveWp_Callback(HWND hWnd, LPARAM lParam)
{
    HWND *phWPELiveWallpaper = (HWND*)lParam;
    HWND hFoundWnd = NULL;

    hFoundWnd = FindWindowEx(hWnd, NULL, NULL, TEXT("WPELiveWallpaper"));

    if (hFoundWnd != NULL) {
        *phWPELiveWallpaper = hFoundWnd;
        return FALSE;
    }
    
    return TRUE;
}

static HWND FindWPELiveWallpaperWindow(VOID)
{
    HWND hWPELiveWallpaper = NULL;
    EnumWindows(FindWPELiveWp_Callback, (LPARAM)&hWPELiveWallpaper);
    return hWPELiveWallpaper;
}

HBITMAP DesktopWallpaperGetHBITMAP(VOID)
{
    HBITMAP hbmWallpaper = NULL;
    LPTSTR pszWallpaperPath = NULL;
    HWND hWallpaperEngineWnd = NULL;

    hWallpaperEngineWnd = FindWPELiveWallpaperWindow();

    if (hWallpaperEngineWnd != NULL)
        return HBITMAP_FromWindow(hWallpaperEngineWnd);
    
    pszWallpaperPath = DesktopWallpaperGetPathSPI();

    if (pszWallpaperPath == NULL)
        pszWallpaperPath = DesktopWallpaperGetPathCached();
    
    if (pszWallpaperPath != NULL) {
        hbmWallpaper = HBITMAP_FromFile(pszWallpaperPath);
        HeapFree(GetProcessHeap(), 0, pszWallpaperPath);
    }

    return hbmWallpaper;
}