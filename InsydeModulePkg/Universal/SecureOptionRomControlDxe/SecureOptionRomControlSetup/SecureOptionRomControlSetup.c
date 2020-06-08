/** @file

  Initial and callback functions for Secure Option Rom Control Setup page

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

#include "PciDevLookupTable.h"
#include "SecureOptionRomControlSetup.h"
 
BOOLEAN                             mSetupInitDone = FALSE;
EFI_HANDLE                          mDriverHandle = NULL;
SECURE_OPROM_CONTROL_PRIVATE_DATA   *mPrivateData = NULL;
CHAR16                              mVariableName[] = L"SecOpRomConfig";

HII_VENDOR_DEVICE_PATH  mHiiVendorDevicePath0 = {
  {
    {
      MEDIA_DEVICE_PATH,
      MEDIA_RELATIVE_OFFSET_RANGE_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    SECURE_OPTION_ROM_CONTROL_FORMSET_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

LIST_ENTRY                   mOpRomPolicyListForSetup;

OPTION_ROM_DEVICE_LIST       *mOpRomDeviceList; // For delete function used.

/**
  Get all option Rom name list and return the number of them.

  @param[in,out] *OptionNumber    - The pointer of the total number of Option Rom policy.

  @retval OPTION_ROM_LIST_OPTION  - Get the option just been created
  @retval NULL                    - Failed to get the new option

**/
CHAR16 **
EFIAPI
GetOpRomNameList (
  IN OUT    UINT32                       *OptionNumber
  )
{
  OPTION_ROM_LIST_OPTION       *Option;
  CHAR16                       **OpRomNameList;
  EFI_LIST_ENTRY               *OptionRomPolicyList;
  EFI_LIST_ENTRY               *Link;
  UINT32                       OptionNum;
  UINT32                       Index;
  CHAR16                       *PciVendorIDString = NULL;

  if (mOpRomPolicyListForSetup.ForwardLink == NULL) {
    return NULL;
  }

  OptionRomPolicyList = &mOpRomPolicyListForSetup;
  OptionNum = 0;
  for (Link = OptionRomPolicyList->ForwardLink; Link != OptionRomPolicyList; Link = Link->ForwardLink) {
    OptionNum++;
  }

  OpRomNameList = AllocateZeroPool (sizeof (CHAR16*) * OptionNum);
  if (OpRomNameList == NULL) {
    return NULL;
  }

  mOpRomDeviceList = AllocateZeroPool (sizeof (OPTION_ROM_DEVICE_LIST) * OptionNum);
  if (mOpRomDeviceList == NULL) {
    return NULL;
  }

  Index = 0;
  for (Link = OptionRomPolicyList->ForwardLink; Link != OptionRomPolicyList; Link = Link->ForwardLink) {
    Option = CR (Link, OPTION_ROM_LIST_OPTION, Link, OPTION_ROM_LIST_OPTION_SIGNATURE);
    mOpRomDeviceList[Index].VendorId = Option->OpRomOption.VendorId;
    mOpRomDeviceList[Index].DeviceId = Option->OpRomOption.DeviceId;
    OpRomNameList[Index] = AllocateZeroPool (sizeof (CHAR16) * MAX_OPROM_NAME_ID_NUM);
    if (OpRomNameList[Index] == NULL) {
      break;
    }
    GetVerdorIdShortName (Option->OpRomOption.VendorId, &PciVendorIDString);
    if (PciVendorIDString == NULL) {
      UnicodeSPrint (OpRomNameList[Index] , sizeof (CHAR16) * MAX_OPROM_NAME_ID_NUM, L"0x%04x 0x%04x", Option->OpRomOption.VendorId, Option->OpRomOption.DeviceId);
    } else {
      UnicodeSPrint (OpRomNameList[Index] , sizeof (CHAR16) * MAX_OPROM_NAME_ID_NUM, L"%s 0x%04x", PciVendorIDString, Option->OpRomOption.DeviceId);
    }
    Index++;
  }

  *OptionNumber = Index;

  return OpRomNameList;

}

/**
  Build the OpRom#### option from the VariableName and the build OpRom#### will
  also be linked to BdsCommonOptionList

  @param[in,out] *OpRomList   - The header of the OpRom#### option link list.
  @param[in]     OptionNumber - The number for the OpRom#### variable.

  @retval OPTION_ROM_LIST_OPTION  - Get the option just been created
  @retval NULL                    - Failed to get the new option

**/
OPTION_ROM_LIST_OPTION *
EFIAPI
OpRomVariableToOption (
  IN OUT EFI_LIST_ENTRY               *OpRomList,
  IN     UINT16                       OptionNumber
  )
{
  UINTN                        VariableSize;
  UINT8                        *Variable;
  OPTION_ROM_LIST_OPTION       *Option;
  CHAR16                       OptionName[20];
  EFI_STATUS                   Status;
  OPTION_ROM_OPTION            *OpRomOption;

  UnicodeSPrint (OptionName, sizeof (OptionName), L"OpRom%04x", OptionNumber);
  //
  // Read the variable. We will never free this data.
  //
  Status = CommonGetVariableDataAndSize (
             OptionName,
             &gH2OSecureOptionRomControlFormsetGuid,
             &VariableSize,
             (VOID **) &Variable
             );
  if (EFI_ERROR (Status) || (Variable == NULL)) {
    DEBUG ((EFI_D_ERROR, "GetVariable %s: %r\n", OptionName, Status));
    return NULL;
  }

  //
  // Validate the variable content.
  //
  OpRomOption = (OPTION_ROM_OPTION *) Variable;
  Status = ValidateOpRomVariable (OpRomOption);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Validate Variable %s: %r\n", OptionName, Status));
    return NULL;
  }

  Option = AllocateZeroPool (sizeof (OPTION_ROM_LIST_OPTION));
  if (Option == NULL) {
    DEBUG ((EFI_D_ERROR, "AllocatePool for Option to %s: Fail.\n", OptionName));
    return NULL;
  }
  Option->Signature = OPTION_ROM_LIST_OPTION_SIGNATURE;
  CopyMem (&Option->OpRomOption, Variable, sizeof (OPTION_ROM_OPTION));
  Option->OptionNumber = OptionNumber;
  InsertTailList (OpRomList, &Option->Link);
  gBS->FreePool (Variable);
  return Option;
}

/**
  This function use OpRom#### and OpRomOrder variable to initialize
  option ROM list.

  @param[in] RomOptionList  - The header of the OptionRom policy link list

  @retval EFI_SUCCESS         - Initialize option ROM list successful.
  @retval EFI_ALREADY_STARTED - The option ROM list has been initialized

**/
EFI_STATUS
EFIAPI
GetOptionRomPolicyList (
  IN EFI_LIST_ENTRY      *RomOptionList
  )
{
  UINT16                   *OptionOrder;
  UINTN                    OptionOrderSize;
  UINTN                    Index;
  OPTION_ROM_LIST_OPTION   *Option;
  EFI_STATUS               Status;

  Status = CommonGetVariableDataAndSize (
             L"OpRomOrder",
             &gH2OSecureOptionRomControlFormsetGuid,
             &OptionOrderSize,
             (VOID **) &OptionOrder
             );
  if (EFI_ERROR (Status) || (OptionOrder == NULL)) {
    return Status;
  }

  for (Index = 0; Index < OptionOrderSize / sizeof (UINT16); Index++) {
    DEBUG ((EFI_D_ERROR, "OpRomVariableToOption() OptionOrder[%d]: %04x\n", Index, OptionOrder[Index]));
    Option = OpRomVariableToOption (RomOptionList, OptionOrder[Index]);
  }

  if (OptionOrder != NULL) {
    gBS->FreePool (OptionOrder);
  }

  return EFI_SUCCESS;
}

/**
  Initialize Secure Option Rom Control sub-menu for setuputility use.

  @param[in]     HiiHandle          - Hii hanlde for the call back routine.
  @param[in]     *OptionList        - The header of the Option Rom policy link list
  @param[in]     *StartOpCodeHandle - Handle to the buffer of opcodes.
  @param[in]     CreateType         - Type of the Option Rom. (Active/Inactive)
  @param[in,out] *OptionNum         - Number of Option Rom option.

  @retval EFI_SUCCESS  -  Function has completed successfully.
  @retval Others       -  Error occurred during execution.

**/
EFI_STATUS
EFIAPI
CreateOptionRomPolicies (
  IN      EFI_HII_HANDLE         HiiHandle,
  IN      EFI_LIST_ENTRY         *OptionList,
  IN      VOID                   *StartOpCodeHandle,
  IN      VOID                   *EndOpCodeHandle,
  IN      UINT8                  CreateType,
  IN OUT  UINTN                  *OptionNum
  )
{
  EFI_LIST_ENTRY                *Link;
  VOID                          *OptionsOpCodeHandle;
  UINT8                         OptionFlags;
  UINT8                         QuestionFlags;
  UINTN                         IfrOptionCount;
  CHAR16                        *NewString;
  CHAR16                        OpRomName[MAX_STRING_BUFFER_NUM];
  CHAR16                        OpRomHelp[MAX_HELP_BUFFER_NUM];
  OPTION_ROM_LIST_OPTION        *Option;
  EFI_STRING_ID                 OptionRomStringId;
  EFI_STRING_ID                 OptionRomHelpStringId;
  UINTN                         Index;
  EFI_STRING_ID                 OpRomTypeStrings [] = {
                                  STRING_TOKEN (STR_NONE_TEXT),
                                  STRING_TOKEN (STR_LEGACY_ROM_TEXT),
                                  STRING_TOKEN (STR_UNSIGNED_EFI_ROM_TEXT),
                                  STRING_TOKEN (STR_SIGNED_EFI_ROM_TEXT),
                                  STRING_TOKEN (STR_ALL_TEXT)
                                  };
  SECURE_OPROM_CONTROL_CONFIGURATION  SecureOpRonConfig = {0};
  CHAR16                        *PciVendorIDString = NULL;
  UINT8                         OpRomPolicy;
  OPTION_ROM_POLICY_SETTING     OpRomPolicySetting = {0};

  if (OptionList == NULL || StartOpCodeHandle == NULL || OptionNum == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (FALSE == HiiGetBrowserData (&gH2OSecureOptionRomControlFormsetGuid, mVariableName, sizeof(SECURE_OPROM_CONTROL_CONFIGURATION), (UINT8 *)&SecureOpRonConfig)) {
    return EFI_NOT_FOUND;
  }
  OpRomPolicy = SecureOpRonConfig.OptionRomPolicy;
  IfrOptionCount = sizeof (OpRomTypeStrings) / sizeof (EFI_STRING_ID);

  //
  // Dynamic create all of OP ROM policy
  //
  for (Link = OptionList->ForwardLink; Link != OptionList; Link = Link->ForwardLink) {
    Option = CR (Link, OPTION_ROM_LIST_OPTION, Link, OPTION_ROM_LIST_OPTION_SIGNATURE);
    if (((Option->OpRomOption.Attribute & OP_ROM_ACTIVE) == OP_ROM_ACTIVE && (CreateType & OP_ROM_ACTIVE) == OP_ROM_ACTIVE) ||
        ((Option->OpRomOption.Attribute & OP_ROM_ACTIVE) != OP_ROM_ACTIVE && (CreateType & OP_ROM_INACTIVE) == OP_ROM_INACTIVE)  ) {

      GetVerdorIdShortName (Option->OpRomOption.VendorId, &PciVendorIDString);

      if (OpRomPolicy == OPTION_ROM_POLICY_AUTO) {
        //
        // Title: L" Vendor ID - Devive ID"
        //
        if (PciVendorIDString == NULL) {
          UnicodeSPrint (
            OpRomName,
            sizeof (OpRomName),
            L"  0x%04x - 0x%04x",
            Option->OpRomOption.VendorId,
            Option->OpRomOption.DeviceId
            );
        } else {
          UnicodeSPrint (
            OpRomName,
            sizeof (OpRomName),
            L"  %s - 0x%04x",
            PciVendorIDString,
            Option->OpRomOption.DeviceId
            );
        }
      } else {
        //
        // Title: L" Vendor ID - Devive ID (Active/Inactive)"
        //
        if (PciVendorIDString == NULL) {
          UnicodeSPrint (
            OpRomName,
            sizeof (OpRomName),
            L"  0x%04x - 0x%04x %s",
            Option->OpRomOption.VendorId,
            Option->OpRomOption.DeviceId,
            (((CreateType & OP_ROM_ACTIVE) == OP_ROM_ACTIVE) ? L"(Active)":L"(Inactive)")
            );
        } else {
          UnicodeSPrint (
            OpRomName,
            sizeof (OpRomName),
            L"  %s - 0x%04x %s",
            PciVendorIDString,
            Option->OpRomOption.DeviceId,
            (((CreateType & OP_ROM_ACTIVE) == OP_ROM_ACTIVE) ? L"(Active)":L"(Inactive)")
            );
        }
      }


      OptionsOpCodeHandle = HiiAllocateOpCodeHandle ();
      ASSERT (OptionsOpCodeHandle != NULL);

      for (Index = 0; Index < IfrOptionCount; Index++) {
        OptionFlags = 0;
        if (Option->OpRomOption.OpRomPolicy == Index) {
          OptionFlags = EFI_IFR_OPTION_DEFAULT;

          if (HiiGetBrowserData (&gH2OSecureOptionRomControlFormsetGuid, L"OpRomSetting", sizeof(OPTION_ROM_POLICY_SETTING), (UINT8 *)&OpRomPolicySetting)) {
            OpRomPolicySetting.OptionRomPolicySetting[Option->OptionNumber] = (UINT8) Index;
            DEBUG ((EFI_D_ERROR, "OpRomPolicySetting[%d]: %d\n", Option->OptionNumber, OpRomPolicySetting.OptionRomPolicySetting[Option->OptionNumber]));
            HiiSetBrowserData (
              &gH2OSecureOptionRomControlFormsetGuid,
              L"OpRomSetting",
              sizeof (OPTION_ROM_POLICY_SETTING),
              (UINT8 *) &OpRomPolicySetting,
              NULL
              );
          }
        }
        HiiCreateOneOfOptionOpCode (
          OptionsOpCodeHandle,
          OpRomTypeStrings[Index],
          OptionFlags,
          EFI_IFR_TYPE_NUM_SIZE_8,
          (UINT8)Index
        );
      }

      NewString = HiiGetString (
                    HiiHandle,
                    STRING_TOKEN (STR_OPTION_ROM_POLICY_HELP),
                    NULL
                    );
      UnicodeSPrint (
        OpRomHelp,
        sizeof (OpRomHelp),
        L"%s\nSupported ROM Type:\n- Legacy ROM: %s\n- Unsigned UEFI ROM: %s\n- Signed UEFI ROM: %s",
        NewString,
        ((Option->OpRomOption.FoundRomType & HAVE_LEGACY_ROM) ? L"YES":L"NO"),
        ((Option->OpRomOption.FoundRomType & HAVE_UNSIGNED_EFI_ROM) ? L"YES":L"NO"),
        ((Option->OpRomOption.FoundRomType & HAVE_SIGNED_EFI_ROM) ? L"YES":L"NO")
        );

      OptionRomStringId = HiiSetString (HiiHandle, 0, OpRomName, NULL);
      OptionRomHelpStringId = HiiSetString (HiiHandle, 0, OpRomHelp, NULL);
      QuestionFlags = EFI_IFR_FLAG_CALLBACK;

      HiiCreateOneOfOpCode (
        StartOpCodeHandle,
        (EFI_QUESTION_ID) (KEY_OPTION_ROM_POLICY_BASE + *OptionNum),
        SETTING_VARSTORE_ID,
        Option->OptionNumber,
        OptionRomStringId,
        OptionRomHelpStringId,
        QuestionFlags,
        EFI_IFR_NUMERIC_SIZE_1,
        OptionsOpCodeHandle,
        NULL
        );

      HiiUpdateForm (
        HiiHandle,
        &gH2OSecureOptionRomControlFormsetGuid,
        (EFI_FORM_ID) SECURE_OPTION_ROM_CONTROL_FORM_ID,
        StartOpCodeHandle,
        EndOpCodeHandle
        );

      HiiFreeOpCodeHandle (OptionsOpCodeHandle);
      (*OptionNum)++;
      gBS->FreePool (NewString);
    }
  }

  return EFI_SUCCESS;
}

/**
  Initialize Secure Option Rom Control sub-menu for setuputility use.

  @param[in] HiiHandle   - Hii hanlde for the call back routine.
  @param[in] OptionList  - Hii hanlde for the call back routine.

  @retval EFI_SUCCESS  -  Function has completed successfully.
  @retval Others       -  Error occurred during execution.

**/
EFI_STATUS
EFIAPI
UpdateSecureOpRomDynamicItems (
  IN EFI_HII_HANDLE       HiiHandle,
  IN EFI_LIST_ENTRY       *OptionList
  )
{
  VOID                          *StartOpCodeHandle = NULL;
  VOID                          *EndOpCodeHandle = NULL;
  EFI_IFR_GUID_LABEL            *StartLabel;
  EFI_IFR_GUID_LABEL            *EndLabel;
  EFI_STATUS                    Status;
  UINTN                         OpRomPolicyNum;

  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);

  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  StartLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = SECURE_OPTION_ROM_POLICY_LABEL;

  EndLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = SECURE_OPTION_ROM_POLICY_END_LABEL;

  OpRomPolicyNum = 0;
  CreateOptionRomPolicies (HiiHandle, OptionList, StartOpCodeHandle, EndOpCodeHandle, OP_ROM_ACTIVE, &OpRomPolicyNum);
  CreateOptionRomPolicies (HiiHandle, OptionList, StartOpCodeHandle, EndOpCodeHandle, OP_ROM_INACTIVE, &OpRomPolicyNum);

  Status = HiiUpdateForm (
             HiiHandle,
             &gH2OSecureOptionRomControlFormsetGuid,
             (EFI_FORM_ID) SECURE_OPTION_ROM_CONTROL_FORM_ID,
             StartOpCodeHandle,
             EndOpCodeHandle
             );

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);

  return Status;
}

/**
  Initialize Secure Option Rom Control sub-menu for setuputility use

  @param[in] HiiHandle   - Hii hanlde for the call back routine

  @retval EFI_SUCCESS  -  Function has completed successfully.
  @retval Others       -  Error occurred during execution.

**/
EFI_STATUS
EFIAPI
InitSecureOpRomSubMenu (
  IN EFI_HII_HANDLE       HiiHandle
  )
{

  EFI_STATUS                         Status;
  UINTN                              BufferSize;
  SECURE_OPROM_CONTROL_CONFIGURATION Configuration = {0};


  BufferSize = sizeof (SECURE_OPROM_CONTROL_CONFIGURATION);
  Status = gRT->GetVariable (
                  mVariableName,
                  &gH2OSecureOptionRomControlFormsetGuid,
                  NULL,
                  &BufferSize,
                  &Configuration
                  );
  if (!EFI_ERROR (Status)) {
    HiiSetBrowserData (
      &gH2OSecureOptionRomControlFormsetGuid,
      mVariableName,
      sizeof (SECURE_OPROM_CONTROL_CONFIGURATION),
      (UINT8 *) &Configuration,
      NULL
      );
  }

  if (mOpRomPolicyListForSetup.ForwardLink != NULL) {
    FreeOptionRomPolicyList ();
  }

  InitializeListHead (&mOpRomPolicyListForSetup);
  GetOptionRomPolicyList (&mOpRomPolicyListForSetup);

  Status = UpdateSecureOpRomDynamicItems (HiiHandle, &mOpRomPolicyListForSetup);
  if (!EFI_ERROR (Status)) {
    mSetupInitDone = TRUE;
  }

  return Status;
}

/**
  Find OP ROM policy in active/inactive Option ROM list.

  @param[in] OpRomPolicyNum   - Number of Option ROM list.

  @retval Option    -  OpRom entry in Option ROM list.
  @retval NULL      -  Not found the OpRom entry.

**/
OPTION_ROM_LIST_OPTION *
EFIAPI
FindOptionRomPolicy (
  IN  UINT16                                 OpRomPolicyNum
  )
{
  EFI_STATUS                 Status;
  UINT16                     Index;
  EFI_LIST_ENTRY             *OptionRomPolicyList;
  EFI_LIST_ENTRY             *Link;
  OPTION_ROM_LIST_OPTION     *Option;
  BOOLEAN                    OptionRomPolicyFound;

  if (mOpRomPolicyListForSetup.ForwardLink == NULL) {
    return NULL;
  }

  OptionRomPolicyList = &mOpRomPolicyListForSetup;
  Index = 0;
  OptionRomPolicyFound = FALSE;
  Option = NULL;
  Status = EFI_NOT_FOUND;
  //
  // Try to find matched OP ROM policy in active OP ROM Policy list first
  //
  for (Link = OptionRomPolicyList->ForwardLink; Link != OptionRomPolicyList; Link = Link->ForwardLink) {
    Option = CR (Link, OPTION_ROM_LIST_OPTION, Link, OPTION_ROM_LIST_OPTION_SIGNATURE);
    if ((Option->OpRomOption.Attribute & OP_ROM_ACTIVE) == OP_ROM_ACTIVE) {
      if (Index == OpRomPolicyNum) {
        OptionRomPolicyFound = TRUE;
        break;
      }
      Index++;
    }
  }
  //
  // If can not find OP ROM policy in active OP ROM list, try to find mathced OP
  // ROM in inactive option ROM policy list
  //
  if (!OptionRomPolicyFound) {
    for (Link = OptionRomPolicyList->ForwardLink; Link != OptionRomPolicyList; Link = Link->ForwardLink) {
      Option = CR (Link, OPTION_ROM_LIST_OPTION, Link, OPTION_ROM_LIST_OPTION_SIGNATURE);
      if ((Option->OpRomOption.Attribute & OP_ROM_INACTIVE) == OP_ROM_INACTIVE) {
        if (Index == OpRomPolicyNum) {
          OptionRomPolicyFound = TRUE;
          break;
        }
        Index++;
      }
    }
  }
  if (!OptionRomPolicyFound) {
    Option = NULL;
  }
  return Option;

}

/**
  Update Option ROM policy in Option ROM list.

  @param[in] OpRomPolicyNum   - Number of Option ROM list.
  @param[in] Type             - Policy of the OpRom entry.

  @retval EFI_SUCCESS     -  OpRom entry in Option ROM list.
  @retval EFI_NOT_FOUND   -  Not found the OpRom entry.

**/
EFI_STATUS
EFIAPI
UpdateOptionRomPolicy (
  IN  UINT16                                 OpRomPolicyNum,
  IN  UINT8                                  Type
  )
{
  OPTION_ROM_LIST_OPTION     *Option;
  EFI_STATUS                 Status;

  Status = EFI_NOT_FOUND;
  Option = FindOptionRomPolicy (OpRomPolicyNum);
  if (Option != NULL) {
    Option->OpRomOption.OpRomPolicy = Type;
    Status = EFI_SUCCESS;
  }
  return Status;
}

/**
  Find Option ROM policy with related Vendor ID/ Device ID.

  @param[in] VendorId   - Vendor ID of the related PCI Option ROM.
  @param[in] DeviceId   - Device ID of the related PCI Option ROM.

  @retval Option    -  OpRom entry in Option ROM list.
  @retval NULL      -  Not found the OpRom entry.

**/
OPTION_ROM_LIST_OPTION *
EFIAPI
FindOptionRomPolicyByID (
  IN  UINT16      VendorId,
  IN  UINT16      DeviceId
  )
{
  EFI_STATUS                 Status;
  UINT16                     Index;
  EFI_LIST_ENTRY             *OptionRomPolicyList;
  EFI_LIST_ENTRY             *Link;
  OPTION_ROM_LIST_OPTION     *Option;
  BOOLEAN                    OptionRomPolicyFound;

  if (mOpRomPolicyListForSetup.ForwardLink == NULL) {
    return NULL;
  }

  OptionRomPolicyList = &mOpRomPolicyListForSetup;
  Index = 0;
  OptionRomPolicyFound = FALSE;
  Option = NULL;
  Status = EFI_NOT_FOUND;
  //
  // Try to find matched OP ROM policy in active OP ROM Policy list first
  //
  for (Link = OptionRomPolicyList->ForwardLink; Link != OptionRomPolicyList; Link = Link->ForwardLink) {
    Option = CR (Link, OPTION_ROM_LIST_OPTION, Link, OPTION_ROM_LIST_OPTION_SIGNATURE);
    if ((Option->OpRomOption.Attribute & OP_ROM_ACTIVE) == OP_ROM_ACTIVE) {
      if ((Option->OpRomOption.VendorId == VendorId) && (Option->OpRomOption.DeviceId == DeviceId)) {
        OptionRomPolicyFound = TRUE;
        break;
      }
      Index++;
    }
  }
  //
  // If can not find OP ROM policy in active OP ROM list, try to find mathced OP
  // ROM in inactive option ROM policy list
  //
  if (!OptionRomPolicyFound) {
    for (Link = OptionRomPolicyList->ForwardLink; Link != OptionRomPolicyList; Link = Link->ForwardLink) {
      Option = CR (Link, OPTION_ROM_LIST_OPTION, Link, OPTION_ROM_LIST_OPTION_SIGNATURE);
      if ((Option->OpRomOption.Attribute & OP_ROM_INACTIVE) == OP_ROM_INACTIVE) {
        if ((Option->OpRomOption.VendorId == VendorId) && (Option->OpRomOption.DeviceId == DeviceId)) {
          OptionRomPolicyFound = TRUE;
          break;
        }
        Index++;
      }
    }
  }
  if (!OptionRomPolicyFound) {
    Option = NULL;
  }
  return Option;

}

/**
  Get new OP ROM option number for OpRom#### variable

  @param[out] NewOptionNum  - Pointer to new OP ROM option number

  @retval EFI_SUCCESS   - Get New OP ROM option number

**/
EFI_STATUS
EFIAPI
GetNewOpRomOptionNoForSetup (
  OUT UINT16             *NewOptionNum
  )
{
  EFI_STATUS                 Status;
  UINT16                     Index;
  EFI_LIST_ENTRY             *OptionRomPolicyList;
  EFI_LIST_ENTRY             *Link;
  OPTION_ROM_LIST_OPTION     *Option;
  UINT16                     OptionNum;
  BOOLEAN                    OptionNumFound;

  if (mOpRomPolicyListForSetup.ForwardLink == NULL) {
    *NewOptionNum = 0;
    return EFI_SUCCESS;
  }

  OptionNum = 0;
  OptionRomPolicyList = &mOpRomPolicyListForSetup;
  Index = 0;
  Option = NULL;
  Status = EFI_NOT_FOUND;

  for (Link = OptionRomPolicyList->ForwardLink; Link != OptionRomPolicyList; Link = Link->ForwardLink) {
    OptionNum++;
  }

  for (Index = 0; Index < OptionNum; Index++) {
    OptionNumFound = FALSE;
    for (Link = OptionRomPolicyList->ForwardLink; Link != OptionRomPolicyList; Link = Link->ForwardLink) {
      Option = CR (Link, OPTION_ROM_LIST_OPTION, Link, OPTION_ROM_LIST_OPTION_SIGNATURE);
      if (Option->OptionNumber == Index) {
        OptionNumFound = TRUE;
      }
    }
    if (!OptionNumFound) {
      break;
    }
  }
  *NewOptionNum = Index;
  return EFI_SUCCESS;
}

/**
  Insert the Option ROM policy into Option ROM policy list.

  @param[in] HiiHandle  - Hii hanlde for the call back routine.
  @param[in] VendorId   - Vendor ID of the related PCI Option ROM.
  @param[in] DeviceId   - Device ID of the related PCI Option ROM.

  @retval EFI_SUCCESS - Insert the Option ROM policy entry success.
  @retval other       - Insert the Option ROM policy entry failure.

**/
EFI_STATUS
EFIAPI
InsertOptionRomPolicy (
  IN  EFI_HII_HANDLE  HiiHandle,
  IN  UINT16          VendorId,
  IN  UINT16          DeviceId
  )
{
  OPTION_ROM_LIST_OPTION              *Option;
  EFI_STATUS                          Status;
  SECURE_OPROM_CONTROL_CONFIGURATION  SecureOpRonConfig = {0};

  //
  // Check this policy exist or not?
  //
  Option = FindOptionRomPolicyByID (VendorId, DeviceId);
  if (Option != NULL) {
    return EFI_ABORTED;
  }

  Option = AllocateZeroPool (sizeof (OPTION_ROM_LIST_OPTION));
  if (Option == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (FALSE == HiiGetBrowserData (&gH2OSecureOptionRomControlFormsetGuid, mVariableName, sizeof(SECURE_OPROM_CONTROL_CONFIGURATION), (UINT8 *)&SecureOpRonConfig)) {
    return EFI_NOT_FOUND;
  }

  Option->Signature = OPTION_ROM_LIST_OPTION_SIGNATURE;
  Option->OpRomOption.Attribute = OP_ROM_INACTIVE;
  Option->OpRomOption.FoundRomType = 0;
  Option->OpRomOption.OpRomPolicy = (SecureOpRonConfig.OptionRomPolicy == OPTION_ROM_POLICY_AUTO) ? SUPPORT_ALL_OPTION_ROM : (SecureOpRonConfig.OptionRomPolicy - 1);
  Option->OpRomOption.VendorId = VendorId;
  Option->OpRomOption.DeviceId = DeviceId;;
  GetNewOpRomOptionNoForSetup (&Option->OptionNumber);
  InsertTailList (&mOpRomPolicyListForSetup, &Option->Link);

  Status = UpdateSecureOpRomDynamicItems (
             HiiHandle,
             &mOpRomPolicyListForSetup
             );

  return Status;
}

/**
  Delete the Option ROM policy from Option ROM policy list.

  @param[in] HiiHandle             - Hii hanlde for the call back routine.
  @param[in] OpRomPolicyIndexedNum - Number of Option ROM list.

  @retval EFI_SUCCESS - Delete the Option ROM policy entry success.
  @retval other       - Delete the Option ROM policy entry failure.

**/
EFI_STATUS
EFIAPI
DeleteOptionRomPolicy (
  IN  EFI_HII_HANDLE       HiiHandle,
  IN  UINT16               OpRomPolicyIndexedNum
  )
{
  OPTION_ROM_LIST_OPTION        *Option;
  EFI_STATUS                    Status;
  OPTION_ROM_POLICY_SETTING     OpRomPolicySetting = {0};
  EFI_FORM_BROWSER2_PROTOCOL    *FormBrowser2;
  CHAR16                        *ConfigRequest;
  UINTN                         ConfigRequestSize;
  UINT8                         MyNameValue0;

  Status = EFI_NOT_FOUND;

  Option = FindOptionRomPolicyByID (mOpRomDeviceList[OpRomPolicyIndexedNum].VendorId, mOpRomDeviceList[OpRomPolicyIndexedNum].DeviceId);
  if (Option != NULL) {
    HiiSetBrowserData (
      &gH2OSecureOptionRomControlFormsetGuid,
      L"OpRomSetting",
      sizeof (OPTION_ROM_POLICY_SETTING),
      (UINT8 *) &OpRomPolicySetting,
      NULL
      );
    MyNameValue0 = (UINT8) (Option->OptionNumber + 1);
    //
    // Remove the selected option ROM policy and update form to HII handle.
    //
    RemoveEntryList (&Option->Link);
    gBS->FreePool (Option);
    Status = UpdateSecureOpRomDynamicItems (
               HiiHandle,
               &mOpRomPolicyListForSetup
               );
    gBS->FreePool (mOpRomDeviceList);

    //
    // Change name value data to make form brower will call RouteConfig () when SubmitForm ()
    //
    Status = gBS->LocateProtocol (&gEfiFormBrowser2ProtocolGuid, NULL, (VOID **) &FormBrowser2);
    if (!EFI_ERROR (Status)) {
      ConfigRequestSize = 0x100;
      ConfigRequest     = AllocateZeroPool (ConfigRequestSize);
      UnicodeSPrint (ConfigRequest, ConfigRequestSize, L"MyNameValue0=%02x", MyNameValue0);
      FormBrowser2->BrowserCallback (
                      FormBrowser2,
                      &ConfigRequestSize,
                      ConfigRequest,
                      FALSE,
                      &gH2OSecureOptionRomControlFormsetGuid,
                      L"MyNameValue0"
                      );
      FreePool (ConfigRequest);
    }
  }
  return Status;
}

/**
  Set variables "OpRomXXXX" and "OpRomOrder" to describe the each OpRom entry.

  @param  VOID

  @retval EFI_SUCCESS            Function has completed successfully.
  @return Other                  Error occurred during execution.

**/
EFI_STATUS
SetOpRomPolicies (
  VOID
  )
{
  UINT16                        OptionNum;
  UINT16                        Index;
  UINT16                        *OptionOrder;
  EFI_LIST_ENTRY                *Link;
  OPTION_ROM_LIST_OPTION        *Option;
  CHAR16                        OptionName[20];
  EFI_STATUS                    Status;
  EFI_LIST_ENTRY                *OptionRomPolicyList;
  UINTN                         OptionOrderSize;
  UINT16                        *OpRomOrder;

  if (mOpRomPolicyListForSetup.ForwardLink == NULL) {
    //
    // No Option ROM policy can save.
    //
    return EFI_SUCCESS;
  }

  OptionRomPolicyList = &mOpRomPolicyListForSetup;
  OptionNum = 0;
  for (Link = OptionRomPolicyList->ForwardLink; Link != OptionRomPolicyList; Link = Link->ForwardLink) {
    Option = CR (Link, OPTION_ROM_LIST_OPTION, Link, OPTION_ROM_LIST_OPTION_SIGNATURE);
    //
    // Set inactive when exiting SCU, the variables will be re-initialize in next boot.
    //
    Option->OpRomOption.Attribute = OP_ROM_INACTIVE;
    UnicodeSPrint (OptionName, sizeof (OptionName), L"OpRom%04x", Option->OptionNumber);
    Status = SetVariableToSensitiveVariable (
               OptionName,
               &gH2OSecureOptionRomControlFormsetGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
               sizeof (OPTION_ROM_OPTION),
               &Option->OpRomOption
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "SetVariable OpRom%04x: %r\n", Option->OptionNumber, Status));
    } else {
      //
      // Todo:
      //   Add code to set this variable in Sensitive variable through SMI.
      //   Need the interface provided by later kernel version.


    }
    OptionNum++;
  }
  OptionOrder = NULL;
  OptionOrder = AllocateZeroPool (OptionNum * sizeof (UINT16));
  Index = 0;
  for (Link = OptionRomPolicyList->ForwardLink; Link != OptionRomPolicyList; Link = Link->ForwardLink) {
    Option = CR (Link, OPTION_ROM_LIST_OPTION, Link, OPTION_ROM_LIST_OPTION_SIGNATURE);
    OptionOrder[Index++] = Option->OptionNumber;
  }
  Status = SetVariableToSensitiveVariable (
             L"OpRomOrder",
             &gH2OSecureOptionRomControlFormsetGuid,
             EFI_VARIABLE_NON_VOLATILE  | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
             OptionNum * sizeof (UINT16),
             OptionOrder
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "SetVariable OpRomOrder: %r\n", Status));
  } else {
    //
    // Todo:
    //   Add code to set this variable in Sensitive variable through SMI.
    //   Need the interface provided by later kernel version.

    
    DEBUG_CODE (
      Status = CommonGetVariableDataAndSize(
                 L"OpRomOrder",
                 &gH2OSecureOptionRomControlFormsetGuid,
                 &OptionOrderSize,
                 (VOID **) &OpRomOrder
                 );
      if (EFI_ERROR (Status) || (OpRomOrder == NULL)) {
        DEBUG ((EFI_D_ERROR, "GetVariable OpRomOrder: %r.\n", Status));
      } else {
        for (Index = 0; Index < (OptionOrderSize / sizeof(UINT16)); Index++) {
          DEBUG ((EFI_D_ERROR, "OpRomOrder[%d] = %d\n", Index, OpRomOrder[Index]));
        }
      }
    );
  }

  if (OptionOrder != NULL) {
    gBS->FreePool (OptionOrder);
  }
  return EFI_SUCCESS;
}

/**
  Callback function for inserting the Option ROM policy.

  @param[in] HiiHandle  - Hii hanlde for the call back routine.
  @param[in] *H2ODialog - Protocol interface of H2O Dialog protocol.

  @retval EFI_SUCCESS   - Insert the Option ROM policy entry success.
  @retval other         - Insert the Option ROM policy entry failure.

**/
EFI_STATUS
EFIAPI
InsertOptionRomPolicyCallback (
  IN  EFI_HII_HANDLE           HiiHandle,
  IN  H2O_DIALOG_PROTOCOL      *H2ODialog
  )
{
  EFI_STATUS                            Status;
  UINT64                                VendorID;
  UINT64                                DeviceID;

  Status = H2ODialog->NumericDialog (
                        L"Set Vendor ID (Hex)",
                        0,
                        0xFFFE,
                        1,
                        TRUE,
                        &VendorID
                        );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (VendorID == 0) {
    return Status;
  }

  Status = H2ODialog->NumericDialog (
                        L"Set Device ID (Hex)",
                        0,
                        0xFFFF,
                        1,
                        TRUE,
                        &DeviceID
                        );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = InsertOptionRomPolicy (HiiHandle, (UINT16)VendorID, (UINT16)DeviceID);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Insert Option ROM policy: %r.\n", Status));
  }

  return Status;
}

/**
  Callback function for deleting the Option ROM policy.

  @param[in] HiiHandle  - Hii hanlde for the call back routine.
  @param[in] *H2ODialog - Protocol interface of H2O Dialog protocol.

  @retval EFI_SUCCESS - Delete the Option ROM policy entry success.
  @retval other       - Delete the Option ROM policy entry failure.

**/
EFI_STATUS
EFIAPI
DeleteOptionRomPolicyCallback (
  IN  EFI_HII_HANDLE           HiiHandle,
  IN  H2O_DIALOG_PROTOCOL      *H2ODialog
  )
{
  EFI_STATUS                            Status = EFI_SUCCESS;
  UINT32                                SelectedIndex;
  CHAR16                                **OpRomNameList;
  UINT32                                OpRomNum;
  CHAR16                                *StringPtr;
  CHAR16                                StringBuffer[MAX_STRING_BUFFER_NUM];
  EFI_INPUT_KEY                         Key;

  OpRomNum = 0;
  OpRomNameList = GetOpRomNameList (&OpRomNum);
  if (OpRomNum == 0) {
    UnicodeSPrint (StringBuffer , MAX_STRING_BUFFER_NUM, L"There is no OpROM option can delete!");
    H2ODialog->ConfirmDialog (
                 DlgOk,
                 FALSE,
                 0,
                 NULL,
                 &Key,
                 StringBuffer
                 );
    return EFI_SUCCESS;
  }

  StringPtr = HiiGetString (
                HiiHandle,
                STRING_TOKEN (STR_DELETE_OPROM_POLICY_DIAG),
                NULL
                );
  H2ODialog->OneOfOptionDialog(
               (UINT32)OpRomNum, // OpRom Number
               FALSE,
               NULL,
               &Key,
               MAX_OPROM_NAME_ID_NUM,
               StringPtr,
               &SelectedIndex,
               (CHAR16 **) (OpRomNameList),
               0
               );
  gBS->FreePool (StringPtr);
  if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
    StringPtr = AllocateZeroPool (MAX_STRING_BUFFER_NUM);
    //
    // User select the device to delete.
    // Popup a dialogue to check again.
    //
    UnicodeSPrint (StringBuffer , MAX_STRING_BUFFER_NUM, L"Do you want to delete \"%s\"", OpRomNameList[SelectedIndex]);
    H2ODialog->ConfirmDialog (
                 DlgYesNo,
                 FALSE,
                 0,
                 NULL,
                 &Key,
                 StringBuffer
                 );
    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      Status = DeleteOptionRomPolicy (HiiHandle, (UINT16)SelectedIndex);
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "Delete Option ROM policy: %r.\n", Status));
      }
    }
  }

  gBS->FreePool (OpRomNameList);
  return Status;
}

/**
  Remove OpRom Entries from OpRomPolicyList in Setup Utility configuration.

  @param  VOID

  @retval EFI_SUCCESS            Function has completed successfully.
  @return Other                  Error occurred during execution.

**/
EFI_STATUS
FreeOptionRomPolicyList (
  VOID
  )
{
  EFI_LIST_ENTRY             *OptionRomPolicyList;
  EFI_LIST_ENTRY             *Link;
  OPTION_ROM_LIST_OPTION     *Option;

  if (mOpRomPolicyListForSetup.ForwardLink == NULL) {
    return EFI_SUCCESS;
  }

  OptionRomPolicyList = &mOpRomPolicyListForSetup;
  while (!IsListEmpty (OptionRomPolicyList)) {
    Link = OptionRomPolicyList->ForwardLink;
    Option = CR (Link, OPTION_ROM_LIST_OPTION, Link, OPTION_ROM_LIST_OPTION_SIGNATURE);
    RemoveEntryList (Link);
    gBS->FreePool (Option);
  }
  return EFI_SUCCESS;
}

/**
  Clear the content OpRomPolicyList in Setup Utility configuration.

  @param[in]  VOID

  @retval EFI_SUCCESS            Function has completed successfully.
  @return Other                  Error occurred during execution.

**/
EFI_STATUS
LoadOpRomPolicyListToDefault (
  IN EFI_HII_HANDLE       HiiHandle
  )
{
  EFI_STATUS                          Status;
  SECURE_OPROM_CONTROL_CONFIGURATION  SecureOpRonConfig = {0};
  BOOLEAN                             BoolStatus;

  Status = FreeOptionRomPolicyList ();
  if (!EFI_ERROR (Status)) {
    UpdateSecureOpRomDynamicItems (HiiHandle, &mOpRomPolicyListForSetup);
    SecureOpRonConfig.OptionRomPolicy = OPTION_ROM_POLICY_AUTO;
    BoolStatus = HiiSetBrowserData (
                   &gH2OSecureOptionRomControlFormsetGuid,
                   mVariableName,
                   sizeof (SECURE_OPROM_CONTROL_CONFIGURATION),
                   (UINT8 *) &SecureOpRonConfig,
                   NULL
                   );
    DEBUG ((EFI_D_ERROR, "HiiSetBrowserData: %d \n", BoolStatus));
  }

  return Status;
}

/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Request                A null-terminated Unicode string in
                                 <ConfigRequest> format.
  @param  Progress               On return, points to a character in the Request
                                 string. Points to the string's null terminator if
                                 request was successful. Points to the most recent
                                 '&' before the first failing name/value pair (or
                                 the beginning of the string if the failure is in
                                 the first name/value pair) if the request was not
                                 successful.
  @param  Results                A null-terminated Unicode string in
                                 <ConfigAltResp> format which has all values filled
                                 in for the names in the Request string. String to
                                 be allocated by the called function.

  @retval EFI_SUCCESS            The Results is filled with the requested values.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.

**/
EFI_STATUS
EFIAPI
ExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                       Status;
  UINTN                            BufferSize;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;
  EFI_STRING                       ConfigRequest;
  EFI_STRING                       ConfigRequestHdr;
  UINTN                            Size;
  BOOLEAN                          AllocatedRequest;
  EFI_STRING                       Value;
  UINTN                            ValueStrLen;
  UINT8                            MyNameValue0;
  CHAR16                           BackupChar;
  
  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize the local variables.
  //
  ConfigRequestHdr  = NULL;
  ConfigRequest     = NULL;
  Size              = 0;
  *Progress         = Request;
  AllocatedRequest  = FALSE;

  HiiConfigRouting = mPrivateData->HiiConfigRouting;

  //
  // Get Buffer Storage data from EFI variable.
  // Try to get the current setting from variable.
  //
  BufferSize = sizeof (SECURE_OPROM_CONTROL_CONFIGURATION);
  Status = gRT->GetVariable (
                  mVariableName,
                  &gH2OSecureOptionRomControlFormsetGuid,
                  NULL,
                  &BufferSize,
                  &mPrivateData->Configuration
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  if (Request == NULL) {
    //
    // Request is set to NULL, construct full request string.
    //

    //
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    ConfigRequestHdr = HiiConstructConfigHdr (&gH2OSecureOptionRomControlFormsetGuid, mVariableName, mPrivateData->DriverHandle);
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    ASSERT (ConfigRequest != NULL);
    AllocatedRequest = TRUE;
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64)BufferSize);
    FreePool (ConfigRequestHdr);
    ConfigRequestHdr = NULL;
  } else {
    if (!HiiIsConfigHdrMatch (Request, &gH2OSecureOptionRomControlFormsetGuid, mVariableName)) {
      return EFI_NOT_FOUND;
    }
    //
    // Set Request to the unified request string.
    //
    ConfigRequest = Request;
  }

  if (StrStr (ConfigRequest, L"OFFSET") == NULL) {
    //
    // If requesting Name/Value storage, return value 0.
    //
    //
    // Allocate memory for <ConfigResp>, e.g. Name0=0x11, Name1=0x1234, Name2="ABCD"
    // <Request>   ::=<ConfigHdr>&Name0&Name1&Name2
    // <ConfigResp>::=<ConfigHdr>&Name0=11&Name1=1234&Name2=0041004200430044
    //
    BufferSize = (StrLen (ConfigRequest) + 1 + sizeof (MyNameValue0) * 2 + 1) * sizeof (CHAR16);
    *Results = AllocateZeroPool (BufferSize);
    ASSERT (*Results != NULL);
    StrCpy (*Results, ConfigRequest);
    Value = *Results;

    //
    // Append value of NameValueVar0, type is UINT8
    //
    if ((Value = StrStr (*Results, L"MyNameValue0")) != NULL) {
      MyNameValue0 = 0;

      Value += StrLen (L"MyNameValue0");
      ValueStrLen = ((sizeof (MyNameValue0) * 2) + 1);
      CopyMem (Value + ValueStrLen, Value, StrSize (Value));

      BackupChar = Value[ValueStrLen];
      *Value++   = L'=';
      Value += UnicodeValueToString (
                 Value,
                 PREFIX_ZERO | RADIX_HEX,
                 MyNameValue0,
                 sizeof (MyNameValue0) * 2
                 );
      *Value = BackupChar;
    }
    return EFI_SUCCESS;
  }

  Status = HiiConfigRouting->BlockToConfig (
                               HiiConfigRouting,
                               ConfigRequest,
                               (UINT8 *)&mPrivateData->Configuration,
                               1,
                               Results,
                               Progress
                               );
  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    FreePool (ConfigRequest);
  }

  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return EFI_SUCCESS;
}

/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Configuration          A null-terminated Unicode string in <ConfigResp>
                                 format.
  @param  Progress               A pointer to a string filled in with the offset of
                                 the most recent '&' before the first failing
                                 name/value pair (or the beginning of the string if
                                 the failure is in the first name/value pair) or
                                 the terminating NULL if all was successful.

  @retval EFI_SUCCESS            The Results is processed successfully.
  @retval EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.
**/
EFI_STATUS
EFIAPI
RouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  SECURE_OPROM_CONTROL_CONFIGURATION  SecureOpRonConfig = {0};
  EFI_STATUS                          Status;
  EFI_HII_CONFIG_ROUTING_PROTOCOL     *HiiConfigRouting;
  UINTN                               BufferSize;
  
  if (Configuration == NULL || Progress == NULL || This == NULL) {
   return EFI_INVALID_PARAMETER;
  }

  if (!HiiIsConfigHdrMatch(Configuration, &gH2OSecureOptionRomControlFormsetGuid, mVariableName)) {
    return EFI_SUCCESS;
  }

  SetOpRomPolicies ();

  HiiConfigRouting = mPrivateData->HiiConfigRouting;
  BufferSize = sizeof (SECURE_OPROM_CONTROL_CONFIGURATION);
  Status = HiiConfigRouting->ConfigToBlock (
                               HiiConfigRouting,
                               Configuration,
                               (UINT8 *) &SecureOpRonConfig,
                               &BufferSize,
                               Progress
                               );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Store broswer data Buffer Storage to EFI variable
  //
  Status = SetVariableToSensitiveVariable (
             mVariableName,
             &gH2OSecureOptionRomControlFormsetGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
             sizeof (SECURE_OPROM_CONTROL_CONFIGURATION),
             &SecureOpRonConfig
             );
  return Status;

}

/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Action                 Specifies the type of action taken by the browser.
  @param  QuestionId             A unique value which is sent to the original
                                 exporting driver so that it can identify the type
                                 of data to expect.
  @param  Type                   The type of value for the question.
  @param  Value                  A pointer to the data being sent to the original
                                 exporting driver.
  @param  ActionRequest          On return, points to the action requested by the
                                 callback function.

  @retval EFI_SUCCESS            The callback successfully handled the action.
  @retval EFI_OUT_OF_RESOURCES   Not enough storage is available to hold the
                                 variable and its data.
  @retval EFI_DEVICE_ERROR       The variable could not be saved.
  @retval EFI_UNSUPPORTED        The specified Action is not supported by the
                                 callback.

**/
EFI_STATUS
EFIAPI
DriverCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS                                 Status;
  H2O_DIALOG_PROTOCOL                        *H2ODialog;

  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE))||
    (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;

  switch (Action) {
  case EFI_BROWSER_ACTION_FORM_OPEN:
    DEBUG ((EFI_D_ERROR, "EFI_BROWSER_ACTION_FORM_OPEN. \n"));
    if (mSetupInitDone == FALSE) {
      Status = InitSecureOpRomSubMenu (mPrivateData->HiiHandle);  
      return Status;
    } 
    break;

  case EFI_BROWSER_ACTION_CHANGING:
    DEBUG ((EFI_D_ERROR, "EFI_BROWSER_ACTION_CHANGING. \n"));
    {
      Status = gBS->LocateProtocol (
                      &gH2ODialogProtocolGuid,
                      NULL,
                      (VOID **)&H2ODialog
                      );
      if (EFI_ERROR (Status)) {
        return Status;
      }

      switch (QuestionId) {
      case KEY_OPROM_POLICY_INSERT:
        DEBUG ((EFI_D_ERROR, "KEY_OPROM_POLICY_INSERT!!\n"));
        Status = InsertOptionRomPolicyCallback (mPrivateData->HiiHandle, H2ODialog);
        break;

      case KEY_OPROM_POLICY_DELETE:
        DEBUG ((EFI_D_ERROR, "KEY_OPROM_POLICY_DELETE!!\n"));
        Status = DeleteOptionRomPolicyCallback (mPrivateData->HiiHandle, H2ODialog);
        break;

      case KEY_OPROM_POLICY_CONTROL:
        DEBUG ((EFI_D_ERROR, "KEY_OPROM_POLICY_CONTROL!!\n"));
        Status = UpdateSecureOpRomDynamicItems (mPrivateData->HiiHandle, &mOpRomPolicyListForSetup);
        break;

      default:
        if ((QuestionId >= KEY_OPTION_ROM_POLICY_BASE) && (QuestionId < KEY_OPTION_ROM_POLICY_BASE + MAX_OPTION_ROM_POLICY)) {
          Status = UpdateOptionRomPolicy (QuestionId - KEY_OPTION_ROM_POLICY_BASE, Value->u8);
          break;
        }
      }

    }
    break;

  case EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING:
    DEBUG ((EFI_D_ERROR, "EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING. \n"));
    break;

  case EFI_BROWSER_ACTION_DEFAULT_STANDARD:
    DEBUG ((EFI_D_ERROR, "EFI_BROWSER_ACTION_DEFAULT_STANDARD. \n"));
    if (QuestionId == KEY_LOAD_DEFAULT) {
      //
      // Load Option Rom Policy list to default. The action is to delete all of specific
      // option ROM policy.
      //
      LoadOpRomPolicyListToDefault (mPrivateData->HiiHandle);
    } else {
      Status = EFI_UNSUPPORTED;
    }
    break;

  case EFI_BROWSER_ACTION_RETRIEVE:
    DEBUG ((EFI_D_ERROR, "EFI_BROWSER_ACTION_RETRIEVE. \n"));
    if (QuestionId == KEY_LOAD_DEFAULT) {
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_UNSUPPORTED;
    }
    break;

  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  return Status;
}

/**
  Unloads the application and its installed protocol.

  @param[in]  ImageHandle       Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
**/
EFI_STATUS
EFIAPI
SetupUnload (
  IN EFI_HANDLE  ImageHandle
  )
{

  ASSERT (mPrivateData != NULL);

  if (mDriverHandle != NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           mDriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mHiiVendorDevicePath0,
           &gEfiHiiConfigAccessProtocolGuid,
           &mPrivateData->ConfigAccess,
           NULL
           );
    mDriverHandle = NULL;
  }

  if (mPrivateData->HiiHandle != NULL) {
    HiiRemovePackages (mPrivateData->HiiHandle);
  }

  FreePool (mPrivateData);
  mPrivateData = NULL;

  return EFI_SUCCESS;
}

VOID
EFIAPI
SetupUtilityApplicationNotifyFn (
  IN EFI_EVENT                             Event,
  IN VOID                                  *Context
  )
{
  EFI_STATUS                               Status;
  EFI_SETUP_UTILITY_BROWSER_PROTOCOL       *Interface;

  DEBUG ((EFI_D_ERROR, "SetupUtilityApplicationNotifyFn Start.\n"));
  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityApplicationProtocolGuid,
                  NULL,
                  (VOID **) &Interface
                  );
  if (EFI_ERROR(Status)) {
    return;
  }

  DEBUG ((EFI_D_ERROR, "Set mSetupInitDone to FALSE.\n"));
  mSetupInitDone = FALSE;

  DEBUG ((EFI_D_ERROR, "SetupUtilityApplicationNotifyFn End.\n"));
  return;
  
}

/**
  Initialization for the Setup related functions.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to SystemTable.

  @retval EFI_SUCESS     This function always complete successfully.

**/
EFI_STATUS
EFIAPI
SetupInit (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_HII_HANDLE                        HiiHandle;
  EFI_HII_DATABASE_PROTOCOL             *HiiDatabase;
  EFI_HII_STRING_PROTOCOL               *HiiString;
  EFI_HII_CONFIG_ROUTING_PROTOCOL       *HiiConfigRouting;
  CHAR16                                *NewString;
  UINTN                                 BufferSize;
  SECURE_OPROM_CONTROL_CONFIGURATION    *Configuration;
  BOOLEAN                               ActionFlag;
  EFI_STRING                            ConfigRequestHdr;
  EFI_EVENT                             Event;
  VOID                                  *Registration;

  //
  // Initialize the local variables.
  //
  ConfigRequestHdr = NULL;
  NewString        = NULL;

  //
  // When installing Setup Utility Browser, set mSetupInitDone to FALSE to initialize the vfr.
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK - 1,
                  SetupUtilityApplicationNotifyFn,
                  NULL,
                  &Event
                  );
  if (!EFI_ERROR (Status)) {
    Status = gBS->RegisterProtocolNotify (
                    &gEfiSetupUtilityApplicationProtocolGuid,
                    Event,
                    &Registration
                    );
  }

  //
  // Initialize driver private data
  //
  mPrivateData = AllocateZeroPool (sizeof (SECURE_OPROM_CONTROL_PRIVATE_DATA));
  if (mPrivateData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->Signature = SECURE_OPROM_CONTROL_PRIVATE_SIGNATURE;

  mPrivateData->ConfigAccess.ExtractConfig = ExtractConfig;
  mPrivateData->ConfigAccess.RouteConfig = RouteConfig;
  mPrivateData->ConfigAccess.Callback = DriverCallback;

  //
  // Locate Hii Database protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiDatabase = HiiDatabase;

  //
  // Locate HiiString protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiStringProtocolGuid, NULL, (VOID **) &HiiString);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiString = HiiString;

  //
  // Locate ConfigRouting protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &HiiConfigRouting);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiConfigRouting = HiiConfigRouting;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mDriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath0,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mPrivateData->ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  mPrivateData->DriverHandle = mDriverHandle;

  //
  // Publish our HII data
  //
  HiiHandle = HiiAddPackages (
                &gH2OSecureOptionRomControlFormsetGuid,
                mDriverHandle,
                SecureOptionRomControlDxeStrings,
                SecureOptionRomControlSetupBin,
                NULL
                );
  if (HiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->HiiHandle = HiiHandle;

  //
  // Initialize configuration data
  //
  Configuration = &mPrivateData->Configuration;
  ZeroMem (Configuration, sizeof (SECURE_OPROM_CONTROL_CONFIGURATION));

  //
  // Try to read NV config EFI variable first
  //
  ConfigRequestHdr = HiiConstructConfigHdr (&gH2OSecureOptionRomControlFormsetGuid, mVariableName, mDriverHandle);
  ASSERT (ConfigRequestHdr != NULL);
  if (ConfigRequestHdr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  DEBUG ((EFI_D_ERROR, "ConfigRequestHdr: %s\n", ConfigRequestHdr));

  BufferSize = sizeof (SECURE_OPROM_CONTROL_CONFIGURATION);
  Status = gRT->GetVariable (mVariableName, &gH2OSecureOptionRomControlFormsetGuid, NULL, &BufferSize, Configuration);
  if (EFI_ERROR (Status)) {
    //
    // Store zero data Buffer Storage to EFI variable
    //
    Status = SetVariableToSensitiveVariable (
               mVariableName,
               &gH2OSecureOptionRomControlFormsetGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
               sizeof (SECURE_OPROM_CONTROL_CONFIGURATION),
               Configuration
               );
    if (EFI_ERROR (Status)) {
      SetupUnload (ImageHandle);
      return Status;
    }
    //
    // EFI variable for NV config doesn't exit, we should build this variable
    // based on default values stored in IFR
    //

    ActionFlag = HiiSetToDefaults (ConfigRequestHdr, EFI_HII_DEFAULT_CLASS_STANDARD);
    if (!ActionFlag) {
      SetupUnload (ImageHandle);
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // EFI variable does exist and Validate Current Setting
    //

    ActionFlag = HiiValidateSettings (ConfigRequestHdr);
    if (!ActionFlag) {
      SetupUnload (ImageHandle);
      return EFI_INVALID_PARAMETER;
    }
  }
  FreePool (ConfigRequestHdr);

  return EFI_SUCCESS;
}
