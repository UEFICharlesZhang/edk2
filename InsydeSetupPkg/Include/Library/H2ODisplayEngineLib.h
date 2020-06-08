/** @file
  H2ODisplayEngineLib library include file

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

#ifndef _H2O_DISPLAY_ENGINE_LIB_H_
#define _H2O_DISPLAY_ENGINE_LIB_H_

#include <Uefi/UefiInternalFormRepresentation.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/H2OFormBrowser.h>
#include <Protocol/DisplayProtocol.h>

#define YEAR_MIN                              2000
#define YEAR_MAX                              2099

#define IN_RANGE(i, min, max) ((i >= min) && (i <= max)) ? TRUE : FALSE
#define IS_OVERLAP(Start1, End1, Start2, End2) ((End1 < Start2) || (Start1 > End2)) ? FALSE : TRUE

typedef enum {
  YearItem,
  MonthItem,
  DayItem,
  HourItem,
  MinuteItem,
  SecondItem,
  UnknownItem
} H2O_DATE_TIME_ITEM;

EFI_STATUS
SendChangeQNotify (
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_HII_VALUE                            *HiiValue
  );

EFI_STATUS
SendChangingQNotify (
  IN EFI_HII_VALUE                            *BodyHiiValue,
  IN EFI_HII_VALUE                            *ButtonHiiValue
  );

EFI_STATUS
SendShutDNotify (
  VOID
  );

EFI_STATUS
SendSelectQNotify (
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  );

EFI_STATUS
SendOpenQNotify (
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  );

EFI_STATUS
SendSelectPNotify (
  IN H2O_PAGE_ID                              PageId
  );

EFI_STATUS
SendDefaultNotify (
  VOID
  );

EFI_STATUS
SendSubmitExitNotify (
  VOID
  );

EFI_STATUS
SendDiscardExitNotify (
  VOID
  );

EFI_STATUS
SendSubmitNotify (
  VOID
  );

EFI_STATUS
SendDiscardNotify (
  VOID
  );

EFI_STATUS
SendRefreshNotify (
  VOID
  );

EFI_STATUS
SendShowHelpNotify (
  VOID
  );

BOOLEAN
IsDecChar (
  IN CHAR16                                   UnicodeChar
  );

BOOLEAN
IsHexChar (
  IN CHAR16                                   UnicodeChar
  );

BOOLEAN
IsHexString (
  IN CHAR16                                   *Str
  );

BOOLEAN
IsVisibleChar (
  IN CHAR16                                   UnicodeChar
  );

BOOLEAN
IsDayValid (
  IN EFI_TIME                                 *EfiTime
  );

BOOLEAN
IsTimeValid (
  IN EFI_TIME                                 *EfiTime
  );

BOOLEAN
IsLeapYear (
  IN UINT16                                   Year
  );

BOOLEAN
IsRootPage (
  VOID
  );

BOOLEAN
IsEditValueValid (
  IN UINT64                                   EditValue,
  IN UINT64                                   MinValue,
  IN UINT64                                   MaxValue,
  IN BOOLEAN                                  IsHex
  );

EFI_STATUS
TransferEfiTimeToHiiValue (
  IN  BOOLEAN                                 IsDate,
  IN  EFI_TIME                                *EfiTime,
  OUT EFI_HII_VALUE                           *HiiValue
  );

EFI_STATUS
TransferHiiValueToEfiTime (
  IN  EFI_HII_VALUE                           *HiiValue,
  OUT EFI_TIME                                *EfiTime
  );

EFI_STATUS
GetNextDateTimeValue (
  IN     H2O_DATE_TIME_ITEM                   DateTimeItem,
  IN     BOOLEAN                              Increasement,
  IN OUT EFI_TIME                             *EfiTime
  );

VOID
FreeSetupMenuData (
  IN H2O_FORM_BROWSER_SM                      *SetupMenuData
  );

EFI_STATUS
GetNextQuestionValue (
  IN  H2O_FORM_BROWSER_Q                      *Question,
  IN  BOOLEAN                                 GoDown,
  OUT EFI_HII_VALUE                           *ResultHiiValue
  );

UINT8
GetOpCodeByDialogType (
  IN UINT32                                   DialogType
  );

UINT64
GetHiiBufferValue (
  IN UINT8                                    *Buffer,
  IN UINT8                                    Type,
  IN UINT32                                   Index
  );

VOID
SetHiiBufferValue (
  IN UINT8                                    *Buffer,
  IN UINT8                                    Type,
  IN UINT32                                   Index,
  IN UINT64                                   Value
  );

#endif

