/** @file

Copyright (c) 2010 - 2015, Byosoft Corporation.<BR>
All rights reserved.This software and associated documentation (if any)
is furnished under a license and may only be used or copied in
accordance with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be reproduced,
stored in a retrieval system, or transmitted in any form or by any
means without the express written consent of Byosoft Corporation.

**/

#ifndef __CUSTOMIZED_DISPLAY_LIB_H__
#define __CUSTOMIZED_DISPLAY_LIB_H__

#include <Protocol/SetupMouseProtocol.h>

typedef enum {
  TEXT_ALIGIN_CENTER,
  TEXT_ALIGIN_LEFT,
  TEXT_ALIGIN_RIGHT,
  TEXT_ALIGIN_MAX
} TEXT_ALIGIN;

typedef enum {
  DIALOG_YESNO,
  DIALOG_WARNING,
  DIALOG_INFO,
  DIALOG_NO_KEY,
  DIALOG_MAX
} DIALOG_TYPE;

typedef enum {
  USER_INPUT_KEY,
  USER_INPUT_MOUSE,
  USER_INPUT_MAX
} USER_INPUT_TYPE;

typedef struct {
  UINTN    Left;
  UINTN    Right;
  UINTN    Top;
  UINTN    Bottom;
} DIALOG_RANGE;

/**
  Draw a confirm pop up windows based on the Type, strings and Text Align. 
  TRUE will be return If YES be selected.

**/
BOOLEAN
UiConfirmDialog (
  IN DIALOG_TYPE    Type,
  IN CHAR16    *Title,
  IN TEXT_ALIGIN   Align,
  IN  CHAR16    *String,
  ...
  );

/**
  Wait for user Input by Key or Mouse.

  @param Key         The key which is pressed by user.
  @param Mouse      The Mouse which is pressed by user.

  @retval USER_INPUT_TYPE  The Input type

**/
USER_INPUT_TYPE
WaitForUserInput (
  OUT  EFI_INPUT_KEY    *Key,
  OUT  MOUSE_ACTION_INFO *Mouse
  );

/**
  Set range of item.

  @param Range      The Range of a item.
  @param StartColum
  @param Length
  @param StartRow
  @param Heigth
  
  @retval EFI_STATUS

**/
EFI_STATUS 
SetRange (
  IN  MOUSE_CURSOR_RANGE    *Range,
  IN  UINTN    StartColum,
  IN  UINTN    Length,
  IN  UINTN    StartRow,
  IN  UINTN    Height 
);

/**
  Check whether mouse in the range.

  @param Range      The Range of a item.
  @param Mouse      The Mouse which is pressed by user.

  @retval BOOLEAN  The Input type

**/
BOOLEAN 
IsMouseInRange (
  IN  MOUSE_CURSOR_RANGE    *Range,
  IN  MOUSE_ACTION_INFO    *Mouse
);

/**
  Set Title of Button form Left to Right according Type of Dialog which will 
  be created by UiConfirmDialog(). Setting valid once.

**/
BOOLEAN
UiSetButton (
  IN DIALOG_TYPE    Type,
  IN  CHAR16    *String,
  ...
  );

/**
  Restore Backgroud before creating it and Free memory.
  This service  is only for DIALOG_NO_KEY dialog now.

**/
BOOLEAN
UiClearDialog (
  IN DIALOG_TYPE    Type
  );
  
#endif
