#ifndef _BMUTILS_H_
#define _BMUTILS_H_

#include <windows.h>

#if __cplusplus
extern "C" {
#endif /* __cplusplus */

HBITMAP HBITMAP_FromFile(LPCTSTR pszPath);

LPRGBQUAD HBITMAP_GetPixels(HBITMAP hBitmap, LPDWORD lpcPixelCount);

HBITMAP HBITMAP_Resize(HBITMAP hBitmap, UINT uNewWidth, UINT uNewHeight);

#if __cplusplus
}
#endif /* __cplusplus */

#endif /* _UTILS_H_ */