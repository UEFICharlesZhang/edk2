/** @file
  Provide support functions for MultiConfig Update.
  
;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MultiConfigBaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Protocol/Smbios.h>
#include <Guid/EventGroup.h>
#include <Guid/DebugMask.h>
#include "MultiConfigUpdateDxe.h"
#include "DmiStringInformation.h"

STATIC UINT8                   *mDmiStart   = NULL;
STATIC UINT8                   *mDmiEnd     = NULL;
STATIC UINT8                   *mVarStart   = NULL;
STATIC UINT8                   *mVarEnd     = NULL;

/**
 Check if SmbiosTool or variable update the same data
 
 @param[in]         Type                Update Smbios data Type
 @param[in]         Offset              Update Smbios data Offset

 @retval TRUE                           SmbiosTool or Variable update the same data
 @retval FALSE                          No one update the same data
**/
BOOLEAN
CheckReplaceDataFromSmbiosTool (
  IN   UINT8             Type,
  IN   UINT8             Offset
  )
{
  DMI_STRING_STRUCTURE   *DmiEntry;

  if (mDmiStart == NULL && mVarStart == NULL) {
    return FALSE;
  }

  if (mDmiStart != NULL) {
    DmiEntry = (DMI_STRING_STRUCTURE *)mDmiStart;
    while ((UINT8 *)DmiEntry < mDmiEnd && DmiEntry->Type != 0xFF && DmiEntry->Length != 0) {
      if (DmiEntry->Valid == SMBIOS_DMI_VALID && DmiEntry->Type == Type && DmiEntry->Offset == Offset) {
        return TRUE;
      }
      DmiEntry = (DMI_STRING_STRUCTURE *)((CHAR8 *)DmiEntry + DmiEntry->Length);
    }
  }

  if (mVarStart != NULL) {
    DmiEntry = (DMI_STRING_STRUCTURE *)mVarStart;
    while ((UINT8 *)DmiEntry < mVarEnd && DmiEntry->Type != 0xFF && DmiEntry->Length != 0) {
      if (DmiEntry->Valid == SMBIOS_DMI_VALID && DmiEntry->Type == Type && DmiEntry->Offset == Offset) {
        return TRUE;
      }
      DmiEntry = (DMI_STRING_STRUCTURE *)((CHAR8 *)DmiEntry + DmiEntry->Length);
    }
  }
  return FALSE;
}

/**
 Update Smbios Data function

 @param[in]         This                Smbios protocol
 @param[in]         DMIData             Update Smbios Data
**/
VOID
UpdateSmbiosData (
  IN EFI_SMBIOS_PROTOCOL        *This,
  IN MC_SMBIOS_SETTING          *SmbiosData
)
{
  EFI_SMBIOS_TABLE_HEADER   *Record;
  UINT16                    Instance;
  UINT8                     *Data;
  EFI_STATUS                Status;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  BOOLEAN                   SmbiosToolUpdateData;

  Record = NULL;
  Data = NULL;
  Instance = 0;
  SmbiosHandle = MC_SMBIOS_HANDLE_PI_RESERVED;
  
  if ((This == NULL) || (SmbiosData == NULL)) {
    return;
  }

  while (TRUE) {
    Status = This->GetNext (This, &SmbiosHandle, &SmbiosData->Type, &Record, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }
    if (SmbiosData->Instance != Instance) {
      Instance ++;
      continue;
    }
    if (SmbiosData->DataAttr == SMBIOS_REMOVE_ATTR) {
      DEBUG ((EFI_D_ERROR, "Remove Smbios Type:%Xh Instance:%d \n", SmbiosData->Type, SmbiosData->Instance));
      Record->Type = SMBIOS_REMOVE_TYPE;
    } else {
      //
      // Check DMI Update the same data
      //
      SmbiosToolUpdateData = FALSE;
      if (mDmiStart != NULL || mVarStart != NULL) {
        SmbiosToolUpdateData = CheckReplaceDataFromSmbiosTool (SmbiosData->Type, (UINT8) SmbiosData->SmbiosOffset);
      }
      if (!SmbiosToolUpdateData) {
        Data = (UINT8 *) Record;
        CopyMem ((VOID *)(Data + SmbiosData->SmbiosOffset), (VOID *) SmbiosData->Data, SmbiosData->DataSize);
        DEBUG ((EFI_D_ERROR, "Update Smbios Type:%Xh Instance:%d Offset:%d, Size:%d\n", SmbiosData->Type, SmbiosData->Instance, SmbiosData->SmbiosOffset, SmbiosData->DataSize));
      } else {
        DEBUG ((EFI_D_ERROR, "Smbios Tool update the same Data\n"));
      }
    }
    break;
  } 
}

/**
 Update Smbios String function

 @param[in]         This                Smbios protocol
 @param[in]         DMIData             Update Smbios Data
**/
VOID
UpdateSmbiosString (
  IN  EFI_SMBIOS_PROTOCOL       *This,
  IN  MC_SMBIOS_SETTING         *SmbiosData
)
{
  EFI_STATUS                Status;
  BOOLEAN                   SmbiosToolUpdateString;
  UINTN                     StringNum;
  CHAR8                     *UpdateString;
  EFI_SMBIOS_TABLE_HEADER   *Record;
  UINT8                     *RecordData;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  UINT16                    Instance;
  UINTN                     Index;

  SmbiosToolUpdateString = FALSE;
  StringNum = 0;
  UpdateString = NULL;
  SmbiosHandle = 0xFFFE;
  Instance = 0;
  
  if ((This == NULL) || (SmbiosData == NULL)) {
    return;
  }
  
  do {
    Status = This->GetNext (This, &SmbiosHandle, &SmbiosData->Type, &Record, NULL);
    Instance ++;
  } while (!((Instance == SmbiosData->Instance + 1) || EFI_ERROR (Status)));
  
  RecordData = (UINT8 *)Record;
  if (!EFI_ERROR (Status)) {
    //
    // Check DMI Update the same string
    //
    if (mDmiStart != NULL || mVarStart != NULL) {
      SmbiosToolUpdateString = CheckReplaceDataFromSmbiosTool (SmbiosData->Type, (UINT8) SmbiosData->SmbiosOffset);
    }
    //
    // If DMI not update the same string, MultiConfig Update this string
    //
    if (!SmbiosToolUpdateString) {
      StringNum = RecordData[SmbiosData->SmbiosOffset];
      UpdateString = AllocateZeroPool (SmbiosData->DataSize + 1);
      if (UpdateString != NULL) {
        CopyMem ((VOID *) UpdateString, (VOID *) (SmbiosData->Data), SmbiosData->DataSize);
        This->UpdateString (This, &SmbiosHandle, &StringNum, UpdateString);

DEBUG_CODE_BEGIN ();
        DEBUG ((EFI_D_ERROR, "Update Smbios Type:%Xh Instance:%d Offset:%d String:", SmbiosData->Type, SmbiosData->Instance, SmbiosData->SmbiosOffset));
        for (Index = 0; Index < SmbiosData->DataSize; Index ++) {
          DEBUG ((EFI_D_ERROR, "%c", UpdateString[Index]));
        }
        DEBUG ((EFI_D_ERROR, "\n"));
DEBUG_CODE_END ();
        
        FreePool (UpdateString);
        UpdateString = NULL;
      }
    } else {
      DEBUG ((EFI_D_ERROR, "Smbios Tool update the same String\n"));
    }
  }

  return;
}

/**
 Update Smbios information function

 @param[in]         Event               Event whose notification function is being invoked
 @param[in]         Context             Pointer to the notification function's context
**/
VOID
EFIAPI
UpdateSmbiosInfo (
  IN  EFI_EVENT                               Event,
  IN  VOID                                    *Context
  )
{
  EFI_STATUS                            Status;
  VOID                                  *MultiConfigRegionBase;
  UINT16                                ConfigCount;
  CONFIG_HEADER                         *TempConfigHeader;
  UINT16                                Index;
  MC_SMBIOS_HEADER                      *SmbiosStart;
  UINT16                                Totalitems;
  EFI_SMBIOS_PROTOCOL                   *SmbiosProtocol;
  UINT16                                ItemIndex;
  MC_SMBIOS_SETTING                     *SmbiosData;
  UINTN                                 VarSize;
  UINT64                                Signature;

  ConfigCount = 0;
  SmbiosStart = NULL;
  Totalitems = 0;
  SmbiosProtocol= NULL;
  SmbiosData = NULL;
  
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID**)&SmbiosProtocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Locate SMBIOS protocol failed (%r)!!\n", Status));
    return;
  }
  
  //
  // Get Dmi & Variable
  // If Dmi or Variable Update the same string as Multiconfig
  // Multiconfig will not update this string
  // 
  mDmiStart = (UINT8*)(UINTN) (PcdGet32 (PcdFlashNvStorageDmiBase));
  if (*(UINT32*)mDmiStart != DMI_UPDATE_STRING_SIGNATURE) {
    DEBUG ((EFI_D_ERROR, "DMI region signature error!!\n"));
    mDmiStart = NULL;
  } else {
    mDmiEnd = mDmiStart + PcdGet32 (PcdFlashNvStorageDmiSize);
    mDmiStart += sizeof (DMI_UPDATE_STRING_SIGNATURE);
  }
  
  Status = GetVariable2 (L"PnpRuntime", &gEfiGenericVariableGuid, (VOID **)&mVarStart, &VarSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Get Variable PnpRuntime Fail - %r!!\n", Status));
    
    if (mVarStart != NULL) {
      FreePool (mVarStart);
      mVarStart = NULL;
    }
  } else if (mVarStart != NULL) {
    if (*(UINT32 *) mVarStart != DMI_UPDATE_STRING_SIGNATURE) {
      DEBUG ((EFI_D_ERROR, "Variable PnpRuntime signature error!!\n"));
      FreePool (mVarStart);
      mVarStart = NULL;
    } else {
      mVarEnd = mVarStart + VarSize;
      mVarStart += sizeof (DMI_UPDATE_STRING_SIGNATURE);
    }
  }
  
  MultiConfigRegionBase = (VOID*) (UINTN)PcdGet32 (PcdFlashNvStorageMultiConfigBase);
  //
  // Check MultiConfig Header
  //
  Signature = MULTI_CONFIG_SIGNATURE;
  if (CompareMem (&Signature, MultiConfigRegionBase, sizeof (UINT64)) != 0) {
    return;
  }
  //
  // Check CRC32 for Multi Config Header
  //
  Status = CheckMultiConfigHeaderCrc32 (MultiConfigRegionBase);
  if (EFI_ERROR (Status)) {
    return;
  }
  ConfigCount = GetConfigCount ();
  
  for (Index = 0; Index < ConfigCount; Index ++) {
    TempConfigHeader = GetConfigHeaderByOrder (Index);

    if ((TempConfigHeader->Type == SMBIOS_MC_UPDATE_SMBIOS) && (TempConfigHeader->SkuId == MC_VALID_SKUID) && (TempConfigHeader->Attribute & MULTI_CONFIG_ATTRIBUTE_DEFAULT)) {
      SmbiosStart = (MC_SMBIOS_HEADER *) GetConfigDataByOrder (Index, MultiConfigRegionBase);
      Status = CheckMultiConfigDataCrc32 (TempConfigHeader->Crc32, TempConfigHeader->ConfigDataSize, SmbiosStart);
      if (EFI_ERROR (Status)) {
        continue;
      }
      Totalitems = SmbiosStart->TotalItem;
      SmbiosData = &SmbiosStart->FirstData;
      
      for (ItemIndex = 0; ItemIndex < Totalitems; ItemIndex ++) {
        DEBUG ((EFI_D_ERROR, "Try to Update Smbios Type:%Xh Instance:%d Offset:%d Attribute:%d\n", SmbiosData->Type, SmbiosData->Instance, SmbiosData->SmbiosOffset, SmbiosData->DataAttr));
        if (SmbiosData->DataSize > MC_UPDATE_SMBIOS_MAX_LENGTH) {
          break;
        }
        if (SmbiosData->DataAttr == SMBIOS_STRING_ATTR) {
          UpdateSmbiosString (SmbiosProtocol, SmbiosData);
        } else {
          UpdateSmbiosData (SmbiosProtocol, SmbiosData);
        }
        SmbiosData = (MC_SMBIOS_SETTING *) ((UINT8 *)SmbiosData + SmbiosData->DataSize + sizeof (MC_SMBIOS_SETTING_L));
      }
    }
  }  
  
  if (mVarStart != NULL) {
    FreePool (mVarStart);
  }
  return;
}

/**
 Entry point of Smbios Update Entry

 @param[in]         ImageHandle         A handle for this module
 @param[in]         SystemTable         A pointer to the EFI System Table

 @retval EFI_SUCCESS                    No Multiconfig or set call back success
 @retval EFI_INVALID_PARAMETER          One or more parameters are invalid.
 @retval EFI_OUT_OF_RESOURCES           The event could not be allocated.
**/
EFI_STATUS
MultiConfigUpdateEntry (
  IN      EFI_HANDLE          ImageHandle,
  IN      EFI_SYSTEM_TABLE    *SystemTable
  )
{
  EFI_EVENT                             EndOfDxeEvent;
  EFI_STATUS                            Status;
  EFI_HANDLE                            Handle;

  Handle = NULL;
  Status = EFI_SUCCESS;
  if (FeaturePcdGet (PcdH2OMultiConfigUpdateSMBiosSupported)) {
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    UpdateSmbiosInfo,
                    NULL,
                    &gEfiEndOfDxeEventGroupGuid,
                    &EndOfDxeEvent
                    );  
  }
  return  Status;
}

