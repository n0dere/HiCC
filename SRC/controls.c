#include "controls.h"

#include <CommCtrl.h>
#include <windowsx.h>

#include "resource.h"

#define LOADSTRING_MAX_SZ			512

#define PALETTE_CHKMBTN(palette, prect)										\
	((prect)->left < (palette)->iCurX && (prect)->right > (palette)->iCurX)

static void DrawTransparentRectangle(HDC hDC, LPRECT rect, COLORREF color,
									 BYTE bAlpha)
{
	HDC hMemDC = CreateCompatibleDC(hDC);
	INT iWidth = rect->right - rect->left;
	INT iHeight = rect->bottom - rect->top;
	HBITMAP bitmap = CreateCompatibleBitmap(hDC, iWidth, iHeight);
	HBITMAP oldBitmap = (HBITMAP)SelectObject(hMemDC, bitmap);
	HBRUSH brush = CreateSolidBrush(color);
	RECT fillRect = { 0, 0, iWidth, iHeight };
	BLENDFUNCTION blend;

	FillRect(hMemDC, &fillRect, brush);
	DeleteObject(brush);

	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = bAlpha;
	blend.AlphaFormat = 0;

	AlphaBlend(hDC, rect->left, rect->top, iWidth, iHeight, hMemDC, 0, 0,
			   iWidth, iHeight, blend);

	SelectObject(hMemDC, oldBitmap);
	DeleteObject(bitmap);
	DeleteDC(hMemDC);
}

typedef struct tagPREVIEW
{
	HBITMAP             hBitmapBG;
	COLORREF            crHilight;
	COLORREF            crHotTrackingColor;
} *PPREVIEW;

static void Preview_PaintBG(PPREVIEW pPreview, HDC hDC, PRECT pRect)
{
	HDC hDCBitmap;
	HBITMAP hOldBitmap;
	BITMAP bm;
	INT x, y;

	if (pPreview == NULL || hDC == NULL || pRect == NULL)
		return;

	if (pPreview->hBitmapBG == NULL)
		return;

	GetObject(pPreview->hBitmapBG, sizeof bm, (LPVOID)&bm);

	hDCBitmap = CreateCompatibleDC(hDC);
	hOldBitmap = SelectObject(hDCBitmap, pPreview->hBitmapBG);

	x = bm.bmWidth - (INT)((pRect->right + bm.bmWidth) / 2.0);
	y = bm.bmHeight - (INT)((pRect->bottom + bm.bmHeight) / 2.0);

	BitBlt(hDC, 0, 0, pRect->right, pRect->bottom, hDCBitmap, x, y, SRCCOPY);

	SelectObject(hDCBitmap, hOldBitmap);
	DeleteDC(hDCBitmap);
}

static void Preview_OnPaint(PPREVIEW pPreview, HDC hDC, PRECT pRect)
{
	HBRUSH hBrush;
	INT x, y;

	if (pPreview == NULL || hDC == NULL || pRect == NULL)
		return;

	FillRect(hDC, pRect, GetSysColorBrush(COLOR_BACKGROUND));
	Preview_PaintBG(pPreview, hDC, pRect);

	x = (INT)(pRect->left + pRect->right * 0.2); /* 20% */
	y = (INT)(pRect->top + pRect->bottom * 0.2); /* 20% */

	InflateRect(pRect, -x, -y);
	DrawTransparentRectangle(hDC, pRect, pPreview->crHotTrackingColor, 65);
	hBrush = CreateSolidBrush(pPreview->crHilight);
	FrameRect(hDC, pRect, hBrush);
	DrawIcon(hDC, pRect->right, pRect->bottom, LoadCursor(NULL, IDC_ARROW));
	DeleteObject(hBrush);
	InflateRect(pRect, x, y);

	DrawEdge(hDC, pRect, EDGE_SUNKEN, BF_RECT);
}

static LRESULT CALLBACK Preview_Proc(HWND hWnd, UINT uMsg, WPARAM wParam,
									 LPARAM lParam)
{
	PPREVIEW pPreview = (PPREVIEW)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	PAINTSTRUCT ps;
	HDC hDC;
	RECT rect;

	switch (uMsg) {
		case WM_NCCREATE:
			pPreview = HeapAlloc(GetProcessHeap(), 0, sizeof * pPreview);

			if (pPreview == NULL)
				return FALSE;

			ZeroMemory(pPreview, sizeof * pPreview);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pPreview);
			break;

		case XXM_PREVIEW_UPDATE_BG:
			pPreview->hBitmapBG = (HBITMAP)lParam;
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case XXM_PREVIEW_UPDATE_HILIGHT:
			pPreview->crHilight = (COLORREF)lParam;
			GetClientRect(hWnd, &rect);
			InflateRect(&rect, -5, -5);
			InvalidateRect(hWnd, &rect, FALSE);
			break;

		case XXM_PREVIEW_UPDATE_HTC:
			pPreview->crHotTrackingColor = (COLORREF)lParam;
			GetClientRect(hWnd, &rect);
			InflateRect(&rect, -5, -5);
			InvalidateRect(hWnd, &rect, FALSE);
			break;

		case WM_PAINT:
			GetClientRect(hWnd, &rect);
			hDC = BeginPaint(hWnd, &ps);
			Preview_OnPaint(pPreview, hDC, &rect);
			EndPaint(hWnd, &ps);
			break;

		case WM_NCDESTROY:
			if (pPreview != NULL)
				HeapFree(GetProcessHeap(), 0, pPreview);
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND Preview_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
					INT nHeight)
{
	return CreateWindow(PREVIEWCLASSNAME, NULL, WS_CHILD | WS_VISIBLE,
						x, y, nWidth, nHeight, hParent, (HMENU)uId, NULL,
						NULL);
}

typedef struct tagPALETTE
{
	INT						iCurX, iCurY;
	PKM_PALETTE				pKmPalette;
	BOOL					bLMB_Down;
	DOUBLE					dbSegWidth;
} *PPALETTE;

static void Palette_OnPaint(PPALETTE pPalette, HDC hDC, PRECT pRect)
{
	HBRUSH hBrush = NULL;
	INT i;

	if (pPalette == NULL || hDC == NULL || pRect == NULL)
		return;

	if (pPalette->pKmPalette == NULL) {
		FillRect(hDC, pRect, GetSysColorBrush(COLOR_BACKGROUND));
		SetTextColor(hDC, RGB(255, 255, 255));
		SetBkMode(hDC, TRANSPARENT);
		DrawText(hDC, L"No Wallpapers", -1, pRect,
			     DT_VCENTER | DT_SINGLELINE | DT_CENTER);
		DrawEdge(hDC, pRect, EDGE_SUNKEN, BF_RECT);
		return;
	}

	for (i = 0; i < (INT)pPalette->pKmPalette->dwColorsCount; i++) {
		hBrush = CreateSolidBrush(pPalette->pKmPalette->aColors[i]);

		pRect->left = (LONG)(pPalette->dbSegWidth * i);
		pRect->right = (LONG)(pPalette->dbSegWidth * (i + 1));

		FillRect(hDC, pRect, hBrush);
		DrawEdge(hDC, pRect, EDGE_SUNKEN, BF_RECT);
		DeleteObject(hBrush);

		if (pPalette->bLMB_Down && PALETTE_CHKMBTN(pPalette, pRect)) {
			hBrush = CreateSolidBrush(RGB(0, 0, 0));
			pRect->right -= 1;
			pRect->bottom -= 1;
			FrameRect(hDC, pRect, hBrush);
			pRect->right += 1;
			pRect->bottom += 1;
			DeleteObject(hBrush);
		}
	}
}

static void Palette_OnMouseUp(PPALETTE pPalette, HWND hWnd)
{
	INT i;

	if (pPalette == NULL || hWnd == NULL || pPalette->pKmPalette == NULL)
		return;

	i = (INT)(pPalette->iCurX / pPalette->dbSegWidth);

	SendMessage(GetParent(hWnd), XXM_PALETTE_CLRSEL, 0,
				(LPARAM)pPalette->pKmPalette->aColors[i]);
}

static LRESULT CALLBACK Palette_Proc(HWND hWnd, UINT uMsg, WPARAM wParam,
									 LPARAM lParam)
{
	PPALETTE pPalette = (PPALETTE)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	TRACKMOUSEEVENT tme;
	PAINTSTRUCT ps;
	HDC hDC;
	RECT rect;
	DWORD dwColors;

	switch (uMsg) {
		case WM_NCCREATE:
			pPalette = HeapAlloc(GetProcessHeap(), 0, sizeof * pPalette);

			if (pPalette == NULL)
				return FALSE;

			ZeroMemory(pPalette, sizeof * pPalette);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pPalette);
			break;

		case XXM_PALETTE_UPDATE:
			pPalette->pKmPalette = (PKM_PALETTE)lParam;
			
			if (pPalette->pKmPalette != NULL) {
				GetClientRect(hWnd, &rect);
				dwColors = pPalette->pKmPalette->dwColorsCount;
				pPalette->dbSegWidth = ((DOUBLE)rect.right) / dwColors;
			}

			break;

		case WM_PAINT:
			GetClientRect(hWnd, &rect);
			hDC = BeginPaint(hWnd, &ps);
			Palette_OnPaint(pPalette, hDC, &rect);
			EndPaint(hWnd, &ps);
			break;

		case WM_MOUSEMOVE:
			pPalette->iCurX = GET_X_LPARAM(lParam);
			pPalette->iCurY = GET_Y_LPARAM(lParam);
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_LEAVE;
			tme.hwndTrack = hWnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
			break;

		case WM_LBUTTONUP:
			Palette_OnMouseUp(pPalette, hWnd);

		case WM_MOUSELEAVE:
		case WM_LBUTTONDOWN:
			pPalette->bLMB_Down = (uMsg == WM_LBUTTONDOWN);
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case WM_NCDESTROY:
			if (pPalette != NULL)
				HeapFree(GetProcessHeap(), 0, pPalette);
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND Palette_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
					INT nHeight, PKM_PALETTE pKmPalette)
{
	HWND hWnd = CreateWindow(PALETTECLASSNAME, NULL, WS_CHILD | WS_VISIBLE,
							 x, y, nWidth, nHeight, hParent, (HMENU)uId, NULL,
							 NULL);

	SendMessage(hWnd, XXM_PALETTE_UPDATE, 0, (LPARAM)pKmPalette);

	return hWnd;
}

HWND Button_Create(HWND hParent, UINT uId, INT x, INT y, INT nWidth,
				   INT nHeight, UINT uNameId)
{
	TCHAR szName[LOADSTRING_MAX_SZ] = { 0 };

	if (uNameId != 0)
		LoadString(NULL, uNameId, szName, ARRAYSIZE(szName));

	return CreateWindow(TEXT("Button"), szName, WS_VISIBLE | WS_CHILD, x, y,
						nWidth, nHeight, hParent, (HMENU)uId, NULL, NULL);
}

HWND Static_Create(HWND hParent, INT x, INT y, INT nWidth, INT nHeight,
				   UINT uTextId)
{
	TCHAR szName[LOADSTRING_MAX_SZ] = { 0 };

	if (uTextId != 0)
		LoadString(NULL, uTextId, szName, ARRAYSIZE(szName));

	return CreateWindow(TEXT("Static"), szName, WS_VISIBLE | WS_CHILD, x, y,
						nWidth, nHeight, hParent, NULL, NULL, NULL);
}

BOOL Controls_RegisterAllClasses(VOID)
{
	WNDCLASS wndClass;

	ZeroMemory(&wndClass, sizeof wndClass);

	wndClass.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = Preview_Proc;
	wndClass.lpszClassName = PREVIEWCLASSNAME;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	if (RegisterClass(&wndClass) == 0)
		return FALSE;

	wndClass.lpfnWndProc = Palette_Proc;
	wndClass.lpszClassName = PALETTECLASSNAME;
	wndClass.hCursor = LoadCursor(NULL, IDC_HAND);

	if (RegisterClass(&wndClass) == 0)
		return FALSE;

	return TRUE;
}

VOID Controls_UnregisterAllClasses(VOID)
{
	UnregisterClass(PALETTECLASSNAME, NULL);
	UnregisterClass(PREVIEWCLASSNAME, NULL);
}