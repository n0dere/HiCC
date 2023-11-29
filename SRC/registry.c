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

#include "registry.h"

#include <tchar.h>

#include "syscolors.h"

#define STRING_RGB_SZ               12

#define RGB_CMP_VALID(clr_cmp)  ((clr_cmp) >= 0 && (clr_cmp) < 256)

#define RGB_IS_VALID(r, g, b)                                       \
    (RGB_CMP_VALID(r) && RGB_CMP_VALID(g) && RGB_CMP_VALID(b))

static const TCHAR* g_pszColorsRegistrySubKey = TEXT("Control Panel\\Colors");
static const TCHAR* g_pszHiCCRegistrySubKey = TEXT("SOFTWARE\\HiCC");

static LPTSTR ConvertColorToStringRGB(COLORREF crColor)
{
    DWORD cbBufSize = STRING_RGB_SZ * sizeof(TCHAR);
    LPTSTR pszBuffer = HeapAlloc(GetProcessHeap(), 0, cbBufSize);

    if (pszBuffer != NULL) {
        _sntprintf(pszBuffer, STRING_RGB_SZ, TEXT("%u %u %u"),
                   GetRValue(crColor), GetGValue(crColor),
                   GetBValue(crColor));
    }

    return pszBuffer;
}

static COLORREF ConvertStringRGBToColor(LPCTSTR pszStringRGB)
{
    UINT uRed = 0, uGreen = 0, uBlue = 0;

    if (pszStringRGB == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    _sntscanf(pszStringRGB, STRING_RGB_SZ, TEXT("%u %u %u"),
              &uRed, &uGreen, &uBlue);

    if (!RGB_IS_VALID(uRed, uGreen, uBlue)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    return RGB(uRed, uGreen, uBlue);
}

static DWORD RegistryGetDWORDValue(HKEY hKey, LPCTSTR pszSubKey,
                                   LPCTSTR pszValueName)
{
    DWORD cbDataSize = sizeof(DWORD);
    LONG lResult = ERROR_SUCCESS;
    DWORD dwValue = 0;

    if (hKey == NULL || pszSubKey == NULL || pszValueName == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    lResult = RegGetValue(hKey, pszSubKey, pszValueName, RRF_RT_REG_DWORD,
                          NULL, &dwValue, &cbDataSize);
    
    if (lResult != ERROR_SUCCESS)
        return 0;
    
    return dwValue;
}

static LONG RegistrySetDWORDValue(HKEY hKey, LPCTSTR pszSubKey,
                                  LPCTSTR pszValueName, DWORD dwValue)
{
    DWORD cbDataSize = sizeof(DWORD);

    if (hKey == NULL || pszSubKey == NULL || pszValueName == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    return RegSetKeyValue(hKey, pszSubKey, pszValueName, REG_DWORD,
                          &dwValue, cbDataSize);
}

static LPTSTR RegistryGetStringValue(HKEY hKey, LPCTSTR pszSubKey,
                                     LPCTSTR pszValueName)
{
    LPTSTR pszData = NULL;
    LONG lResult = ERROR_SUCCESS;
    DWORD cbDataSize = 0;

    if (hKey == NULL || pszSubKey == NULL || pszValueName == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    lResult = RegGetValue(hKey, pszSubKey, pszValueName, RRF_RT_REG_SZ,
                          NULL, NULL, &cbDataSize);

    if (lResult != ERROR_SUCCESS)
        return NULL;

    pszData = HeapAlloc(GetProcessHeap(), 0, cbDataSize);

    if (pszData == NULL)
        return NULL;

    lResult = RegGetValue(hKey, pszSubKey, pszValueName, RRF_RT_REG_SZ,
                          NULL, (PVOID)pszData, &cbDataSize);

    if (lResult != ERROR_SUCCESS) {
        HeapFree(GetProcessHeap(), 0, pszData);
        return NULL;
    }

    return pszData;
}

static LONG RegistrySetStringValue(HKEY hKey, LPCTSTR pszSubKey,
                                   LPCTSTR pszValueName, LPCTSTR pszValue)
{
    DWORD cbValueSize = 0;

    if (hKey == NULL || pszSubKey == NULL || pszValueName == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return ERROR_INVALID_PARAMETER;
    }

    if (pszValue == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return ERROR_INVALID_PARAMETER;
    }

    cbValueSize = (DWORD)(sizeof(TCHAR) * (_tcslen(pszValue) + 1));

    return RegSetKeyValue(hKey, pszSubKey, pszValueName, REG_SZ,
                          (LPCVOID)pszValue, cbValueSize);
}

LONG HiCCRegistrySetBOOL(LPCTSTR pszValueName, BOOL bValue)
{
    return RegistrySetDWORDValue(HKEY_CURRENT_USER, g_pszHiCCRegistrySubKey,
                                 pszValueName, (DWORD)bValue);
}

BOOL HiCCRegistryGetBOOL(LPCTSTR pszValueName)
{
    return RegistryGetDWORDValue(HKEY_CURRENT_USER, g_pszHiCCRegistrySubKey,
                                 pszValueName);
}

LONG HiCCRegistrySaveColors(COLORREF aColors[16])
{
    return RegSetKeyValue(HKEY_CURRENT_USER, g_pszHiCCRegistrySubKey,
                          TEXT("Colors"), REG_BINARY, (BYTE*)aColors,
                          16 * sizeof(COLORREF));
}

LONG HiCCRegistryGetColors(COLORREF aOutColors[16])
{
    DWORD cbColorsSize = 16 * sizeof(COLORREF);

    return RegGetValue(HKEY_CURRENT_USER, g_pszHiCCRegistrySubKey,
                       TEXT("Colors"), RRF_RT_REG_BINARY, NULL,
                       (PVOID)aOutColors, &cbColorsSize);
}

LONG ColorsRegistrySet(LPCTSTR pszValueName, COLORREF crValue)
{
    LPTSTR pszColorValue = NULL;
    LONG lResult = ERROR_SUCCESS;

    pszColorValue = ConvertColorToStringRGB(crValue);

    if (pszColorValue == NULL) {
        HeapFree(GetProcessHeap(), 0, pszColorValue);
        return GetLastError();
    }

    lResult = RegistrySetStringValue(HKEY_CURRENT_USER,
                                     g_pszColorsRegistrySubKey,
                                     pszValueName, pszColorValue);

    HeapFree(GetProcessHeap(), 0, pszColorValue);

    return lResult;
}

COLORREF ColorsRegistryGet(LPCTSTR pszValueName)
{
    LPTSTR pszColorValue = NULL;
    COLORREF crResult = 0;

    if (pszValueName != NULL) {
        pszColorValue = RegistryGetStringValue(HKEY_CURRENT_USER,
                                               g_pszColorsRegistrySubKey,
                                               pszValueName);

        if (pszColorValue != NULL) {
            crResult = ConvertStringRGBToColor(pszColorValue);
            HeapFree(GetProcessHeap(), 0, pszColorValue);
        }
    }

    return crResult;
}

LONG ColorsRegistryResetToDefaultAll(VOID)
{
    LONG lResult = ERROR_SUCCESS;
    SIZE_T i;

    for (i = 0; i < SYSTEM_COLORS_COUNT; i++) {
        lResult = ColorsRegistrySet(g_v_SystemColors[i].pszValueName,
                                    g_v_SystemColors[i].crDefault);
        
        if (lResult != ERROR_SUCCESS)
            break;
    }

    return lResult;
}

LONG ColorsRegistryResetToDefault(INT nColorId)
{
    SIZE_T i;

    for (i = 0; i < SYSTEM_COLORS_COUNT; i++) {
        if (g_v_SystemColors[i].nId == nColorId) {
            return ColorsRegistrySet(g_v_SystemColors[i].pszValueName,
                                     g_v_SystemColors[i].crDefault);
        }
    }

    SetLastError(ERROR_NOT_FOUND);

    return ERROR_NOT_FOUND;
}