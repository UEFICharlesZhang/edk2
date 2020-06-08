/** @file
  H2OIhisi Protocol

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

#ifndef _H2O_IHISI_PROTOCOL_H_
#define _H2O_IHISI_PROTOCOL_H_

#include <Protocol/SmmCpu.h>

#define H2O_IHISI_PROTOCOL_GUID \
  { 0x6C23A1EF, 0x2CB7, 0x4A60, 0x8F, 0x8C, 0x08, 0xA3, 0xDE, 0x8D, 0x7A, 0xCF }

typedef struct _H2O_IHISI_PROTOCOL H2O_IHISI_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *IHISI_FUNCTION) (
  VOID
);

typedef struct {
  UINT32                        CmdNumber;
  IHISI_FUNCTION                IhisiFunction;
  UINT8                         Priority;
} IHISI_SMI_SUB_FUNCTION;

typedef enum {
  IhisiLowestPriority        = 0x00,
  IhisiLowPriority           = 0x20,
  IhisiBelowNormalPriority   = 0x40,
  IhisiNormalPriority        = 0x80,
  IhisiAboveNormalPriority   = 0xc0,
  IhisiHighPriority          = 0xe0,
  IhisiHighestPriority       = 0xff
} IHISI_PRIORIY;

/**
  IHISI Registration Protocol function for registering IHISI functions

  @param FunctionNumber         Command code of the IHISI function
  @param IhisiFunction          The IHISI function pointer

  @retval EFI_SUCCESS           IHISI function successfully registered
  @retval EFI_ABORTED           faile to register IHISI function
  @retval EFI_ALREADY_STARTED   IHISI function already registered
**/
typedef
EFI_STATUS
(EFIAPI *IHISI_REGISTER_COMMAND) (
  IN UINT32                     CmdCode,
  IN IHISI_FUNCTION             IhisiFunction,
  IN UINT8                      Priority
  );


/**
  IHISI Registration Protocol function for registering IHISI functions

  @param[in] CmdCode            Command code of the IHISI function
  @param[in] UINT8              FromPriority
  @param[in] UINT8              ToPriority

  @retval EFI_SUCCESS           IHISI function successfully unregistered
  @retval EFI_ABORTED           failed to unregister IHISI function
**/
typedef
EFI_STATUS
(EFIAPI *IHISI_REMOVE_FUNCTIONS) (
  IN UINT32                     CommandCode,
  IN UINT8                      FromPriority,
  IN UINT8                      ToPriority
  );

/**
  Execute IHISI Command

  @param  CmdCode               Command code of IHISI function
  @param  CommBuf               Communication Buffer
  @param  BufSize               Buffer size

  @return                       IHISI Status
**/
typedef
EFI_STATUS
(EFIAPI *IHISI_EXECUTE_COMMAND) (
  IN UINT32                     CommandCode
  );

typedef
UINT32
(EFIAPI *IHISI_READ_CPU_REGISTER32) (
  IN  EFI_SMM_SAVE_STATE_REGISTER       RegisterNum
  );

typedef
EFI_STATUS
(EFIAPI *IHISI_WRITE_CPU_REGISTER32) (
  IN  EFI_SMM_SAVE_STATE_REGISTER       RegisterNum,
  IN  UINT32                            RegisterValue
  );

typedef
BOOLEAN
(EFIAPI *IHISI_BUFFER_OVERLAP_SMRAM) (
  IN  VOID                     *Buffer,
  IN UINTN                      BufferSize
  );

struct _H2O_IHISI_PROTOCOL {
  IHISI_REGISTER_COMMAND                RegisterCommand;
  IHISI_REMOVE_FUNCTIONS                RemoveFunctions;
  IHISI_EXECUTE_COMMAND                 ExecuteCommand;
  IHISI_READ_CPU_REGISTER32             ReadCpuReg32;
  IHISI_WRITE_CPU_REGISTER32            WriteCpuReg32;
  IHISI_BUFFER_OVERLAP_SMRAM            BufferOverlapSmram;
};

extern EFI_GUID gH2OIhisiProtocolGuid;
#endif
