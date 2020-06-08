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

#ifndef _H2O_DATE_ITEM_H
#define _H2O_DATE_ITEM_H

#include <Protocol/H2OFormBrowser.h>
#include "UiControls.h"

typedef struct _H2O_DATE_ITEM        H2O_DATE_ITEM;
typedef struct _H2O_DATE_ITEM_CLASS  H2O_DATE_ITEM_CLASS;

H2O_DATE_ITEM_CLASS *
EFIAPI
GetDateItemClass (
  VOID
  );

typedef
VOID
(EFIAPI *H2O_DATE_ITEM_ON_DATE_CHANGE) (
  H2O_DATE_ITEM                 *This,
  EFI_TIME                      *EfiTime
  );

struct _H2O_DATE_ITEM {
  UI_CONTROL                    Control;

  UI_NUMBER_PICKER              *YearNumberPicker;
  UI_NUMBER_PICKER              *MonthNumberPicker;
  UI_NUMBER_PICKER              *DayNumberPicker;
  UI_NUMBER_PICKER              *SelectedControl;

  //
  // XML defined attritube
  //
  EFI_TIME                      EfiTime;

  H2O_DATE_ITEM_ON_DATE_CHANGE  OnDateChange;
};

struct _H2O_DATE_ITEM_CLASS {
  UI_CONTROL_CLASS              ParentClass;
};


#endif
