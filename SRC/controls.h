#ifndef _CONTROLS_H_
#define _CONTROLS_H_

#include <Windows.h>

#include "clrkmeans.h"

#define PREVIEWCLASSNAME				TEXT("PreviewHiCC")
#define PALETTECLASSNAME				TEXT("PaletteHiCC")
#define COLORBOXCLASSNAME				TEXT("ColorBoxHiCC")

#define XXM_PREVIEW_UPDATE_BG			(WM_USER + 1)
#define XXM_PREVIEW_UPDATE_HILIGHT		(WM_USER + 2)
#define XXM_PREVIEW_UPDATE_HTC			(WM_USER + 3)

#define XXM_PALETTE_UPDATE_PALETTE		(WM_USER + 4)
#define XXM_PALETTE_CLRSEL				(WM_USER + 5)
#define XXM_PALETTE_ERROR_MSG			(WM_USER + 6)

#define XXM_COLORBOX_SET_COLOR			(WM_USER + 7)

HWND Preview_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
					INT nHeight);

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
