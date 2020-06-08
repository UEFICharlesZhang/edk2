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

#ifndef _H2O_TIME_ITEM_H
#define _H2O_TIME_ITEM_H

#include <Protocol/H2OFormBrowser.h>
#include "UiControls.h"

typedef struct _H2O_TIME_ITEM        H2O_TIME_ITEM;
typedef struct _H2O_TIME_ITEM_CLASS  H2O_TIME_ITEM_CLASS;

H2O_TIME_ITEM_CLASS *
EFIAPI
GetTimeItemClass (
  VOID
  );

typedef
VOID
(EFIAPI *UI_TIME_ITEM_ON_TIME_CHANGE) (
  H2O_TIME_ITEM                  *This,
  EFI_TIME                      *EfiTime
  );

struct _H2O_TIME_ITEM {
  UI_CONTROL                    Control;

  UI_NUMBER_PICKER              *HourNumberPicker;
  UI_NUMBER_PICKER              *MinuteNumberPicker;
  UI_NUMBER_PICKER              *SecondNumberPicker;
  UI_NUMBER_PICKER              *SelectedControl;

  //
  // XML defined attritube
  //
  EFI_TIME                      EfiTime;

  UI_TIME_ITEM_ON_TIME_CHANGE   OnTimeChange;
};

struct _H2O_TIME_ITEM_CLASS {
  UI_CONTROL_CLASS              ParentClass;
};


#endif
