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

LONG ColorsRegistrySet(LPCTSTR pszValueName, COLORREF crValue);

COLORREF ColorsRegistryGet(LPCTSTR pszValueName);

LONG ColorsRegistryResetToDefaults(VOID);

BOOL ColorsRegistryUpdateSystem(VOID);

#endif /* _CLRREGISTRY_H_ */