

#ifndef __MY_UEFI_STDC_H__
#define __MY_UEFI_STDC_H__

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

typedef VOID  *FILE;

#define INT_MAX      2147483647       /* max value for an int */


#define memcpy(dest,source,count)         CopyMem(dest,source,(UINTN)(count))
#define memset(dest,ch,count)             SetMem(dest,(UINTN)(count),(UINT8)(ch))
#define memchr(buf,ch,count)              ScanMem8(buf,(UINTN)(count),(UINT8)ch)
#define memcmp(buf1,buf2,count)           (int)(CompareMem(buf1,buf2,(UINTN)(count)))
#define memmove(dest,source,count)        CopyMem(dest,source,(UINTN)(count))
#define strcmp                            AsciiStrCmp
#define strncmp(string1,string2,count)    (int)(AsciiStrnCmp(string1,string2,(UINTN)(count)))
#define strcpy(strDest,strSource)         AsciiStrCpy(strDest,strSource)
#define strncpy(strDest,strSource,count)  AsciiStrnCpy(strDest,strSource,(UINTN)count)
#define strlen(str)                       (UINTN)(AsciiStrLen(str))
#define strcat(strDest,strSource)         AsciiStrCat(strDest,strSource)
#define strchr(str,ch)                    ScanMem8((VOID *)(str),AsciiStrSize(str),(UINT8)ch)
#define abort()                           ASSERT (FALSE)
#define assert(expression)
#define localtime(timer)                  NULL
#define gmtime_r(timer,result)            (result = NULL)
#define snprintf                          AsciiSPrint


#define malloc(n) AllocatePool(n)
#define free(n)   FreePool(n)
void *realloc(void *Buffer, UINTN Size);

#endif

