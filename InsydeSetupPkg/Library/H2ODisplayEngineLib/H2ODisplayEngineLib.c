/** @file
   Implement H2O display engine related functions.

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "H2ODisplayEngineLibInternal.h"

/**
  Check if the input character is a decimal character

  @param[in] UnicodeChar     Unicode character

  @retval TRUE               The input character is a decimal character
  @retval FALSE              The input character is not a decimal character
**/
BOOLEAN
IsDecChar (
  IN CHAR16                                   UnicodeChar
  )
{
  return (BOOLEAN) (UnicodeChar >= '0' && UnicodeChar <= '9');
}

/**
  Check if the input character is a hexadecimal character

  @param[in] UnicodeChar     Unicode character

  @retval TRUE               The input character is a hexadecimal character
  @retval FALSE              The input character is not a hexadecimal character
**/
BOOLEAN
IsHexChar (
  IN CHAR16                                   UnicodeChar
  )
{
  if ((UnicodeChar >= '0' && UnicodeChar <= '9') ||
      (UnicodeChar >= 'a' && UnicodeChar <= 'f') ||
      (UnicodeChar >= 'A' && UnicodeChar <= 'F')) {
    return TRUE;
  }

  return FALSE;
}

/**
  Check if the input character is a visible character

  @param[in] UnicodeChar     Unicode character

  @retval TRUE               The input character is a visible character
  @retval FALSE              The input character is not a visible character
**/
BOOLEAN
IsVisibleChar (
  IN CHAR16                                   UnicodeChar
  )
{
  return (BOOLEAN) (UnicodeChar >= ' ' && UnicodeChar <= '~');
}

/**
  Check if the input string is a hexadecimal string

  @param[in] Str             Unicode string

  @retval TRUE               The input string is a hexadecimal string
  @retval FALSE              The input string is not a hexadecimal string
**/
BOOLEAN
IsHexString (
  IN CHAR16                                   *Str
  )
{
  //
  // skip preceeding white space
  //
  while ((*Str != 0) && *Str == L' ') {
    Str++;
  }
  //
  // skip preceeding zeros
  //
  while ((*Str != 0) && *Str == L'0') {
    Str++;
  }

  return (BOOLEAN) (*Str == L'x' || *Str == L'X');
}

/**
  Check if the input year value is leap year

  @param[in] Year            Year value

  @retval TRUE               The input year value is leap year
  @retval FALSE              The input year value is not leap year
**/
BOOLEAN
IsLeapYear (
  IN UINT16                                   Year
  )
{
  return (Year%4 == 0) && ((Year%100 != 0) || (Year%400 == 0));
}

/**
  Check if the input time is valid

  @param[in] EfiTime         Pointer to EFI time

  @retval TRUE               The input time is valid
  @retval FALSE              The input time is not valid
**/
BOOLEAN
IsTimeValid (
  IN EFI_TIME                                 *EfiTime
  )
{
  if (EfiTime == NULL || (EfiTime->Hour > 23) || (EfiTime->Minute > 60) || (EfiTime->Second > 60)) {
    return FALSE;
  }

  return TRUE;
}

/**
  Check if the input day is valid

  @param[in] EfiTime         Pointer to EFI time

  @retval TRUE               The input day is valid
  @retval FALSE              The input day is not valid
**/
BOOLEAN
IsDayValid (
  IN EFI_TIME                                 *EfiTime
  )
{
  UINT8  DayOfMonth[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  if (EfiTime == NULL ||
      (EfiTime->Day < 1) ||
      (EfiTime->Day > DayOfMonth[EfiTime->Month - 1]) ||
      (EfiTime->Month == 2 && (!IsLeapYear (EfiTime->Year) && EfiTime->Day > 28)) ||
      (EfiTime->Year < YEAR_MIN || EfiTime->Year > YEAR_MAX)) {
    return FALSE;
  }

  return TRUE;
}

/**
  Check if current page is root page

  @retval TRUE               The current page is root page
  @retval FALSE              The current page is not root page, fail to locate protocol, current page pointer is NULL or
                             fail to get setup menu.
**/
BOOLEAN
IsRootPage (
  VOID
  )
{
  EFI_STATUS                                  Status;
  H2O_FORM_BROWSER_PROTOCOL                   *FBProtocol;
  H2O_FORM_BROWSER_SM                         *SetupMenuData;
  BOOLEAN                                     Result;
  UINT32                                      Index;

  Result = FALSE;
  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status) || FBProtocol->CurrentP == NULL) {
    return Result;
  }

  Status = FBProtocol->GetSMInfo (FBProtocol, &SetupMenuData);
  if (EFI_ERROR (Status)) {
    return Result;
  }

  for (Index = 0; Index < SetupMenuData->NumberOfSetupMenus; Index++) {
    if (SetupMenuData->SetupMenuInfoList[Index].PageId == FBProtocol->CurrentP->PageId) {
      Result = TRUE;
      break;
    }
  }

  FreeSetupMenuData (SetupMenuData);
  return Result;
}

/**
 Check if the edit value is valid or not.
 The valid condition is that the edit value or passiable range overlap on [MinValue, MaxValue].

 @param[in] EditValue      Edit value
 @param[in] MinValue       Minimum value
 @param[in] MaxValue       Maximum value
 @param[in] IsHex          Flag to determine that the edit value is hex

 @retval TRUE              The edit value is valid
 @retval FALSE             The edit value is not valid
**/
BOOLEAN
IsEditValueValid (
  IN UINT64                                   EditValue,
  IN UINT64                                   MinValue,
  IN UINT64                                   MaxValue,
  IN BOOLEAN                                  IsHex
  )
{
  UINT32                                      Base;
  UINT64                                      PossibleRangeMin;
  UINT64                                      PossibleRangeMax;
  UINT64                                      Limit;

  //
  // First, check if the edit value overlap on [MinValue, MaxValue]
  //
  if (EditValue == 0 || IN_RANGE (EditValue, MinValue, MaxValue)) {
    return TRUE;
  }

  //
  // Second, check if the passible range of edit value overlap on [MinValue, MaxValue]
  // Kepp possible range be multiplied by base until min value of possible range exceed MaxValue.
  // For example: If [MinValue, MaxValue] is [2, 200], possibel range will be [10, 19] and [100, 199] when edit value is 1.
  //
  if (EditValue > MaxValue) {
    return FALSE;
  }

  Base             = IsHex ? 16 : 10;
  Limit            = DivU64x32 ((UINT64) -1, Base);
  PossibleRangeMin = EditValue;
  PossibleRangeMax = EditValue;

  while (PossibleRangeMin <= Limit) {
    PossibleRangeMin = MultU64x32 (PossibleRangeMin, Base);
    PossibleRangeMax = MultU64x32 (PossibleRangeMax, Base) + (Base - 1);
    if (PossibleRangeMin > MaxValue) {
      return FALSE;
    }

    if (IS_OVERLAP (PossibleRangeMin, PossibleRangeMax, MinValue, MaxValue)) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  Transfer EFI time to EFI HII value

  @param[in]  IsDate         If true, transfer to HII value with date type. Otherwise, transfer to HII value with time type.
  @param[in]  EfiTime        Pointer to EFI time
  @param[out] HiiValue       Pointer to EFI HII value

  @retval EFI_SUCCESS             Transfer EFI time to EFI HII value successfully
  @retval EFI_INVALID_PARAMETER   EfiTime or HiiValue is NULL
**/
EFI_STATUS
TransferEfiTimeToHiiValue (
  IN  BOOLEAN                                 IsDate,
  IN  EFI_TIME                                *EfiTime,
  OUT EFI_HII_VALUE                           *HiiValue
  )
{
  if (EfiTime == NULL || HiiValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IsDate) {
    HiiValue->Type              = EFI_IFR_TYPE_DATE;
    HiiValue->Value.date.Year   = EfiTime->Year;
    HiiValue->Value.date.Month  = EfiTime->Month;
    HiiValue->Value.date.Day    = EfiTime->Day;
  } else {
    HiiValue->Type              = EFI_IFR_TYPE_TIME;
    HiiValue->Value.time.Hour   = EfiTime->Hour;
    HiiValue->Value.time.Minute = EfiTime->Minute;
    HiiValue->Value.time.Second = EfiTime->Second;
  }

  return EFI_SUCCESS;
}

/**
  Transfer EFI HII value to EFI time

  @param[in]  HiiValue       Pointer to EFI HII value
  @param[out] EfiTime        Pointer to EFI time

  @retval EFI_SUCCESS             Transfer EFI HII value to EFI time successfully
  @retval EFI_INVALID_PARAMETER   EfiTime or HiiValue is NULL or EFI HII value type is not date or time
**/
EFI_STATUS
TransferHiiValueToEfiTime (
  IN  EFI_HII_VALUE                           *HiiValue,
  OUT EFI_TIME                                *EfiTime
  )
{
  if (EfiTime == NULL || HiiValue == NULL || (HiiValue->Type != EFI_IFR_TYPE_DATE && HiiValue->Type != EFI_IFR_TYPE_TIME)) {
    return EFI_INVALID_PARAMETER;
  }

  if (HiiValue->Type == EFI_IFR_TYPE_DATE) {
    EfiTime->Year   = HiiValue->Value.date.Year;
    EfiTime->Month  = HiiValue->Value.date.Month;
    EfiTime->Day    = HiiValue->Value.date.Day;
  } else {
    EfiTime->Hour   = HiiValue->Value.time.Hour;
    EfiTime->Minute = HiiValue->Value.time.Minute;
    EfiTime->Second = HiiValue->Value.time.Second;
  }

  return EFI_SUCCESS;
}

/**
  Get next date or time value

  @param[in]      DateTimeItem    Date time item
  @param[in]      Increasement    If true, get next value. Otherwise, get previous value
  @param[in, out] EfiTime         Pointer to EFI time

  @retval EFI_SUCCESS             Get next date or time value successfully
  @retval EFI_INVALID_PARAMETER   EfiTime is NULL or DateTimeItem is invalid
  @retval EFI_ABORTED             Fail to get next date value
**/
EFI_STATUS
GetNextDateTimeValue (
  IN     H2O_DATE_TIME_ITEM                   DateTimeItem,
  IN     BOOLEAN                              Increasement,
  IN OUT EFI_TIME                             *EfiTime
  )
{
  UINT8                                       DayOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  UINT8                                       MonthIndex;
  EFI_TIME                                    EfiTimeValue;

  if (EfiTime == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CopyMem (&EfiTimeValue, EfiTime, sizeof(EFI_TIME));

  switch (DateTimeItem) {

  case YearItem:
    if (Increasement) {
      EfiTimeValue.Year = (EfiTimeValue.Year == YEAR_MAX) ? YEAR_MIN : (EfiTimeValue.Year + 1);
    } else {
      EfiTimeValue.Year = (EfiTimeValue.Year == YEAR_MIN) ? YEAR_MAX : (EfiTimeValue.Year - 1);
    }
    break;

  case MonthItem:
    if (Increasement) {
      EfiTimeValue.Month = (EfiTimeValue.Month == 12) ? 1 : (EfiTimeValue.Month + 1);
    } else {
      EfiTimeValue.Month = (EfiTimeValue.Month == 1) ? 12 : (EfiTimeValue.Month - 1);
    }
    break;

  case DayItem:
    MonthIndex = EfiTimeValue.Month - 1;

    if (Increasement) {
      if (EfiTimeValue.Month == 2 && IsLeapYear (EfiTimeValue.Year)) {
        EfiTimeValue.Day = (EfiTimeValue.Day == 29) ? 1 : (EfiTimeValue.Day + 1);
      }  else {
        EfiTimeValue.Day = (EfiTimeValue.Day == DayOfMonth[MonthIndex]) ? 1 : (EfiTimeValue.Day + 1);
      }
    } else {
      if (EfiTimeValue.Month == 2 && IsLeapYear (EfiTimeValue.Year)) {
        EfiTimeValue.Day = (EfiTimeValue.Day == 1) ? 29 : (EfiTimeValue.Day - 1);
      } else {
        EfiTimeValue.Day = (EfiTimeValue.Day == 1) ? DayOfMonth[MonthIndex] : (EfiTimeValue.Day - 1);
      }
    }
    break;

  case HourItem:
    if (Increasement) {
      EfiTimeValue.Hour = (EfiTimeValue.Hour == 23) ? 0  : (EfiTimeValue.Hour + 1);
    } else {
      EfiTimeValue.Hour = (EfiTimeValue.Hour == 0 ) ? 23 : (EfiTimeValue.Hour - 1);
    }
    break;

  case MinuteItem:
    if (Increasement) {
      EfiTimeValue.Minute = (EfiTimeValue.Minute == 59) ? 0 : (EfiTimeValue.Minute + 1);
    } else {
      EfiTimeValue.Minute = (EfiTimeValue.Minute == 0) ? 59 : (EfiTimeValue.Minute - 1);
    }
    break;

  case SecondItem:
    if (Increasement) {
      EfiTimeValue.Second = (EfiTimeValue.Second == 59) ? 0 : (EfiTimeValue.Second + 1);
    } else {
      EfiTimeValue.Second = (EfiTimeValue.Second == 0) ? 59 : (EfiTimeValue.Second - 1);
    }
    break;

  default:
    return EFI_INVALID_PARAMETER;
  }

  if (DateTimeItem == YearItem || DateTimeItem == MonthItem || DateTimeItem == DayItem) {
    while (!IsDayValid (&EfiTimeValue)) {
      EfiTimeValue.Day--;
      if (EfiTimeValue.Day == 0) {
        ASSERT(FALSE);
        return EFI_ABORTED;
      }
    }
  }

  CopyMem (EfiTime, &EfiTimeValue, sizeof(EFI_TIME));
  return EFI_SUCCESS;
}

/**
  Get next question value

  @param[in]  Question            Pointer to form browser question
  @param[in]  GoDown              If true, get next value. Otherwise, get previous value
  @param[out] ResultHiiValue      Pointer to EFI HII value

  @retval EFI_SUCCESS             Get next question value successfully
  @retval EFI_NOT_FOUND           Step value is zero if question is numeric opcode or option list is empty if question is oneof opcode
  @retval EFI_UNSUPPORTED         Question operand is not numeric or oneof opcode
**/
EFI_STATUS
GetNextQuestionValue (
  IN  H2O_FORM_BROWSER_Q                      *Question,
  IN  BOOLEAN                                 GoDown,
  OUT EFI_HII_VALUE                           *ResultHiiValue
  )
{
  EFI_STATUS                                  Status;
  UINT64                                      Step;
  UINT64                                      Minimum;
  UINT64                                      Maximum;
  UINT64                                      EditValue;
  UINT64                                      CurrentValue;
  UINT32                                      Index;
  UINT32                                      OptionCount;
  H2O_FORM_BROWSER_O                          *OptionList;
  H2O_FORM_BROWSER_O                          *TargetOption;

  Status = EFI_NOT_FOUND;

  switch (Question->Operand) {

  case EFI_IFR_NUMERIC_OP:
    if (Question->Step == 0) {
      break;
    }

    Step      = Question->Step;
    Minimum   = Question->Minimum;
    Maximum   = Question->Maximum == 0 ? ((UINT64) -1) : Question->Maximum;
    EditValue = Question->HiiValue.Value.u64;

    if (GoDown) {
      if (EditValue + Step <= Maximum) {
        EditValue = EditValue + Step;
      } else if (EditValue < Maximum) {
        EditValue = Maximum;
      } else {
        EditValue = Minimum;
      }
    } else {
      if (EditValue >= Minimum + Step) {
        EditValue = EditValue - Step;
      } else if (EditValue > Minimum){
        EditValue = Minimum;
      } else {
        EditValue = Maximum;
      }
    }
    CopyMem (ResultHiiValue, &Question->HiiValue, sizeof (EFI_HII_VALUE));
    ResultHiiValue->Value.u64 = EditValue;
    Status = EFI_SUCCESS;
    break;

  case EFI_IFR_ONE_OF_OP:
    if (Question->NumberOfOptions == 0 || Question->Options == NULL) {
      break;
    }

    CurrentValue = Question->HiiValue.Value.u64;
    OptionList   = Question->Options;
    OptionCount  = (UINT32) Question->NumberOfOptions;

    for (Index = 0; Index < OptionCount; Index++) {
      if (OptionList[Index].HiiValue.Value.u64 == CurrentValue) {
        break;
      }
    }
    if (Index == OptionCount) {
      break;
    }

    TargetOption = &OptionList[Index];
    if (GoDown) {
      if (Index != OptionCount - 1) {
        TargetOption = &OptionList[Index + 1];
      } else {
        TargetOption = &OptionList[0];
      }
    } else {
      if (Index != 0) {
        TargetOption = &OptionList[Index - 1];
      } else {
        TargetOption = &OptionList[OptionCount - 1];
      }
    }
    if (TargetOption == &OptionList[Index]) {
      break;
    }
    CopyMem (ResultHiiValue, &Question->HiiValue, sizeof (EFI_HII_VALUE));
    ResultHiiValue->Value.u64 = TargetOption->HiiValue.Value.u64;
    Status = EFI_SUCCESS;
    break;


  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  return Status;
}

/**
  Get opcode value by dialog type

  @param[in] DialogType      Dialog type

  @return opcode value or zero if dialog type is not recognized
**/
UINT8
GetOpCodeByDialogType (
  IN UINT32                                   DialogType
  )
{
  switch ((DialogType & H2O_FORM_BROWSER_D_TYPE_QUESTIONS) >> 16) {

  case H2O_FORM_BROWSER_D_TYPE_ONE_OF:
    return EFI_IFR_ONE_OF_OP;

  case H2O_FORM_BROWSER_D_TYPE_ORDERED_LIST:
    return EFI_IFR_ORDERED_LIST_OP;

  case H2O_FORM_BROWSER_D_TYPE_NUMERIC:
    return EFI_IFR_NUMERIC_OP;

  case H2O_FORM_BROWSER_D_TYPE_STRING:
    return EFI_IFR_STRING_OP;

  case H2O_FORM_BROWSER_D_TYPE_DATE:
    return EFI_IFR_DATE_OP;

  case H2O_FORM_BROWSER_D_TYPE_TIME:
    return EFI_IFR_TIME_OP;

  case H2O_FORM_BROWSER_D_TYPE_PASSWORD:
    return EFI_IFR_PASSWORD_OP;

  default:
    break;
  }

  return 0;
}

/**
  Get HII buffer value by buffer type and index

  @param[in] Buffer          Pointer to buffer
  @param[in] Type            Buffer type
  @param[in] Index           Buffer index

  @return buffer value or zero if Buffer is NULL or Type is not recognized
**/
UINT64
GetHiiBufferValue (
  IN UINT8                                    *Buffer,
  IN UINT8                                    Type,
  IN UINT32                                   Index
  )
{
  UINT64                                      Data;

  ASSERT (Buffer != NULL);
  if (Buffer == NULL) {
    return 0;
  }

  Data = 0;
  switch (Type) {

  case EFI_IFR_TYPE_NUM_SIZE_8:
    Data = (UINT64) *(((UINT8 *) Buffer) + Index);
    break;

  case EFI_IFR_TYPE_NUM_SIZE_16:
    Data = (UINT64) *(((UINT16 *) Buffer) + Index);
    break;

  case EFI_IFR_TYPE_NUM_SIZE_32:
    Data = (UINT64) *(((UINT32 *) Buffer) + Index);
    break;

  case EFI_IFR_TYPE_NUM_SIZE_64:
    Data = (UINT64) *(((UINT64 *) Buffer) + Index);
    break;

  default:
    break;
  }

  return Data;
}

/**
  Set HII buffer value by buffer type and index

  @param[in] Buffer          Pointer to buffer
  @param[in] Type            Buffer type
  @param[in] Index           Buffer index
  @param[in] Value           The value to be set
**/
VOID
SetHiiBufferValue (
  IN UINT8                                    *Buffer,
  IN UINT8                                    Type,
  IN UINT32                                   Index,
  IN UINT64                                   Value
  )
{
  ASSERT (Buffer != NULL);
  if (Buffer == NULL) {
    return;
  }

  switch (Type) {

  case EFI_IFR_TYPE_NUM_SIZE_8:
    *(((UINT8 *) Buffer) + Index) = (UINT8) Value;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_16:
    *(((UINT16 *) Buffer) + Index) = (UINT16) Value;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_32:
    *(((UINT32 *) Buffer) + Index) = (UINT32) Value;
    break;

  case EFI_IFR_TYPE_NUM_SIZE_64:
    *(((UINT64 *) Buffer) + Index) = (UINT64) Value;
    break;

  default:
    break;
  }
}

/**
  Free form browser setup menu data

  @param[in] SetupMenuData   Pointer to form browser setup menu data
**/
VOID
FreeSetupMenuData (
  IN H2O_FORM_BROWSER_SM                      *SetupMenuData
  )
{
  if (SetupMenuData == NULL) {
    return;
  }

  if (SetupMenuData->TitleString != NULL) {
    FreePool (SetupMenuData->TitleString);
  }
  if (SetupMenuData->CoreVersionString != NULL) {
    FreePool (SetupMenuData->CoreVersionString);
  }
  if (SetupMenuData->SetupMenuInfoList != NULL) {
    FreePool (SetupMenuData->SetupMenuInfoList);
  }
  FreePool (SetupMenuData);
}

