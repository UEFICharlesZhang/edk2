#ifndef __BASE_LIBRARY_H_
#define __BASE_LIBRARY_H_


#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

typedef union {
  UINT8 Utf8_1;
  UINT8 Utf8_2[2];
  UINT8 Utf8_3[3];
} UTF8_CHAR;

/**
  Get one valid VT-UTF8 characters.

  @param  Utf8StrBuffer       The Utf8 string buffer.
  @param  Utf8Char            Returned valid VT-UTF8 characters set.
  @param  ValidBytes          The count of returned VT-VTF8 characters.
                              If ValidBytes is zero, no valid VT-UTF8 returned.

**/
CHAR8 *
EFIAPI
GetOneValidUtf8Char (
  IN  CHAR8                  *Utf8StrBuffer,
  OUT UTF8_CHAR              *Utf8Char,
  OUT UINT8                  *ValidBytes
  );

/**
  Translate VT-UTF8 characters into one Unicode character.

  UTF8 Encoding Table
  Bits per Character | Unicode Character Range | Unicode Binary  Encoding |  UTF8 Binary Encoding
       0-7           |     0x0000 - 0x007F     |     00000000 0xxxxxxx    |   0xxxxxxx
       8-11          |     0x0080 - 0x07FF     |     00000xxx xxxxxxxx     |   110xxxxx 10xxxxxx
       12-16         |     0x0800 - 0xFFFF     |     xxxxxxxx xxxxxxxx    |   1110xxxx 10xxxxxx 10xxxxxx


  @param  Utf8Char         VT-UTF8 character set needs translating.
  @param  ValidBytes       The count of valid VT-UTF8 characters.
  @param  UnicodeChar      Returned unicode character.

**/
VOID
Utf8ToUnicode (
  IN  UTF8_CHAR              Utf8Char,
  IN  UINT8                  ValidBytes,
  OUT CHAR16                 *UnicodeChar
  );

/**
  Convert one Null-terminated Utf8 string to a Null-terminated
  Unicode string and returns the Unicode string.

  @param  Utf8Str       A pointer to a Null-terminated Utf8 string.

  @return Destination   A pointer to a Null-terminated Unicode string.

**/
CHAR16 *
EFIAPI
Utf8StrToUnicodeStr (
  IN CONST CHAR8             *Utf8Str
  );


#endif

