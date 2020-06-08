/** @file

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#ifndef _UI_SCROLL_VIEW_H
#define _UI_SCROLL_VIEW_H

typedef struct _UI_SCROLL_VIEW UI_SCROLL_VIEW;
typedef struct _UI_SCROLL_VIEW_CLASS UI_SCROLL_VIEW_CLASS;

UI_SCROLL_VIEW_CLASS *
EFIAPI
GetScrollViewClass (
  VOID
  );

struct _UI_SCROLL_VIEW {
  UI_CONTROL                    Control;

};


struct _UI_SCROLL_VIEW_CLASS {
  UI_CONTROL                    ParentClass;
};

#endif
