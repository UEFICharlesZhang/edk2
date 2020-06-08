/** @file

  H2O Setup Change DXE implementation.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <H2OSetupChange.h>

H2O_SUBMIT_SVC_PROTOCOL                gH2OSubmitSvcProtocol = {
                                         ExecuteSubmitSvc
                                         };

LIST_ENTRY                             mQuestionChangedList = INITIALIZE_LIST_HEAD_VARIABLE (mQuestionChangedList);

BOOLEAN                                mShowDone = FALSE;

CHAR16                                    *mFullDiagTitleString = L"History is full, please set action:";
CHAR16                                    *mFullDiagActionList[SETUP_CHANGE_MAX_OPTION] = {
                                              {L"Stop the history"},
                                              {L"Clear all history"},
                                              {L"Overwrite older history"}
                                              };

/**
 This function verifies the leap year.

 @param[in]         Year                Year in YYYY format.

 @retval TRUE                           The year is a leap year.
 @retval FALSE                          The year is not a leap year.
*/
BOOLEAN
IsLeapYear (
  IN  UINT16                            Year
  )
{
  if (Year % 4 == 0) {
    if (Year % 100 == 0) {
      if (Year % 400 == 0) {
        return TRUE;
      } else {
        return FALSE;
      }
    } else {
      return TRUE;
    }
  } else {
    return FALSE;
  }
}

/**
 Converts EFI_TIME structure to a TimeStamp.

 @param[in]         Time                EFI_TIME structure to be converted.
 @param[out]        TimeStamp           TimeStamp converted from EFI_TIME structure.
*/
VOID
EfiTimeToTimeStamp (
  IN  EFI_TIME                          *Time,
  OUT UINT32                            *TimeStamp
  )
{
  UINT16                                Year;
  UINT16                                AddedDays;
  UINT8                                 Month;
  UINT32                                DaysOfMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  //
  // Find number of leap years
  //
  AddedDays = 0;
  for (Year = BASE_YEAR; Year < Time->Year; ++Year) {
    if (IsLeapYear (Year)) {
      ++AddedDays;
    }
  }

  //
  // Number of days of complete years (include all leap years)
  //
  *TimeStamp = (Time->Year - BASE_YEAR) * DAYS_PER_YEAR;
  *TimeStamp += AddedDays;

  //
  // Number of days from 1970/1/1 to now
  //
  for (Month = 0; Month < Time->Month - BASE_MONTH; ++Month) {
    *TimeStamp += DaysOfMonth[Month];
  }
  *TimeStamp += Time->Day - BASE_DAY;

  //
  // Check this Feb. is 28 days or 29 days
  //
  if (IsLeapYear (Time->Year) && Time->Month > 2) {
    *TimeStamp += 1;
  }

  //
  // Convert days to seconds
  //
  *TimeStamp *= SECONDS_PER_DAY;

  //
  // Add rest seconds
  //
  *TimeStamp += (Time->Hour * SECONDS_PER_HOUR) +
                (Time->Minute * SECONDS_PER_MINUTE) +
                Time->Second;

}

/**
 Store all of the showing string to specific storage or EFI Variable.

 @param[out]  This               A pointer to the showing string.

 @retval EFI_SUCCESS             The showing string has been stored.
 @retval EFI_Status              Otherwise.
*/
EFI_STATUS
EFIAPI
StoreInformation (
  IN CHAR16             *StringBuf
  )
{
  EFI_STATUS                                Status;
  CHAR16                                    *VariableName = H2O_SETUP_CHANGE_VARIABLE_NAME;
  UINTN                                     OrgVariableDataSize;
  UINT8                                     *OrgVariableData = NULL;
  VOID                                      *SetupChangeVarPool = NULL;
  H2O_SETUP_CHANGE_VARIABLE                 *SetupChangeVar;
  H2O_SETUP_CHANGE_VARIABLE                 *OrgSetupChangeVar;
  UINTN                                     SetupChangeVarDataSize = 0;
  UINT32                                    TimeStamp;
  EFI_TIME                                  Time;
  CHAR16                                    *NewUnicodeStr;
  UINTN                                     NewUnicodeStrSize;
  UINT8                                     AdjustPolicy;
  H2O_DIALOG_PROTOCOL                       *H2ODialog;
  UINT32                                    SelectedIndex;
  EFI_INPUT_KEY                             Key;
  UINTN                                     NeedVariableSize;
  UINTN                                     CountVariableSize = 0;
  UINT8                                     *CopyAddr;

  //
  // Calculate new data size and check if it is over the max Setup Change variable size.
  //
  NewUnicodeStrSize = StrLen (StringBuf) * 2;
  NewUnicodeStr = StringBuf;
  NeedVariableSize = sizeof(SetupChangeVar->TimeStamp) + sizeof(SetupChangeVar->Size) + NewUnicodeStrSize;
  if (NeedVariableSize > PcdGet32(PcdMaxSetupChangeVariableSize)) {
    DEBUG ((EFI_D_ERROR, "Needed Setup Change information size is over the max Setup Change Variable size.\n"));
    DEBUG ((EFI_D_ERROR, "Needed Setup Change information size: 0x%x\n", NeedVariableSize));
    DEBUG ((EFI_D_ERROR, "PcdMaxSetupChangeVariableSize: 0x%x\n", PcdGet32(PcdMaxSetupChangeVariableSize)));
    return EFI_OUT_OF_RESOURCES;
  }

  OrgVariableDataSize = 0;
  Status = CommonGetVariableDataAndSize (
             VariableName,
             &gH2OSetupChangeVariableGuid,
             &OrgVariableDataSize,
             &OrgVariableData
             );
  if (!EFI_ERROR (Status)) {
    //
    // Find old variable data, and update it with new data..
    //

    //
    // Calculate the total size, and check if it exceeds the maximum size.
    //
    SetupChangeVarDataSize = NeedVariableSize + OrgVariableDataSize;
    if (SetupChangeVarDataSize >= PcdGet32(PcdMaxSetupChangeVariableSize)) {
      DEBUG ((EFI_D_INFO, "Total Setup Change information size is over the max Setup Change Variable size..\n"));
      //
      // Get option for the policy: Overwrite, Clear, Do nothing.
      //
      Status = gBS->LocateProtocol (&gH2ODialogProtocolGuid, NULL, (VOID **) &H2ODialog);
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_INFO, "Locate H2ODialog Protocol: %r \n", Status));
        FreePool (OrgVariableData);
        return Status;
      }
      H2ODialog->OneOfOptionDialog (
                   (UINT32)SETUP_CHANGE_MAX_OPTION,
                   FALSE,
                   NULL,
                   &Key,
                   SETUP_CHANGE_MAX_OPTION_STRING_SIZE,
                   mFullDiagTitleString,
                   &SelectedIndex,
                   (CHAR16 **) (mFullDiagActionList),
                   0
                   );
      if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
        AdjustPolicy = (UINT8)SelectedIndex;

        switch (AdjustPolicy) {
        case SETUP_CHANGE_DISPLAY_DO_NOTHING:
          FreePool (OrgVariableData);
          return EFI_SUCCESS;
          break;

        case SETUP_CHANGE_DISPLAY_CLEAR:
          //
          // Clear all old data, and only will store the new data.
          //
          SetupChangeVarDataSize -= OrgVariableDataSize;
          OrgVariableDataSize = 0;
          break;

        case SETUP_CHANGE_DISPLAY_OVERWRITE:
          NeedVariableSize = sizeof(SetupChangeVar->TimeStamp) + sizeof(SetupChangeVar->Size) + NewUnicodeStrSize;
          CountVariableSize = 0;
          OrgSetupChangeVar = (H2O_SETUP_CHANGE_VARIABLE *) OrgVariableData;
          while (CountVariableSize < OrgVariableDataSize) {
            if (OrgSetupChangeVar->Size == 0) {
              //
              // Variable may be destroyed, skip all old data and break.
              //
              CountVariableSize = 0;
              break;
            }
            CountVariableSize += OrgSetupChangeVar->Size;
            //
            // Count the suitable size to get enough space to store new data.
            // The oldest data will be skipped.
            //
            if ((PcdGet32(PcdMaxSetupChangeVariableSize) - CountVariableSize) < NeedVariableSize) {
              CountVariableSize -= OrgSetupChangeVar->Size;
              break;
            }
            OrgSetupChangeVar = (H2O_SETUP_CHANGE_VARIABLE *) (OrgVariableData + CountVariableSize);
          }

          //
          // Final data content: (Need data) + (part of old data base on OrgVariableDataSize)
          //
          SetupChangeVarDataSize = NeedVariableSize + CountVariableSize;
          OrgVariableDataSize = CountVariableSize;
          break;

        default:
          break;
        }
      } else {
        //
        // User doesn't select any action for over size, don't do anything and returen.
        //
        DEBUG ((EFI_D_INFO, "Total size is over the max Setup Change variable size, and keep the old data content\n"));
        FreePool (OrgVariableData);
        return EFI_SUCCESS;
      }
    }
  } else {
    //
    // Can't find old data from Setup Change Variable, just create new one with new data.
    //
    SetupChangeVarDataSize = NeedVariableSize;
  }

  if (SetupChangeVarDataSize == 0) {
    if (OrgVariableData != NULL) {
      FreePool (OrgVariableData);
    }
    return EFI_SUCCESS;
  }

  SetupChangeVarPool = AllocateZeroPool (SetupChangeVarDataSize);
  if (SetupChangeVarPool == NULL) {
    DEBUG ((EFI_D_INFO, "AllocateZeroPool(): %r \n", Status));
    if (OrgVariableData != NULL) {
      FreePool (OrgVariableData);
    }
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gRT->GetTime (&Time, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "EfiGetTime(): %r \n", Status));
    if (OrgVariableData != NULL) {
      FreePool (OrgVariableData);
    }
    FreePool (SetupChangeVarPool);
    return EFI_OUT_OF_RESOURCES;
  } else {
    EfiTimeToTimeStamp (&Time, &TimeStamp);
    DEBUG ((EFI_D_INFO, "TimeStamp: %x \n", TimeStamp));
  }

  SetupChangeVar = (H2O_SETUP_CHANGE_VARIABLE *)SetupChangeVarPool;
  SetupChangeVar->TimeStamp = TimeStamp;
  //
  // Size value will base on the newest data, because each copy of data will have its own size value.
  //
  SetupChangeVar->Size = (UINT16) (sizeof(SetupChangeVar->TimeStamp) +
                                   sizeof(SetupChangeVar->Size) +
                                   NewUnicodeStrSize
                                   );
  CopyMem (&SetupChangeVar->Data, NewUnicodeStr, NewUnicodeStrSize);
  if (OrgVariableDataSize != 0) {
    CopyAddr = (UINT8*) &(SetupChangeVar->Data);
    CopyAddr += NewUnicodeStrSize;
    CopyMem (CopyAddr, OrgVariableData, OrgVariableDataSize);
    FreePool (OrgVariableData);
  }

  Status = CommonSetVariable (
             VariableName,
             &gH2OSetupChangeVariableGuid,
             EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
             SetupChangeVarDataSize,
             SetupChangeVar
             );
  DEBUG ((EFI_D_INFO, "CommonSetVariable() Setup Change Variable: %r \n", Status));
  FreePool (SetupChangeVarPool);

  return Status;

}

/**
 Contruct the showing string for the specified question data.

 @param[in]   This               A pointer to the specified question data.
 @param[out]  This               A pointer to the showing string.

 @retval EFI_SUCCESS             The showing string has been contructed.
 @retval EFI_Status              Otherwise.
*/
EFI_STATUS
EFIAPI
ContructString (
  IN  H2O_FORM_BROWSER_Q *QuestionData,
  OUT CHAR16             **StringBuf
  )
{
  EFI_STATUS                                Status;
  H2O_FORM_BROWSER_PROTOCOL                 *FBProtocol;
  UINT32                                    VarStoreIdIndex;
  UINTN                                     VarStoreCount;
  EFI_VARSTORE_ID                           *VarStoreIdBuffer;
  H2O_FORM_BROWSER_VS                       *VarbleStoreData;
  CHAR16                                    OrgSettingStr[MAX_SINGLE_CHANGED_STATEMENT_STRING_SIZE];
  CHAR16                                    ModSettingStr[MAX_SINGLE_CHANGED_STATEMENT_STRING_SIZE];
  CHAR16                                    TempSettingStr[MAX_SINGLE_SETTING_STRING_SIZE];
  UINT32                                    OptionIndex;
  UINT32                                    OptionIndex2;
  UINT8                                     Data8;
  UINT16                                    Data16;
  UINT32                                    Data32;
  UINT64                                    Data64;
  UINT8                                     Width;

  Status = gBS->LocateProtocol (
                  &gH2OFormBrowserProtocolGuid,
                  NULL,
                  (VOID **)&FBProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Locate H2OFormBrowser Protocol :%r \n", Status));
    return Status;
  }

  VarbleStoreData = NULL;
  Status = FBProtocol->GetVSAll (FBProtocol, QuestionData->PageId, &VarStoreCount, &VarStoreIdBuffer);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "GetVSAll() %r \n", Status));
  } else {
    for (VarStoreIdIndex = 0; VarStoreIdIndex < VarStoreCount; VarStoreIdIndex++) {
      if (QuestionData->VarStoreId == VarStoreIdBuffer[VarStoreIdIndex]) {
        Status = FBProtocol->GetVSInfo (FBProtocol, QuestionData->PageId, VarStoreIdBuffer[VarStoreIdIndex], &VarbleStoreData);
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_INFO, "GetVSInfo() %r \n", Status));
          return Status;
        }
        break;
      }
    }
  }

  switch (QuestionData->Operand) {

  case EFI_IFR_ONE_OF_OP:
    for (OptionIndex = 0; OptionIndex < QuestionData->NumberOfOptions; OptionIndex++) {
      if (QuestionData->Options[OptionIndex].HiiValue.Value.u8 == VarbleStoreData->Buffer[QuestionData->VariableOffset]) {
        StrCpy (OrgSettingStr, QuestionData->Options[OptionIndex].Text);
      }

      if (QuestionData->Options[OptionIndex].HiiValue.Value.u8 == VarbleStoreData->EditBuffer[QuestionData->VariableOffset]) {
        StrCpy (ModSettingStr, QuestionData->Options[OptionIndex].Text);
      }
    }
    *StringBuf = CatSPrint (NULL, L"%s: [%s] to [%s]\n", QuestionData->Prompt, OrgSettingStr, ModSettingStr);
    break;

  case EFI_IFR_NUMERIC_OP:
    switch (QuestionData->Flags & EFI_IFR_NUMERIC_SIZE) {
    case EFI_IFR_NUMERIC_SIZE_1:

      if (QuestionData->Flags & EFI_IFR_DISPLAY_UINT_HEX) {
        // For Hex display
        Data8 = (UINT8)VarbleStoreData->Buffer[QuestionData->VariableOffset];
        *StringBuf = CatSPrint (
                       NULL,
                       L"%s: [0x%x] to [0x%x]\n",
                       QuestionData->Prompt,
                       Data8,
                       QuestionData->HiiValue.Value.u8
                       );
      } else {
        Data8 = (UINT8)VarbleStoreData->Buffer[QuestionData->VariableOffset];
        *StringBuf = CatSPrint (
                       NULL,
                       L"%s: [%d] to [%d]\n",
                       QuestionData->Prompt,
                       Data8,
                       QuestionData->HiiValue.Value.u8
                       );
      }

      break;
    case EFI_IFR_NUMERIC_SIZE_2:

      if (QuestionData->Flags & EFI_IFR_DISPLAY_UINT_HEX) {
        // For Hex display
        CopyMem (&Data16, &VarbleStoreData->Buffer[QuestionData->VariableOffset], sizeof (UINT16));
        *StringBuf = CatSPrint (
                       NULL,
                       L"%s: [0x%x] to [0x%x]\n",
                       QuestionData->Prompt,
                       Data16,
                       QuestionData->HiiValue.Value.u16
                       );
      } else {
        CopyMem (&Data16, &VarbleStoreData->Buffer[QuestionData->VariableOffset], sizeof (UINT16));
        *StringBuf = CatSPrint (
                       NULL,
                       L"%s: [%d] to [%d]\n",
                       QuestionData->Prompt,
                       Data16,
                       QuestionData->HiiValue.Value.u16
                       );
      }

      break;
    case EFI_IFR_NUMERIC_SIZE_4:
      if (QuestionData->Flags & EFI_IFR_DISPLAY_UINT_HEX) {
        // For Hex display
        CopyMem (&Data32, &VarbleStoreData->Buffer[QuestionData->VariableOffset], sizeof (UINT32));
        *StringBuf = CatSPrint (
                       NULL,
                       L"%s: [0x%x] to [0x%x]\n",
                       QuestionData->Prompt,
                       Data32,
                       QuestionData->HiiValue.Value.u32
                       );
      } else {
        CopyMem (&Data32, &VarbleStoreData->Buffer[QuestionData->VariableOffset], sizeof (UINT32));
        *StringBuf = CatSPrint (
                       NULL,
                       L"%s: [%d] to [%d]\n",
                       QuestionData->Prompt,
                       Data32,
                       QuestionData->HiiValue.Value.u32
                       );
      }

      break;
    case EFI_IFR_NUMERIC_SIZE_8:

      if (QuestionData->Flags & EFI_IFR_DISPLAY_UINT_HEX) {
        // For Hex display
        CopyMem (&Data64, &VarbleStoreData->Buffer[QuestionData->VariableOffset], sizeof (UINT64));
        *StringBuf = CatSPrint (
                       NULL,
                       L"%s: [0x%x] to [0x%x]\n",
                       QuestionData->Prompt,
                       Data64,
                       QuestionData->HiiValue.Value.u64
                       );
      } else {
        CopyMem (&Data64, &VarbleStoreData->Buffer[QuestionData->VariableOffset], sizeof (UINT64));
        *StringBuf = CatSPrint (
                       NULL,
                       L"%s: [%d] to [%d]\n",
                       QuestionData->Prompt,
                       Data64,
                       QuestionData->HiiValue.Value.u64
                       );
      }

      break;
    default:
      break;
    }
    break;

  case EFI_IFR_ORDERED_LIST_OP:
    for (OptionIndex2 = 0; OptionIndex2 < QuestionData->ContainerCount; OptionIndex2++) {
      Data8 = (UINT8)VarbleStoreData->Buffer[QuestionData->VariableOffset + OptionIndex2*(QuestionData->Options[OptionIndex2].HiiValue.Type + 1)];
    }

    for (OptionIndex2 = 0; OptionIndex2 < QuestionData->NumberOfOptions; OptionIndex2++) {
      Data8 = (UINT8)VarbleStoreData->EditBuffer[QuestionData->VariableOffset + OptionIndex2*(QuestionData->Options[OptionIndex2].HiiValue.Type + 1)];
    }

    // Use Option's value type to determine the width as the behavior of IfrParse.c.
    Width = 1;
    switch (QuestionData->Options[0].HiiValue.Type) {
    case EFI_IFR_TYPE_NUM_SIZE_8:
      Width = 1;
      break;

    case EFI_IFR_TYPE_NUM_SIZE_16:
      Width = 2;
      break;

    case EFI_IFR_TYPE_NUM_SIZE_32:
      Width = 4;
      break;

    case EFI_IFR_TYPE_NUM_SIZE_64:
      Width = 8;
      break;

    default:
      //
      // Invalid type for Ordered List
      //
      break;
    }

    ZeroMem (OrgSettingStr, sizeof(OrgSettingStr));
    ZeroMem (ModSettingStr, sizeof(ModSettingStr));

    for (OptionIndex = 0; OptionIndex < QuestionData->ContainerCount; OptionIndex++) {
      for (OptionIndex2 = 0; OptionIndex2 < QuestionData->NumberOfOptions; OptionIndex2++) {
        if (VarbleStoreData->Buffer[QuestionData->VariableOffset + OptionIndex*Width] == QuestionData->Options[OptionIndex2].HiiValue.Value.u64) {
          UnicodeSPrint (TempSettingStr, sizeof(TempSettingStr), L"<%s> ", QuestionData->Options[OptionIndex2].Text);
          StrCat (OrgSettingStr, TempSettingStr);
        }
      }
    }

    for (OptionIndex = 0; OptionIndex < QuestionData->ContainerCount; OptionIndex++) {
      for (OptionIndex2 = 0; OptionIndex2 < QuestionData->NumberOfOptions; OptionIndex2++) {
        if (VarbleStoreData->EditBuffer[QuestionData->VariableOffset + OptionIndex*Width] == QuestionData->Options[OptionIndex2].HiiValue.Value.u64) {
          UnicodeSPrint (TempSettingStr, sizeof(TempSettingStr), L"<%s> ", QuestionData->Options[OptionIndex2].Text);
          StrCat (ModSettingStr, TempSettingStr);
        }
      }
    }

    *StringBuf = CatSPrint (
                   NULL,
                   L"%s: [%s] to [%s]\n",
                   QuestionData->Prompt,
                   OrgSettingStr,
                   ModSettingStr
                   );

    break;

  case EFI_IFR_CHECKBOX_OP:
    Data8 = (UINT8)VarbleStoreData->Buffer[QuestionData->VariableOffset];
    if (Data8 == 0) {
      UnicodeSPrint (OrgSettingStr, sizeof(OrgSettingStr), L"Disabled");
    } else {
      UnicodeSPrint (OrgSettingStr, sizeof(OrgSettingStr), L"Enabled");
    }

    Data8 = (UINT8)VarbleStoreData->EditBuffer[QuestionData->VariableOffset];
    if (Data8 == 0) {
      UnicodeSPrint (ModSettingStr, sizeof(ModSettingStr), L"Disabled");
    } else {
      UnicodeSPrint (ModSettingStr, sizeof(ModSettingStr), L"Enabled");
    }

    *StringBuf = CatSPrint (
                   NULL,
                   L"%s: [%s] to [%s]\n",
                   QuestionData->Prompt,
                   OrgSettingStr,
                   ModSettingStr
                   );
    break;

  case EFI_IFR_STRING_OP:
    *StringBuf = CatSPrint (
                   NULL,
                   L"%s: [%s] to [%s]\n",
                   QuestionData->Prompt,
                   (CHAR16*)&VarbleStoreData->Buffer[QuestionData->VariableOffset],
                   QuestionData->HiiValue.Buffer
                   );
    break;

  default:
    break;

  }

  if (*StringBuf == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

/**
 Remove the all question nodes from mQuestionChangedList.

 @param[in]  None.

 @retval EFI_SUCCESS         Remove the node successfully.
*/
EFI_STATUS
EFIAPI
RemoveQuestionChangedList (
  VOID
  )
{
  LIST_ENTRY                         *Link;
  FORM_BROWSER_STATEMENT_CHANGED     *QuestionNode;

  Link = GetFirstNode (&mQuestionChangedList);
  while (!IsNull (&mQuestionChangedList, Link)) {
    QuestionNode = FORM_BROWSER_STATEMENT_CHANGED_FROM_LINK (Link);
    Link = GetNextNode (&mQuestionChangedList, Link);

    RemoveEntryList (&QuestionNode->Link);
    FreePool (QuestionNode->ChangedInfoStr);
    FreePool (QuestionNode);
  }

  return EFI_SUCCESS;

}

/**
 Add the question node into mQuestionChangedList.

 @param[in]  QuestionData               A pointer to the specified question.

 @retval EFI_SUCCESS                    Add the question node successfully.
 @retval EFI_OUT_OF_RESOURCES           Add failure when allocating pool.

*/
EFI_STATUS
EFIAPI
AddToChangedList (
  IN  H2O_FORM_BROWSER_Q *QuestionData
  )
{
  FORM_BROWSER_STATEMENT_CHANGED     *Question;
  CHAR16                             *TempStringPtr;

  Question = AllocateZeroPool (sizeof (FORM_BROWSER_STATEMENT_CHANGED));
  if (Question == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Question->Signature = FORM_BROWSER_STATEMENT_CHANGED_SIGNATURE;
  CopyMem (&(Question->Question), QuestionData, sizeof(H2O_FORM_BROWSER_Q));

  ContructString (QuestionData, &TempStringPtr);

  Question->ChangedInfoStr = TempStringPtr;
  InsertTailList (&mQuestionChangedList, &Question->Link);
  return EFI_SUCCESS;
}

/**
 Execute the Submit service function.

 @param[in]  This                A pointer to the H2O_SUBMIT_SVC_PROTOCOL instance.
 @param[out] Request             A pointer to the request from the Submit service functions.
                                 Related definition can refer to "Browser actions" of FormBrowserEx.h.
 @param[out] ShowSubmitDialog    A pointer to the value if needing to show the original submit dialog.

 @retval EFI_SUCCESS             Execute the Submit service functions successfully.
 @retval EFI_Status              Otherwise.
*/
EFI_STATUS
EFIAPI
ExecuteSubmitSvc (
  IN    H2O_SUBMIT_SVC_PROTOCOL             *This,
  OUT   UINT32                              *Request,
  OUT   BOOLEAN                             *ShowSubmitDialog
  )
{
  EFI_STATUS                         Status;
  LIST_ENTRY                         *Link;
  FORM_BROWSER_STATEMENT_CHANGED     *QuestionNode;
  CHAR16                             *TitleString = L"BIOS Setting Changed:";
  CHAR16                             *ConfirmString = L"Save the changes?";
  CHAR16                             *ConfirmString2 = L"Exit the Setup?";
  CHAR16                             *NoChangeString = L"There is no changed item.";
  CHAR16                             *StringPtr;
  H2O_DIALOG_PROTOCOL                *H2ODialog;
  H2O_FORM_BROWSER_PROTOCOL          *FBProtocol;
  UINT32                             ChangedQuestionCount;
  H2O_FORM_BROWSER_Q                 *ChangedQuestionBuffer;
  UINT32                             QuestionIndex;
  UINTN                              ShowStrSize;
  EFI_INPUT_KEY                      Key;

  DEBUG ((EFI_D_INFO, "H2OSetupChange ExecuteSubmitSvc() \n"));

  Status = gBS->LocateProtocol (&gH2ODialogProtocolGuid, NULL, (VOID **) &H2ODialog);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Locate H2ODialog Protocol: %r \n", Status));
    return Status;
  }

  Status = gBS->LocateProtocol (
                  &gH2OFormBrowserProtocolGuid,
                  NULL,
                  (VOID **)&FBProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Locate H2OFormBrowser Protocol :%r \n", Status));
    return Status;
  }

  Status = FBProtocol->GetChangedQuestions (FBProtocol, &ChangedQuestionCount, &ChangedQuestionBuffer);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "FBProtocol->GetChangedQuestions() :%r \n", Status));
    return Status;
  }

  if (ChangedQuestionCount == 0) {
    DEBUG ((EFI_D_INFO, "There is no changed items.\n"));
    Status = H2ODialog->ConfirmPageDialog (
                          DlgYesNo,
                          TitleString,
                          ConfirmString2,
                          NoChangeString,
                          &Key
                          );
    if (Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
      DEBUG ((EFI_D_INFO, "No: \n"));
      *Request = BROWSER_ACTION_NONE;
      *ShowSubmitDialog = FALSE;
    } else {
      DEBUG ((EFI_D_INFO, "YES: \n"));
      *Request = BROWSER_ACTION_SUBMIT | BROWSER_ACTION_RESET;
      *ShowSubmitDialog = FALSE;
    }
  } else {
    for (QuestionIndex = 0; QuestionIndex < ChangedQuestionCount; QuestionIndex++) {
      AddToChangedList (&ChangedQuestionBuffer[QuestionIndex]);
    }

    ShowStrSize = 0;
    Link = GetFirstNode (&mQuestionChangedList);
    while (!IsNull (&mQuestionChangedList, Link)) {
      QuestionNode = FORM_BROWSER_STATEMENT_CHANGED_FROM_LINK (Link);
      ShowStrSize += StrLen(QuestionNode->ChangedInfoStr);

      Link = GetNextNode (&mQuestionChangedList, Link);
    }

    StringPtr = NULL;
    StringPtr = AllocateZeroPool (sizeof (CHAR16) * (ShowStrSize + 1));
    if (StringPtr == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Link = GetFirstNode (&mQuestionChangedList);
    while (!IsNull (&mQuestionChangedList, Link)) {
      QuestionNode = FORM_BROWSER_STATEMENT_CHANGED_FROM_LINK (Link);
      StrCat (StringPtr, QuestionNode->ChangedInfoStr);
      Link = GetNextNode (&mQuestionChangedList, Link);
    }

    Status = H2ODialog->ConfirmPageDialog (
                        DlgYesNo,
                        TitleString,
                        ConfirmString,
                        StringPtr,
                        &Key
                        );
    if (Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
      DEBUG ((EFI_D_INFO, "No: \n"));
      *Request = BROWSER_ACTION_NONE;
      *ShowSubmitDialog = FALSE;
    } else {
      DEBUG ((EFI_D_INFO, "YES: \n"));
      *Request = BROWSER_ACTION_SUBMIT | BROWSER_ACTION_RESET;
      *ShowSubmitDialog = FALSE;
      StoreInformation (StringPtr);
    }

    FreePool (StringPtr);
    FreePool (ChangedQuestionBuffer);
    RemoveQuestionChangedList ();
  }

  return Status;
}

/**
 Entry point of this driver. Install H2O Setup Change protocol into DXE.

 @param[in] ImageHandle       Image handle of this driver.
 @param[in] SystemTable       Global system service table.

 @retval EFI Status
*/
EFI_STATUS
EFIAPI
H2OSetupChangeDxeEntry (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                   Status;

  //
  // Install H2OSetupChange protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gH2OSubmitSvcProtocolGuid,
                  &gH2OSubmitSvcProtocol,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS ;
}
