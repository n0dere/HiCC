#ifndef _CLRKMEANS_H_
#define _CLRKMEANS_H_

#include <windows.h>

typedef struct _KM_PALETTE
{
    LPCOLORREF      aColors;
    DWORD           dwColorsCount;
} *PKM_PALETTE;

PKM_PALETTE KM_GeneratePalette(DWORD k, LPRGBQUAD aColors, DWORD cColorsCount);

PKM_PALETTE KM_GeneratePaletteFromHBITMAP(DWORD k, HBITMAP hBitmap);

VOID KM_SortPaletteByBrightness(PKM_PALETTE kmPalette);

VOID KM_FreePalette(PKM_PALETTE kmPalette);

#endif /* _CLRKMEANS_H_ */