/** @file

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

#ifndef _H2O_HOTKEY_ITEM_H
#define _H2O_HOTKEY_ITEM_H

#include <Protocol/H2OFormBrowser.h>
typedef struct _H2O_HOTKEY_ITEM          H2O_HOTKEY_ITEM;
typedef struct _H2O_HOTKEY_ITEM_CLASS    H2O_HOTKEY_ITEM_CLASS;

H2O_HOTKEY_ITEM_CLASS *
EFIAPI
GetHotkeyItemClass (
  VOID
  );

#define UI_HOTKEY_ITEM_CHILD_KEY_MAX    10
struct _H2O_HOTKEY_ITEM {
  UI_CONTROL                    Parent;
  UINT32                        NumberOfHotkey;
  HOT_KEY_INFO                  *HotkeyInfo[UI_HOTKEY_ITEM_CHILD_KEY_MAX];
};

struct _H2O_HOTKEY_ITEM_CLASS {
  UI_CONTROL_CLASS              ParentClass;
};

#endif
