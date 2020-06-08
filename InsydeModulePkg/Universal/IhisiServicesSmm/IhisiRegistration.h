/** @file
  Type definition for the IHISI Registration functions

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

#ifndef _IHISI_REGISTRATION_H_
#define _IHISI_REGISTRATION_H_

#include <Uefi.h>
#include <H2OIhisi.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Protocol/H2OIhisi.h>
#include <Protocol/SmmVariable.h>
#include <Protocol/SmmFwBlockService.h>

#define IHISI_EBX_SIGNATURE        SIGNATURE_32 ('O', '2', 'H', '$')

#define IHISI_SIGNATURE            SIGNATURE_32 ('I','H','I','S')

typedef struct {
  UINT32                        Signature;
  LIST_ENTRY                    Link;
  IHISI_FUNCTION                Function;
  UINT8                         Priority;
} IHISI_FUNCTION_ENTRY;

typedef struct _IHISI_FUNCTION_ENTRY {
  UINT32                        Signature;
  LIST_ENTRY                    Link;
  UINT32                        CmdNumber;
  LIST_ENTRY                    FunctionChain;
} IHISI_COMMAND_ENTRY;

typedef struct _IHISI_REGISTRATION_CONTEXT {
  UINT32                        Signature;
  H2O_IHISI_PROTOCOL            Ihisi;
  LIST_ENTRY                    CommandList;
  UINTN                         IhisiCpuIndex;
  EFI_SMM_CPU_PROTOCOL         *SmmCpu;
  UINT32                        IhisiStatus;
} IHISI_CONTEXT;

#define IHISI_CONTEXT_FROM_THIS(a)                 CR(a, IHISI_CONTEXT, Ihisi, IHISI_SIGNATURE)
#define IHISI_COMMAND_ENTRY_FROM_LINK(a)           CR(a, IHISI_COMMAND_ENTRY, Link, IHISI_SIGNATURE)
#define IHISI_FUNCTION_ENTRY_FROM_LINK(a)          CR(a, IHISI_FUNCTION_ENTRY, Link, IHISI_SIGNATURE)

EFI_STATUS
EFIAPI
InitIhisi (
VOID
  );

EFI_STATUS
IhisiRegisterCommand (
  IN UINT32                             CmdCode,
  IN IHISI_FUNCTION                     IhisiFunction,
  IN UINT8                              Priority
  );

EFI_STATUS
IhisiRemoveFunctions (
  IN UINT32                             CmdCode,
  IN UINT8                              FromPriority,
  IN UINT8                              ToPriority
  );

EFI_STATUS
IhisiExecuteFunction (
  UINT32                                CmdCode
  );

UINT32
IhisiReadCpuReg32 (
  IN  EFI_SMM_SAVE_STATE_REGISTER       RegisterNum
  );

EFI_STATUS
IhisiWriteCpuReg32 (
  IN  EFI_SMM_SAVE_STATE_REGISTER       RegisterNum,
  OUT UINT32                            Value
  );

BOOLEAN
EFIAPI
BufferOverlapSmram (
  IN VOID   *Buffer,
  IN UINTN  BufferSize
  );

/**
  Returned error code in AL.

  @param[in] IhisiStatus  Returned error code in AL.
**/
VOID
IhisiErrorCodeHandler (
  IN     UINT32                          IhisiStatus
  );

/**
  IHISI Protocol installation routine

  @retval EFI_SUCCESS:          IHISI Protocol is successfully installed
  @retval Others                Failed to install IHISI Protocol
**/
EFI_STATUS
EFIAPI
InstallIhisiProtocol(
  VOID
  );

extern IHISI_CONTEXT *mIhisiContext;
extern EFI_SMM_FW_BLOCK_SERVICE_PROTOCOL   *mSmmFwBlockService;
extern EFI_SMM_VARIABLE_PROTOCOL *mSmmVariable;

#endif
