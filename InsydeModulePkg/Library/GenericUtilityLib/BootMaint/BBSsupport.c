/** @file
 GenericUtilityLib
 This function deal with the legacy boot option, it create, delete
 and manage the legacy boot option, all legacy boot option is getting from
 the legacy BBS table.

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

Copyright (c) 2004 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

*/

#include "BBSsupport.h"

STATIC BOOT_OPTION_BBS_MAPPING  *mBootOptionBbsMapping     = NULL;
STATIC UINTN                    mBootOptionBbsMappingCount = 0;


/**
 Translate the first n characters of an Ascii string to
 Unicode characters. The count n is indicated by parameter
 Size. If Size is greater than the length of string, then
 the entire string is translated.

 @param[in]  AStr               Pointer to input Ascii string.
 @param[in]  Size               The number of characters to translate.
 @param[out] UStr               Pointer to output Unicode string buffer.
**/
VOID
AsciiToUnicodeSize (
  IN UINT8              *AStr,
  IN UINTN              Size,
  OUT UINT16            *UStr
  )
{
  UINTN Idx;

  Idx = 0;
  while (AStr[Idx] != 0) {
    UStr[Idx] = (CHAR16) AStr[Idx];
    if (Idx == Size) {
      break;
    }

    Idx++;
  }
  UStr[Idx] = 0;
}

/**
 Build Legacy Device Name String according.

 @param[in]  CurBBSEntry     BBS Table.
 @param[in]  Index           Index.
 @param[in]  BufSize         The buffer size.
 @param[out] BootString      The output string.
**/
VOID
BuildLegacyDevNameString (
  IN  BBS_TABLE                 *CurBBSEntry,
  IN  UINTN                     Index,
  IN  UINTN                     BufSize,
  OUT CHAR16                    *BootString
  )
{
  CHAR16  *Fmt;
  CHAR16  *Type;
  UINT8   *StringDesc;
  CHAR16  Temp[80];

  switch (Index) {
  //
  // Primary Master
  //
  case 1:
    Fmt = L"Primary Master %s";
    break;

  //
  // Primary Slave
  //
  case 2:
    Fmt = L"Primary Slave %s";
    break;

  //
  // Secondary Master
  //
  case 3:
    Fmt = L"Secondary Master %s";
    break;

  //
  // Secondary Slave
  //
  case 4:
    Fmt = L"Secondary Slave %s";
    break;

  default:
    Fmt = L"%s";
    break;
  }

  switch (CurBBSEntry->DeviceType) {
  case BBS_FLOPPY:
    Type = L"Floppy";
    break;

  case BBS_HARDDISK:
    Type = L"Harddisk";
    break;

  case BBS_CDROM:
    Type = L"CDROM";
    break;

  case BBS_PCMCIA:
    Type = L"PCMCIAe";
    break;

  case BBS_USB:
    Type = L"USB";
    break;

  case BBS_EMBED_NETWORK:
    Type = L"Network";
    break;

  case BBS_BEV_DEVICE:
    Type = L"BEVe";
    break;

  case BBS_UNKNOWN:
  default:
    Type = L"Unknown";
    break;
  }
  //
  // If current BBS entry has its description then use it.
  //
  StringDesc = (UINT8 *) (UINTN) ((CurBBSEntry->DescStringSegment << 4) + CurBBSEntry->DescStringOffset);
  if (NULL != StringDesc) {
    //
    // Only get fisrt 32 characters, this is suggested by BBS spec
    //
    AsciiToUnicodeSize (StringDesc, 32, Temp);
    Fmt   = L"%s";
    Type  = Temp;
  }

  //
  // BbsTable 16 entries are for onboard IDE.
  // Set description string for SATA harddisks, Harddisk 0 ~ Harddisk 11
  //
  if (Index >= 5 && Index <= 16 && (CurBBSEntry->DeviceType == BBS_HARDDISK || CurBBSEntry->DeviceType == BBS_CDROM)) {
    Fmt = L"%s %d";
    UnicodeSPrint (BootString, BufSize, Fmt, Type, Index - 5);
  } else {
    UnicodeSPrint (BootString, BufSize, Fmt, Type);
  }
}

/**
 Build Legacy Device Name String according.

 @param[in]  CurBBSEntry     BBS Table.
 @param[in]  HddInfo         Onboard IDE controller information.
 @param[in]  Index           Index.
 @param[in]  BufSize         The buffer size.
 @param[out] BootString      The output string.
**/
VOID
BdsBuildLegacyDevNameString (
  IN BBS_TABLE                 *CurBBSEntry,
  IN HDD_INFO                  *HddInfo,
  IN UINTN                     Index,
  IN UINTN                     BufSize,
  OUT CHAR16                   *BootString
  )
{
  UINT8                        *StringDesc;
  UINT16                       Channel;
  UINT16                       Device;

  if (HddInfo == NULL) {
    BuildLegacyDevNameString (CurBBSEntry, Index, BufSize, BootString);
    return;
  }

  ZeroMem (BootString, BufSize);

  if (Index == 0) {
    UnicodeSPrint (BootString, BufSize, L"%s", L"Floppy");
    return;
  }

  //
  // If current BBS entry has its description then use it.
  //
  StringDesc = (UINT8*)(UINTN)((CurBBSEntry->DescStringSegment << 4) + CurBBSEntry->DescStringOffset);
  if (StringDesc != NULL) {
    //
    // Only get fisrt 32 characters, this is suggested by BBS spec
    //
    BdsLibAsciiToUnicodeSize (
      StringDesc,
      32,
      BootString
      );
  //
  // Maximum Onboard devices 8 IDE * 2 (Master/Slave) + Floppy
  //
  } else if (Index < (8 * 2 + 1)) {
    //
    // Get the drive information data. Odd indices are masters and even indices are slaves.
    //
    Channel = (UINT16)((Index - 1) / 2);
    Device  = (UINT16)((Index - 1) % 2);
    BdsLibUpdateAtaString (
      (EFI_IDENTIFY_DATA *) &HddInfo[Channel].IdentifyDrive[Device],
      BufSize,
      &BootString
      );
  } else {
    UnicodeSPrint (
      BootString,
      BufSize, L"%s",
      L"Unknow Device"
      );
  }

  return;
}

/**
 Create a legacy boot option for the specified entry of
 BBS table, save it as variable, and append it to the boot
 order list.

 @param[in] CurrentBbsEntry         Pointer to current BBS table.
 @param[in] CurrentBbsDevPath       Pointer to the Device Path Protocol instance of BBS
 @param[in] Index                   Index of the specified entry in BBS table.
 @param[in, out] BootOrderList      On input, the original boot order list.
                                    On output, the new boot order list attached with the created node.
 @param[in, out] BootOrderListSize  On input, the original size of boot order list.
                                    On output, the size of new boot order list.

 @retval EFI_SUCCESS             Boot Option successfully created.
 @retval EFI_OUT_OF_RESOURCES    Fail to allocate necessary memory.
 @retval Other                   Error occurs while setting variable.
**/
EFI_STATUS
BdsCreateLegacyBootOption (
  IN BBS_TABLE                        *CurrentBbsEntry,
  IN EFI_DEVICE_PATH_PROTOCOL         *CurrentBbsDevPath,
  IN UINTN                            Index,
  IN OUT UINT16                       **BootOrderList,
  IN OUT UINTN                        *BootOrderListSize
  )
{
  EFI_STATUS           Status;
  UINT16               CurrentBootOptionNo;
  UINT16               BootString[10];
  CHAR16               BootDesc[100];
  CHAR8                HelpString[100];
  UINT16               *NewBootOrderList;
  UINTN                BufferSize;
  UINTN                StringLen;
  VOID                 *Buffer;
  UINT8                *Ptr;
  UINT16               CurrentBbsDevPathSize;
  BBS_BBS_DEVICE_PATH  *NewBbsDevPathNode;
  UINT16               HddCount;
  UINT16               BbsCount;
  HDD_INFO             *LocalHddInfo;
  BBS_TABLE            *LocalBbsTable;
  EFI_LEGACY_BIOS_PROTOCOL  *LegacyBios;
  UINTN                               BootOptionNum;
  EFI_BOOT_OPTION_POLICY_PROTOCOL     *BootOptionPolicy;
  UINTN                               NewPosition;
  UINTN                               BootOptionType;
  UINTN                               DevicePathProtocolSize;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePathProtocol;

  BootOptionNum = 0;
  NewPosition = 0;
  BootOptionType = 0;

  if ((*BootOrderList) == NULL) {
    CurrentBootOptionNo = 0;
  } else {
    BootOptionNum = (UINTN) (*BootOrderListSize) / sizeof (UINT16);
    GetNewBootOptionNo (
      *BootOrderList,
      BootOptionNum,
      &CurrentBootOptionNo
      );
  }

  LocalHddInfo = NULL;
  Status = EfiLibLocateProtocol (&gEfiLegacyBiosProtocolGuid, (VOID **) &LegacyBios);
  if (!EFI_ERROR (Status)) {
    LegacyBios->GetBbsInfo (
                  LegacyBios,
                  &HddCount,
                  &LocalHddInfo,
                  &BbsCount,
                  &LocalBbsTable
                  );
  }

  UnicodeSPrint (
    BootString,
    sizeof (BootString),
    L"Boot%04x",
    CurrentBootOptionNo
    );

  BdsBuildLegacyDevNameString (CurrentBbsEntry, LocalHddInfo, Index, sizeof (BootDesc), BootDesc);

  //
  // Create new BBS device path node with description string
  //
  UnicodeStrToAsciiStr (BootDesc, HelpString);

  StringLen = AsciiStrLen (HelpString);
  NewBbsDevPathNode = AllocateZeroPool (sizeof (BBS_BBS_DEVICE_PATH) + StringLen);
  if (NewBbsDevPathNode == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (NewBbsDevPathNode        , CurrentBbsDevPath, sizeof (BBS_BBS_DEVICE_PATH));
  CopyMem (NewBbsDevPathNode->String, HelpString       , StringLen + 1);
  SetDevicePathNodeLength (&(NewBbsDevPathNode->Header), sizeof (BBS_BBS_DEVICE_PATH) + StringLen);

  //
  // Create entire new CurrentBbsDevPath with end node
  //
  CurrentBbsDevPath = AppendDevicePathNode (
                        EndDevicePath,
                        (EFI_DEVICE_PATH_PROTOCOL *) NewBbsDevPathNode
                        );
  if (CurrentBbsDevPath == NULL) {
    FreePool (NewBbsDevPathNode);
    return EFI_OUT_OF_RESOURCES;
  }

  CurrentBbsDevPathSize = (UINT16) (GetDevicePathSize (CurrentBbsDevPath));

  DevicePathProtocolSize   = 0;
  DevicePathProtocol = (EFI_DEVICE_PATH_PROTOCOL *) (UINTN) CurrentBbsEntry->IBV2;
  if (DevicePathProtocol != NULL && IsDevicePathValid (DevicePathProtocol, 0)) {
    DevicePathProtocolSize = GetDevicePathSize (DevicePathProtocol);
  }

  BufferSize = sizeof (UINT32) +
               sizeof (UINT16) +
               StrSize (BootDesc) +
               CurrentBbsDevPathSize +
               sizeof (BBS_TABLE) +
               sizeof (UINT16) +
               DevicePathProtocolSize;

  Buffer = AllocateZeroPool (BufferSize);
  if (Buffer == NULL) {
    FreePool (NewBbsDevPathNode);
    FreePool (CurrentBbsDevPath);
    return EFI_OUT_OF_RESOURCES;
  }

  Ptr               = (UINT8 *) Buffer;

  *((UINT32 *) Ptr) = LOAD_OPTION_ACTIVE;
  Ptr += sizeof (UINT32);

  *((UINT16 *) Ptr) = CurrentBbsDevPathSize;
  Ptr += sizeof (UINT16);

  CopyMem (
    Ptr,
    BootDesc,
    StrSize (BootDesc)
    );
  Ptr += StrSize (BootDesc);

  CopyMem (
    Ptr,
    CurrentBbsDevPath,
    CurrentBbsDevPathSize
    );
  Ptr += CurrentBbsDevPathSize;

  CopyMem (
    Ptr,
    CurrentBbsEntry,
    sizeof (BBS_TABLE)
    );

  Ptr += sizeof (BBS_TABLE);
  *((UINT16 *) Ptr) = (UINT16) Index;

  if (DevicePathProtocolSize != 0) {
    Ptr += sizeof (UINT16);
    CopyMem (
      Ptr,
      DevicePathProtocol,
      DevicePathProtocolSize
      );
  }

  Status = gRT->SetVariable (
                  BootString,
                  &gEfiGlobalVariableGuid,
                  VAR_FLAG,
                  BufferSize,
                  Buffer
                  );
  FreePool (Buffer);
  Buffer = NULL;


  //
  // find the first Legacy BootOption in BootOrder
  //
  Status = gBS->LocateProtocol (
                  &gEfiBootOptionPolicyProtocolGuid,
                  NULL,
                  (VOID **) &BootOptionPolicy
                  );
  if (!EFI_ERROR (Status)) {
    BootOptionType = LEGACY_BOOT_DEV;
    BootOptionPolicy->FindPositionOfNewBootOption (
                        BootOptionPolicy,
                        CurrentBbsDevPath,
                        *BootOrderList,
                        BootOptionType,
                        BootOptionNum,
                        CurrentBootOptionNo,
                        &NewPosition
                        );
  } else {
    NewPosition = 0;
  }

  //
  // Insert new BootOption number to BootOrder
  //
  BdsLibNewBootOptionPolicy (
    &NewBootOrderList,
    *BootOrderList,
    BootOptionNum,
    CurrentBootOptionNo,
    NewPosition
    );

  *BootOrderListSize += sizeof (UINT16);
  SafeFreePool ((VOID **) BootOrderList);
  *BootOrderList = NewBootOrderList;

  FreePool (NewBbsDevPathNode);
  FreePool (CurrentBbsDevPath);

  return Status;
}

/**
 Get device path data from optional data of boot option variable.

 @param[in] BootOptionVar           Pointer to boot option variable
 @param[in] BootOptionVarSize       Size of boot option variable

 @return Pointer to device path protocol or NULL if variable is not support or invalid parameter.
**/
EFI_DEVICE_PATH_PROTOCOL *
GetDevicePathProtocolFromVar (
  IN UINT8                         *BootOptionVar,
  IN UINTN                         BootOptionVarSize
  )
{
  UINT8                     *Ptr;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  UINT16                    DevPathSize;

  if (BootOptionVar == NULL || BootOptionVarSize == 0) {
    return NULL;
  }

  Ptr = BootOptionVar;
  Ptr += sizeof (UINT32);
  DevPathSize = *(UINT16 *) Ptr;
  Ptr += sizeof (UINT16);
  Ptr += StrSize ((UINT16 *) Ptr);
  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) Ptr;

  if ((DevicePath->Type != BBS_DEVICE_PATH) || (DevicePath->SubType != BBS_BBS_DP)) {
    return NULL;
  }

  Ptr += (DevPathSize + sizeof (BBS_TABLE) + sizeof (UINT16));
  if ((UINTN) (Ptr - BootOptionVar) == BootOptionVarSize) {
    return NULL;
  }

  return (EFI_DEVICE_PATH_PROTOCOL *) Ptr;
}

/**
 Get bbs index from bbs table by comparing device path.

 @param[in]  BbsTable                Pointer to bbs table
 @param[in]  BbsCount                Number of bbs table
 @param[in]  DevicePathProtocol      Pointer to device path protocol.
 @param[out] BbsIndex                Pointer to bbs index

 @retval EFI_SUCCESS                 Get bbs index successfully.
 @retval EFI_INVALID_PARAMETER       Bbs table pointer is NULL or device path pointer is NULL.
 @retval EFI_NOT_FOUND               There is no match bbs index.
**/
EFI_STATUS
GetBbsIndexByDevicePathProtocol (
  IN  BBS_TABLE                    *BbsTable,
  IN  UINT16                       BbsCount,
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePathProtocol,
  OUT UINT16                       *BbsIndex
  )
{
  UINT16                           Index;
  UINTN                            DevicePathProtocolSize;
  EFI_DEVICE_PATH_PROTOCOL         *BbsTableDevicePathProtocol;

  if (BbsTable == NULL || DevicePathProtocol == NULL || BbsIndex == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DevicePathProtocolSize = GetDevicePathSize (DevicePathProtocol);

  for (Index = 0; Index < BbsCount; Index++) {
    if ((BbsTable[Index].BootPriority == BBS_IGNORE_ENTRY) ||
        (BbsTable[Index].BootPriority == BBS_DO_NOT_BOOT_FROM)) {
      continue;
    }

    BbsTableDevicePathProtocol = (EFI_DEVICE_PATH_PROTOCOL *) (UINTN) BbsTable[Index].IBV2;

    if (BbsTableDevicePathProtocol != NULL &&
        IsDevicePathValid (BbsTableDevicePathProtocol, 0) &&
        GetDevicePathSize (BbsTableDevicePathProtocol) == DevicePathProtocolSize &&
        CompareMem (BbsTableDevicePathProtocol, DevicePathProtocol, DevicePathProtocolSize) == 0) {
      *BbsIndex = Index;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
 Check if the boot option is a legacy one.

 @param[in]  BootOptionVar   The boot option data payload.
 @param[out] BbsEntry        The BBS Table.
 @param[out] BbsIndex        The table index.

 @retval TRUE           It is a legacy boot option.
 @retval FALSE          It is not a legacy boot option.
**/
BOOLEAN
BdsIsLegacyBootOption (
  IN UINT8                 *BootOptionVar,
  OUT BBS_TABLE            **BbsEntry,
  OUT UINT16               *BbsIndex
  )
{
  UINT8                     *Ptr;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  BOOLEAN                   Ret;
  UINT16                    DevPathLen;

  Ptr = BootOptionVar;
  Ptr += sizeof (UINT32);
  DevPathLen = *(UINT16 *) Ptr;
  Ptr += sizeof (UINT16);
  Ptr += StrSize ((UINT16 *) Ptr);
  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) Ptr;
  if ((BBS_DEVICE_PATH == DevicePath->Type) && (BBS_BBS_DP == DevicePath->SubType)) {
    Ptr += DevPathLen;
    *BbsEntry = (BBS_TABLE *) Ptr;
    Ptr += sizeof (BBS_TABLE);
    *BbsIndex = *(UINT16 *) Ptr;
    Ret       = TRUE;
  } else {
    *BbsEntry = NULL;
    Ret       = FALSE;
  }

  return Ret;
}

/**
 Re-order the Boot Option according to the DevOrder.

 The routine re-orders the Boot Option in BootOption array according to
 the order specified by DevOrder.

 @param BootOption         Pointer to buffer containing the Boot Option Numbers
 @param BootOptionCount    Count of the Boot Option Numbers
 @param DevOrder           Pointer to buffer containing the BBS Index,
                           high 8-bit value 0xFF indicating a disabled boot option
 @param DevOrderCount      Count of the BBS Index
 @param EnBootOption       Pointer to buffer receiving the enabled Boot Option Numbers
 @param EnBootOptionCount  Count of the enabled Boot Option Numbers
 @param DisBootOption      Pointer to buffer receiving the disabled Boot Option Numbers
 @param DisBootOptionCount Count of the disabled Boot Option Numbers
**/
VOID
OrderLegacyBootOption4SameType (
  UINT16                   *BootOption,
  UINTN                    BootOptionCount,
  UINT16                   *DevOrder,
  UINTN                    DevOrderCount,
  UINT16                   *EnBootOption,
  UINTN                    *EnBootOptionCount,
  UINT16                   *DisBootOption,
  UINTN                    *DisBootOptionCount
  )
{
  UINTN                    Index;
  UINTN                    MappingIndex;
  UINT16                   *NewBootOption;
  UINT16                   BbsType;

  *DisBootOptionCount = 0;
  *EnBootOptionCount  = 0;
  BbsType             = 0;

  //
  // Record the corresponding Boot Option Numbers according to the DevOrder
  // Record the EnBootOption and DisBootOption according to the DevOrder
  //
  NewBootOption = AllocatePool (DevOrderCount * sizeof (UINT16));
  ASSERT (NewBootOption != NULL);
  if (NewBootOption == NULL) {
    return;
  }

  while (DevOrderCount-- != 0) {
    for (Index = 0; Index < mBootOptionBbsMappingCount; Index++) {
      if (mBootOptionBbsMapping[Index].BbsIndex == (DevOrder[DevOrderCount] & 0xFF)) {
        BbsType = mBootOptionBbsMapping[Index].BbsType;
        NewBootOption[DevOrderCount] = mBootOptionBbsMapping[Index].BootOptionNumber;

        if ((DevOrder[DevOrderCount] & 0xFF00) == 0xFF00) {
          DisBootOption[*DisBootOptionCount] = NewBootOption[DevOrderCount];
          (*DisBootOptionCount)++;
        } else {
          EnBootOption[*EnBootOptionCount] = NewBootOption[DevOrderCount];
          (*EnBootOptionCount)++;
        }
        break;
      }
    }
  }

  for (Index = 0; Index < BootOptionCount; Index++) {
    //
    // Find the start position for the BbsType in BootOption
    //
    for (MappingIndex = 0; MappingIndex < mBootOptionBbsMappingCount; MappingIndex++) {
      if (mBootOptionBbsMapping[MappingIndex].BbsType == BbsType && mBootOptionBbsMapping[MappingIndex].BootOptionNumber == BootOption[Index]) {
        break;
      }
    }

    //
    // Overwrite the old BootOption
    //
    if (MappingIndex < mBootOptionBbsMappingCount) {
      CopyMem (&BootOption[Index], NewBootOption, (*DisBootOptionCount + *EnBootOptionCount) * sizeof (UINT16));
      break;
    }
  }
}

/**
 Group the legacy boot options in the BootOption.

 The routine assumes the boot options in the beginning that covers all the device
 types are ordered properly and re-position the following boot options just after
 the corresponding boot options with the same device type.
 For example:
 1. Input  = [Harddisk1 CdRom2 Efi1 Harddisk0 CdRom0 CdRom1 Harddisk2 Efi0]
    Assuming [Harddisk1 CdRom2 Efi1] is ordered properly
    Output = [Harddisk1 Harddisk0 Harddisk2 CdRom2 CdRom0 CdRom1 Efi1 Efi0]

 2. Input  = [Efi1 Efi0 CdRom1 Harddisk0 Harddisk1 Harddisk2 CdRom0 CdRom2]
    Assuming [Efi1 Efi0 CdRom1 Harddisk0] is ordered properly
    Output = [Efi1 Efi0 CdRom1 CdRom0 CdRom2 Harddisk0 Harddisk1 Harddisk2]

 @param BootOption      Pointer to buffer containing Boot Option Numbers
 @param BootOptionCount Count of the Boot Option Numbers
**/
VOID
GroupMultipleLegacyBootOption4SameType (
  UINT16                   *BootOption,
  UINTN                    BootOptionCount
  )
{
  UINTN                    DeviceTypeIndex[7];
  UINTN                    Index;
  UINTN                    MappingIndex;
  UINTN                    *NextIndex;
  UINT16                   OptionNumber;
  UINTN                    DeviceIndex;
  UINTN                    ArrayIndex;

  SetMem (DeviceTypeIndex, sizeof (DeviceTypeIndex), 0xFF);

  for (Index = 0; Index < BootOptionCount; Index++) {
    //
    // Find the DeviceType
    //
    for (MappingIndex = 0; MappingIndex < mBootOptionBbsMappingCount; MappingIndex++) {
      if (mBootOptionBbsMapping[MappingIndex].BootOptionNumber == BootOption[Index]) {
        break;
      }
    }
    if (MappingIndex == mBootOptionBbsMappingCount) {
      //
      // Is not a legacy boot option
      //
      continue;
    }

    ArrayIndex = (mBootOptionBbsMapping[MappingIndex].BbsType & 0xF);
    ASSERT ((mBootOptionBbsMapping[MappingIndex].BbsType & 0xF) <
             sizeof (DeviceTypeIndex) / sizeof (DeviceTypeIndex[0]));
    if (ArrayIndex >= (sizeof (DeviceTypeIndex) / sizeof (DeviceTypeIndex[0]))) {
      return;
    }

    NextIndex = &DeviceTypeIndex[ArrayIndex];
    if (*NextIndex == (UINTN) -1) {
      //
      // *NextIndex is the index in BootOption to put the next Option Number for the same type
      //
      *NextIndex = Index + 1;
    } else {
      //
      // insert the current boot option before *NextIndex, causing [*Next .. Index] shift right one position
      //
      OptionNumber = BootOption[Index];
      CopyMem (&BootOption[*NextIndex + 1], &BootOption[*NextIndex], (Index - *NextIndex) * sizeof (UINT16));
      BootOption[*NextIndex] = OptionNumber;

      //
      // Update the DeviceTypeIndex array to reflect the right shift operation
      //
      for (DeviceIndex = 0; DeviceIndex < sizeof (DeviceTypeIndex) / sizeof (DeviceTypeIndex[0]); DeviceIndex++) {
        if (DeviceTypeIndex[DeviceIndex] != (UINTN) -1 && DeviceTypeIndex[DeviceIndex] >= *NextIndex) {
          DeviceTypeIndex[DeviceIndex]++;
        }
      }
    }
  }
}

/**
 Update boot option variable for legacy boot option.

 @param[in] BbsItem         Pointer to BBS table.
 @param[in] BootDesc        Pointer to boot device description.
 @param[in] BootString      Pointer to boot option variable string.
 @param[in] TempIndex       BBS table index.

 @retval EFI_SUCCESS            Update legacy boot option variable success.
 @retval EFI_OUT_OF_RESOURCES   Allocate memory fail.
**/
EFI_STATUS
UpdateBootVar (
  IN BBS_TABLE              *BbsItem,
  IN CHAR16                 *BootDesc,
  IN CHAR16                 *BootString,
  IN UINTN                  TempIndex
  )
{
  BBS_BBS_DEVICE_PATH       BbsDevPathNode;
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  UINT16                    CurrentBbsDevPathSize;
  UINTN                     BufferSize;
  VOID                      *Buffer;
  UINT8                     *Ptr;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathProtocol;
  UINTN                     DevicePathProtocolSize;

  DevPath                       = NULL;

  BbsDevPathNode.Header.Type    = BBS_DEVICE_PATH;
  BbsDevPathNode.Header.SubType = BBS_BBS_DP;
  SetDevicePathNodeLength (&BbsDevPathNode.Header, sizeof (BBS_BBS_DEVICE_PATH));
  BbsDevPathNode.DeviceType = BbsItem->DeviceType;
  CopyMem (&BbsDevPathNode.StatusFlag, &BbsItem->StatusFlags, sizeof (UINT16));

  DevicePathProtocolSize = 0;
  DevicePathProtocol = (EFI_DEVICE_PATH_PROTOCOL *) (UINTN) BbsItem->IBV2;
  if (DevicePathProtocol != NULL && IsDevicePathValid (DevicePathProtocol, 0)) {
    DevicePathProtocolSize = GetDevicePathSize (DevicePathProtocol);
  }

  DevPath = AppendDevicePathNode (
              EndDevicePath,
              (EFI_DEVICE_PATH_PROTOCOL *) &BbsDevPathNode
              );
  if (NULL == DevPath) {
    return EFI_OUT_OF_RESOURCES;
  }

  CurrentBbsDevPathSize = (UINT16) (GetDevicePathSize (DevPath));

  BufferSize = sizeof (UINT32) +
               sizeof (UINT16) +
               StrSize (BootDesc) +
               CurrentBbsDevPathSize +
               sizeof (BBS_TABLE) +
               sizeof (UINT16) +
               DevicePathProtocolSize;

  Buffer = AllocateZeroPool (BufferSize);
  if (Buffer == NULL) {
    FreePool (DevPath);
    return EFI_OUT_OF_RESOURCES;
  }

  Ptr               = (UINT8 *) Buffer;

  *((UINT32 *) Ptr) = LOAD_OPTION_ACTIVE;
  Ptr += sizeof (UINT32);

  *((UINT16 *) Ptr) = CurrentBbsDevPathSize;
  Ptr += sizeof (UINT16);

  CopyMem (
    Ptr,
    BootDesc,
    StrSize (BootDesc)
    );
  Ptr += StrSize (BootDesc);

  CopyMem (
    Ptr,
    DevPath,
    CurrentBbsDevPathSize
    );
  Ptr += CurrentBbsDevPathSize;

  CopyMem (
    Ptr,
    BbsItem,
    sizeof (BBS_TABLE)
    );

  Ptr += sizeof (BBS_TABLE);
  *((UINT16 *) Ptr) = (UINT16) TempIndex;

  if (DevicePathProtocolSize != 0) {
    Ptr += sizeof (UINT16);
    CopyMem (
      Ptr,
      DevicePathProtocol,
      DevicePathProtocolSize
      );
  }

  Status = gRT->SetVariable (
                  BootString,
                  &gEfiGlobalVariableGuid,
                  VAR_FLAG,
                  BufferSize,
                  Buffer
                  );

  FreePool (DevPath);
  FreePool (Buffer);

  return Status;
}

/**
 Delete all the invalid legacy boot options.

 @retval EFI_SUCCESS             All invalide legacy boot options are deleted.
 @retval EFI_OUT_OF_RESOURCES    Fail to allocate necessary memory.
 @retval EFI_NOT_FOUND           Fail to retrive variable of boot order.
**/
EFI_STATUS
EFIAPI
BdsDeleteAllInvalidLegacyBootOptions (
  VOID
  )
{
  UINT16                    *BootOrder;
  UINT8                     *BootOptionVar;
  UINTN                     BootOrderSize;
  UINTN                     BootOptionSize;
  EFI_STATUS                Status;
  UINT16                    HddCount;
  UINT16                    BbsCount;
  HDD_INFO                  *LocalHddInfo;
  BBS_TABLE                 *LocalBbsTable;
  BBS_TABLE                 *BbsEntry;
  UINT16                    BbsIndex;
  EFI_LEGACY_BIOS_PROTOCOL  *LegacyBios;
  UINTN                     Index;
  UINT16                    BootOption[10];
  UINT16                    BootDesc[100];
  BOOLEAN                   DescStringMatch;
  BOOLEAN                   BbsEntryMatch;
  KERNEL_CONFIGURATION      SystemConfiguration;
  EFI_STATUS                SystemConfigStatus;
  UINTN                     Index2;
  UINTN                     Size;
  OPROM_STORAGE_DEVICE_INFO *OpromStorageDev;
  UINTN                     OpromStorageDevCount;
  UINT8                     *DisableOpromStorageDevBoot;
  BOOLEAN                   DeleteBootOption;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathProtocol;
  UINT16                    CurrentBbsIndex;
  BOOLEAN                   BbsIndexMatch;


  Status        = EFI_SUCCESS;
  BootOrder     = NULL;
  BootOrderSize = 0;
  HddCount      = 0;
  BbsCount      = 0;
  LocalHddInfo  = NULL;
  LocalBbsTable = NULL;
  BbsEntry      = NULL;

  SystemConfigStatus = GetKernelConfiguration (&SystemConfiguration);

  Status = EfiLibLocateProtocol (&gEfiLegacyBiosProtocolGuid, (VOID **) &LegacyBios);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  LegacyBios->GetBbsInfo (
                LegacyBios,
                &HddCount,
                &LocalHddInfo,
                &BbsCount,
                &LocalBbsTable
                );

  BootOrder = BdsLibGetVariableAndSize (
                L"BootOrder",
                &gEfiGlobalVariableGuid,
                &BootOrderSize
                );
  if (BootOrder == NULL) {
    BootOrderSize = 0;
  }

  //
  // If DisableOpromStorageDevBoot variable exist, get Oprom info then delete Oprom storage device.
  //
  OpromStorageDev = NULL;
  OpromStorageDevCount = 0;
  DisableOpromStorageDevBoot = BdsLibGetVariableAndSize (
                                 L"DisableOpromStorageDevBoot",
                                 &gEfiGenericVariableGuid,
                                 &Size
                                 );
  if (DisableOpromStorageDevBoot != NULL) {
    BdsLibGetOpromStorageDevInfo (&OpromStorageDev, &OpromStorageDevCount);
  }

  Index = 0;
  while (Index < BootOrderSize / sizeof (UINT16)) {
    UnicodeSPrint (BootOption, sizeof (BootOption), L"Boot%04x", BootOrder[Index]);
    BootOptionVar = BdsLibGetVariableAndSize (
                      BootOption,
                      &gEfiGlobalVariableGuid,
                      &BootOptionSize
                      );
    if (NULL == BootOptionVar) {
      BootOptionSize = 0;
      Status = gRT->GetVariable (
                      BootOption,
                      &gEfiGlobalVariableGuid,
                      NULL,
                      &BootOptionSize,
                      BootOptionVar
                      );
      if (Status == EFI_NOT_FOUND) {
        //
        // Update BootOrder
        //
        BdsLibDeleteBootOption (
          BootOrder[Index],
          BootOrder,
          &BootOrderSize
          );
        continue;
      } else {
        FreePool (BootOrder);
        return EFI_OUT_OF_RESOURCES;
      }
    }

    //
    // Skip Non-Legacy boot option
    //
    if (!BdsIsLegacyBootOption (BootOptionVar, &BbsEntry, &BbsIndex)) {
      if (BootOptionVar != NULL) {
        FreePool (BootOptionVar);
      }
      Index++;
      continue;
    }

    DeleteBootOption = FALSE;

    if (BbsIndex >= BbsCount) {
      DeleteBootOption = TRUE;
    }

    if (BbsEntry->Class == PCI_CLASS_NETWORK &&
        BbsEntry->SubClass == PCI_CLASS_NETWORK_ETHERNET &&
        !EFI_ERROR (SystemConfigStatus) &&
        SystemConfiguration.PxeBootToLan == 0) {
      DeleteBootOption = TRUE;
    }

    if (DisableOpromStorageDevBoot != NULL &&
        BbsEntry->Class == PCI_CLASS_MASS_STORAGE) {
      for (Index2 = 0; Index2 < OpromStorageDevCount; Index2++) {
        if (BbsEntry->Bus      == (UINT32) OpromStorageDev[Index2].Bus &&
            BbsEntry->Device   == (UINT32) OpromStorageDev[Index2].Device &&
            BbsEntry->Function == (UINT32) OpromStorageDev[Index2].Function) {
          break;
        }
      }

      if (Index2 < OpromStorageDevCount) {
        DeleteBootOption = TRUE;
      }
    }

    if (DeleteBootOption) {
      SafeFreePool ((VOID **) &BootOptionVar);
      BdsLibDeleteBootOption (
        BootOrder[Index],
        BootOrder,
        &BootOrderSize
        );
      continue;
    }

    //
    // Check if BBS Description String is changed
    //
    DescStringMatch = FALSE;
    BbsEntryMatch = FALSE;
    BbsIndexMatch = TRUE;

    DevicePathProtocol = GetDevicePathProtocolFromVar (BootOptionVar, BootOptionSize);
    if (DevicePathProtocol != NULL) {
      //
      // Get correct bbs Index by comparing device path to prevent two devices have the same boot description strings.
      //
      Status = GetBbsIndexByDevicePathProtocol (LocalBbsTable, BbsCount, DevicePathProtocol, &CurrentBbsIndex);
      if (!EFI_ERROR(Status)) {
        if (CurrentBbsIndex != BbsIndex) {
          BbsIndexMatch = FALSE;
          BbsIndex = CurrentBbsIndex;
        }
      } else {
        SafeFreePool ((VOID **) &BootOptionVar);
        BdsLibDeleteBootOption (
          BootOrder[Index],
          BootOrder,
          &BootOrderSize
          );
        continue;
      }
    }

    BdsBuildLegacyDevNameString (
      &LocalBbsTable[BbsIndex],
      LocalHddInfo,
      BbsIndex,
      sizeof (BootDesc),
      BootDesc
      );

    if (StrCmp (BootDesc, (UINT16*)(BootOptionVar + sizeof (UINT32) + sizeof (UINT16))) == 0) {
      DescStringMatch = TRUE;
    }
    //
    // Skip check IBV2 of bba table because it is a memory address which may be change every boot.
    //
    if (CompareMem (&LocalBbsTable[BbsIndex], BbsEntry, sizeof (BBS_TABLE) - sizeof(UINT32)) == 0) {
      BbsEntryMatch = TRUE;
    }

    if (!((LocalBbsTable[BbsIndex].BootPriority == BBS_IGNORE_ENTRY) ||
          (LocalBbsTable[BbsIndex].BootPriority == BBS_DO_NOT_BOOT_FROM) ||
          (LocalBbsTable[BbsIndex].BootPriority == BBS_LOWEST_PRIORITY)) &&
        (LocalBbsTable[BbsIndex].DeviceType == BbsEntry->DeviceType) &&
        DescStringMatch &&
        BbsEntryMatch &&
        BbsIndexMatch) {
      Index++;
      continue;
    }

    //
    // If only the content of the BBS_TABLE is different, just need update current Bootxxxx variable
    //
    if (!((LocalBbsTable[BbsIndex].BootPriority == BBS_IGNORE_ENTRY) ||
          (LocalBbsTable[BbsIndex].BootPriority == BBS_DO_NOT_BOOT_FROM) ||
          (LocalBbsTable[BbsIndex].BootPriority == BBS_LOWEST_PRIORITY)) &&
        (LocalBbsTable[BbsIndex].DeviceType == BbsEntry->DeviceType) &&
        DescStringMatch &&
        (!BbsEntryMatch || !BbsIndexMatch)) {
      BdsBuildLegacyDevNameString (
        &LocalBbsTable[BbsIndex],
        LocalHddInfo,
        BbsIndex,
        sizeof(BootDesc),
        BootDesc
        );
      UpdateBootVar (&LocalBbsTable[BbsIndex], BootDesc, BootOption, BbsIndex);
      SafeFreePool ((VOID **) &BootOptionVar);
      Index++;
      continue;
    }

    SafeFreePool ((VOID **) &BootOptionVar);

    BdsLibDeleteBootOption (
      BootOrder[Index],
      BootOrder,
      &BootOrderSize
      );
  }

  //
  // Adjust the number of boot options.
  //
  if (BootOrderSize != 0) {
    Status = gRT->SetVariable (
                    L"BootOrder",
                    &gEfiGlobalVariableGuid,
                    VAR_FLAG,
                    BootOrderSize,
                    BootOrder
                    );
  } else {
    EfiLibDeleteVariable (L"BootOrder", &gEfiGlobalVariableGuid);
  }

  if (BootOrder != NULL) {
    FreePool (BootOrder);
  }
  SafeFreePool ((VOID **) &DisableOpromStorageDevBoot);
  SafeFreePool ((VOID **) &OpromStorageDev);

  return Status;
}

/**
 Find all legacy boot option by device type.

 @param[in]  BootOrder       The boot order array.
 @param[in]  BootOptionNum   The number of boot option.
 @param[in]  DevType         Device type.
 @param[in]  DevName         Device name.
 @param[out] Attribute       The boot option attribute.
 @param[out] BbsIndex        The BBS table index.
 @param[out] OptionNumber    The boot option index.

 @retval TRUE           The Legacy boot option is found.
 @retval FALSE          The legacy boot option is not found.
**/
BOOLEAN
BdsFindLegacyBootOptionByDevTypeAndName (
  IN UINT16                   *BootOrder,
  IN UINTN                    BootOptionNum,
  IN UINT16                   DevType,
  IN CHAR16                   *DevName,
  IN UINT16                   DevIndex,
  OUT UINT32                  *Attribute,
  OUT UINT16                  *BbsIndex,
  OUT UINT16                  *OptionNumber
  )
{
  UINTN     Index;
  CHAR16    BootOption[9];
  UINTN     BootOptionSize;
  UINT8     *BootOptionVar;
  BBS_TABLE *BbsEntry;
  BOOLEAN   Found;

  BbsEntry  = NULL;
  Found     = FALSE;

  if (NULL == BootOrder) {
    return Found;
  }

  //
  // Loop all boot option from variable
  //
  for (Index = 0; Index < BootOptionNum; Index++) {
    UnicodeSPrint (BootOption, sizeof (BootOption), L"Boot%04x", (UINTN) BootOrder[Index]);
    BootOptionVar = BdsLibGetVariableAndSize (
                      BootOption,
                      &gEfiGlobalVariableGuid,
                      &BootOptionSize
                      );
    if (NULL == BootOptionVar) {
      continue;
    }

    //
    // Skip Non-legacy boot option
    //
    if (!BdsIsLegacyBootOption (BootOptionVar, &BbsEntry, BbsIndex)) {
      FreePool (BootOptionVar);
      continue;
    }

    if (*BbsIndex != DevIndex ||
        (BbsEntry->DeviceType != DevType) ||
        (StrCmp (DevName, (CHAR16*)(BootOptionVar + sizeof (UINT32) + sizeof (UINT16))) != 0)
       ) {
      FreePool (BootOptionVar);
      continue;
    }

    *Attribute    = *(UINT32 *) BootOptionVar;
    *OptionNumber = BootOrder[Index];
    Found         = TRUE;
    FreePool (BootOptionVar);
    break;
  }

  return Found;
}

/**
 Create a legacy boot option.

 @param[in]      BbsItem           The BBS Table entry.
 @param[in]      Index             Index of the specified entry in BBS table.
 @param[in, out] BootOrderList     The boot order list.
 @param[in, out] BootOrderListSize The size of boot order list.

 @retval EFI_SUCCESS          The function complete successfully.
 @retval EFI_OUT_OF_RESOURCE  No enough memory.
 @return Other value if the legacy boot option is not created.
**/
EFI_STATUS
BdsCreateOneLegacyBootOption (
  IN BBS_TABLE              *BbsItem,
  IN UINTN                  Index,
  IN OUT UINT16             **BootOrderList,
  IN OUT UINTN              *BootOrderListSize
  )
{
  BBS_BBS_DEVICE_PATH       BbsDevPathNode;
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;

  DevPath                       = NULL;

  //
  // Create device path node.
  //
  BbsDevPathNode.Header.Type    = BBS_DEVICE_PATH;
  BbsDevPathNode.Header.SubType = BBS_BBS_DP;
  SetDevicePathNodeLength (&BbsDevPathNode.Header, sizeof (BBS_BBS_DEVICE_PATH));
  BbsDevPathNode.DeviceType = BbsItem->DeviceType;
  CopyMem (&BbsDevPathNode.StatusFlag, &BbsItem->StatusFlags, sizeof (UINT16));

  DevPath = AppendDevicePathNode (
              EndDevicePath,
              (EFI_DEVICE_PATH_PROTOCOL *) &BbsDevPathNode
              );
  if (NULL == DevPath) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = BdsCreateLegacyBootOption (
            BbsItem,
            DevPath,
            Index,
            BootOrderList,
            BootOrderListSize
            );
  BbsItem->BootPriority = (UINT16) (*BootOrderListSize / sizeof(UINT16));

  FreePool (DevPath);

  return Status;
}

/**
 Add the legacy boot options from BBS table if they do not exist.

 @retval EFI_SUCCESS          The boot options are added successfully
                              or they are already in boot options.
 @retval EFI_NOT_FOUND        No legacy boot options is found.
 @retval EFI_OUT_OF_RESOURCE  No enough memory.
 @return Other value          LegacyBoot options are not added.
**/
EFI_STATUS
EFIAPI
BdsAddNonExistingLegacyBootOptions (
  VOID
  )
{
  UINT16                    *BootOrder;
  UINTN                     BootOrderSize;
  EFI_STATUS                Status;
  CHAR16                    Desc[100];
  UINT16                    HddCount;
  UINT16                    BbsCount;
  HDD_INFO                  *LocalHddInfo;
  BBS_TABLE                 *LocalBbsTable;
  UINT16                    BbsIndex;
  EFI_LEGACY_BIOS_PROTOCOL  *LegacyBios;
  UINT16                    Index;
  UINT32                    Attribute;
  UINT16                    OptionNumber;
  BOOLEAN                   Exist;
  KERNEL_CONFIGURATION      SystemConfiguration;
  EFI_STATUS                SystemConfigStatus;
  OPROM_STORAGE_DEVICE_INFO *OpromStorageDev;
  UINTN                     OpromStorageDevCount;
  UINT8                     *DisableOpromStorageDevBoot;
  UINTN                     Size;
  UINTN                     Index2;

  SystemConfigStatus = GetKernelConfiguration (&SystemConfiguration);

  HddCount      = 0;
  BbsCount      = 0;
  LocalHddInfo  = NULL;
  LocalBbsTable = NULL;

  Status        = EfiLibLocateProtocol (&gEfiLegacyBiosProtocolGuid, (VOID **) &LegacyBios);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  LegacyBios->GetBbsInfo (
                LegacyBios,
                &HddCount,
                &LocalHddInfo,
                &BbsCount,
                &LocalBbsTable
                );

  BootOrder = BdsLibGetVariableAndSize (
                L"BootOrder",
                &gEfiGlobalVariableGuid,
                &BootOrderSize
                );
  if (BootOrder == NULL) {
    BootOrderSize = 0;
  }

  OpromStorageDev = NULL;
  OpromStorageDevCount = 0;
  DisableOpromStorageDevBoot = BdsLibGetVariableAndSize (
                                 L"DisableOpromStorageDevBoot",
                                 &gEfiGenericVariableGuid,
                                 &Size
                                 );
  if (DisableOpromStorageDevBoot != NULL) {
    BdsLibGetOpromStorageDevInfo (&OpromStorageDev, &OpromStorageDevCount);
  }

  for (Index = 0; Index < BbsCount; Index++) {
    if ((LocalBbsTable[Index].BootPriority == BBS_IGNORE_ENTRY) ||
        (LocalBbsTable[Index].BootPriority == BBS_DO_NOT_BOOT_FROM) ||
        (LocalBbsTable[Index].BootPriority == BBS_LOWEST_PRIORITY)) {
      continue;
    }

    BdsBuildLegacyDevNameString (&LocalBbsTable[Index], LocalHddInfo,Index, sizeof (Desc), Desc);

    Exist = BdsFindLegacyBootOptionByDevTypeAndName (
              BootOrder,
              BootOrderSize / sizeof (UINT16),
              LocalBbsTable[Index].DeviceType,
              Desc,
              Index,
              &Attribute,
              &BbsIndex,
              &OptionNumber
              );
    if (Exist && (Attribute & LOAD_OPTION_ACTIVE) != 0) {
      continue;
    }

    if (Exist) {
      if (Index != BbsIndex) {
        BdsLibDeleteBootOption (
          OptionNumber,
          BootOrder,
          &BootOrderSize
          );
      } else {
        continue;
      }
    }

    if (LocalBbsTable[Index].Class == PCI_CLASS_NETWORK &&
        LocalBbsTable[Index].SubClass == PCI_CLASS_NETWORK_ETHERNET &&
        !EFI_ERROR (SystemConfigStatus) &&
        SystemConfiguration.PxeBootToLan == 0) {
      continue;
    }

    if (DisableOpromStorageDevBoot != NULL &&
        LocalBbsTable[Index].Class == PCI_CLASS_MASS_STORAGE) {
      for (Index2 = 0; Index2 < OpromStorageDevCount; Index2++) {
        if (LocalBbsTable[Index].Bus      == (UINT32) OpromStorageDev[Index2].Bus &&
            LocalBbsTable[Index].Device   == (UINT32) OpromStorageDev[Index2].Device &&
            LocalBbsTable[Index].Function == (UINT32) OpromStorageDev[Index2].Function) {
          break;
        }
      }

      if (Index2 < OpromStorageDevCount) {
        continue;
      }
    }

    Status = BdsCreateOneLegacyBootOption (
              &LocalBbsTable[Index],
              Index,
              &BootOrder,
              &BootOrderSize
              );
    if (EFI_ERROR (Status)) {
      break;
    }
  }

  if (BootOrderSize > 0) {
    Status = gRT->SetVariable (
                    L"BootOrder",
                    &gEfiGlobalVariableGuid,
                    VAR_FLAG,
                    BootOrderSize,
                    BootOrder
                    );
  } else {
    EfiLibDeleteVariable (L"BootOrder", &gEfiGlobalVariableGuid);
  }

  if (BootOrder != NULL) {
    FreePool (BootOrder);
  }

  SafeFreePool ((VOID **) &DisableOpromStorageDevBoot);
  SafeFreePool ((VOID **) &OpromStorageDev);

  return Status;
}

/**
 Fill the device order buffer.

 @param[in]  BbsTable        The BBS table.
 @param[in]  BbsType         The BBS Type.
 @param[in]  BbsCount        The BBS Count.
 @param[out] Buf             device order buffer.

 @return The device order buffer.
**/
UINT16 *
BdsFillDevOrderBuf (
  IN BBS_TABLE                    *BbsTable,
  IN BBS_TYPE                     BbsType,
  IN UINTN                        BbsCount,
  OUT UINT16                      *Buf
  )
{
  UINTN Index;

  for (Index = 0; Index < BbsCount; Index++) {
    if (BbsTable[Index].BootPriority == BBS_IGNORE_ENTRY) {
      continue;
    }

    if (BbsTable[Index].DeviceType != BbsType) {
      continue;
    }

    *Buf = (UINT16) (Index & 0xFF);
    Buf++;
  }

  return Buf;
}

/**
 Create the device order buffer.

 @param[in] BbsTable        The BBS table.
 @param[in] BbsCount        The BBS Count.

 @retval EFI_SUCCES             The buffer is created and the EFI variable named
                                VAR_LEGACY_DEV_ORDER and gEfiLegacyDevOrderVariableGuid is
                                set correctly.
 @retval EFI_OUT_OF_RESOURCES   Memmory or storage is not enough.
 @retval EFI_DEVICE_ERROR       Fail to add the device order into EFI variable fail
                                because of hardware error.
**/
EFI_STATUS
BdsCreateDevOrder (
  IN BBS_TABLE                  *BbsTable,
  IN UINT16                     BbsCount
  )
{
  UINTN                       Index;
  UINTN                       FDCount;
  UINTN                       HDCount;
  UINTN                       CDCount;
  UINTN                       NETCount;
  UINTN                       BEVCount;
  UINTN                       TotalSize;
  UINTN                       HeaderSize;
  LEGACY_DEV_ORDER_ENTRY      *DevOrder;
  LEGACY_DEV_ORDER_ENTRY      *DevOrderPtr;
  EFI_STATUS                  Status;

  FDCount     = 0;
  HDCount     = 0;
  CDCount     = 0;
  NETCount    = 0;
  BEVCount    = 0;
  TotalSize   = 0;
  HeaderSize  = sizeof (BBS_TYPE) + sizeof (UINT16);
  DevOrder    = NULL;
  Status      = EFI_SUCCESS;

  //
  // Count all boot devices
  //
  for (Index = 0; Index < BbsCount; Index++) {
    if (BbsTable[Index].BootPriority == BBS_IGNORE_ENTRY) {
      continue;
    }

    switch (BbsTable[Index].DeviceType) {
    case BBS_FLOPPY:
      FDCount++;
      break;

    case BBS_HARDDISK:
      HDCount++;
      break;

    case BBS_CDROM:
      CDCount++;
      break;

    case BBS_EMBED_NETWORK:
      NETCount++;
      break;

    case BBS_BEV_DEVICE:
      BEVCount++;
      break;

    default:
      break;
    }
  }

  TotalSize += (HeaderSize + sizeof (UINT16) * FDCount);
  TotalSize += (HeaderSize + sizeof (UINT16) * HDCount);
  TotalSize += (HeaderSize + sizeof (UINT16) * CDCount);
  TotalSize += (HeaderSize + sizeof (UINT16) * NETCount);
  TotalSize += (HeaderSize + sizeof (UINT16) * BEVCount);

  //
  // Create buffer to hold all boot device order
  //
  DevOrder = (LEGACY_DEV_ORDER_ENTRY *) (UINTN) AllocateZeroPool (TotalSize);
  if (NULL == DevOrder) {
    return EFI_OUT_OF_RESOURCES;
  }
  DevOrderPtr          = DevOrder;

  DevOrderPtr->BbsType = BBS_FLOPPY;
  DevOrderPtr->Length  = (UINT16) (sizeof (DevOrderPtr->Length) + FDCount * sizeof (UINT16));
  DevOrderPtr          = (LEGACY_DEV_ORDER_ENTRY *) BdsFillDevOrderBuf (BbsTable, BBS_FLOPPY, BbsCount, DevOrderPtr->Data);

  DevOrderPtr->BbsType = BBS_HARDDISK;
  DevOrderPtr->Length  = (UINT16) (sizeof (UINT16) + HDCount * sizeof (UINT16));
  DevOrderPtr          = (LEGACY_DEV_ORDER_ENTRY *) BdsFillDevOrderBuf (BbsTable, BBS_HARDDISK, BbsCount, DevOrderPtr->Data);

  DevOrderPtr->BbsType = BBS_CDROM;
  DevOrderPtr->Length  = (UINT16) (sizeof (UINT16) + CDCount * sizeof (UINT16));
  DevOrderPtr          = (LEGACY_DEV_ORDER_ENTRY *) BdsFillDevOrderBuf (BbsTable, BBS_CDROM, BbsCount, DevOrderPtr->Data);

  DevOrderPtr->BbsType = BBS_EMBED_NETWORK;
  DevOrderPtr->Length  = (UINT16) (sizeof (UINT16) + NETCount * sizeof (UINT16));
  DevOrderPtr          = (LEGACY_DEV_ORDER_ENTRY *) BdsFillDevOrderBuf (BbsTable, BBS_EMBED_NETWORK, BbsCount, DevOrderPtr->Data);

  DevOrderPtr->BbsType = BBS_BEV_DEVICE;
  DevOrderPtr->Length  = (UINT16) (sizeof (UINT16) + BEVCount * sizeof (UINT16));
  DevOrderPtr          = (LEGACY_DEV_ORDER_ENTRY *) BdsFillDevOrderBuf (BbsTable, BBS_BEV_DEVICE, BbsCount, DevOrderPtr->Data);

  ASSERT (TotalSize == (UINTN) ((UINT8 *) DevOrderPtr - (UINT8 *) DevOrder));

  //
  // Save device order for legacy boot device to variable.
  //
  Status = gRT->SetVariable (
                  VAR_LEGACY_DEV_ORDER,
                  &gEfiLegacyDevOrderVariableGuid,
                  VAR_FLAG,
                  TotalSize,
                  DevOrder
                  );
  FreePool (DevOrder);

  return Status;
}

/**
 Add the legacy boot devices from BBS table into
 the legacy device boot order.

 @retval EFI_SUCCESS           The boot devices are added successfully.
 @retval EFI_NOT_FOUND         The legacy boot devices are not found.
 @retval EFI_OUT_OF_RESOURCES  Memmory or storage is not enough.
 @retval EFI_DEVICE_ERROR      Fail to add the legacy device boot order into EFI variable
                               because of hardware error.
**/
EFI_STATUS
EFIAPI
BdsUpdateLegacyDevOrder (
  VOID
  )
{
  LEGACY_DEV_ORDER_ENTRY      *DevOrder;
  LEGACY_DEV_ORDER_ENTRY      *NewDevOrder;
  LEGACY_DEV_ORDER_ENTRY      *Ptr;
  LEGACY_DEV_ORDER_ENTRY      *NewPtr;
  UINTN                       DevOrderSize;
  EFI_LEGACY_BIOS_PROTOCOL    *LegacyBios;
  EFI_STATUS                  Status;
  UINT16                      HddCount;
  UINT16                      BbsCount;
  HDD_INFO                    *LocalHddInfo;
  BBS_TABLE                   *LocalBbsTable;
  UINTN                       Index;
  UINTN                       Index2;
  UINTN                       *Idx;
  UINTN                       FDCount;
  UINTN                       HDCount;
  UINTN                       CDCount;
  UINTN                       NETCount;
  UINTN                       BEVCount;
  UINTN                       TotalSize;
  UINTN                       HeaderSize;
  UINT16                      *NewFDPtr;
  UINT16                      *NewHDPtr;
  UINT16                      *NewCDPtr;
  UINT16                      *NewNETPtr;
  UINT16                      *NewBEVPtr;
  UINT16                      *NewDevPtr;
  UINTN                       FDIndex;
  UINTN                       HDIndex;
  UINTN                       CDIndex;
  UINTN                       NETIndex;
  UINTN                       BEVIndex;

  Idx           = NULL;
  FDCount       = 0;
  HDCount       = 0;
  CDCount       = 0;
  NETCount      = 0;
  BEVCount      = 0;
  TotalSize     = 0;
  HeaderSize    = sizeof (BBS_TYPE) + sizeof (UINT16);
  FDIndex       = 0;
  HDIndex       = 0;
  CDIndex       = 0;
  NETIndex      = 0;
  BEVIndex      = 0;
  NewDevPtr     = NULL;

  Status        = EfiLibLocateProtocol (&gEfiLegacyBiosProtocolGuid, (VOID **) &LegacyBios);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = LegacyBios->GetBbsInfo (
                         LegacyBios,
                         &HddCount,
                         &LocalHddInfo,
                         &BbsCount,
                         &LocalBbsTable
                         );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DevOrder = BdsLibGetVariableAndSize (
               VAR_LEGACY_DEV_ORDER,
               &gEfiLegacyDevOrderVariableGuid,
               &DevOrderSize
               );
  if (NULL == DevOrder) {
    return BdsCreateDevOrder (LocalBbsTable, BbsCount);
  }
  //
  // First we figure out how many boot devices with same device type respectively
  //
  for (Index = 0; Index < BbsCount; Index++) {
    if ((LocalBbsTable[Index].BootPriority == BBS_IGNORE_ENTRY) ||
        (LocalBbsTable[Index].BootPriority == BBS_DO_NOT_BOOT_FROM) ||
        (LocalBbsTable[Index].BootPriority == BBS_LOWEST_PRIORITY)
        ) {
      continue;
    }

    switch (LocalBbsTable[Index].DeviceType) {
    case BBS_FLOPPY:
      FDCount++;
      break;

    case BBS_HARDDISK:
      HDCount++;
      break;

    case BBS_CDROM:
      CDCount++;
      break;

    case BBS_EMBED_NETWORK:
      NETCount++;
      break;

    case BBS_BEV_DEVICE:
      BEVCount++;
      break;

    default:
      break;
    }
  }

  TotalSize += (HeaderSize + FDCount * sizeof (UINT16));
  TotalSize += (HeaderSize + HDCount * sizeof (UINT16));
  TotalSize += (HeaderSize + CDCount * sizeof (UINT16));
  TotalSize += (HeaderSize + NETCount * sizeof (UINT16));
  TotalSize += (HeaderSize + BEVCount * sizeof (UINT16));

  NewDevOrder = AllocateZeroPool (TotalSize);
  if (NULL == NewDevOrder) {
    return EFI_OUT_OF_RESOURCES;
  }



  //
  // copy FD
  //
  Ptr             = DevOrder;
  NewPtr          = NewDevOrder;
  NewPtr->BbsType = Ptr->BbsType;
  NewPtr->Length  = (UINT16) (sizeof (UINT16) + FDCount * sizeof (UINT16));
  for (Index = 0; Index < Ptr->Length / sizeof (UINT16) - 1; Index++) {
    if (LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_IGNORE_ENTRY ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_DO_NOT_BOOT_FROM ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_LOWEST_PRIORITY ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].DeviceType != BBS_FLOPPY
        ) {
      continue;
    }

    NewPtr->Data[FDIndex] = Ptr->Data[Index];
    FDIndex++;
  }
  NewFDPtr = NewPtr->Data;

  //
  // copy HD
  //
  Ptr             = (LEGACY_DEV_ORDER_ENTRY *) (&Ptr->Data[Ptr->Length / sizeof (UINT16) - 1]);
  NewPtr          = (LEGACY_DEV_ORDER_ENTRY *) (&NewPtr->Data[NewPtr->Length / sizeof (UINT16) -1]);
  NewPtr->BbsType = Ptr->BbsType;
  NewPtr->Length  = (UINT16) (sizeof (UINT16) + HDCount * sizeof (UINT16));
  for (Index = 0; Index < Ptr->Length / sizeof (UINT16) - 1; Index++) {
    if (LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_IGNORE_ENTRY ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_DO_NOT_BOOT_FROM ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_LOWEST_PRIORITY ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].DeviceType != BBS_HARDDISK
        ) {
      continue;
    }

    NewPtr->Data[HDIndex] = Ptr->Data[Index];
    HDIndex++;
  }
  NewHDPtr = NewPtr->Data;

  //
  // copy CD
  //
  Ptr    = (LEGACY_DEV_ORDER_ENTRY *) (&Ptr->Data[Ptr->Length / sizeof (UINT16) - 1]);
  NewPtr = (LEGACY_DEV_ORDER_ENTRY *) (&NewPtr->Data[NewPtr->Length / sizeof (UINT16) -1]);
  NewPtr->BbsType = Ptr->BbsType;
  NewPtr->Length  = (UINT16) (sizeof (UINT16) + CDCount * sizeof (UINT16));
  for (Index = 0; Index < Ptr->Length / sizeof (UINT16) - 1; Index++) {
    if (LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_IGNORE_ENTRY ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_DO_NOT_BOOT_FROM ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_LOWEST_PRIORITY ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].DeviceType != BBS_CDROM
        ) {
      continue;
    }

    NewPtr->Data[CDIndex] = Ptr->Data[Index];
    CDIndex++;
  }
  NewCDPtr = NewPtr->Data;

  //
  // copy NET
  //
  Ptr    = (LEGACY_DEV_ORDER_ENTRY *) (&Ptr->Data[Ptr->Length / sizeof (UINT16) - 1]);
  NewPtr = (LEGACY_DEV_ORDER_ENTRY *) (&NewPtr->Data[NewPtr->Length / sizeof (UINT16) -1]);
  NewPtr->BbsType = Ptr->BbsType;
  NewPtr->Length  = (UINT16) (sizeof (UINT16) + NETCount * sizeof (UINT16));
  for (Index = 0; Index < Ptr->Length / sizeof (UINT16) - 1; Index++) {
    if (LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_IGNORE_ENTRY ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_DO_NOT_BOOT_FROM ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_LOWEST_PRIORITY ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].DeviceType != BBS_EMBED_NETWORK
        ) {
      continue;
    }

    NewPtr->Data[NETIndex] = Ptr->Data[Index];
    NETIndex++;
  }
  NewNETPtr = NewPtr->Data;

  //
  // copy BEV
  //
  Ptr    = (LEGACY_DEV_ORDER_ENTRY *) (&Ptr->Data[Ptr->Length / sizeof (UINT16) - 1]);
  NewPtr = (LEGACY_DEV_ORDER_ENTRY *) (&NewPtr->Data[NewPtr->Length / sizeof (UINT16) -1]);
  NewPtr->BbsType = Ptr->BbsType;
  NewPtr->Length  = (UINT16) (sizeof (UINT16) + BEVCount * sizeof (UINT16));
  for (Index = 0; Index < Ptr->Length / sizeof (UINT16) - 1; Index++) {
    if (LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_IGNORE_ENTRY ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_DO_NOT_BOOT_FROM ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].BootPriority == BBS_LOWEST_PRIORITY ||
        LocalBbsTable[Ptr->Data[Index] & 0xFF].DeviceType != BBS_BEV_DEVICE
        ) {
      continue;
    }

    NewPtr->Data[BEVIndex] = Ptr->Data[Index];
    BEVIndex++;
  }
  NewBEVPtr = NewPtr->Data;

  for (Index = 0; Index < BbsCount; Index++) {
    if ((LocalBbsTable[Index].BootPriority == BBS_IGNORE_ENTRY) ||
        (LocalBbsTable[Index].BootPriority == BBS_DO_NOT_BOOT_FROM) ||
        (LocalBbsTable[Index].BootPriority == BBS_LOWEST_PRIORITY)
        ) {
      continue;
    }

    switch (LocalBbsTable[Index].DeviceType) {
    case BBS_FLOPPY:
      Idx       = &FDIndex;
      NewDevPtr = NewFDPtr;
      break;

    case BBS_HARDDISK:
      Idx       = &HDIndex;
      NewDevPtr = NewHDPtr;
      break;

    case BBS_CDROM:
      Idx       = &CDIndex;
      NewDevPtr = NewCDPtr;
      break;

    case BBS_EMBED_NETWORK:
      Idx       = &NETIndex;
      NewDevPtr = NewNETPtr;
      break;

    case BBS_BEV_DEVICE:
      Idx       = &BEVIndex;
      NewDevPtr = NewBEVPtr;
      break;

    default:
      Idx = NULL;
      break;
    }
    //
    // at this point we have copied those valid indexes to new buffer
    // and we should check if there is any new appeared boot device
    //
    if (Idx != NULL) {
      for (Index2 = 0; Index2 < *Idx; Index2++) {
        if ((NewDevPtr[Index2] & 0xFF) == (UINT16) Index) {
          break;
        }
      }

      if (Index2 == *Idx) {
        //
        // Index2 == *Idx means we didn't find Index
        // so Index is a new appeared device's index in BBS table
        // insert it before disabled indexes.
        //
        for (Index2 = 0; Index2 < *Idx; Index2++) {
          if ((NewDevPtr[Index2] & 0xFF00) == 0xFF00) {
            break;
          }
        }
        CopyMem (&NewDevPtr[Index2 + 1], &NewDevPtr[Index2], (*Idx - Index2) * sizeof (UINT16));
        NewDevPtr[Index2] = (UINT16) (Index & 0xFF);
        (*Idx)++;
      }
    }
  }

  FreePool (DevOrder);

  Status = gRT->SetVariable (
                  VAR_LEGACY_DEV_ORDER,
                  &gEfiLegacyDevOrderVariableGuid,
                  VAR_FLAG,
                  TotalSize,
                  NewDevOrder
                  );
  FreePool (NewDevOrder);

  return Status;
}

/**
 Set Boot Priority for specified device type.

 @param[in]      DeviceType      The device type.
 @param[in]      BbsIndex        The BBS index to set the highest priority. Ignore when -1.
 @param[in, out] LocalBbsTable   The BBS table.
 @param[in, out] Priority        The prority table.

 @retval EFI_SUCCESS           The function completes successfully.
 @retval EFI_NOT_FOUND         Failed to find device.
 @retval EFI_OUT_OF_RESOURCES  Failed to get the efi variable of device order.
**/
EFI_STATUS
BdsSetBootPriority4SameTypeDev (
  IN UINT16                                              DeviceType,
  IN UINTN                                               BbsIndex,
  IN OUT BBS_TABLE                                       *LocalBbsTable,
  IN OUT UINT16                                          *Priority
  )
{
  LEGACY_DEV_ORDER_ENTRY      *DevOrder;
  LEGACY_DEV_ORDER_ENTRY      *DevOrderPtr;
  UINTN                       DevOrderSize;
  UINTN                       Index;

  DevOrder = BdsLibGetVariableAndSize (
               VAR_LEGACY_DEV_ORDER,
               &gEfiLegacyDevOrderVariableGuid,
               &DevOrderSize
               );
  if (NULL == DevOrder) {
    return EFI_OUT_OF_RESOURCES;
  }

  DevOrderPtr = DevOrder;
  while ((UINT8 *) DevOrderPtr < (UINT8 *) DevOrder + DevOrderSize) {
    if (DevOrderPtr->BbsType == DeviceType) {
      break;
    }

    DevOrderPtr = (LEGACY_DEV_ORDER_ENTRY *) ((UINT8 *) DevOrderPtr + sizeof (BBS_TYPE) + DevOrderPtr->Length);
  }

  if ((UINT8 *) DevOrderPtr >= (UINT8 *) DevOrder + DevOrderSize) {
    FreePool (DevOrder);
    return EFI_NOT_FOUND;
  }

  if (BbsIndex != (UINTN) -1) {
    LocalBbsTable[BbsIndex].BootPriority = *Priority;
    (*Priority)++;
  }
  //
  // If the high byte of the DevIndex is 0xFF, it indicates that this device has been disabled.
  //
  for (Index = 0; Index < DevOrderPtr->Length / sizeof (UINT16) - 1; Index++) {
    if ((DevOrderPtr->Data[Index] & 0xFF00) == 0xFF00) {
      //
      // LocalBbsTable[DevIndex[Index] & 0xFF].BootPriority = BBS_DISABLED_ENTRY;
      //
    } else if (DevOrderPtr->Data[Index] != BbsIndex) {
      LocalBbsTable[DevOrderPtr->Data[Index]].BootPriority = *Priority;
      (*Priority)++;
    }
  }

  FreePool (DevOrder);
  return EFI_SUCCESS;
}

/**
 Print the BBS Table.

 @param[in] LocalBbsTable   The BBS table.
 @param[in] BbsCount        The count of entry in BBS table.
**/
VOID
PrintBbsTable (
  IN BBS_TABLE  *LocalBbsTable,
  IN UINT16     BbsCount
  )
{
  UINT16  Idx;

  DEBUG ((EFI_D_INFO, "\n"));
  DEBUG ((EFI_D_INFO, " NO  Prio bb/dd/ff cl/sc Type Stat segm:offs\n"));
  DEBUG ((EFI_D_INFO, "=============================================\n"));
  for (Idx = 0; Idx < BbsCount; Idx++) {
    if ((LocalBbsTable[Idx].BootPriority == BBS_IGNORE_ENTRY) ||
        (LocalBbsTable[Idx].BootPriority == BBS_DO_NOT_BOOT_FROM) ||
        (LocalBbsTable[Idx].BootPriority == BBS_LOWEST_PRIORITY)
        ) {
      continue;
    }

    DEBUG (
      (EFI_D_INFO,
      " %02x: %04x %02x/%02x/%02x %02x/%02x %04x %04x %04x:%04x\n",
      (UINTN) Idx,
      (UINTN) LocalBbsTable[Idx].BootPriority,
      (UINTN) LocalBbsTable[Idx].Bus,
      (UINTN) LocalBbsTable[Idx].Device,
      (UINTN) LocalBbsTable[Idx].Function,
      (UINTN) LocalBbsTable[Idx].Class,
      (UINTN) LocalBbsTable[Idx].SubClass,
      (UINTN) LocalBbsTable[Idx].DeviceType,
      (UINTN) * (UINT16 *) &LocalBbsTable[Idx].StatusFlags,
      (UINTN) LocalBbsTable[Idx].BootHandlerSegment,
      (UINTN) LocalBbsTable[Idx].BootHandlerOffset,
      (UINTN) ((LocalBbsTable[Idx].MfgStringSegment << 4) + LocalBbsTable[Idx].MfgStringOffset),
      (UINTN) ((LocalBbsTable[Idx].DescStringSegment << 4) + LocalBbsTable[Idx].DescStringOffset))
      );
  }

  DEBUG ((EFI_D_INFO, "\n"));
}

/**
 Set the boot priority for BBS entries based on boot option entry and boot order.

 @param[in] Entry              The boot option is to be checked for refresh BBS table.

 @retval EFI_SUCCESS           The boot priority for BBS entries is refreshed successfully.
 @retval EFI_NOT_FOUND         BBS entries can't be found.
 @retval EFI_OUT_OF_RESOURCES  Failed to get the legacy device boot order.
**/
EFI_STATUS
EFIAPI
BdsRefreshBbsTableForBoot (
  IN BDS_COMMON_OPTION        *Entry
  )
{
  EFI_STATUS                Status;
  UINT16                    BbsIndex;
  UINT16                    HddCount;
  UINT16                    BbsCount;
  HDD_INFO                  *LocalHddInfo;
  BBS_TABLE                 *LocalBbsTable;
  UINT16                    DevType;
  EFI_LEGACY_BIOS_PROTOCOL  *LegacyBios;
  UINTN                     Index;
  UINT16                    Priority;
  UINT16                    *BootOrder;
  UINTN                     BootOrderSize;
  UINT8                     *BootOptionVar;
  UINTN                     BootOptionSize;
  CHAR16                    BootOption[9];
  UINT8                     *Ptr;
  UINT16                    DevPathLen;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  UINT16                    *DeviceType;
  UINTN                     DeviceTypeCount;
  UINTN                     DeviceTypeIndex;

  HddCount      = 0;
  BbsCount      = 0;
  LocalHddInfo  = NULL;
  LocalBbsTable = NULL;
  DevType       = BBS_UNKNOWN;

  Status        = EfiLibLocateProtocol (&gEfiLegacyBiosProtocolGuid, (VOID **) &LegacyBios);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  LegacyBios->GetBbsInfo (
                LegacyBios,
                &HddCount,
                &LocalHddInfo,
                &BbsCount,
                &LocalBbsTable
                );
  //
  // First, set all the present devices' boot priority to BBS_UNPRIORITIZED_ENTRY
  // We will set them according to the settings setup by user
  //
  for (Index = 0; Index < BbsCount; Index++) {
    if (!((BBS_IGNORE_ENTRY == LocalBbsTable[Index].BootPriority) ||
        (BBS_DO_NOT_BOOT_FROM == LocalBbsTable[Index].BootPriority) ||
         (BBS_LOWEST_PRIORITY == LocalBbsTable[Index].BootPriority))) {
      LocalBbsTable[Index].BootPriority = BBS_UNPRIORITIZED_ENTRY;
    }
  }
  //
  // boot priority always starts at 0
  //
  Priority = 0;
  if (Entry->LoadOptionsSize == sizeof (BBS_TABLE) + sizeof (UINT16)) {
    //
    // If Entry stands for a legacy boot option, we prioritize the devices with the same type first.
    //
    DevType  = ((BBS_TABLE *) Entry->LoadOptions)->DeviceType;
    BbsIndex = *(UINT16 *) ((BBS_TABLE *) Entry->LoadOptions + 1);
    Status = BdsSetBootPriority4SameTypeDev (
              DevType,
              BbsIndex,
              LocalBbsTable,
              &Priority
              );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  //
  // we have to set the boot priority for other BBS entries with different device types
  //
  BootOrder = BdsLibGetVariableAndSize (
                L"BootOrder",
                &gEfiGlobalVariableGuid,
                &BootOrderSize
                );
  DeviceType = AllocatePool (BootOrderSize + sizeof (UINT16));
  ASSERT (DeviceType != NULL);
  if (DeviceType == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DeviceType[0]   = DevType;
  DeviceTypeCount = 1;
  for (Index = 0; ((BootOrder != NULL) && (Index < BootOrderSize / sizeof (UINT16))); Index++) {
    UnicodeSPrint (BootOption, sizeof (BootOption), L"Boot%04x", BootOrder[Index]);
    BootOptionVar = BdsLibGetVariableAndSize (
                      BootOption,
                      &gEfiGlobalVariableGuid,
                      &BootOptionSize
                      );
    if (NULL == BootOptionVar) {
      continue;
    }

    Ptr = BootOptionVar;

    Ptr += sizeof (UINT32);
    DevPathLen = *(UINT16 *) Ptr;
    Ptr += sizeof (UINT16);
    Ptr += StrSize ((UINT16 *) Ptr);
    DevPath = (EFI_DEVICE_PATH_PROTOCOL *) Ptr;
    if (BBS_DEVICE_PATH != DevPath->Type || BBS_BBS_DP != DevPath->SubType) {
      FreePool (BootOptionVar);
      continue;
    }

    Ptr += DevPathLen;
    DevType = ((BBS_TABLE *) Ptr)->DeviceType;
    for (DeviceTypeIndex = 0; DeviceTypeIndex < DeviceTypeCount; DeviceTypeIndex++) {
      if (DeviceType[DeviceTypeIndex] == DevType) {
        break;
      }
    }
    if (DeviceTypeIndex < DeviceTypeCount) {
      //
      // We don't want to process twice for a device type
      //
      FreePool (BootOptionVar);
      continue;
    }

    DeviceType[DeviceTypeCount] = DevType;
    DeviceTypeCount++;

    Status = BdsSetBootPriority4SameTypeDev (
              DevType,
              (UINTN) -1,
              LocalBbsTable,
              &Priority
              );
    FreePool (BootOptionVar);
    if (EFI_ERROR (Status)) {
      break;
    }
  }

  if (BootOrder != NULL) {
    FreePool (BootOrder);
  }

  DEBUG_CODE_BEGIN();
    PrintBbsTable (LocalBbsTable, BbsCount);
  DEBUG_CODE_END();

  return Status;
}

