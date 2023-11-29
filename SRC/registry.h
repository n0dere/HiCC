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

#ifndef _CLRREGISTRY_H_
#define _CLRREGISTRY_H_

#include <windows.h>

#define ColorsRegistrySetHilight(clr)							\
    ColorsRegistrySet(TEXT("Hilight"), (clr))

#define ColorsRegistrySetHTC(clr)								\
    ColorsRegistrySet(TEXT("HotTrackingColor"), (clr))

#define ColorsRegistryGetHilight()								\
    ColorsRegistryGet(TEXT("Hilight"))

#define ColorsRegistryGetHTC()									\
    ColorsRegistryGet(TEXT("HotTrackingColor"))

#define HiCCRegistrySetResetAll(val)                            \
    HiCCRegistrySetBOOL(TEXT("ResetAll"), (val))

#define HiCCRegistryGetResetAll()                               \
    HiCCRegistryGetBOOL(TEXT("ResetAll"))

LONG HiCCRegistrySetBOOL(LPCTSTR pszValueName, BOOL bValue);

BOOL HiCCRegistryGetBOOL(LPCTSTR pszValueName);

LONG ColorsRegistrySet(LPCTSTR pszValueName, COLORREF crValue);

COLORREF ColorsRegistryGet(LPCTSTR pszValueName);

LONG ColorsRegistryResetToDefaultAll(VOID);

LONG ColorsRegistryResetToDefault(INT nColorId);

#endif /* _CLRREGISTRY_H_ */