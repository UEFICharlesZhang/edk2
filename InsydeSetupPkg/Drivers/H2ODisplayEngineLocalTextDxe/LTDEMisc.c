/** @file
  Common functions for H2O display engine driver.

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

#include <H2ODisplayEngineLocalText.h>

CHAR16 *
PrintFormattedNumber (
  IN H2O_FORM_BROWSER_Q                       *Question
  )
{
  INT64                                       Value;
  CHAR16                                      *Format;
  EFI_HII_VALUE                               *QuestionValue;
  CHAR16                                      FormattedNumber[21];

  QuestionValue = &Question->HiiValue;

  Value = (INT64) QuestionValue->Value.u64;
  switch (Question->Flags & EFI_IFR_DISPLAY) {

  case EFI_IFR_DISPLAY_INT_DEC:
    switch (QuestionValue->Type) {

    case EFI_IFR_NUMERIC_SIZE_1:
      Value = (INT64) ((INT8) QuestionValue->Value.u8);
      break;

    case EFI_IFR_NUMERIC_SIZE_2:
      Value = (INT64) ((INT16) QuestionValue->Value.u16);
      break;

    case EFI_IFR_NUMERIC_SIZE_4:
      Value = (INT64) ((INT32) QuestionValue->Value.u32);
      break;

    case EFI_IFR_NUMERIC_SIZE_8:
    default:
      break;
    }

    if (Value < 0) {
      Value = -Value;
      Format = L"-%ld";
    } else {
      Format = L"%ld";
    }
    break;

  case EFI_IFR_DISPLAY_UINT_DEC:
    Format = L"%ld";
    break;

  case EFI_IFR_DISPLAY_UINT_HEX:
    Format = L"%lx";
    break;

  default:
    return NULL;
  }

  UnicodeSPrint (FormattedNumber, sizeof (FormattedNumber), Format, Value);

  return AllocateCopyPool (StrSize (FormattedNumber), FormattedNumber);
}

CHAR16 *
CreateSpaceString (
  IN UINT32                                   StringLength
  )
{
  CHAR16                                      *String;
  UINT32                                      Index;

  String = AllocateZeroPool ((StringLength + 1) * sizeof (CHAR16));
  if (String == NULL) {
    return NULL;
  }

  for (Index = 0; Index < StringLength; Index++) {
    String[Index] = L' ';
  }

  return String;
}

