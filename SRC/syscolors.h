#ifndef _SYSCOLORS_H_
#define _SYSCOLORS_H_

#include <windows.h>

#define COLOR_BTNALTFACE        25

#define SYSTEM_COLORS_COUNT     ARRAYSIZE(g_v_SystemColors)

struct _SYSTEM_COLOR
{
    INT         nId;
    LPCTSTR     pszValueName;
    COLORREF    crDefault;
} static const g_v_SystemColors[] = {
    { COLOR_ACTIVEBORDER,               TEXT("ActiveBorder"),             RGB(180, 180, 180) },
    { COLOR_ACTIVECAPTION,              TEXT("ActiveTitle"),              RGB(153, 180, 209) },
    { COLOR_APPWORKSPACE,               TEXT("AppWorkspace"),             RGB(171, 171, 171) },
    { COLOR_BACKGROUND,                 TEXT("Background"),               RGB(  0,   0,   0) },
    { COLOR_BTNALTFACE,                 TEXT("ButtonAlternateFace"),      RGB(  0,   0,   0) },
    { COLOR_3DDKSHADOW,                 TEXT("ButtonDkShadow"),           RGB(105, 105, 105) },
    { COLOR_BTNFACE,                    TEXT("ButtonFace"),               RGB(240, 240, 240) },
    { COLOR_BTNHILIGHT,                 TEXT("ButtonHilight"),            RGB(255, 255, 255) },
    { COLOR_3DLIGHT,                    TEXT("ButtonLight"),              RGB(227, 227, 227) },
    { COLOR_BTNSHADOW,                  TEXT("ButtonShadow"),             RGB(160, 160, 160) },
    { COLOR_BTNTEXT,                    TEXT("ButtonText"),               RGB(  0,   0,   0) },
    { COLOR_GRADIENTACTIVECAPTION,      TEXT("GradientActiveTitle"),      RGB(185, 209, 234) },
    { COLOR_GRADIENTINACTIVECAPTION,    TEXT("GradientInactiveTitle"),    RGB(215, 228, 242) },
    { COLOR_GRAYTEXT,                   TEXT("GrayText"),                 RGB(109, 109, 109) },
    { COLOR_HIGHLIGHT,                  TEXT("Hilight"),                  RGB(  0, 120, 215) },
    { COLOR_HIGHLIGHTTEXT,              TEXT("HilightText"),              RGB(255, 255, 255) },
    { COLOR_HOTLIGHT,                   TEXT("HotTrackingColor"),         RGB(  0, 102, 204) },
    { COLOR_INACTIVEBORDER,             TEXT("InactiveBorder"),           RGB(244, 247, 252) },
    { COLOR_INACTIVECAPTION,            TEXT("InactiveTitle"),            RGB(191, 205, 219) },
    { COLOR_INACTIVECAPTIONTEXT,        TEXT("InactiveTitleText"),        RGB(  0,   0,   0) },
    { COLOR_INFOTEXT,                   TEXT("InfoText"),                 RGB(  0,   0,   0) },
    { COLOR_INFOBK,                     TEXT("InfoWindow"),               RGB(255, 255, 255) },
    { COLOR_MENU,                       TEXT("Menu"),                     RGB(240, 240, 240) },
    { COLOR_MENUBAR,                    TEXT("MenuBar"),                  RGB(240, 240, 240) },
    { COLOR_MENUHILIGHT,                TEXT("MenuHilight"),              RGB(  0, 102, 215) },
    { COLOR_MENUTEXT,                   TEXT("MenuText"),                 RGB(  0,   0,   0) },
    { COLOR_SCROLLBAR,                  TEXT("Scrollbar"),                RGB(200, 200, 200) },
    { COLOR_CAPTIONTEXT,                TEXT("TitleText"),                RGB(  0,   0,   0) },
    { COLOR_WINDOW,                     TEXT("Window"),                   RGB(255, 255, 255) },
    { COLOR_WINDOWFRAME,                TEXT("WindowFrame"),              RGB(100, 100, 100) },
    { COLOR_WINDOWTEXT,                 TEXT("WindowText"),               RGB(  0,   0,   0) },
};

typedef struct _SYSTEM_COLOR const* LPCSYSTEM_COLOR;

#endif /* _SYSCOLORS_H_ */