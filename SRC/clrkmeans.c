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

#include "clrkmeans.h"

#include <float.h>
#include <math.h>

#include "bmutils.h"

#define KM_EPS                      0.01
#define KM_MAX_ITER                 100
#define KM_RESIZE_WIDTH             150
#define KM_RESIZE_HEIGHT            150

#define RGB_CHANNEL_LIN(ch)                                 \
    (((ch) <= 0.04045) ? ((ch) / 12.92) : pow((((ch) + 0.055) / 1.055), 2.4))

typedef struct _KM_CENTROID
{
    DOUBLE      dbRed;
    DOUBLE      dbGreen;
    DOUBLE      dbBlue;
} *PKM_CENTROID;

static DOUBLE KM_Distance(LPRGBQUAD lpColor, PKM_CENTROID pCentroid)
{
    DOUBLE dbRed = lpColor->rgbRed - pCentroid->dbRed;
    DOUBLE dbGreen = lpColor->rgbGreen - pCentroid->dbGreen;
    DOUBLE dbBlue = lpColor->rgbBlue - pCentroid->dbBlue;

    return dbRed * dbRed + dbGreen * dbGreen + dbBlue * dbBlue;
}

static INT KM_NearestCentroid(DWORD k, LPRGBQUAD lpColor,
                              PKM_CENTROID aCentroids)
{
    INT iMinIndex = 0;
    DOUBLE dbMinDist = DBL_MAX;
    DOUBLE dbDist = 0.0;
    UINT i;

    for (i = 1; i < k; i++) {
        dbDist = KM_Distance(lpColor, &aCentroids[i]);

        if (dbDist < dbMinDist) {
            dbMinDist = dbDist;
            iMinIndex = i;
        }
    }

    return iMinIndex;
}

static VOID KM_UpdateCentroid(PKM_CENTROID pCentroid, LPRGBQUAD aColors,
                              PINT aLabels, UINT n, INT iIndex)
{
    DOUBLE dbSumRed = 0.0;
    DOUBLE dbSumGreen = 0.0;
    DOUBLE dbSumBlue = 0.0;
    UINT uCount = 0;
    UINT i = 0;

    for (i = 0; i < n; i++) {
        if (aLabels[i] == iIndex) {
            dbSumRed += aColors[i].rgbRed;
            dbSumGreen += aColors[i].rgbGreen;
            dbSumBlue += aColors[i].rgbBlue;
            uCount++;
        }
    }

    if (uCount > 0) {
        pCentroid->dbRed = dbSumRed / uCount;
        pCentroid->dbGreen = dbSumGreen / uCount;
        pCentroid->dbBlue = dbSumBlue / uCount;
    }
}

static BOOL KM_IsConverged(DWORD k, PKM_CENTROID aOldCentroids,
                           PKM_CENTROID aNewCentroids)
{
    RGBQUAD tmpColor;
    DOUBLE dbDiff = 0.0;
    UINT i;

    for (i = 0; i < k; i++) {
        tmpColor.rgbRed = (BYTE)aOldCentroids[i].dbRed;
        tmpColor.rgbGreen = (BYTE)aOldCentroids[i].dbGreen;
        tmpColor.rgbBlue = (BYTE)aOldCentroids[i].dbBlue;

        dbDiff = KM_Distance(&tmpColor, &aNewCentroids[i]);
    }

    return dbDiff < KM_EPS;
}

static VOID KM_InitCentroidsPP(DWORD k, PKM_CENTROID aCentroids,
                               LPRGBQUAD aColors, DWORD cColorsCount)
{
    DOUBLE dbSum, dbR, dbMinDist, dbDist;
    INT i, j, n;

    i = rand() % cColorsCount;

    aCentroids[0].dbRed = aColors[i].rgbRed;
    aCentroids[0].dbGreen = aColors[i].rgbGreen;
    aCentroids[0].dbBlue = aColors[i].rgbBlue;

    for (i = 1; i < (INT)k; i++) {
        for (j = 0, dbSum = 0.0; j < (INT)cColorsCount; j++) {
            dbMinDist = DBL_MAX;

            for (n = 0; n < i; n++) {
                dbDist = KM_Distance(&aColors[j], &aCentroids[n]);
                dbMinDist = min(dbDist, dbMinDist);
            }
            
            dbSum += dbMinDist;
            
            aColors[j].rgbReserved = (BYTE)dbMinDist;
        }

        dbR = ((DOUBLE)rand() / RAND_MAX) * dbSum;

        for (j = 0, dbSum = 0.0; j < (INT)cColorsCount; j++) {
            dbSum += pow(aColors[j].rgbReserved, 2);

            if (dbSum >= dbR)
                break;
        }

        aCentroids[i].dbRed = aColors[j].rgbRed;
        aCentroids[i].dbGreen = aColors[j].rgbGreen;
        aCentroids[i].dbBlue = aColors[j].rgbBlue;
    }
}

static PKM_PALETTE KM_NewPalette(DWORD k, PKM_CENTROID aCentroids)
{
    PKM_PALETTE kmPalette = NULL;
    UINT uRed = 0, uGreen = 0, uBlue = 0;
    UINT i;

    kmPalette = (PKM_PALETTE)HeapAlloc(GetProcessHeap(), 0,
                                       (sizeof * kmPalette));

    if (kmPalette == NULL)
        return NULL;

    kmPalette->aColors = (LPCOLORREF)HeapAlloc(GetProcessHeap(), 0,
                                               k * sizeof(COLORREF));

    if (kmPalette->aColors == NULL) {
        HeapFree(GetProcessHeap(), 0, kmPalette->aColors);
        return NULL;
    }

    for (i = 0; i < k; i++) {
        uRed = (UINT)round(aCentroids[i].dbRed);
        uGreen = (UINT)round(aCentroids[i].dbGreen);
        uBlue = (UINT)round(aCentroids[i].dbBlue);
        kmPalette->aColors[i] = RGB(uRed, uGreen, uBlue);
    }

    kmPalette->dwColorsCount = k;

    return kmPalette;
}

/* https://en.wikipedia.org/wiki/K-means_clustering */
PKM_PALETTE KM_GeneratePalette(DWORD k, LPRGBQUAD aColors, DWORD cColorsCount)
{
    PKM_CENTROID aCentroids = NULL;
    PKM_CENTROID aOldCentroids = NULL;
    PINT aLabels = NULL;
    PKM_PALETTE kmPalette = NULL;
    BOOL bConverged = FALSE;
    INT iIter = 0;
    UINT i;

    aCentroids = (PKM_CENTROID)HeapAlloc(GetProcessHeap(), 0,
                                         k * (sizeof * aCentroids));

    if (aCentroids == NULL)
        return NULL;

    KM_InitCentroidsPP(k, aCentroids, aColors, cColorsCount);

    aLabels = (PINT)HeapAlloc(GetProcessHeap(), 0,
                              cColorsCount * (sizeof * aLabels));

    if (aLabels == NULL) {
        HeapFree(GetProcessHeap(), 0, aCentroids);
        return NULL;
    }

    aOldCentroids = (PKM_CENTROID)HeapAlloc(GetProcessHeap(), 0,
                                            k * (sizeof * aCentroids));

    if (aOldCentroids == NULL) {
        HeapFree(GetProcessHeap(), 0, aLabels);
        HeapFree(GetProcessHeap(), 0, aCentroids);
        return NULL;
    }

    while (!bConverged && iIter < KM_MAX_ITER) {
        for (i = 0; i < k; i++) {
            CopyMemory(&aOldCentroids[i], &aCentroids[i],
                       sizeof * aOldCentroids);
        }

        for (i = 0; i < cColorsCount; i++) {
            aLabels[i] = KM_NearestCentroid(k, &aColors[i], aCentroids);
        }

        for (i = 0; i < k; i++) {
            KM_UpdateCentroid(&aCentroids[i], aColors, aLabels,
                              cColorsCount, i);
        }

        bConverged = KM_IsConverged(k, aOldCentroids, aCentroids);

        iIter++;
    }

    kmPalette = KM_NewPalette(k, aCentroids);

    HeapFree(GetProcessHeap(), 0, aOldCentroids);
    HeapFree(GetProcessHeap(), 0, aLabels);
    HeapFree(GetProcessHeap(), 0, aCentroids);

    return kmPalette;
}

PKM_PALETTE KM_GeneratePaletteFromHBITMAP(DWORD k, HBITMAP hBitmap)
{
    DWORD cPixelsCount = 0;
    LPRGBQUAD aPixelsRGB = NULL;
    PKM_PALETTE kmPalette = NULL;
    HBITMAP hbmTmp;

    if (hBitmap == NULL)
        return NULL;

    hbmTmp = HBITMAP_Resize(hBitmap, KM_RESIZE_WIDTH, KM_RESIZE_HEIGHT);

    if (hbmTmp == NULL)
        return NULL;

    aPixelsRGB = HBITMAP_GetPixels(hbmTmp, &cPixelsCount);

    if (aPixelsRGB == NULL) {
        DeleteObject(hbmTmp);
        return NULL;
    }

    kmPalette = KM_GeneratePalette(k, aPixelsRGB, cPixelsCount);

    HeapFree(GetProcessHeap(), 0, (LPVOID)aPixelsRGB);

    DeleteObject(hbmTmp);

    return kmPalette;
}

/* https://stackoverflow.com/q/596216 */
static inline DOUBLE COLORREF_GetBrightness(COLORREF color)
{
    DOUBLE dbRedLin = RGB_CHANNEL_LIN(GetRValue(color) / 255.0);
    DOUBLE dbGreenLin = RGB_CHANNEL_LIN(GetGValue(color) / 255.0);
    DOUBLE dbBlueLin = RGB_CHANNEL_LIN(GetBValue(color) / 255.0);

    return 0.2126 * dbRedLin + 0.7152 * dbGreenLin + 0.0722 * dbBlueLin;
}

static INT KM_SortPaletteByBrightnessCMP(COLORREF crOne, COLORREF crTwo)
{
    DOUBLE dbBrightnessOne = COLORREF_GetBrightness(crOne);
    DOUBLE dbBrightnessTwo = COLORREF_GetBrightness(crTwo);

    if (dbBrightnessOne < dbBrightnessTwo)
        return -1;
    else if (dbBrightnessOne > dbBrightnessTwo)
        return 1;
    else
        return 0;
}

VOID KM_SortPaletteByBrightness(PKM_PALETTE kmPalette)
{
    COLORREF crMin, crCur;
    INT iMin = 0;
    INT i, j;

    if (kmPalette == NULL)
        return;

    for (i = 0; i < (INT)kmPalette->dwColorsCount - 1; i++) {
        for (iMin = i, j = i + 1; j < (INT)kmPalette->dwColorsCount; j++) {
            crCur = kmPalette->aColors[j];
            crMin = kmPalette->aColors[iMin];

            if (KM_SortPaletteByBrightnessCMP(crCur, crMin) < 0)
                iMin = j;
        }

        crMin = kmPalette->aColors[iMin];
        kmPalette->aColors[iMin] = kmPalette->aColors[i];
        kmPalette->aColors[i] = crMin;
    }
}

VOID KM_FreePalette(PKM_PALETTE kmPalette)
{
    if (kmPalette == NULL)
        return;

    HeapFree(GetProcessHeap(), 0, kmPalette->aColors);
    HeapFree(GetProcessHeap(), 0, kmPalette);
}