/** @file
  Terminal Message protocol definition
  
;******************************************************************************
;* Copyright (c) 2012 - 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#ifndef _TERMINAL_MESSAGE_H_
#define _TERMINAL_MESSAGE_H_

#include <Uefi.h>

typedef enum {
  TM_REFRESH_SCREEN,
  TM_READ_KEY,
  TM_WINDOW_CHANGE,
  TM_SVC_CONIN_ENABLE,
  TM_SVC_CONIN_DISABLE
} TERMINAL_MESSAGE;

typedef 
EFI_STATUS
(EFIAPI *TERMINAL_MESSAGE_CALLBACK) (
  VOID              *Context,
  TERMINAL_MESSAGE  Message,
  VOID              *Parameter
  );
  
#endif // _TERMINAL_MESSAGE_H_