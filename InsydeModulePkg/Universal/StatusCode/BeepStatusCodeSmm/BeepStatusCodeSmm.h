/** @file

  Header file of Beep Status Code SMM implementation.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _BEEP_STATUS_CODE_SMM_H_
#define _BEEP_STATUS_CODE_SMM_H_

//
// Statements that include other header files
//
#include <BeepStatusCode.h>

#include <Library/SmmServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/SmmOemSvcKernelLib.h>

#include <Protocol/SmmReportStatusCodeHandler.h>
#include <Protocol/Speaker.h>
#include <Guid/EventGroup.h>

EFI_STATUS
EFIAPI
BeepStatusCode (
  IN EFI_STATUS_CODE_TYPE           CodeType,
  IN EFI_STATUS_CODE_VALUE          CodeValue,
  IN UINT32                         Instance    OPTIONAL,
  IN EFI_GUID                       * CallerId  OPTIONAL,
  IN EFI_STATUS_CODE_DATA           * Data      OPTIONAL  
  ); 

EFI_STATUS
EFIAPI
BeepHandler (
  IN STATUS_CODE_BEEP_ENTRY     *BeepEntry,
  IN BEEP_TYPE                  *BeepType
  );

EFI_STATUS
EFIAPI
RscHandlerProtocolCallback (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  );

#endif
