/** @file
  Functions for H2O display engine driver.

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <H2ODisplayEngineLocalText.h>
#include <LTDESpecificQuestionBehavior.h>
#include <LTDEDialog.h>
#include <LTDEControl.h>

BOOLEAN
IsDateTimeOpCode (
  IN H2O_CONTROL_INFO                         *Control  OPTIONAL,
  IN H2O_FORM_BROWSER_Q                       *Question OPTIONAL
  )
{
  return (BOOLEAN) ((Control  != NULL && (Control->Operand  == EFI_IFR_DATE_OP || Control->Operand  == EFI_IFR_TIME_OP)) ||
                    (Question != NULL && (Question->Operand == EFI_IFR_DATE_OP || Question->Operand == EFI_IFR_TIME_OP)));
}

BOOLEAN
IsQuestionBeingModified (
  IN H2O_CONTROL_INFO                         *Control
  )
{
  return (BOOLEAN) (Control != NULL &&
                    Control->ValueStrInfo.String != NULL &&
                    StrStr (Control->ValueStrInfo.String, L" ") != NULL);
}

EFI_STATUS
RestoreQuestionValueStr (
  IN H2O_CONTROL_INFO                         *Control
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_S                          *Statement;

  Status = mDEPrivate->FBProtocol->GetSInfo (mDEPrivate->FBProtocol, Control->PageId, Control->StatementId, &Statement);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SafeFreePool ((VOID **) &mDEPrivate->MenuSelected->ValueStrInfo.String);
  mDEPrivate->MenuSelected->ValueStrInfo.String = GetQuestionValueStr (Statement);
  SafeFreePool ((VOID **) &Statement);

  if (IsCurrentHighlight (mDEPrivate, Control->PageId, Control->QuestionId, Control->IfrOpCode)) {
    DisplayHighLightControl (FALSE, Control);
  } else {
    DisplayNormalControls (1, Control);
  }

  return EFI_SUCCESS;
}

H2O_DATE_TIME_ITEM
DateTimeOpCodeGetItemValue (
  IN H2O_CONTROL_INFO                         *Control
  )
{
  if (Control == NULL || !IsDateTimeOpCode (Control, NULL)) {
    return UnknownItem;
  }

  //
  // Date display format: L"[12/31/2014]"
  // Time display format: L"[23:59:59]"
  //
  switch (Control->Sequence) {

  case 0:
    return (Control->Operand  == EFI_IFR_DATE_OP) ? MonthItem : HourItem;

  case 1:
    return (Control->Operand  == EFI_IFR_DATE_OP) ? DayItem   : MinuteItem;

  case 2:
    return (Control->Operand  == EFI_IFR_DATE_OP) ? YearItem  : SecondItem;

  default:
    break;
  }

  return UnknownItem;
}

CHAR16 *
DateTimeOpCodeCreateValueStr (
  IN H2O_FORM_BROWSER_Q                       *Question
  )
{
  if (Question == NULL || !IsDateTimeOpCode (NULL, Question)) {
    return NULL;
  }

  if (Question->Operand == EFI_IFR_DATE_OP) {
    return CatSPrint (
             NULL,
             L"%c%02d%c%02d%c%04d%c",
             LEFT_NUMERIC_DELIMITER,
             Question->HiiValue.Value.date.Month,
             DATE_SEPARATOR,
             Question->HiiValue.Value.date.Day,
             DATE_SEPARATOR,
             Question->HiiValue.Value.date.Year,
             RIGHT_NUMERIC_DELIMITER
             );
  } else {
    return CatSPrint (
             NULL,
             L"%c%02d%c%02d%c%02d%c",
             LEFT_NUMERIC_DELIMITER,
             Question->HiiValue.Value.time.Hour,
             TIME_SEPARATOR,
             Question->HiiValue.Value.time.Minute,
             TIME_SEPARATOR,
             Question->HiiValue.Value.time.Second,
             RIGHT_NUMERIC_DELIMITER
             );
  }
}

EFI_STATUS
DateTimeOpCodeDisplayValueStr (
  IN H2O_CONTROL_INFO                         *Control,
  IN RECT                                     *ValueStrField,
  IN UINT32                                   NormalAttribute,
  IN UINT32                                   HighlightAttribute,
  IN BOOLEAN                                  IsHighlight
  )
{
  UINTN                                       ValueStrSize;
  CHAR16                                      *ValueStr;
  CHAR16                                      *StrPtr;
  CHAR16                                      CharStr[2];
  BOOLEAN                                     IsDate;
  UINT32                                      StartX;
  UINT32                                      StartY;
  RECT                                        Field;
  UINTN                                       Width;

  if (Control == NULL || ValueStrField == NULL || !IsDateTimeOpCode (Control, NULL) || Control->ValueStrInfo.String == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  StartX       = ValueStrField->left;
  StartY       = ValueStrField->top;
  IsDate       = (Control->Operand == EFI_IFR_DATE_OP) ? TRUE : FALSE;
  ValueStrSize = StrSize (Control->ValueStrInfo.String);
  if ((IsDate  && ValueStrSize != sizeof (L"[12/31/2014]")) ||
      (!IsDate && ValueStrSize != sizeof (L"[23:59:59]"  ))) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Clean remaining region.
  //
  DEConOutSetAttribute (mDEPrivate, NormalAttribute);
  Width = GetStringDisplayWidth (Control->ValueStrInfo.String);
  SetRect (
    &Field,
    StartX + (INT32) Width,
    StartY,
    StartX + (ValueStrField->right - ValueStrField->left),
    StartY
    );
  ClearField (NormalAttribute, &Field, NULL);

  //
  // Display Date/Time string and highlight current sequence
  //
  if (!IsHighlight) {
    DEConOutSetAttribute (mDEPrivate, NormalAttribute);
    return DisplayString (StartX, StartY, Control->ValueStrInfo.String);
  }

  ValueStr = AllocateCopyPool (ValueStrSize, Control->ValueStrInfo.String);
  if (ValueStr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DEConOutSetAttribute (mDEPrivate, NormalAttribute);
  ZeroMem (CharStr, sizeof(CharStr));

  CharStr[0]  = LEFT_NUMERIC_DELIMITER;
  DisplayString (StartX, StartY, CharStr);
  ValueStr[0] = CHAR_NULL;

  CharStr[0]  = IsDate ? DATE_SEPARATOR : TIME_SEPARATOR;
  DisplayString (StartX + 3, StartY, CharStr);
  DisplayString (StartX + 6, StartY, CharStr);
  ValueStr[3] = CHAR_NULL;
  ValueStr[6] = CHAR_NULL;

  CharStr[0]   = RIGHT_NUMERIC_DELIMITER;
  if (IsDate) {
    DisplayString (StartX + 11, StartY, CharStr);
    ValueStr[11] = CHAR_NULL;
  } else {
    CharStr[0]   = RIGHT_NUMERIC_DELIMITER;
    DisplayString (StartX + 9, StartY, CharStr);
    ValueStr[9] = CHAR_NULL;
  }

  StrPtr = &ValueStr[1];
  DEConOutSetAttribute (mDEPrivate, (Control->Sequence == 0) ? HighlightAttribute : NormalAttribute);
  DisplayString (StartX + 1, StartY, StrPtr);
  StrPtr += StrLen (StrPtr) + 1;

  DEConOutSetAttribute (mDEPrivate, (Control->Sequence == 1) ? HighlightAttribute : NormalAttribute);
  DisplayString (StartX + 4, StartY, StrPtr);
  StrPtr += StrLen (StrPtr) + 1;

  DEConOutSetAttribute (mDEPrivate, (Control->Sequence == 2) ? HighlightAttribute : NormalAttribute);
  DisplayString (StartX + 7, StartY, StrPtr);

  FreePool ((VOID *) ValueStr);
  return EFI_SUCCESS;
}

EFI_STATUS
DateTimeOpCodeGetSequenceValue (
  IN  BOOLEAN                                 IsDate,
  IN  RECT                                    *ValueStrAbsField,
  IN  UINT32                                  StartX,
  IN  UINT32                                  StartY,
  OUT UINT8                                   *Sequence
  )
{
  INT32                                       Offset;

  if (ValueStrAbsField == NULL || Sequence == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!IsPointOnField (ValueStrAbsField, (INT32) StartX, (INT32) StartY)) {
    return EFI_NOT_FOUND;
  }

  Offset = (INT32) StartX - ValueStrAbsField->left;

  if (Offset >= 1 && Offset <= 2) {
    *Sequence = 0;
  } else if (Offset >= 4 && Offset <= 5) {
    *Sequence = 1;
  } else if ((IsDate  && Offset >= 7 && Offset <= 10) ||
             (!IsDate && Offset >= 7 && Offset <= 8)) {
    *Sequence = 2;
  } else {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DateTimeOpCodeGetHiiValueFromValueStr (
  IN  H2O_CONTROL_INFO                        *Control,
  OUT EFI_HII_VALUE                           *HiiValue
  )
{
  CHAR16                                      *ValueStr;
  EFI_HII_VALUE                               StrHiiValue;
  BOOLEAN                                     IsDate;
  EFI_TIME                                    StrEfiTime;

  if (Control == NULL || !IsDateTimeOpCode (Control, NULL) || Control->ValueStrInfo.String == NULL) {
    return EFI_UNSUPPORTED;
  }

  CopyMem (&StrHiiValue, &Control->HiiValue, sizeof (HiiValue));
  ValueStr = AllocateCopyPool (StrSize (Control->ValueStrInfo.String), Control->ValueStrInfo.String);
  if (ValueStr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Date display format: L"[12/31/2014]"
  // Time display format: L"[23:59:59]"
  //
  IsDate = (Control->Operand == EFI_IFR_DATE_OP) ? TRUE : FALSE;
  if (IsDate) {
    ValueStr[3]  = CHAR_NULL;
    ValueStr[6]  = CHAR_NULL;
    ValueStr[11] = CHAR_NULL;
    StrHiiValue.Value.date.Month = (UINT8)  StrDecimalToUint64 (&ValueStr[1]);
    StrHiiValue.Value.date.Day   = (UINT8)  StrDecimalToUint64 (&ValueStr[4]);
    StrHiiValue.Value.date.Year  = (UINT16) StrDecimalToUint64 (&ValueStr[7]);
  } else {
    ValueStr[3] = CHAR_NULL;
    ValueStr[6] = CHAR_NULL;
    ValueStr[9] = CHAR_NULL;
    StrHiiValue.Value.time.Hour   = (UINT8) StrDecimalToUint64 (&ValueStr[1]);
    StrHiiValue.Value.time.Minute = (UINT8) StrDecimalToUint64 (&ValueStr[4]);
    StrHiiValue.Value.time.Second = (UINT8) StrDecimalToUint64 (&ValueStr[7]);
  }

  FreePool ((VOID *) ValueStr);

  TransferHiiValueToEfiTime (&StrHiiValue, &StrEfiTime);
  if ((IsDate  && !IsDayValid (&StrEfiTime)) ||
      (!IsDate && !IsTimeValid (&StrEfiTime))) {
    return EFI_UNSUPPORTED;
  }

  CopyMem (HiiValue, &StrHiiValue, sizeof (StrHiiValue));

  return EFI_SUCCESS;
}

EFI_STATUS
DateTimeOpCodeInputOnTheFly (
  IN H2O_CONTROL_INFO                         *Control,
  IN EFI_INPUT_KEY                            *Key
  )
{
  BOOLEAN                                     IsInputingOnTheFly;
  CHAR16                                      *StrPtr;
  EFI_STATUS                                  Status;
  EFI_HII_VALUE                               HiiValue;

  if (Control == NULL || Key == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  IsInputingOnTheFly = IsQuestionBeingModified (Control);

  if (!IsDecChar (Key->UnicodeChar)) {
    return EFI_UNSUPPORTED;
  }

  if (IsInputingOnTheFly) {
    StrPtr = StrStr (Control->ValueStrInfo.String, L" ");
    if (StrPtr == NULL) {
      return EFI_ABORTED;
    }
    *StrPtr = Key->UnicodeChar;

    if (IsQuestionBeingModified (Control)) {
      if (IsCurrentHighlight (mDEPrivate, Control->PageId, Control->QuestionId, Control->IfrOpCode)) {
        DisplayHighLightControl (FALSE, Control);
      } else {
        DisplayNormalControls (1, Control);
      }
    } else {
      Status = DateTimeOpCodeGetHiiValueFromValueStr (Control, &HiiValue);
      if (!EFI_ERROR (Status)) {
        SendChangeQNotify (Control->PageId, Control->QuestionId, &HiiValue);
      } else {
        RestoreQuestionValueStr (Control);
      }
    }
  } else {
    //
    // Date display format: L"[12/31/2014]"
    //
    switch (Control->Sequence) {

    case 0:
      CopyMem (&Control->ValueStrInfo.String[1], L"  ", sizeof (L"  ") - sizeof (CHAR16));
      break;
    case 1:
      CopyMem (&Control->ValueStrInfo.String[4], L"  ", sizeof (L"  ") - sizeof (CHAR16));
      break;
    case 2:
      if (Control->Operand == EFI_IFR_DATE_OP) {
        CopyMem (&Control->ValueStrInfo.String[7], L"    ", sizeof (L"    ") - sizeof (CHAR16));
      } else {
        CopyMem (&Control->ValueStrInfo.String[7], L"  "  , sizeof (L"  ") - sizeof (CHAR16));
      }
      break;
    default:
      ASSERT(FALSE);
      return EFI_ABORTED;
    }

    StrPtr = StrStr (Control->ValueStrInfo.String, L" ");
    if (StrPtr == NULL) {
      return EFI_ABORTED;
    }
    *StrPtr = Key->UnicodeChar;

    if (IsCurrentHighlight (mDEPrivate, Control->PageId, Control->QuestionId, Control->IfrOpCode)) {
      DisplayHighLightControl (FALSE, Control);
    } else {
      DisplayNormalControls (1, Control);
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DateTimeOpCodeProcessInputEvt (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  )
{
  EFI_STATUS                                  Status;
  EFI_INPUT_KEY                               *Key;
  EFI_HII_VALUE                               HiiValue;
  EFI_TIME                                    EfiTime;
  RECT                                        ControlAbsField;
  UINT32                                      Column;
  UINT32                                      Row;
  UINT32                                      BorderLineWidth;
  UINT8                                       Sequence;
  BOOLEAN                                     IsInputingOnTheFly;

  Status             = EFI_UNSUPPORTED;
  IsInputingOnTheFly = IsQuestionBeingModified (Private->MenuSelected);

  if (!UserInputData->IsKeyboard) {
    if (IsInputingOnTheFly) {
      return EFI_SUCCESS;
    }

    if (UserInputData->KeyData.Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
      return Status;
    }

    Status = TransferToTextModePosition (Private, UserInputData->CursorX, UserInputData->CursorY, &Column, &Row);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    BorderLineWidth = GetBorderWidth (Private->MenuSelected->ParentPanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL);
    Status = GetControlAbsField (
               &Private->MenuSelected->ParentPanel->PanelField,
               BorderLineWidth,
               &Private->MenuSelected->ControlField,
               &ControlAbsField
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (!IsPointOnField (&ControlAbsField, (INT32) Column, (INT32) Row)) {
      return EFI_UNSUPPORTED;
    }

    Status = GetControlAbsField (
               &Private->MenuSelected->ParentPanel->PanelField,
               BorderLineWidth,
               &Private->MenuSelected->ValueStrInfo.StringField,
               &ControlAbsField
               );
    if (EFI_ERROR (Status)) {
      return EFI_SUCCESS;
    }

    Status = DateTimeOpCodeGetSequenceValue ((Private->MenuSelected->Operand == EFI_IFR_DATE_OP), &ControlAbsField, (INT32) Column, (INT32) Row, &Sequence);
    if (!EFI_ERROR (Status) && Sequence != Private->MenuSelected->Sequence) {
      Private->MenuSelected->Sequence = Sequence;
      DisplayHighLightControl (FALSE, Private->MenuSelected);
    }
    return EFI_SUCCESS;
  }

  Key = &UserInputData->KeyData.Key;

  switch (Key->ScanCode) {

  case SCAN_ESC:
    if (IsInputingOnTheFly) {
      RestoreQuestionValueStr (Private->MenuSelected);
      Status = EFI_SUCCESS;
    }
    return Status;

  default:
    break;
  }

  switch (Key->UnicodeChar) {

  case CHAR_CARRIAGE_RETURN:
  case CHAR_TAB:
    if (IsInputingOnTheFly) {
      Status = DateTimeOpCodeGetHiiValueFromValueStr (Private->MenuSelected, &HiiValue);
      if (!EFI_ERROR (Status)) {
        SendChangeQNotify (
          Private->MenuSelected->PageId,
          Private->MenuSelected->QuestionId,
          &HiiValue
          );
      } else {
        RestoreQuestionValueStr (Private->MenuSelected);
      }
    } else {
      Private->MenuSelected->Sequence = (Private->MenuSelected->Sequence == 2) ? 0 : Private->MenuSelected->Sequence + 1;
      DisplayHighLightControl (FALSE, Private->MenuSelected);
    }
    return EFI_SUCCESS;

  case CHAR_ADD:
  case CHAR_SUB:
    TransferHiiValueToEfiTime (&Private->MenuSelected->HiiValue, &EfiTime);
    Status = GetNextDateTimeValue (
               DateTimeOpCodeGetItemValue (Private->MenuSelected),
               (Key->UnicodeChar == CHAR_ADD),
               &EfiTime
               );
    if (!EFI_ERROR (Status)) {
      TransferEfiTimeToHiiValue (Private->MenuSelected->Operand == EFI_IFR_DATE_OP, &EfiTime, &Private->MenuSelected->HiiValue);
      SendChangeQNotify (
        Private->MenuSelected->PageId,
        Private->MenuSelected->QuestionId,
        &Private->MenuSelected->HiiValue
        );
    }
    return EFI_SUCCESS;

  case CHAR_BACKSPACE:
    if (IsInputingOnTheFly) {
      RestoreQuestionValueStr (Private->MenuSelected);
      Status = EFI_SUCCESS;
    }
    return Status;

  default:
    if (IsInputingOnTheFly) {
      if (!IsDecChar (Key->UnicodeChar)) {
        RestoreQuestionValueStr (Private->MenuSelected);
        return EFI_SUCCESS;
      }

      DateTimeOpCodeInputOnTheFly (Private->MenuSelected, Key);
    } else {
      if (!IsDecChar (Key->UnicodeChar)) {
        return EFI_UNSUPPORTED;
      }
      DateTimeOpCodeInputOnTheFly (Private->MenuSelected, Key);
    }
    break;
  }

  return EFI_SUCCESS;
}

CHAR16 *
OrderListOpCodeCreateValueStr (
  IN H2O_FORM_BROWSER_Q                       *Question
  )
{
  UINT32                                      OptionIndex;
  UINT32                                      ContainerIndex;
  H2O_FORM_BROWSER_O                          *Option;
  CHAR16                                      *TempStr;
  CHAR16                                      *ResultStr;
  UINT64                                      Value;
  UINT8                                       ValueType;

  if (Question == NULL || Question->Operand != EFI_IFR_ORDERED_LIST_OP) {
    return NULL;
  }

  if (Question->NumberOfOptions == 0 || Question->Options == NULL) {
    return NULL;
  }

  ValueType = Question->Options[0].HiiValue.Type;
  ResultStr = AllocateZeroPool (sizeof (CHAR16));
  if (ResultStr == NULL) {
    return NULL;
  }

  for (ContainerIndex = 0; ContainerIndex < Question->ContainerCount; ContainerIndex++) {
    Value = GetHiiBufferValue (Question->HiiValue.Buffer, ValueType, ContainerIndex);
    if (Value == 0) {
      break;
    }

    for (OptionIndex = 0; OptionIndex < Question->NumberOfOptions; OptionIndex++) {
      Option = &(Question->Options[OptionIndex]);
      if (!Option->Visibility || Option->HiiValue.Value.u64 != Value) {
        continue;
      }

      TempStr = ResultStr;
      if (*ResultStr == CHAR_NULL) {
        ResultStr = AllocateCopyPool (StrSize (Option->Text), Option->Text);
      } else {
        ResultStr = CatSPrint (NULL, L"%s%s%s", ResultStr, L"\n\r", Option->Text);
      }
      FreePool (TempStr);
      if (ResultStr == NULL) {
        return NULL;
      }
      break;
    }
  }

  return ResultStr;
}

EFI_STATUS
OrderListOpCodeShiftOrder (
  IN H2O_CONTROL_INFO                         *Control,
  IN BOOLEAN                                  ShiftNext
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_Q                          *Question;
  UINT64                                      CurrentValue;
  UINT64                                      NextValue;
  UINT8                                       Type;

  if (Control == NULL || Control->Operand != EFI_IFR_ORDERED_LIST_OP) {
    return EFI_INVALID_PARAMETER;
  }

  Status = mDEPrivate->FBProtocol->GetQInfo (mDEPrivate->FBProtocol, Control->PageId, Control->QuestionId, &Question);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  switch (Question->HiiValue.BufferLen / Question->ContainerCount) {

  case 1:
    Type = EFI_IFR_TYPE_NUM_SIZE_8;
    break;

  case 2:
    Type = EFI_IFR_TYPE_NUM_SIZE_16;
    break;

  case 4:
    Type = EFI_IFR_TYPE_NUM_SIZE_32;
    break;
  case 8:
    Type = EFI_IFR_TYPE_NUM_SIZE_64;
    break;

  default:
    return EFI_ABORTED;
  }

  if (ShiftNext) {
    if (Control->Sequence == Question->NumberOfOptions - 1) {
      FreePool ((VOID *) Question);
      return EFI_SUCCESS;
    }

    CurrentValue = GetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence);
    NextValue    = GetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence + 1);

    SetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence    , NextValue);
    SetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence + 1, CurrentValue);
    Control->Sequence++;
  } else {
    if (Control->Sequence == 0) {
      FreePool ((VOID *) Question);
      return EFI_SUCCESS;
    }

    CurrentValue = GetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence);
    NextValue    = GetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence - 1);

    SetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence    , NextValue);
    SetHiiBufferValue (Control->HiiValue.Buffer, Type, Control->Sequence - 1, CurrentValue);
    Control->Sequence--;
  }

  FreePool ((VOID *) Question);
  return SendChangeQNotify (Question->PageId, Question->QuestionId, &Control->HiiValue);
}

CHAR16 *
GetQuestionPromptStr (
  IN H2O_FORM_BROWSER_Q                       *Question
  )
{
  CHAR16                                      *QuestionPromptStr;
  UINTN                                       QuestionPromptStrSize;
  CHAR16                                      *ModifiedPromptStr;

  if (Question == NULL) {
    return NULL;
  }

  QuestionPromptStr     = (Question->Prompt != NULL && *Question->Prompt != CHAR_NULL) ? Question->Prompt : L" ";
  QuestionPromptStrSize = StrSize (QuestionPromptStr);

  switch (Question->Operand) {

  case EFI_IFR_REF_OP:
    ModifiedPromptStr = AllocateZeroPool (QuestionPromptStrSize + sizeof (CHAR16));
    if (ModifiedPromptStr != NULL) {
      ModifiedPromptStr[0] = REF_OP_DELIMITER;
      CopyMem (&ModifiedPromptStr[1], QuestionPromptStr, QuestionPromptStrSize);
    }
    return ModifiedPromptStr;

  default:
    return (CHAR16 *) AllocateCopyPool (QuestionPromptStrSize, QuestionPromptStr);
  }
}

CHAR16 *
GetQuestionValueStr (
  IN H2O_FORM_BROWSER_Q                       *Question
  )
{
  CHAR16                                      *ValueStr;
  H2O_FORM_BROWSER_O                          *Option;
  UINT32                                      Index;
  CHAR16                                      *TempStr;

  if (Question == NULL) {
    return NULL;
  }

  switch (Question->Operand) {

  case EFI_IFR_SUBTITLE_OP:
  case EFI_IFR_REF_OP:
  case EFI_IFR_RESET_BUTTON_OP:
    return NULL;

  case EFI_IFR_TEXT_OP:
  case EFI_IFR_ACTION_OP:
    return (Question->TextTwo != NULL) ? AllocateCopyPool (StrSize (Question->TextTwo), Question->TextTwo) : NULL;

  case EFI_IFR_ONE_OF_OP:
    for (Index = 0; Index < Question->NumberOfOptions; Index++) {
      Option = &(Question->Options[Index]);
      if ((Option->HiiValue.Type      == Question->HiiValue.Type     ) &&
          (Option->HiiValue.Value.u64 == Question->HiiValue.Value.u64)) {
        return CatSPrint (
                 NULL,
                 L"%c%s%c",
                 LEFT_ONEOF_DELIMITER,
                 Option->Text,
                 RIGHT_ONEOF_DELIMITER
                 );
      }
    }
    break;

  case EFI_IFR_ORDERED_LIST_OP:
    return OrderListOpCodeCreateValueStr (Question);

  case EFI_IFR_NUMERIC_OP:
    TempStr = PrintFormattedNumber (Question);
    if (TempStr == NULL) {
      break;
    }
    ValueStr = CatSPrint (
                 NULL,
                 L"%c%s%c",
                 LEFT_NUMERIC_DELIMITER,
                 TempStr,
                 RIGHT_NUMERIC_DELIMITER
                 );
    FreePool (TempStr);
    return ValueStr;

  case EFI_IFR_CHECKBOX_OP:
    return CatSPrint (
             NULL,
             L"%c%c%c",
             LEFT_CHECKBOX_DELIMITER,
             (Question->HiiValue.Value.b) ? CHECK_ON : CHECK_OFF,
             RIGHT_CHECKBOX_DELIMITER
             );

  case EFI_IFR_TIME_OP:
  case EFI_IFR_DATE_OP:
    return DateTimeOpCodeCreateValueStr (Question);

  case EFI_IFR_STRING_OP:
  case EFI_IFR_PASSWORD_OP:
    return (Question->HiiValue.Buffer != NULL) ? AllocateCopyPool (StrSize ((CHAR16 *) Question->HiiValue.Buffer), Question->HiiValue.Buffer) : NULL;

  default:
    break;
  }

  return NULL;
}

EFI_STATUS
ExchangeContainerValue (
  IN     H2O_CONTROL_LIST                     *PopUpControls,
  IN OUT UINT8                                *Buffer,
  IN     UINT8                                Type,
  IN     EFI_HII_VALUE                        *HiiValue1,
  IN     EFI_HII_VALUE                        *HiiValue2
  )
{
  EFI_STATUS                                  Status;
  H2O_CONTROL_INFO                            *Controls = NULL;
  UINT32                                      Index1;
  UINT32                                      Index2;

  H2O_CONTROL_INFO                            *OrgControl1;
  H2O_CONTROL_INFO                            *OrgControl2;


  if (HiiValue1 == NULL || HiiValue2 == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = GetPopUpSelectionByValue (PopUpControls, HiiValue1, &Controls, &Index1);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }
  Status = GetPopUpSelectionByValue (PopUpControls, HiiValue2, &Controls, &Index2);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  if (Index1 >= PopUpControls->Count || Index2 >= PopUpControls->Count) {
    return EFI_NOT_FOUND;
  }

  //
  // Exchange buffer
  //
  SetHiiBufferValue (Buffer, Type, Index1, HiiValue2->Value.u64);
  SetHiiBufferValue (Buffer, Type, Index2, HiiValue1->Value.u64);

  //
  // Exchange controls
  //
  OrgControl1 = AllocateCopyPool (sizeof (H2O_CONTROL_INFO), &PopUpControls->ControlArray[Index1]);
  if (OrgControl1 == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  OrgControl2 = AllocateCopyPool (sizeof (H2O_CONTROL_INFO), &PopUpControls->ControlArray[Index2]);
  if (OrgControl2 == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (&PopUpControls->ControlArray[Index1], OrgControl2, sizeof (H2O_CONTROL_INFO));
  CopyMem (&PopUpControls->ControlArray[Index2], OrgControl1, sizeof (H2O_CONTROL_INFO));

  PopUpControls->ControlArray[Index1].ControlField.top = OrgControl1->ControlField.top;
  PopUpControls->ControlArray[Index1].ControlField.bottom   = OrgControl1->ControlField.top + (OrgControl2->ControlField.bottom - OrgControl2->ControlField.top);

  PopUpControls->ControlArray[Index2].ControlField.top = OrgControl2->ControlField.top;
  PopUpControls->ControlArray[Index2].ControlField.bottom   = OrgControl2->ControlField.top + (OrgControl1->ControlField.bottom - OrgControl1->ControlField.top);

  FreePool (OrgControl1);
  FreePool (OrgControl2);

  return EFI_SUCCESS;
}

EFI_STATUS
InitializeVirtualKb (
  VOID
  )
{
  EFI_STATUS                                  Status;
  EFI_SETUP_MOUSE_PROTOCOL                    *SetupMouse;
  LIST_ENTRY                                  *Link;
  KEYBOARD_ATTRIBUTES                         KeyboardAttributes;
  H2O_FORM_BROWSER_CONSOLE_DEV_NODE           *ConsoleDevNode;
  UINTN                                       X;
  UINTN                                       Y;

  Status = gBS->LocateProtocol (&gSetupMouseProtocolGuid, NULL, (VOID **) &SetupMouse);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = SetupMouse->GetKeyboardAttributes (SetupMouse, &KeyboardAttributes);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (KeyboardAttributes.IsStart) {
    return EFI_SUCCESS;
  }

  KeyboardAttributes.WidthPercentage  = 100;
  KeyboardAttributes.HeightPercentage = 40;
  SetupMouse->SetKeyboardAttributes (SetupMouse, &KeyboardAttributes);

  Link = mDEPrivate->ConsoleDevListHead.ForwardLink;
  ConsoleDevNode = H2O_FORM_BROWSER_CONSOLE_DEV_NODE_FROM_LINK (Link);
  Status = ConsoleDevNode->SimpleTextOut->QueryMode (
                                            ConsoleDevNode->SimpleTextOut,
                                            ConsoleDevNode->SimpleTextOut->Mode->Mode,
                                            &X,
                                            &Y
                                            );
  if (!EFI_ERROR (Status)) {
    Y = (Y * EFI_GLYPH_HEIGHT * (100 - KeyboardAttributes.HeightPercentage)) / 100;
  } else {
    Y = 0;
  }

  return SetupMouse->StartKeyboard (SetupMouse, 0, Y);
}

EFI_STATUS
ShutdownVirtualKb (
  VOID
  )
{
  EFI_STATUS                                  Status;
  EFI_SETUP_MOUSE_PROTOCOL                    *SetupMouse;
  KEYBOARD_ATTRIBUTES                         KeyboardAttributes;

  Status = gBS->LocateProtocol (&gSetupMouseProtocolGuid, NULL, (VOID **) &SetupMouse);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = SetupMouse->GetKeyboardAttributes (SetupMouse, &KeyboardAttributes);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!KeyboardAttributes.IsStart) {
    return EFI_SUCCESS;
  }

  return SetupMouse->CloseKeyboard (SetupMouse);
}

EFI_STATUS
ProcessNumberInput (
  IN     H2O_CONTROL_LIST                    *PopUpControls,
  IN     BOOLEAN                             IsHex,
  IN     EFI_INPUT_KEY                       *Key,
  IN OUT UINT32                              *TabIndex,
  OUT    BOOLEAN                             *IsEnterVisibleChar,
  IN OUT CHAR16                              **ResultStr
  )
{
  BOOLEAN                                    InputNumber;
  UINT32                                     TempTabIndex;
  BOOLEAN                                    TempIsEnterVisibleChar;
  CHAR16                                     *TempStr = NULL;
  CHAR16                                     *Ptr;
  UINT64                                     TempValue;


  SafeFreePool ((VOID **) ResultStr);

  TempTabIndex           = (TabIndex != NULL) ? *TabIndex : 0;
  TempIsEnterVisibleChar = TRUE;
  GetInputWithTab (PopUpControls, TempTabIndex, &TempStr);
  InputNumber            = IsHex ? IsHexChar (Key->UnicodeChar) : IsDecChar (Key->UnicodeChar);

  if (InputNumber) {
    //
    // Press visible char
    //
    Ptr = TempStr;
    TempStr = CatSPrint (TempStr, L"%c", Key->UnicodeChar);
    FreePool (Ptr);
  } else if (Key->UnicodeChar == CHAR_BACKSPACE) {
    //
    // Press backspace
    //
    TempStr[StrLen(TempStr) - 1] = '\0';
  } else if (TabIndex != NULL && (Key->UnicodeChar == CHAR_TAB || Key->UnicodeChar == CHAR_CARRIAGE_RETURN)) {
    //
    // Press tab
    //
    if (TempTabIndex < GetInputMaxTabNumber (PopUpControls)) {
      TempTabIndex ++;
    } else {
      TempTabIndex = 0;
    }
    GetInputWithTab (PopUpControls, TempTabIndex, &TempStr);
  } else if (Key->UnicodeChar == CHAR_SUB || Key->UnicodeChar == CHAR_ADD) {
    //
    // Press '+' or '-'
    //
    if (IsHex) {
      TempValue = StrHexToUint64 (TempStr);
    } else {
      TempValue = StrDecimalToUint64 (TempStr);
    }
    FreePool (TempStr);
    if (Key->UnicodeChar == CHAR_SUB) {
      TempValue --;
    } else {
      TempValue ++;
    }
    if (IsHex) {
      TempStr = CatSPrint (NULL, L"%X", TempValue);
    } else {
      TempStr = CatSPrint (NULL, L"%d", TempValue);
    }
  } else {
    //
    // Not press visible char or backspace or tab
    //
    TempIsEnterVisibleChar = FALSE;
  }

  if (TabIndex != NULL) {
    *TabIndex = TempTabIndex;
  }
  *IsEnterVisibleChar = TempIsEnterVisibleChar;
  *ResultStr = TempStr;

  return EFI_SUCCESS;
}

EFI_STATUS
ProcessCharInput (
  IN     H2O_CONTROL_LIST                    *PopUpControls,
  IN     EFI_INPUT_KEY                       *Key,
  IN OUT UINT32                              *TabIndex,
  OUT    BOOLEAN                             *IsEnterVisibleChar,
  IN OUT CHAR16                              **ResultStr
  )
{
  UINT32                                     TempTabIndex;
  BOOLEAN                                    TempIsEnterVisibleChar;
  CHAR16                                     *TempStr = NULL;

  SafeFreePool ((VOID **) ResultStr);

  TempTabIndex = (TabIndex != NULL) ? *TabIndex : 0;

  TempIsEnterVisibleChar = TRUE;
  GetInputWithTab (PopUpControls, TempTabIndex, &TempStr);

  if (IsVisibleChar (Key->UnicodeChar)) {
    TempStr = CatSPrint (TempStr, L"%c", Key->UnicodeChar);
  } else if (Key->UnicodeChar == CHAR_BACKSPACE) {
    //
    // Press backspace
    //
    TempStr[StrLen(TempStr) - 1] = '\0';
  } else if (TabIndex != NULL && (Key->UnicodeChar == CHAR_TAB || Key->UnicodeChar == CHAR_CARRIAGE_RETURN)) {
    //
    // Press tab
    //
    if (TempTabIndex < GetInputMaxTabNumber (PopUpControls)) {
      TempTabIndex++;
    } else if (Key->UnicodeChar == CHAR_TAB) {
      TempTabIndex = 0;
    } else {
      //
      // End of tab index and press CHAR_CARRIAGE_RETURN
      //
      TempTabIndex = 0;
      TempIsEnterVisibleChar = FALSE;
    }
    GetInputWithTab (PopUpControls, TempTabIndex, &TempStr);
  } else {
    //
    // Not press visible char or backspace or tab
    //
    TempIsEnterVisibleChar = FALSE;
  }

  if (TabIndex != NULL) {
    *TabIndex = TempTabIndex;
  }

  *IsEnterVisibleChar = TempIsEnterVisibleChar;
  *ResultStr          = TempStr;

  return EFI_SUCCESS;
}

VOID
LetDateRegular (
  IN OUT EFI_HII_VALUE                     *HiiValue
  )
{
  //
  // Year
  //
  if (HiiValue->Value.date.Year < 2000) {
    HiiValue->Value.date.Year = 2099;
  } else if (HiiValue->Value.date.Year > 2099) {
    HiiValue->Value.date.Year = 2000;
  }

  //
  // Month
  //
  if (HiiValue->Value.date.Month < 1) {
    HiiValue->Value.date.Month = 12;
  } else if (HiiValue->Value.date.Month > 12) {
    HiiValue->Value.date.Month = 1;
  }

  //
  // Day
  //
  switch (HiiValue->Value.date.Month) {
    case 2:
      if (HiiValue->Value.date.Year / 4 == 0) {
        if (HiiValue->Value.date.Day < 1) {
          HiiValue->Value.date.Day = 29;
        } else if (HiiValue->Value.date.Day > 29) {
          HiiValue->Value.date.Day = 1;
        }
      } else {
        if (HiiValue->Value.date.Day < 1) {
          HiiValue->Value.date.Day = 28;
        } else if (HiiValue->Value.date.Day > 28) {
          HiiValue->Value.date.Day = 1;
        }
      }
      break;

    case 4:
    case 6:
    case 9:
    case 11:
      if (HiiValue->Value.date.Day < 1) {
        HiiValue->Value.date.Day = 30;
      } else if (HiiValue->Value.date.Day > 30) {
        HiiValue->Value.date.Day = 1;
      }
      break;

    default:
      if (HiiValue->Value.date.Day < 1) {
        HiiValue->Value.date.Day = 31;
      } else if (HiiValue->Value.date.Day > 31) {
        HiiValue->Value.date.Day = 1;
      }
      break;
  }
}

VOID
LetTimeRegular (
  IN OUT EFI_HII_VALUE                       *HiiValue
  )
{
  //
  // Hour
  //
  if (HiiValue->Value.time.Hour == (UINT8) (-1)) {
    HiiValue->Value.time.Hour = 23;
  } else if (HiiValue->Value.time.Hour > 23) {
    HiiValue->Value.time.Hour = 0;
  }

  //
  // Minute
  //
  if (HiiValue->Value.time.Minute == (UINT8) (-1)) {
    HiiValue->Value.time.Minute = 59;
  } else if (HiiValue->Value.time.Minute > 59) {
    HiiValue->Value.time.Minute = 0;
  }

  //
  // Second
  //
  if (HiiValue->Value.time.Second == (UINT8) (-1)) {
    HiiValue->Value.time.Second = 59;
  } else if (HiiValue->Value.time.Second > 59) {
    HiiValue->Value.time.Second = 0;
  }
}

EFI_STATUS
CreateValueControls (
  IN H2O_FORM_BROWSER_Q                       *Question,
  IN H2O_CONTROL_LIST                         *PromptControls,
  IN H2O_CONTROL_LIST                         *ValueControls,
  IN H2O_CONTROL_LIST                         *PopUpControls
  )
{
  UINT32                                      Index;
  RECT                                        ValueField;
  UINT32                                      ContainerIndex;
  UINT32                                      OptionIndex;
  H2O_FORM_BROWSER_O                          *Option;
  UINT64                                      ContainerIndexValue;
  CHAR16                                      *ControlStr;
  UINT32                                      ControlCount;
  H2O_CONTROL_INFO                            *ControlArray;
  EFI_HII_VALUE                               HiiValue;
  H2O_PANEL_INFO                              *ValuePanel;
  UINT32                                      ValuePanelWidth;
  CHAR16                                      *OptionStr;
  LIST_ENTRY                                  *PanelLink;
  UINT32                                      PseudoClass;
  INT32                                       BorderLineWidth;

  PanelLink = &mDEPrivate->Layout->PanelListHead;
  if (IsNull (PanelLink, PanelLink->ForwardLink)) {
    return EFI_NOT_FOUND;
  }

  ValuePanel      = GetPanelInfoByType (&mDEPrivate->Layout->PanelListHead, H2O_PANEL_TYPE_SETUP_PAGE);
  BorderLineWidth = (ValuePanel != NULL) ? GetBorderWidth (ValuePanel, NULL, NULL, NULL, NULL, H2O_IFR_STYLE_TYPE_PANEL, H2O_STYLE_PSEUDO_CLASS_NORMAL) : 0;
  ValuePanelWidth = (ValuePanel != NULL) ? ((UINT32) (ValuePanel->PanelField.right - ValuePanel->PanelField.left) / 2 - 2 * BorderLineWidth) : 10;

  //
  // Get question value field
  //
  for (Index = 0; Index < PromptControls->Count; Index ++) {
    if (PromptControls->ControlArray[Index].PageId     == Question->PageId &&
        PromptControls->ControlArray[Index].QuestionId == Question->QuestionId) {
      break;
    }
  }
  if (Index == PromptControls->Count) {
    return EFI_NOT_FOUND;
  }

  CopyRect (&ValueField, &PromptControls->ControlArray[Index].ValueStrInfo.StringField);
//  CopyMem (&ValueField, &PromptControls->ControlArray[Index].ControlField, sizeof(RECT));
  PseudoClass = PromptControls->ControlArray[Index].ControlStyle.PseudoClass;

  ControlArray = NULL;
  ControlStr   = NULL;
  ControlCount = 0;
  ZeroMem (&HiiValue, sizeof (EFI_HII_VALUE));

  switch (Question->Operand) {

  case EFI_IFR_ORDERED_LIST_OP:
    if (Question->ContainerCount == 0) {
      return EFI_NOT_FOUND;
    }

    ControlCount = Question->ContainerCount;
    OptionStr    = AllocateZeroPool (sizeof (CHAR16) * (ValuePanelWidth + 1));
    ControlArray = (H2O_CONTROL_INFO *) AllocateZeroPool (sizeof (H2O_CONTROL_INFO) * ControlCount);
    if (OptionStr == NULL || ControlArray == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    for (ContainerIndex = 0; ContainerIndex < ControlCount; ContainerIndex ++) {
      for (OptionIndex = 0; OptionIndex < Question->NumberOfOptions; OptionIndex++) {
        Option = &(Question->Options[OptionIndex]);
        if (!Option->Visibility) {
          continue;
        }

        ContainerIndexValue = GetHiiBufferValue (Question->HiiValue.Buffer, Option->HiiValue.Type, ContainerIndex);
        if (ContainerIndexValue == Option->HiiValue.Value.u64) {
          for (Index = 0; Index < ValuePanelWidth; Index ++) {
            OptionStr [Index] = ' ';
          }

          if ((GetStringWidth (Option->Text) / sizeof (CHAR16) - 1) < ValuePanelWidth) {
            CopyMem (OptionStr, Option->Text, sizeof (CHAR16) * StrLen (Option->Text));
          } else {
            CopyMem (OptionStr, Option->Text, sizeof (CHAR16) * (ValuePanelWidth - 5));
            CopyMem (&OptionStr[ValuePanelWidth - 5], L"...", sizeof (L"..."));
          }
          ControlStr = CatSPrint (NULL, L"%s", OptionStr);
          CopyMem (&HiiValue, &Option->HiiValue, sizeof (EFI_HII_VALUE));
          break;
        }
      }

      if (OptionIndex >= Question->NumberOfOptions) {
        FreePool (ControlArray);
        FreePool (OptionStr);
        return EFI_NOT_FOUND;
      }

      ControlArray[ContainerIndex].Selectable               = TRUE;
      ControlArray[ContainerIndex].Modifiable               = FALSE;
      ControlArray[ContainerIndex].Editable                 = FALSE;
      ControlArray[ContainerIndex].Text.String              = ControlStr;
      ControlArray[ContainerIndex].ControlField.left        = ValueField.left;
      ControlArray[ContainerIndex].ControlField.right       = ValueField.right;
      ControlArray[ContainerIndex].ControlField.top         = ValueField.top + ContainerIndex;
      ControlArray[ContainerIndex].ControlField.bottom      = ControlArray[ContainerIndex].ControlField.top;
      ControlArray[ContainerIndex].ParentPanel              = ValuePanel;
      ControlArray[ContainerIndex].ControlStyle.PseudoClass = PseudoClass;
      CopyMem (&ControlArray[ContainerIndex].HiiValue, &HiiValue, sizeof (EFI_HII_VALUE));
    }
    FreePool (OptionStr);
    break;

  case EFI_IFR_DATE_OP:
    ControlCount = 3;
    ControlArray = (H2O_CONTROL_INFO *) AllocateZeroPool (sizeof (H2O_CONTROL_INFO) * ControlCount);
    if (ControlArray == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    for (Index = 0; Index < ControlCount; Index ++) {
      if (Index == 0) {
        ControlStr = CatSPrint (NULL, L"%04d", Question->HiiValue.Value.date.Year);
      } else if (Index == 1) {
        ControlStr = CatSPrint (NULL, L"%02d", Question->HiiValue.Value.date.Month);
      } else {
        ControlStr = CatSPrint (NULL, L"%02d", Question->HiiValue.Value.date.Day);
      }

      ControlArray[Index].Selectable               = TRUE;
      ControlArray[Index].Modifiable               = TRUE;
      ControlArray[Index].Editable                 = TRUE;
      ControlArray[Index].Text.String              = ControlStr;
      ControlArray[Index].HiiValue.Type            = EFI_IFR_TYPE_STRING;
      ControlArray[Index].HiiValue.BufferLen       = (UINT16) StrSize (ControlStr);
      ControlArray[Index].HiiValue.Buffer          = (UINT8 *) ControlStr;
      if (Index == 0) {
        ControlArray[Index].ControlField.left      = ValueField.left;
        ControlArray[Index].ControlField.right     = ControlArray[Index].ControlField.left + 3;
      } else {
        ControlArray[Index].ControlField.left      = ControlArray[Index - 1].ControlField.right + 2;
        ControlArray[Index].ControlField.right     = ControlArray[Index].ControlField.left + 1;
      }
      ControlArray[Index].ControlField.top         = ValueField.top;
      ControlArray[Index].ControlField.bottom      = ValueField.bottom;
      ControlArray[Index].ParentPanel              = ValuePanel;
      ControlArray[Index].ControlStyle.PseudoClass = PseudoClass;
    }
    break;

  case EFI_IFR_TIME_OP:
    ControlCount = 3;
    ControlArray = (H2O_CONTROL_INFO *) AllocateZeroPool (sizeof (H2O_CONTROL_INFO) * ControlCount);
    if (ControlArray == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    for (Index = 0; Index < ControlCount; Index ++) {
      if (Index == 0) {
        ControlStr = CatSPrint (NULL, L"%02d", Question->HiiValue.Value.time.Hour);
      } else if (Index == 1) {
        ControlStr = CatSPrint (NULL, L"%02d", Question->HiiValue.Value.time.Minute);
      } else {
        ControlStr = CatSPrint (NULL, L"%02d", Question->HiiValue.Value.time.Second);
      }

      ControlArray[Index].Selectable               = TRUE;
      ControlArray[Index].Modifiable               = TRUE;
      ControlArray[Index].Editable                 = TRUE;
      ControlArray[Index].Text.String              = ControlStr;
      ControlArray[Index].HiiValue.Type            = EFI_IFR_TYPE_STRING;
      ControlArray[Index].HiiValue.BufferLen       = (UINT16) StrSize (ControlStr);
      ControlArray[Index].HiiValue.Buffer          = (UINT8 *) ControlStr;
      if (Index == 0) {
        ControlArray[Index].ControlField.left      = ValueField.left;
        ControlArray[Index].ControlField.right     = ControlArray[Index].ControlField.left + 1;
      } else {
        ControlArray[Index].ControlField.left      = ControlArray[Index - 1].ControlField.right + 2;
        ControlArray[Index].ControlField.right     = ControlArray[Index].ControlField.left + 1;
      }
      ControlArray[Index].ControlField.top         = ValueField.top;
      ControlArray[Index].ControlField.bottom      = ValueField.bottom;
      ControlArray[Index].ParentPanel              = ValuePanel;
      ControlArray[Index].ControlStyle.PseudoClass = PseudoClass;
    }
    break;

  default:
    return EFI_NOT_FOUND;
  }

  //
  // Save
  //
  if (PopUpControls != NULL && ControlArray != NULL) {
    FreeControlList (PopUpControls);
    PopUpControls->Count        = ControlCount;
    PopUpControls->ControlArray = AllocateCopyPool (sizeof (H2O_CONTROL_INFO) * ControlCount, ControlArray);
    if (PopUpControls->ControlArray == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    FreePool (ControlArray);
    //
    // Display
    //
    DisplayNormalControls (PopUpControls->Count, PopUpControls->ControlArray);
  }

  return EFI_SUCCESS;
}

BOOLEAN
IsDialogStatus (
  IN UINT8                                    DEStatus
  )
{
  if (DEStatus == DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG ||
      DEStatus == DISPLAY_ENGINE_STATUS_OPEN_D ||
      DEStatus == DISPLAY_ENGINE_STATUS_SHUT_D ||
      DEStatus == DISPLAY_ENGINE_STATUS_CHANGING_Q) {
    return TRUE;
  }

  return FALSE;
}

H2O_CONTROL_INFO *
FindControlByControlId (
  IN H2O_CONTROL_INFO                         *ControlArray,
  IN UINT32                                   ControlArrayCount,
  IN UINT32                                   ControlId,
  IN UINT32                                   SequenceIndex
  )
{
  UINT32                                      Index;
  UINT32                                      Count;

  if (ControlArray == NULL) {
    return NULL;
  }

  Count = 0;

  for (Index = 0; Index < ControlArrayCount; Index++) {
    if (ControlArray[Index].ControlId != ControlId) {
      continue;
    }

    if (Count == SequenceIndex) {
      return &ControlArray[Index];
    }

    Count++;
  }

  return NULL;
}

H2O_CONTROL_INFO *
FindNextSelectableControl (
  IN H2O_CONTROL_INFO                         *ControlArray,
  IN UINT32                                   ControlArrayCount,
  IN H2O_CONTROL_INFO                         *CurrentControl,
  IN BOOLEAN                                  GoDown,
  IN BOOLEAN                                  IsLoop
  )
{
  UINT32                                      Index;
  H2O_CONTROL_INFO                            *FirstControl;
  H2O_CONTROL_INFO                            *LastControl;
  H2O_CONTROL_INFO                            *PreviousControl;
  H2O_CONTROL_INFO                            *NextControl;
  BOOLEAN                                     CurrentControlFound;


  if (ControlArray == NULL) {
    return NULL;
  }

  FirstControl        = NULL;
  PreviousControl     = NULL;
  NextControl         = NULL;
  LastControl         = NULL;
  CurrentControlFound = (CurrentControl == NULL) ? TRUE : FALSE;

  for (Index = 0; Index < ControlArrayCount; Index++) {
    if (!IsValidHighlightStatement(&ControlArray[Index])) {
      continue;
    }

    if (FirstControl == NULL) {
      FirstControl = &ControlArray[Index];
    }

    if (CurrentControlFound && NextControl == NULL) {
      NextControl = &ControlArray[Index];
    }

    if (&ControlArray[Index] == CurrentControl) {
      CurrentControlFound = TRUE;
    }

    if (!CurrentControlFound) {
      PreviousControl = &ControlArray[Index];
    }

    LastControl = &ControlArray[Index];
  }

  if (GoDown) {
    if (NextControl != NULL) {
      return NextControl;
    }

    if (IsLoop && FirstControl != NULL && FirstControl != CurrentControl) {
      return FirstControl;
    }
  } else {
    if (PreviousControl != NULL) {
      return PreviousControl;
    }

    if (IsLoop && LastControl != NULL && LastControl != CurrentControl) {
      return LastControl;
    }
  }

  return NULL;
}

EFI_STATUS
DlgProcessUserInput (
  IN  BOOLEAN                               IsKeyboard,
  IN  EFI_INPUT_KEY                         *InputKey,
  IN  UINT32                                MouseX,
  IN  UINT32                                MouseY,
  IN  H2O_CONTROL_LIST                      *PopUpControls,
  OUT H2O_CONTROL_INFO                      **SelectedControl,
  OUT CHAR16                                **InputString,
  OUT BOOLEAN                               *IsShutdownDialog
  )
{
  BOOLEAN                                   CurrentIsBody;
  BOOLEAN                                   CurrentIsBodyInput;
  BOOLEAN                                   CurrentIsButton;
  UINTN                                     StringLength;
  UINTN                                     MinStringLength;
  H2O_CONTROL_INFO                          *NextControl;
  EFI_STATUS                                Status;

  if (InputKey == NULL || PopUpControls == NULL || SelectedControl == NULL || InputString == NULL || IsShutdownDialog == NULL) {
    return EFI_UNSUPPORTED;
  }

  *SelectedControl   = NULL;
  *InputString       = NULL;
  *IsShutdownDialog  = FALSE;
  CurrentIsBody      = (mDEPrivate->PopUpSelected->ControlId == H2O_CONTROL_ID_DIALOG_BODY)       ? TRUE : FALSE;
  CurrentIsBodyInput = (mDEPrivate->PopUpSelected->ControlId == H2O_CONTROL_ID_DIALOG_BODY_INPUT) ? TRUE : FALSE;
  CurrentIsButton    = (mDEPrivate->PopUpSelected->ControlId == H2O_CONTROL_ID_DIALOG_BUTTON)     ? TRUE : FALSE;
  Status             = EFI_SUCCESS;

  if (!IsKeyboard) {
    if (CheckPressControls (IsKeyboard, InputKey, MouseX, MouseY, PopUpControls->Count, PopUpControls->ControlArray, &NextControl)) {
      if (NextControl == NULL) {
        *IsShutdownDialog = TRUE;
      } else {
        *SelectedControl = NextControl;
      }
    }

    return Status;
  }

  //
  // Process keyboard event
  //
  switch (InputKey->UnicodeChar) {

  case CHAR_TAB:
    if (CurrentIsBody) {
      *SelectedControl = FindControlByControlId (PopUpControls->ControlArray, PopUpControls->Count, H2O_CONTROL_ID_DIALOG_BUTTON, 0);
      break;
    }

    NextControl = FindNextSelectableControl (PopUpControls->ControlArray, PopUpControls->Count, mDEPrivate->PopUpSelected, TRUE, TRUE);
    if (NextControl != NULL && NextControl != mDEPrivate->PopUpSelected) {
      *SelectedControl = NextControl;
    }
    break;

  case CHAR_CARRIAGE_RETURN:
    if (CurrentIsBodyInput) {
      *SelectedControl = FindNextSelectableControl (PopUpControls->ControlArray, PopUpControls->Count, mDEPrivate->PopUpSelected, TRUE, FALSE);
      if (*SelectedControl == NULL) {
        *SelectedControl = mDEPrivate->PopUpSelected;
      }
      break;
    }

    if (CurrentIsBody || CurrentIsButton) {
      *SelectedControl = mDEPrivate->PopUpSelected;
      break;
    }
    break;

  case CHAR_BACKSPACE:
    if (CurrentIsBodyInput) {
      if (mDEPrivate->PopUpSelected->Operand == EFI_IFR_NUMERIC_OP && IsHexString (mDEPrivate->PopUpSelected->Text.String)) {
        MinStringLength = sizeof (L"0x") / sizeof(CHAR16) - 1;
      } else {
        MinStringLength = 0;
      }

      StringLength = StrLen (mDEPrivate->PopUpSelected->Text.String);
      if (StringLength > MinStringLength) {
        *InputString = AllocateZeroPool (StringLength * sizeof (CHAR16));
        CopyMem (*InputString, mDEPrivate->PopUpSelected->Text.String, (StringLength - 1) * sizeof (CHAR16));
      }
    }
    break;

  case CHAR_NULL:
    switch (InputKey->ScanCode) {

    case SCAN_RIGHT:
    case SCAN_LEFT:
      if (CurrentIsButton) {
        NextControl = FindNextSelectableControl (
                        PopUpControls->ControlArray,
                        PopUpControls->Count,
                        mDEPrivate->PopUpSelected,
                        (InputKey->ScanCode == SCAN_RIGHT) ? TRUE : FALSE,
                        FALSE
                        );
        if (NextControl != NULL && NextControl->ControlId == H2O_CONTROL_ID_DIALOG_BUTTON) {
          *SelectedControl = NextControl;
        }
      }
      break;

    case SCAN_UP:
    case SCAN_DOWN:
      if (CurrentIsBody) {
        NextControl = FindNextSelectableControl (
                        PopUpControls->ControlArray,
                        PopUpControls->Count,
                        mDEPrivate->PopUpSelected,
                        (InputKey->ScanCode == SCAN_DOWN) ? TRUE : FALSE,
                        FALSE
                        );
        if (NextControl != NULL && NextControl->ControlId == H2O_CONTROL_ID_DIALOG_BODY) {
          *SelectedControl = NextControl;
        }
      }
      break;

    case SCAN_ESC:
      *IsShutdownDialog = TRUE;
      break;
    }
    break;

  default:
    if (CurrentIsBodyInput && IsVisibleChar (InputKey->UnicodeChar)) {
      *InputString = CatSPrint (NULL, L"%s%c", mDEPrivate->PopUpSelected->Text.String, InputKey->UnicodeChar);
    }
    break;
  }

  return Status;
}

EFI_STATUS
GetPasswordHiiValue (
  IN  H2O_CONTROL_LIST                        *PopUpControls,
  OUT EFI_HII_VALUE                           *HiiValue
  )
{
  UINTN                                       Index;
  UINTN                                       TotalStrSize;
  UINTN                                       StringSize;
  UINTN                                       PasswordCount;
  UINT8                                       *StringBuffer;

  if (PopUpControls == NULL || HiiValue == NULL || PopUpControls->ControlArray == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TotalStrSize  = 0;
  PasswordCount = 0;

  for (Index = 0; Index < PopUpControls->Count; Index ++) {
    if (PopUpControls->ControlArray[Index].ControlId == H2O_CONTROL_ID_DIALOG_BODY_INPUT) {
      TotalStrSize += StrSize (PopUpControls->ControlArray[Index].Text.String);
      PasswordCount++;
    }
  }

  if (PasswordCount == 0 || TotalStrSize == 0) {
    return EFI_NOT_FOUND;
  }

  StringBuffer = AllocateZeroPool (TotalStrSize);
  if (StringBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  HiiValue->Type      = EFI_IFR_TYPE_STRING;
  HiiValue->BufferLen = (UINT16) TotalStrSize;
  HiiValue->Buffer    = StringBuffer;

  for (Index = 0; Index < PopUpControls->Count; Index ++) {
    if (PopUpControls->ControlArray[Index].ControlId == H2O_CONTROL_ID_DIALOG_BODY_INPUT) {
      StringSize = StrSize (PopUpControls->ControlArray[Index].Text.String);

      CopyMem (StringBuffer, PopUpControls->ControlArray[Index].Text.String, StringSize);
      StringBuffer += StringSize;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
AtSimpleDialog (
  IN       H2O_FORM_BROWSER_PROTOCOL          *FBProtocol,
  IN       UINT8                              DEStatus,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_CONTROL_LIST                   *PopUpControls
  )
{
  EFI_STATUS                                  Status;
  STATIC H2O_FORM_BROWSER_D                   *Dialog = NULL;
  H2O_PANEL_INFO                              *QuestionPanel;
  H2O_CONTROL_INFO                            *SelectedControl;
  CHAR16                                      *UpdatedString;
  BOOLEAN                                     IsShutdownDialog;

  Status = EFI_NOT_FOUND;

  switch (DEStatus) {

  case DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG:
    if (UserInputData == NULL) {
      return Status;
    }
    if (mDEPrivate->PopUpSelected == NULL) {
      if (UserInputData->IsKeyboard && UserInputData->KeyData.Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
        Status = SendShutDNotify ();
      } else if (UserInputData->IsKeyboard && (UserInputData->KeyData.Key.ScanCode == SCAN_PAGE_UP || UserInputData->KeyData.Key.ScanCode == SCAN_PAGE_DOWN)) {
        QuestionPanel = GetPanelInfoByType (&mDEPrivate->Layout->PanelListHead, H2O_PANEL_TYPE_QUESTION);
        if (QuestionPanel == NULL) {
          break;
        }

        if (UserInputData->KeyData.Key.ScanCode == SCAN_PAGE_UP) {
          PanelPageUp (QuestionPanel);
        } else {
          PanelPageDown (QuestionPanel);
        }
        DisplayDialog (TRUE, Dialog, NULL);
      }
      break;
    }

    Status = DlgProcessUserInput (
               UserInputData->IsKeyboard,
               &UserInputData->KeyData.Key,
               UserInputData->CursorX,
               UserInputData->CursorY,
               PopUpControls,
               &SelectedControl,
               &UpdatedString,
               &IsShutdownDialog
               );
    if (EFI_ERROR (Status)) {
      break;
    }

    if (SelectedControl != NULL) {
      if (SelectedControl == mDEPrivate->PopUpSelected) {
        Status = SendChangeQNotify (0, 0, &mDEPrivate->PopUpSelected->HiiValue);
      } else {
        DisplayControls (FALSE, FALSE, 1, mDEPrivate->PopUpSelected);
        mDEPrivate->PopUpSelected = SelectedControl;
        DisplayHighLightControl (FALSE, mDEPrivate->PopUpSelected);
      }
    }

    if (IsShutdownDialog) {
      Status = SendShutDNotify ();
    }
    break;

  case DISPLAY_ENGINE_STATUS_OPEN_D:
    Dialog = &((H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify)->Dialog;
    Status = DisplayDialog (FALSE, Dialog, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }

    QuestionPanel = GetPanelInfoByType (&mDEPrivate->Layout->PanelListHead, H2O_PANEL_TYPE_QUESTION);
    if (QuestionPanel == NULL) {
      break;
    }

    mDEPrivate->PopUpSelected = FindNextSelectableControl (
                                  QuestionPanel->ControlList.ControlArray,
                                  QuestionPanel->ControlList.Count,
                                  NULL,
                                  TRUE,
                                  TRUE
                                  );
    if (mDEPrivate->PopUpSelected != NULL) {
      DisplayHighLightControl (FALSE, mDEPrivate->PopUpSelected);
    }
    break;

  case DISPLAY_ENGINE_STATUS_SHUT_D:
    Status = ShutDialog ();
    break;
  }

  return Status;
}

EFI_STATUS
AtSimpleInputDialog (
  IN       H2O_FORM_BROWSER_PROTOCOL          *FBProtocol,
  IN       UINT8                              DEStatus,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_CONTROL_LIST                   *PopUpControls
  )
{
  EFI_STATUS                                  Status;

  STATIC H2O_PANEL_INFO                       *OldDialogPanel = NULL;
  H2O_CONTROL_INFO                            *Select;

  BOOLEAN                                     IsEnterVisibleChar;
  EFI_HII_VALUE                               HiiValue;

  CHAR16                                      *TempStr = NULL;
  STATIC BOOLEAN                              HasButton = FALSE;
  EFI_HII_VALUE                               SelectHiiValue;
  STATIC UINT32                               StrWidth = 0;

  H2O_DISPLAY_ENGINE_EVT_CHANGING_Q           *ChangingQNotify;
  H2O_DISPLAY_ENGINE_EVT_OPEN_D               *OpenDNotify;


  Status = EFI_NOT_FOUND;
  switch (DEStatus) {

    case DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG:
      if (PopUpControls == NULL || UserInputData == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      //
      // Check Input
      //
      IsEnterVisibleChar = FALSE;
      if (UserInputData->IsKeyboard) {
        ProcessCharInput (PopUpControls, &UserInputData->KeyData.Key, NULL, &IsEnterVisibleChar, &TempStr);
      } else {
        GetInputWithTab (PopUpControls, 0, &TempStr);
      }

      if (IsEnterVisibleChar) {
        //
        // Prepare HIIValue
        //
        HiiValue.Type = EFI_IFR_TYPE_STRING;
        HiiValue.BufferLen = (UINT16) StrSize (TempStr);
        HiiValue.Buffer = (UINT8 *) TempStr;
        //
        // Press visible char or backspace or tab, notify Changing Question (CHANGING_Q)
        // (In SimpleInput: ChangingQNotify->BodyHiiValue store "Current String")
        // (In SimpleInput: ChangingQNotify->ButtonHiiValue store "PopUpSelected")
        //
        if (HasButton) {
          CopyMem (&SelectHiiValue, &mDEPrivate->PopUpSelected->HiiValue, sizeof (EFI_HII_VALUE));
        } else {
          SelectHiiValue.Type = EFI_IFR_TYPE_BOOLEAN;
          SelectHiiValue.Value.b = TRUE;
        }
        Status = SendChangingQNotify (&HiiValue, &SelectHiiValue);
        break;
      }
      //
      // Change Select
      //
      if (!HasButton) {
        if (UserInputData->IsKeyboard && UserInputData->KeyData.Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
          HiiValue.Type      = EFI_IFR_TYPE_STRING;
          HiiValue.BufferLen = (UINT16) StrSize (TempStr);
          HiiValue.Buffer    = (UINT8 *) TempStr;

          Status = SendChangeQNotify (0, 0, &HiiValue);
        }
        break;
      }
      Select = mDEPrivate->PopUpSelected;
      if (CheckPressControls (UserInputData->IsKeyboard, &UserInputData->KeyData.Key, UserInputData->CursorX, UserInputData->CursorY, PopUpControls->Count, PopUpControls->ControlArray, &Select)) {
        if (Select == NULL) {
          //
          // Same as Standard
          //
          return EFI_NOT_FOUND;
        } else if (Select == mDEPrivate->PopUpSelected && UserInputData->KeyData.Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
          if (mDEPrivate->PopUpSelected->HiiValue.Value.b) {
            HiiValue.Type      = EFI_IFR_TYPE_STRING;
            HiiValue.BufferLen = (UINT16) StrSize (TempStr);
            HiiValue.Buffer    = (UINT8 *) TempStr;

            Status = SendChangeQNotify (0, 0, &HiiValue);
          } else {
            Status = SendShutDNotify ();
          }
          break;
        } else {
          if (Select->Selectable && !Select->Modifiable) {
            //
            // Prepare HIIValue
            //
            HiiValue.Type = EFI_IFR_TYPE_STRING;
            HiiValue.BufferLen = (UINT16) StrSize (TempStr);
            HiiValue.Buffer = (UINT8 *) TempStr;
            //
            // Notify Changing Question (CHANGING_Q)
            // (In SimpleInput: ChangingQNotify->BodyHiiValue store "Current String")
            // (In SimpleInput: ChangingQNotify->ButtonHiiValue store "PopUpSelected")
            //
            Status = SendChangingQNotify (&HiiValue, &Select->HiiValue);
            break;
            }
        }
      }
      break;

    case DISPLAY_ENGINE_STATUS_OPEN_D:
      if (PopUpControls == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      //
      // Pop-Up Input Dialog
      //
      OpenDNotify = (H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify;
      if (OpenDNotify->Dialog.ButtonCount != 0) {
        HasButton = TRUE;
      } else {
        HasButton = FALSE;
      }
      Status = DisplayDialog (FALSE, &OpenDNotify->Dialog, &OldDialogPanel);
      if (!EFI_ERROR (Status)) {
        //
        // Set PopUpSelected
        //
        if (HasButton) {
          if (PopUpControls->ControlArray != NULL && PopUpControls->Count >= OpenDNotify->Dialog.ButtonCount) {
            mDEPrivate->PopUpSelected = &PopUpControls->ControlArray[PopUpControls->Count - OpenDNotify->Dialog.ButtonCount];
          }
        } else {
          mDEPrivate->PopUpSelected = NULL;
        }
        //
        // Init strings
        //
        StrWidth = OpenDNotify->Dialog.BodyHiiValueArray[0].BufferLen;
        TempStr = CatSPrint (NULL, L"%s", OpenDNotify->Dialog.BodyHiiValueArray[0].Buffer);
        Status = RefreshPasswordInput (PopUpControls->ControlArray, PopUpControls->Count, TempStr);
        FreePool (TempStr);
        //
        // High-Light PopUpSelected
        //
        if (HasButton) {
          DisplayHighLightControl (FALSE, mDEPrivate->PopUpSelected);
        }
      }
      break;

    case DISPLAY_ENGINE_STATUS_SHUT_D:
      Status = ShutDialog ();
      break;

    case DISPLAY_ENGINE_STATUS_CHANGING_Q:
      if (PopUpControls == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      ChangingQNotify = (H2O_DISPLAY_ENGINE_EVT_CHANGING_Q *) Notify;
      //
      // (In SimpleInput: ChangingQNotify->ButtonHiiValue store "PopUpSelected")
      //
      if (HasButton) {
        Status = RefreshPopUpSelectionByValue (PopUpControls, FALSE, FALSE, &mDEPrivate->PopUpSelected->HiiValue);
        Status = RefreshPopUpSelectionByValue (PopUpControls, TRUE, FALSE, &ChangingQNotify->ButtonHiiValue);
      }
      //
      // (In SimpleInput: ChangingQNotify->BodyHiiValue store "Current String")
      //
      TempStr = CatSPrint (NULL, L"%s", (CHAR16 *) ChangingQNotify->BodyHiiValue.Buffer);
      Status = RefreshPasswordInput (PopUpControls->ControlArray, PopUpControls->Count, TempStr);
      FreePool (TempStr);
      //
      // Set PopUpSelected
      //
      if (HasButton) {
        GetPopUpSelectionByValue (PopUpControls, &ChangingQNotify->ButtonHiiValue, &mDEPrivate->PopUpSelected, NULL);
      }
      break;
  }
  return Status;
}

EFI_STATUS
AtCheckBox (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  )
{
  EFI_STATUS                                  Status;
  H2O_CONTROL_INFO                            *Select;
  H2O_PANEL_INFO                              *SetupPagePanel;

  if (Private == NULL || Notify == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;

  switch (Private->DEStatus) {

  case DISPLAY_ENGINE_STATUS_AT_MENU:
    SetupPagePanel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_SETUP_PAGE);
    if (SetupPagePanel == NULL || UserInputData == NULL) {
      break;
    }

    Select = Private->MenuSelected;
    if (CheckPressControls (
          UserInputData->IsKeyboard,
          &UserInputData->KeyData.Key,
          UserInputData->CursorX,
          UserInputData->CursorY,
          SetupPagePanel->ControlList.Count,
          SetupPagePanel->ControlList.ControlArray,
          &Select
          )) {
       if (Select != NULL &&
           Select->PageId     == Private->MenuSelected->PageId &&
           Select->QuestionId == Private->MenuSelected->QuestionId &&
           UserInputData->KeyData.Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
        Private->MenuSelected                   = Select;
        Private->MenuSelected->HiiValue.Type    = EFI_IFR_TYPE_BOOLEAN;
        Private->MenuSelected->HiiValue.Value.b = (BOOLEAN) (Private->MenuSelected->HiiValue.Value.b ? FALSE : TRUE);

        Status = SendChangeQNotify (
                   Private->MenuSelected->PageId,
                   Private->MenuSelected->QuestionId,
                   &Private->MenuSelected->HiiValue
                   );
      }
    }
    break;
  }

  return Status;
}

EFI_STATUS
AtNumeric (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_CONTROL_LIST                   *PopUpControls
  )
{
  EFI_STATUS                                  Status;
  EFI_HII_VALUE                               HiiValue;
  H2O_CONTROL_INFO                            *Control;
  H2O_CONTROL_INFO                            *SelectedControl;
  CHAR16                                      *UpdatedString;
  BOOLEAN                                     IsShutdownDialog;
  BOOLEAN                                     IsChangeQ;
  H2O_FORM_BROWSER_D                          *Dialog;

  if (Private == NULL || Notify == NULL || PopUpControls == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;

  switch (Private->DEStatus) {

  case DISPLAY_ENGINE_STATUS_AT_MENU:
    if (UserInputData == NULL) {
      return Status;
    }

    if (UserInputData->KeyData.Key.UnicodeChar == CHAR_SUB ||
        UserInputData->KeyData.Key.UnicodeChar == CHAR_ADD) {
      Status = GetNextQuestionValue (Private->FBProtocol->CurrentQ, (UserInputData->KeyData.Key.UnicodeChar == CHAR_ADD), &HiiValue);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Status = SendChangeQNotify (0, 0, &HiiValue);
    }
    break;

  case DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG:
    if (UserInputData == NULL) {
      return Status;
    }

    Status = DlgProcessUserInput (
               UserInputData->IsKeyboard,
               &UserInputData->KeyData.Key,
               UserInputData->CursorX,
               UserInputData->CursorY,
               PopUpControls,
               &SelectedControl,
               &UpdatedString,
               &IsShutdownDialog
               );
    if (EFI_ERROR (Status)) {
      break;
    }

    if (SelectedControl != NULL) {
      if (SelectedControl == Private->PopUpSelected) {
        IsChangeQ = FALSE;
        if (SelectedControl->ControlId == H2O_CONTROL_ID_DIALOG_BODY_INPUT) {
          if (UserInputData->IsKeyboard) {
            IsChangeQ = TRUE;
          } else {
            InitializeVirtualKb ();
          }
        }

        if (SelectedControl->ControlId == H2O_CONTROL_ID_DIALOG_BUTTON) {
          if (SelectedControl->HiiValue.Value.b) {
            IsChangeQ = TRUE;
          } else {
            Status = SendShutDNotify ();
          }
        }

        if (IsChangeQ) {
          Control = FindControlByControlId (PopUpControls->ControlArray, PopUpControls->Count, H2O_CONTROL_ID_DIALOG_BODY_INPUT, 0);
          if (Control == NULL) {
            break;
          }

          CopyMem (&HiiValue, &Control->HiiValue, sizeof (EFI_HII_VALUE));
          HiiValue.Value.u64 = IsHexString (Control->Text.String) ? StrHexToUint64 (Control->Text.String) : StrDecimalToUint64 (Control->Text.String);
          if (HiiValue.Value.u64 < Control->Minimum || HiiValue.Value.u64 > Control->Maximum) {
            break;
          }
          Status = SendChangeQNotify (0, 0, &HiiValue);
        }
      } else {
        DisplayControls (FALSE, FALSE, 1, Private->PopUpSelected);
        Private->PopUpSelected = SelectedControl;
        DisplayHighLightControl (FALSE, Private->PopUpSelected);
      }
    }

    if (UpdatedString != NULL) {
      Control = FindControlByControlId (PopUpControls->ControlArray, PopUpControls->Count, H2O_CONTROL_ID_DIALOG_BODY_INPUT, 0);
      if (Control == NULL) {
        break;
      }
      CopyMem (&HiiValue, &Control->HiiValue, sizeof (EFI_HII_VALUE));
      HiiValue.Value.u64 = IsHexString (UpdatedString) ? StrHexToUint64 (UpdatedString) : StrDecimalToUint64 (UpdatedString);
      if (!IsEditValueValid (HiiValue.Value.u64, Control->Minimum, Control->Maximum, IsHexString (UpdatedString))) {
        break;
      }
      Status = SendChangingQNotify (&HiiValue, NULL);
    }

    if (IsShutdownDialog) {
      Status = SendShutDNotify ();
    }
    break;

  case DISPLAY_ENGINE_STATUS_OPEN_D:
    Dialog = &((H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify)->Dialog;
    Status = DisplayDialog (FALSE, Dialog, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }

    Private->PopUpSelected = FindNextSelectableControl (
                               PopUpControls->ControlArray,
                               PopUpControls->Count,
                               NULL,
                               TRUE,
                               FALSE
                               );
    if ((Dialog->H2OStatement->Flags & EFI_IFR_DISPLAY_UINT_HEX) == EFI_IFR_DISPLAY_UINT_HEX) {
      Control = FindControlByControlId (PopUpControls->ControlArray, PopUpControls->Count, H2O_CONTROL_ID_DIALOG_BODY_INPUT, 0);
      if (Control == NULL) {
        break;
      }
      SafeFreePool ((VOID **) &Control->Text.String);
      Control->Text.String = AllocateCopyPool (sizeof(L"0x"), L"0x");
      DisplayControls (FALSE, FALSE, 1, Control);
    }

    DisplayHighLightControl (FALSE, Private->PopUpSelected);
    break;

  case DISPLAY_ENGINE_STATUS_SHUT_D:
    Status = ShutDialog ();
    ShutdownVirtualKb ();
    break;

  case DISPLAY_ENGINE_STATUS_CHANGING_Q:
    Control = FindControlByControlId (PopUpControls->ControlArray, PopUpControls->Count, H2O_CONTROL_ID_DIALOG_BODY_INPUT, 0);
    if (Control == NULL) {
      break;
    }

    if (IsHexString (Control->Text.String)) {
      UpdatedString = CatSPrint (NULL, L"0x%x", ((H2O_DISPLAY_ENGINE_EVT_CHANGING_Q *) Notify)->BodyHiiValue.Value.u64);
    } else {
      UpdatedString = CatSPrint (NULL, L"%d", ((H2O_DISPLAY_ENGINE_EVT_CHANGING_Q *) Notify)->BodyHiiValue.Value.u64);
    }
    SafeFreePool ((VOID **) &Control->Text.String);
    Control->Text.String = UpdatedString;
    if (Private->PopUpSelected == Control) {
      DisplayHighLightControl (FALSE, Control);
    } else {
      DisplayControls (FALSE, FALSE, 1, Control);
    }
    break;
  }

  return Status;
}

EFI_STATUS
AtOneOf (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_CONTROL_LIST                   *PopUpControls
  )
{
  EFI_STATUS                                  Status;
  H2O_CONTROL_INFO                            *Select;
  EFI_HII_VALUE                               CurrentQHiiValue;
  EFI_HII_VALUE                               HiiValue;
  H2O_DISPLAY_ENGINE_EVT_OPEN_D               *OpenDNotify;
  H2O_CONTROL_INFO                            *SelectedControl;
  CHAR16                                      *UpdatedString;
  BOOLEAN                                     IsShutdownDialog;

  if (Private == NULL || Notify == NULL || PopUpControls == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;

  switch (Private->DEStatus) {

  case DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG:
    if (UserInputData == NULL) {
      return Status;
    }

    Status = DlgProcessUserInput (
               UserInputData->IsKeyboard,
               &UserInputData->KeyData.Key,
               UserInputData->CursorX,
               UserInputData->CursorY,
               PopUpControls,
               &SelectedControl,
               &UpdatedString,
               &IsShutdownDialog
               );
    if (EFI_ERROR (Status)) {
      break;
    }

    if (SelectedControl != NULL) {
      if (SelectedControl == Private->PopUpSelected) {
        CopyMem (&CurrentQHiiValue, &Private->PopUpSelected->HiiValue, sizeof (EFI_HII_VALUE));
        if (Private->FBProtocol->CurrentQ != NULL && Private->FBProtocol->CurrentQ->NumberOfOptions != 0) {
          CopyMem (&CurrentQHiiValue, &Private->FBProtocol->CurrentQ->HiiValue, sizeof (EFI_HII_VALUE));
        }
        CopyMem (&HiiValue.Type, &CurrentQHiiValue, sizeof (EFI_HII_VALUE));
        HiiValue.Value.u64 = Private->PopUpSelected->HiiValue.Value.u64;

        Status = SendChangeQNotify (0, 0, &HiiValue);
      } else {
        if (SelectedControl->Selectable) {
          Status = SendChangingQNotify (&SelectedControl->HiiValue, NULL);
        }
      }
    }

    if (IsShutdownDialog) {
      Status = SendShutDNotify ();
    }
    break;

  case DISPLAY_ENGINE_STATUS_OPEN_D:
    if (PopUpControls == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    OpenDNotify = (H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify;
    if (OpenDNotify->Dialog.BodyStringCount == 0 || OpenDNotify->Dialog.BodyHiiValueArray == NULL) {
      break;
    }

    Status = DisplayDialog (FALSE, &OpenDNotify->Dialog, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }

    Status = GetPopUpSelectionByValue (PopUpControls, &OpenDNotify->Dialog.ConfirmHiiValue , &Select, NULL);
    if (EFI_ERROR (Status)) {
      Select = &PopUpControls->ControlArray[0];
    }

    Private->PopUpSelected = Select;
    DisplayHighLightControl (FALSE, Private->PopUpSelected);
    break;

  case DISPLAY_ENGINE_STATUS_SHUT_D:
    Status = ShutDialog ();
    break;

  case DISPLAY_ENGINE_STATUS_CHANGING_Q:
    Status = GetPopUpSelectionByValue (PopUpControls, &((H2O_DISPLAY_ENGINE_EVT_CHANGING_Q *) Notify)->BodyHiiValue, &Select, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }

    DisplayControls (FALSE, FALSE, 1, Private->PopUpSelected);
    Private->PopUpSelected = Select;
    DisplayHighLightControl (FALSE, Private->PopUpSelected);
    break;
  }

  return Status;
}

EFI_STATUS
AtOrderList (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_CONTROL_LIST                   *PopUpControls
  )
{
  EFI_STATUS                                  Status;
  H2O_CONTROL_INFO                            *Select;
  UINT32                                      Index;
  STATIC UINT32                               TabIndex = 0;
  STATIC BOOLEAN                              Exchanged = FALSE;
  STATIC UINT8                                *Buffer = NULL;
  STATIC EFI_HII_VALUE                        OldHiiValue;
  EFI_HII_VALUE                               HiiValue;
  H2O_DISPLAY_ENGINE_EVT_CHANGING_Q           *ChangingQNotify;
  H2O_DISPLAY_ENGINE_EVT_OPEN_D               *OpenDNotify;

  if (Private == NULL || Notify == NULL || PopUpControls == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;

  switch (Private->DEStatus) {

  case DISPLAY_ENGINE_STATUS_AT_MENU:
    if (UserInputData == NULL) {
      return Status;
    }

    if (UserInputData->KeyData.Key.UnicodeChar == CHAR_SUB ||
        UserInputData->KeyData.Key.UnicodeChar == CHAR_ADD) {
      OrderListOpCodeShiftOrder (Private->MenuSelected, UserInputData->KeyData.Key.UnicodeChar == CHAR_SUB);
      return EFI_SUCCESS;
    }
    break;

#if 0
    if (UserInputData->KeyData.Key.ScanCode == SCAN_F5 ||
        UserInputData->KeyData.Key.ScanCode == SCAN_F6 ||
        UserInputData->KeyData.Key.UnicodeChar == CHAR_SUB ||
        UserInputData->KeyData.Key.UnicodeChar == CHAR_ADD) {
      for (Index = 0; Index < Private->FBProtocol->CurrentQ->NumberOfOptions; Index++) {
        if (Private->MenuSelected->HiiValue.Value.u64 == GetHiiBufferValue (Buffer, Private->MenuSelected->HiiValue.Type, Index)) {
          break;
        }
      }
      if (Index >= Private->FBProtocol->CurrentQ->NumberOfOptions) {
        break;
      }
      //
      // Exchanged
      //
      CopyMem (&HiiValue, &Private->MenuSelected->HiiValue, sizeof (EFI_HII_VALUE));
      if (UserInputData->KeyData.Key.UnicodeChar == CHAR_ADD || UserInputData->KeyData.Key.ScanCode == SCAN_F6) {
        if (Index <= 0) {
          break;
        }
        HiiValue.Value.u64 = GetHiiBufferValue (Buffer, Private->MenuSelected->HiiValue.Type, Index - 1);
        SetHiiBufferValue (Buffer, Private->MenuSelected->HiiValue.Type, Index - 1, Private->MenuSelected->HiiValue.Value.u64);
        SetHiiBufferValue (Buffer, Private->MenuSelected->HiiValue.Type, Index, HiiValue.Value.u64);
      } else {
        if (Index >= (UINT32) (Private->FBProtocol->CurrentQ->NumberOfOptions - 1)) {
          break;
        }
        HiiValue.Value.u64 = GetHiiBufferValue (Buffer, Private->MenuSelected->HiiValue.Type, Index + 1);
        SetHiiBufferValue (Buffer, Private->MenuSelected->HiiValue.Type, Index + 1, Private->MenuSelected->HiiValue.Value.u64);
        SetHiiBufferValue (Buffer, Private->MenuSelected->HiiValue.Type, Index, HiiValue.Value.u64);
      }
      //
      // Get Select
      //
      GetPopUpSelectionByValue (PopUpControls, &HiiValue, &Private->MenuSelected, &TabIndex);
      //
      // Prepare HIIValue
      //
      CopyMem (&HiiValue, &Private->FBProtocol->CurrentQ->HiiValue, sizeof (EFI_HII_VALUE));
      CopyMem (HiiValue.Buffer, Buffer, HiiValue.BufferLen);
      //
      // Notify Change Question (CHANGE_Q)
      //
      Status = SendChangeQNotify (Private->FBProtocol->CurrentQ->PageId, Private->FBProtocol->CurrentQ->QuestionId, &HiiValue);
      CopyMem (&OldHiiValue, &HiiValue, sizeof (EFI_HII_VALUE));
      break;
    }
#endif
    break;

    case DISPLAY_ENGINE_STATUS_SELECT_Q:
      Private->MenuSelected->Sequence = 0;
      break;

    case DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG:
      if (PopUpControls == NULL || UserInputData == NULL) {
        return EFI_INVALID_PARAMETER;
      }

      Select = Private->PopUpSelected;
      if (CheckPressControls (UserInputData->IsKeyboard, &UserInputData->KeyData.Key, UserInputData->CursorX, UserInputData->CursorY, PopUpControls->Count, PopUpControls->ControlArray, &Select)) {
        if (Select == NULL) {
          //
          // Same as Standard
          //
          return EFI_NOT_FOUND;
        } else if (Select == Private->PopUpSelected && UserInputData->KeyData.Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
          if (Private->PopUpSelected->HiiValue.Value.b) {
            CopyMem (&HiiValue, &Private->FBProtocol->CurrentQ->HiiValue, sizeof (EFI_HII_VALUE));
            CopyMem (HiiValue.Buffer, Buffer, HiiValue.BufferLen);

            Status = SendChangeQNotify (0, 0, &HiiValue);
          } else {
            Status = SendShutDNotify ();
          }
          break;
        } else {
          //
          // Notify Changing Question (CHANGING_Q)
          //
          Status = SendChangingQNotify (&Select->HiiValue, NULL);
          CopyMem (&OldHiiValue, &Private->PopUpSelected->HiiValue, sizeof (EFI_HII_VALUE));
          break;
        }
      }

      if (UserInputData->KeyData.Key.ScanCode == SCAN_F5 ||
          UserInputData->KeyData.Key.ScanCode == SCAN_F6 ||
          UserInputData->KeyData.Key.UnicodeChar == CHAR_SUB ||
          UserInputData->KeyData.Key.UnicodeChar == CHAR_ADD) {
        for (Index = 0; Index < Private->FBProtocol->CurrentQ->NumberOfOptions; Index ++) {
          if (Private->PopUpSelected->HiiValue.Value.u64 == GetHiiBufferValue (Buffer, Private->PopUpSelected->HiiValue.Type, Index)) {
            break;
          }
        }
        if (Index >= Private->FBProtocol->CurrentQ->NumberOfOptions) {
          break;
        }
        //
        // Exchanged
        //
        Exchanged = TRUE;
        //
        // Prepare HIIValue
        //
        CopyMem (&HiiValue.Type, &Private->PopUpSelected->HiiValue, sizeof (EFI_HII_VALUE));
        if (UserInputData->KeyData.Key.UnicodeChar == CHAR_ADD || UserInputData->KeyData.Key.ScanCode == SCAN_F6) {
          if (Index <= 0) {
            break;
          }
          HiiValue.Value.u64 = GetHiiBufferValue (Buffer, Private->PopUpSelected->HiiValue.Type, Index - 1);
        } else {
          if (Index >= (UINT32) (Private->FBProtocol->CurrentQ->NumberOfOptions - 1)) {
            break;
          }
          HiiValue.Value.u64 = GetHiiBufferValue (Buffer, Private->PopUpSelected->HiiValue.Type, Index + 1);
        }

        Status = SendChangingQNotify (&Private->PopUpSelected->HiiValue, NULL);
        CopyMem (&OldHiiValue, &HiiValue, sizeof (EFI_HII_VALUE));
        break;
      }
      break;

    case DISPLAY_ENGINE_STATUS_OPEN_D:
      if (PopUpControls == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      if (Private->FBProtocol->CurrentQ->NumberOfOptions == 0) {
        break;
      }

      Status = FreeControlList (PopUpControls);
      if (EFI_ERROR (Status)) {
        break;
      }

      OpenDNotify = (H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify;
      Status = DisplayDialog (FALSE, &OpenDNotify->Dialog, NULL);
      if (EFI_ERROR (Status)) {
        break;
      }
      TabIndex = 0;
      Status = RefreshPopUpSelectionByValue (PopUpControls, TRUE, FALSE, &PopUpControls->ControlArray[TabIndex].HiiValue);

      Private->PopUpSelected = &PopUpControls->ControlArray[TabIndex];

      SafeFreePool ((VOID **) &Buffer);
      Buffer = AllocateCopyPool (Private->FBProtocol->CurrentQ->HiiValue.BufferLen, Private->FBProtocol->CurrentQ->HiiValue.Buffer);
      break;

    case DISPLAY_ENGINE_STATUS_SHUT_D:
      Status = ShutDialog ();
      break;

    case DISPLAY_ENGINE_STATUS_CHANGING_Q:
      ChangingQNotify = (H2O_DISPLAY_ENGINE_EVT_CHANGING_Q *) Notify;
      if (Exchanged) {
        ExchangeContainerValue (PopUpControls, Buffer, OldHiiValue.Type, &OldHiiValue, &ChangingQNotify->BodyHiiValue);
        Exchanged = FALSE;
      }
      //
      // Redraw old select
      //
      Status = RefreshPopUpSelectionByValue (PopUpControls, FALSE, FALSE, &OldHiiValue);
      //
      // Select Option
      //
      Status = RefreshPopUpSelectionByValue (PopUpControls, TRUE, FALSE, &ChangingQNotify->BodyHiiValue);
      //
      // Set PopUpSelected
      //
      GetPopUpSelectionByValue (PopUpControls, &ChangingQNotify->BodyHiiValue, &Private->PopUpSelected, &TabIndex);
      break;
  }

  return Status;
}

EFI_STATUS
AtString (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_CONTROL_LIST                   *PopUpControls
  )
{
  EFI_STATUS                                  Status;
  H2O_CONTROL_INFO                            *Control;
  EFI_HII_VALUE                               HiiValue;
  H2O_CONTROL_INFO                            *SelectedControl;
  CHAR16                                      *UpdatedString;
  BOOLEAN                                     IsShutdownDialog;
  BOOLEAN                                     IsChangeQ;
  UINTN                                       StringSize;

  Status = EFI_NOT_FOUND;

  switch (Private->DEStatus) {

  case DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG:
    if (PopUpControls == NULL || UserInputData == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    Status = DlgProcessUserInput (
               UserInputData->IsKeyboard,
               &UserInputData->KeyData.Key,
               UserInputData->CursorX,
               UserInputData->CursorY,
               PopUpControls,
               &SelectedControl,
               &UpdatedString,
               &IsShutdownDialog
               );
    if (EFI_ERROR (Status)) {
      break;
    }

    if (SelectedControl != NULL) {
      if (SelectedControl == Private->PopUpSelected) {
        IsChangeQ = FALSE;

        if (SelectedControl->ControlId == H2O_CONTROL_ID_DIALOG_BODY_INPUT) {
          if (UserInputData->IsKeyboard) {
            IsChangeQ = TRUE;
          } else {
            InitializeVirtualKb ();
          }
        }

        if (SelectedControl->ControlId == H2O_CONTROL_ID_DIALOG_BUTTON) {
          if (SelectedControl->HiiValue.Value.b) {
            IsChangeQ = TRUE;
          } else {
            Status = SendShutDNotify ();
          }
        }

        if (IsChangeQ) {
          Control = FindControlByControlId (PopUpControls->ControlArray, PopUpControls->Count, H2O_CONTROL_ID_DIALOG_BODY_INPUT, 0);
          if (Control == NULL) {
            break;
          }

          StringSize = StrSize (Control->Text.String);

          HiiValue.Type      = EFI_IFR_TYPE_STRING;
          HiiValue.BufferLen = (UINT16) StringSize;
          HiiValue.Buffer    = (UINT8 *) AllocateCopyPool (StringSize, Control->Text.String);
          if (HiiValue.Buffer == NULL) {
            return EFI_OUT_OF_RESOURCES;
          }

          Status = SendChangeQNotify (0, 0, &HiiValue);
        }
      } else {
        DisplayControls (FALSE, FALSE, 1, Private->PopUpSelected);
        Private->PopUpSelected = SelectedControl;
        DisplayHighLightControl (FALSE, Private->PopUpSelected);
      }
    }

    if (UpdatedString != NULL) {
      if (StrSize (UpdatedString) > Private->PopUpSelected->HiiValue.BufferLen) {
        FreePool ((VOID *) UpdatedString);
        break;
      }

      HiiValue.Type      = EFI_IFR_TYPE_STRING;
      HiiValue.BufferLen = (UINT16) StrSize (UpdatedString);
      HiiValue.Buffer    = (UINT8 *) UpdatedString;

      Status = SendChangingQNotify (&HiiValue, NULL);
    }

    if (IsShutdownDialog) {
      Status = SendShutDNotify ();
    }
    break;

  case DISPLAY_ENGINE_STATUS_OPEN_D:
    Status = DisplayDialog (FALSE, &((H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify)->Dialog, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }
    if (PopUpControls == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    Private->PopUpSelected = FindNextSelectableControl (
                               PopUpControls->ControlArray,
                               PopUpControls->Count,
                               NULL,
                               TRUE,
                               FALSE
                               );
    DisplayHighLightControl (FALSE, Private->PopUpSelected);
    break;

  case DISPLAY_ENGINE_STATUS_SHUT_D:
    ShutdownVirtualKb ();
    Status = ShutDialog ();
    break;

  case DISPLAY_ENGINE_STATUS_CHANGING_Q:
    if (PopUpControls == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    Control = FindControlByControlId (PopUpControls->ControlArray, PopUpControls->Count, H2O_CONTROL_ID_DIALOG_BODY_INPUT, 0);
    if (Control == NULL) {
      break;
    }

    SafeFreePool ((VOID **) &Control->Text.String);
    Control->Text.String = (CHAR16 *) (((H2O_DISPLAY_ENGINE_EVT_CHANGING_Q *) Notify)->BodyHiiValue.Buffer);

    if (Private->PopUpSelected == Control) {
      DisplayHighLightControl (FALSE, Control);
    } else {
      DisplayControls (FALSE, FALSE, 1, Control);
    }
    break;
  }

  return Status;
}

EFI_STATUS
AtPassword (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_CONTROL_LIST                   *PopUpControls
  )
{
  EFI_STATUS                                  Status;
  EFI_HII_VALUE                               HiiValue;
  H2O_PANEL_INFO                              *QuestionPanel;
  H2O_CONTROL_INFO                            *SelectedControl;
  CHAR16                                      *UpdatedString;
  BOOLEAN                                     IsShutdownDialog;
  BOOLEAN                                     IsChangeQ;

  Status = EFI_NOT_FOUND;

  switch (Private->DEStatus) {

  case DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG:
    if (UserInputData == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    if (Private->FBProtocol->CurrentP == NULL &&
        UserInputData->IsKeyboard &&
        UserInputData->KeyData.Key.ScanCode == SCAN_ESC) {
      return EFI_SUCCESS;
    }
    Status = DlgProcessUserInput (
               UserInputData->IsKeyboard,
               &UserInputData->KeyData.Key,
               UserInputData->CursorX,
               UserInputData->CursorY,
               PopUpControls,
               &SelectedControl,
               &UpdatedString,
               &IsShutdownDialog
               );
    if (EFI_ERROR (Status)) {
      break;
    }

    if (SelectedControl != NULL) {
      if (SelectedControl == Private->PopUpSelected) {
        IsChangeQ = FALSE;
        if (SelectedControl->ControlId == H2O_CONTROL_ID_DIALOG_BODY_INPUT) {
          if (UserInputData->IsKeyboard) {
            IsChangeQ = TRUE;
          } else {
            InitializeVirtualKb ();
          }
        }

        if (SelectedControl->ControlId == H2O_CONTROL_ID_DIALOG_BUTTON) {
          if (SelectedControl->HiiValue.Value.b) {
            IsChangeQ = TRUE;
          } else {
            Status = SendShutDNotify ();
          }
        }

        if (IsChangeQ) {
          Status = GetPasswordHiiValue (PopUpControls, &HiiValue);
          if (EFI_ERROR (Status)) {
            break;
          }
          Status = SendChangeQNotify (0, 0, &HiiValue);
        }
      } else {
        if (Private->PopUpSelected->ControlId == H2O_CONTROL_ID_DIALOG_BUTTON) {
          DisplayControls (FALSE, FALSE, 1, Private->PopUpSelected);
        }
        Private->PopUpSelected = SelectedControl;
        RefreshPassword (PopUpControls, Private->PopUpSelected, NULL);
        if (Private->PopUpSelected->ControlId == H2O_CONTROL_ID_DIALOG_BUTTON) {
          DisplayHighLightControl (FALSE, Private->PopUpSelected);
        }
      }
    }

    if (UpdatedString != NULL) {
      if (StrSize (UpdatedString) > Private->PopUpSelected->HiiValue.BufferLen) {
        FreePool ((VOID *) UpdatedString);
        break;
      }

      HiiValue.Type      = EFI_IFR_TYPE_STRING;
      HiiValue.BufferLen = (UINT16) StrSize (UpdatedString);
      HiiValue.Buffer    = (UINT8 *) UpdatedString;
      Status = SendChangingQNotify (&HiiValue, NULL);
    }

    if (IsShutdownDialog) {
      Status = SendShutDNotify ();
    }
    break;

  case DISPLAY_ENGINE_STATUS_OPEN_D:
    QuestionPanel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_QUESTION);
    if (QuestionPanel == NULL) {
      break;
    }

    Status = DisplayDialog (FALSE, &((H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify)->Dialog, NULL);
    if (EFI_ERROR (Status)) {
      break;
    }

    Private->PopUpSelected = FindNextSelectableControl (
                                  QuestionPanel->ControlList.ControlArray,
                                  QuestionPanel->ControlList.Count,
                                  NULL,
                                  TRUE,
                                  TRUE
                                  );
    DisplayHighLightControl (FALSE, Private->PopUpSelected);
    break;

  case DISPLAY_ENGINE_STATUS_SHUT_D:
    QuestionPanel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_QUESTION);
    if (QuestionPanel == NULL) {
      break;
    }
    ShutdownVirtualKb ();
    Status = ShutDialog ();
    break;

  case DISPLAY_ENGINE_STATUS_CHANGING_Q:
    RefreshPassword (PopUpControls, Private->PopUpSelected, (CHAR16 *) ((H2O_DISPLAY_ENGINE_EVT_CHANGING_Q *) Notify)->BodyHiiValue.Buffer);
    break;
  }

  return Status;
}

EFI_STATUS
AtDate (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_CONTROL_LIST                   *PopUpControls
  )
{
  EFI_STATUS                                  Status;
  H2O_CONTROL_INFO                            *Select;
  BOOLEAN                                     IsEnterVisibleChar;
  EFI_HII_VALUE                               HiiValue;
  CHAR16                                      *TempStr = NULL;
  H2O_DISPLAY_ENGINE_EVT_CHANGING_Q           *ChangingQNotify;
  H2O_DISPLAY_ENGINE_EVT_OPEN_D               *OpenDNotify;

  STATIC H2O_PANEL_INFO                       *OldDialogPanel = NULL;
  STATIC UINT32                               TabIndex = 0;
  STATIC CHAR16                               *NotifyStr = NULL;

  Status = EFI_UNSUPPORTED;

  switch (Private->DEStatus) {

  case DISPLAY_ENGINE_STATUS_AT_MENU:
    if (UserInputData == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    if (Notify->Type == H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS     ||
        Notify->Type == H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE ||
        Notify->Type == H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE) {
      return DateTimeOpCodeProcessInputEvt (Private, Notify, UserInputData);
    }
    break;

  case DISPLAY_ENGINE_STATUS_SELECT_Q:
    Private->MenuSelected->Sequence = 0;
    break;

  //
  // BUGBUG: Organize code for Date dialog
  //
  case DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG:
    if (UserInputData == NULL) {
      return EFI_INVALID_PARAMETER;
    }
    if (NotifyStr != NULL) {
      FreePool (NotifyStr);
      NotifyStr = NULL;
    }
    //
    // Check Input
    //
    IsEnterVisibleChar = FALSE;
    if (UserInputData->IsKeyboard) {
      ProcessNumberInput (PopUpControls, FALSE, &UserInputData->KeyData.Key, &TabIndex, &IsEnterVisibleChar, &TempStr);
    } else {
      GetInputWithTab (PopUpControls, TabIndex, &TempStr);
    }

    //
    // Regular Date HIIValue and TempStr
    //
    CopyMem (&HiiValue, &Private->FBProtocol->CurrentQ->HiiValue, sizeof (EFI_HII_VALUE));
    if (!IsEnterVisibleChar || TempStr == NULL) {
      if (TabIndex == 0) {
        TempStr = CatSPrint (NULL, L"%04d", HiiValue.Value.date.Year);
      } else if (TabIndex == 1) {
        TempStr = CatSPrint (NULL, L"%02d", HiiValue.Value.date.Month);
      } else {
        TempStr = CatSPrint (NULL, L"%02d", HiiValue.Value.date.Day);
      }
    }
    if (TabIndex == 0) {
      HiiValue.Value.date.Year = (UINT16) StrDecimalToUint64 (TempStr);
      FreePool (TempStr);
      TempStr = CatSPrint (NULL, L"%04d", HiiValue.Value.date.Year);
    } else if (TabIndex == 1) {
      HiiValue.Value.date.Month = (UINT8) StrDecimalToUint64 (TempStr);
      FreePool (TempStr);
      TempStr = CatSPrint (NULL, L"%02d", HiiValue.Value.date.Month);
    } else {
      HiiValue.Value.date.Day = (UINT8) StrDecimalToUint64 (TempStr);
      FreePool (TempStr);
      TempStr = CatSPrint (NULL, L"%02d", HiiValue.Value.date.Day);
    }
    LetDateRegular (&HiiValue);

    if (IsEnterVisibleChar) {
      //
      // Prepare HIIValue
      //
      HiiValue.Type = EFI_IFR_TYPE_STRING;
      HiiValue.BufferLen = (UINT16) StrSize (TempStr);
      HiiValue.Buffer = (UINT8 *) TempStr;
      NotifyStr = TempStr;
      //
      // Press visible char or backspace or tab, notify Changing Question (CHANGING_Q)
      // (In Date: ChangingQNotify->BodyHiiValue store "Current Date Index Str")
      // (In Date: ChangingQNotify->ButtonHiiValue store "PopUpSelected")
      //
      Status = SendChangingQNotify (&HiiValue, &Private->PopUpSelected->HiiValue);
      break;
    }
    //
    // Change Select
    //
    Select = Private->PopUpSelected;
    if (CheckPressControls (UserInputData->IsKeyboard, &UserInputData->KeyData.Key, UserInputData->CursorX, UserInputData->CursorY, PopUpControls->Count, PopUpControls->ControlArray, &Select)) {
      if (Select == NULL) {
        //
        // Same as Standard
        //
        FreePool (TempStr);
        return EFI_NOT_FOUND;
      } else if (Select == Private->PopUpSelected && UserInputData->KeyData.Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
        //
        // Reset TabIndex
        //
        TabIndex = 0;
        FreePool (TempStr);

        if (Private->PopUpSelected->HiiValue.Value.b) {
          Status = SendChangeQNotify (0, 0, &HiiValue);
        } else {
          Status = SendShutDNotify ();
        }
        break;
      } else {
        if (Select->Selectable && !Select->Modifiable) {
          //
          // Prepare HIIValue
          //
          HiiValue.Type = EFI_IFR_TYPE_STRING;
          HiiValue.BufferLen = (UINT16) StrSize (TempStr);
          HiiValue.Buffer = (UINT8 *) TempStr;
          NotifyStr = TempStr;
          //
          // Press visible char or backspace or tab, notify Changing Question (CHANGING_Q)
          // (In Date: ChangingQNotify->BodyHiiValue store "Current Date Index Str")
          // (In Date: ChangingQNotify->ButtonHiiValue store "PopUpSelected")
          //
          Status = SendChangingQNotify (&HiiValue, &Select->HiiValue);
          break;
        }
      }
    }
    break;

  case DISPLAY_ENGINE_STATUS_OPEN_D:
    TabIndex = 0;
    //
    // Create temp value controls
    //
    Status = FreeControlList (PopUpControls);
    if (EFI_ERROR (Status)) {
      break;
    }
    //
    // Pop-Up Input Dialog
    //
    OpenDNotify = (H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify;
    Status = DisplayDialog (FALSE, &OpenDNotify->Dialog, &OldDialogPanel);
    if (EFI_ERROR (Status)) {
      break;
    }
    //
    // Set PopUpSelected
    //
    if (PopUpControls->ControlArray != NULL &&
        OpenDNotify->Dialog.ButtonCount > 0 &&
        PopUpControls->Count >= OpenDNotify->Dialog.ButtonCount) {
      Private->PopUpSelected = &PopUpControls->ControlArray[PopUpControls->Count - OpenDNotify->Dialog.ButtonCount];
    }
    //
    // Init popup Month, Day, Year string
    //
    TempStr = CatSPrint (NULL, L"%02d", Private->FBProtocol->CurrentQ->HiiValue.Value.date.Month);
    Status = RefreshInputWithTab (PopUpControls, 1, TempStr);
    FreePool (TempStr);
    TempStr = CatSPrint (NULL, L"%02d", Private->FBProtocol->CurrentQ->HiiValue.Value.date.Day);
    Status = RefreshInputWithTab (PopUpControls, 2, TempStr);
    FreePool (TempStr);
    TempStr = CatSPrint (NULL, L"%04d", Private->FBProtocol->CurrentQ->HiiValue.Value.date.Year);
    Status = RefreshInputWithTab (PopUpControls, 0, TempStr);
    FreePool (TempStr);
    //
    // High-Light PopUpSelected
    //
    DisplayHighLightControl (FALSE, Private->PopUpSelected);
    break;

  case DISPLAY_ENGINE_STATUS_SHUT_D:
    Status = ShutDialog ();
    if (NotifyStr != NULL) {
      FreePool (NotifyStr);
      NotifyStr = NULL;
    }
    break;

  case DISPLAY_ENGINE_STATUS_CHANGING_Q:
    ChangingQNotify = (H2O_DISPLAY_ENGINE_EVT_CHANGING_Q *) Notify;
    //
    // (In Date: ChangingQNotify->ButtonHiiValue store "PopUpSelected")
    //
    Status = RefreshPopUpSelectionByValue (PopUpControls, FALSE, FALSE, &Private->PopUpSelected->HiiValue);
    Status = RefreshPopUpSelectionByValue (PopUpControls, TRUE, FALSE, &ChangingQNotify->ButtonHiiValue);
    //
    // (In Date: ChangingQNotify->BodyHiiValue store "Current Input String")
    //
    TempStr = CatSPrint (NULL, (CHAR16 *) ChangingQNotify->BodyHiiValue.Buffer);
    Status = RefreshInputWithTab (PopUpControls, TabIndex, TempStr);
    FreePool (TempStr);
    //
    // Set PopUpSelected
    //
    GetPopUpSelectionByValue (PopUpControls, &ChangingQNotify->ButtonHiiValue, &Private->PopUpSelected, NULL);
    break;
  }

  return Status;
}

EFI_STATUS
AtTime (
  IN       H2O_DISPLAY_ENGINE_PRIVATE_DATA    *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData,
  IN       H2O_CONTROL_LIST                   *PopUpControls
  )
{
  EFI_STATUS                                  Status;

  STATIC H2O_PANEL_INFO                       *OldDialogPanel = NULL;
  H2O_CONTROL_INFO                            *Select;

  BOOLEAN                                     IsEnterVisibleChar;
  EFI_HII_VALUE                               HiiValue;

  STATIC UINT32                               TabIndex = 0;
  STATIC CHAR16                               *NotifyStr = NULL;
  CHAR16                                      *TempStr = NULL;

  H2O_DISPLAY_ENGINE_EVT_CHANGING_Q           *ChangingQNotify;
  H2O_DISPLAY_ENGINE_EVT_OPEN_D               *OpenDNotify;


  Status = EFI_NOT_FOUND;
  switch (Private->DEStatus) {

  case DISPLAY_ENGINE_STATUS_AT_MENU:
    if (Notify->Type == H2O_DISPLAY_ENGINE_EVT_TYPE_KEYPRESS     ||
        Notify->Type == H2O_DISPLAY_ENGINE_EVT_TYPE_REL_PTR_MOVE ||
        Notify->Type == H2O_DISPLAY_ENGINE_EVT_TYPE_ABS_PTR_MOVE) {
      return DateTimeOpCodeProcessInputEvt (Private, Notify, UserInputData);
    }
    break;

  case DISPLAY_ENGINE_STATUS_SELECT_Q:
    Private->MenuSelected->Sequence = 0;
    break;

    //
    // BUGBUG: Organize code for Date dialog
    //
    case DISPLAY_ENGINE_STATUS_AT_POPUP_DIALOG:
      if (UserInputData == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      if (NotifyStr != NULL) {
        FreePool (NotifyStr);
        NotifyStr = NULL;
      }
      //
      // Check Input
      //
      IsEnterVisibleChar = FALSE;
      if (UserInputData->IsKeyboard) {
        ProcessNumberInput (PopUpControls, FALSE, &UserInputData->KeyData.Key, &TabIndex, &IsEnterVisibleChar, &TempStr);
      } else {
        GetInputWithTab (PopUpControls, TabIndex, &TempStr);
      }

      //
      // Regular Time HIIValue and TempStr
      //
      CopyMem (&HiiValue, &Private->FBProtocol->CurrentQ->HiiValue, sizeof (EFI_HII_VALUE));
      if (!IsEnterVisibleChar || TempStr == NULL) {
        if (TabIndex == 0) {
          TempStr = CatSPrint (NULL, L"%02d", HiiValue.Value.time.Hour);
        } else if (TabIndex == 1) {
          TempStr = CatSPrint (NULL, L"%02d", HiiValue.Value.time.Minute);
        } else {
          TempStr = CatSPrint (NULL, L"%02d", HiiValue.Value.time.Second);
        }
      }
      if (TabIndex == 0) {
        HiiValue.Value.time.Hour = (UINT8) StrDecimalToUint64 (TempStr);
        FreePool (TempStr);
        TempStr = CatSPrint (NULL, L"%02d", HiiValue.Value.time.Hour);
      } else if (TabIndex == 1) {
        HiiValue.Value.time.Minute = (UINT8) StrDecimalToUint64 (TempStr);
        FreePool (TempStr);
        TempStr = CatSPrint (NULL, L"%02d", HiiValue.Value.time.Minute);
      } else {
        HiiValue.Value.time.Second = (UINT8) StrDecimalToUint64 (TempStr);
        FreePool (TempStr);
        TempStr = CatSPrint (NULL, L"%02d", HiiValue.Value.time.Second);
      }
      LetTimeRegular (&HiiValue);

      if (IsEnterVisibleChar) {
        //
        // Prepare HIIValue
        //
        HiiValue.Type = EFI_IFR_TYPE_STRING;
        HiiValue.BufferLen = (UINT16) StrSize (TempStr);
        HiiValue.Buffer = (UINT8 *) TempStr;
        NotifyStr = TempStr;
        //
        // Press visible char or backspace or tab, notify Changing Question (CHANGING_Q)
        // (In Time: ChangingQNotify->BodyHiiValue store "Current Time Index Str")
        // (In Time: ChangingQNotify->ButtonHiiValue store "PopUpSelected")
        //
        Status = SendChangingQNotify (&HiiValue, &Private->PopUpSelected->HiiValue);
        break;
      }
      //
      // Change Select
      //
      Select = Private->PopUpSelected;
      if (CheckPressControls (UserInputData->IsKeyboard, &UserInputData->KeyData.Key, UserInputData->CursorX, UserInputData->CursorY, PopUpControls->Count, PopUpControls->ControlArray, &Select)) {
        if (Select == NULL) {
          //
          // Same as Standard
          //
          FreePool (TempStr);
          return EFI_NOT_FOUND;
        } else if (Select == Private->PopUpSelected && UserInputData->KeyData.Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
          //
          // Reset TabIndex
          //
          TabIndex = 0;
          FreePool (TempStr);

          if (Private->PopUpSelected->HiiValue.Value.b) {
            Status = SendChangeQNotify (0, 0, &HiiValue);
          } else {
            Status = SendShutDNotify ();
          }
          break;
        } else {
          if (Select->Selectable && !Select->Modifiable) {
            //
            // Prepare HIIValue
            //
            HiiValue.Type = EFI_IFR_TYPE_STRING;
            HiiValue.BufferLen = (UINT16) StrSize (TempStr);
            HiiValue.Buffer = (UINT8 *) TempStr;
            NotifyStr = TempStr;
            //
            // Press visible char or backspace or tab, notify Changing Question (CHANGING_Q)
            // (In Time: ChangingQNotify->BodyHiiValue store "Current Time Index Str")
            // (In Time: ChangingQNotify->ButtonHiiValue store "PopUpSelected")
            //
            Status = SendChangingQNotify (&HiiValue, &Select->HiiValue);
            break;
          }
        }
      }
      break;

    case DISPLAY_ENGINE_STATUS_OPEN_D:
      TabIndex = 0;
      //
      // Create temp value controls
      //
      Status = FreeControlList (PopUpControls);
      if (EFI_ERROR (Status)) {
        break;
      }
      //
      // Pop-Up Input Dialog
      //
      OpenDNotify = (H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify;
      Status = DisplayDialog (FALSE, &OpenDNotify->Dialog, &OldDialogPanel);
      if (EFI_ERROR (Status)) {
        break;
      }

      //
      // Set PopUpSelected
      //
      if (PopUpControls->ControlArray != NULL && OpenDNotify->Dialog.ButtonCount > 0 && PopUpControls->Count >= OpenDNotify->Dialog.ButtonCount) {
        Private->PopUpSelected = &PopUpControls->ControlArray[PopUpControls->Count - OpenDNotify->Dialog.ButtonCount];
      }
      //
      // Init popup Minute, Second, Hour string
      //
      TempStr = CatSPrint (NULL, L"%02d", Private->FBProtocol->CurrentQ->HiiValue.Value.time.Minute);
      Status = RefreshInputWithTab (PopUpControls, 1, TempStr);
      FreePool (TempStr);
      TempStr = CatSPrint (NULL, L"%02d", Private->FBProtocol->CurrentQ->HiiValue.Value.time.Second);
      Status = RefreshInputWithTab (PopUpControls, 2, TempStr);
      FreePool (TempStr);
      TempStr = CatSPrint (NULL, L"%02d", Private->FBProtocol->CurrentQ->HiiValue.Value.time.Hour);
      Status = RefreshInputWithTab (PopUpControls, 0, TempStr);
      FreePool (TempStr);
      //
      // High-Light PopUpSelected
      //
      DisplayHighLightControl (FALSE, Private->PopUpSelected);
      break;

    case DISPLAY_ENGINE_STATUS_SHUT_D:
      Status = ShutDialog ();
      if (NotifyStr != NULL) {
        FreePool (NotifyStr);
        NotifyStr = NULL;
      }
      break;

    case DISPLAY_ENGINE_STATUS_CHANGING_Q:
      ChangingQNotify = (H2O_DISPLAY_ENGINE_EVT_CHANGING_Q *) Notify;
      //
      // (In Time: ChangingQNotify->ButtonHiiValue store "PopUpSelected")
      //
      Status = RefreshPopUpSelectionByValue (PopUpControls, FALSE, FALSE, &Private->PopUpSelected->HiiValue);
      Status = RefreshPopUpSelectionByValue (PopUpControls, TRUE, FALSE, &ChangingQNotify->ButtonHiiValue);
      //
      // (In Time: ChangingQNotify->BodyHiiValue store "Current Input String")
      //
      TempStr = CatSPrint (NULL, (CHAR16 *) ChangingQNotify->BodyHiiValue.Buffer);
      Status = RefreshInputWithTab (PopUpControls, TabIndex, TempStr);
      FreePool (TempStr);
      //
      // Set PopUpSelected
      //
      GetPopUpSelectionByValue (PopUpControls, &ChangingQNotify->ButtonHiiValue, &Private->PopUpSelected, NULL);
      break;
  }

  return Status;
}

EFI_STATUS
GetDialogInfo (
  IN       H2O_PANEL_INFO                     *QuestionPanel,
  IN       UINT8                              DEStatus,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  OUT      BOOLEAN                            *IsQuestion,
  OUT      UINT8                              *QuestionOpCode,
  OUT      BOOLEAN                            *HasInput
  )
{
  H2O_DISPLAY_ENGINE_EVT_OPEN_D               *OpenDNotify;
  H2O_CONTROL_INFO                             *ControlArray;
  UINT32                                       Count;
  UINT32                                       Index;

  if (QuestionPanel == NULL || !IsDialogStatus (DEStatus) || Notify == NULL || IsQuestion == NULL || HasInput == NULL || QuestionOpCode == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get dialog info from OPEN_D event because ControlArray of Question Panel is not initialized.
  //
  if (DEStatus == DISPLAY_ENGINE_STATUS_OPEN_D) {
    OpenDNotify = (H2O_DISPLAY_ENGINE_EVT_OPEN_D *) Notify;

    *IsQuestion     = ((OpenDNotify->Dialog.DialogType & H2O_FORM_BROWSER_D_TYPE_QUESTIONS) == 0) ? FALSE : TRUE;
    *QuestionOpCode = IsQuestion ? GetOpCodeByDialogType (OpenDNotify->Dialog.DialogType) : 0;
    *HasInput       = (OpenDNotify->Dialog.BodyInputCount != 0) ? TRUE : FALSE;

    return EFI_SUCCESS;
  }

  //
  // Get dialog info from ControlArray of Question Panel.
  //
  ControlArray = QuestionPanel->ControlList.ControlArray;
  Count        = QuestionPanel->ControlList.Count;
  if (ControlArray == NULL || Count == 0) {
    return EFI_NOT_FOUND;
  }

  *IsQuestion     = (ControlArray[0].Operand != 0) ? TRUE : FALSE;
  *QuestionOpCode = ControlArray[0].Operand;
  for (Index = 0; Index < Count; Index++) {
    if (ControlArray[Index].ControlId == H2O_CONTROL_ID_DIALOG_BODY_INPUT) {
      break;
    }
  }
  *HasInput = (Index < Count) ? TRUE : FALSE;

  return EFI_SUCCESS;
}

EFI_STATUS
ProcessEvtInQuestionFunc (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA          *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_Q                          *Question;
  H2O_DISPLAY_ENGINE_EVT_SHUT_Q               *ShutQNotify;
  H2O_CONTROL_LIST                            *PopUpControls;
  H2O_PANEL_INFO                              *QuestionPanel;
  BOOLEAN                                     IsQuestion;
  BOOLEAN                                     HasInput;
  UINT8                                       QuestionOpCode;

  QuestionPanel = GetPanelInfoByType (&Private->Layout->PanelListHead, H2O_PANEL_TYPE_QUESTION);
  if (QuestionPanel == NULL) {
    return EFI_NOT_FOUND;
  }
  PopUpControls = &QuestionPanel->ControlList;

  QuestionOpCode = 0;
  if (IsDialogStatus (Private->DEStatus)) {
    Status = GetDialogInfo (QuestionPanel, Private->DEStatus, Notify, &IsQuestion, &QuestionOpCode, &HasInput);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (!IsQuestion) {
      if (HasInput) {
        return AtSimpleInputDialog (Private->FBProtocol, Private->DEStatus, Notify, UserInputData, PopUpControls);
      } else {
        return AtSimpleDialog (Private->FBProtocol, Private->DEStatus, Notify, UserInputData, PopUpControls);
      }
    }
  } else {
    if (Private->MenuSelected != NULL) {
      Status = Private->FBProtocol->GetQInfo (Private->FBProtocol, Private->MenuSelected->PageId, Private->MenuSelected->QuestionId, &Question);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      QuestionOpCode = Question->Operand;
      SafeFreePool ((VOID **) &Question);
    }

    if (Private->DEStatus == DISPLAY_ENGINE_STATUS_SHUT_Q) {
      ShutQNotify = (H2O_DISPLAY_ENGINE_EVT_SHUT_Q *) Notify;
      //
      // SHUT_Q status must get another specific question
      //
      Status = Private->FBProtocol->GetQInfo (Private->FBProtocol, ShutQNotify->PageId, ShutQNotify->QuestionId, &Question);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      QuestionOpCode = Question->Operand;
      SafeFreePool ((VOID **) &Question);
    }
  }

  switch (QuestionOpCode) {

  case EFI_IFR_CHECKBOX_OP:
    Status = AtCheckBox (Private, Notify, UserInputData);
    break;

  case EFI_IFR_NUMERIC_OP:
    Status = AtNumeric (Private, Notify, UserInputData, PopUpControls);
    break;

  case EFI_IFR_ONE_OF_OP:
    Status = AtOneOf (Private, Notify, UserInputData, PopUpControls);
    break;

  case EFI_IFR_ORDERED_LIST_OP:
    Status = AtOrderList (Private, Notify, UserInputData, PopUpControls);
    break;

  case EFI_IFR_STRING_OP:
    Status = AtString (Private, Notify, UserInputData, PopUpControls);
    break;

  case EFI_IFR_PASSWORD_OP:
    Status = AtPassword (Private, Notify, UserInputData, PopUpControls);
    break;

  case EFI_IFR_DATE_OP:
    Status = AtDate (Private, Notify, UserInputData, PopUpControls);
    break;

  case EFI_IFR_TIME_OP:
    Status = AtTime (Private, Notify, UserInputData, PopUpControls);
    break;

  case EFI_IFR_REF_OP:
  case EFI_IFR_ACTION_OP:
  case EFI_IFR_RESET_BUTTON_OP:
  default:
    Status = EFI_NOT_FOUND;
    break;
  }

  return Status;
}

