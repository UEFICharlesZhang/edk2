/** @file

  Header file of H2O Setup Change DXE implementation.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _H2O_SETUP_CHANGE_H_
#define _H2O_SETUP_CHANGE_H_

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/VariableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/H2ODialog.h>
#include <Protocol/H2OSubmitSvc.h>
#include <Protocol/H2OFormBrowser.h>
#include <Guid/SetupChangeVariable.h>
#include <Uefi/UefiBaseType.h>

//
// TimeStamp use
//
#define BASE_YEAR                           1970
#define BASE_MONTH                          1
#define BASE_DAY                            1

#define DAYS_PER_YEAR                       365
#define HOURS_PER_DAY                       24
#define MINUTES_PER_HOUR                    60
#define SECONDS_PER_MINUTE                  60
#define SECONDS_PER_DAY                     86400
#define SECONDS_PER_HOUR                    3600

//
//The policy when there is not enough space to record the all history of Setup Change.
//  0: Do Nothing. Stop recording the history.
//  1: Clear all of the old history.
//  2: Overwrite the older history. Delete the oldest history to have enough space for new history.
#define SETUP_CHANGE_MAX_OPTION             3
#define SETUP_CHANGE_DISPLAY_DO_NOTHING     0
#define SETUP_CHANGE_DISPLAY_CLEAR          1
#define SETUP_CHANGE_DISPLAY_OVERWRITE      2
#define SETUP_CHANGE_MAX_OPTION_STRING_SIZE 100

#define MAX_SINGLE_SETTING_STRING_SIZE              0x30
#define MAX_SINGLE_CHANGED_STATEMENT_STRING_SIZE    0x100  

#define FORM_BROWSER_STATEMENT_CHANGED_SIGNATURE    SIGNATURE_32 ('F', 'B', 'S', 'C')

typedef struct _FORM_BROWSER_STATEMENT_CHANGED FORM_BROWSER_STATEMENT_CHANGED;

struct _FORM_BROWSER_STATEMENT_CHANGED {
  UINTN                           Signature;
  LIST_ENTRY                      Link;
  H2O_FORM_BROWSER_Q              Question;
  CHAR16                          *ChangedInfoStr;
} ;

#define FORM_BROWSER_STATEMENT_CHANGED_FROM_LINK(a)       CR (a, FORM_BROWSER_STATEMENT_CHANGED, Link, FORM_BROWSER_STATEMENT_CHANGED_SIGNATURE)

EFI_STATUS
EFIAPI
ExecuteSubmitSvc (
  IN    H2O_SUBMIT_SVC_PROTOCOL             *This,
  OUT   UINT32                              *Request,
  OUT   BOOLEAN                             *ShowSubmitDialog
  );

#endif  
