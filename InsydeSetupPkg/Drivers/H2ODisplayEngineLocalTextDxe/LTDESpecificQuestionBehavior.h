/** @file

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

#ifndef _LTDE_SPECIFIC_QUESTION_BEHAVIOR_H_
#define _LTDE_SPECIFIC_QUESTION_BEHAVIOR_H_

#include "H2ODisplayEngineLocalText.h"
#include <Protocol/SetupMouse.h>

typedef struct _H2O_DISPLAY_ENGINE_USER_INPUT_DATA {
  BOOLEAN                            IsKeyboard;
  EFI_KEY_DATA                       KeyData;
  UINT32                             CursorX;
  UINT32                             CursorY;
} H2O_DISPLAY_ENGINE_USER_INPUT_DATA;

CHAR16 *
GetQuestionPromptStr (
  IN H2O_FORM_BROWSER_Q                       *Question
  );

CHAR16 *
GetQuestionValueStr (
  IN     H2O_FORM_BROWSER_Q                   *Question
  );

EFI_STATUS
ProcessEvtInQuestionFunc (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA          *Private,
  IN CONST H2O_DISPLAY_ENGINE_EVT             *Notify,
  IN       H2O_DISPLAY_ENGINE_USER_INPUT_DATA *UserInputData
  );

H2O_CONTROL_INFO *
FindNextSelectableControl (
  IN H2O_CONTROL_INFO                         *ControlArray,
  IN UINT32                                   ControlArrayCount,
  IN H2O_CONTROL_INFO                         *CurrentControl,
  IN BOOLEAN                                  GoDown,
  IN BOOLEAN                                  IsLoop
  );

BOOLEAN
IsValidHighlightStatement (
  IN H2O_CONTROL_INFO                            *StatementControl
  );

BOOLEAN
IsQuestionBeingModified (
  IN H2O_CONTROL_INFO                            *Control
  );

BOOLEAN
IsCurrentHighlight (
  IN H2O_DISPLAY_ENGINE_PRIVATE_DATA          *Private,
  IN H2O_PAGE_ID                              PageId,
  IN EFI_QUESTION_ID                          QuestionId,
  IN EFI_IFR_OP_HEADER                        *IfrOpCode
  );

H2O_DATE_TIME_ITEM
DateTimeOpCodeGetItemValue (
  IN H2O_CONTROL_INFO                         *Control
  );

EFI_STATUS
DateTimeOpCodeDisplayValueStr (
  IN H2O_CONTROL_INFO                         *Control,
  IN RECT                                     *ValueStrField,
  IN UINT32                                   NormalAttribute,
  IN UINT32                                   HighlightAttribute,
  IN BOOLEAN                                  IsHighlight
  );

EFI_STATUS
OrderListOpCodeShiftOrder (
  IN H2O_CONTROL_INFO                         *Control,
  IN BOOLEAN                                  ShiftNext
  );

#endif
