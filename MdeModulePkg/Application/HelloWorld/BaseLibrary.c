#include "BaseLibrary.h"
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
  )
{
  UINT8                      Temp;
  UINT8                      Index;
  BOOLEAN                    FetchFlag;
  CHAR8                      *pUtf8StrBuffer;

  Temp      = 0;
  Index     = 0;
  FetchFlag = TRUE;

  if ((Utf8StrBuffer == NULL) || (Utf8StrBuffer[0] == '\0')) {
    return NULL;
  }

  if ((Utf8Char == NULL) || (ValidBytes == NULL)) {
    return NULL;
  }

  //
  // if no valid Utf8 char is found in the RawFiFo,
  // then *ValidBytes will be zero.
  //
  *ValidBytes = 0;

  pUtf8StrBuffer = Utf8StrBuffer;
  while (*pUtf8StrBuffer != '\0') {
    Temp = *pUtf8StrBuffer ++;
    switch (*ValidBytes) {
      case 0:
        if ((Temp & 0x80) == 0) {
          //
          // one-byte utf8 char
          //
          *ValidBytes       = 1;
          Utf8Char->Utf8_1  = Temp;
          FetchFlag         = FALSE;
        } else if ((Temp & 0xe0) == 0xc0) {
          //
          // two-byte utf8 char
          //
          *ValidBytes         = 2;
          Utf8Char->Utf8_2[1] = Temp;
        } else if ((Temp & 0xf0) == 0xe0) {
          //
          // three-byte utf8 char
          //
          *ValidBytes         = 3;
          Utf8Char->Utf8_3[2] = Temp;
          Index++;
        } else {
          //
          // reset *ValidBytes to zero, let valid utf8 char search restart
          //
          *ValidBytes = 0;
        }
        break;
      case 2:
        //
        // two-byte utf8 char go on
        //
        if ((Temp & 0xc0) == 0x80) {
          Utf8Char->Utf8_2[0] = Temp;
          FetchFlag           = FALSE;
        } else {
          *ValidBytes = 0;
        }
        break;
      case 3:
        //
        // three-byte utf8 char go on
        //
        if ((Temp & 0xc0) == 0x80) {
          if (Index == 1) {
            Utf8Char->Utf8_3[1] = Temp;
            Index++;
          } else {
            Utf8Char->Utf8_3[0] = Temp;
            FetchFlag = FALSE;
          }
        } else {
          //
          // reset *ValidBytes and Index to zero, let valid utf8 char search restart
          //
          *ValidBytes = 0;
          Index       = 0;
        }
        break;
      default:
        break;
    }
    if (!FetchFlag) {
      break;
    }
  }

  return pUtf8StrBuffer;
}

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
  )
{
  UINT8                      UnicodeByte0;
  UINT8                      UnicodeByte1;
  UINT8                      Byte0;
  UINT8                      Byte1;
  UINT8                      Byte2;

  if (UnicodeChar == NULL) {
    return ;
  }

  *UnicodeChar = 0;

  //
  // translate utf8 code to unicode, in terminal standard,
  // up to 3 bytes utf8 code is supported.
  //
  switch (ValidBytes) {
    case 1:
      //
      // one-byte utf8 code
      //
      *UnicodeChar = (UINT16) Utf8Char.Utf8_1;

      break;
    case 2:
      //
      // two-byte utf8 code
      //
      Byte0         = Utf8Char.Utf8_2[0];
      Byte1         = Utf8Char.Utf8_2[1];

      UnicodeByte0  = (UINT8) ((Byte1 << 6) | (Byte0 & 0x3f));
      UnicodeByte1  = (UINT8) ((Byte1 >> 2) & 0x07);
      *UnicodeChar  = (UINT16) (UnicodeByte0 | (UnicodeByte1 << 8));
      break;

    case 3:
      //
      // three-byte utf8 code
      //
      Byte0         = Utf8Char.Utf8_3[0];
      Byte1         = Utf8Char.Utf8_3[1];
      Byte2         = Utf8Char.Utf8_3[2];

      UnicodeByte0  = (UINT8) ((Byte1 << 6) | (Byte0 & 0x3f));
      UnicodeByte1  = (UINT8) ((Byte2 << 4) | ((Byte1 >> 2) & 0x0f));
      *UnicodeChar  = (UINT16) (UnicodeByte0 | (UnicodeByte1 << 8));

    default:
      break;
  }

  return ;
}
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
  )
{
  UTF8_CHAR                  Utf8Char;
  UINT8                      ValidBytes;
  CHAR8                      *pUtf8Str;
  CHAR16                     *UnicodeStrBuffer;
  UINTN                      UnicodeStrBufferSize;
  UINTN                      UnicodeStrBufferIndex;

  ValidBytes           = 0x00;
  UnicodeStrBuffer     = NULL;
  UnicodeStrBufferSize = 0x00;
  ZeroMem (&Utf8Char, sizeof (Utf8Char));

  if ((Utf8Str == NULL) || (Utf8Str[0] == '\0')) {
    return NULL;
  }

  pUtf8Str = (CHAR8 *)Utf8Str;
  while (*pUtf8Str != '\0') {
    pUtf8Str = GetOneValidUtf8Char (pUtf8Str, &Utf8Char, &ValidBytes);
    if (pUtf8Str == NULL) {
      return NULL;
    }
    if ((ValidBytes < 0x01) || (ValidBytes > 0x03)) {
      continue;
    }
    UnicodeStrBufferSize ++;
  }

  UnicodeStrBufferSize += 0x01;

  UnicodeStrBuffer = (CHAR16 *)AllocateZeroPool (UnicodeStrBufferSize * sizeof (CHAR16));
  if (UnicodeStrBuffer == NULL) {
    return NULL;
  }

  UnicodeStrBufferIndex = 0x00;
  pUtf8Str = (CHAR8 *)Utf8Str;
  while (*pUtf8Str != '\0') {
    pUtf8Str = GetOneValidUtf8Char (pUtf8Str, &Utf8Char, &ValidBytes);
    if (pUtf8Str == NULL) {
      return NULL;
    }
    if ((ValidBytes < 0x01) || (ValidBytes > 0x03)) {
      continue;
    }
    Utf8ToUnicode (Utf8Char, ValidBytes, &UnicodeStrBuffer[UnicodeStrBufferIndex]);
    UnicodeStrBufferIndex ++;
  }
  UnicodeStrBuffer[UnicodeStrBufferIndex] = L'\0';

  return UnicodeStrBuffer;
}
