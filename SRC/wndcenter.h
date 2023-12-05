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

#ifndef _WNDCENTER_H_
#define _WNDCENTER_H_

#include <Windows.h>

#define MoveWindowToScreenCenter(hWnd)                                  \
    MoveWindowToParentCenter(GetDesktopWindow(), (hWnd))

static VOID MoveWindowToParentCenter(HWND hParent, HWND hWnd)
{
    RECT rcParent, rcWindow;
    INT iParentCenterX, iParentCenterY;
    INT x, y;

    if (hWnd == NULL || hParent == NULL)
        return;

    GetWindowRect(hParent, &rcParent);
    GetWindowRect(hWnd, &rcWindow);
    
    iParentCenterX = rcParent.left + (rcParent.right - rcParent.left) / 2;
    iParentCenterY = rcParent.top + (rcParent.bottom - rcParent.top) / 2;

    x = iParentCenterX - (rcWindow.right - rcWindow.left) / 2;
    y = iParentCenterY - (rcWindow.bottom - rcWindow.top) / 2;

    SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE);
}

#endif /* _WNDCENTER_H_ */