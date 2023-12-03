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

#ifndef _CONTROLS_H_
#define _CONTROLS_H_

#include <Windows.h>
#include <windowsx.h>
#include <CommCtrl.h>

#include "clrkmeans.h"

#define PREVIEWCLASSNAME                TEXT("PreviewHiCC")
#define PALETTECLASSNAME                TEXT("PaletteHiCC")
#define COLORBOXCLASSNAME               TEXT("ColorBoxHiCC")

#define XXM_PREVIEW_UPDATE_BG           (WM_USER + 1)
#define XXM_PREVIEW_UPDATE_HILIGHT      (WM_USER + 2)
#define XXM_PREVIEW_UPDATE_HTC          (WM_USER + 3)

#define XXM_PALETTE_UPDATE_PALETTE      (WM_USER + 4)
#define XXM_PALETTE_CLRSEL              (WM_USER + 5)
#define XXM_PALETTE_ERROR_MSG           (WM_USER + 6)

#define XXM_COLORBOX_SET_COLOR          (WM_USER + 7)

BOOL LoadStringUTF8(UINT uId, LPTSTR pszBuffer, DWORD dwMaxSize);

HWND Preview_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
                    INT nHeight);

VOID Preview_UpdateColors(HWND hPreview, COLORREF crHi, COLORREF crHTC);

VOID Preview_UpdateBG(HWND hPreview, HBITMAP hbmBG);

HWND Palette_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
                    INT nHeight, PKM_PALETTE pKmPalette);

VOID Palette_SetError(HWND hPalette, UINT uMsgId);

HWND ColorBox_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
                     INT nHeight);

VOID ColorBox_ChangeColor(HWND hColorBox, COLORREF crColor);

HWND Button_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
                   INT nHeight, UINT uNameId);



HWND Static_Create(HWND hParent, INT x, INT y, INT nWidth, INT nHeight,
                   UINT uTextId);

HWND GroupBox_Create(HWND hParent, INT x, INT y, INT nWidth, INT nHeight,
                     UINT uTextId);

HWND Edit_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
                 INT nHeight);

BOOL Controls_RegisterAllClasses(VOID);

VOID Controls_UnregisterAllClasses(VOID);

#endif /* _CONTROLS_H_ */
