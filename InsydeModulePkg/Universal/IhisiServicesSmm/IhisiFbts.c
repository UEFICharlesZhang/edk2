/** @file
  FbtsLib Library Instance implementation

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "IhisiFbts.h"
#include <EfiFlashMap.h>
#include <Library/HobLib.h>
#include <Library/BvdtLib.h>
#include <Library/VariableLib.h>
#include <Library/FdSupportLib.h>
#include <Library/SmmOemSvcKernelLib.h>
#include <Guid/FlashMapHob.h>

#define        DEFAULT_VARIABLE_NAME_SIZE    0x50
EFI_GUID       mDefaultPreservedVendorGuid = { 0x77fa9abd, 0x0359, 0x4d32, 0xbd, 0x60, 0x28, 0xf4, 0xe7, 0x8f, 0x78, 0x4b };

FLASH_MAP_MAPPING_TABLE mFlashMapTypeMappingTable[] = {
  {FbtsRomMapPei,           EFI_FLASH_AREA_RECOVERY_BIOS},
  {FbtsRomMapCpuMicrocode,  EFI_FLASH_AREA_CPU_MICROCODE},
  {FbtsRomMapNVRam,         EFI_FLASH_AREA_EFI_VARIABLES},
  {FbtsRomMapDxe,           EFI_FLASH_AREA_MAIN_BIOS},
  {FbtsRomMapEc,            EFI_FLASH_AREA_FV_EC},
  {FbtsRomMapNvStorage,     EFI_FLASH_AREA_GUID_DEFINED},
  {FbtsRomMapFtwBackup,     EFI_FLASH_AREA_FTW_BACKUP},
  {FbtsRomMapFtwState,      EFI_FLASH_AREA_FTW_STATE},
  {FbtsRomMapSmbiosLog,     EFI_FLASH_AREA_SMBIOS_LOG},
  {FbtsRomMapOemData,       EFI_FLASH_AREA_OEM_BINARY},
  {FbtsRomMapGpnv,          EFI_FLASH_AREA_GPNV},
  {FbtsRomMapDmiFru,        EFI_FLASH_AREA_DMI_FRU},
  {FbtsRomMapPalB,          EFI_FLASH_AREA_PAL_B},
  {FbtsRomMapMcaLog,        EFI_FLASH_AREA_MCA_LOG},
  {FbtsRomMapPassword,      EFI_FLASH_AREA_RESERVED_03},
  {FbtsRomMapOemNvs,        EFI_FLASH_AREA_RESERVED_04},
  {FbtsRomMapReserved07,    EFI_FLASH_AREA_RESERVED_07},
  {FbtsRomMapReserved08,    EFI_FLASH_AREA_RESERVED_08},
  {FbtsRomMapReserved0A,    EFI_FLASH_AREA_RESERVED_0A},
  {FbtsRomMapUnused,        EFI_FLASH_AREA_UNUSED},
  {FbtsRomMapUndefined,     EFI_FLASH_AREA_GUID_DEFINED},
  {FbtsRomMapFactoryCopy,   EFI_FLASH_AREA_RESERVED_09}, // factory default
  {FbtsRomMapEos,           FbtsRomMapEos}               //End of table
};

//
// FBTS Support Functions
//
IHISI_SMI_SUB_FUNCTION
FBTS_FUNCTION_TABLE[] = {
  { FBTSGetSupportVersion,          FbtsGetSupportVersion,      IhisiNormalPriority}, \
  { FBTSGetPlatformInfo,            FbtsGetPlatformInfo,        IhisiNormalPriority}, \
  { FBTSGetPlatformRomMap,          FbtsGetPlatformRomMap,      IhisiNormalPriority}, \
  { FBTSGetFlashPartInfo,           FbtsGetFlashPartInfo,       IhisiNormalPriority}, \
  { FBTSRead,                       FbtsRead,                   IhisiNormalPriority}, \
  { FBTSWrite,                      FbtsWrite,                  IhisiNormalPriority}, \
  { FBTSComplete,                   FbtsComplete,               IhisiNormalPriority}, \
  { FBTSSkipMcCheckAndBinaryTrans,  SkipMcCheckAndBinaryTrans,  IhisiNormalPriority}, \
  { FBTSGetWholeBiosRomMap,         FbtsGetWholeBiosRomMap,     IhisiNormalPriority}
};

FBTS_PLATFORM_ROM_MAP_BUFFER                mRomMapBuffer;
FBTS_PLATFORM_PRIVATE_ROM_BUFFER            mPrivateRomMapBuffer;
extern BOOLEAN                              mInPOST;

/**
 Fill platform protect ROM map information to module ROM map buffer.

 @retval            EFI_SUCCESS         Set platform protect ROM map information to module ROM map buffer successful.
 @retval            EFI_UNSUPPORTED     Module ROM map buffer is full.
*/
EFI_STATUS
FillPlatformRomMapBuffer (
  IN UINT8      Type,
  IN UINT32     Address,
  IN UINT32     Length,
  IN UINT8      Entry
  )
{
  EFI_STATUS    Status;
  UINTN         ConvertedAddress;

  if (Entry >= (sizeof (FBTS_PLATFORM_ROM_MAP_BUFFER) / sizeof (FBTS_PLATFORM_ROM_MAP))) {
   return EFI_UNSUPPORTED;
  }

  if (Type != FbtsRomMapEos) {
    Status = mSmmFwBlockService->ConvertToSpiAddress(
                                   mSmmFwBlockService,
                                   (UINTN) Address,
                                   &ConvertedAddress
                                   );
    if (!EFI_ERROR (Status)) {
      Address = (UINT32) ConvertedAddress;
    }
  }

  mRomMapBuffer.PlatFormRomMap[Entry].Type    = Type;
  mRomMapBuffer.PlatFormRomMap[Entry].Address = Address;
  mRomMapBuffer.PlatFormRomMap[Entry].Length  = Length;

  return EFI_SUCCESS;
}

/**
 Fill platform private ROM map information to module ROM map buffer.

 @retval            EFI_SUCCESS         Set platform ROM map information to module ROM map buffer successful.
 @retval            EFI_UNSUPPORTED     Module ROM map buffer is full.
*/
EFI_STATUS
  FillPlatformPrivateRomMapBuffer (
  IN UINT32   Address,
  IN UINT32   Length,
  IN UINT8    Entry
  )
{
  EFI_STATUS    Status;
  UINTN         ConvertedAddress;

  if (Entry >= (sizeof (FBTS_PLATFORM_PRIVATE_ROM_BUFFER) / sizeof (FBTS_PLATFORM_PRIVATE_ROM))) {
    return EFI_UNSUPPORTED;
  }

  if (Address != FbtsRomMapEos) {
    Status = mSmmFwBlockService->ConvertToSpiAddress(
                                   mSmmFwBlockService,
                                   (UINTN) Address,
                                   &ConvertedAddress
                                   );
    if (!EFI_ERROR (Status)) {
      Address = (UINT32) ConvertedAddress;
    }
  }
  mPrivateRomMapBuffer.PlatFormRomMap[Entry].LinearAddress = Address;
  mPrivateRomMapBuffer.PlatFormRomMap[Entry].Size = Length;

  return EFI_SUCCESS;
}


/**
  Get IHISI status.translated from EFI status

  @param[in] Status  EFI_STATUS

  @return UINT32     IHISI status
**/
UINT32
GetFbtsIhisiStatus (
  EFI_STATUS                             Status
  )
{
  switch (Status) {
    case EFI_SUCCESS:
      return (UINT32) IHISI_SUCCESS;
      break;

    case EFI_BUFFER_TOO_SMALL:
      return (UINT32) IHISI_OB_LEN_TOO_SMALL;
      break;

    case EFI_UNSUPPORTED:
      return (UINT32) IHISI_UNSUPPORTED_FUNCTION;
      break;

    default:
      return (UINT32) IHISI_FBTS_PERMISSION_DENIED;
      break;
  }
}

BOOLEAN
IsVariableServiceSupported (
  )
{
  EFI_STATUS                    Status;
  UINTN                         VariableDataSize;

  Status = CommonGetVariable (NULL, NULL, &VariableDataSize, NULL);

  return Status == EFI_UNSUPPORTED ? FALSE : TRUE;
}

/**
  Check if this variable should be deleted. Search from the preserved list.

  @param[in]  VariableName               The variable name.
  @param[in]  VendorGuid                 The varialbe GUID.
  @param[in]  VariablePreservedTablePtr  The preserved table.
  @param[in]  IsKeepVariableInList       This flag determines the property of the preserved table.

  @retval TRUE                           This variable should be delteted
  @retval FALSE                          Keep this variable
**/
BOOLEAN
CheckVariableDelete (
  IN CHAR16                      *VariableName,
  IN EFI_GUID                    *VendorGuid,
  IN PRESERVED_VARIABLE_TABLE    *VariablePreservedTablePtr,
  IN BOOLEAN                      IsKeepVariableInList
  )
{
  UINTN                         Index;
  BOOLEAN                       IsVariableFound;

  //
  // According to logo requirement, should preserve all UEFI variables with VendorGuid
  // {77fa9abd-0359-4d32-bd60-28f4e78f784b}
  //
  if (CompareGuid (VendorGuid, &mDefaultPreservedVendorGuid)) {
    return FALSE;
  }
  //
  // Check if the variable is in the preserved list or not.
  //
  Index = 0;
  IsVariableFound = FALSE;
  while (VariablePreservedTablePtr[Index].VariableName != NULL) {
    if ((CompareGuid (VendorGuid, &VariablePreservedTablePtr[Index].VendorGuid)) &&
        (StrCmp (VariableName, VariablePreservedTablePtr[Index].VariableName) == 0)) {
      IsVariableFound = TRUE;
      break;
    } else {
      Index++;
    }
  }

  //
  //  IsKeepVariableInList | IsVariableFound | result
  // ------------------------------------------------
  //         TRUE          |      TRUE       | Keep
  //         TRUE          |      FALSE      | Delete
  //         FALSE         |      TRUE       | Delete
  //         FALSE         |      FALSE      | Keep
  //
  if (IsKeepVariableInList != IsVariableFound) {
    return TRUE;
  }

  return FALSE;
}

/**
  Use variable service to get the next variable. If the variable name size is not enough, re-allocate memory for it.
  return the memory buffer size to "MaxVariableNameSize".

  @param[in, out]  VariableSize         The variable size.
  @param[in, out]  VariableName         The varialbe name.
  @param[in, out]  VendorGuid           The variable GUID.
  @param[in, out]  MaxVariableNameSize  The max variable name size. Will allocate memory according to this size.

  @retval EFI_INVALID_PARAMETER         Invalid parameters
  @retval EFI_OUT_OF_RESOURCES          Not enough memory
  @retval EFI_SUCCESS                   Successfully
**/
EFI_STATUS
RelocateNextVariableName (
  IN OUT UINTN                 *VariableSize,
  IN OUT CHAR16               **VariableName,
  IN OUT EFI_GUID              *VendorGuid,
  IN OUT UINTN                 *MaxVariableNameSize
  )
{
  EFI_STATUS      Status;
  EFI_GUID        NextVendorGuid;
  UINTN           NextVariableSize;
  CHAR16         *NextVariableName;

  if (VariableSize == NULL || VariableName == NULL || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  NextVariableName = *VariableName;
  CopyGuid (&NextVendorGuid, VendorGuid);
  if (*VariableName == NULL) {
    NextVariableName = AllocateZeroPool (*MaxVariableNameSize);
    if (NextVariableName == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  NextVariableSize = *MaxVariableNameSize;
  Status = CommonGetNextVariableName (&NextVariableSize, NextVariableName, &NextVendorGuid);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    NextVariableName = ReallocatePool (*MaxVariableNameSize, NextVariableSize, NextVariableName);
    if (NextVariableName == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    *MaxVariableNameSize = NextVariableSize;
    Status = CommonGetNextVariableName (&NextVariableSize, NextVariableName, &NextVendorGuid);
  }

  if (!EFI_ERROR (Status)) {
    *VariableSize = NextVariableSize;
    *VariableName = NextVariableName;
    CopyGuid (VendorGuid, &NextVendorGuid);
  }

  return Status;
}

BOOLEAN
IsZeroGuid (
  IN EFI_GUID                   *Guid
  )
{
  UINTN       Index;
  UINT8      *TempPtr;

  TempPtr = (UINT8 *) Guid;

  for (Index = 0; Index < sizeof (EFI_GUID); Index++) {
    if (TempPtr[Index] != 0) {
      return FALSE;
    }
  }

  return TRUE;
}

UINTN
GetNumberOfVariable (
  IN UINT8                      *TablePtr
  )
{
  UINTN         NumberOfVariable;

  NumberOfVariable = 0;
  while (!IsZeroGuid ((EFI_GUID *) TablePtr)) {
    TablePtr += sizeof (EFI_GUID);
    NumberOfVariable++;
    TablePtr += StrSize ((CHAR16 *) TablePtr);
  }

  //
  // Add one for the end of data
  //
  NumberOfVariable++;

  return NumberOfVariable;
}
/**
  Get corresponding IHISI area type by flash area type accordingly.

  @param[in] FlashAreaType  Flash area type.

  @return UINT8      Corresponding IHISI area type.
**/
STATIC
UINT8
GetMappingType (
  UINT8  FlashAreaType
  )
{
  UINTN     Index = 0;

  while (mFlashMapTypeMappingTable[Index].FlashAreaType != FbtsRomMapEos) {
    if (FlashAreaType == mFlashMapTypeMappingTable[Index].FlashAreaType) {
      return mFlashMapTypeMappingTable[Index].IhisiAreaType;
    } else {
      Index++;
    }
  }

  return FbtsRomMapUndefined;
}


/**
  Get flash map from HOB.

  @retval EFI_SUCCESS        Get flash map from HOB successful.
  @return Others             Any error occurred.
**/
EFI_STATUS
GetFlashMapByHob (
  VOID
  )
{
  EFI_STATUS                    Status;
  UINT8                         Index;
  VOID                         *HobList;
  EFI_FLASH_MAP_ENTRY_DATA     *FlashMapEntryData;

  HobList = GetHobList ();
  ASSERT (HobList != NULL);

  Index = 0;
  do {
    HobList = GetNextGuidHob (&gEfiFlashMapHobGuid, HobList);
    if (HobList == NULL) {
      //
      // Fill end of structure ROM map if all of flash map HOBs have been filled to ROM map buffer.
      //
      Status = FillPlatformRomMapBuffer ((UINT8)FbtsRomMapEos, 0, 0, Index);
      break;
    }

    FlashMapEntryData = (EFI_FLASH_MAP_ENTRY_DATA *) HobList;
    Status = FillPlatformRomMapBuffer (GetMappingType (
               FlashMapEntryData->AreaType),
               (UINT32) (FlashMapEntryData->Entries[0].Base),
               (UINT32) (FlashMapEntryData->Entries[0].Length),
               Index
               );
    Index++;
  } while (!EFI_ERROR (Status));

  return Status;
}

/**
  Get the default preserved variable table from PCD.

  @param[out]  TablePtr  The pointer to the default table
**/
VOID
GetDefaultTable (
  OUT PRESERVED_VARIABLE_TABLE   **TablePtr
  )
{
  UINTN                         Index;
  UINTN                         NumberOfVariable;
  UINT8                        *StringPtr;
  PRESERVED_VARIABLE_TABLE     *TempTablePtr;

  TempTablePtr = NULL;
  *TablePtr = (PRESERVED_VARIABLE_TABLE *) PcdGetPtr (PcdDefaultPreservedVariableList);

  if (!IsZeroGuid ((EFI_GUID *) *TablePtr)) {
    NumberOfVariable = GetNumberOfVariable ((UINT8 *) *TablePtr);
    gSmst->SmmAllocatePool (
             EfiRuntimeServicesData,
             NumberOfVariable * sizeof (PRESERVED_VARIABLE_TABLE),
             (VOID **)&TempTablePtr
             );

    StringPtr = (UINT8 *) *TablePtr;
    for (Index = 0; (!IsZeroGuid ((EFI_GUID *) StringPtr)); Index++) {
      CopyMem (&TempTablePtr[Index].VendorGuid, StringPtr, sizeof (EFI_GUID));

      StringPtr += sizeof (EFI_GUID);
      TempTablePtr[Index].VariableName = (CHAR16 *) StringPtr;
      //
      // Go to the next variable.
      //
      StringPtr += StrSize (TempTablePtr[Index].VariableName);
    }
  }

  *TablePtr = TempTablePtr;
}

/**
 Purify the variables if needed. If there is NO OemService "OemSvcVariablePreservedTable",
 do nothing in this function.

 @retval EFI_SUCCESS            Success
 @retval EFI_UNSUPPORTED        there is no SmmVariable service
 @retval EFI_BUFFER_TOO_SMALL   not enough memory

**/
EFI_STATUS
PurifyVariable (
  VOID
  )
{
  EFI_STATUS                    Status;
  EFI_GUID                      VendorGuid;
  EFI_GUID                      NextVendorGuid;
  UINTN                         VariableNameSize;
  UINTN                         MaxVariableNameSize;
  BOOLEAN                       IsKeepVariableInList;
  UINTN                         NextVariableNameSize;
  CHAR16                       *VariableName;
  CHAR16                       *NextVariableName;
  PRESERVED_VARIABLE_TABLE     *VariablePreservedTablePtr;

  if (!IsVariableServiceSupported ()) {
    return EFI_UNSUPPORTED;
  }

  //
  // Get the default preserved variable table from PCD.
  //
  GetDefaultTable (&VariablePreservedTablePtr);
  IsKeepVariableInList = PcdGetBool (PcdKeepVariableInList);
  //
  // Get variable preserved table from OemServices
  //
  Status = OemSvcVariablePreservedTable (
             &VariablePreservedTablePtr,
             &IsKeepVariableInList
             );
  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  if (VariablePreservedTablePtr == NULL) {
    //
    // This OemService does not exist, so do nothing.
    //
    return EFI_SUCCESS;
  }

  if ((!IsKeepVariableInList) && (VariablePreservedTablePtr->VariableName == NULL)) {
    //
    // Clear an empty table, so do nothing.
    //
    return EFI_SUCCESS;
  }

  VariableName = NULL;
  NextVariableName = NULL;
  MaxVariableNameSize = DEFAULT_VARIABLE_NAME_SIZE;
  Status = RelocateNextVariableName (&VariableNameSize, &VariableName, &VendorGuid, &MaxVariableNameSize);

  while (!EFI_ERROR (Status)) {
    Status = RelocateNextVariableName (&NextVariableNameSize, &NextVariableName, &NextVendorGuid, &MaxVariableNameSize);
    if (Status == EFI_OUT_OF_RESOURCES) {
      goto Done;
    }

    if (CheckVariableDelete (VariableName, &VendorGuid, VariablePreservedTablePtr, IsKeepVariableInList)) {
      CommonSetVariable (VariableName, &VendorGuid, 0, 0, NULL);
    }

    if (VariableNameSize < MaxVariableNameSize) {
      VariableName = ReallocatePool (VariableNameSize, MaxVariableNameSize, VariableName);
      VariableNameSize = MaxVariableNameSize;
    }
    CopyMem (VariableName, NextVariableName, NextVariableNameSize);
    CopyGuid (&VendorGuid, &NextVendorGuid);
  }
  Status = EFI_SUCCESS;

Done:

  if (VariableName != NULL) {
    gSmst->SmmFreePool (VariableName);
  }

  if (NextVariableName != NULL) {
    gSmst->SmmFreePool (NextVariableName);
  }

  return Status;
}

STATIC
EFI_STATUS
UpdateExtendPlatformBuildDateTimeInfo (
  IN OUT EXTEND_PLATFORM_DATA_ITEM                *ExtInfoDataItemPtr
  )
{
  UINTN                                     StringSize;
  EFI_STATUS                                Status;
  CHAR16                                    StringDate[MODEL_DATE_SIZE];
  CHAR16                                    StringTime[MODEL_TIME_SIZE];
  UINT32                                    BufferSize;

  BufferSize = ExtInfoDataItemPtr->DataSize;
  ExtInfoDataItemPtr->DataSize = 0;

  //
  // Get build date string
  //
  StringSize = MODEL_DATE_SIZE;
  Status  = GetBvdtInfo ((BVDT_TYPE) BvdtBuildDate, &StringSize, StringDate);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get build time string
  //
  Status  = GetBvdtInfo ((BVDT_TYPE) BvdtBuildTime, &StringSize, StringTime);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ExtInfoDataItemPtr->DataSize = (UINT32)(StrSize (StringDate) + StrSize (StringTime));
  if (ExtInfoDataItemPtr->DataSize > BufferSize) {
    return EFI_BUFFER_TOO_SMALL;
  }

  //Merge Build date String . Build time string
  StringSize = StrSize (StringDate);
  StringDate[(StrSize (StringDate)/sizeof (StringDate[0])) - 1] = L' ';
  CopyMem ( ExtInfoDataItemPtr->Data, StringDate, StringSize);
  //
  // Update build time string
  //
  ExtInfoDataItemPtr = (EXTEND_PLATFORM_DATA_ITEM *)(ExtInfoDataItemPtr->Data + StringSize);
  StringSize = StrSize (StringTime);
  CopyMem ( ExtInfoDataItemPtr, StringTime, StringSize);

  return EFI_SUCCESS;
}

EFI_STATUS
UpdateExtendPlatform (
  )
{
  EFI_STATUS                                Status;
  UINT8                                     Index;
  UINT32                                    BufferSize;
  UINT32                                    DataItemUseSize;
  UINT32                                    DataItemTotalSize;
  EXTEND_PLATFORM_DATA_ITEM                *DataItemPtr;
  EXTEND_PLATFORM_DATA_ITEM                *TempDataItemPtr;
  FBTS_EXTEND_PLATFORM_INFO_TABLE_OUTPUT   *ExtendPlatformInfoOutput;
  FBTS_EXTEND_PLATFORM_INFO_TABLE_INPUT    *ExternPlatformInfoInput;
  UPDATE_EXT_ITEM_FUN_TABLE                 UpdateFunTable[] = {
                                                                {BuildDaeTimeID, UpdateExtendPlatformBuildDateTimeInfo}
                                                               };

  ExternPlatformInfoInput = (FBTS_EXTEND_PLATFORM_INFO_TABLE_INPUT *)(UINTN)IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  if (ExternPlatformInfoInput->Sigature != EXTEND_PLATFORM_INPUT_BUFFER_SIGNATURE) {
    //
    // Tool do not support Extend Platform Info, no change , return success.
    //
    return IHISI_SUCCESS;
  }
  if (BufferOverlapSmram ((VOID *) ExternPlatformInfoInput, ExternPlatformInfoInput->StructureSize)){
    return IHISI_UNSUPPORTED_FUNCTION;
  }
  BufferSize = ExternPlatformInfoInput->StructureSize;

  ExtendPlatformInfoOutput = (FBTS_EXTEND_PLATFORM_INFO_TABLE_OUTPUT *)(UINTN)IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  DataItemPtr = ExtendPlatformInfoOutput->DataItem;
  DataItemTotalSize = BufferSize - EXTEND_OFFSET(FBTS_EXTEND_PLATFORM_INFO_TABLE_OUTPUT, DataItem);
  TempDataItemPtr = AllocatePool (DataItemTotalSize);
  if (TempDataItemPtr == NULL) {
    return EFI_SUCCESS;
  }
  ExtendPlatformInfoOutput->DataItemCount = 0;

  for (Index = 0, DataItemUseSize = 0; Index < sizeof(UpdateFunTable)/sizeof(UpdateFunTable[0]); Index++) {
    ZeroMem (TempDataItemPtr, DataItemTotalSize);
    TempDataItemPtr->DataID = UpdateFunTable[Index].DataID;
    //
    //DataSize :Indicates how many available data size in Data field (offset 05h). Not include Data_ID (offset 00h) and Data_Size (offset 01h) fields.
    //
    TempDataItemPtr->DataSize = DataItemTotalSize - EXTEND_OFFSET(EXTEND_PLATFORM_DATA_ITEM, Data);
    Status = UpdateFunTable[Index].UpdateExtItemFun(TempDataItemPtr);

    if (Status == EFI_SUCCESS || Status == EFI_BUFFER_TOO_SMALL) {
      //
      //Add each function dataitem use size to DataItemUseSize
      //
      ExtendPlatformInfoOutput->Sigature = EXTEND_PLATFORM_OUTPUT_BUFFER_SIGNATURE;
      DataItemUseSize += TempDataItemPtr->DataSize + EXTEND_OFFSET(EXTEND_PLATFORM_DATA_ITEM, Data);
      if (DataItemUseSize < DataItemTotalSize) {
        //
        //Copy and point to next DataItem start address.
        //
        ExtendPlatformInfoOutput->DataItemCount ++;
        CopyMem (DataItemPtr, TempDataItemPtr, TempDataItemPtr->DataSize + EXTEND_OFFSET(EXTEND_PLATFORM_DATA_ITEM, Data));
        DataItemPtr = (EXTEND_PLATFORM_DATA_ITEM *)((UINT8 *)DataItemPtr + TempDataItemPtr->DataSize + EXTEND_OFFSET(EXTEND_PLATFORM_DATA_ITEM, Data));
      }
    }
  }
  FreePool (TempDataItemPtr);
  //
  //if DataItemUseSize over AP provide buffer size(DataItemTotalSize),restore sigature and return buffer to small.
  //
  if (DataItemUseSize > DataItemTotalSize) {
    ExtendPlatformInfoOutput->DataItemCount = DataItemUseSize;
    return IHISI_OB_LEN_TOO_SMALL;
  }
  return EFI_SUCCESS;
}

/**
  AH=10h, Get FBTS supported version and FBTS permission.

  @retval EFI_SUCCESS        Command successful returned.
**/
EFI_STATUS
FbtsGetSupportVersion (
  VOID
  )
{
  UINT32                            Eax;
  UINT32                            Ecx;
  UINT16                            Permission;
  FBTS_TOOLS_VERSION_BUFFER        *VersionPtr;
  FBTS_PLATFORM_STATUS_BUFFER      *PlatformInfoPtr;

  VersionPtr = (FBTS_TOOLS_VERSION_BUFFER *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  if (BufferOverlapSmram ((VOID *) VersionPtr, sizeof (FBTS_TOOLS_VERSION_BUFFER)) ||
       VersionPtr->Signature != FBTS_VERSION_SIGNATURE) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }
  //
  //Get Permission
  //
  Permission = FBTS_PERMISSION_DENY;
  SmmCsSvcIhisiFbtsGetPermission (VersionPtr, &Permission);
  Eax = IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RAX);
  Eax = (UINT32)((Eax & 0xffff0000) | Permission);
  IhisiWriteCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RAX, Eax);

  //
  //Get IHISI version
  //
  Ecx = IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  Ecx = (UINT32)((Ecx & 0xffff0000) | PcdGet16 (PcdIhisiFbtsVersion));
  IhisiWriteCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX, Ecx);

  //
  //Get AC Status and BattLife.
  //
  PlatformInfoPtr = (FBTS_PLATFORM_STATUS_BUFFER *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  if (BufferOverlapSmram ((VOID *) PlatformInfoPtr, sizeof (FBTS_PLATFORM_STATUS_BUFFER))) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }
  PlatformInfoPtr->OemHelp1.Data = 0;
  PlatformInfoPtr->OemHelp2.Data = 0;
  PlatformInfoPtr->AcStatus = SmmCsSvcIhisiFbtsGetAcStatus ();
  PlatformInfoPtr->Battery  = SmmCsSvcIhisiFbtsGetBatteryLife ();
  PlatformInfoPtr->Bound    = PcdGet8 (PcdIhisiFbtsBatteryLowBound);
  PlatformInfoPtr->Customer = PcdGet16 (PcdIhisiFbtsVendorId);

  return EFI_SUCCESS;
}


/**
  AH=11h, Get platform information.

  @retval EFI_SUCCESS        Get platform information successful.
  @return Other              Get platform information failed.
**/
EFI_STATUS
FbtsGetPlatformInfo (
  VOID
  )
{
  EFI_STATUS                                Status;
  UINT32                                    Eax;
  UINT8                                     ApCheck;
  UINTN                                     StrSize;
  FBTS_PLATFORM_INFO_BUFFER                *PlatformInfoPtr;

  //
  // DS:EDI - Pointer to platform information structure as below.
  //
  //  Offset | Size | Item          | Description
  // --------|------|---------------|--------------
  //   00h   | 40h  | Model Name    | Unicode string, end with '00h'.
  //   40h   | FFh  | Model Version | Unicode string, end with '00h'.
  //
  PlatformInfoPtr = (FBTS_PLATFORM_INFO_BUFFER *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  //
  // Check input address and whole input buffer isn't located in SM RAM.
  //
  if (BufferOverlapSmram ((VOID *) PlatformInfoPtr, sizeof (FBTS_PLATFORM_INFO_BUFFER))) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }
  ZeroMem (PlatformInfoPtr, sizeof (FBTS_PLATFORM_INFO_BUFFER));

  //
  //Update Model version
  //
  StrSize = MODEL_VERSION_SIZE;
  Status  = GetBvdtInfo ((BVDT_TYPE) BvdtBiosVer, &StrSize, PlatformInfoPtr->ModelVersion);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  //Update Model name
  //
  StrSize = MODEL_NAME_SIZE;
  Status  = GetBvdtInfo ((BVDT_TYPE) BvdtProductName, &StrSize, PlatformInfoPtr->ModelName);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  //Udpate AP check capability
  //
  ApCheck = SmmCsSvcIhisiFbtsApCheck ();
  Eax = IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RAX);
  Eax = (UINT32) ((Eax & 0xffff00ff) | (ApCheck<<8));
  IhisiWriteCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RAX, Eax);

  //
  // Update extend platform information
  //
  Status = UpdateExtendPlatform ();

  return Status;
}

/**
  AH=12h, Get Platform ROM map protection.

  @retval EFI_SUCCESS        Get Platform ROM map protection successful.
**/
EFI_STATUS
FbtsGetPlatformRomMap (
  VOID
  )
{
  FBTS_PLATFORM_ROM_MAP_BUFFER             *InputRomMapBuffer;
  FBTS_PLATFORM_PRIVATE_ROM_BUFFER         *InputPrivateRomMapBuffer;

  //
  // Get ROM map protection structure.
  //
  InputRomMapBuffer = (FBTS_PLATFORM_ROM_MAP_BUFFER *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  InputPrivateRomMapBuffer = (FBTS_PLATFORM_PRIVATE_ROM_BUFFER *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  //
  // Check input address and whole input buffer isn't located in SM RAM.
  //
  if (BufferOverlapSmram ((VOID *) InputRomMapBuffer, sizeof (FBTS_PLATFORM_ROM_MAP_BUFFER)) ||
      BufferOverlapSmram ((VOID *) InputPrivateRomMapBuffer, sizeof (FBTS_PLATFORM_PRIVATE_ROM_BUFFER))) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  //
  // Update protect ROM map
  //
  CopyMem (InputRomMapBuffer, &mRomMapBuffer, sizeof (FBTS_PLATFORM_ROM_MAP_BUFFER));

  //
  // Update private ROM map
  //
  CopyMem (InputPrivateRomMapBuffer, &mPrivateRomMapBuffer, sizeof (FBTS_PLATFORM_PRIVATE_ROM_BUFFER));

  return EFI_SUCCESS;
}

/**
   AH=13h, Get Flash part information.

  @retval EFI_SUCCESS        Get Flash part information successful.
  @return Other              Get Flash part information failed.
**/
EFI_STATUS
FbtsGetFlashPartInfo (
  VOID
  )
{
  FLASH_DEVICE                              *Buffer;
  EFI_STATUS                                Status;
  FBTS_FLASH_DEVICE                         FlashDevice;
  UINT16                                    BlockMap[3];
  UINT8                                     SpiFlashNumber;
  UINT8                                     *FlashInfo;
  UINTN                                     FlashInfoSize;
  UINT8                                     *FlashBlockMap;

  //
  // CL = 00h => default, no choice from AP.
  //      01h => AP need flash SPI flash part.
  //      02h => AP need flash non-SPI flash part (LPC, FWH).
  //

  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (FLASH_DEVICE),
                    (VOID **)&Buffer
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = mSmmFwBlockService->DetectDevice (
                                mSmmFwBlockService,
                                (UINT8*)Buffer
                                );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FlashDevice.Id = (UINT32)Buffer->DeviceInfo.Id;
  FlashDevice.Size = Buffer->DeviceInfo.Size;
  FlashDevice.SpecifiedSize = 0;
  AsciiStrCpy (FlashDevice.VendorName, Buffer->DeviceInfo.VendorName);
  AsciiStrCpy (FlashDevice.DeviceName, Buffer->DeviceInfo.DeviceName);
  CopyMem ((VOID *) BlockMap, &(Buffer->DeviceInfo.BlockMap), sizeof (FD_BLOCK_MAP));
  if (FlashDevice.Size == 0xFF) {
    //
    // The BlockSize unit is 256(0x100) byte.
    //
    FlashDevice.SpecifiedSize = (Buffer->DeviceInfo.BlockMap.Mutiple * Buffer->DeviceInfo.BlockMap.BlockSize) * 0x100;
  } else {
    Status = mSmmFwBlockService->GetSpiFlashNumber (
                                  mSmmFwBlockService,
                                  &SpiFlashNumber
                                  );

    if (!EFI_ERROR (Status)) {
      FlashDevice.Size += SpiFlashNumber - 1;
      BlockMap[1] *= 1 << (SpiFlashNumber - 1);
    }
  }
  //
  // DS:EDI - Pointer to flash part information structure.
  // DS:ESI - Pointer to flash part block map structure.
  //
  FlashInfo     = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  FlashInfoSize = sizeof(FBTS_FLASH_DEVICE);
  //
  // Check output address and whole output buffer isn't located in SM RAM.
  //
  if (BufferOverlapSmram ((VOID *) FlashInfo , FlashInfoSize)) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }
  CopyMem (
    (VOID *) FlashInfo,
    &FlashDevice,
    FlashInfoSize
    );

  FlashBlockMap     = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  //
  // Check output address and whole output buffer isn't located in SM RAM.
  //
  if (BufferOverlapSmram ((VOID *) FlashBlockMap, sizeof (FD_BLOCK_MAP))) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }
  CopyMem ((VOID *) FlashBlockMap, BlockMap, sizeof (FD_BLOCK_MAP));

  return EFI_SUCCESS;
}

/**
  AH=1Bh,Skip module check allows and binary file transmissions.

  @retval EFI_SUCCESS   Success returns.
**/
EFI_STATUS
SkipMcCheckAndBinaryTrans (
  VOID
  )
{
  //
  // return IhisiSuccess to allow skip module check. If doesn't allow
  // skip module check, please return FbtsCannotSkipModuleCheck (0x27)
  //
  //
  //IhisiLibErrorCodeHandler ((UINT32)IhisiSuccess);

  return EFI_SUCCESS;
}

/**
  Get default BIOS ROM map

  @param[out] BiosRomMap              Pointer to the returned (FBTS_INTERNAL_BIOS_ROM_MAP *) data
  @param[out] NumberOfRegions         The total number of regions in BiosRomMap

  @retval EFI_SUCCESS                 FBTS get BIOS ROM map success.
  @return Others                      FBTS get BIOS ROM map failed.
**/
EFI_STATUS
GetDefaultBiosRomMap (
  OUT FBTS_INTERNAL_BIOS_ROM_MAP    **BiosRomMap,
  OUT UINTN                          *NumberOfRegions
  )
{
  UINTN  Index;
  extern FBTS_INTERNAL_BIOS_ROM_MAP mDefaultBiosRomMap[];
  UINT32 NvStorageRegionSize;
  UINTN  Conuter;

  if ((BiosRomMap == NULL) || (NumberOfRegions == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

#define ADD_ROM_MAP_ENTRY(RegionType, RegionAddress, RegionSize, RegionAttr)        \
            mDefaultBiosRomMap[Index].Type = (UINT8)RegionType;                     \
            mDefaultBiosRomMap[Index].Address = RegionAddress;                      \
            mDefaultBiosRomMap[Index].Size = RegionSize;                            \
            mDefaultBiosRomMap[Index].Attribute = RegionAttr;                       \
            Index++;                                                                \
            Conuter = __COUNTER__;

#define ADD_ROM_MAP_ENTRY_FROM_PCD(RegionType, AddressPcd, SizePcd, RegionAttr)                 \
          if (PcdGet32(SizePcd) > 0) {                                                          \
            ADD_ROM_MAP_ENTRY(RegionType, PcdGet32(AddressPcd), PcdGet32(SizePcd), RegionAttr); \
          }

  Index = 0;
  ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapDxe,          PcdFlashFvMainBase,               PcdFlashFvMainSize,               0);
  ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapCpuMicrocode, PcdFlashNvStorageMicrocodeBase,   PcdFlashNvStorageMicrocodeSize,   0);
  ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapDmiFru,       PcdFlashNvStorageDmiBase,         PcdFlashNvStorageDmiSize,         0);
  ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapOemData,      PcdFlashNvStorageMsdmDataBase,    PcdFlashNvStorageMsdmDataSize,    0);
  ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapOemData,      PcdFlashNvStorageBvdtBase,        PcdFlashNvStorageBvdtSize,        0);
  ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapPei,          PcdFlashFvRecoveryBase,           PcdFlashFvRecoverySize,           0);

  NvStorageRegionSize = PcdGet32 (PcdFlashNvStorageVariableSize) + PcdGet32 (PcdFlashNvStorageFtwWorkingSize) +
                        PcdGet32 (PcdFlashNvStorageFtwSpareSize) + PcdGet32 (PcdFlashNvStorageFactoryCopySize);
  if (NvStorageRegionSize > 0) {
    ADD_ROM_MAP_ENTRY (FbtsRomMapNvStorage, PcdGet32 (PcdFlashNvStorageVariableBase), NvStorageRegionSize, 0);
  }

  //
  // End of ROM map
  //
  ADD_ROM_MAP_ENTRY (FbtsRomMapEos, 0, 0, 0);

  *BiosRomMap = (FBTS_INTERNAL_BIOS_ROM_MAP *)mDefaultBiosRomMap;
  *NumberOfRegions = Index;

  return EFI_SUCCESS;
}
//
// the mDefaultBiosRomMap is declared after GetDefaultRomMap() on purpose to
// ensure the array size of mDefaultBiosRomMap is enough, DO NOT move it to the top
//
FBTS_INTERNAL_BIOS_ROM_MAP mDefaultBiosRomMap[__COUNTER__];

/**
  Get whole BIOS ROM map.(AH=1Eh)

  @param[out] IhisiStatus             Return IHISI status.

  @retval EFI_SUCCESS                 FBTS get BIOS ROM map success.
  @return Others                      FBTS get BIOS ROM map failed.
**/
EFI_STATUS
FbtsGetWholeBiosRomMap (
  VOID
  )
{
  EFI_STATUS                            Status;
  UINTN                                 RomMapSize;
  UINTN                                 NumberOfRegions;
  UINT8                                *RomMapPtr;
  FBTS_INTERNAL_BIOS_ROM_MAP           *BiosRomMap;

  GetDefaultBiosRomMap ((FBTS_INTERNAL_BIOS_ROM_MAP **)&BiosRomMap, &NumberOfRegions);
  Status = OemSvcIhisiGetWholeBiosRomMap ((VOID **)&BiosRomMap, &NumberOfRegions);

  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }
  if (BiosRomMap[NumberOfRegions - 1].Type != FbtsRomMapEos) {
    return IHISI_FBTS_UNKNOWN_PLATFORM_ROM_MAP;
  }

  //
  // Check input address and whole input buffer isn't located in SM RAM.
  //
  RomMapPtr  = (UINT8 *) (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  RomMapSize = NumberOfRegions * sizeof (FBTS_INTERNAL_BIOS_ROM_MAP);
  if (BufferOverlapSmram ((VOID *) RomMapPtr, RomMapSize)) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }
  //
  // Get ROM map protection structure.
  //
  CopyMem ((VOID *)RomMapPtr, (VOID *)BiosRomMap, RomMapSize);

  return EFI_SUCCESS;
}

/**
  FBTS Read.(AH=14h)

  @param[in]  mSmmFwBlockService  The EFI_SMM_FW_BLOCK_SERVICE_PROTOCOL instance.
  @param[out] IhisiStatus        Return IHISI status

  @retval EFI_SUCCESS            FBTS read success.
  @return Others                 FBTS read failed.
**/
EFI_STATUS
FbtsRead (
  )
{
  EFI_STATUS                      Status;
  UINTN                           Size;
  UINTN                           Offset;
  UINTN                           Address;
  UINT8                          *DataBuffer;

  //
  // ECX    - Size to read.
  // DS:ESI - Pointer to returned data buffer. Size in ECX.
  // EDI    - Target linear address to read.
  //
  DataBuffer = (UINT8 *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  Size       = (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  Address    = (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);
  Offset     = 0;
  if (BufferOverlapSmram ((VOID *) DataBuffer, Size) || BufferOverlapSmram ((VOID *) Address, Size)) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  Status = SmmCsSvcIhisiFbtsDoBeforeReadProcess(&Address, &Size, DataBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = mSmmFwBlockService->Read (
                                mSmmFwBlockService,
                                Address,
                                Offset,
                                &Size,
                                DataBuffer
                                );
  if (EFI_ERROR (Status)) {
    Status = IHISI_FBTS_READ_FAILED;
  }
  SmmCsSvcIhisiFbtsDoAfterReadProcess(Status);

  return Status;
}

/**
  FBTS write.(AH=15h)

  @retval EFI_SUCCESS            FBTS write success.
  @return Others                 FBTS write failed.
**/
EFI_STATUS
FbtsWrite (
  VOID
  )
{
  EFI_STATUS                      Status;
  UINTN                           WriteSize;
  UINT8                           EraseCount;
  UINT8                           WriteCount;
  UINTN                           RomBaseAddress;
  BOOLEAN                         InUnsignedRegion;
  UINTN                           UnsignedRegionBase;
  UINT8                          *WriteDataBuffer;

  //
  // ECX    - Size to write.
  // DS:ESI - Pointer to returned data buffer. Size in ECX.
  // EDI    - Target linear address to write.
  //
  WriteDataBuffer = (UINT8 *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  WriteSize       = (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
  RomBaseAddress  = (UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);

  if (BufferOverlapSmram ((VOID *) WriteDataBuffer, WriteSize) || BufferOverlapSmram ((VOID *) RomBaseAddress, WriteSize)) {
    return IHISI_UNSUPPORTED_FUNCTION;
  }

  Status = SmmCsSvcIhisiFbtsDoBeforeWriteProcess(WriteDataBuffer, &WriteSize, &RomBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  InUnsignedRegion = FALSE;
  if (PcdGetBool (PcdSecureFlashSupported)) {
    Status = mSmmFwBlockService->ConvertToSpiAddress(
                                   mSmmFwBlockService,
                                   (UINTN) PcdGet32 (PcdFlashUnsignedFvRegionBase),
                                   &UnsignedRegionBase
                                   );
    if (!EFI_ERROR (Status)) {
      //
      // Check Write address is in the Unsigned Region or not
      //
      if ((RomBaseAddress >= UnsignedRegionBase) && \
          ((RomBaseAddress + WriteSize) <= (UnsignedRegionBase + PcdGet32 (PcdEndOfFlashUnsignedFvRegionTotalSize)))) {
        InUnsignedRegion = TRUE;
      }
    }

    if (!mInPOST && !InUnsignedRegion) {
      Status = IHISI_FBTS_WRITE_FAILED;
      goto WriteDone;
    }
  }

  EraseCount      = 0;
  WriteCount      = 0;
  do {
    Status = mSmmFwBlockService->EraseBlocks (
                                  mSmmFwBlockService,
                                  RomBaseAddress,
                                  (UINTN *) &WriteSize
                                  );
    if (!EFI_ERROR (Status)) {
      EraseCount = 0;
      Status = mSmmFwBlockService->Write (
                                    mSmmFwBlockService,
                                    RomBaseAddress,
                                    (UINTN *) &WriteSize,
                                    WriteDataBuffer
                                    );
      if (!EFI_ERROR (Status)) {
        goto WriteDone;
      } else {
        Status = IHISI_FBTS_WRITE_FAILED;
        WriteCount++;
      }
    } else {
      Status = IHISI_FBTS_ERASE_FAILED;
      EraseCount++;
    }
  } while ((EraseCount < 100) && (WriteCount < 100));

WriteDone:
  SmmCsSvcIhisiFbtsDoAfterWriteProcess(Status);

  return Status;
}

/**
  AH=16h, This function uses to execute some specific action after the flash process is completed.

  @retval EFI_SUCCESS        Function succeeded.
  @return Other              Error occurred in this function.
**/
EFI_STATUS
FbtsComplete (
  VOID
  )
{
  EFI_STATUS                    Status;
  UINT8                         ApRequest;
  FBTS_FLASH_COMPLETE_STATUS   *FlashCompleteStatus;

  *(mSmmFwBlockService->FlashMode) = SMM_FW_DEFAULT_MODE;

  FlashCompleteStatus = (FBTS_FLASH_COMPLETE_STATUS *)(UINTN) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  if (FlashCompleteStatus->Signature == FLASH_COMPLETE_STATUS_SIGNATURE &&
      FlashCompleteStatus->StructureSize == sizeof (FBTS_FLASH_COMPLETE_STATUS) &&
      (BufferOverlapSmram ((VOID *) FlashCompleteStatus, sizeof (FBTS_FLASH_COMPLETE_STATUS)) == FALSE)) {
    switch (FlashCompleteStatus->CompleteStatus) {
      case ApTerminated:
        Status = SmmCsSvcIhisiFbtsApTerminated ();
        break;

      case NormalFlash:
        Status = SmmCsSvcIhisiFbtsNormalFlash ();
        if (EFI_ERROR(Status)) {
          Status = PurifyVariable();
        }
        break;

      case PartialFlash:
        Status = SmmCsSvcIhisiFbtsPartialFlash ();
        break;

      default:
        Status = EFI_SUCCESS;
        break;
    }
  }

  ApRequest = (UINT8) IhisiReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);

  Status = SmmCsSvcIhisiFbtsOemComplete(ApRequest);
  if (Status == EFI_SUCCESS) {
    return Status;
  }

  switch(ApRequest) {
    case FlashCompleteReboot: //Reboot
      Status = SmmCsSvcIhisiFbtsReboot ();
      break;

    case FlashCompleteShutdown: //Shut down
      Status = SmmCsSvcIhisiFbtsShutDown ();
      break;

    case FlashCompleteDoNothing: //Do not thing
      Status = SmmCsSvcIhisiFbtsApRequestDoNothing ();
      break;

    default:
      Status = EFI_SUCCESS;
      break;
  }

  return Status;
}

EFI_STATUS
UpdateOemFlashMap (
  VOID
  )
{
  EFI_STATUS                    Status;
  UINTN                         Index;
  FBTS_PLATFORM_ROM_MAP        *OemRomMapBuffer;
  FBTS_PLATFORM_PRIVATE_ROM    *OemPrivateRomMap;
  //
  //Update Oem Flash Map
  //
  ZeroMem ((VOID *)&mRomMapBuffer, sizeof (mRomMapBuffer));
  FillPlatformRomMapBuffer ((UINT8) FbtsRomMapEos, 0, 0, 0);
  ZeroMem ((VOID *)&mPrivateRomMapBuffer, sizeof (mPrivateRomMapBuffer));
  FillPlatformPrivateRomMapBuffer ((UINT8) FbtsRomMapEos, 0, 0);
  OemRomMapBuffer = NULL;
  OemPrivateRomMap = NULL;
  Status = SmmCsSvcIhisiFbtsGetOemFlashMap (&OemRomMapBuffer, &OemPrivateRomMap);
  if (Status == EFI_SUCCESS) {
    if (OemPrivateRomMap != NULL) {
      for (Index = 0; Index < sizeof(FBTS_PLATFORM_PRIVATE_ROM_BUFFER)/sizeof(FBTS_PLATFORM_PRIVATE_ROM); Index++ ) {
        Status = FillPlatformPrivateRomMapBuffer (
                  OemPrivateRomMap[Index].LinearAddress,
                  OemPrivateRomMap[Index].Size,
                  (UINT8) Index
                  );
        if (EFI_ERROR(Status)) {
          FillPlatformPrivateRomMapBuffer ((UINT8) FbtsRomMapEos, 0, 0);
          break;
        }
        if (OemPrivateRomMap[Index].LinearAddress == (UINT32)FbtsRomMapEos) {
          break;
        }
      }
    }

    if (OemRomMapBuffer != NULL) {
      for (Index = 0 ; Index < sizeof(FBTS_PLATFORM_ROM_MAP_BUFFER)/sizeof(FBTS_PLATFORM_ROM_MAP); Index++) {
        Status = FillPlatformRomMapBuffer (
                  OemRomMapBuffer[Index].Type,
                  OemRomMapBuffer[Index].Address,
                  OemRomMapBuffer[Index].Length,
                  (UINT8) Index
                  );
        if (EFI_ERROR(Status)) {
          FillPlatformRomMapBuffer ((UINT8) FbtsRomMapEos, 0, 0, 0);
          break;
        }
        if (OemRomMapBuffer[Index].Type == FbtsRomMapEos) {
          break;
        }
      }
    }
  }

  //
  //If Get Oem Flash Map fail.
  //Try to get Flash Map By Hob dynamically.
  //
  if (EFI_ERROR (Status)) {
    Status = GetFlashMapByHob ();
  }
  return EFI_SUCCESS;
}

EFI_STATUS
InstallFbtsServices (
  VOID
  )
{
  EFI_STATUS            Status;
  UINTN                 Index;
  //
  //Update Oem Flash Map
  //
  Status = UpdateOemFlashMap ();
  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR (Status);
  }

  for (Index = 0; Index < sizeof(FBTS_FUNCTION_TABLE)/sizeof(FBTS_FUNCTION_TABLE[0]); Index++) {
    Status = IhisiRegisterCommand (FBTS_FUNCTION_TABLE[Index].CmdNumber ,
                                   FBTS_FUNCTION_TABLE[Index].IhisiFunction,
                                   FBTS_FUNCTION_TABLE[Index].Priority);
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}

