/** @file

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "LayoutSupportLib.h"

UINT32
GetStyleTypeByOpCode (
  IN    UINT8                                  OpCode
  )
{
  switch (OpCode) {

  case EFI_IFR_SUBTITLE_OP:
    return H2O_IFR_STYLE_TYPE_SUBTITLE;

  case EFI_IFR_CHECKBOX_OP:
    return H2O_IFR_STYLE_TYPE_CHECKBOX;

  case EFI_IFR_TEXT_OP:
    return H2O_IFR_STYLE_TYPE_TEXT;

  case EFI_IFR_ACTION_OP:
    return H2O_IFR_STYLE_TYPE_ACTION;

  case EFI_IFR_REF_OP:
    return H2O_IFR_STYLE_TYPE_GOTO;

  case EFI_IFR_PASSWORD_OP:
    return H2O_IFR_STYLE_TYPE_PASSWORD;

  case EFI_IFR_NUMERIC_OP:
    return H2O_IFR_STYLE_TYPE_NUMERIC;

  case EFI_IFR_ONE_OF_OP:
    return H2O_IFR_STYLE_TYPE_ONEOF;

  case EFI_IFR_TIME_OP:
    return H2O_IFR_STYLE_TYPE_TIME;

  case EFI_IFR_DATE_OP:
    return H2O_IFR_STYLE_TYPE_DATE;

  case EFI_IFR_ORDERED_LIST_OP:
    return H2O_IFR_STYLE_TYPE_ORDEREDLIST;

  case EFI_IFR_RESET_BUTTON_OP:
    return H2O_IFR_STYLE_TYPE_RESETBUTTON;

  case EFI_IFR_STRING_OP:
    return H2O_IFR_STYLE_TYPE_STRING;

  default:
    break;
  }

  ASSERT (FALSE);
  return H2O_IFR_STYLE_TYPE_SHEET;
}

