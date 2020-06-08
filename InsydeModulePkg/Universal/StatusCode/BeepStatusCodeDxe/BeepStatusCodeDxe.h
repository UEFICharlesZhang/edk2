/** @file

  Header file of Beep Status Code Dxe implementation.

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

#ifndef _BEEP_STATUS_CODE_DXE_H_
#define _BEEP_STATUS_CODE_DXE_H_

//
// Statements that include other header files
//
#include <BeepStatusCode.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/DxeOemSvcKernelLib.h>

#include <Protocol/ReportStatusCodeHandler.h>
#include <Protocol/Speaker.h>
#include <Protocol/SmmCommunication.h>

#include <Guid/EventGroup.h>

#define SMM_BSC_COMMUNICATION_BUFFER_SIZE (OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data)  + sizeof (SMM_BEEP_STATUS_CODE_COMMUNICATE))

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

VOID
EFIAPI
RscHandlerProtocolCallback (
  IN EFI_EVENT   Event,
  IN VOID        *Context
  );

VOID
EFIAPI
UnregisterBootTimeHandlers (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );

#endif
