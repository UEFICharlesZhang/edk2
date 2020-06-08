/** @file
  preovide H2O BDS service protocol interface

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#include "Bds.h"

#define SHELL_ENVIRONMENT_INTERFACE_PROTOCOL \
  { 0x47c7b221, 0xc42a, 0x11d2, 0x8e, 0x57, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b }


STATIC   LIST_ENTRY       mBootList = {&mBootList, &mBootList};
STATIC   LIST_ENTRY       mDriverList = {&mDriverList, &mDriverList};
STATIC   VOID             *mShellEnvProtocolCallbackReg;
STATIC   BOOLEAN          mNeedSyncBootOrder = TRUE;

H2O_BDS_SERVICES_PROTOCOL  mH2OBdsServices = {
  sizeof (H2O_BDS_SERVICES_PROTOCOL),
  BdsServicesGetOsIndications,
  BdsServicesGetTimeout,
  BdsServicesGetBootMode,
  BdsServicesGetBootType,
  NULL,
  NULL,
  NULL,
  BdsServicesGetBootList,
  BdsServicesGetDriverList,
  BdsServicesCreateLoadOption,
  BdsServicesFreeLoadOption,
  BdsServicesConvertVarToLoadOption,
  BdsServicesConvertLoadOptionToVar,
  BdsServicesInsertLoadOption,
  BdsServicesRemoveLoadOption,
  BdsServicesExpandLoadOption,
  BdsServicesLaunchLoadOption
};

/**
  Frees pool.

  @param  Buffer                 The allocated pool entry to free.
**/
STATIC
VOID
InternalFreePool (
  IN VOID        *Buffer
  )
{
  if (Buffer != NULL) {
    FreePool (Buffer);
  }
}

/**
  Internal function to free all of allocated memory in specific H2O_BDS_LOAD_OPTION.

  @param[in]  LoadOption                 The allocated load option to free.
**/
STATIC
VOID
InternalFreeBdsLoadOption (
  IN   H2O_BDS_LOAD_OPTION         *LoadOption
  )
{
  if (LoadOption == NULL) {
    return;
  }
  InternalFreePool (LoadOption->DevicePath);
  InternalFreePool (LoadOption->Description);
  InternalFreePool (LoadOption->LoadOptionalData);
  InternalFreePool (LoadOption->StatusString);
  FreePool (LoadOption);
}

/**
  Internal function to check the input str is whether a correct #### format. Here the #### is replaced
  by a unique option number in printable hexadecimal representation using the digits 0¡V9, and the upper
  case versions of the characters A¡VF (0000¡VFFFF).

  @param[in] Str   Input #### string to be checked.

  @retval TRUE     The input string is correct #### format.
  @retval FALSE    The input string is incorrect #### format.
**/
STATIC
BOOLEAN
IsValidOptionNumber (
  IN CONST   CHAR16       *Str
  )
{
  UINTN         Index;

  if (Str == NULL || StrLen (Str) != 4) {
    return FALSE;
  }

  for (Index = 0; Str[Index] != 0; Index++) {
    if (!((Str[Index] >= L'0' && Str[Index] <= L'9') || (Str[Index] >= L'A' && Str[Index] <= L'F'))) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
  Internal function to calculate UINT16 value from input #### string.

  @param[in] Str   Input #### string to be calculated.

  @return The UINT16 value which calculate from input string.
**/
STATIC
UINT16
GetOptionNumber (
  IN CONST   CHAR16       *Str
  )
{
  UINTN         Index;
  UINT16        Number;

  if (Str == NULL || StrLen (Str) != 4) {
    return 0;
  }

  Number = 0;
  for (Index = 0; Str[Index] != 0; Index++) {
    if (Str[Index] >= L'0' && Str[Index] <= L'9') {
      Number = Number * 0x10 + Str[Index] - L'0';
    } else if (Str[Index] >= L'A' && Str[Index] <= L'F') {
      Number = Number * 0x10 + Str[Index] - L'A' + 0x0A;
    } else {
      return 0;
    }
  }
  return Number;
}

/**
  According to variable name and GUID to Determine the variable is BootPerv or not.

  @param[in]  VariableName   Name of Variable to be found.
  @param[in]  VendorGuid     Variable vendor GUID.

  @retval     TRUE           This is BootPerv variable.
  @retval     FALSE          This isn't BootPerv variable.
**/
STATIC
BOOLEAN
IsBootPreviousVariable (
  IN CONST  CHAR16                             *VariableName,
  IN CONST  EFI_GUID                           *VendorGuid
  )
{
  if (VariableName != NULL && VendorGuid != NULL) {
    if (StrCmp (VariableName, H2O_BOOT_PREVIOUS_VARIABLE_NAME) == 0 && CompareGuid (VendorGuid, &gEfiGenericVariableGuid)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  According to variable name and GUID to Determine the variable is BootCurrent or not.

  @param[in]  VariableName   Name of Variable to be found.
  @param[in]  VendorGuid     Variable vendor GUID.

  @retval     TRUE           This is BootCurrent variable.
  @retval     FALSE          This isn't BootCurrent variable.
**/
STATIC
BOOLEAN
IsBootCurrentVariable (
  IN CONST  CHAR16                             *VariableName,
  IN CONST  EFI_GUID                           *VendorGuid
  )
{
  if (VariableName != NULL && VendorGuid != NULL) {
    if (StrCmp (VariableName, EFI_BOOT_CURRENT_VARIABLE_NAME) == 0 && CompareGuid (VendorGuid, &gEfiGlobalVariableGuid)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  According to variable name and GUID to Determine the variable is BootNext or not.

  @param[in]  VariableName   Name of Variable to be found.
  @param[in]  VendorGuid     Variable vendor GUID.

  @retval     TRUE           This is BootNext variable.
  @retval     FALSE          This isn't BootNext variable.
**/
STATIC
BOOLEAN
IsBootNextVariable (
  IN CONST  CHAR16                             *VariableName,
  IN CONST  EFI_GUID                           *VendorGuid
  )
{
  if (VariableName != NULL && VendorGuid != NULL) {
    if (StrCmp (VariableName, EFI_BOOT_NEXT_VARIABLE_NAME) == 0 && CompareGuid (VendorGuid, &gEfiGlobalVariableGuid)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Internal function to get index in the input option order array by input option number.

  @param[in]  OptionNum          Load option number.
  @param[in]  OptionOrder        Pointer to input option order array.
  @param[in]  OptionOrderSize    Option order array size by bytes.
  @param[out] OptionIndex        Index in the input option order.

  @retval EFI_SUCCESS            Get index in input option order successfully.
  @retval EFI_INVALID_PARAMETER  OptionOrder is NULL, OptionOrderSize is 0 or OptionIndex is NULL.
  @retval EFI_NOT_FOUND          Load option number isn't in input load option order.
**/
STATIC
EFI_STATUS
GetIndexInOptionOrder (
  IN   UINT16                      OptionNum,
  IN   UINT16                      *OptionOrder,
  IN   UINTN                       OptionOrderSize,
  OUT  UINTN                       *OptionIndex
  )
{
  UINTN                  OptionOrderNum;
  UINTN                  Index;

  if (OptionOrder == NULL || OptionOrderSize == 0 || OptionIndex == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  OptionOrderNum = OptionOrderSize / 2;
  for (Index = 0; Index < OptionOrderNum; Index++) {
    if (OptionOrder[Index] == OptionNum) {
      *OptionIndex = Index;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Internal function to insert load option to load option list.

  @param[in]  BdsLoadOption      Load option want to insert to load option list.
  @param[in]  OptionOrder        Pointer to input option order array.
  @param[in]  OptionOrderSize    Option order array size by bytes.

  @retval EFI_SUCCESS            Insert load option to load option list successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL, OptionOrder is NULL, OptionOrderSize is 0.
  @retval EFI_NOT_FOUND          Load option number isn't in input load option order.
**/
STATIC
EFI_STATUS
InsertLoadOptionToList (
  IN   H2O_BDS_LOAD_OPTION         *BdsLoadOption,
  IN   UINT16                      *OptionOrder,
  IN   UINTN                       OptionOrderSize
  )
{
  LIST_ENTRY             *OptionList;
  LIST_ENTRY             *CurrentList;
  LIST_ENTRY             *NextList;
  UINTN                  InsertOptionIndex;
  UINTN                  Index;
  H2O_BDS_LOAD_OPTION    *CurrentLoadOption;
  EFI_STATUS             Status;


  if (BdsLoadOption == NULL || (OptionOrder == NULL && OptionOrderSize != 0)) {
    return EFI_INVALID_PARAMETER;
  }

  OptionList = BdsLoadOption->DriverOrBoot ? &mBootList : &mDriverList;
  Status     = GetIndexInOptionOrder (BdsLoadOption->LoadOrder, OptionOrder, OptionOrderSize, &InsertOptionIndex);
  if (Status != EFI_SUCCESS) {
    return Status;
  }
  if (IsListEmpty (OptionList)) {
    InsertTailList (OptionList, &BdsLoadOption->Link);
    return EFI_SUCCESS;
  }

  CurrentList       = GetFirstNode (OptionList);
  for (Index = 0; Index < InsertOptionIndex; Index++) {
    CurrentLoadOption = BDS_OPTION_FROM_LINK (CurrentList);
    if (CurrentLoadOption->LoadOrder == OptionOrder[Index]) {
      CurrentList = GetNextNode (OptionList, CurrentList);
    }
  }

  if (CurrentList != OptionList) {
    CurrentLoadOption = BDS_OPTION_FROM_LINK (CurrentList);
    if (CurrentLoadOption->LoadOrder == BdsLoadOption->LoadOrder) {
      NextList = GetNextNode (OptionList, CurrentList);
      RemoveEntryList (&CurrentLoadOption->Link);
      BdsServicesFreeLoadOption (&mH2OBdsServices, CurrentLoadOption);
      CurrentList = NextList;
    }
  }

  InsertTailList (CurrentList, &BdsLoadOption->Link);

  return EFI_SUCCESS;
}

/**
  Internal function to check the input device path is whether a legacy boot option device path.

  @param[in]  DevicePath   A pointer to a device path data structure.

  @retval TRUE             This is a legacy boot option.
  @retval FALSE            This isn't a legacy boot option.
**/
STATIC
BOOLEAN
IsLegacyBootOption (
  IN  CONST EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  if (DevicePath == NULL) {
    return FALSE;
  }

  if ((BBS_DEVICE_PATH == DevicePath->Type) && (BBS_BBS_DP == DevicePath->SubType)) {
    return TRUE;
  }
  return FALSE;
}

/**
  Internal function to check the input device is whether in expanded option list.

  @param[in]  BdsLoadOption  Load option want to insert to load option list.
  @param[in]  DevicePath     A pointer to a device path data structure.

  @retval TRUE               The device path is already in expanded option list
  @retval FALSE              The device path isn't in expanded option list
**/
STATIC
BOOLEAN
IsInExpandedLoadOptions (
  IN        H2O_BDS_LOAD_OPTION         *BdsLoadOption,
  IN  CONST EFI_DEVICE_PATH_PROTOCOL    *DevicePath
  )
{
  LIST_ENTRY               *ExpandedOptions;
  LIST_ENTRY               *Link;
  H2O_BDS_LOAD_OPTION      *CurrentLoadOption;

  if (BdsLoadOption == NULL || !BdsLoadOption->Expanded || DevicePath == NULL) {
    return FALSE;
  }

  ExpandedOptions = &BdsLoadOption->ExpandedLoadOptions;
  for (Link = GetFirstNode (ExpandedOptions); !IsNull (ExpandedOptions, Link); Link = GetNextNode (ExpandedOptions, Link)) {
    CurrentLoadOption = BDS_OPTION_FROM_LINK (Link);
    if (GetDevicePathSize (DevicePath) == GetDevicePathSize (CurrentLoadOption->DevicePath) &&
        CompareMem (DevicePath, CurrentLoadOption->DevicePath, GetDevicePathSize (DevicePath)) == 0) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Internal function to add expanded load option to expand option list.

  @param[in]  BdsLoadOption      Load option want to insert to load option list.

  @retval EFI_SUCCESS            Add expaanded load option to option list successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL.
**/
STATIC
EFI_STATUS
AddExpandedWindowsToGoOption (
  IN OUT  H2O_BDS_LOAD_OPTION         *BdsLoadOption
  )
{
  UINTN                         Index;
  UINTN                         NumberFileSystemHandles;
  EFI_HANDLE                    *FileSystemHandles;
  H2O_BDS_LOAD_OPTION           *NewLoadOption;
  EFI_STATUS                    Status;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;

  if (BdsLoadOption == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &NumberFileSystemHandles,
                  &FileSystemHandles
                  );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  for (Index = 0; Index < NumberFileSystemHandles; Index++) {
    DevicePath = DevicePathFromHandle (FileSystemHandles[Index]);
    if (IsWindowsToGo (FileSystemHandles[Index]) && !IsInExpandedLoadOptions (BdsLoadOption, DevicePath)) {
      Status = BdsServicesCreateLoadOption (
                 &mH2OBdsServices,
                 BOOT_OPTION,
                 BdsLoadOption->LoadOrderVarName,
                 &BdsLoadOption->LoadOrderVarGuid,
                 LOAD_OPTION_ACTIVE,
                 DevicePath,
                 BdsLoadOption->Description,
                 BdsLoadOption->LoadOptionalData,
                 BdsLoadOption->LoadOptionalDataSize,
                 &NewLoadOption
                 );
      if (Status == EFI_SUCCESS) {
        InsertTailList (&BdsLoadOption->ExpandedLoadOptions, &NewLoadOption->Link);
      }
    }
  }
  InternalFreePool (FileSystemHandles);
  return EFI_SUCCESS;
}

/**
  Internal function to check the input device is whether in input device path array.

  @param[in]  DevicePath        A Pointer to a UEFI device path.
  @param[in]  DevicePaths       A Pointer to a UEFI device path array.
  @param[in]  DevicePathCount   The device path count in device path array.

  @retval TRUE          Device path is in device path array.
  @retval FALSE         Device path isn't in device path array.
**/
STATIC
BOOLEAN
IsDevicePathInArray (
  IN  EFI_DEVICE_PATH_PROTOCOL      *DevicePath,
  IN  EFI_DEVICE_PATH_PROTOCOL      **DevicePaths,
  IN  UINTN                         DevicePathCount
  )
{
  UINTN                 Index;

  if (DevicePath == NULL || DevicePaths == NULL || DevicePathCount == 0) {
    return FALSE;
  }

  for (Index = 0; Index < DevicePathCount; Index++) {
    if (GetDevicePathSize (DevicePath) == GetDevicePathSize (DevicePaths[Index]) &&
        CompareMem (DevicePath, DevicePaths[Index], GetDevicePathSize (DevicePath)) == 0) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Internal function to remove redundant boot option in expanded list.

  @param[in]  BdsLoadOption      Load option want to insert to load option list.

  @retval EFI_SUCCESS            Remove redundant boot option successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL.
  @retval EFI_OUT_OF_RESOURCES   Unable to allocate memory to store device path.
**/
STATIC
EFI_STATUS
RemoveRedundantExpandedWindowsToGoOption (
  IN OUT  H2O_BDS_LOAD_OPTION         *BdsLoadOption
  )
{
  UINTN                         Index;
  UINTN                         NumberFileSystemHandles;
  EFI_HANDLE                    *FileSystemHandles;
  H2O_BDS_LOAD_OPTION           *CurrentLoadOption;
  EFI_STATUS                    Status;
  EFI_DEVICE_PATH_PROTOCOL      **DevicePaths;
  LIST_ENTRY                    *ExpandedOptions;
  LIST_ENTRY                    *Link;
  LIST_ENTRY                    *CurrentLink;

  if (BdsLoadOption == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &NumberFileSystemHandles,
                  &FileSystemHandles
                  );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }
  DevicePaths = AllocateZeroPool (NumberFileSystemHandles * sizeof (EFI_DEVICE_PATH_PROTOCOL *));
  if (DevicePaths == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  for (Index = 0; Index < NumberFileSystemHandles; Index++) {
    DevicePaths[Index] = DevicePathFromHandle (FileSystemHandles[Index]);
  }

  ExpandedOptions = &BdsLoadOption->ExpandedLoadOptions;
  for (Link = GetFirstNode (ExpandedOptions); !IsNull (ExpandedOptions, Link); Link = GetNextNode (ExpandedOptions, Link)) {
     CurrentLoadOption = BDS_OPTION_FROM_LINK (Link);
     if (!IsDevicePathInArray (CurrentLoadOption->DevicePath, DevicePaths, NumberFileSystemHandles)) {
       CurrentLink = Link;
       Link = CurrentLink->BackLink;
       RemoveEntryList (CurrentLink);
       BdsServicesFreeLoadOption (&mH2OBdsServices, CurrentLoadOption);
     }
  }
  InternalFreePool (DevicePaths);
  InternalFreePool (FileSystemHandles);

  return EFI_SUCCESS;
}
/**
  Internal function to expand windows to go boot option.

  It will set Expanded member in BdsLoadOption to TRUE and insert all of expanded load options
  to ExpandedLoadOptions list in BdsLoadOption if this function return EFI_SUCESS.

  @param[in,out] BdsLoadOption  A pointer to BDS load option.

  @retval EFI_SUCCESS           Expand windows to go boot option successfully.
  @retval EFI_INVALID_PARAMETER BdsLoadOption is NULL.
**/
STATIC
EFI_STATUS
ExpandWinowsToGoOption (
  IN OUT  H2O_BDS_LOAD_OPTION         *BdsLoadOption
  )
{
  if (BdsLoadOption == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  AddExpandedWindowsToGoOption (BdsLoadOption);
  RemoveRedundantExpandedWindowsToGoOption (BdsLoadOption);
  BdsLoadOption->Expanded = TRUE;
  return EFI_SUCCESS;
}

/**
  Internal function to expand hard drive media short-form device path to hard driver media full device
  path.

  It will set Expanded member in BdsLoadOption to TRUE and insert all of expanded load options
  to ExpandedLoadOptions list in BdsLoadOption if this function return EFI_SUCESS.

  @param[in,out] BdsLoadOption  A pointer to BDS load option.

  @retval EFI_SUCCESS           Expand hard driver media device path successfully.
  @retval EFI_INVALID_PARAMETER BdsLoadOption is NULL.
**/
STATIC
EFI_STATUS
ExpandHddOption (
  IN   H2O_BDS_LOAD_OPTION         *BdsLoadOption
  )
{
  EFI_DEVICE_PATH_PROTOCOL       *FullDevicePath;
  EFI_STATUS                     Status;
  H2O_BDS_LOAD_OPTION            *NewLoadOption;

  if (BdsLoadOption == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  FullDevicePath = BdsExpandPartitionPartialDevicePathToFull ((HARDDRIVE_DEVICE_PATH *)BdsLoadOption->DevicePath);
  if (FullDevicePath != NULL && !IsInExpandedLoadOptions (BdsLoadOption, FullDevicePath)) {
    Status = BdsServicesCreateLoadOption (
               &mH2OBdsServices,
               BOOT_OPTION,
               BdsLoadOption->LoadOrderVarName,
               &BdsLoadOption->LoadOrderVarGuid,
               LOAD_OPTION_ACTIVE,
               FullDevicePath,
               BdsLoadOption->Description,
               BdsLoadOption->LoadOptionalData,
               BdsLoadOption->LoadOptionalDataSize,
               &NewLoadOption
               );
    InsertTailList (&BdsLoadOption->ExpandedLoadOptions, &NewLoadOption->Link);
    BdsLoadOption->Expanded = TRUE;
  }
  InternalFreePool (FullDevicePath);
  return EFI_SUCCESS;
}

/**
  Internal function to expand H2O BDS groupt boot option.

  It will set Expanded member in BdsLoadOption to TRUE and insert all of expanded load options
  to ExpandedLoadOptions list in BdsLoadOption if this function return EFI_SUCESS.

  @param[in,out] BdsLoadOption  A pointer to BDS load option.

  @retval EFI_SUCCESS           Expand H2O BDS groupt boot option successfully.
  @retval EFI_INVALID_PARAMETER BdsLoadOption is NULL.
**/
STATIC
EFI_STATUS
ExpandBootGroupOption (
  IN   H2O_BDS_LOAD_OPTION         *BdsLoadOption
  )
{
  EFI_STATUS                          Status;
  UINTN                               HandleCount;
  EFI_HANDLE                          *HandleBuffer;
  H2O_BDS_BOOT_GROUP_DEVICE_PATH      *BootGroupDevicePath;
  UINTN                               Index;
  H2O_BDS_BOOT_GROUP_PROTOCOL         *BootGroup;
  UINT32                              BootOptionArrayLen;
  H2O_BDS_LOAD_OPTION                 *BootOptionArray;
  UINT32                              ArrayIndex;
  H2O_BDS_LOAD_OPTION                 *NewLoadOption;

  if (BdsLoadOption == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gH2OBdsBootGroupProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  BootGroupDevicePath = (H2O_BDS_BOOT_GROUP_DEVICE_PATH *) BdsLoadOption->DevicePath;
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gH2OBdsBootGroupProtocolGuid,
                    (VOID **) &BootGroup
                    );
    ASSERT (Status == EFI_SUCCESS);
    if (CompareGuid (&BootGroup->VendorGuid, &BootGroupDevicePath->VendorGuid)) {
      Status = BootGroup->GetGroupDevices (
                            BootGroup,
                            BdsLoadOption,
                            &BootOptionArrayLen,
                            &BootOptionArray
                            );
      if (Status != EFI_SUCCESS) {
        break;
      }
      for (ArrayIndex = 0; ArrayIndex < BootOptionArrayLen; Index++) {
        if (!IsInExpandedLoadOptions(BdsLoadOption, BootOptionArray[Index].DevicePath)) {
          Status = BdsServicesCreateLoadOption (
                     &mH2OBdsServices,
                     BootOptionArray[Index].DriverOrBoot,
                     BootOptionArray[Index].LoadOrderVarName,
                     &BootOptionArray[Index].LoadOrderVarGuid,
                     BootOptionArray[Index].Attributes,
                     BootOptionArray[Index].DevicePath,
                     BootOptionArray[Index].Description,
                     BootOptionArray[Index].LoadOptionalData,
                     BootOptionArray[Index].LoadOptionalDataSize,
                     &NewLoadOption
                     );
          ASSERT (Status == EFI_SUCCESS);
          InsertTailList (&BdsLoadOption->ExpandedLoadOptions, &NewLoadOption->Link);
        }
      }
      FreePool (BootOptionArray);
      break;
    }
  }

  FreePool (HandleBuffer);
  return EFI_SUCCESS;
}


/**
  Internal function to check the load order number is whether in boot order.

  @param[in]  LoadOrder   Unsigned integer that specifies the current boot option being booted.
  @param[in]  BootOrder   The pointer of BootOrder
  @param[in]  OrderCount  A pointer to BDS load option.

  @retval TRUE            Load order number is in the boot order.
  @retval FALSE           Load order number isn't in the boot order.
**/
STATIC
BOOLEAN
IsOptionNumInBootOrder (
  IN      UINT16      LoadOrder,
  IN      UINT16      *BootOrder,
  IN      UINTN       OrderCount
  )
{
  UINTN        Index;

  if (BootOrder == NULL || OrderCount == 0) {
    return FALSE;
  }
  for (Index = 0; Index < OrderCount; Index++) {
    if (BootOrder[Index] == LoadOrder) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Internal function to check the load order number is whether in boot list.

  @param[in]  LoadOrder   Unsigned integer that specifies the current boot option being booted.
  @param[in]  ListHead    A list head of linked list. Each entry is a H2O_BDS_LOAD_OPTION.

  @retval TRUE            Load order number is in the boot list.
  @retval FALSE           Load order number isn't in the boot list.
**/
STATIC
BOOLEAN
IsOptionNumInBootList (
  IN  UINT16              LoadOrder,
  IN  LIST_ENTRY          *ListHead
  )
{
  LIST_ENTRY             *OptionList;
  LIST_ENTRY             *Link;
  H2O_BDS_LOAD_OPTION    *CurrentLoadOption;

  if (ListHead == NULL) {
    return FALSE;
  }

  OptionList  = ListHead;
  for (Link = GetFirstNode (OptionList); !IsNull (OptionList, Link); Link = GetNextNode (OptionList, Link)) {
    CurrentLoadOption = BDS_OPTION_FROM_LINK (Link);
    if (CurrentLoadOption->LoadOrder == LoadOrder) {
      return TRUE;
    }
  }

  return FALSE;
}



/**
  Internal function to create new H2O_BDS_LOAD_OPTION by input load order.

  @param[in]  LoadOrder   Unsigned integer that specifies the current boot option being booted.

  @return pointer to H2O_BDS_LOAD_OPTION instance or return NULL if create new boot option failed.
**/
STATIC
H2O_BDS_LOAD_OPTION *
GetNewBdsBootOption (
  IN  UINT16      LoadOrder
  )
{
  H2O_BDS_LOAD_OPTION    *LoadOption;
  UINT16                 BootOption[11];

  LoadOption = NULL;
  UnicodeSPrint (BootOption, sizeof (BootOption), L"Boot%04x", LoadOrder);
  BdsServicesConvertVarToLoadOption (&mH2OBdsServices, BootOption, &gEfiGlobalVariableGuid, &LoadOption);
  return LoadOption;

}

/**
  Internal function to check the load option list and driver or boto order are matched.

  @param[in]  LoadOrder   Unsigned integer that specifies the current boot option being booted.
  @param[in]  BootOrder   The pointer of BootOrder
  @param[in]  OrderCount  A pointer to BDS load option.

  @retval TRUE            The load option list data is correct.
  @retval FALSE           The load option list data is incorrect.
**/
STATIC
BOOLEAN
IsLoadOptionListCorrect (
  IN      BOOLEAN    DriverOrBoot,
  IN      UINT16     *BootOrder,
  IN      UINTN      OrderCount
  )
{
  LIST_ENTRY             *OptionList;
  LIST_ENTRY             *Link;
  UINTN                  Index;
  H2O_BDS_LOAD_OPTION    *CurrentLoadOption;


  if (BootOrder == NULL && OrderCount != 0) {
    return FALSE;
  }

  Index       = 0;
  OptionList  = DriverOrBoot == BOOT_OPTION ? &mBootList : &mDriverList;
  for (Link = GetFirstNode (OptionList); !IsNull (OptionList, Link); Link = GetNextNode (OptionList, Link)) {
    if (Index >= OrderCount) {
      return FALSE;
    }
    CurrentLoadOption = BDS_OPTION_FROM_LINK (Link);
    if (CurrentLoadOption->LoadOrder != BootOrder[Index++]) {
      return FALSE;
    }
  }

  return Index == OrderCount ? TRUE : FALSE;
}


/**
  Internal function to remove all of load options in load option list.

  @param[in]  LoadOrder   Unsigned integer that specifies the current boot option being booted.
**/
STATIC
VOID
RemoveAllLoadOptions (
  IN  BOOLEAN        DriverOrBoot
  )
{
  LIST_ENTRY             *OptionList;
  LIST_ENTRY             *Link;
  H2O_BDS_LOAD_OPTION    *CurrentLoadOption;


  OptionList  = DriverOrBoot == BOOT_OPTION ? &mBootList : &mDriverList;
  while (!IsListEmpty (OptionList)) {
    Link = GetFirstNode (OptionList);
    RemoveEntryList (Link);
    CurrentLoadOption = BDS_OPTION_FROM_LINK (Link);
    BdsServicesFreeLoadOption (&mH2OBdsServices, CurrentLoadOption);
  }
}

/**
  Internal function to check device path in H2O_BDS_LOAD_OPTION and device path in variable are the same.

  @param[in]  LoadOption   A pointer to BDS load option.

  @retval TRUE      The device path is match.
  @retval FALSE     The device path isn't match.
**/
STATIC
BOOLEAN
IsDevicePathMatch (
  IN H2O_BDS_LOAD_OPTION    *LoadOption
  )
{
  CHAR16                       OptionName[11];
  EFI_STATUS                   Status;
  UINT8                        *EfiLoadOption;
  UINTN                        EfiLoadOptionSize;
  UINT8                        *WorkingPtr;
  CHAR16                       *Description;
  EFI_DEVICE_PATH_PROTOCOL     *DevicePath;

  if (LoadOption == NULL) {
    return FALSE;
  }

  if (LoadOption->DriverOrBoot == BOOT_OPTION) {
    UnicodeSPrint (OptionName, sizeof (OptionName),  L"Boot%04x" , LoadOption->LoadOrder);
  } else {
    UnicodeSPrint (OptionName, sizeof (OptionName),  L"Driver%04x", LoadOption->LoadOrder);
  }
  Status = CommonGetVariableDataAndSize (
             OptionName,
             &gEfiGlobalVariableGuid,
             &EfiLoadOptionSize,
             (VOID **) &EfiLoadOption
             );
  if (Status != EFI_SUCCESS) {
    return FALSE;
  }

  WorkingPtr = (UINT8 *) EfiLoadOption;
  WorkingPtr += (sizeof (UINT32) + sizeof (UINT16));
  Description = (CHAR16 *) WorkingPtr;
  WorkingPtr += StrSize (Description);
  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) WorkingPtr;

  if (GetDevicePathSize (LoadOption->DevicePath) != GetDevicePathSize (DevicePath) ||
      CompareMem (LoadOption->DevicePath, DevicePath, GetDevicePathSize (DevicePath)) != 0) {
    return FALSE;
  }
  return TRUE;
}

/**
  Synchronize boot list with BootOrder and Boot#### variables or DriverOrder and Driver#### variables.

  @param[in] DriverOrBoot   Boolean that specifies whether this load option represents a Driver load option (FALSE)
                             or Boot load option (TRUE).

  @retval EFI_SUCCESS      Synchronize boot list successfully.
**/
STATIC
EFI_STATUS
SyncBdsLoadList (
  IN  BOOLEAN        DriverOrBoot
  )
{
  UINT16                 *OptionOrder;
  UINTN                  OptionOrderSize;
  UINTN                  OptionOrderCount;
  LIST_ENTRY             *OptionList;
  LIST_ENTRY             *Link;
  LIST_ENTRY             *CurrentLink;
  H2O_BDS_LOAD_OPTION    *CurrentLoadOption;
  UINTN                  Index;
  H2O_BDS_LOAD_OPTION    *LoadOption;
  EFI_STATUS             Status;

  OptionOrder     = NULL;
  OptionOrderSize = 0;
  Status = CommonGetVariableDataAndSize (
             DriverOrBoot == BOOT_OPTION ? L"BootOrder" : L"DriverOrder",
             &gEfiGlobalVariableGuid,
             &OptionOrderSize,
             (VOID **) &OptionOrder
             );
  //
  // Remove redundant bds boot option.
  //
  OptionList  = DriverOrBoot == BOOT_OPTION ? &mBootList : &mDriverList;
  for (Link = GetFirstNode (OptionList); !IsNull (OptionList, Link); Link = GetNextNode (OptionList, Link)) {
    CurrentLoadOption = BDS_OPTION_FROM_LINK (Link);
    if (!IsOptionNumInBootOrder (CurrentLoadOption->LoadOrder, OptionOrder, OptionOrderSize / 2) || !IsDevicePathMatch (CurrentLoadOption)) {
      CurrentLink = Link;
      Link = CurrentLink->BackLink;
      RemoveEntryList (&CurrentLoadOption->Link);
      BdsServicesFreeLoadOption  (&mH2OBdsServices, CurrentLoadOption);
    }
  }
  //
  // Add BDS boot option.
  //
  OptionOrderCount = OptionOrderSize / sizeof (UINT16);
  for (Index = 0; Index < OptionOrderCount; Index++) {
    if (!IsOptionNumInBootList (OptionOrder[Index], OptionList)) {
      LoadOption = GetNewBdsBootOption (OptionOrder[Index]);
      if (LoadOption == NULL) {
        continue;
      }
      Status = BdsServicesInsertLoadOption (&mH2OBdsServices, LoadOption);
      if (Status != EFI_SUCCESS) {
        continue;
      }
    }
  }

  if (!IsLoadOptionListCorrect (DriverOrBoot, OptionOrder, OptionOrderSize / sizeof (UINT16))) {
    if (mNeedSyncBootOrder) {
      mNeedSyncBootOrder = FALSE;
      RemoveAllLoadOptions (DriverOrBoot);
      SyncBdsLoadList (DriverOrBoot);
      for (Link = GetFirstNode (OptionList); !IsNull (OptionList, Link); Link = GetNextNode (OptionList, Link)) {
        CurrentLoadOption = BDS_OPTION_FROM_LINK (Link);
        CurrentLoadOption->Connected = TRUE;
      }
    }
  }
  mNeedSyncBootOrder = TRUE;

  if (DriverOrBoot == DRIVER_OPTION) {
    return EFI_SUCCESS;
  }
  //
  // Expand all of BDS boot option
  //
  for (Link = GetFirstNode (OptionList); !IsNull (OptionList, Link); Link = GetNextNode (OptionList, Link)) {
    CurrentLoadOption = BDS_OPTION_FROM_LINK (Link);
    BdsServicesExpandLoadOption  (&mH2OBdsServices, CurrentLoadOption);
  }
  return EFI_SUCCESS;
}

/**
  According to order in Boot list to update boot priority in BBS table.

  @param[in] *LegacyBios   A pointer to EFI_LEGACY_BIOS_PROTOCOL instance.

  @retval EFI_SUCCESS            Update boot priority successfully.
  @retval EFI_INVALID_PARAMETER  LegacyBios is NULL.
**/
STATIC
EFI_STATUS
UpdateBbsPriority (
  IN  EFI_LEGACY_BIOS_PROTOCOL          *LegacyBios
  )
{
  BBS_TABLE                     *LocalBbsTable;
  UINT16                        BbsIndex;
  UINT16                        PriorityIndex;
  LIST_ENTRY                    *OptionList;
  LIST_ENTRY                    *Link;
  H2O_BDS_LOAD_OPTION           *BootOption;
  BOOLEAN                       BootCurrentFound;

  if (LegacyBios == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  BootCurrentFound  = FALSE;
  OptionList        = &mBootList;
  for (Link = GetFirstNode (OptionList), PriorityIndex = 0; !IsNull (OptionList, Link);
       Link = GetNextNode (OptionList, Link)) {
    BootOption = BDS_OPTION_FROM_LINK (Link);
    if (!IsLegacyBootOption (BootOption->DevicePath)) {
      continue;
    }
    BbsIndex = *((UINT16 *) ((UINT8 *) BootOption->LoadOptionalData + sizeof (BBS_TABLE)));
    LegacyBios->GetBbsInfo (LegacyBios, NULL, NULL, NULL, &LocalBbsTable);
    LocalBbsTable[BbsIndex].BootPriority = (UINT16) PriorityIndex;
    if (!BootCurrentFound) {
      if (BootOption->LoadOrder == mH2OBdsServices.BootCurrentLoadOption->LoadOrder) {
        LocalBbsTable[BbsIndex].BootPriority = 0;
        BootCurrentFound = TRUE;
      } else {
        LocalBbsTable[BbsIndex].BootPriority++;
      }
    }
    PriorityIndex++;
  }

  return EFI_SUCCESS;
}

/**
  Boot the legacy system with the boot option

  @param  Option                 The legacy boot option which have BBS device path

  @retval EFI_SUCCESS            It will never return EFI_SUCCESS. If boot success, system will enter
                                 legacy OS directly.
  @retval EFI_UNSUPPORTED        There is no legacybios protocol, do not support legacy boot.
  @retval Other                  Return the status of LegacyBios->LegacyBoot ().
**/
EFI_STATUS
LauchLegacyBootOption (
  IN  H2O_BDS_LOAD_OPTION         *Option
  )
{
  EFI_STATUS                Status;
  EFI_LEGACY_BIOS_PROTOCOL  *LegacyBios;

  Status = gBS->LocateProtocol (&gEfiLegacyBiosProtocolGuid, NULL, (VOID **) &LegacyBios);
  if (Status) {
    //
    // If no LegacyBios protocol we do not support legacy boot
    //
    return EFI_UNSUPPORTED;
  }
  //
  // Notes: if we separate the int 19, then we don't need to refresh BBS
  //
  // to set BBS Table priority
  //
  UpdateBbsPriority (LegacyBios);
  //
  // Write boot to OS performance data for legacy boot.
  //
  WRITE_BOOT_TO_OS_PERFORMANCE_DATA;

  DEBUG ((DEBUG_INFO | DEBUG_LOAD, "Legacy Boot: %S\n", Option->Description));
  return LegacyBios->LegacyBoot (
                      LegacyBios,
                      (BBS_BBS_DEVICE_PATH *) Option->DevicePath,
                      Option->LoadOptionalDataSize,
                      Option->LoadOptionalData
                      );
}

/**
  Callback function to clear screen to prevent from screen has some garbage after
  booting to shell environment.

  @param[in] Event    Event whose notification function is being invoked.
  @param[in] Context  Pointer to the notification function's context.
**/
VOID
EFIAPI
ShellEnvProtocolCallback (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  )
{
  gST->ConOut->ClearScreen(gST->ConOut);
}

/**
  According to device path and image handle to boot.

  @param[in]  ImageHandle       Image handle.
  @param[in]  Option            The boot option need to be processed.
  @param[out] ExitDataSize      Returned directly from gBS->StartImage ().
  @param[out] ExitData          Returned directly from gBS->StartImage ().

  @retval EFI_SUCCESS          Boot from recovery boot option successfully.
  @retval Other                Some errors occured during boot process.
**/
STATIC
EFI_STATUS
LaunchBootImage (
  IN  EFI_HANDLE                    ImageHandle,
  IN  H2O_BDS_LOAD_OPTION           *Option,
  OUT UINTN                         *ExitDataSize,
  OUT CHAR16                        **ExitData OPTIONAL
  )
{
  EFI_STATUS                        Status;
  EFI_LOADED_IMAGE_PROTOCOL         *ImageInfo;
  EFI_EVENT                         ShellImageEvent;
  EFI_GUID                          ShellEnvProtocol = SHELL_ENVIRONMENT_INTERFACE_PROTOCOL;


  Status = gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &ImageInfo);
  ASSERT_EFI_ERROR (Status);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  if (Option->LoadOptionalDataSize != 0) {
    ImageInfo->LoadOptionsSize  = Option->LoadOptionalDataSize;
    ImageInfo->LoadOptions      = Option->LoadOptionalData;
  }

  //
  // Set a monitor to its native resolution when boot to UEFI Windows and update Bgrt if needed
  //
  //
  // Check Image is PXE image, DEVICE PATH will carry Message MAC device information
  //
  if (IsPxeBoot (Option->DevicePath)) {
    //
    // If is PXE image, perpare to TEXT mode via DisableQuietBoot
    //
    DisableQuietBoot ();
  }

  //
  // Register Event for Shell Image
  //
  if (mShellEnvProtocolCallbackReg == NULL) {
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    ShellEnvProtocolCallback,
                    NULL,
                    &ShellImageEvent
                    );
    if (!EFI_ERROR (Status)) {
      Status = gBS->RegisterProtocolNotify (
                      &ShellEnvProtocol,
                      ShellImageEvent,
                      &mShellEnvProtocolCallbackReg
                      );
    }
  }
  TriggerCpBootBefore ();
  //
  // Before calling the image, enable the Watchdog Timer for the 5 Minute period
  //
  gBS->SetWatchdogTimer (5 * 60, 0x0000, 0x00, NULL);

  //
  // PostCode = 0xFB, UEFI Boot Start Image
  //
  POST_CODE (POST_BDS_START_IMAGE);

  PERF_END (0, "PostBDS", NULL, 0);
  WRITE_BOOT_TO_OS_PERFORMANCE_DATA;
  if (FeaturePcdGet (PcdAutoCreateDummyBootOption)) {
    BdsLibChangeToVirtualBootOrder ();
  }

  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Image Start...\n"));

  //
  // Report status code for OS Loader StartImage.
  //
  REPORT_STATUS_CODE (EFI_PROGRESS_CODE, PcdGet32 (PcdProgressCodeOsLoaderStart));

  Status = gBS->StartImage (ImageHandle, ExitDataSize, ExitData);
  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Image Return Status = %r\n", Status));

  //
  // Clear the Watchdog Timer after the image returns
  //
  gBS->SetWatchdogTimer (0x0000, 0x0000, 0x0000, NULL);
  if (FeaturePcdGet (PcdAutoCreateDummyBootOption)) {
    SyncBootOrder ();
    BdsLibRestoreBootOrderFromPhysicalBootOrder ();
  }
  TriggerCpBootAfter ();
  return Status;
}

/**
  According to device path  to boot from reocvery boot option.

  @param[in]  Option           The boot option need to be processed.
  @param[in]  ExitDataSize     Returned directly from gBS->StartImage ().
  @param[in]  ExitData         Returned directly from gBS->StartImage ().

  @retval EFI_SUCCESS          Boot from recovery boot option successfully.
  @retval Other                Some errors occured during boot process.
**/
STATIC
EFI_STATUS
LaunchRecoveryOption (
  IN  H2O_BDS_LOAD_OPTION           *Option,
  OUT UINTN                         *ExitDataSize,
  OUT CHAR16                        **ExitData OPTIONAL
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  EFI_HANDLE                Handle;
  EFI_STATUS                Status;
  EFI_HANDLE                ImageHandle;


  DevicePath = Option->DevicePath;
  Status = gBS->LocateDevicePath (
                  &gEfiSimpleFileSystemProtocolGuid,
                  &DevicePath,
                  &Handle
                  );
  if (!EFI_ERROR (Status)) {
    FilePath = FileDevicePath (Handle, EFI_REMOVABLE_MEDIA_FILE_NAME);
    if (FilePath != NULL) {
      Status = gBS->LoadImage (
                      TRUE,
                      gImageHandle,
                      FilePath,
                      NULL,
                      0,
                      &ImageHandle
                      );
      if (!EFI_ERROR (Status)) {
        Status = LaunchBootImage (ImageHandle, Option, ExitDataSize, ExitData);
      }

      gBS->FreePool (FilePath);
    }
  }

  return Status;
}

/**
  Launch a BDS boot option.

  @param[in]  BdsLoadOption      A pointer to BDS load option.
  @param[out] ExitDataSize       Pointer to the size, in bytes, of ExitData.
  @param[out] ExitData           Pointer to a pointer to a data buffer that includes a Null-terminated
                                 string, optionally followed by additional binary data.

  @retval EFI_SUCCESS            Boot from the input boot option successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL.
  @retval EFI_NOT_FOUND          If the Device Path is not found in the system
**/
STATIC
EFI_STATUS
LaunchBootOption (
  IN OUT  H2O_BDS_LOAD_OPTION      *BdsLoadOption,
  OUT     UINTN                    *ExitDataSize,
  OUT     CHAR16                   **ExitData      OPTIONAL
  )
{
  EFI_STATUS                                      Status;
  EFI_STATUS                                      RecoveryBootStatus;
  EFI_HANDLE                                      Handle;
  EFI_HANDLE                                      ImageHandle;
  EFI_DEVICE_PATH_PROTOCOL                        *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL                        *FilePath;
  EFI_ACPI_S3_SAVE_PROTOCOL                       *AcpiS3Save;
  EFI_BLOCK_IO_PROTOCOL                           *BlkIo;
  VOID                                            *Buffer;
  EFI_STATUS                                      LocateDevicePathStatus;
  EFI_TREE_MEASURE_OS_LOADER_AUTHORITY_PROTOCOL   *MeasureOsLoaderAuthorityProtocol;
  H2O_BDS_LOAD_OPTION                             *CurrentBootOption;
  LIST_ENTRY                                      *Link;

  //
  // PostCode = 0x2C, Shadow Misc Option ROM
  //
  POST_CODE (BDS_EFI64_SHADOW_ALL_LEGACY_ROM);

  //
  // All the driver options should have been processed since now boot will be performed.
  //
  PERF_END (0, BDS_TOK, NULL, 0);

  PERF_START (0, "PostBDS", NULL, 0);

  if (BdsLoadOption == NULL ) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Before boot to device, always clean BootNext variable.
  //
  gRT->SetVariable (
         L"BootNext",
         &gEfiGlobalVariableGuid,
         0,
         0,
         NULL
         );

  //
  // Signal the EVT_SIGNAL_READY_TO_BOOT event
  //
  //
  // PostCode = 0x2E, Last Chipset initial before boot to OS
  //
  TriggerCpReadyToBootBefore ();
  POST_CODE (BDS_READY_TO_BOOT_EVENT);
  EfiSignalEventReadyToBoot();
  TriggerCpReadyToBootAfter ();
  //
  // S3Save should be executed after EventReadyToBoot
  //
  Status = gBS->LocateProtocol (&gEfiAcpiS3SaveProtocolGuid, NULL, (VOID **) &AcpiS3Save);
  if (!EFI_ERROR (Status)) {
    AcpiS3Save->S3Save (AcpiS3Save, NULL);
  }

  //
  // For a temporary boot (i.e. a boot by selected a EFI Shell using "Boot From File"), Boot Current is actually not valid.
  // In this case, "BootCurrent" is not created.
  // Only create the BootCurrent variable when it points to a valid Boot#### variable.
  //
  gRT->SetVariable (
        EFI_BOOT_CURRENT_VARIABLE_NAME,
        &gEfiGlobalVariableGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        sizeof (UINT16),
        &BdsLoadOption->LoadOrder
        );
  BdsServicesConvertVarToLoadOption (
    &mH2OBdsServices,
    EFI_BOOT_CURRENT_VARIABLE_NAME,
    &gEfiGlobalVariableGuid,
    &mH2OBdsServices.BootCurrentLoadOption
    );

  ASSERT (BdsLoadOption->DevicePath != NULL);
  if ((DevicePathType (BdsLoadOption->DevicePath) == BBS_DEVICE_PATH) &&
      (DevicePathSubType (BdsLoadOption->DevicePath) == BBS_BBS_DP)
     ) {
    //
    // Check to see if we should legacy BOOT. If yes then do the legacy boot
    //
    if (FeaturePcdGet (PcdAutoCreateDummyBootOption) && BdsLibIsBootOrderHookEnabled ()) {
      BdsLibRestoreBootOrderFromPhysicalBootOrder ();
    }
    //
    // PostCode = 0x2F, Start to boot Legacy OS
    //
    POST_CODE (BDS_GO_LEGACY_BOOT);
    return LauchLegacyBootOption (BdsLoadOption);
  }
  //
  // Drop the TPL level from EFI_TPL_DRIVER to EFI_TPL_APPLICATION
  //
  //
  // PostCode = 0x30, Start to boot UEFI OS
  //
  POST_CODE (BDS_GO_UEFI_BOOT);

#ifndef MDEPKG_NDEBUG
  DumpMemoryMap();
#endif
  if (FeaturePcdGet (PcdMemoryMapConsistencyCheck)) {
    CheckRtAndBsMemUsage ();
  }



  DEBUG_CODE_BEGIN();
  if (BdsLoadOption->Description == NULL) {
    DEBUG ((DEBUG_INFO | DEBUG_LOAD, "Booting from unknown device path\n"));
  } else {
    DEBUG ((DEBUG_INFO | DEBUG_LOAD, "Booting %S\n", BdsLoadOption->Description));
  }
  DEBUG_CODE_END();

  EnableOptimalTextMode ();

  //
  // Report status code for OS Loader LoadImage.
  //
  REPORT_STATUS_CODE (EFI_PROGRESS_CODE, PcdGet32 (PcdProgressCodeOsLoaderLoad));



  if (IsPxeBoot (BdsLoadOption->DevicePath)) {
    DisableQuietBoot ();
  }

  Status = gBS->LocateProtocol (
                &gEfiTrEEMeasureOsLoaderAuthorityProtocolGuid,
                NULL,
                (VOID**)&MeasureOsLoaderAuthorityProtocol
                );
  if (!EFI_ERROR(Status)) {
    MeasureOsLoaderAuthorityProtocol->OsLoader = TRUE;
  }

  CurrentBootOption = BdsLoadOption;
  if (BdsLoadOption->Expanded) {
    if (IsListEmpty (&BdsLoadOption->ExpandedLoadOptions)) {
      return EFI_NOT_FOUND;
    }
    Link = GetFirstNode (&BdsLoadOption->ExpandedLoadOptions);
    CurrentBootOption = BDS_OPTION_FROM_LINK (Link);
  }

  Status = EFI_NOT_FOUND;
  ImageHandle = NULL;
  while (CurrentBootOption != NULL) {
    Status = gBS->LoadImage (
                    TRUE,
                    gImageHandle,
                    CurrentBootOption->DevicePath,
                    NULL,
                    0,
                    &ImageHandle
                    );
    //
    // If we didn't find an image, we may need to load the default boot behavior for the device.
    //
    if (EFI_ERROR (Status)) {
      //
      // Find a Simple File System protocol on the device path.
      // If the remaining device path is set to end then no Files are being specified, so try the removable media file name.
      //
      TempDevicePath = CurrentBootOption->DevicePath;
      LocateDevicePathStatus = gBS->LocateDevicePath (
                                      &gEfiSimpleFileSystemProtocolGuid,
                                      &TempDevicePath,
                                      &Handle
                                      );
      if (!EFI_ERROR (LocateDevicePathStatus) && IsDevicePathEnd (TempDevicePath)) {
        FilePath = FileDevicePath (Handle, EFI_REMOVABLE_MEDIA_FILE_NAME);
        if (FilePath) {
          //
          // Issue a dummy read to the device to check for media change.
          // When the removable media is changed, any Block IO read/write will cause the BlockIo protocol be reinstalled and EFI_MEDIA_CHANGED is returned.
          // After the Block IO protocol is reinstalled, subsequent Block IO read/write will success.
          //
          Status = gBS->HandleProtocol (
                          Handle,
                          &gEfiBlockIoProtocolGuid,
                          (VOID **) &BlkIo
                          );
          if (!EFI_ERROR (Status)) {
            Buffer = AllocatePool (BlkIo->Media->BlockSize);
            if (Buffer != NULL) {
              BlkIo->ReadBlocks (
                       BlkIo,
                       BlkIo->Media->MediaId,
                       0,
                       BlkIo->Media->BlockSize,
                       Buffer
                       );
              gBS->FreePool (Buffer);
            }
          }

          Status = gBS->LoadImage (
                          TRUE,
                          gImageHandle,
                          FilePath,
                          NULL,
                          0,
                          &ImageHandle
                          );
          gBS->FreePool (FilePath);
        }
      }
    }
    if (Status == EFI_SUCCESS || !BdsLoadOption->Expanded) {
      break;
    }
    Link = GetNextNode (&BdsLoadOption->ExpandedLoadOptions, &CurrentBootOption->Link);
    if (Link == &BdsLoadOption->ExpandedLoadOptions) {
      CurrentBootOption = NULL;
    } else {
      CurrentBootOption = BDS_OPTION_FROM_LINK (Link);
    }
  }

  if (Status == EFI_SUCCESS) {
    BitlockerVirtualKeyboardSupport (CurrentBootOption->DevicePath);

    //
    // Provide the image with it's load options
    //
    Status = LaunchBootImage (ImageHandle, CurrentBootOption, ExitDataSize, ExitData);

  }

  if (CurrentBootOption == NULL && BdsLoadOption->Expanded) {
    CurrentBootOption = BDS_OPTION_FROM_LINK (BdsLoadOption->ExpandedLoadOptions.BackLink);
  }
  if (EFI_ERROR (Status) && IsUefiOsFilePath (CurrentBootOption->DevicePath)) {
    //
    // Try to boot from recovery boot option and only update status to EFI_SUCCESS,
    // if boot from this recovery file path is successful.
    //
    RecoveryBootStatus = LaunchRecoveryOption (CurrentBootOption, ExitDataSize, ExitData);
    if (!EFI_ERROR (RecoveryBootStatus)) {
      Status = EFI_SUCCESS;
    }
  }
  //
  // Clear Boot Current
  //
  gRT->SetVariable (
        EFI_BOOT_CURRENT_VARIABLE_NAME,
        &gEfiGlobalVariableGuid,
        0,
        0,
        NULL
        );
  if (mH2OBdsServices.BootCurrentLoadOption != NULL) {
    BdsServicesFreeLoadOption (&mH2OBdsServices, mH2OBdsServices.BootCurrentLoadOption);
    mH2OBdsServices.BootCurrentLoadOption = NULL;
  }
  gRT->SetVariable (
         H2O_BOOT_PREVIOUS_VARIABLE_NAME,
         &gEfiGenericVariableGuid,
         EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
         sizeof (UINT16),
         &BdsLoadOption->LoadOrder
         );
  if (mH2OBdsServices.BootPrevLoadOption != NULL) {
    BdsServicesFreeLoadOption (&mH2OBdsServices, mH2OBdsServices.BootPrevLoadOption);
    mH2OBdsServices.BootPrevLoadOption = NULL;
  }
  BdsServicesConvertVarToLoadOption (
    &mH2OBdsServices,
    H2O_BOOT_PREVIOUS_VARIABLE_NAME,
    &gEfiGenericVariableGuid,
    &mH2OBdsServices.BootPrevLoadOption
    );
  //
  // Signal BIOS after the image returns
  //
  SignalImageReturns ();

  return Status;
}

/**
  Launch a BDS driver option.

  @param[in]  BdsLoadOption      A pointer to BDS load option.
  @param[out] ExitDataSize       Pointer to the size, in bytes, of ExitData.
  @param[out] ExitData           Pointer to a pointer to a data buffer that includes a Null-terminated
                                 string, optionally followed by additional binary data.

  @retval EFI_SUCCESS            Boot from the input boot option successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL.
  @retval EFI_NOT_FOUND          If the Device Path is not found in the system
**/
STATIC
EFI_STATUS
LaunchDriverOption (
  IN OUT  H2O_BDS_LOAD_OPTION      *BdsLoadOption,
  OUT     UINTN                    *ExitDataSize,
  OUT     CHAR16                   **ExitData      OPTIONAL
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                ImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL *ImageInfo;

  if (BdsLoadOption == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((BdsLoadOption->Attributes & LOAD_OPTION_ACTIVE) != LOAD_OPTION_ACTIVE) {
    return EFI_SUCCESS;
  }
  //
  // Make sure the driver path is connected.
  //
  BdsLibConnectDevicePath (BdsLoadOption->DevicePath);

  //
  // Load and start the image that Driver#### describes
  //
  Status = gBS->LoadImage (
                  FALSE,
                  gImageHandle,
                  BdsLoadOption->DevicePath,
                  NULL,
                  0,
                  &ImageHandle
                  );
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &ImageInfo);
  //
  // Verify whether this image is a driver, if not,
  // exit it and continue to parse next load option
  //
  if (ImageInfo->ImageCodeType != EfiBootServicesCode && ImageInfo->ImageCodeType != EfiRuntimeServicesCode) {
    gBS->Exit (ImageHandle, EFI_INVALID_PARAMETER, 0, NULL);
    return EFI_UNSUPPORTED;
  }

  if (BdsLoadOption->LoadOptionalDataSize != 0) {
    ImageInfo->LoadOptionsSize  = BdsLoadOption->LoadOptionalDataSize;
    ImageInfo->LoadOptions      = BdsLoadOption->LoadOptionalData;
  }
  //
  // Before calling the image, enable the Watchdog Timer for
  // the 5 Minute period
  //
  gBS->SetWatchdogTimer (5 * 60, 0x0000, 0x00, NULL);

  Status = gBS->StartImage (ImageHandle, ExitDataSize, ExitData);
  DEBUG ((DEBUG_INFO | DEBUG_LOAD, "Driver Return Status = %r\n", Status));

  //
  // Clear the Watchdog Timer after the image returns
  //
  gBS->SetWatchdogTimer (0x0000, 0x0000, 0x0000, NULL);

  return EFI_SUCCESS;
}

/**
  Initialize gH2OBdsServicesProtocolGuid protocol data and install gH2OBdsServicesProtocolGuid protocol.

  @retval EFI_SUCCESS           Install gH2OBdsServicesProtocolGuid protocol successfully.
  @return Others                Any error occurred while installing gH2OBdsServicesProtocolGuid protocol.
**/
EFI_STATUS
InstallH2OBdsServicesProtocol (
  VOID
  )
{
  EFI_HANDLE             Handle;
  EFI_STATUS             Status;
  H2O_BDS_LOAD_OPTION    *LoadOption;

  InitializeListHead (&mBootList);
  InitializeListHead (&mDriverList);
  Status = BdsServicesConvertVarToLoadOption (&mH2OBdsServices, EFI_BOOT_NEXT_VARIABLE_NAME, &gEfiGlobalVariableGuid, &LoadOption);
  if (Status == EFI_SUCCESS) {
    mH2OBdsServices.BootNextLoadOption = LoadOption;
  }
  Handle = NULL;
  return gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gH2OBdsServicesProtocolGuid,
                  &mH2OBdsServices,
                  NULL
                  );
}

/**
  Return the current value of the OsIndications and OsIndicationsSupported UEFI variable.

  @param[in]  This                    A Pointer to current instance of this protocol.
  @param[out] OsIndications           A pointer to contain the value of the OsIndications.
  @param[out] OsIndicationsSupported  A pointer to contain the value of the OsIndicationsSupported.

  @retval EFI_SUCCESS            Get OsIndications and OsIndicationsSupported successfully.
  @retval EFI_INVALID_PARAMETER  OsIndications or OsIndicationsSupported is NULL.
  @retval EFI_NOT_FOUND          Cannot find OsIndications or OsIndicationsSupported value.
**/
EFI_STATUS
EFIAPI
BdsServicesGetOsIndications (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  UINT64                      *OsIndications,
  OUT  UINT64                      *OsIndicationsSupported
  )
{
  EFI_STATUS     Status;
  UINT64         Indications;
  UINT64         IndicationsSupport;
  UINTN          Size;

  if (OsIndications == NULL || OsIndicationsSupported == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Size = sizeof (Indications);
  Status = CommonGetVariable (
             EFI_OS_INDICATIONS_VARIABLE_NAME,
             &gEfiGlobalVariableGuid,
             &Size,
             &Indications
             );
  if (Status != EFI_SUCCESS) {
    return Status;
  }
  Size = sizeof (IndicationsSupport);
  Status = CommonGetVariable (
             EFI_OS_INDICATIONS_SUPPORT_VARIABLE_NAME,
             &gEfiGlobalVariableGuid,
             &Size,
             &IndicationsSupport
             );
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  *OsIndications = Indications;
  *OsIndicationsSupported = IndicationsSupport;

  return EFI_SUCCESS;
}

/**
  Return the current value of the OS loader timeout.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[out] Timeout            A pointer to contain the value of the timeout.

  @retval EFI_SUCCESS            Get Timeout value successfully.
  @retval EFI_INVALID_PARAMETER  Timeout is NULL.
  @retval EFI_NOT_FOUND          Cannot find Timeout value.
**/
EFI_STATUS
EFIAPI
BdsServicesGetTimeout (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  UINT16                      *Timeout
  )
{
  UINTN       Size;

  if (Timeout == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Size = sizeof (UINT16);
  return CommonGetVariable (
           EFI_TIME_OUT_VARIABLE_NAME,
           &gEfiGlobalVariableGuid,
           &Size,
           Timeout
           );
}

/**
  Return the current boot mode, such as S4 resume, diagnostics, full configuration, etc.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BootMode           A pointer to contain the value of the boot mode.

  @retval EFI_SUCCESS            Get BootMode successfully.
  @retval EFI_INVALID_PARAMETER  BootMode is NULL.
**/
EFI_STATUS
EFIAPI
BdsServicesGetBootMode (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  EFI_BOOT_MODE               *BootMode
  )
{
  if (BootMode == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  *BootMode = GetBootModeHob ();

  return EFI_SUCCESS;
}

/**
  Return the current boot type, uch as legacy, UEFI or dual.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BootType           A pointer to contain the value of the boot mode.

  @retval EFI_SUCCESS            Get BootType successfully.
  @retval EFI_INVALID_PARAMETER  BootType is NULL.
**/
EFI_STATUS
EFIAPI
BdsServicesGetBootType (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  UINT8                       *BootType
  )
{
  UINTN                        Size;

  if (BootType == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Size = sizeof (UINT8);
  *BootType = DUAL_BOOT_TYPE;
  CommonGetVariable (
         L"BootType",
         &gSystemConfigurationGuid,
         &Size,
         BootType
         );

  return EFI_SUCCESS;
}

/**
  Return linked list of BDS boot options derived from the BootOrder and Boot#### UEFI variables.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[out] BootList           Pointer to linked list of BDS boot options derived from the BootOrder and Boot#### UEFI variables.

  @retval EFI_SUCCESS            Get boot list successfully.
**/
EFI_STATUS
EFIAPI
BdsServicesGetBootList (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  LIST_ENTRY                  **BootList
  )
{
  SyncBdsLoadList (BOOT_OPTION);
  *BootList = &mBootList;
  return EFI_SUCCESS;
}

/**
  Return linked list of BDS boot options derived from the DriverOrder and Driver#### UEFI variables.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[out] DriverList         Pointer to linked list of BDS boot options derived from the DriverOrder and Driver#### UEFI variables.

  @retval EFI_SUCCESS            Get driver list successfully.
**/
EFI_STATUS
EFIAPI
BdsServicesGetDriverList (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  OUT  LIST_ENTRY                  **DriverList
  )
{
  SyncBdsLoadList (DRIVER_OPTION);
  *DriverList = &mDriverList;
  return EFI_SUCCESS;
}

/**
  Get load order from input load order type.

  @param[in]  Type               Input load order type.

  @return   the load order number get from load order type.
**/
STATIC
UINT16
GetLoadOrderFromType (
  IN  UINT16   Type
  )
{
  UINT16       LoadOrder;
  UINTN        DataSize;


  LoadOrder = BOOT_NEXT_LOAD_ORDER;
  if (Type != BOOT_PREVIOUS_LOAD_ORDER && Type != BOOT_CURRENT_LOAD_ORDER && Type != BOOT_NEXT_LOAD_ORDER) {
    return LoadOrder;
  }

  DataSize = sizeof (UINT16);
  switch (Type) {

  case BOOT_PREVIOUS_LOAD_ORDER:
    gRT->GetVariable (H2O_BOOT_PREVIOUS_VARIABLE_NAME, &gEfiGenericVariableGuid, NULL, &DataSize, &LoadOrder);
    break;

  case BOOT_CURRENT_LOAD_ORDER:
    gRT->GetVariable (EFI_BOOT_CURRENT_VARIABLE_NAME, &gEfiGlobalVariableGuid, NULL, &DataSize, &LoadOrder);
    break;

  case BOOT_NEXT_LOAD_ORDER:
    gRT->GetVariable (EFI_BOOT_NEXT_VARIABLE_NAME, &gEfiGlobalVariableGuid, NULL, &DataSize, &LoadOrder);
    break;
  }

  return LoadOrder;
}


/**
  Create a BDS load option in a buffer allocated from pool.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  DriverOrBoot       Boolean that specifies whether the Boot Manager has connected this device
                                 path (TRUE) or not (FALSE).
  @param[in]  OptionName         A Null-terminated string that is the name of the vendor's variable.
  @param[in]  OptionGuid         A unique identifier for the vendor.
  @param[in]  Attributes         The attributes for this load option entry.
  @param[in]  DevicePath         A Pointer to a packed array of UEFI device paths.
  @param[in]  Description        The user readable description for the load option.
  @param[in]  OptionalData       A Pointer to optional data for load option.
  @param[in]  OptionalDataSize   The size by bytes of optional data.
  @param[out] LoadOption         Dynamically allocated memory that contains a new created H2O_BDS_LOAD_OPTION
                                 instance. Caller is responsible freeing the buffer

  @retval EFI_SUCCESS            Create load option successfully.
  @retval EFI_INVALID_PARAMETER  DevicePath is NULL or LoadOption is NULL.
  @retval EFI_INVALID_PARAMETER  The OptionName is correct boot#### or driver#### variable name.
  @retval EFI_OUT_OF_RESOURCES   Allocate memory to create H2O_BDS_LOAD_OPTION failed.
**/
EFI_STATUS
EFIAPI
BdsServicesCreateLoadOption (
  IN        H2O_BDS_SERVICES_PROTOCOL    *This,
  IN        BOOLEAN                      DriverOrBoot,
  IN CONST  CHAR16                       *OptionName OPTIONAL,
  IN CONST  EFI_GUID                     *OptionGuid OPTIONAL,
  IN        UINT32                       Attributes,
  IN CONST  EFI_DEVICE_PATH_PROTOCOL     *DevicePath,
  IN CONST  CHAR16                       *Description OPTIONAL,
  IN CONST  UINT8                        *OptionalData OPTIONAL,
  IN        UINT32                       OptionalDataSize,
  OUT       H2O_BDS_LOAD_OPTION          **LoadOption
  )
{
  H2O_BDS_LOAD_OPTION      *CurrentOption;
  EFI_STATUS               Status;

  if (DevicePath == NULL || LoadOption == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;
  CurrentOption = NULL;
  CurrentOption = AllocateZeroPool (sizeof (H2O_BDS_LOAD_OPTION));
  if (CurrentOption == NULL) {
   return EFI_OUT_OF_RESOURCES;
  }

  CurrentOption->Signature = H2O_BDS_LOAD_OPTION_SIGNATURE;
  InitializeListHead (&CurrentOption->Link);
  CurrentOption->DevicePath = AllocateCopyPool  (GetDevicePathSize (DevicePath), DevicePath);
  if (CurrentOption->DevicePath == NULL) {
    InternalFreeBdsLoadOption (CurrentOption);
    return EFI_OUT_OF_RESOURCES;
  }
  CurrentOption->Connected = FALSE;


  //
  // Determine this is boot option or driver option
  //
  if (OptionName != NULL && OptionGuid != NULL) {
    if (IsBootPreviousVariable (OptionName, OptionGuid)) {
      CurrentOption->DriverOrBoot = BOOT_OPTION;
      CurrentOption->LoadOrder = GetLoadOrderFromType (BOOT_PREVIOUS_LOAD_ORDER);
    } else if (IsBootCurrentVariable (OptionName, OptionGuid)) {
      CurrentOption->DriverOrBoot = BOOT_OPTION;
      CurrentOption->LoadOrder = GetLoadOrderFromType (BOOT_CURRENT_LOAD_ORDER);
    } else if (IsBootNextVariable (OptionName, OptionGuid)) {
      CurrentOption->DriverOrBoot = BOOT_OPTION;
      CurrentOption->LoadOrder = GetLoadOrderFromType (BOOT_NEXT_LOAD_ORDER);
    } else if (StrnCmp (OptionName, L"Boot", StrLen (L"Boot")) == 0 && StrLen (OptionName) == StrLen (L"Boot####") &&
               IsValidOptionNumber (&OptionName[StrLen (OptionName) - 4])) {
      CurrentOption->DriverOrBoot = BOOT_OPTION;
      CurrentOption->LoadOrder = GetOptionNumber (&OptionName[StrLen (OptionName) - 4]);
    } else if (StrnCmp (OptionName, L"Driver", StrLen (L"Driver")) == 0 && StrLen (OptionName) == StrLen (L"Driver####") &&
               IsValidOptionNumber (&OptionName[StrLen (OptionName) - 4])) {
      CurrentOption->DriverOrBoot = DRIVER_OPTION;
      CurrentOption->LoadOrder = GetOptionNumber (&OptionName[StrLen (OptionName) - 4]);
    } else {
      InternalFreeBdsLoadOption (CurrentOption);
      return EFI_INVALID_PARAMETER;
    }
    StrCpy (CurrentOption->LoadOrderVarName, OptionName);
    CopyGuid (&CurrentOption->LoadOrderVarGuid, OptionGuid);
  } else {
    CurrentOption->DriverOrBoot = DriverOrBoot;
  }

  CurrentOption->Expanded = FALSE;
  InitializeListHead (&CurrentOption->ExpandedLoadOptions);
  CurrentOption->Attributes = Attributes;

  if (Description != NULL) {
    CurrentOption->Description = AllocateCopyPool  (StrSize (Description), Description);
    if (CurrentOption->Description == NULL) {
      InternalFreeBdsLoadOption (CurrentOption);
      return EFI_OUT_OF_RESOURCES;
    }
  }
  if (OptionalDataSize != 0 && OptionalData != NULL &&
      !(AsciiStrnCmp ((CHAR8 *) OptionalData, "RC", 2) == 0 && (OptionalDataSize == 2 || OptionalDataSize == 6))) {
    CurrentOption->LoadOptionalData = AllocateCopyPool  (OptionalDataSize, OptionalData);
    if (CurrentOption->LoadOptionalData == NULL) {
      InternalFreeBdsLoadOption (CurrentOption);
      return EFI_OUT_OF_RESOURCES;
    }
    CurrentOption->LoadOptionalDataSize = OptionalDataSize;
  }

  *LoadOption = CurrentOption;
  return EFI_SUCCESS;


}

/**
  Free the memory associated with a BDS load option.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  LoadOption         The allocated H2O_BDS_LOAD_OPTION instance to free.

  @retval EFI_SUCCESS            Get BootType successfully.
  @retval EFI_INVALID_PARAMETER  LoadOption is NULL.
**/
EFI_STATUS
EFIAPI
BdsServicesFreeLoadOption (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   H2O_BDS_LOAD_OPTION         *LoadOption
  )
{
  LIST_ENTRY                        *ExpandedOptions;
  LIST_ENTRY                        *Link;
  LIST_ENTRY                        *CurrentLink;

  if (LoadOption == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (LoadOption->Expanded) {
    ExpandedOptions = &LoadOption->ExpandedLoadOptions;
    for (Link = GetFirstNode (ExpandedOptions); !IsNull (ExpandedOptions, Link); Link = GetNextNode (ExpandedOptions, Link)) {
       CurrentLink = Link;
       Link = CurrentLink->BackLink;
       RemoveEntryList (CurrentLink);
       InternalFreeBdsLoadOption (BDS_OPTION_FROM_LINK (CurrentLink));
    }
  }
  InternalFreeBdsLoadOption (LoadOption);
  return EFI_SUCCESS;
}

/**
  Converts a UEFI variable formatted as a UEFI load option to a BDS load option.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  VariableName       A Null-terminated string that is the name of the vendor's variable.
  @param[in]  VariableGuid       A unique identifier for the vendor.
  @param[out] BdsLoadOption      Dynamically allocated memory that contains a new created H2O_BDS_LOAD_OPTION
                                 instance. Caller is responsible freeing the buffer.

  @retval EFI_SUCCESS            Convert BDS load option to UEFI load option successfully.
  @retval EFI_INVALID_PARAMETER  VariableName is NULL VariableGuid is NULL or BdsLoadOption is NULL.
  @retval EFI_NOT_FOUND          Variable doesn't exist.
  @retval EFI_OUT_OF_RESOURCES   Allocate memory to create H2O_BDS_LOAD_OPTION failed.
**/
EFI_STATUS
EFIAPI
BdsServicesConvertVarToLoadOption (
  IN         H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   CONST CHAR16                      *VariableName,
  IN   CONST EFI_GUID                    *VariableGuid,
  OUT        H2O_BDS_LOAD_OPTION         **BdsLoadOption
  )
{
  EFI_STATUS                   Status;
  UINT8                        *EfiLoadOption;
  UINTN                        EfiLoadOptionSize;
  UINT8                        *WorkingPtr;
  UINT16                       DevicePathLength;
  CHAR16                       *Description;
  EFI_DEVICE_PATH_PROTOCOL     *DevicePath;
  UINTN                        UsedSize;
  UINT8                        *OptionalData;
  UINT32                       OptionalDataSize;
  UINT32                       Attributes;
  CHAR16                       OptionName[11];

  if (VariableName == NULL || VariableGuid == NULL || BdsLoadOption == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = CommonGetVariableDataAndSize (
             (CHAR16 *) VariableName,
             (EFI_GUID *) VariableGuid,
             &EfiLoadOptionSize,
             (VOID **) &EfiLoadOption
             );
  if (Status != EFI_SUCCESS) {
    return Status;
  }
  if (IsBootPreviousVariable (VariableName, VariableGuid) || IsBootCurrentVariable (VariableName, VariableGuid) ||
      IsBootNextVariable (VariableName, VariableGuid)) {
    UnicodeSPrint (OptionName, sizeof (OptionName), L"Boot%04x", *((UINT16 *) EfiLoadOption));
    FreePool (EfiLoadOption);
    Status = CommonGetVariableDataAndSize (
               OptionName,
               &gEfiGlobalVariableGuid,
               &EfiLoadOptionSize,
               (VOID **) &EfiLoadOption
               );
    if (Status != EFI_SUCCESS) {
      return Status;
    }
  }

  WorkingPtr = (UINT8 *) EfiLoadOption;
  Attributes = *((UINT32 *) WorkingPtr);
  WorkingPtr += sizeof (UINT32);
  DevicePathLength = *((UINT16 *) WorkingPtr);
  WorkingPtr += sizeof (UINT16);
  Description = (CHAR16 *) WorkingPtr;
  WorkingPtr += StrSize (Description);
  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) WorkingPtr;
  WorkingPtr += DevicePathLength;
  UsedSize = (UINTN) (WorkingPtr - (UINT8 *) EfiLoadOption);
  if (EfiLoadOptionSize > UsedSize && EfiLoadOption != NULL) {
    OptionalDataSize = (UINT32) (EfiLoadOptionSize - UsedSize);
    OptionalData = WorkingPtr;
  } else {
    OptionalData = NULL;
    OptionalDataSize = 0;
  }
  Status = BdsServicesCreateLoadOption (
             &mH2OBdsServices,
             VariableName[0] == L'B' ? BOOT_OPTION : DRIVER_OPTION,
             VariableName,
             VariableGuid,
             Attributes,
             DevicePath,
             Description,
             OptionalData,
             OptionalDataSize,
             BdsLoadOption
             );
  FreePool (EfiLoadOption);
  return Status;
}

/**
  Converts a BDS load option in a UEFI variable formatted as a UEFI load option.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BdsLoadOption      A pointer to BDS load option.
  @param[out] VariableName       A Null-terminated string that is the name of the vendor's variable.
                                 Caller is responsible freeing the buffer.
  @Param[out] VariableGuid       A unique identifier for the vendor.

  @retval EFI_SUCCESS            Convert UEFI load option to BDS load option successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL, VariableName is NULL or VariableGuid is NULL.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption format is incorrect.
  @retval EFI_OUT_OF_RESOURCES   Allocate memory to contain variable name is failed.
  @retval Others                 Any other error occurred in this function.
**/
EFI_STATUS
EFIAPI
BdsServicesConvertLoadOptionToVar (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   H2O_BDS_LOAD_OPTION         *BdsLoadOption,
  OUT  CHAR16                      **VariableName,
  OUT  EFI_GUID                    *VariableGuid
  )
{
  UINT8               *EfiLoadOption;
  UINTN               EfiLoadOptionSize;
  UINT8               *WorkingPtr;
  EFI_STATUS          Status;

  if (BdsLoadOption == NULL || BdsLoadOption->Signature != H2O_BDS_LOAD_OPTION_SIGNATURE ||
      VariableName == NULL || VariableGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // For BootPrev, BootCurrent, BootNext variable, needn't save related variable and just return variable name
  // variable GUID. other variable will save corresponding Boot#### and Driver#### variable.
  //
  if (!(StrCmp (BdsLoadOption->LoadOrderVarName, H2O_BOOT_PREVIOUS_VARIABLE_NAME) == 0 || StrCmp (BdsLoadOption->LoadOrderVarName, EFI_BOOT_CURRENT_VARIABLE_NAME) == 0 ||
      StrCmp (BdsLoadOption->LoadOrderVarName, EFI_BOOT_NEXT_VARIABLE_NAME) == 0)) {
    EfiLoadOptionSize = sizeof (UINT32) + sizeof (UINT16) + StrSize (BdsLoadOption->Description) +
                        GetDevicePathSize (BdsLoadOption->DevicePath) + BdsLoadOption->LoadOptionalDataSize;
    EfiLoadOption = AllocateZeroPool (EfiLoadOptionSize);
    if (EfiLoadOption == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    WorkingPtr = EfiLoadOption;
    *(UINT32 *) WorkingPtr = BdsLoadOption->Attributes;
    WorkingPtr += sizeof (UINT32);
    *(UINT16 *) WorkingPtr = (UINT16) GetDevicePathSize (BdsLoadOption->DevicePath);
    WorkingPtr += sizeof (UINT16);
    CopyMem (WorkingPtr, BdsLoadOption->Description, StrSize (BdsLoadOption->Description));
    WorkingPtr += StrSize (BdsLoadOption->Description);
    CopyMem (WorkingPtr, BdsLoadOption->DevicePath, GetDevicePathSize (BdsLoadOption->DevicePath));
    WorkingPtr += GetDevicePathSize (BdsLoadOption->DevicePath);
    CopyMem (WorkingPtr, BdsLoadOption->LoadOptionalData, BdsLoadOption->LoadOptionalDataSize);

    Status = CommonSetVariable (
               BdsLoadOption->LoadOrderVarName,
               &BdsLoadOption->LoadOrderVarGuid,
               EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
               EfiLoadOptionSize,
               EfiLoadOption
               );
    if (Status != EFI_SUCCESS) {
      FreePool (EfiLoadOption);
      return Status;
    }
    FreePool (EfiLoadOption);
  }

  *VariableName = AllocateCopyPool (StrSize (BdsLoadOption->LoadOrderVarName), BdsLoadOption->LoadOrderVarName);
  if (*VariableName == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyGuid (VariableGuid, &BdsLoadOption->LoadOrderVarGuid);

  return EFI_SUCCESS;
}

/**
  Insert a BDS load option into either the Driver or Boot order.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BdsLoadOption      A pointer to BDS load option.

  @retval EFI_SUCCESS            Insert a BDS load option into either the Driver or Boot order successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption format is incorrect.
**/
EFI_STATUS
EFIAPI
BdsServicesInsertLoadOption (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   H2O_BDS_LOAD_OPTION         *BdsLoadOption
  )
{
  UINTN                                OptionOrderSize;
  UINT16                               *OptionOrder;
  CHAR16                               *VariableName;
  UINTN                                Index;
  UINTN                                BootOptionNum;
  CHAR16                               OptionName[11];
  UINTN                                LoadOptionSize;
  UINT16                               *LoadOption;
  UINT8                                *TempPtr;
  EFI_DEVICE_PATH_PROTOCOL             *OptionDevicePath;
  EFI_STATUS                           Status;
  CHAR16                               *Description;
  EFI_BOOT_OPTION_POLICY_PROTOCOL      *BootOptionPolicy;
  UINTN                                BootOptionType;
  UINTN                                NewPosition;
  UINT16                               *NewOptionOrder;
  UINT16                               VarBbsTableIndex;
  UINT16                               OptionBbsTableIndex;

  if (BdsLoadOption == NULL || BdsLoadOption->Signature != H2O_BDS_LOAD_OPTION_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  VariableName    = BdsLoadOption->DriverOrBoot ? L"BootOrder" : L"DriverOrder";
  OptionOrder     = NULL;
  OptionOrderSize = 0;
  Status = CommonGetVariableDataAndSize (
             VariableName,
             &gEfiGlobalVariableGuid,
             &OptionOrderSize,
             (VOID **) &OptionOrder
             );
  //
  // TODO: Need check Legacy boot option
  //
  //
  // Compare with current option variable
  //
  BootOptionNum     = OptionOrderSize / sizeof (UINT16);
  for (Index = 0; (OptionOrder != NULL) && (Index < BootOptionNum); Index++) {
    if (*VariableName == 'B') {
      UnicodeSPrint (OptionName, sizeof (OptionName), L"Boot%04x", OptionOrder[Index]);
    } else {
      UnicodeSPrint (OptionName, sizeof (OptionName), L"Driver%04x", OptionOrder[Index]);
    }

    Status = CommonGetVariableDataAndSize (
                  OptionName,
                  &gEfiGlobalVariableGuid,
                  &LoadOptionSize,
                  (VOID **) &LoadOption
                  );
    if (Status != EFI_SUCCESS) {
      continue;
    }

    if (*VariableName == 'B' && BdsLibIsDummyBootOption (OptionOrder[Index])) {
      if (BdsLoadOption->LoadOrder != OptionOrder[Index]) {
        InternalFreePool (LoadOption);
        continue;
      }
      InsertLoadOptionToList (BdsLoadOption, OptionOrder, OptionOrderSize);
      InternalFreePool (LoadOption);
      InternalFreePool (OptionOrder);
      return EFI_SUCCESS;
    }

    TempPtr = (UINT8 *) LoadOption;
    TempPtr += sizeof (UINT32) + sizeof (UINT16);
    Description = (CHAR16 *) TempPtr;
    TempPtr += StrSize ((CHAR16 *) TempPtr);
    OptionDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) TempPtr;

    if (IsLegacyBootOption (OptionDevicePath)) {
      VarBbsTableIndex = *((UINT16 *) (TempPtr + GetDevicePathSize (OptionDevicePath) + sizeof (BBS_TABLE)));
      OptionBbsTableIndex = *((UINT16 *) ((UINT8 *) BdsLoadOption->LoadOptionalData + sizeof (BBS_TABLE)));
      if (VarBbsTableIndex == OptionBbsTableIndex) {
        InsertLoadOptionToList (BdsLoadOption, OptionOrder, OptionOrderSize);
        InternalFreePool (LoadOption);
        InternalFreePool (OptionOrder);
        return EFI_SUCCESS;
      }
    } else {
      if (CompareMem (OptionDevicePath, BdsLoadOption->DevicePath, GetDevicePathSize (OptionDevicePath)) == 0) {
        //
        // Got the option, so just return
        //
        InsertLoadOptionToList (BdsLoadOption, OptionOrder, OptionOrderSize);
        InternalFreePool (LoadOption);
        InternalFreePool (OptionOrder);
        return EFI_SUCCESS;
      //
      // Vista will multiply the partition start lba by 512,
      // so only compare partition type and signature
      //
      } else if (MatchPartitionDevicePathNode (BdsLoadOption->DevicePath, (HARDDRIVE_DEVICE_PATH *) OptionDevicePath) &&
                 BdsLibMatchFilePathDevicePathNode (BdsLoadOption->DevicePath, OptionDevicePath)) {

        //
        // Got the option, so just return
        //
        InsertLoadOptionToList (BdsLoadOption, OptionOrder, OptionOrderSize);
        InternalFreePool (LoadOption);
        InternalFreePool (OptionOrder);
        return EFI_SUCCESS;
      }
    }
    InternalFreePool (LoadOption);
  }

  if (*VariableName == 'D') {
    return EFI_UNSUPPORTED;
  }
  //
  // TODO: Currently, we only support insert new Boot####. Doesn't support insert Driver#####.
  //       Need implement Driver#### related if need use in the future.
  //

  Status = gBS->LocateProtocol (
                  &gEfiBootOptionPolicyProtocolGuid,
                  NULL,
                  (VOID **) &BootOptionPolicy
                  );

  if (Status == EFI_SUCCESS) {
    BootOptionType = IsLegacyBootOption (BdsLoadOption->DevicePath) ? LEGACY_BOOT_DEV : EFI_BOOT_DEV;
    BootOptionPolicy->FindPositionOfNewBootOption (
                        BootOptionPolicy,
                        BdsLoadOption->DevicePath,
                        OptionOrder,
                        BootOptionType,
                        OptionOrderSize / sizeof (UINT16),
                        BdsLoadOption->LoadOrder,
                        &NewPosition
                        );
  } else {
    NewPosition = 0;
  }
  BdsLibNewBootOptionPolicy (
    &NewOptionOrder,
    OptionOrder,
    OptionOrderSize / sizeof (UINT16),
    BdsLoadOption->LoadOrder,
    NewPosition
    );
  Status = CommonSetVariable (
             VariableName,
             &gEfiGlobalVariableGuid,
             EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
             OptionOrderSize + sizeof (UINT16),
             NewOptionOrder
             );
  InsertLoadOptionToList (BdsLoadOption, NewOptionOrder, OptionOrderSize + sizeof (UINT16));
  InternalFreePool (OptionOrder);
  InternalFreePool (NewOptionOrder);


  return EFI_SUCCESS;
}

/**
  Remove a BDS load option from either the Driver or Boot order.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  DriverOrBoot       Boolean that specifies whether this load option represents a Driver load option
                                 (FALSE) or Boot load option (TRUE).
  @param[in]  LoadOrder          Unsigned integer that specifies the current boot option being booted. Corresponds to the four
                                 hexadecimal digits in the #### portion of the UEFI variable name Boot#### or Driver####.

  @retval EFI_SUCCESS            Remove a BDS load option from either the Driver or Boot order successfully.
**/
EFI_STATUS
EFIAPI
BdsServicesRemoveLoadOption (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   BOOLEAN                     DriverOrBoot,
  IN   UINT16                      LoadOrder
  )
{
  CHAR16                 *VariableName;
  LIST_ENTRY             *OptionList;
  LIST_ENTRY             *Link;
  H2O_BDS_LOAD_OPTION    *CurrentLoadOption;
  UINT16                 *EfiLoadOption;
  UINTN                  EfiLoadOptionSize;
  UINTN                  Index;
  EFI_STATUS             Status;
  CHAR16                 LoadOptionName[11];

  OptionList    = DriverOrBoot ? &mBootList : &mDriverList;
  for (Link = GetFirstNode (OptionList); !IsNull (OptionList, Link); Link = GetNextNode (OptionList, Link)) {
    CurrentLoadOption = BDS_OPTION_FROM_LINK (Link);
    if (CurrentLoadOption->LoadOrder == LoadOrder) {
      RemoveEntryList (&CurrentLoadOption->Link);
      BdsServicesFreeLoadOption (NULL, CurrentLoadOption);
      break;
    }
  }
  VariableName  = DriverOrBoot ? L"BootOrder" : L"DriverOrder";

  Status = CommonGetVariableDataAndSize (
             VariableName,
             &gEfiGlobalVariableGuid,
             &EfiLoadOptionSize,
             (VOID **) &EfiLoadOption
             );
  if (Status != EFI_SUCCESS) {
    return EFI_SUCCESS;
  }

  for (Index = 0; Index < EfiLoadOptionSize / sizeof (UINT16); Index++) {
    if (EfiLoadOption[Index] == LoadOrder) {
      CopyMem (&EfiLoadOption[Index], &EfiLoadOption[Index + 1], EfiLoadOptionSize - (Index + 1) * sizeof (UINT16));
      Status = CommonSetVariable (
                 VariableName,
                 &gEfiGlobalVariableGuid,
                 EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                 EfiLoadOptionSize - sizeof (UINT16),
                 EfiLoadOption
                 );
      UnicodeSPrint (LoadOptionName, sizeof (LoadOptionName), DriverOrBoot ? L"Boot%04x" : L"Driver%04x", LoadOrder);
      Status = CommonSetVariable (
                 LoadOptionName,
                 &gEfiGlobalVariableGuid,
                 0,
                 0,
                 NULL
                 );
      break;
    }
  }
  InternalFreePool (EfiLoadOption);
  return EFI_SUCCESS;
}

/**
  Expand a partial load option to one or more fully qualified load options using the rules specified in the
  UEFI specification (USB WWID, hard disk) and the InsydeH2O boot groups.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BdsLoadOption      A pointer to BDS load option.

  @retval EFI_SUCCESS            Expand load option successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption format is incorrect.
**/
EFI_STATUS
EFIAPI
BdsServicesExpandLoadOption (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   H2O_BDS_LOAD_OPTION         *BdsLoadOption
  )
{
  USB_CLASS_DEVICE_PATH          *DevicePath;

  if (BdsLoadOption == NULL || BdsLoadOption->Signature != H2O_BDS_LOAD_OPTION_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  DevicePath = (USB_CLASS_DEVICE_PATH *) BdsLoadOption->DevicePath;
  if ((DevicePathType (DevicePath) == MESSAGING_DEVICE_PATH && DevicePathSubType (DevicePath) == MSG_USB_CLASS_DP) &&
      DevicePath->VendorId == 0xFFFF && DevicePath->ProductId == 0xFFFF && DevicePath->DeviceClass == 0xFF &&
      DevicePath->DeviceSubClass == 0xFF && DevicePath->DeviceProtocol == 0xFF) {
    ExpandWinowsToGoOption (BdsLoadOption);
  } else if (DevicePathType (BdsLoadOption->DevicePath) == MEDIA_DEVICE_PATH &&
            (DevicePathSubType (BdsLoadOption->DevicePath) == MEDIA_HARDDRIVE_DP)) {
    ExpandHddOption (BdsLoadOption);
  } else if ((DevicePathType (BdsLoadOption->DevicePath) == HARDWARE_DEVICE_PATH) &&
             (DevicePathSubType (BdsLoadOption->DevicePath) == HW_VENDOR_DP) &&
             (CompareGuid (&((VENDOR_DEVICE_PATH *) BdsLoadOption->DevicePath)->Guid, &gH2OBdsBootDeviceGroupGuid))) {
    ExpandBootGroupOption (BdsLoadOption);
  }

  return EFI_SUCCESS;
}

/**
  Launch a BDS load option.

  @param[in]  This               A Pointer to current instance of this protocol.
  @param[in]  BdsLoadOption      A pointer to BDS load option.
  @param[out] ExitDataSize       Pointer to the size, in bytes, of ExitData.
  @param[out] ExitData           Pointer to a pointer to a data buffer that includes a Null-terminated
                                 string, optionally followed by additional binary data.

  @retval EFI_SUCCESS            Boot from the input boot option successfully.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption is NULL.
  @retval EFI_INVALID_PARAMETER  BdsLoadOption format is incorrect.
  @retval EFI_NOT_FOUND          If the Device Path is not found in the system
**/
EFI_STATUS
EFIAPI
BdsServicesLaunchLoadOption (
  IN   H2O_BDS_SERVICES_PROTOCOL   *This,
  IN   H2O_BDS_LOAD_OPTION         *BdsLoadOption,
  OUT  UINTN                       *ExitDataSize,
  OUT  CHAR16                      **ExitData      OPTIONAL
  )
{
  if (BdsLoadOption == NULL || BdsLoadOption->Signature != H2O_BDS_LOAD_OPTION_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  if (BdsLoadOption->DriverOrBoot == BOOT_OPTION) {
    return LaunchBootOption (BdsLoadOption, ExitDataSize, ExitData);
  } else {
    return LaunchDriverOption (BdsLoadOption, ExitDataSize, ExitData);
  }
}