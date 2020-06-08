/** @file
  preovide H2O deafult boot list related functions

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "Bds.h"
#include <Guid/BdsDefaultBootOptions.h>

/**
  Get whole file path from gH2OBdsDefaultBootOptionsTokenSpaceGuid token space PCD.

  This function allocates space for a new copy of the device path. If DevicePath is not
  found or allocate memory failed, then NULL is returned. The memory for the new device
  path is allocated from EFI boot services memory. It is the responsibility of the caller
  to free the memory allocated.

  @param[in] Handle      A pointer to a device handle.

  @return pointer to device path or NULL if not found or allocate memory failed.
**/
STATIC
EFI_DEVICE_PATH_PROTOCOL *
GetDevicePathFromDefaultBootOptionsPcd (
  IN UINTN          TokenNum
  )
{
  CHAR16                        *DefaultBootOption;
  CHAR16                        *FilePath;
  UINTN                          FilePathSize;
  UINTN                          Index;
  UINTN                          CurrentIndex;
  EFI_DEVICE_PATH_PROTOCOL       *DevicePath;

  DefaultBootOption = (CHAR16 *) LibPcdGetExPtr(&gH2OBdsDefaultBootOptionsTokenSpaceGuid, TokenNum);
  if (DefaultBootOption == NULL) {
    return NULL;
  }

  for (Index = 0; !(DefaultBootOption[Index] == L'\\' && DefaultBootOption[Index + 1] == L't'); Index++) {
    if (DefaultBootOption[Index] == L'\0') {
      break;
    }
  }
  FilePathSize = (Index + 1) * sizeof (CHAR16);
  FilePath = AllocateZeroPool (FilePathSize);
  if (FilePath == NULL) {
    return NULL;
  }
  CopyMem (FilePath, DefaultBootOption, FilePathSize - sizeof (CHAR16));
  //
  // Convert string "\\" to string "\"
  //
  for (Index = 0, CurrentIndex = 0; FilePath[Index] != L'\0'; Index++, CurrentIndex++) {
    if (FilePath[Index] ==  L'\\' && FilePath[Index + 1] ==  L'\\') {
      Index++;
    }
    FilePath[CurrentIndex] = FilePath[Index];
  }
  FilePath[CurrentIndex] = L'\0';
  DevicePath = ConvertTextToDevicePath (FilePath);
  FreePool (FilePath);
  return DevicePath;
}

/**
  Get option description string from gH2OBdsDefaultBootOptionsTokenSpaceGuid token space PCD.

  This function allocates space for option description string. If description is not found or allocate
  memory failed, then NULL is returned. The memory for the description is allocated from EFI boot
  services memory. It is the responsibility of the caller to free the memory allocated.

  @param[in] TokenNum     Token number in gH2OBdsDefaultBootOptionsTokenSpaceGuid token space.

  @return pointer to description or NULL if not found.
**/
STATIC
CHAR16 *
GetDescriptionFromDefaultBootOptionsPcd (
  IN UINTN          TokenNum
  )
{
  CHAR16     *DefaultBootOption;
  CHAR16     *TempPtr;
  CHAR16     *Description;
  UINTN      DescriptionSize;
  UINTN      Index;

  DefaultBootOption = (CHAR16 *) LibPcdGetExPtr(&gH2OBdsDefaultBootOptionsTokenSpaceGuid, TokenNum);
  if (DefaultBootOption == NULL) {
    return NULL;
  }

  for (Index = 0; !(DefaultBootOption[Index] == L'\\' && DefaultBootOption[Index + 1] == L't'); Index++) {
    if (DefaultBootOption[Index] == L'\0') {
      return NULL;
    }
  }
  TempPtr = &DefaultBootOption[Index];
  Index++;
  while (!(DefaultBootOption[Index] == L'\\' && DefaultBootOption[Index + 1] == L't') && DefaultBootOption[Index] != L'\0') {
    Index++;
  }

  DescriptionSize = (UINTN) (&DefaultBootOption[Index]) - (UINTN) TempPtr - sizeof (CHAR16);
  Description = AllocateZeroPool (DescriptionSize);
  if (Description == NULL) {
    return NULL;
  }
  CopyMem (Description, TempPtr + 2, DescriptionSize - sizeof (CHAR16));
  return Description;
}


/**
  Get option attributes from gH2OBdsDefaultBootOptionsTokenSpaceGuid token space PCD.

  @param[in] TokenNum     Token number in gH2OBdsDefaultBootOptionsTokenSpaceGuid token space.

  @return Attributes value of option. Attribute will be considered 0 if not found.
**/
STATIC
UINT32
GetAttributesFromDefaultBootOptionsPcd (
  IN UINTN          TokenNum
  )
{
  CHAR16     *DefaultBootOption;
  CHAR16     *TempPtr;
  CHAR16     *AttributesString;
  UINTN      AttributesStringSize;
  UINTN      Index;
  UINTN      TabIndex;
  UINT32     Attributes;

  DefaultBootOption = (CHAR16 *) LibPcdGetExPtr(&gH2OBdsDefaultBootOptionsTokenSpaceGuid, TokenNum);
  if (DefaultBootOption == NULL) {
    return 0;
  }

  //
  // skip device path and description
  //
  Index = 0;
  for (TabIndex = 0; TabIndex < 2; TabIndex++) {
    for (; !(DefaultBootOption[Index] == L'\\' && DefaultBootOption[Index + 1] == L't'); Index++) {
      if (DefaultBootOption[Index] == L'\0') {
        return 0;
      }
    }
    Index += 2;
  }
  TempPtr = &DefaultBootOption[Index];
  Index++;
  while (!(DefaultBootOption[Index] == L'\\' && DefaultBootOption[Index + 1] == L't') && DefaultBootOption[Index] != L'\0') {
    Index++;
  }

  AttributesStringSize = (UINTN) (&DefaultBootOption[Index]) - (UINTN) TempPtr + sizeof (CHAR16);
  AttributesString = AllocateZeroPool (AttributesStringSize);
  if (AttributesString == NULL) {
    return 0;
  }
  CopyMem (AttributesString, TempPtr, AttributesStringSize - sizeof (CHAR16));
  Attributes = (UINT32 ) StrHexToUintn (AttributesString);
  FreePool (AttributesString);
  return Attributes;
}


/**
  Convert all of escape sequences in string to C standard hex value.

  @param[in,out] String          A Null-terminated string to be converted.

  @retval EFI_SUCCESS            Convert string successfully.
  @retval EFI_INVALID_PARAMETER  String is NULL or input string format is incorrect.
**/
STATIC
EFI_STATUS
ConvertEscapeSequences (
  IN OUT  CHAR16     *String
  )
{
  UINTN     Index;
  UINTN     CurrentIndex;
  CHAR16    NumberString[4];


  if (String == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CurrentIndex = 0;
  for (Index = 0; String[Index] != L'\0'; Index++, CurrentIndex++) {
    if (String[Index] =='\\') {
      switch (String[Index + 1]) {

      case L'a':
        String[CurrentIndex] = L'\a';
        Index++;
        break;

      case L'b':
        String[CurrentIndex] = L'\b';
        Index++;
        break;

      case L'f':
        String[CurrentIndex] = L'\f';
        Index++;
        break;

      case L'n':
        String[CurrentIndex] = L'\n';
        Index++;
        break;

      case L'r':
        String[CurrentIndex] = L'\r';
        Index++;
        break;

      case L't':
        String[CurrentIndex] = L'\t';
        Index++;
        break;

      case L'\\':
        String[CurrentIndex] = L'\\';
        Index++;
        break;

      case L'\'':
        String[CurrentIndex] = L'\'';
        Index++;
        break;

      case L'\"':
        String[CurrentIndex] = L'\"';
        Index++;
        break;

      case L'?':
        String[CurrentIndex] = L'\?';
        Index++;
        break;

      default:
        if (String[Index + 1] == L'x') {
          if (String [Index + 2] == L'\0' || String [Index + 3] == L'\0') {
            return EFI_INVALID_PARAMETER;
          }
          CopyMem (NumberString, &String [Index + 2], 2 * sizeof (CHAR16));
          NumberString[2] = L'\0';
          String[CurrentIndex] = (CHAR16) StrHexToUintn (NumberString);
          Index += 3;
        } else if (String [Index + 1] != L'\0' || String [Index + 2] != L'\0' || String [Index + 3] != L'\0') {
          CopyMem (NumberString, &String [Index + 1], 3 * sizeof (CHAR16));
          NumberString[3] = L'\0';
          String[CurrentIndex] = (CHAR16) StrDecimalToUintn (NumberString);
          Index += 3;
        } else {
          return EFI_INVALID_PARAMETER;
        }
      }
    } else {
      String[CurrentIndex] = String[Index];
    }
  }
  String[CurrentIndex] = L'\0';
  return EFI_SUCCESS;
}


/**
  Convert optional data string which format is array data to binary data.

  This function allocates space for optional data. If optional data is not found or allocate
  memory failed, then failed is returned. The memory for the optional data is allocated from EFI boot
  services memory. It is the responsibility of the caller to free the memory allocated.

  @param[in]  TokenNum             Token number in gH2OBdsDefaultBootOptionsTokenSpaceGuid token space.
  @param[out] OptionalData         Pointer to optional data.
  @param[out] OptionalDataSize     Pointer to optional data size.

  @retval EFI_SUCCESS              Convert optional data string successfully.
  @retval EFI_INVALID_PARAMETER    OptionalDataString is NULL, OptinalData is NULL or OptionalDataSize is NULL.
  @retval EFI_INVALID_PARAMETER    Input string isn't array data format.
  @retval EFI_OUT_OF_RESOURCES     Allocate memory failed.
**/
STATIC
EFI_STATUS
ConvertArrayToData (
  IN  CHAR16         *OptionalDataString,
  OUT VOID           **OptionalData,
  OUT UINT32         *OptionalDataSize
  )
{
  UINTN      Index;
  UINTN      PreviousIndex;
  UINT32     Size;
  UINT8      *Data;
  CHAR16     *WorkingString;
  UINTN      Offset;

  if (OptionalDataString == NULL || OptionalData == NULL || OptionalDataSize == NULL ||
      OptionalDataString[0] != L'{') {
    return EFI_INVALID_PARAMETER;
  }

  Size = 1;
  for (Index = 1; OptionalDataString[Index] != L'}'; Index++) {
    if (OptionalDataString[Index] == L',') {
      Size++;
    }
  }

  WorkingString = AllocateCopyPool (StrSize (OptionalDataString), OptionalDataString);
  if (WorkingString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  Data = AllocateZeroPool (Size);
  if (Data == NULL) {
    FreePool (WorkingString);
    return EFI_OUT_OF_RESOURCES;
  }

  PreviousIndex = 1;
  Offset        = 0;
  for (Index = 1; WorkingString[Index] != L'}'; Index++) {
    if (WorkingString[Index] == ',') {
      WorkingString[Index] = L'\0';
      Data[Offset++] = (UINT8) StrHexToUintn (&WorkingString[PreviousIndex]);
      PreviousIndex  = Index + 1;
    }
  }
  WorkingString[Index] = L'\0';
  Data[Offset] = (UINT8) StrHexToUintn (&WorkingString[PreviousIndex]);
  FreePool (WorkingString);
  *OptionalData     = Data;
  *OptionalDataSize = Size;
  return EFI_SUCCESS;
}

/**
  Convert optional data string which format is CHAR16 string to binary data.

  This function allocates space for optional data. If optional data is not found or allocate
  memory failed, then failed is returned. The memory for the optional data is allocated from EFI boot
  services memory. It is the responsibility of the caller to free the memory allocated.

  @param[in]  TokenNum             Token number in gH2OBdsDefaultBootOptionsTokenSpaceGuid token space.
  @param[out] OptionalData         Pointer to optional data.
  @param[out] OptionalDataSize     Pointer to optional data size.

  @retval EFI_SUCCESS              Convert optional data string successfully.
  @retval EFI_INVALID_PARAMETER    OptionalDataString is NULL, OptinalData is NULL or OptionalDataSize is NULL.
  @retval EFI_INVALID_PARAMETER    Input string isn't CHAR16 string format.
  @retval EFI_OUT_OF_RESOURCES     Allocate memory failed.
**/
STATIC
EFI_STATUS
ConvertStringToData (
  IN  CHAR16         *OptionalDataString,
  OUT VOID           **OptionalData,
  OUT UINT32         *OptionalDataSize
  )
{
  CHAR16       *CopiedString;
  CHAR16       *WorkingString;
  UINTN        Index;
  EFI_STATUS   Status;

  if (OptionalDataString == NULL || OptionalData == NULL || OptionalDataSize == NULL ||
      OptionalDataString[0] != L'L' || OptionalDataString[1] != L'\\' || OptionalDataString[2] != L'\"') {
    return EFI_INVALID_PARAMETER;
  }

  CopiedString = AllocateCopyPool (StrSize (OptionalDataString), OptionalDataString);
  if (CopiedString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  WorkingString = AllocateZeroPool (StrSize (OptionalDataString));
  if (WorkingString == NULL) {
    FreePool (CopiedString);
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 3; !(CopiedString[Index - 1] == L'\\' && CopiedString[Index] == L'\"'); Index++) {
  }
  CopiedString[Index - 1] = L'\0';


  StrCpy (WorkingString, CopiedString + 3);
  FreePool (CopiedString);
  Status = ConvertEscapeSequences (WorkingString);
  if (Status != EFI_SUCCESS) {
    FreePool (WorkingString);
    return Status;
  }

  *OptionalData     = WorkingString;
  *OptionalDataSize = (UINT32) StrSize (WorkingString);
  return EFI_SUCCESS;
}

/**
  Convert optional data string which format is ASCII string to binary data.

  This function allocates space for optional data. If optional data is not found or allocate
  memory failed, then failed is returned. The memory for the optional data is allocated from EFI boot
  services memory. It is the responsibility of the caller to free the memory allocated.

  @param[in]  TokenNum             Token number in gH2OBdsDefaultBootOptionsTokenSpaceGuid token space.
  @param[out] OptionalData         Pointer to optional data.
  @param[out] OptionalDataSize     Pointer to optional data size.

  @retval EFI_SUCCESS              Convert optional data string successfully.
  @retval EFI_INVALID_PARAMETER    OptionalDataString is NULL, OptinalData is NULL or OptionalDataSize is NULL.
  @retval EFI_INVALID_PARAMETER    Input string isn't ASCII string format.
  @retval EFI_OUT_OF_RESOURCES     Allocate memory failed.
**/
STATIC
EFI_STATUS
ConvertAsciiStringToData (
  IN  CHAR16         *OptionalDataString,
  OUT VOID           **OptionalData,
  OUT UINT32         *OptionalDataSize
  )
{
  CHAR16       *CopiedString;
  CHAR8        *AsciiString;
  UINTN        Index;
  EFI_STATUS   Status;

  if (OptionalDataString == NULL || OptionalData == NULL || OptionalDataSize == NULL ||
      OptionalDataString[0] != L'\\' || OptionalDataString[1] != L'\"') {
    return EFI_INVALID_PARAMETER;
  }

  CopiedString = AllocateCopyPool (StrSize (OptionalDataString), OptionalDataString);
  if (CopiedString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 2; !(CopiedString[Index - 1] == L'\\' && CopiedString[Index] == L'\"'); Index++) {
  }
  CopiedString[Index - 1] = L'\0';
  Status = ConvertEscapeSequences (CopiedString + 2);
  if (Status != EFI_SUCCESS) {
    FreePool (CopiedString);
    return Status;
  }

  AsciiString = AllocateZeroPool (StrSize (OptionalDataString));
  if (AsciiString == NULL) {
    FreePool (CopiedString);
    return EFI_OUT_OF_RESOURCES;
  }
  UnicodeStrToAsciiStr (CopiedString + 2, AsciiString);
  FreePool (CopiedString);


  *OptionalData     = AsciiString;
  *OptionalDataSize = (UINT32) AsciiStrSize (AsciiString);
  return EFI_SUCCESS;
}

/**
  Convert optional data string from gH2OBdsDefaultBootOptionsTokenSpaceGuid token space PCD to binary data.

  This function allocates space for optional data. If optional data is not found or allocate
  memory failed, then failed is returned. The memory for the optional data is allocated from EFI boot
  services memory. It is the responsibility of the caller to free the memory allocated.

  @param[in]  TokenNum             Token number in gH2OBdsDefaultBootOptionsTokenSpaceGuid token space.
  @param[out] OptionalData         Pointer to optional data.
  @param[out] OptionalDataSize     Pointer to optional data size.

  @retval EFI_SUCCESS              Convert optional data string successfully.
  @retval EFI_INVALID_PARAMETER    OptionalDataString is NULL, OptinalData is NULL or OptionalDataSize is NULL.
  @retval EFI_UNSUPPORTED          Input optional data string format is unsupported.
**/
STATIC
EFI_STATUS
ConvertDataStringToData (
  IN  CHAR16         *OptionalDataString,
  OUT VOID           **OptionalData,
  OUT UINT32         *OptionalDataSize
  )
{

  if (OptionalDataString == NULL || OptionalData == NULL || OptionalDataSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (OptionalDataString[0] == L'{') {
    return ConvertArrayToData (OptionalDataString, OptionalData, OptionalDataSize);
  } else if (OptionalDataString[0] == L'L' && OptionalDataString[1] == L'\\' && OptionalDataString[2] == L'\"') {
    return ConvertStringToData (OptionalDataString, OptionalData, OptionalDataSize);
  } else if (OptionalDataString[0] == L'\\' && OptionalDataString[1] == L'\"') {
    return ConvertAsciiStringToData (OptionalDataString, OptionalData, OptionalDataSize);
  }

  return EFI_UNSUPPORTED;
}


/**
  Get optional data from gH2OBdsDefaultBootOptionsTokenSpaceGuid token space PCD.

  This function allocates space for optional data. If optional data is not found or allocate
  memory failed, then failed is returned. The memory for the optional data is allocated from EFI boot
  services memory. It is the responsibility of the caller to free the memory allocated.

  @param[in]  TokenNum             Token number in gH2OBdsDefaultBootOptionsTokenSpaceGuid token space.
  @param[out] OptionalData         Pointer to optional data.
  @param[out] OptionalDataSize     Pointer to optional data size.

  @retval EFI_SUCCESS              Get optional data successfully.
  @retval EFI_INVALID_PARAMETER    OptinalData is NULL or OptionalDataSize is NULL.
  @retval EFI_NOT_FOUND            Cannot find optional data.
  @retval EFI_OUT_OF_RESOURCES     Allocate memory to store optional data failed.
**/
STATIC
EFI_STATUS
GetOptionalDataFromDefaultBootOptionsPcd (
  IN  UINTN          TokenNum,
  OUT VOID           **OptinalData,
  OUT UINT32         *OptionalDataSize
  )
{
  CHAR16     *DefaultBootOption;
  CHAR16     *DataString;
  UINTN      DataStringSize;
  UINTN      Index;
  UINTN      TabIndex;
  VOID       *Data;
  UINT32     DataSize;
  EFI_STATUS Status;

  DefaultBootOption = (CHAR16 *) LibPcdGetExPtr(&gH2OBdsDefaultBootOptionsTokenSpaceGuid, TokenNum);
  if (DefaultBootOption == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // skip device path ,description and attribute
  //
  Index = 0;
  for (TabIndex = 0; TabIndex < 3; TabIndex++) {
    for (; !(DefaultBootOption[Index] == L'\\' && DefaultBootOption[Index + 1] == L't'); Index++) {
      if (DefaultBootOption[Index] == L'\0') {
        return EFI_NOT_FOUND;
      }
    }
    Index += 2;
  }

  DataStringSize = StrSize (&DefaultBootOption[Index]);
  DataString = AllocateZeroPool (DataStringSize);
  if (DataString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (DataString, &DefaultBootOption[Index], DataStringSize);
  Data     = NULL;
  DataSize = 0;
  Status = ConvertDataStringToData (DataString, &Data, &DataSize);
  if (Status == EFI_SUCCESS) {
   *OptinalData      = Data;
   *OptionalDataSize = DataSize;
  }
  return Status;
}

/**
  Add all of default boot options to boot list.

  @param[out] BootList             List header for boot options.

  @retval EFI_SUCCESS              Add boot options to boot list successfully.
  @retval EFI_INVALID_PARAMETER    BootList is NULL.
  @retval Other                    Create new boot option failed..
**/
EFI_STATUS
AddDefaultBootOptionsToBootList (
  OUT LIST_ENTRY           *BootList
  )
{
  UINTN                         TokenNum;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  CHAR16                        *Description;
  UINT32                        Attributes;
  VOID                          *OptionalData;
  UINT32                        OptionalDataSize;
  H2O_BDS_LOAD_OPTION           *BootOption;
  EFI_STATUS                    Status;

  if (BootList == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  for (TokenNum = LibPcdGetNextToken(&gH2OBdsDefaultBootOptionsTokenSpaceGuid, 0); TokenNum != 0; \
       TokenNum = LibPcdGetNextToken(&gH2OBdsDefaultBootOptionsTokenSpaceGuid, TokenNum)) {
    DevicePath = GetDevicePathFromDefaultBootOptionsPcd (TokenNum);
    if (DevicePath == NULL) {
      continue;
    }
    Description      = GetDescriptionFromDefaultBootOptionsPcd (TokenNum);
    Attributes       = GetAttributesFromDefaultBootOptionsPcd (TokenNum);
    OptionalData     = NULL;
    OptionalDataSize = 0;
    GetOptionalDataFromDefaultBootOptionsPcd (TokenNum, &OptionalData, &OptionalDataSize);
    Status = gBdsServices->CreateLoadOption (
                             gBdsServices,
                             BOOT_OPTION,
                             NULL,
                             NULL,
                             Attributes,
                             DevicePath,
                             Description,
                             OptionalData,
                             OptionalDataSize,
                             &BootOption
                             );
    FreePool (DevicePath);
    if (Description != NULL) {
      FreePool (Description);
    }
    if (OptionalData != NULL) {
      FreePool (OptionalData);
    }
    if (Status != EFI_SUCCESS) {
      return Status;
    }
    InsertTailList (BootList, &BootOption->Link);
  }

  return EFI_SUCCESS;
}