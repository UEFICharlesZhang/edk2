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

#ifndef _H2O_SETUP_MENU_ITEM_H
#define _H2O_SETUP_MENU_ITEM_H

typedef struct _H2O_SETUP_MENU_ITEM        H2O_SETUP_MENU_ITEM;
typedef struct _H2O_SETUP_MENU_ITEM_CLASS  H2O_SETUP_MENU_ITEM_CLASS;

H2O_SETUP_MENU_ITEM_CLASS *
EFIAPI
GetSetupMenuItemClass (
  VOID
  );

struct _H2O_SETUP_MENU_ITEM {
  UI_CONTROL                    Control;
  SETUP_MENU_INFO               *SetupMenuInfo;
};

struct _H2O_SETUP_MENU_ITEM_CLASS {
  UI_CONTROL_CLASS              ParentClass;
};


#endif
