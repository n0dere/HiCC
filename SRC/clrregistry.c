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

#include "clrregistry.h"

#include <tchar.h>

#include "syscolors.h"

#define STRING_RGB_SZ               12

#define RGB_CMP_VALID(clr_cmp)  ((clr_cmp) >= 0 && (clr_cmp) < 256)

#define RGB_IS_VALID(r, g, b)                                       \
    (RGB_CMP_VALID(r) && RGB_CMP_VALID(g) && RGB_CMP_VALID(b))

static const TCHAR* g_pszColorsRegistrySubKey = TEXT("Control Panel\\Colors");

LPTSTR ConvertColorToStringRGB(COLORREF crColor)
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

COLORREF ConvertStringRGBToColor(LPCTSTR pszStringRGB)
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

LONG ColorsRegistryResetToDefaults(VOID)
{
    LPCTSTR pszName = NULL;
    COLORREF crDefault = 0;
    LONG lResult = ERROR_SUCCESS;
    SIZE_T i;

    for (i = 0; i < SYSTEM_COLORS_COUNT; i++) {
        pszName = g_v_SystemColors[i].pszValueName;
        crDefault = g_v_SystemColors[i].crDefault;

        lResult = ColorsRegistrySet(pszName, crDefault);
        
        if (lResult != ERROR_SUCCESS)
            break;
    }

    return lResult;
}

BOOL ColorsRegistryUpdateSystem(VOID)
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