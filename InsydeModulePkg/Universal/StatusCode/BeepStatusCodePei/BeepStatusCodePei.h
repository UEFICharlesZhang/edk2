/** @file

  Header file of Beep Status Code PEI implementation.

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

#ifndef _BEEP_STATUS_CODE_PEI_H_
#define _BEEP_STATUS_CODE_PEI_H_

//
// Statements that include other files.
//
#include <BeepStatusCode.h>

#include <Ppi/Speaker.h>
#include <Ppi/ReportStatusCodeHandler.h>

#include <Library/DebugLib.h>
#include <Library/PeiOemSvcKernelLib.h>

EFI_STATUS
EFIAPI
PeiBeepStatusCode (
  IN CONST EFI_PEI_SERVICES         **PeiServices,
  IN EFI_STATUS_CODE_TYPE           CodeType,
  IN EFI_STATUS_CODE_VALUE          CodeValue, 
  IN UINT32                         Instance  OPTIONAL,
  IN CONST EFI_GUID                 *CallerId OPTIONAL,
  IN CONST EFI_STATUS_CODE_DATA     *Data     OPTIONAL
  ); 

EFI_STATUS
EFIAPI
BeepHandler (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN STATUS_CODE_BEEP_ENTRY     *BeepEntry,
  IN BEEP_TYPE                  *BeepType
  );

EFI_STATUS
EFIAPI
RscHandlerPpiNotifyCallback (
  IN CONST EFI_PEI_SERVICES         **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR      *NotifyDescriptor,
  IN VOID                           *Ppi
  );

#endif

