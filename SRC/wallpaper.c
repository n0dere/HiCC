#include "wallpaper.h"

#include <tchar.h>
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

    pszPathBuffer = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
        cbPathSize);

    if (pszPathBuffer == NULL)
        return NULL;

    if (FAILED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, pszPathBuffer))) {
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

HBITMAP DesktopWallpaperGetHBITMAP(VOID)
{
    HBITMAP hbmWallpaper = NULL;
    LPTSTR pszWallpaperPath = DesktopWallpaperGetPathSPI();

    if (pszWallpaperPath == NULL)
        pszWallpaperPath = DesktopWallpaperGetPathCached();

    if (pszWallpaperPath == NULL)
        return NULL;

    hbmWallpaper = HBITMAP_FromFile(pszWallpaperPath);

    HeapFree(GetProcessHeap(), 0, pszWallpaperPath);

    return hbmWallpaper;
}