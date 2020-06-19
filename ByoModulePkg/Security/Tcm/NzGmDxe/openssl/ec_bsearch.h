/* bsearch.h */

void *wapi_Bsearch(const void *key, const void *base, UINTN num, UINTN width, int (__cdecl *compare)(const void *, const void *));